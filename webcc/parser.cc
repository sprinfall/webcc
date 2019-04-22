#include "webcc/parser.h"

#include "boost/algorithm/string.hpp"

#include "webcc/logger.h"
#include "webcc/message.h"
#include "webcc/utility.h"

#if WEBCC_ENABLE_GZIP
#include "webcc/gzip.h"
#endif

namespace webcc {

// -----------------------------------------------------------------------------

namespace {

bool StringToSizeT(const std::string& str, int base, std::size_t* output) {
  try {
    *output = static_cast<std::size_t>(std::stoul(str, 0, base));
  } catch (const std::exception&) {
    return false;
  }
  return true;
}

}  // namespace

// -----------------------------------------------------------------------------

Parser::Parser(Message* message)
    : message_(message),
      content_length_(kInvalidLength),
      start_line_parsed_(false),
      content_length_parsed_(false),
      header_ended_(false),
      chunked_(false),
      chunk_size_(kInvalidLength),
      finished_(false) {
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
  } else {
    LOG_INFO("HTTP headers just ended.");
    // NOTE: The left data, if any, is still in the pending data.
    return ParseContent("", 0);
  }
}

void Parser::Reset() {
  pending_data_.clear();
  content_.clear();

  content_length_ = kInvalidLength;
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
  if (!Split2(line, ':', &header.first, &header.second)) {
    return false;
  }

  do {
    if (!chunked_ && !content_length_parsed_) {
      if (boost::iequals(header.first, headers::kContentLength)) {
        content_length_parsed_ = true;

        if (!StringToSizeT(header.second, 10, &content_length_)) {
          LOG_ERRO("Invalid content length: %s.", header.second.c_str());
          return false;
        }

        LOG_INFO("Content length: %u.", content_length_);

        // Reserve memory to avoid frequent reallocation when append.
        try {
          content_.reserve(content_length_);
        } catch (const std::exception& e) {
          LOG_ERRO("Failed to reserve content memory: %s.", e.what());
          return false;
        }

        break;
      }
    }

    // TODO: Replace `!chunked_` with <TransferEncodingParsed>.
    if (!chunked_ && !content_length_parsed_) {
      if (boost::iequals(header.first, headers::kTransferEncoding)) {
        if (header.second == "chunked") {
          // The content is chunked.
          chunked_ = true;
        }

        break;
      }
    }
  } while (false);

  // Parse Content-Type.
  if (boost::iequals(header.first, headers::kContentType)) {
    ContentType content_type(header.second);
    if (!content_type.Valid()) {
      LOG_ERRO("Invalid content-type header: %s", header.second.c_str());
      return false;
    }
    message_->SetContentType(content_type);
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
    AppendContent(pending_data_);
    pending_data_.clear();
  }

  // NOTE: Don't have to firstly put the data to the pending data.
  AppendContent(data, length);

  if (IsContentFull()) {
    // All content has been read.
    Finish();
  }

  return true;
}

bool Parser::ParseChunkedContent(const char* data, std::size_t length) {
  // Append the new data to the pending data.
  // NOTE: It's more difficult to avoid this than fixed-length content.
  pending_data_.append(data, length);

  LOG_VERB("Parse chunked content (pending data size: %u).",
           pending_data_.size());

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
      AppendContent(pending_data_.c_str(), chunk_size_);

      pending_data_.erase(0, chunk_size_ + 2);

      // Reset chunk-size (NOT to 0).
      chunk_size_ = kInvalidLength;

      // Continue (explicitly) to parse next chunk.
      continue;

    } else if (chunk_size_ > pending_data_.size()) {
      AppendContent(pending_data_);

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

  if (!StringToSizeT(hex_str, 16, &chunk_size_)) {
    LOG_ERRO("Invalid chunk-size: %s.", hex_str.c_str());
    return false;
  }

  return true;
}

bool Parser::Finish() {
  finished_ = true;

  if (content_.empty()) {
    return true;
  }

  if (!IsContentCompressed()) {
    message_->SetContent(std::move(content_), false);
    return true;
  }

#if WEBCC_ENABLE_GZIP

  LOG_INFO("Decompress the HTTP content...");

  std::string decompressed;
  if (!gzip::Decompress(content_, &decompressed)) {
    LOG_ERRO("Cannot decompress the HTTP content!");
    return false;
  }

  message_->SetContent(std::move(decompressed), false);

  return true;

#else

  LOG_WARN("Compressed HTTP content remains untouched.");

  message_->SetContent(std::move(content_), false);

  return true;

#endif  // WEBCC_ENABLE_GZIP
}

void Parser::AppendContent(const char* data, std::size_t count) {
  content_.append(data, count);
}

void Parser::AppendContent(const std::string& data) {
  content_.append(data);
}

bool Parser::IsContentFull() const {
  return content_length_ != kInvalidLength &&
         content_length_ <= content_.length();
}

bool Parser::IsContentCompressed() const {
  return message_->GetContentEncoding() != ContentEncoding::kUnknown;
}

}  // namespace webcc
