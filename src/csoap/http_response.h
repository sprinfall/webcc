#ifndef CSOAP_HTTP_RESPONSE_H_
#define CSOAP_HTTP_RESPONSE_H_

#include <string>
#include "boost/asio/buffer.hpp"  // for const_buffer
#include "csoap/http_message.h"

namespace csoap {

class HttpResponse;

std::ostream& operator<<(std::ostream& os, const HttpResponse& response);

class HttpResponse : public HttpMessage {
  friend std::ostream& operator<<(std::ostream& os,
                                  const HttpResponse& response);

public:
  HttpResponse() {
  }

  int status() const {
    return status_;
  }
  void set_status(int status) {
    status_ = status;
  }

  // Convert the response into a vector of buffers. The buffers do not own the
  // underlying memory blocks, therefore the response object must remain valid
  // and not be changed until the write operation has completed.
  std::vector<boost::asio::const_buffer> ToBuffers() const;

  // Get a fault response when HTTP status is not OK.
  static HttpResponse Fault(HttpStatus status);

private:
  int status_ = HttpStatus::OK;  // TODO: HttpStatus
};

}  // namespace csoap

#endif  // CSOAP_HTTP_RESPONSE_H_
