#ifndef WEBCC_CLIENT_SESSION_H_
#define WEBCC_CLIENT_SESSION_H_

#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "boost/asio/io_context.hpp"

#include "webcc/client_pool.h"
#include "webcc/request_builder.h"
#include "webcc/response.h"

namespace webcc {

// Client session provides connection-pooling, configuration and more.
// If a client session is shared by multiple threads, the requests sent through
// it will be serialized by using a mutex.
class ClientSession {
public:
  explicit ClientSession(std::size_t buffer_size = 0);

  ~ClientSession();

  // Start Asio loop in a thread.
  // You don't have to call Start() manually because it's called by the
  // constructor.
  void Start();

  // Stop Asio loop.
  // You can call Start() to resume the loop.
  void Stop();

  void set_connect_timeout(int timeout) {
    if (timeout > 0) {
      connect_timeout_ = timeout;
    }
  }

  void set_read_timeout(int timeout) {
    if (timeout > 0) {
      read_timeout_ = timeout;
    }
  }

  void set_buffer_size(std::size_t buffer_size) {
    buffer_size_ = buffer_size;
  }

  void SetHeader(const std::string& key, const std::string& value) {
    headers_.Set(key, value);
  }

  // Set `Content-Type` header, e.g., ("application/json", "utf-8").
  // Only applied when:
  //   - the request to send has no `Content-Type` header, and
  //   - the request has a body.
  void SetContentType(const std::string& media_type,
                      const std::string& charset = "") {
    media_type_ = media_type;
    charset_ = charset;
  }

  // Set content types to accept.
  void Accept(const std::string& content_types);

#if WEBCC_ENABLE_GZIP

  // Accept Gzip compressed response data or not.
  void AcceptGzip(bool gzip = true);

#endif  // WEBCC_ENABLE_GZIP

  // Set authorization.
  void Auth(const std::string& type, const std::string& credentials);

  // Set Basic authorization.
  void AuthBasic(const std::string& login, const std::string& password);

  // Set Token authorization.
  void AuthToken(const std::string& token);

  // Send a request.
  // Please use RequestBuilder to build the request.
  // If |stream| is true, the response data will be written into a temp file,
  // the response body will be FileBody, and you can easily move the temp file
  // to another path with FileBody::Move(). So, |stream| is really useful for
  // downloading files (JPEG, etc.) or saving memory for huge data responses.
  ResponsePtr Send(RequestPtr request, bool stream = false,
                   ProgressCallback callback = {});

  // Cancel any in-progress connecting, writing or reading.
  // Return if any client object has been closed.
  // It could be used to exit the program as soon as possible.
  bool Cancel();

private:
  void InitHeaders();

  ResponsePtr DoSend(RequestPtr request, bool stream,
                     ProgressCallback callback);

private:
  boost::asio::io_context io_context_;

  // The thread to run Asio loop.
  std::unique_ptr<std::thread> io_thread_;

  using ExecutorType = boost::asio::io_context::executor_type;
  boost::asio::executor_work_guard<ExecutorType> work_guard_;

  // TODO
#if WEBCC_ENABLE_SSL
  boost::asio::ssl::context ssl_context_;
#endif

  // Is Asio loop running?
  bool started_ = false;

  // The media (or MIME) type of `Content-Type` header.
  // E.g., "application/json".
  std::string media_type_;

  // The charset of `Content-Type` header.
  // E.g., "utf-8".
  std::string charset_;

  // Additional headers for each request.
  Headers headers_;

  // Timeout (seconds) for connecting to server.
  int connect_timeout_ = 0;

  // Timeout (seconds) for reading response.
  int read_timeout_ = 0;

  // The size of the buffer for reading response.
  // 0 means default value will be used.
  std::size_t buffer_size_;

  // Persistent (keep-alive) client connections.
  ClientPool pool_;

  // Current requested client.
  ClientPtr client_;

  // The mutex to serialize the requests.
  std::mutex mutex_;
};

}  // namespace webcc

#endif  // WEBCC_CLIENT_SESSION_H_
