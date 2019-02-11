#include "webcc/http_parser.h"

#include "boost/algorithm/string.hpp"

#include "webcc/http_message.h"
#include "webcc/logger.h"

namespace webcc {

// -----------------------------------------------------------------------------

static bool StringToSizeT(const std::string& str, int base,
                          std::size_t* output) {
  try {
    *output = static_cast<std::size_t>(std::stoul(str, 0, base));
  } catch (const std::exception&) {
    return false;
  }
  return true;
}

// -----------------------------------------------------------------------------

HttpParser::HttpParser(HttpMessage* message)
    : message_(message),
      content_length_(kInvalidLength),
      start_line_parsed_(false),
      content_length_parsed_(false),
      header_ended_(false),
      chunked_(false),
      chunk_size_(kInvalidLength),
      finished_(false) {
}

bool HttpParser::Parse(const char* data, std::size_t length) {
  // Append the new data to the pending data.
  pending_data_.append(data, length);

  if (!header_ended_) {
    // If headers not ended yet, continue to parse headers.
    if (!ParseHeaders()) {
      return false;
    }

    if (header_ended_) {
      LOG_INFO("HTTP headers just ended.");
    }
  }

  // If headers still not ended, just return and wait for next read.
  if (!header_ended_) {
    LOG_INFO("HTTP headers will continue in next read.");
    return true;
  }

  // Now, parse the content.

  if (chunked_) {
    return ParseChunkedContent();
  } else {
    return ParseFixedContent();
  }
}

bool HttpParser::ParseHeaders() {
  std::size_t off = 0;

  while (true) {
    std::string line;
    if (!NextPendingLine(off, &line, false)) {
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
      message_->set_start_line(line + CRLF);
      if (!ParseStartLine(line)) {
        return false;
      }
    } else {
      ParseHeaderLine(line);
    }
  }

  // Remove the parsed data.
  pending_data_.erase(0, off);

  return true;
}

bool HttpParser::NextPendingLine(std::size_t off, std::string* line,
                                 bool remove) {
  std::size_t pos = pending_data_.find(CRLF, off);

  if (pos == std::string::npos) {
    return false;
  }

  std::size_t count = pos - off;

  if (pos > off) {
    *line = pending_data_.substr(off, count);
  }  // else: empty line

  if (remove) {
    pending_data_.erase(off, count + 2);
  }

  return true;
}

bool HttpParser::ParseHeaderLine(const std::string& line) {
  // NOTE: Can't split with ":" because date time also contains ":".
  std::size_t pos = line.find(':');
  if (pos == std::string::npos) {
    return false;
  }

  std::string name = line.substr(0, pos);
  boost::trim(name);

  std::string value = line.substr(pos + 1);
  boost::trim(value);

  do {
    if (!chunked_ && !content_length_parsed_) {
      if (boost::iequals(name, http::headers::kContentLength)) {
        content_length_parsed_ = true;

        if (!StringToSizeT(value, 10, &content_length_)) {
          LOG_ERRO("Invalid content length: %s.", value.c_str());
          return false;
        }

        LOG_INFO("Content length: %u.", content_length_);

        try {
          // Reserve memory to avoid frequent reallocation when append.
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
      if (boost::iequals(name, http::headers::kTransferEncoding)) {
        if (value == "chunked") {
          // The content is chunked.
          chunked_ = true;
        }

        break;
      }
    }
  } while (false);

  // Save the header to the result message.
  message_->SetHeader(std::move(name), std::move(value));

  return true;
}

bool HttpParser::ParseFixedContent() {
  if (!content_length_parsed_) {
    // No Content-Length, no content.
    Finish();
    return true;
  }

  if (content_length_ == kInvalidLength) {
    // Invalid content length (syntax error).
    // Normally, shouldn't be here.
    return false;
  }

  // TODO: Avoid copy using std::move.
  AppendContent(pending_data_);

  pending_data_.clear();

  if (IsContentFull()) {
    // All content has been read.
    Finish();
  }

  return true;
}

bool HttpParser::ParseChunkedContent() {
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

bool HttpParser::ParseChunkSize() {
  LOG_VERB("Parse chunk size.");

  std::size_t off = 0;
  std::string line;
  if (!NextPendingLine(off, &line, true)) {
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

void HttpParser::Finish() {
  if (!content_.empty()) {
    message_->SetContent(std::move(content_), /*set_length*/false);
  }
  finished_ = true;
}

void HttpParser::AppendContent(const char* data, std::size_t count) {
  content_.append(data, count);
}

void HttpParser::AppendContent(const std::string& data) {
  content_.append(data);
}

bool HttpParser::IsContentFull() const {
  return content_length_ != kInvalidLength &&
         content_length_ <= content_.length();
}

}  // namespace webcc
