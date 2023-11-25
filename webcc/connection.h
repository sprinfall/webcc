#ifndef WEBCC_CONNECTION_H_
#define WEBCC_CONNECTION_H_

#include "webcc/connection_base.h"

namespace webcc {

class Connection : public ConnectionBase {
public:
  Connection(boost::asio::io_context& io_context, ConnectionPool* pool,
             Queue<ConnectionPtr>* queue, ViewMatcher&& view_matcher,
             std::size_t buffer_size)
      : ConnectionBase(io_context, pool, queue, std::move(view_matcher),
                       buffer_size),
        socket_(io_context) {
  }

  ~Connection() override = default;

  SocketType& GetSocket() override {
    return socket_;
  }
 
  // Start to read and process the client request.
  void Start() override {
    PrepareRequest();
    AsyncRead();
  }

protected:
  void AsyncWrite(const std::vector<boost::asio::const_buffer>& buffers,
                  AsyncRWHandler&& handler) override;

  void AsyncReadSome(boost::asio::mutable_buffer buffer,
                     AsyncRWHandler&& handler) override;

private:
  // The socket for the connection.
  boost::asio::ip::tcp::socket socket_;
};

}  // namespace webcc

#endif  // WEBCC_CONNECTION_H_
