#include "csoap/http_request_parser.h"

#include <vector>
#include "boost/algorithm/string.hpp"

#include "csoap/http_request.h"

namespace csoap {

HttpRequestParser::HttpRequestParser(HttpRequest* request)
  : HttpParser(request)
  , request_(request) {
}

Error HttpRequestParser::ParseStartLine(const std::string& line) {
  // Example: POST / HTTP/1.1

  std::vector<std::string> strs;
  boost::split(strs, line, boost::is_any_of(" "));

  if (strs.size() != 3) {
    return kHttpStartLineError;
  }

  if (strs[0] != "POST") {
    // Only POST method is supported.
    return kHttpStartLineError;
  }

  request_->SetURL(strs[1]);

  // TODO: strs[2];

  return kNoError;
}

}  // namespace csoap
