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

#if WEBCC_ENABLE_SSL
namespace ssl = boost::asio::ssl;
#endif

namespace webcc {

#if WEBCC_ENABLE_SSL

// -----------------------------------------------------------------------------

#if (defined(_WIN32) || defined(_WIN64))

// Let OpenSSL on Windows use the system certificate store.
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
// See: https://stackoverflow.com/a/11763389/6825348

static bool UseSystemCertificateStore(SSL_CTX* ssl_ctx) {
  LOG_INFO("Use Windows certificate store");

  // NOTE: Cannot use nullptr to replace NULL.
  // Enum Windows store with "ROOT" (not "CA").
  HCERTSTORE cert_store = CertOpenSystemStoreW(NULL, L"ROOT");
  if (cert_store == nullptr) {
    LOG_ERRO("Cannot open Windows certificate store");
    return false;
  }

  X509_STORE* x509_store = SSL_CTX_get_cert_store(ssl_ctx);
  PCCERT_CONTEXT cert_context = nullptr;
  int count = 0;

  while (cert_context = CertEnumCertificatesInStore(cert_store, cert_context)) {
    auto in = (const unsigned char**)&cert_context->pbCertEncoded;
    X509* x509 = d2i_X509(nullptr, in, cert_context->cbCertEncoded);

    if (x509 != nullptr) {
      ++count;

#if 0
      // Log the issuer for debug purpose.
      CERT_NAME_BLOB issuer_blob = cert_context->pCertInfo->Issuer;
      std::string issuer((const char*)issuer_blob.pbData, issuer_blob.cbData);
      LOG_INFO("Certificate issuer: %s", issuer.c_str());
#endif

      if (X509_STORE_add_cert(x509_store, x509) == 0) {
        LOG_ERRO("Add certificate error!");
      }

      X509_free(x509);
    }
  }

  LOG_INFO("Certificate added: %d", count);

  CertFreeCertificateContext(cert_context);
  CertCloseStore(cert_store, 0);
  return true;
}

#endif  // defined(_WIN32) || defined(_WIN64)

// -----------------------------------------------------------------------------

// NOTE:
// Because SSL context needs to be shared among SSL sessions, the ssl::context
// class is fully, internally thread safe. You can use an ssl::context object
// in multiple SSL connections and from multiple threads however you want.
// See: https://stackoverflow.com/a/33519766/6825348

class SslContextManager {
public:
  static SslContextManager* Instance() {
    static SslContextManager s_instance;
    return &s_instance;
  }

  void AddContext(const std::string& key, SslContextPtr ssl_context) {
    assert(ssl_context != nullptr);
    std::lock_guard<std::mutex> lock{ mutex_ };
    ssl_context_map_[key] = ssl_context;
  }

  bool AddContext(const std::string& key, const std::string& cert_file) {
    std::lock_guard<std::mutex> lock{ mutex_ };

    auto ssl_context =
        std::make_shared<ssl::context>(ssl::context::sslv23_client);

    boost::system::error_code ec;
    ssl_context->load_verify_file(cert_file, ec);

    if (ec) {
      return false;
    }

    ssl_context_map_[key] = ssl_context;

    return true;
  }

  // Add a certificate to the SSL context with the given key.
  bool AddCertificate(const std::string& key,
                      boost::asio::const_buffer cert_buffer) {
    std::lock_guard<std::mutex> lock{ mutex_ };

    SslContextPtr ssl_context;

    auto iter = ssl_context_map_.find(key);
    if (iter == ssl_context_map_.end()) {
      ssl_context = std::make_shared<ssl::context>(ssl::context::sslv23_client);
      ssl_context_map_[key] = ssl_context;
    } else {
      ssl_context = iter->second;
    }

    boost::system::error_code ec;
    ssl_context->add_certificate_authority(cert_buffer, ec);
    // -> X509_STORE_add_cert()

    if (ec) {
      return false;
    }

    return true;
  }

  SslContextPtr Get(const std::string& key) {
    std::lock_guard<std::mutex> lock{ mutex_ };

    auto iter = ssl_context_map_.find(key);
    if (iter != ssl_context_map_.end()) {
      return iter->second;
    }

    return GetDefault();
  }

protected:
  SslContextManager() = default;

  SslContextPtr GetDefault() {
    if (default_ssl_context_ != nullptr) {
      return default_ssl_context_;
    }

    default_ssl_context_.reset(new ssl::context{ ssl::context::sslv23_client });

#if (defined(_WIN32) || defined(_WIN64))
    UseSystemCertificateStore(default_ssl_context_->native_handle());
#else
    default_ssl_context_->set_default_verify_paths();
#endif

    return default_ssl_context_;
  }

private:
  std::map<std::string, SslContextPtr> ssl_context_map_;
  SslContextPtr default_ssl_context_;
  std::mutex mutex_;
};

#define SSL_CONTEXT_MANAGER SslContextManager::Instance()

#endif  // WEBCC_ENABLE_SSL

// -----------------------------------------------------------------------------
// static functions

#if WEBCC_ENABLE_SSL

bool ClientSession::AddCertificate(const std::string& ssl_context_key,
                                   boost::asio::const_buffer cert_buffer) {
  return SSL_CONTEXT_MANAGER->AddCertificate(ssl_context_key, cert_buffer);
}

bool ClientSession::AddSslContext(const std::string& key,
                                  const std::string& cert_file) {
  return SSL_CONTEXT_MANAGER->AddContext(key, cert_file);
}

void ClientSession::AddSslContext(const std::string& key,
                                  SslContextPtr ssl_context) {
  return SSL_CONTEXT_MANAGER->AddContext(key, ssl_context);
}

#endif  // WEBCC_ENABLE_SSL

// -----------------------------------------------------------------------------

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

void ClientSession::Accept(std::string_view content_types) {
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

ResponsePtr ClientSession::Send(RequestPtr request, bool stream,
                                ProgressCallback callback) {
  assert(request);

  std::lock_guard<std::mutex> lock{ mutex_ };

  if (!started_) {
    throw Error{ Error::kStateError, "Loop is not running" };
  }

  for (auto& h : headers_.data()) {
    if (!request->HeaderExist(h.first)) {
      request->SetHeader(h.first, h.second);
    }
  }

  if (!request->body()->IsEmpty() && !media_type_.empty() &&
      !request->HeaderExist(headers::kContentType)) {
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
    auto ssl_context = SSL_CONTEXT_MANAGER->Get(ssl_context_key_);
    return std::make_shared<SslClient>(io_context_, *ssl_context);
  }
#endif  // WEBCC_ENABLE_SSL

  return {};
}

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
