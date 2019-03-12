#include "webcc/http_client_session.h"

#include "webcc/url.h"

namespace webcc {

HttpClientSession::HttpClientSession() {
  InitHeaders();
}

HttpResponsePtr HttpClientSession::Request(HttpRequestArgs&& args) {
  assert(args.parameters_.size() % 2 == 0);
  assert(args.headers_.size() % 2 == 0);

  HttpRequest request{ args.method_, args.url_ };

  for (std::size_t i = 1; i < args.parameters_.size(); i += 2) {
    request.AddParameter(args.parameters_[i - 1], args.parameters_[i]);
  }

  if (!args.data_.empty()) {
    request.SetContent(std::move(args.data_), true);

    // TODO: Request-level charset.
    if (args.json_) {
      request.SetContentType(http::media_types::kApplicationJson, charset_);
    } else if (!content_type_.empty()) {
      request.SetContentType(content_type_, charset_);
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

  const HttpClientPool::Key key{ request.url() };
  bool new_created = false;

  HttpClientPtr client = pool_.Get(key);
  if (!client) {
    new_created = true;
    client.reset(new HttpClient{ 0, args.ssl_verify_ });
  } else {
    new_created = false;
    LOG_VERB("Reuse an existing connection.");
  }

  if (!client->Request(request, args.buffer_size_, new_created)) {
    throw Exception(client->error(), client->timed_out());
  }

  if (new_created) {
    if (!client->closed()) {
      pool_.Add(key, client);

      LOG_VERB("Added connection to the pool (%s, %s, %s).",
               key.scheme.c_str(), key.host.c_str(), key.port.c_str());
    }
  } else {
    if (client->closed()) {
      pool_.Remove(key);

      LOG_VERB("Removed connection from the pool (%s, %s, %s).",
               key.scheme.c_str(), key.host.c_str(), key.port.c_str());
    }
  }

  return client->response();
}

HttpResponsePtr HttpClientSession::Get(const std::string& url,
                                       std::vector<std::string>&& parameters,
                                       std::vector<std::string>&& headers,
                                       HttpRequestArgs&& args) {
  return Request(args.method(http::kGet)
                     .url(url)
                     .parameters(std::move(parameters))
                     .headers(std::move(headers)));
}

HttpResponsePtr HttpClientSession::Post(const std::string& url,
                                        std::string&& data, bool json,
                                        std::vector<std::string>&& headers,
                                        HttpRequestArgs&& args) {
  return Request(args.method(http::kPost)
                     .url(url)
                     .data(std::move(data))
                     .json(json)
                     .headers(std::move(headers)));
}

void HttpClientSession::InitHeaders() {
  headers_.Add(http::headers::kUserAgent, http::UserAgent());

  // TODO: Support gzip, deflate
  headers_.Add(http::headers::kAcceptEncoding, "identity");

  headers_.Add(http::headers::kAccept, "*/*");

  headers_.Add(http::headers::kConnection, "Keep-Alive");
}

}  // namespace webcc
