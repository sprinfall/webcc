#ifndef WEBCC_SSL_SERVER_H_
#define WEBCC_SSL_SERVER_H_

#include "boost/asio/ssl/context.hpp"

#include "webcc/server.h"

namespace webcc {

namespace ssl = boost::asio::ssl;

class SslServer : public Server {
public:
  SslServer(boost::asio::ip::tcp protocol, std::uint16_t port,
            const sfs::path& doc_root = {},
            ssl::context::method method = ssl::context::sslv23);

  ~SslServer() override = default;

  // Expose the SSL context for the user to configure it.
  // E.g.,
  //   ssl_context().set_options(ssl::context::default_workarounds |
  //                             ssl::context::no_sslv2 |
  //                             ssl::context::single_dh_use);
  //   ssl_context().set_password_callback(MyPasswordCallback);
  //   ssl_context().use_certificate_chain_file("server.pem");
  //   ssl_context().use_private_key_file("server.pem", ssl::context::pem);
  //   ssl_context().use_tmp_dh_file("dh4096.pem");
  //   ...
  boost::asio::ssl::context& ssl_context() {
    return ssl_context_;
  }

private:
  // Override to create a SSL connection.
  ConnectionPtr NewConnection() override;

  boost::asio::ssl::context ssl_context_;
};

}  // namespace webcc

#endif  // WEBCC_SSL_SERVER_H_
