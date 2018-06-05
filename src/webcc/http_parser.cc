#include "webcc/http_parser.h"

#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"

#include "webcc/http_message.h"

namespace webcc {

HttpParser::HttpParser(HttpMessage* message)
    : message_(message),
      content_length_(kInvalidLength),
      start_line_parsed_(false),
      content_length_parsed_(false),
      header_parsed_(false),
      finished_(false) {
}

bool HttpParser::Parse(const char* data, std::size_t len) {
  if (header_parsed_) {
    // Add the data to the content.
    AppendContent(data, len);

    if (IsContentFull()) {
      // All content has been read.
      Finish();
    }

    return true;
  }

  pending_data_.append(data, len);
  std::size_t off = 0;

  while (true) {
    std::size_t pos = pending_data_.find("\r\n", off);
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

    if (!content_length_parsed_) {
      // No Content-Length, no content. (TODO: Support chucked data)
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
      content_length_ = boost::lexical_cast<std::size_t>(value);
    } catch (boost::bad_lexical_cast&) {
    }
  }
}

void HttpParser::Finish() {
  // Move temp content to message.
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
