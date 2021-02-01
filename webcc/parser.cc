#include "webcc/parser.h"

#include "boost/filesystem/operations.hpp"

#include "webcc/logger.h"
#include "webcc/message.h"
#include "webcc/string.h"
#include "webcc/utility.h"

#ifdef WEBCC_ENABLE_GZIP
#include "webcc/gzip.h"
#endif

namespace bfs = boost::filesystem;

namespace webcc {

// -----------------------------------------------------------------------------

bool BodyHandler::IsCompressed() const {
  return message_->GetContentEncoding() != ContentEncoding::kUnknown;
}

// -----------------------------------------------------------------------------

// TODO
//  // Reserve memory to avoid frequent reallocation when append.
//  try {
//    content_.reserve(content_length_);
//  } catch (const std::exception& e) {
//    LOG_ERRO("Failed to reserve content memory: %s.", e.what());
//  }

void StringBodyHandler::AddContent(const char* data, std::size_t count) {
  content_.append(data, count);
}

void StringBodyHandler::AddContent(const std::string& data) {
  content_.append(data);
}

bool StringBodyHandler::Finish() {
  if (content_.empty()) {
    // The call to message_->SetBody() is not necessary since message is
    // always initialized with an empty body.
    return true;
  }

  auto body = std::make_shared<StringBody>(std::move(content_), IsCompressed());

#ifdef WEBCC_ENABLE_GZIP
  LOG_INFO("Decompress the HTTP content...");
  if (!body->Decompress()) {
    LOG_ERRO("Cannot decompress the HTTP content!");
    return false;
  }
#else
  if (body->compressed()) {
    LOG_WARN("Compressed HTTP content remains untouched.");
  }
#endif  // WEBCC_ENABLE_GZIP

  message_->SetBody(body, false);

  return true;
}

// -----------------------------------------------------------------------------

bool FileBodyHandler::OpenFile() {
  try {
    temp_path_ = bfs::temp_directory_path();

    // Generate a random string as file name.
    // A replacement of boost::filesystem::unique_path().
    temp_path_ /= random_string(10);

    LOG_VERB("Generate a temp path for streaming: %s",
             temp_path_.string().c_str());

  } catch (const bfs::filesystem_error&) {
    LOG_ERRO("Failed to generate temp path for streaming.");
    return false;
  }

  ofstream_.open(temp_path_, std::ios::binary);

  if (ofstream_.fail()) {
    LOG_ERRO("Failed to open the temp file: %s", temp_path_.string().c_str());
    return false;
  }

  return true;
}

void FileBodyHandler::AddContent(const char* data, std::size_t count) {
  ofstream_.write(data, count);
  streamed_size_ += count;
}

void FileBodyHandler::AddContent(const std::string& data) {
  ofstream_ << data;
  streamed_size_ += data.size();
}

bool FileBodyHandler::Finish() {
  ofstream_.close();

  // Create a file body based on the streamed temp file.
  auto body = std::make_shared<FileBody>(temp_path_, true);

  // TODO: Compress

  message_->SetBody(body, false);

  return true;
}

// -----------------------------------------------------------------------------

Parser::Parser() {
  Reset();
}

void Parser::Init(Message* message) {
  Reset();
  message_ = message;
}

bool Parser::Parse(const char* data, std::size_t length) {
  if (header_ended_) {
    return ParseContent(data, length);
  }

  // Append the new data to the pending data.
  pending_data_.append(data, length);

  if (!ParseHeaders()) {
    return false;
  }

  if (!header_ended_) {
    LOG_INFO("HTTP headers will continue in next read.");
    return true;
  }

  LOG_INFO("HTTP headers just ended.");

  if (!OnHeadersEnd()) {
    // Only request parser can reach here when no view matches the request.
    // Data streaming or not is also determined for request parser.
    return false;
  }

  CreateBodyHandler();

  if (!body_handler_) {
    // The only reason to reach here is that it was failed to generate the temp
    // file for streaming. Normally, it shouldn't happen.
    return false;
  }

  // The left data, if any, is still in the pending data.
  return ParseContent("", 0);
}

void Parser::Reset() {
  message_ = nullptr;
  body_handler_.reset();
  stream_ = false;

  pending_data_.clear();

  content_length_ = kInvalidLength;
  content_type_.Reset();
  start_line_parsed_ = false;
  content_length_parsed_ = false;
  header_ended_ = false;
  chunked_ = false;
  chunk_size_ = kInvalidLength;
  finished_ = false;
}

bool Parser::ParseHeaders() {
  std::size_t off = 0;

  while (true) {
    std::string line;
    if (!GetNextLine(off, &line, false)) {
      // Can't find a full header line, need more data from next read.
      break;
    }

    off = off + line.size() + 2;  // +2 for CRLF

    if (line.empty()) {
      header_ended_ = true;
      break;
    }

    if (!start_line_parsed_) {
      start_line_parsed_ = true;
      message_->set_start_line(line);

      if (!ParseStartLine(line)) {
        return false;
      }
    } else {
      if (!ParseHeaderLine(line)) {
        return false;
      }
    }
  }

  // Remove the data which has just been parsed.
  pending_data_.erase(0, off);

  return true;
}

void Parser::CreateBodyHandler() {
  if (stream_) {
    auto file_body_handler = new FileBodyHandler{ message_ };
    if (!file_body_handler->OpenFile()) {
      body_handler_.reset();
      delete file_body_handler;
    } else {
      body_handler_.reset(file_body_handler);
    }
  } else {
    body_handler_.reset(new StringBodyHandler{ message_ });
  }
}

bool Parser::GetNextLine(std::size_t off, std::string* line, bool erase) {
  std::size_t pos = pending_data_.find(kCRLF, off);

  if (pos == std::string::npos) {
    return false;
  }

  std::size_t count = pos - off;

  if (count > 0) {
    *line = pending_data_.substr(off, count);
  }  // else: empty line

  if (erase) {
    pending_data_.erase(off, count + 2);
  }

  return true;
}

bool Parser::ParseHeaderLine(const std::string& line) {
  Header header;
  if (!split_kv(header.first, header.second, line, ':')) {
    LOG_ERRO("Invalid header: %s", line.c_str());
    return false;
  }

  if (iequals(header.first, headers::kContentLength)) {
    content_length_parsed_ = true;

    std::size_t content_length = kInvalidLength;
    if (!to_size_t(header.second, 10, &content_length)) {
      LOG_ERRO("Invalid content length: %s.", header.second.c_str());
      return false;
    }

    LOG_INFO("Content length: %u.", content_length);
    content_length_ = content_length;

  } else if (iequals(header.first, headers::kContentType)) {
    content_type_.Parse(header.second);
    if (!content_type_.Valid()) {
      LOG_ERRO("Invalid content-type header: %s", header.second.c_str());
      return false;
    }
  } else if (iequals(header.first, headers::kTransferEncoding)) {
    if (header.second == "chunked") {
      // The content is chunked.
      chunked_ = true;
    }
  }

  message_->SetHeader(std::move(header));

  return true;
}

bool Parser::ParseContent(const char* data, std::size_t length) {
  if (chunked_) {
    return ParseChunkedContent(data, length);
  } else {
    return ParseFixedContent(data, length);
  }
}

bool Parser::ParseFixedContent(const char* data, std::size_t length) {
  if (!content_length_parsed_) {
    // No Content-Length, no content.
    Finish();
    return true;
  }

  if (content_length_ == kInvalidLength) {
    // Invalid content length (syntax error).
    return false;
  }

  if (!pending_data_.empty()) {
    // This is the data left after the headers are parsed.
    body_handler_->AddContent(pending_data_);
    pending_data_.clear();
  }

  // Don't have to firstly put the data to the pending data.
  body_handler_->AddContent(data, length);

  if (IsFixedContentFull()) {
    // All content has been read.
    Finish();
  }

  return true;
}

bool Parser::ParseChunkedContent(const char* data, std::size_t length) {
  pending_data_.append(data, length);

  while (true) {
    // Read chunk-size if necessary.
    if (chunk_size_ == kInvalidLength) {
      if (!ParseChunkSize()) {
        return false;
      }

      LOG_VERB("Chunk size: %u.", chunk_size_);
    }

    if (chunk_size_ == 0) {
      Finish();
      return true;
    }

    if (chunk_size_ + 2 <= pending_data_.size()) {  // +2 for CRLF
      body_handler_->AddContent(pending_data_.c_str(), chunk_size_);

      pending_data_.erase(0, chunk_size_ + 2);

      // Reset chunk-size (NOT to 0).
      chunk_size_ = kInvalidLength;

      // Continue (explicitly) to parse next chunk.
      continue;

    } else if (chunk_size_ > pending_data_.size()) {
      body_handler_->AddContent(pending_data_);

      chunk_size_ -= pending_data_.size();

      pending_data_.clear();

      // Wait for more data from next read.
      break;

    } else {
      // Wait for more data from next read.
      // if (chunk_size_ == pending_data_.size()) {
      //   <Also wait for CRLF from next read>
      // }
      break;
    }
  }

  return true;
}

bool Parser::ParseChunkSize() {
  LOG_VERB("Parse chunk size.");

  std::string line;
  if (!GetNextLine(0, &line, true)) {
    return true;
  }

  LOG_VERB("Chunk size line: [%s].", line.c_str());

  std::string hex_str;  // e.g., "cf0" (3312)

  std::size_t pos = line.find(' ');
  if (pos != std::string::npos) {
    hex_str = line.substr(0, pos);
  } else {
    hex_str = line;
  }

  if (!to_size_t(hex_str, 16, &chunk_size_)) {
    LOG_ERRO("Invalid chunk-size: %s.", hex_str.c_str());
    return false;
  }

  return true;
}

bool Parser::IsFixedContentFull() const {
  assert(content_length_ != kInvalidLength);
  return body_handler_->GetContentLength() >= content_length_;
}

bool Parser::Finish() {
  finished_ = true;

  // Could be `kInvalidLength` (chunked).
  // Could be `0` (empty body and `Content-Length : 0`).
  message_->set_content_length(content_length_);

  return body_handler_->Finish();
}

}  // namespace webcc
