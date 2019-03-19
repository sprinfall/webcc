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

  // Determine SSL verify flag.
  bool ssl_verify = true;
  if (args.ssl_verify_) {
    ssl_verify = args.ssl_verify_.value();
  } else if (ssl_verify_) {
    ssl_verify = ssl_verify_.value();
  }

  bool reuse = false;
  const HttpClientPool::Key key{ request.url() };

  HttpClientPtr client = pool_.Get(key);
  if (!client) {
    client.reset(new HttpClient{ 0, ssl_verify });
    reuse = false;
  } else {
    // TODO: Apply args.ssl_verify even if reuse a client.
    reuse = false;
    LOG_VERB("Reuse an existing connection.");
  }

  if (!client->Request(request, args.buffer_size_, !reuse)) {
    throw Exception(client->error(), client->timed_out());
  }

  // Update pool.
  if (reuse) {
    if (client->closed()) {
      pool_.Remove(key);
    }
  } else {
    if (!client->closed()) {
      pool_.Add(key, client);
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

HttpResponsePtr HttpClientSession::Put(const std::string& url,
                                       std::string&& data, bool json,
                                       std::vector<std::string>&& headers,
                                       HttpRequestArgs&& args) {
  return Request(args.method(http::kPut)
                     .url(url)
                     .data(std::move(data))
                     .json(json)
                     .headers(std::move(headers)));
}

HttpResponsePtr HttpClientSession::Delete(const std::string& url,
                                          std::vector<std::string>&& headers,
                                          HttpRequestArgs&& args) {
  return Request(args.method(http::kDelete)
                 .url(url)
                 .headers(std::move(headers)));
}

void HttpClientSession::InitHeaders() {
  using namespace http::headers;

  headers_.Add(kUserAgent, http::UserAgent());

  // Content-Encoding Tokens:
  //   (https://en.wikipedia.org/wiki/HTTP_compression)
  // * compress 每 UNIX "compress" program method (historic; deprecated in most
  //              applications and replaced by gzip or deflate);
  // * deflate  每 compression based on the deflate algorithm, a combination of
  //              the LZ77 algorithm and Huffman coding, wrapped inside the
  //              zlib data format;
  // * gzip     每 GNU zip format. Uses the deflate algorithm for compression,
  //              but the data format and the checksum algorithm differ from
  //              the "deflate" content-encoding. This method is the most
  //              broadly supported as of March 2011.
  // * identity 每 No transformation is used. This is the default value for
  //              content coding.
  //
  // A note about "deflate":
  //   (https://www.zlib.net/zlib_faq.html#faq39)
  // "gzip" is the gzip format, and "deflate" is the zlib format. They should
  // probably have called the second one "zlib" instead to avoid confusion with
  // the raw deflate compressed data format.
  // Simply put, "deflate" is not recommended for HTTP 1.1 encoding.
  //
  headers_.Add(kAcceptEncoding, "gzip, deflate");

  headers_.Add(kAccept, "*/*");

  headers_.Add(kConnection, "Keep-Alive");
}

}  // namespace webcc
