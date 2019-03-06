#include "webcc/http_request.h"

#include "webcc/logger.h"

namespace webcc {

HttpRequest::HttpRequest(const std::string& method,
                         const std::string& url,
                         const std::vector<std::string>& parameters)
    : method_(method), url_(url) {
  assert(parameters.size() % 2 == 0);
  for (std::size_t i = 1; i < parameters.size(); i += 2) {
    url_.AddParameter(parameters[i - 1], parameters[i]);
  }
}

bool HttpRequest::Prepare() {
  if (url_.host().empty()) {
    LOG_ERRO("Invalid request: host is missing.");
    return false;
  }

  std::string target = "/" + url_.path();
  if (!url_.query().empty()) {
    target += "?";
    target += url_.query();
  }

  start_line_ = method_;
  start_line_ += " ";
  start_line_ += target;
  start_line_ += " HTTP/1.1";
  start_line_ += CRLF;

  if (url_.port().empty()) {
    SetHeader(http::headers::kHost, url_.host());
  } else {
    SetHeader(http::headers::kHost, url_.host() + ":" + url_.port());
  }

  return true;
}

HttpRequestPtr HttpRequest::New(const std::string& method,
                                const std::string& url,
                                const std::vector<std::string>& parameters,
                                bool prepare) {
  HttpRequestPtr request{ new HttpRequest{ method, url, parameters } };

  if (prepare) {
    request->Prepare();
  }

  return request;
}

}  // namespace webcc
