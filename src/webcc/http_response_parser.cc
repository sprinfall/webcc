#include "webcc/http_response_parser.h"

#include "boost/lexical_cast.hpp"

#include "webcc/logger.h"
#include "webcc/http_response.h"

namespace webcc {

HttpResponseParser::HttpResponseParser(HttpResponse* response)
    : HttpParser(response), response_(response) {
}

bool HttpResponseParser::ParseStartLine(const std::string& line) {
  response_->set_start_line(line + "\r\n");

  std::size_t off = 0;

  std::size_t pos = line.find(' ');
  if (pos == std::string::npos) {
    return false;
  }

  // HTTP version

  off = pos + 1;  // Skip space.

  pos = line.find(' ', off);
  if (pos == std::string::npos) {
    return false;
  }

  // Status code
  std::string status_str = line.substr(off, pos - off);

  try {
    response_->set_status(boost::lexical_cast<int>(status_str));
  } catch (boost::bad_lexical_cast&) {
    LOG_ERRO("Invalid HTTP status: %s", status_str.c_str());
    return false;
  }

  if (response_->status() != HttpStatus::kOK) {
    return false;
  }

  return true;
}

}  // namespace webcc
