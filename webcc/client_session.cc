#include "webcc/client_session.h"

#include "webcc/base64.h"
#include "webcc/logger.h"
#include "webcc/url.h"
#include "webcc/utility.h"

namespace webcc {

void ClientSession::Auth(const std::string& type,
                         const std::string& credentials) {
  headers_.Set(headers::kAuthorization, type + " " + credentials);
}

void ClientSession::AuthBasic(const std::string& login,
                              const std::string& password) {
  auto credentials = Base64Encode(login + ":" + password);
  return Auth("Basic", credentials);
}

void ClientSession::AuthToken(const std::string& token) {
  return Auth("Token", token);
}

ResponsePtr ClientSession::Request(RequestPtr request) {
  assert(request);

  for (auto& h : headers_.data()) {
    if (!request->HasHeader(h.first)) {
      request->SetHeader(h.first, h.second);
    }
  }

  if (!request->body()->IsEmpty() &&
      !media_type_.empty() && !request->HasHeader(headers::kContentType)) {
    request->SetContentType(media_type_, charset_);
  }

  request->Prepare();

  return Send(request);
}

static void SetHeaders(const Strings& headers, RequestBuilder* builder) {
  assert(headers.size() % 2 == 0);

  for (std::size_t i = 1; i < headers.size(); i += 2) {
    builder->Header(headers[i - 1], headers[i]);
  }
}

ResponsePtr ClientSession::Get(const std::string& url,
                               const Strings& parameters,
                               const Strings& headers) {
  RequestBuilder builder;
  builder.Get(url);

  assert(parameters.size() % 2 == 0);
  for (std::size_t i = 1; i < parameters.size(); i += 2) {
    builder.Query(parameters[i - 1], parameters[i]);
  }

  SetHeaders(headers, &builder);

  return Request(builder());
}

ResponsePtr ClientSession::Head(const std::string& url,
                                const Strings& parameters,
                                const Strings& headers) {
  RequestBuilder builder;
  builder.Head(url);

  assert(parameters.size() % 2 == 0);
  for (std::size_t i = 1; i < parameters.size(); i += 2) {
    builder.Query(parameters[i - 1], parameters[i]);
  }

  SetHeaders(headers, &builder);

  return Request(builder());
}

ResponsePtr ClientSession::Post(const std::string& url, std::string&& data,
                                bool json, const Strings& headers) {
  RequestBuilder builder;
  builder.Post(url);

  SetHeaders(headers, &builder);

  builder.Body(std::move(data));

  if (json) {
    builder.Json();
  }

  return Request(builder());
}

ResponsePtr ClientSession::Put(const std::string& url, std::string&& data,
                               bool json, const Strings& headers) {
  RequestBuilder builder;
  builder.Put(url);

  SetHeaders(headers, &builder);

  builder.Body(std::move(data));

  if (json) {
    builder.Json();
  }

  return Request(builder());
}

ResponsePtr ClientSession::Delete(const std::string& url,
                                  const Strings& headers) {
  RequestBuilder builder;
  builder.Delete(url);

  SetHeaders(headers, &builder);

  return Request(builder());
}

ResponsePtr ClientSession::Patch(const std::string& url, std::string&& data,
                                 bool json, const Strings& headers) {
  RequestBuilder builder;
  builder.Patch(url);

  SetHeaders(headers, &builder);

  builder.Body(std::move(data));

  if (json) {
    builder.Json();
  }

  return Request(builder());
}

void ClientSession::InitHeaders() {
  using namespace headers;

  headers_.Set(kUserAgent, utility::UserAgent());

  // Content-Encoding Tokens:
  //   (https://en.wikipedia.org/wiki/HTTP_compression)
  //
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
  // "gzip" is the gzip format, and "deflate" is the zlib format. They should
  // probably have called the second one "zlib" instead to avoid confusion with
  // the raw deflate compressed data format.
  // Simply put, "deflate" is not recommended for HTTP 1.1 encoding.
  // (https://www.zlib.net/zlib_faq.html#faq39)

#if WEBCC_ENABLE_GZIP
  headers_.Set(kAcceptEncoding, "gzip, deflate");
#else
  headers_.Set(kAcceptEncoding, "identity");
#endif  // WEBCC_ENABLE_GZIP

  headers_.Set(kAccept, "*/*");

  headers_.Set(kConnection, "Keep-Alive");
}

ResponsePtr ClientSession::Send(RequestPtr request) {
  const ClientPool::Key key{ request->url() };

  // Reuse a pooled connection.
  bool reuse = false;

  ClientPtr client = pool_.Get(key);
  if (!client) {
    client.reset(new Client{});
    reuse = false;
  } else {
    LOG_VERB("Reuse an existing connection.");
    reuse = true;
  }

  client->set_ssl_verify(ssl_verify_);
  client->set_buffer_size(buffer_size_);
  client->set_timeout(timeout_);

  Error error = client->Request(request, !reuse);

  if (error) {
    if (reuse && error.code() == Error::kSocketWriteError) {
      LOG_WARN("Cannot send request with the reused connection. "
               "The server must have closed it, reconnect and try again.");
      error = client->Request(request, true);
    }
  }

  if (error) {
    // Remove the failed connection from pool.
    if (reuse) {
      pool_.Remove(key);
    }
    throw error;
  }

  // Update connection pool.

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

}  // namespace webcc
