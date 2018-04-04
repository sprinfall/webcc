#include "webcc/http_parser.h"

#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"

#include "webcc/http_message.h"

namespace webcc {

HttpParser::HttpParser(HttpMessage* message)
    : message_(message)
    , start_line_parsed_(false)
    , content_length_parsed_(false)
    , header_parsed_(false)
    , finished_(false) {
}

Error HttpParser::Parse(const char* data, std::size_t len) {
  if (header_parsed_) {
    // Add the data to the content.
    message_->AppendContent(data, len);

    if (message_->IsContentFull()) {
      // All content has been read.
      finished_ = true;
    }

    return kNoError;
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
      Error error = ParseStartLine(line);
      if (error != kNoError) {
        return error;
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
      // No Content-Length, no content.
      message_->SetContentLength(0);
      finished_ = true;
      return kNoError;
    } else {
      // Invalid Content-Length in the request. 
      if (!message_->IsContentLengthValid()) {
        return kHttpContentLengthError;
      }
    }

    message_->AppendContent(pending_data_.substr(off));

    if (message_->IsContentFull()) {
      // All content has been read.
      finished_ = true;
    }
  } else {
    // Save the unparsed piece for next parsing.
    pending_data_ = pending_data_.substr(off);
  }

  return kNoError;
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
      std::size_t length = boost::lexical_cast<std::size_t>(value);
      message_->SetContentLength(length);
    } catch (boost::bad_lexical_cast&) {
    }
  }
}

}  // namespace webcc
