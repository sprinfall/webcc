#include "webcc/client_session.h"

#include "webcc/base64.h"
#include "webcc/logger.h"
#include "webcc/url.h"
#include "webcc/utility.h"

namespace webcc {

ClientSession::ClientSession(int timeout, bool ssl_verify,
                             std::size_t buffer_size)
    : timeout_(timeout), ssl_verify_(ssl_verify), buffer_size_(buffer_size) {
  InitHeaders();
}

void ClientSession::Accept(const std::string& content_types) {
  if (!content_types.empty()) {
    headers_.Set(headers::kAccept, content_types);
  }
}

#if WEBCC_ENABLE_GZIP

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

void ClientSession::AcceptGzip(bool gzip) {
  if (gzip) {
    headers_.Set(headers::kAcceptEncoding, "gzip, deflate");
  } else {
    headers_.Set(headers::kAcceptEncoding, "identity");
  }
}

#endif  // WEBCC_ENABLE_GZIP

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

ResponsePtr ClientSession::Send(RequestPtr request, bool stream) {
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

  return DoSend(request, stream);
}

void ClientSession::InitHeaders() {
  headers_.Set(headers::kUserAgent, utility::UserAgent());

  headers_.Set(headers::kAccept, "*/*");

  // Accept-Encoding is always default to "identity", even if GZIP is enabled.
  // Please overwrite with AcceptGzip().
  headers_.Set(headers::kAcceptEncoding, "identity");

  headers_.Set(headers::kConnection, "Keep-Alive");
}

ResponsePtr ClientSession::DoSend(RequestPtr request, bool stream) {
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

  Error error = client->Request(request, !reuse, stream);

  if (error) {
    if (reuse && error.code() == Error::kSocketWriteError) {
      LOG_WARN("Cannot send request with the reused connection. "
               "The server must have closed it, reconnect and try again.");
      error = client->Request(request, true, stream);
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

  auto response = client->response();

  // The client object might be cached in the pool.
  // Reset to make sure it won't keep a reference to the response object.
  client->Reset();

  return response;
}

}  // namespace webcc
