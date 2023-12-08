#ifndef WEBCC_SSL_CLIENT_H_
#define WEBCC_SSL_CLIENT_H_

#include "boost/asio/ssl/context.hpp"
#include "boost/asio/ssl/stream.hpp"

#include "webcc/client_base.h"

namespace webcc {

class SslClient final : public ClientBase {
public:
  SslClient(boost::asio::io_context& io_context,
            boost::asio::ssl::context& ssl_context, SslVerify ssl_verify);

  ~SslClient() override = default;

  std::shared_ptr<SslClient> shared_from_this() {
    return std::dynamic_pointer_cast<SslClient>(ClientBase::shared_from_this());
  }

  bool Close() override;

  void set_ssl_shutdown_timeout(int timeout) {
    if (timeout > 0) {
      ssl_shutdown_timeout_ = timeout;
    }
  }

protected:
  SocketType& GetSocket() override {
    return ssl_stream_.lowest_layer();
  }

  void AsyncWrite(const std::vector<boost::asio::const_buffer>& buffers,
                  AsyncRWHandler&& handler) override;

  void AsyncReadSome(boost::asio::mutable_buffer buffer,
                     AsyncRWHandler&& handler) override;

  void RequestBegin() override {
    ClientBase::RequestBegin();
    hand_shaken_ = false;
  }

  void OnConnected() override;

private:
  void OnHandshake(boost::system::error_code ec);

  void OnSslShutdownTimer(boost::system::error_code ec);

  void OnSslShutdown(boost::system::error_code ec);

  void StopSslShutdownTimer();

private:
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_stream_;

  // SSL verification mode.
  SslVerify ssl_verify_;

  // SSL handshake finished or not.
  std::atomic_bool hand_shaken_ = false;

  // Timeout (seconds) for SSL shutdown.
  int ssl_shutdown_timeout_ = 10;

  // Deadline timer for SSL shutdown.
  boost::asio::steady_timer ssl_shutdown_timer_;
  std::atomic_bool ssl_shutdown_timer_active_ = false;
};

}  // namespace webcc

#endif  // WEBCC_SSL_CLIENT_H_
