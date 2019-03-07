#include "webcc/http_request_parser.h"

#include <vector>
#include "boost/algorithm/string.hpp"

#include "webcc/http_request.h"

namespace webcc {

HttpRequestParser::HttpRequestParser(HttpRequest* request)
    : HttpParser(request), request_(request) {
}

bool HttpRequestParser::ParseStartLine(const std::string& line) {
  std::vector<std::string> strs;
  boost::split(strs, line, boost::is_any_of(" "), boost::token_compress_on);

  if (strs.size() != 3) {
    return false;
  }

  request_->set_method(std::move(strs[0]));
  request_->set_url(std::move(strs[1]));

  // HTTP version is ignored.

  return true;
}

}  // namespace webcc
