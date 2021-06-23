#ifndef WEBCC_SOCKET_BASE_H_
#define WEBCC_SOCKET_BASE_H_

#include "boost/asio/ip/tcp.hpp"

#include "webcc/globals.h"

namespace webcc {

class SocketBase {
public:
  using Endpoints = boost::asio::ip::tcp::resolver::results_type;

  using ConnectHandler = std::function<void(boost::system::error_code,
                                            boost::asio::ip::tcp::endpoint)>;

  using WriteHandler =
      std::function<void(boost::system::error_code, std::size_t)>;

  using ReadHandler =
      std::function<void(boost::system::error_code, std::size_t)>;

  SocketBase() = default;

  SocketBase(const SocketBase&) = delete;
  SocketBase& operator=(const SocketBase&) = delete;

  virtual ~SocketBase() = default;

  virtual void AsyncConnect(const std::string& host, const Endpoints& endpoints,
                            ConnectHandler&& handler) = 0;

  virtual void AsyncWrite(const Payload& payload, WriteHandler&& handler) = 0;

  virtual void AsyncReadSome(ReadHandler&& handler,
                             std::vector<char>* buffer) = 0;

  virtual bool Shutdown() = 0;

  virtual bool Close() = 0;
};

}  // namespace webcc

#endif  // WEBCC_SOCKET_BASE_H_
