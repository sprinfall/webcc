#ifndef WEBCC_SSL_CONNECTION_H_
#define WEBCC_SSL_CONNECTION_H_

#include "boost/asio/ssl/context.hpp"
#include "boost/asio/ssl/stream.hpp"

#include "webcc/connection_base.h"

namespace webcc {

class SslConnection : public ConnectionBase {
public:
  SslConnection(boost::asio::io_context& io_context,
                boost::asio::ssl::context& ssl_context, ConnectionPool* pool,
                Queue<ConnectionPtr>* queue, ViewMatcher&& view_matcher,
                std::size_t buffer_size)
      : ConnectionBase(io_context, pool, queue, std::move(view_matcher),
                       buffer_size),
        ssl_stream_(io_context, ssl_context) {
  }

  ~SslConnection() override = default;

  SocketType& GetSocket() override {
    return ssl_stream_.lowest_layer();
  }

  // Override to firstly handshake before read the client request.
  void Start() override;

  // Override to firstly shutdown SSL.
  void Close() override;

protected:
  void AsyncWrite(const std::vector<boost::asio::const_buffer>& buffers,
                  RWHandler&& handler) override;

  void AsyncReadSome(boost::asio::mutable_buffer buffer,
                     RWHandler&& handler) override;

private:
  void OnHandshake(boost::system::error_code ec);

  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_stream_;
};

}  // namespace webcc

#endif  // WEBCC_SSL_CONNECTION_H_
