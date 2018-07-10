#include "webcc/http_response_parser.h"

#include "webcc/logger.h"
#include "webcc/http_response.h"

namespace webcc {

HttpResponseParser::HttpResponseParser(HttpResponse* response)
    : HttpParser(response), response_(response) {
}

bool HttpResponseParser::ParseStartLine(const std::string& line) {
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
    response_->set_status(std::stoi(status_str));
  } catch (const std::exception&) {
    LOG_ERRO("Invalid HTTP status: %s", status_str.c_str());
    return false;
  }

  if (response_->status() != HttpStatus::kOK) {
    return false;
  }

  return true;
}

}  // namespace webcc
