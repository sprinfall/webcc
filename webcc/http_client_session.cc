#include "webcc/http_client_session.h"

#include "webcc/http_client.h"
#include "webcc/http_request.h"
#include "webcc/url.h"

namespace webcc {

HttpClientSession::HttpClientSession() {
  InitHeaders();
}

HttpResponsePtr HttpClientSession::Request(HttpRequestArgs&& args) {
  assert(args.parameters_.size() % 2 == 0);
  assert(args.headers_.size() % 2 == 0);

  HttpRequest request(args.method_, args.url_, args.parameters_);

  if (!args.data_.empty()) {
    request.SetContent(std::move(args.data_), true);

    // TODO: charset/encoding
    if (args.json_) {
      request.SetContentType(http::media_types::kApplicationJson, "");
    }
  }

  // Apply the session-level headers.
  for (const HttpHeader& h : headers_.data()) {
    request.SetHeader(h.first, h.second);
  }

  // Apply the request-level headers.
  // This will overwrite the session-level headers.
  for (std::size_t i = 1; i < args.headers_.size(); i += 2) {
    request.SetHeader(std::move(args.headers_[i - 1]),
                      std::move(args.headers_[i]));
  }

  request.Prepare();

  HttpClient client;
  if (!client.Request(request, args.buffer_size_)) {
    return HttpResponsePtr{};
  }

  return client.response();
}

HttpResponsePtr HttpClientSession::Get(const std::string& url,
                                       std::vector<std::string>&& parameters,
                                       std::vector<std::string>&& headers,
                                       HttpRequestArgs&& args) {
  return Request(args.method(http::kGet).url(url)
                 .parameters(std::move(parameters))
                 .headers(std::move(headers)));
}

HttpResponsePtr HttpClientSession::Post(const std::string& url,
                                        std::string&& data, bool json,
                                        std::vector<std::string>&& headers,
                                        HttpRequestArgs&& args) {
  return Request(args.method(http::kPost).url(url).data(std::move(data))
                 .json(json).headers(std::move(headers)));
}

void HttpClientSession::InitHeaders() {
  // NOTE: C++11 requires a space between literal and string macro.
  headers_.Add(http::headers::kUserAgent, USER_AGENT);

  // TODO: Support gzip, deflate
  headers_.Add(http::headers::kAcceptEncoding, "identity");

  headers_.Add(http::headers::kAccept, "*/*");

  // TODO: Support Keep-Alive connection.
  //headers_.Add(http::headers::kConnection, "close");
}

}  // namespace webcc
