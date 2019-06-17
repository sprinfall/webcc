#include "webcc/socket.h"

#if WEBCC_ENABLE_SSL
#if (defined(_WIN32) || defined(_WIN64))

#include <windows.h>
#include <wincrypt.h>
#include <cryptuiapi.h>

#include "openssl/x509.h"

#endif  // defined(_WIN32) || defined(_WIN64)
#endif  // WEBCC_ENABLE_SSL

#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"
#include "boost/core/ignore_unused.hpp"

#include "webcc/logger.h"

namespace webcc {

// -----------------------------------------------------------------------------

Socket::Socket(boost::asio::io_context& io_context)
    : socket_(io_context) {
}

void Socket::Connect(const std::string& host, const Endpoints& endpoints,
                     boost::system::error_code* ec) {
  boost::ignore_unused(host);

  boost::asio::connect(socket_, endpoints, *ec);
}

void Socket::Write(const Request& request, boost::system::error_code* ec) {
  boost::asio::write(socket_, request.payload(), *ec);
}

void Socket::AsyncReadSome(ReadHandler&& handler, std::vector<char>* buffer) {
  socket_.async_read_some(boost::asio::buffer(*buffer), std::move(handler));
}

void Socket::Close(boost::system::error_code* ec) {
  socket_.close(*ec);
}

// -----------------------------------------------------------------------------

#if WEBCC_ENABLE_SSL

#if (defined(_WIN32) || defined(_WIN64))

// See: https://stackoverflow.com/a/40046425/6825348
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
    auto in = (const unsigned char **)&cert_context->pbCertEncoded;
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

namespace ssl = boost::asio::ssl;

SslSocket::SslSocket(boost::asio::io_context& io_context, bool ssl_verify)
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

void SslSocket::Connect(const std::string& host, const Endpoints& endpoints,
                        boost::system::error_code* ec) {
  boost::asio::connect(ssl_socket_.lowest_layer(), endpoints, *ec);

  if (*ec) {
    return;
  }

  Handshake(host, ec);
}

void SslSocket::Write(const Request& request, boost::system::error_code* ec) {
  boost::asio::write(ssl_socket_, request.payload(), *ec);
}

void SslSocket::AsyncReadSome(ReadHandler&& handler,
                              std::vector<char>* buffer) {
  ssl_socket_.async_read_some(boost::asio::buffer(*buffer), std::move(handler));
}

void SslSocket::Close(boost::system::error_code* ec) {
  ssl_socket_.lowest_layer().close(*ec);
}

void SslSocket::Handshake(const std::string& host,
                          boost::system::error_code* ec) {
  if (ssl_verify_) {
    ssl_socket_.set_verify_mode(ssl::verify_peer);
  } else {
    ssl_socket_.set_verify_mode(ssl::verify_none);
  }

  ssl_socket_.set_verify_callback(ssl::rfc2818_verification(host));

  // Use sync API directly since we don't need timeout control.
  ssl_socket_.handshake(ssl::stream_base::client, *ec);

  if (*ec) {
    LOG_ERRO("Handshake error (%s).", ec->message().c_str());
  }
}

#endif  // WEBCC_ENABLE_SSL

}  // namespace webcc
