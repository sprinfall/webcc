#ifndef WEBCC_SSL_CLIENT_H_
#define WEBCC_SSL_CLIENT_H_

#include "boost/asio/ssl/context.hpp"
#include "boost/asio/ssl/stream.hpp"

#include "webcc/blocking_client_base.h"

namespace webcc {

// SSL verification mode.
enum class SslVerify {
  kDefault,
  kHostName,  // use ssl::host_name_verification
};

class SslClient final : public BlockingClientBase {
public:
  SslClient(boost::asio::io_context& io_context,
            boost::asio::ssl::context& ssl_context, SslVerify ssl_verify)
      : BlockingClientBase(io_context, "443"),
        ssl_stream_(io_context, ssl_context),
        ssl_verify_(ssl_verify) {
  }

  ~SslClient() override = default;

  void Close() override;

protected:
  SocketType& GetSocket() override {
    return ssl_stream_.lowest_layer();
  }

  void OnConnected() override;

  void AsyncWrite(const std::vector<boost::asio::const_buffer>& buffers,
                  AsyncRWHandler&& handler) override;

  void AsyncReadSome(boost::asio::mutable_buffer buffer,
                     AsyncRWHandler&& handler) override;

private:
  void OnHandshake(boost::system::error_code ec);

  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_stream_;

  SslVerify ssl_verify_;
};

}  // namespace webcc

#endif  // WEBCC_SSL_CLIENT_H_
