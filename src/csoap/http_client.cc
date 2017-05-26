#include "csoap/http_client.h"

#include "boost/bind.hpp"
#include "boost/algorithm/string.hpp"

#include "csoap/common.h"

namespace csoap {

////////////////////////////////////////////////////////////////////////////////

static const std::string kCRLF = "\r\n";

// NOTE:
// Each header field consists of a name followed by a colon (":") and the
// field value. Field names are case-insensitive.
// See http://stackoverflow.com/questions/5258977/are-http-headers-case-sensitive
static const std::string kFieldContentTypeName = "Content-Type";
static const std::string kFieldContentLengthName = "Content-Length";

////////////////////////////////////////////////////////////////////////////////

HttpRequest::HttpRequest(HttpVersion version)
  : version_(version)
  , keep_alive_(true)
  , content_length_(std::string::npos) {
}

void HttpRequest::ToString(std::string& req_string) const {
  // Start line

  req_string += "POST ";
  req_string += url_;
  req_string += " ";

  if (version_ == kHttpV10) {
    req_string += "HTTP/1.0";
  } else {
    req_string += "HTTP/1.1";
  }
  req_string += kCRLF;

  // Header fields

  if (!content_type_.empty()) {
    req_string += kFieldContentTypeName;
    req_string += ": ";
    req_string += content_type_;
    req_string += kCRLF;
  }

  req_string += kFieldContentLengthName;
  req_string += ": ";
  req_string += LexicalCast<std::string>(content_length_, "0");
  req_string += kCRLF;

  req_string += "SOAPAction: ";
  req_string += soap_action_;
  req_string += kCRLF;

  req_string += "Host: ";
  req_string += host_;
  if (!port_.empty()) {
    req_string += ":";
    req_string += port_;
  }
  req_string += kCRLF;

  if (keep_alive_) {
    req_string += "Connection: Keep-Alive";
    req_string += kCRLF;
  }

  req_string += kCRLF;
}

////////////////////////////////////////////////////////////////////////////////

HttpResponse::HttpResponse()
    : status_(0)
    , content_length_(0)
    , start_line_parsed_(false)
    , header_parsed_(false)
    , finished_(false) {
}

void HttpResponse::Parse(const char* data, size_t len) {
  if (header_parsed_) {
    // Add the data to the content.
    content_.append(data, len);

    if (content_.length() >= content_length_) {
      // All content has been read.
      finished_ = true;
      return;
    }

    return;
  }

  pending_data_.append(data, len);
  size_t off = 0;

  while (true) {
    size_t pos = pending_data_.find(kCRLF, off);
    if (pos == std::string::npos) {
      break;
    }

    if (pos == off) {  // End of headers.
      off = pos + 2;  // Skip "\r\n".
      header_parsed_ = true;
      break;
    }

    std::string line = pending_data_.substr(off, pos - off);

    if (!start_line_parsed_) {
      ParseStartLine(line);  // TODO: Error handling.
      start_line_parsed_ = true;
    } else {
      ParseHeaderField(line);
    }

    off = pos + 2;  // Skip "\r\n".
  }

  if (header_parsed_) {
    // Headers just ended.
    content_ += pending_data_.substr(off);

    if (content_.length() >= content_length_) {
      // All content has been read.
      finished_ = true;
    }
  } else {
    // Save the unparsed piece for next parsing.
    pending_data_ = pending_data_.substr(off);
  }
}

bool HttpResponse::ParseStartLine(const std::string& line) {
  std::vector<std::string> parts;
  boost::split(parts, line, boost::is_any_of(" "), boost::token_compress_on);

  if (parts.size() != 3) {
    return false;
  }

  try {
    status_ = boost::lexical_cast<int>(parts[1]);
  } catch (boost::bad_lexical_cast&) {
    return false;
  }

  reason_ = parts[2];

  return true;
}

bool HttpResponse::ParseHeaderField(const std::string& line) {
  size_t pos = line.find(':');
  if (pos == std::string::npos) {
    return false;
  }

  std::string name = line.substr(0, pos);

  ++pos;  // Skip ':'.
  while (line[pos] == ' ') {  // Skip spaces.
    ++pos;
  }

  std::string value = line.substr(pos);

  if (boost::iequals(name, kFieldContentTypeName)) {
    content_type_ = value;
  } else if (boost::iequals(name, kFieldContentLengthName)) {
    try {
      content_length_ = boost::lexical_cast<size_t>(value);
    } catch (boost::bad_lexical_cast&) {
      // TODO
    }
  } else {
    // Unsupported, ignore.
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

HttpClient::HttpClient() {
}

HttpClient::~HttpClient() {
}

bool HttpClient::SendRequest(const HttpRequest& request,
                             const std::string& body) {
  using boost::asio::ip::tcp;

  tcp::socket socket(io_service_);

  tcp::resolver resolver(io_service_);

  std::string port = request.port();
  if (port.empty()) {
    port = "80";
  }

  tcp::resolver::query query(request.host(), port);

  boost::system::error_code error;
  tcp::resolver::iterator it = resolver.resolve(query, error);

  if (error) {
    return false;
  }

  socket.connect(*it, error);

  if (error) {
    return false;
  }

  std::string request_str;
  request.ToString(request_str);

  try {
    boost::asio::write(socket, boost::asio::buffer(request_str));
    boost::asio::write(socket, boost::asio::buffer(body));
  } catch (boost::system::system_error&) {
    return false;
  }

  // Read and parse HTTP response.

  while (!response_.finished()) {
    try {
      size_t len = socket.read_some(boost::asio::buffer(bytes_));
      response_.Parse(bytes_.data(), len);

    } catch (boost::system::system_error&) {
      // Should be EOF, but ...
      break;
    }
  }

  return true;
}

}  // namespace csoap
