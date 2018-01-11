#ifndef CSOAP_HTTP_REQUEST_H_
#define CSOAP_HTTP_REQUEST_H_

#include <string>
#include <vector>

#include "csoap/common.h"
#include "csoap/http_message.h"

namespace csoap {

////////////////////////////////////////////////////////////////////////////////

class HttpRequest;

std::ostream& operator<<(std::ostream& os, const HttpRequest& request);

////////////////////////////////////////////////////////////////////////////////

// HTTP request.
// NOTE:
// - Only POST method is supported.
//   See https://stackoverflow.com/a/26339467
//
class HttpRequest : public HttpMessage {
  friend std::ostream& operator<<(std::ostream& os,
                                  const HttpRequest& request);

public:
  HttpRequest() {
  }

  // Set the URL for the HTTP request start line.
  // Either a complete URL or the path component it is acceptable.
  // E.g., both of the following URLs are OK:
  //   - http://ws1.parasoft.com/glue/calculator
  //   - /glue/calculator
  void set_url(const std::string& url) {
    url_ = url;
  }

  const std::string& host() const {
    return host_;
  }

  const std::string& port() const {
    return port_;
  }

  // \param host Descriptive host name or numeric IP address.
  // \param port Numeric port number, "80" will be used if it's empty.
  void set_host(const std::string& host, const std::string& port) {
    host_ = host;
    port_ = port;
  }

  // SOAP specific.
  void set_soap_action(const std::string& soap_action) {
    soap_action_ = soap_action;
  }

  std::string GetHeaders() const;

private:
  // Request URL.
  // A complete URL naming the requested resource, or the path component of
  // the URL.
  std::string url_;

  std::string host_;
  std::string port_;

  std::string soap_action_;
};

}  // namespace csoap

#endif  // CSOAP_HTTP_REQUEST_H_
