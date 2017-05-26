#ifndef CSOAP_HTTP_CLIENT_H_
#define CSOAP_HTTP_CLIENT_H_

#include <string>
#include "boost/asio.hpp"

// A little concept about URL (From HTTP The Definitive Guide):
// Say you want to fetch the URL http://www.joes-hardware.com/seasonal/index-fall.html:
//   - The first part of the URL(http) is the URL scheme. The scheme tells a web client
//     *how* to access the resource. In this case, the URL says to use the HTTP protocol.
//   - The second part of the URL (www.joes-hardware.com) is the server location.
//     This tells the web client *where* the resource is hosted.
//   - The third part of the URL(/seasonal/index-fall.html) is the resource path. The
//     path tells *what* particular local resource on the server is being requested.

namespace csoap {

////////////////////////////////////////////////////////////////////////////////

enum HttpVersion {
  kHttpV10,
  kHttpV11,
};

//enum HttpStatus {
//  kHttpOK = 200,
//};

enum HeaderField {
  kHeaderContentType,
  kHeaderContentLength,
  kHeaderHost,
};

////////////////////////////////////////////////////////////////////////////////

// HTTP request.
// NOTE:
// - Only POST method is supported.
//   See http://stackoverflow.com/questions/26339317/do-soap-web-services-support-only-post-http-method
class HttpRequest {
public:
  HttpRequest(HttpVersion version);

  void set_uri(const std::string& uri) {
    url_ = uri;
  }

  void set_content_type(const std::string& content_type) {
    content_type_ = content_type;
  }

  void set_content_length(size_t content_length) {
    content_length_ = content_length;
  }

  void set_keep_alive(bool keep_alive) {
    keep_alive_ = keep_alive;
  }

  const std::string& host() const {
    return host_;
  }

  const std::string& port() const {
    return port_;
  }

  // \param host Descriptive host name or numeric IP address.
  // \param port Numeric port number.
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
  // A complete URL naming the requested resource, or the path component of the URL.
  std::string url_;

  std::string content_type_;
  size_t content_length_;

  std::string host_;
  std::string port_;

  bool keep_alive_;

  std::string soap_action_;
};

////////////////////////////////////////////////////////////////////////////////

class HttpResponse {
public:
  HttpResponse();

  void Parse(const char* data, size_t len);

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
  bool ParseStartLine(const std::string& line);

  // Parse a header line, e.g., "Content-Length: 19".
  bool ParseHeaderField(const std::string& line);

private:
  int status_;  // HTTP status, e.g., 200.
  std::string reason_;
  std::string content_type_;
  size_t content_length_;
  std::string content_;

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
  ~HttpClient();

  bool SendRequest(const HttpRequest& request,
                   const std::string& body);

  const HttpResponse& response() const {
    return response_;
  }

private:
  boost::asio::io_service io_service_;
  std::array<char, 1024> bytes_;

  HttpResponse response_;
};

}  // namespace csoap

#endif  // CSOAP_HTTP_CLIENT_H_
