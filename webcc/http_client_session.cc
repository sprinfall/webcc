#include "webcc/http_client_session.h"

#include "webcc/logger.h"
#include "webcc/url.h"

namespace webcc {

namespace {

// -----------------------------------------------------------------------------
// Helper functions.

//bool GetSslVerify(const boost::optional<bool>& session_ssl_verify,
//                  const boost::optional<bool>& request_ssl_verify) {
//  if (request_ssl_verify) {
//    return request_ssl_verify.value();
//  } else if (session_ssl_verify) {
//    return session_ssl_verify.value();
//  }
//  return true;
//}

}  // namespace

// -----------------------------------------------------------------------------

HttpClientSession::HttpClientSession() {
  InitHeaders();
}

HttpResponsePtr HttpClientSession::Request(HttpRequestPtr request) {
  assert(request);

  for (const auto& h : headers_.data()) {
    if (!request->HaveHeader(h.first)) {
      request->SetHeader(h.first, h.second);
    }
  }

  if (!content_type_.empty() &&
      !request->HaveHeader(http::headers::kContentType)) {
    request->SetContentType(content_type_, charset_);
  }

  request->Prepare();

  return Send(request);
}

void HttpClientSession::InitHeaders() {
  using namespace http::headers;

  headers_.Set(kUserAgent, http::UserAgent());

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
  headers_.Set(kAcceptEncoding, "gzip, deflate");

  headers_.Set(kAccept, "*/*");

  headers_.Set(kConnection, "Keep-Alive");
}

HttpResponsePtr HttpClientSession::Send(HttpRequestPtr request) {
  if (request->buffer_size() == 0) {
    request->set_buffer_size(buffer_size_);
  }

  const HttpClientPool::Key key{request->url()};

  // Reuse a connection or not.
  bool reuse = false;

  HttpClientPtr client = pool_.Get(key);
  if (!client) {
    client.reset(new HttpClient{ssl_verify_});
    reuse = false;
  } else {
    client->set_ssl_verify(ssl_verify_);
    reuse = true;
    LOG_VERB("Reuse an existing connection.");
  }

  client->set_timeout(timeout_);

  bool ok = client->Request(request, !reuse);

  if (!ok) {
    if (reuse && client->error() == kSocketWriteError) {
      LOG_WARN("Cannot send request with the reused connection. "
               "The server must have closed it, reconnect and try again.");
      ok = client->Request(request, true);
    }
  }

  if (!ok) {
    if (reuse) {
      // Remove the failed connection from pool.
      pool_.Remove(key);
    }
    throw Exception(client->error(), client->timed_out());
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
