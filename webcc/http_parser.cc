#include "webcc/http_parser.h"

#include "boost/algorithm/string.hpp"

#include "webcc/http_message.h"
#include "webcc/logger.h"

namespace webcc {

HttpParser::HttpParser(HttpMessage* message)
    : message_(message),
      content_length_(kInvalidLength),
      start_line_parsed_(false),
      content_length_parsed_(false),
      header_parsed_(false),
      finished_(false) {
}

bool HttpParser::Parse(const char* data, std::size_t length) {
  if (header_parsed_) {
    // Add the data to the content.
    AppendContent(data, length);

    if (IsContentFull()) {
      // All content has been read.
      Finish();
    }

    return true;
  }

  pending_data_.append(data, length);
  std::size_t off = 0;

  while (true) {
    std::size_t pos = pending_data_.find(CRLF, off);
    if (pos == std::string::npos) {
      break;
    }

    if (pos == off) {   // End of headers.
      off = pos + 2;    // Skip CRLF.
      header_parsed_ = true;
      break;
    }

    std::string line = pending_data_.substr(off, pos - off);

    if (!start_line_parsed_) {
      start_line_parsed_ = true;
      message_->set_start_line(line + CRLF);
      if (!ParseStartLine(line)) {
        return false;
      }
    } else {
      // Currently, only Content-Length is important to us.
      // Other header fields are ignored.
      if (!content_length_parsed_) {
        ParseContentLength(line);
      }
    }

    off = pos + 2;  // Skip CRLF.
  }

  if (header_parsed_) {
    // Headers just ended.
    LOG_INFO("HTTP headers parsed.");

    if (!content_length_parsed_) {
      // No Content-Length, no content.
      Finish();
      return true;
    } else {
      // Invalid Content-Length in the request.
      if (content_length_ == kInvalidLength) {
        return false;
      }
    }

    AppendContent(pending_data_.substr(off));

    if (IsContentFull()) {
      // All content has been read.
      Finish();
    }
  } else {
    // Save the unparsed piece for next parsing.
    pending_data_ = pending_data_.substr(off);
  }

  return true;
}

void HttpParser::ParseContentLength(const std::string& line) {
  std::size_t pos = line.find(':');
  if (pos == std::string::npos) {
    return;
  }

  std::string name = line.substr(0, pos);

  if (boost::iequals(name, kContentLength)) {
    content_length_parsed_ = true;

    ++pos;  // Skip ':'.
    while (line[pos] == ' ') {  // Skip spaces.
      ++pos;
    }

    std::string value = line.substr(pos);

    try {
      content_length_ = static_cast<std::size_t>(std::stoul(value));
    } catch (const std::exception&) {
      LOG_ERRO("Invalid content length: %s.", value.c_str());
    }

    LOG_INFO("Content length: %u.", content_length_);

    try {
      // Reserve memory to avoid frequent reallocation when append.
      content_.reserve(content_length_);
    } catch (const std::exception& e) {
      LOG_ERRO("Failed to reserve content memory: %s.", e.what());
    }
  }
}

void HttpParser::Finish() {
  // Move content to message.
  message_->SetContent(std::move(content_));
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
