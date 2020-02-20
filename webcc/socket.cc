#include "webcc/socket.h"

#if WEBCC_ENABLE_SSL
#if (defined(_WIN32) || defined(_WIN64))

#include <windows.h>
#include <wincrypt.h>
#include <cryptuiapi.h>

#include "openssl/x509.h"

#endif  // defined(_WIN32) || defined(_WIN64)
#endif  // WEBCC_ENABLE_SSL

#include "asio/connect.hpp"
#include "asio/read.hpp"
#include "asio/write.hpp"

#include "webcc/logger.h"

namespace webcc {

// -----------------------------------------------------------------------------

Socket::Socket(asio::io_context& io_context) : socket_(io_context) {
}

bool Socket::Connect(const std::string& /*host*/, const Endpoints& endpoints) {
  std::error_code ec;
  asio::connect(socket_, endpoints, ec);

  if (ec) {
    LOG_ERRO("Socket connect error (%s).", ec.message().c_str());
    return false;
  }

  return true;
}

bool Socket::Write(const Payload& payload, std::error_code* ec) {
  asio::write(socket_, payload, *ec);
  return !(*ec);
}

bool Socket::ReadSome(std::vector<char>* buffer, std::size_t* size,
                      std::error_code* ec) {
  *size = socket_.read_some(asio::buffer(*buffer), *ec);
  return (*size != 0 && !(*ec));
}

void Socket::AsyncReadSome(ReadHandler&& handler, std::vector<char>* buffer) {
  socket_.async_read_some(asio::buffer(*buffer), std::move(handler));
}

bool Socket::Close() {
  std::error_code ec;

  socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);

  if (ec) {
    LOG_WARN("Socket shutdown error (%s).", ec.message().c_str());
    ec.clear();
    // Don't return, try to close the socket anywhere.
  }

  socket_.close(ec);

  if (ec) {
    LOG_WARN("Socket close error (%s).", ec.message().c_str());
    return false;
  }

  return true;
}

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
  HCERTSTORE cert_store = ::CertOpenSystemStoreW(NULL, L"ROOT");
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

namespace ssl = asio::ssl;

SslSocket::SslSocket(asio::io_context& io_context, bool ssl_verify)
    : ssl_context_(ssl::context::sslv23),
      ssl_socket_(io_context, ssl_context_),
      ssl_verify_(ssl_verify) {
#if (defined(_WIN32) || defined(_WIN64))
  if (ssl_verify_) {
    UseSystemCertificateStore(ssl_context_.native_handle());
  }
#else
  // Use the default paths for finding CA certificates.
  ssl_context_.set_default_verify_paths();
#endif  // defined(_WIN32) || defined(_WIN64)
}

bool SslSocket::Connect(const std::string& host, const Endpoints& endpoints) {
  std::error_code ec;
  asio::connect(ssl_socket_.lowest_layer(), endpoints, ec);

  if (ec) {
    LOG_ERRO("Socket connect error (%s).", ec.message().c_str());
    return false;
  }

  return Handshake(host);
}

bool SslSocket::Write(const Payload& payload, std::error_code* ec) {
  asio::write(ssl_socket_, payload, *ec);
  return !(*ec);
}

bool SslSocket::ReadSome(std::vector<char>* buffer, std::size_t* size,
                         std::error_code* ec) {
  *size = ssl_socket_.read_some(asio::buffer(*buffer), *ec);
  return (*size != 0 && !(*ec));
}

void SslSocket::AsyncReadSome(ReadHandler&& handler,
                              std::vector<char>* buffer) {
  ssl_socket_.async_read_some(asio::buffer(*buffer), std::move(handler));
}

bool SslSocket::Close() {
  std::error_code ec;
  ssl_socket_.lowest_layer().close(ec);
  return !ec;
}

bool SslSocket::Handshake(const std::string& host) {
  if (ssl_verify_) {
    ssl_socket_.set_verify_mode(ssl::verify_peer);
  } else {
    ssl_socket_.set_verify_mode(ssl::verify_none);
  }

  ssl_socket_.set_verify_callback(ssl::rfc2818_verification(host));

  // Use sync API directly since we don't need timeout control.
  std::error_code ec;
  ssl_socket_.handshake(ssl::stream_base::client, ec);

  if (ec) {
    LOG_ERRO("Handshake error (%s).", ec.message().c_str());
    return false;
  }

  return true;
}

#endif  // WEBCC_ENABLE_SSL

}  // namespace webcc
