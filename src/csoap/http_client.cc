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
// See https://stackoverflow.com/a/5259004
static const std::string kFieldContentTypeName = "Content-Type";
static const std::string kFieldContentLengthName = "Content-Length";

static const size_t kInvalidContentLength = std::string::npos;

////////////////////////////////////////////////////////////////////////////////

// NOTE (About Connection: keep-alive):
// Keep-alive is deprecated and no longer documented in the current HTTP/1.1
// specification.
// See https://stackoverflow.com/a/43451440

HttpRequest::HttpRequest(HttpVersion version)
    : version_(version)
    , content_length_(0) {
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

  req_string += kFieldContentTypeName;
  req_string += ": ";

  if (!content_type_.empty()) {
    req_string += content_type_;
  } else {
    req_string += "text/xml; charset=utf-8";
  }

  req_string += kCRLF;

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

  req_string += kCRLF;  // End of Headers.
}

////////////////////////////////////////////////////////////////////////////////

HttpResponse::HttpResponse()
    : status_(0)
    , content_length_(kInvalidContentLength)
    , start_line_parsed_(false)
    , header_parsed_(false)
    , finished_(false) {
}

ErrorCode HttpResponse::Parse(const char* data, size_t len) {
  if (header_parsed_) {
    // Add the data to the content.
    content_.append(data, len);

    if (content_.length() >= content_length_) {
      // All content has been read.
      finished_ = true;
    }

    return kNoError;
  }

  pending_data_.append(data, len);
  size_t off = 0;

  while (true) {
    size_t pos = pending_data_.find(kCRLF, off);
    if (pos == std::string::npos) {
      break;
    }

    if (pos == off) {  // End of headers.
      off = pos + 2;  // Skip CRLF.
      header_parsed_ = true;
      break;
    }

    std::string line = pending_data_.substr(off, pos - off);

    if (!start_line_parsed_) {
      start_line_parsed_ = true;
      ErrorCode error = ParseStartLine(line);
      if (error != kNoError) {
        return error;
      }
    } else {
      // Currently, only Content-Length is important to us.
      // Other fields are ignored.
      if (content_length_ == kInvalidContentLength) {  // Not parsed yet.
        ParseContentLength(line);
      }
    }

    off = pos + 2;  // Skip CRLF.
  }

  if (header_parsed_) {
    // Headers just ended.

    if (content_length_ == kInvalidContentLength) {
      // No Content-Length?
      return kHttpContentLengthError;
    }

    content_ += pending_data_.substr(off);

    if (content_.length() >= content_length_) {
      // All content has been read.
      finished_ = true;
    }
  } else {
    // Save the unparsed piece for next parsing.
    pending_data_ = pending_data_.substr(off);
  }

  return kNoError;
}

ErrorCode HttpResponse::ParseStartLine(const std::string& line) {
  size_t off = 0;

  size_t pos = line.find(' ');
  if (pos == std::string::npos) {
    return kHttpStartLineError;
  }

  // HTTP version

  off = pos + 1;  // Skip space.

  pos = line.find(' ', off);
  if (pos == std::string::npos) {
    return kHttpStartLineError;
  }

  // Status code
  std::string status_str = line.substr(off, pos - off);

  try {
    status_ = boost::lexical_cast<int>(status_str);
  } catch (boost::bad_lexical_cast&) {
    return kHttpStartLineError;
  }

  off = pos + 1;  // Skip space.
  reason_ = line.substr(off);

  if (status_ != kHttpOK) {
    return kHttpStatusError;
  }

  return kNoError;
}

void HttpResponse::ParseContentLength(const std::string& line) {
  size_t pos = line.find(':');
  if (pos == std::string::npos) {
    return;
  }

  std::string name = line.substr(0, pos);

  if (boost::iequals(name, kFieldContentLengthName)) {
    ++pos;  // Skip ':'.
    while (line[pos] == ' ') {  // Skip spaces.
      ++pos;
    }

    std::string value = line.substr(pos);

    try {
      content_length_ = boost::lexical_cast<size_t>(value);
    } catch (boost::bad_lexical_cast&) {
      // TODO
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

HttpClient::HttpClient() {
}

ErrorCode HttpClient::SendRequest(const HttpRequest& request,
                                  const std::string& body,
                                  HttpResponse* response) {
  assert(response != NULL);

  using boost::asio::ip::tcp;

  tcp::socket socket(io_service_);

  tcp::resolver resolver(io_service_);

  std::string port = request.port();
  if (port.empty()) {
    port = "80";
  }

  tcp::resolver::query query(request.host(), port);

  boost::system::error_code ec;
  tcp::resolver::iterator it = resolver.resolve(query, ec);

  if (ec) {
    return kHostResolveError;
  }

  socket.connect(*it, ec);

  if (ec) {
    return kEndpointConnectError;
  }

  std::string request_str;
  request.ToString(request_str);

  try {
    boost::asio::write(socket, boost::asio::buffer(request_str));
    boost::asio::write(socket, boost::asio::buffer(body));
  } catch (boost::system::system_error&) {
    return kSocketWriteError;
  }

  // Read and parse HTTP response.

  // We must stop trying to read some once all content has been received,
  // because some servers will block extra call to read_some().
  while (!response->finished()) {
    size_t len = socket.read_some(boost::asio::buffer(bytes_), ec);

    if (len == 0 || ec) {
      return kSocketReadError;
    }

    // Parse the response piece just read.
    // If the content has been fully received, next time flag "finished_" will
    // be set.
    ErrorCode error = response->Parse(bytes_.data(), len);

    if (error != kNoError) {
      return error;
    }
  }

  return kNoError;
}

}  // namespace csoap
