#ifndef WEBCC_HTTP_CLIENT_H_
#define WEBCC_HTTP_CLIENT_H_

#include <array>

#include "boost/asio/io_context.hpp"

#include "webcc/common.h"

namespace webcc {

class HttpRequest;
class HttpResponse;

class HttpClient {
public:
  HttpClient();

  // Set socket send & recv timeout.
  void set_timeout_seconds(int seconds) {
    timeout_seconds_ = seconds;
  }

  // Send an HTTP request, wait until the response is received.
  Error SendRequest(const HttpRequest& request,
                    HttpResponse* response);

private:
  boost::asio::io_context io_context_;
  std::array<char, kBufferSize> buffer_;
  int timeout_seconds_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_CLIENT_H_
