#ifndef WEBCC_WS_CLIENT_H_
#define WEBCC_WS_CLIENT_H_

#include <condition_variable>
#include <mutex>

#include "webcc/async_client_base.h"
#include "webcc/ws_frame.h"
#include "webcc/ws_parser.h"

namespace webcc {

class WSClient;
using WSClientPtr = std::shared_ptr<WSClient>;

class WSClient : public AsyncClientBase {
public:
  using ConnectHandler = std::function<void(WSClientPtr, const Error&)>;
  using SendHandler = std::function<void(WSClientPtr, const Error&)>;
  using ReceiveHandler = std::function<void(WSClientPtr, WSFramePtr)>;

  ~WSClient() override = default;

  static WSClientPtr Make(boost::asio::io_context& io_context) {
    return std::shared_ptr<WSClient>{ new WSClient{ io_context } };
  }

  WSClientPtr shared_from_this() {
    return std::dynamic_pointer_cast<WSClient>(
        AsyncClientBase::shared_from_this());
  }

  void set_receive_handler(ReceiveHandler&& receive_handler) {
    receive_handler_ = std::move(receive_handler);
  }

  void Connect(const Url& url, ConnectHandler&& connect_handler);

  void Send(WSFramePtr frame);

  void SendPing();
  void SendPong();

  void SendClose(std::uint16_t code = 0);

protected:
  explicit WSClient(boost::asio::io_context& io_context);

  SocketType& GetSocket() override {
    return socket_;
  }
 
  void AsyncWrite(const std::vector<boost::asio::const_buffer>& buffers,
                  RWHandler&& handler) override;

  void AsyncReadSome(boost::asio::mutable_buffer buffer,
                     RWHandler&& handler) override;

  void RequestEnd() override;

private:
  void AsyncHandshake();

  void AsyncReadFrame();
  void OnReadFrame(boost::system::error_code ec, std::size_t size);

  void HandleFrameIn(WSFramePtr in_frame);

  void AsyncWriteFrame(WSFramePtr frame);
  void OnWriteFrame(boost::system::error_code ec, std::size_t size);

  boost::asio::ip::tcp::socket socket_;

  Url url_;

  ConnectHandler connect_handler_;
  SendHandler send_handler_;
  ReceiveHandler receive_handler_;

  WSFramePtr in_frame_;
  WSFramePtr out_frame_;

  // The buffer for reading incoming frame.
  std::vector<byte_t> buffer_;

  // The size of the buffer for reading data frame.
  // 0 means default value will be used.
  std::size_t buffer_size_ = kBufferSize;

  WSParser parser_;

  std::mutex out_mutex_;
  std::condition_variable out_idle_cv_;

  bool close_frame_sent_ = false;
  bool close_frame_received_ = false;
};

}  // namespace webcc

#endif  // WEBCC_WS_CLIENT_H_
