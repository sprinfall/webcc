#include "webcc/client_session.h"

#include <cassert>

#if WEBCC_ENABLE_SSL
#if (defined(_WIN32) || defined(_WIN64))

#include <cryptuiapi.h>
#include <wincrypt.h>
#include <windows.h>

#include "openssl/x509.h"

#endif  // defined(_WIN32) || defined(_WIN64)
#endif  // WEBCC_ENABLE_SSL

#include "boost/algorithm/string.hpp"

#include "webcc/base64.h"
#include "webcc/logger.h"
#include "webcc/url.h"
#include "webcc/utility.h"
 
#if WEBCC_ENABLE_SSL
#include "webcc/ssl_client.h"
#endif

namespace webcc {

// -----------------------------------------------------------------------------

#if WEBCC_ENABLE_SSL
#if (defined(_WIN32) || defined(_WIN64))

// Let OpenSSL on Windows use the system certificate store
//   1. Load your certificate (in PCCERT_CONTEXT structure) from Windows Cert
//      store using Crypto APIs.
//   2. Get encrypted content of it in binary format as it is.
//      [PCCERT_CONTEXT->pbCertEncoded].
//   3. Parse this binary buffer into X509 certificate Object using OpenSSL's
//      d2i_X509() method.
//   4. Get handle to OpenSSL's trust store using SSL_CTX_get_cert_store()
//      method.
//   5. Load above parsed X509 certificate into this trust store using
//      X509_STORE_add_cert() method.
//   6. You are done!
// NOTES: Enum Windows store with "ROOT" (not "CA").
// See: https://stackoverflow.com/a/11763389/6825348

static bool UseSystemCertificateStore(SSL_CTX* ssl_ctx) {
  // NOTE: Cannot use nullptr to replace NULL.
  HCERTSTORE cert_store = CertOpenSystemStoreW(NULL, L"ROOT");
  if (cert_store == nullptr) {
    LOG_ERRO("Cannot open Windows system certificate store.");
    return false;
  }

  X509_STORE* x509_store = SSL_CTX_get_cert_store(ssl_ctx);
  PCCERT_CONTEXT cert_context = nullptr;

  while (cert_context = CertEnumCertificatesInStore(cert_store, cert_context)) {
    auto in = (const unsigned char**)&cert_context->pbCertEncoded;
    X509* x509 = d2i_X509(nullptr, in, cert_context->cbCertEncoded);

    if (x509 != nullptr) {
      if (X509_STORE_add_cert(x509_store, x509) == 0) {
        LOG_ERRO("Cannot add Windows root certificate.");
      }

      X509_free(x509);
    }
  }

  CertFreeCertificateContext(cert_context);
  CertCloseStore(cert_store, 0);
  return true;
}

#endif  // defined(_WIN32) || defined(_WIN64)
#endif  // WEBCC_ENABLE_SSL

// -----------------------------------------------------------------------------

ClientSession::ClientSession(std::size_t buffer_size)
    : buffer_size_(buffer_size) {
  InitHeaders();

  Start();
}

ClientSession::~ClientSession() {
  Stop();

#if WEBCC_ENABLE_SSL
  if (ssl_context_ != nullptr) {
    delete ssl_context_;
  }
#endif  // WEBCC_ENABLE_SSL
}

void ClientSession::Start() {
  if (started_) {
    return;
  }

  started_ = true;

  work_guard_.reset(new WorkGuard{ io_context_.get_executor() });

  io_context_.restart();

  // Run the io context off in its own thread so that it operates completely
  // asynchronously with respect to the rest of the program.

  io_thread_.reset(new std::thread{ [this]() { io_context_.run(); }});

  LOG_INFO("Loop is now running");
}

void ClientSession::Stop() {
  if (!started_) {
    return;
  }

  Cancel();

  // NOTE(20220221): Don't call `io_context_.stop()` instead!
  work_guard_.reset();

  io_thread_->join();

  LOG_INFO("Loop stopped");

  started_ = false;
}

void ClientSession::Accept(string_view content_types) {
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

void ClientSession::Auth(string_view type, string_view credentials) {
  headers_.Set(headers::kAuthorization,
               ToString(type) + " " + ToString(credentials));
}

void ClientSession::AuthBasic(string_view login, string_view password) {
  auto credentials =
      Base64Encode(ToString(login) + ":" + ToString(password));
  return Auth("Basic", credentials);
}

void ClientSession::AuthToken(string_view token) {
  return Auth("Token", token);
}

ResponsePtr ClientSession::Send(RequestPtr request, bool stream,
                                ProgressCallback callback) {
  assert(request);

  std::lock_guard<std::mutex> lock{ mutex_ };

  if (!started_) {
    throw Error{ Error::kStateError, "Loop is not running" };
  }

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

  return DoSend(request, stream, callback);
}

bool ClientSession::Cancel() {
  if (client_) {
    client_->Close();
    return true;
  }
  return false;
}

void ClientSession::InitHeaders() {
  headers_.Set(headers::kUserAgent, utility::UserAgent());

  headers_.Set(headers::kAccept, "*/*");

  // Accept-Encoding is always default to "identity", even if GZIP is enabled.
  // Please overwrite with AcceptGzip().
  headers_.Set(headers::kAcceptEncoding, "identity");

  headers_.Set(headers::kConnection, "Keep-Alive");
}

ClientPtr ClientSession::CreateClient(const std::string& url_scheme) {
  if (boost::iequals(url_scheme, "http")) {
    return std::make_shared<Client>(io_context_);
  }

#if WEBCC_ENABLE_SSL
  if (boost::iequals(url_scheme, "https")) {
    CreateSslContext();  // If it's not created yet
    return std::make_shared<SslClient>(io_context_, *ssl_context_);
  }
#endif  // WEBCC_ENABLE_SSL

  return {};
}

#if WEBCC_ENABLE_SSL

void ClientSession::CreateSslContext() {
  if (ssl_context_ != nullptr) {
    return;
  }

  namespace ssl = boost::asio::ssl;

  ssl_context_ = new ssl::context{ ssl::context::sslv23_client };

#if (defined(_WIN32) || defined(_WIN64))
    UseSystemCertificateStore(ssl_context_->native_handle());
#else
    // Use the default paths for finding CA certificates.
    ssl_context_->set_default_verify_paths();
#endif
}

#endif  // WEBCC_ENABLE_SSL

ResponsePtr ClientSession::DoSend(RequestPtr request, bool stream,
                                  ProgressCallback callback) {
  const ClientPool::Key key{ request->url() };

  // Reuse a pooled connection.
  bool reuse = false;

  ClientPtr client = pool_.Get(key);
  
  if (!client) {
    client = CreateClient(request->url().scheme());
    if (!client) {
      throw Error{ Error::kSyntaxError, "Invalid URL scheme" };
    }
    reuse = false;
  } else {
    LOG_VERB("Reuse an existing connection");
    reuse = true;
  }

  client->set_buffer_size(buffer_size_);
  client->set_connect_timeout(connect_timeout_);
  client->set_read_timeout(read_timeout_);

  client->set_progress_callback(callback);

  // Save current client for cancel.
  client_ = client;

  Error error = client->Request(request, stream);

  client_.reset();

  if (error) {
    // Remove the failed connection from pool.
    if (reuse) {
      pool_.Remove(key);
    }
    throw error;
  }

  // Update connection pool.

  if (reuse) {
    if (!client->connected()) {
      pool_.Remove(key);
    }
  } else {
    if (client->connected()) {
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
