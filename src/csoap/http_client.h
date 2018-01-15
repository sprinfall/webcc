#ifndef CSOAP_HTTP_CLIENT_H_
#define CSOAP_HTTP_CLIENT_H_

#include <array>
#include "boost/asio/io_context.hpp"
#include "csoap/common.h"

namespace csoap {

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
  std::array<char, BUF_SIZE> buffer_;
  int timeout_seconds_;
};

}  // namespace csoap

#endif  // CSOAP_HTTP_CLIENT_H_
