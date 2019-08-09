#include "webcc/parser.h"

#include "boost/algorithm/string.hpp"
#include "boost/filesystem/operations.hpp"

#include "webcc/logger.h"
#include "webcc/message.h"
#include "webcc/utility.h"

#if WEBCC_ENABLE_GZIP
#include "webcc/gzip.h"
#endif

namespace bfs = boost::filesystem;

namespace webcc {

// -----------------------------------------------------------------------------

ParseHandlerBase::ParseHandlerBase(Message* message)
    : message_(message), content_length_(kInvalidLength) {
}

void ParseHandlerBase::OnStartLine(const std::string& start_line) {
  message_->set_start_line(start_line);
}

void ParseHandlerBase::OnContentLength(std::size_t content_length) {
  content_length_ = content_length;
}

void ParseHandlerBase::OnHeader(Header&& header) {
  message_->SetHeader(std::move(header));
}

bool ParseHandlerBase::IsCompressed() const {
  return message_->GetContentEncoding() != ContentEncoding::kUnknown;
}

// -----------------------------------------------------------------------------

ParseHandler::ParseHandler(Message* message) : ParseHandlerBase(message) {
}

void ParseHandler::OnContentLength(std::size_t content_length) {
  ParseHandlerBase::OnContentLength(content_length);

  // Reserve memory to avoid frequent reallocation when append.
  try {
    content_.reserve(content_length_);
  } catch (const std::exception& e) {
    LOG_ERRO("Failed to reserve content memory: %s.", e.what());
  }
}

void ParseHandler::AddContent(const char* data, std::size_t count) {
  content_.append(data, count);
}

void ParseHandler::AddContent(const std::string& data) {
  content_.append(data);
}

bool ParseHandler::IsFixedContentFull() const {
  if (content_length_ == kInvalidLength) {
    // Shouldn't be here.
    // See Parser::ParseFixedContent().
    return false;
  }

  return content_.length() >= content_length_;
}

bool ParseHandler::Finish() {
  // Could be `kInvalidLength` (chunked).
  // Could be `0` (empty body and `Content-Length : 0`).
  message_->set_content_length(content_length_);

  if (content_.empty()) {
    // The call to message_->SetBody() is not necessary since message is
    // always initialized with an empty body.
    return true;
  }

  auto body = std::make_shared<StringBody>(std::move(content_), IsCompressed());

#if WEBCC_ENABLE_GZIP
  LOG_INFO("Decompress the HTTP content...");
  if (!body->Decompress()) {
    LOG_ERRO("Cannot decompress the HTTP content!");
    return false;
  }
#else
  LOG_WARN("Compressed HTTP content remains untouched.");
#endif  // WEBCC_ENABLE_GZIP

  message_->SetBody(body, false);
  return true;
}

// -----------------------------------------------------------------------------

StreamedParseHandler::StreamedParseHandler(Message* message)
    : ParseHandlerBase(message) {
}

bool StreamedParseHandler::Init() {
  try {
    temp_path_ = bfs::temp_directory_path() / bfs::unique_path();
    LOG_VERB("Generate a temp path for streaming: %s",
             temp_path_.string().c_str());
  } catch (const bfs::filesystem_error&) {
    LOG_ERRO("Failed to generate temp path: %s", temp_path_.string().c_str());
    return false;
  }

  ofstream_.open(temp_path_, std::ios::binary);

  if (ofstream_.fail()) {
    LOG_ERRO("Failed to open the temp file: %s", temp_path_.string().c_str());
    return false;
  }

  return true;
}

void StreamedParseHandler::AddContent(const char* data, std::size_t count) {
  ofstream_.write(data, count);
  streamed_size_ += count;
}

void StreamedParseHandler::AddContent(const std::string& data) {
  ofstream_ << data;
  streamed_size_ += data.size();
}

bool StreamedParseHandler::IsFixedContentFull() const {
  if (content_length_ == kInvalidLength) {
    // Shouldn't be here.
    // See Parser::ParseFixedContent().
    return false;
  }

  return streamed_size_ >= content_length_;
}

bool StreamedParseHandler::Finish() {
  // Could be `kInvalidLength` (chunked).
  // Could be `0` (empty body and `Content-Length : 0`).
  message_->set_content_length(content_length_);

  ofstream_.close();

  // Create a file body based on the streamed temp file.
  auto body = std::make_shared<FileBody>(temp_path_, true);

  // TODO: Compress

  message_->SetBody(body, false);
  return true;
}

// -----------------------------------------------------------------------------

Parser::Parser()
    : start_line_parsed_(false),
      content_length_parsed_(false),
      header_ended_(false),
      chunked_(false),
      chunk_size_(kInvalidLength),
      finished_(false) {
}

bool Parser::Init(Message* message, bool stream) {
  Reset();

  if (stream) {
    handler_.reset(new StreamedParseHandler{ message });
  } else {
    handler_.reset(new ParseHandler{ message });
  }

  if (!handler_->Init()) {
    // Failed to generate temp file for streaming.
    return false;
  }

  return true;
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

  // The left data, if any, is still in the pending data.
  return ParseContent("", 0);
}

void Parser::Reset() {
  pending_data_.clear();

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
      handler_->OnStartLine(line);

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
  if (!utility::SplitKV(line, ':', &header.first, &header.second)) {
    LOG_ERRO("Invalid header: %s", line.c_str());
    return false;
  }

  if (boost::iequals(header.first, headers::kContentLength)) {
    content_length_parsed_ = true;

    std::size_t content_length = kInvalidLength;
    if (!utility::ToSize(header.second, 10, &content_length)) {
      LOG_ERRO("Invalid content length: %s.", header.second.c_str());
      return false;
    }

    LOG_INFO("Content length: %u.", content_length);
    handler_->OnContentLength(content_length);

  } else if (boost::iequals(header.first, headers::kContentType)) {
    content_type_.Parse(header.second);
    if (!content_type_.Valid()) {
      LOG_ERRO("Invalid content-type header: %s", header.second.c_str());
      return false;
    }
  } else if (boost::iequals(header.first, headers::kTransferEncoding)) {
    if (header.second == "chunked") {
      // The content is chunked.
      chunked_ = true;
    }
  }

  handler_->OnHeader(std::move(header));
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

  if (handler_->content_length() == kInvalidLength) {
    // Invalid content length (syntax error).
    return false;
  }

  if (!pending_data_.empty()) {
    // This is the data left after the headers are parsed.
    handler_->AddContent(pending_data_);
    pending_data_.clear();
  }

  // Don't have to firstly put the data to the pending data.
  handler_->AddContent(data, length);

  if (handler_->IsFixedContentFull()) {
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
      handler_->AddContent(pending_data_.c_str(), chunk_size_);

      pending_data_.erase(0, chunk_size_ + 2);

      // Reset chunk-size (NOT to 0).
      chunk_size_ = kInvalidLength;

      // Continue (explicitly) to parse next chunk.
      continue;

    } else if (chunk_size_ > pending_data_.size()) {
      handler_->AddContent(pending_data_);

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

  if (!utility::ToSize(hex_str, 16, &chunk_size_)) {
    LOG_ERRO("Invalid chunk-size: %s.", hex_str.c_str());
    return false;
  }

  return true;
}

bool Parser::Finish() {
  finished_ = true;
  return handler_->Finish();
}

}  // namespace webcc
