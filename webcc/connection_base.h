#ifndef WEBCC_CONNECTION_BASE_H_
#define WEBCC_CONNECTION_BASE_H_

#include <memory>
#include <string>
#include <vector>

#include "boost/asio/ip/tcp.hpp"

#include "webcc/globals.h"
#include "webcc/queue.h"
#include "webcc/request.h"
#include "webcc/request_parser.h"
#include "webcc/response.h"

// Set 1 to enable the log for the study of server thread model.
// Need to use multiple workers and loops for Server::Run().
// Suggest to configure the log level to USER.
#define WEBCC_STUDY_SERVER_THREADING 0

namespace webcc {

using SocketType = boost::asio::basic_socket<boost::asio::ip::tcp,
                                             boost::asio::any_io_executor>;

class ConnectionBase;
class ConnectionPool;
class Server;

using ConnectionPtr = std::shared_ptr<ConnectionBase>;

class ConnectionBase : public std::enable_shared_from_this<ConnectionBase> {
public:
  ConnectionBase(boost::asio::io_context& io_context, ConnectionPool* pool,
                 Queue<ConnectionPtr>* queue, ViewMatcher&& view_matcher,
                 std::size_t buffer_size);

  ConnectionBase(const ConnectionBase&) = delete;
  ConnectionBase& operator=(const ConnectionBase&) = delete;

  virtual ~ConnectionBase() = default;

  // Get underlying socket.
  virtual SocketType& GetSocket() = 0;
 
  RequestPtr request() const {
    return request_;
  }

  virtual void Start() = 0;

  // Shutdown and close socket.
  virtual void Close();

  // Send a response to the client.
  // The Connection header will be set to "Close" if `no_keep_alive` is true no
  // matter whether the client asked for keep-alive or not.
  void SendResponse(ResponsePtr response, bool no_keep_alive = false);

  // Send a response with the given status and an empty body to the client.
  void SendResponse(int status, bool no_keep_alive = false);

protected:
  // Read/write handler
  using RWHandler = std::function<void(boost::system::error_code, std::size_t)>;

  virtual void AsyncWrite(const std::vector<boost::asio::const_buffer>& buffers,
                          RWHandler&& handler) = 0;

  virtual void AsyncReadSome(boost::asio::mutable_buffer buffer,
                             RWHandler&& handler) = 0;

  void PrepareRequest();

  void AsyncRead();
  void OnRead(boost::system::error_code ec, std::size_t length);

  void AsyncWrite();
  void OnWriteHeaders(boost::system::error_code ec, std::size_t length);

  void AsyncWriteBody();
  void OnWriteBody(boost::system::error_code ec, std::size_t length);

  void HandleWriteOK();
  void HandleWriteError(boost::system::error_code ec);

  // The connection pool.
  ConnectionPool* pool_;

  // The connection queue.
  Queue<ConnectionPtr>* queue_;

  // The functor for the request parser to match views after receive the headers
  // of a request.
  ViewMatcher view_matcher_;

  // The buffer for incoming data.
  std::vector<char> buffer_;

  // The incoming request.
  RequestPtr request_;

  // The parser for the incoming request.
  RequestParser request_parser_;

  // The response to be sent back to the client.
  ResponsePtr response_;
};

}  // namespace webcc

#endif  // WEBCC_CONNECTION_BASE_H_
