#include "webcc/http_response_parser.h"
#include "boost/lexical_cast.hpp"
#include "webcc/http_response.h"

namespace webcc {

HttpResponseParser::HttpResponseParser(HttpResponse* response)
    : HttpParser(response)
    , response_(response) {
}

Error HttpResponseParser::ParseStartLine(const std::string& line) {
  response_->set_start_line(line + "\r\n");

  size_t off = 0;

  size_t pos = line.find(' ');
  if (pos == std::string::npos) {
    return kHttpStartLineError;
  }

  // HTTP version

  off = pos + 1;  // Skip space.

  pos = line.find(' ', off);
  if (pos == std::string::npos) {
    return kHttpStartLineError;
  }

  // Status code
  std::string status_str = line.substr(off, pos - off);

  try {
    response_->set_status(boost::lexical_cast<int>(status_str));
  } catch (boost::bad_lexical_cast&) {
    return kHttpStartLineError;
  }

  off = pos + 1;  // Skip space.

  if (response_->status() != HttpStatus::kOK) {
    return kHttpStatusError;
  }

  return kNoError;
}

}  // namespace webcc
