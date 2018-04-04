#include "csoap/http_request_parser.h"

#include <vector>
#include "boost/algorithm/string.hpp"

#include "csoap/http_request.h"

namespace csoap {

HttpRequestParser::HttpRequestParser(HttpRequest* request)
    : HttpParser(request), request_(request) {
}

Error HttpRequestParser::ParseStartLine(const std::string& line) {
  std::vector<std::string> strs;
  boost::split(strs, line, boost::is_any_of(" "));

  if (strs.size() != 3) {
    return kHttpStartLineError;
  }

  request_->set_method(strs[0]);
  request_->set_url(strs[1]);

  // HTTP version is currently ignored.

  return kNoError;
}

}  // namespace csoap
