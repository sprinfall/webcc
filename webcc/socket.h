#ifndef WEBCC_SOCKET_H_
#define WEBCC_SOCKET_H_

#include "webcc/socket_base.h"

namespace webcc {

class Socket : public SocketBase {
public:
  explicit Socket(boost::asio::io_context& io_context);

  void AsyncConnect(const std::string& host, const Endpoints& endpoints,
                    ConnectHandler&& handler) override;

  void AsyncWrite(const Payload& payload, WriteHandler&& handler) override;

  void AsyncReadSome(ReadHandler&& handler, std::vector<char>* buffer) override;

  bool Shutdown() override;

  bool Close() override;

private:
  boost::asio::ip::tcp::socket socket_;
};

}  // namespace webcc

#endif  // WEBCC_SOCKET_H_
