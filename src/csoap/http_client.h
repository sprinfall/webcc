#ifndef CSOAP_HTTP_CLIENT_H_
#define CSOAP_HTTP_CLIENT_H_

#include <string>
#include "boost/asio.hpp"
#include "csoap/common.h"

namespace csoap {

////////////////////////////////////////////////////////////////////////////////

enum HttpVersion {
  kHttpV10,
  kHttpV11,
};

enum HttpStatus {
  kHttpOK = 200,
  kHttpNotFound = 404,
};

enum HeaderField {
  kHeaderContentType,
  kHeaderContentLength,
  kHeaderHost,
};

////////////////////////////////////////////////////////////////////////////////

// HTTP request.
// NOTE:
// - Only POST method is supported.
//   See https://stackoverflow.com/a/26339467
class HttpRequest {
public:
  HttpRequest(HttpVersion version);

  // Set the URL for the HTTP request start line.
  // Either a complete URL or the path component it is acceptable.
  // E.g., both of the following URLs are OK:
  //   - http://ws1.parasoft.com/glue/calculator
  //   - /glue/calculator
  void set_url(const std::string& url) {
    url_ = url;
  }

  // Default: "text/xml; charset=utf-8"
  void set_content_type(const std::string& content_type) {
    content_type_ = content_type;
  }

  void set_content_length(size_t content_length) {
    content_length_ = content_length;
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

  void ToString(std::string& req_string) const;

private:
  HttpVersion version_;

  // Request URL.
  // A complete URL naming the requested resource, or the path component of
  // the URL.
  std::string url_;

  std::string content_type_;
  size_t content_length_;

  std::string host_;
  std::string port_;

  std::string soap_action_;
};

////////////////////////////////////////////////////////////////////////////////

class HttpResponse {
public:
  HttpResponse();

  ErrorCode Parse(const char* data, size_t len);

  bool finished() const {
    return finished_;
  }

  int status() const {
    return status_;
  }

  const std::string& reason() const {
    return reason_;
  }

  const std::string& content() const {
    return content_;
  };

private:
  // Parse start line, e.g., "HTTP/1.1 200 OK".
  ErrorCode ParseStartLine(const std::string& line);

  void ParseContentLength(const std::string& line);

private:
  int status_;  // HTTP status, e.g., 200.
  std::string reason_;
  size_t content_length_;
  std::string content_;

  ErrorCode error_;

  // Data waiting to be parsed.
  std::string pending_data_;

  // Parsing helper flags.
  bool start_line_parsed_;
  bool header_parsed_;
  bool finished_;
};

////////////////////////////////////////////////////////////////////////////////

class HttpClient {
public:
  HttpClient();

  ErrorCode SendRequest(const HttpRequest& request,
                        const std::string& body,
                        HttpResponse* response);

private:
  boost::asio::io_service io_service_;
  std::array<char, 1024> bytes_;
};

}  // namespace csoap

#endif  // CSOAP_HTTP_CLIENT_H_
