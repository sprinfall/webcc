#ifndef CSOAP_HTTP_REQUEST_H_
#define CSOAP_HTTP_REQUEST_H_

#include <string>
#include "boost/asio/buffer.hpp"  // for const_buffer
#include "csoap/http_message.h"

namespace csoap {

class HttpRequest;

std::ostream& operator<<(std::ostream& os, const HttpRequest& request);

// HTTP request.
// NOTE:
// - Only POST method is supported.
//   See https://stackoverflow.com/a/26339467
//
class HttpRequest : public HttpMessage {
  friend std::ostream& operator<<(std::ostream& os,
                                  const HttpRequest& request);

public:
  const std::string& host() const {
    return host_;
  }

  const std::string& port() const {
    return port_;
  }

  // Set the URL for the HTTP request start line.
  // Either a complete URL or the path component it is acceptable.
  // E.g., both of the following URLs are OK:
  //   - http://ws1.parasoft.com/glue/calculator
  //   - /glue/calculator
  void SetURL(const std::string& url);

  // \param host Descriptive host name or numeric IP address.
  // \param port Numeric port number, "80" will be used if it's empty.
  void SetHost(const std::string& host, const std::string& port);

  // Convert the response into a vector of buffers. The buffers do not own the
  // underlying memory blocks, therefore the response object must remain valid
  // and not be changed until the write operation has completed.
  std::vector<boost::asio::const_buffer> ToBuffers() const;

private:
  // Request URL.
  // A complete URL naming the requested resource, or the path component of
  // the URL.
  std::string url_;

  std::string host_;
  std::string port_;
};

}  // namespace csoap

#endif  // CSOAP_HTTP_REQUEST_H_
