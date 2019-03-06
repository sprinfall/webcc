#include "webcc/http_response_parser.h"

#include "boost/algorithm/string.hpp"

#include "webcc/logger.h"
#include "webcc/http_response.h"

namespace webcc {

HttpResponseParser::HttpResponseParser(HttpResponse* response)
    : HttpParser(response), response_(response) {
}

bool HttpResponseParser::ParseStartLine(const std::string& line) {
  std::vector<std::string> parts;
  boost::split(parts, line, boost::is_any_of(" "), boost::token_compress_on);

  if (parts.size() < 3) {
    LOG_ERRO("Invalid HTTP response status line: %s", line.c_str());
    return false;
  }

  std::string& status_str = parts[1];

  try {
    response_->set_status(std::stoi(status_str));
  } catch (const std::exception&) {
    LOG_ERRO("Invalid HTTP status code: %s", status_str.c_str());
    return false;
  }

  return true;
}

}  // namespace webcc
