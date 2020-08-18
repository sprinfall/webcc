#ifndef WEBCC_CLIENT_SESSION_H_
#define WEBCC_CLIENT_SESSION_H_

#include <string>
#include <vector>

#include "webcc/client_pool.h"
#include "webcc/request_builder.h"
#include "webcc/response.h"

namespace webcc {

// HTTP requests session providing connection-pooling, configuration and more.
// A session shouldn't be shared by multiple threads. Please create a new
// session for each thread instead.
class ClientSession {
public:
  explicit ClientSession(int timeout = 0, bool ssl_verify = true,
                         std::size_t buffer_size = 0);

  ~ClientSession() = default;

  void set_timeout(int timeout) {
    if (timeout > 0) {
      timeout_ = timeout;
    }
  }

  void set_ssl_verify(bool ssl_verify) {
    ssl_verify_ = ssl_verify;
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
  ResponsePtr Send(RequestPtr request, bool stream = false);

private:
  void InitHeaders();

  ResponsePtr DoSend(RequestPtr request, bool stream);

private:
  // The media (or MIME) type of `Content-Type` header.
  // E.g., "application/json".
  std::string media_type_;

  // The charset of `Content-Type` header.
  // E.g., "utf-8".
  std::string charset_;

  // Additional headers for each request.
  Headers headers_;

  // Timeout in seconds for receiving response.
  int timeout_;

  // Verify the certificate of the peer or not.
  bool ssl_verify_;

  // The size of the buffer for reading response.
  // 0 means default value will be used.
  std::size_t buffer_size_;

  // Pool for Keep-Alive client connections.
  ClientPool pool_;
};

}  // namespace webcc

#endif  // WEBCC_CLIENT_SESSION_H_
