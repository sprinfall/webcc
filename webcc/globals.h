#ifndef WEBCC_GLOBALS_H_
#define WEBCC_GLOBALS_H_

#include <cassert>
#include <exception>
#include <string>

namespace webcc {

// -----------------------------------------------------------------------------

const char* const kCRLF = "\r\n";

const std::size_t kInvalidLength = std::string::npos;

// Default timeout for reading response.
const int kMaxReadSeconds = 30;

// Max size of the HTTP body to dump/log.
// If the HTTP, e.g., response, has a very large content, it will be truncated
// when dumped/logged.
const std::size_t kMaxDumpSize = 2048;

// Default buffer size for socket reading.
const std::size_t kBufferSize = 1024;

// Default ports.
const char* const kPort80 = "80";
const char* const kPort443 = "443";

// Why 1400? See the following page:
// https://www.itworld.com/article/2693941/why-it-doesn-t-make-sense-to-
// gzip-all-content-from-your-web-server.html
// TODO: Configurable
const std::size_t kGzipThreshold = 1400;

// -----------------------------------------------------------------------------

// HTTP headers.
namespace http {

namespace methods {

// HTTP methods (verbs) in string.
// Don't use enum to avoid converting back and forth.

const char* const kGet      = "GET";
const char* const kHead     = "HEAD";
const char* const kPost     = "POST";
const char* const kPut      = "PUT";
const char* const kDelete   = "DELETE";
const char* const kConnect  = "CONNECT";
const char* const kOptions  = "OPTIONS";
const char* const kTrace    = "TRACE";
const char* const kPatch    = "PATCH";

}  // namespace methods

// HTTP response status.
// This is not a full list.
// Full list: https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
// NTOE: Don't use enum class because we want to convert to/from int easily.
enum Status {
  kOK = 200,
  kCreated = 201,
  kAccepted = 202,
  kNoContent = 204,
  kNotModified = 304,
  kBadRequest = 400,
  kNotFound = 404,
  kInternalServerError = 500,
  kNotImplemented = 501,
  kServiceUnavailable = 503,
};

namespace headers {

// NOTE: Field names are case-insensitive.
//   See https://stackoverflow.com/a/5259004 for more details.

const char* const kHost = "Host";
const char* const kDate = "Date";
const char* const kContentType = "Content-Type";
const char* const kContentLength = "Content-Length";
const char* const kContentEncoding = "Content-Encoding";
const char* const kConnection = "Connection";
const char* const kTransferEncoding = "Transfer-Encoding";
const char* const kAccept = "Accept";
const char* const kAcceptEncoding = "Accept-Encoding";
const char* const kUserAgent = "User-Agent";
const char* const kServer = "Server";

}  // namespace headers

namespace media_types {

// NOTE:
// According to www.w3.org when placing SOAP messages in HTTP bodies, the HTTP
// Content-type header must be chosen as "application/soap+xml" [RFC 3902].
// But in practice, many web servers cannot understand it.
// See: https://www.w3.org/TR/2007/REC-soap12-part0-20070427/#L26854

const char* const kApplicationJson = "application/json";
const char* const kApplicationSoapXml = "application/soap+xml";
const char* const kTextXml = "text/xml";

}  // namespace media_types

namespace charsets {

const char* const kUtf8 = "utf-8";

}  // namespace charsets

enum class ContentEncoding {
  kUnknown,
  kGzip,
  kDeflate,
};

// Return default user agent for HTTP headers.
const std::string& UserAgent();

}  // namespace http

// -----------------------------------------------------------------------------

// Client side error codes.
enum Error {
  kNoError = 0,  // i.e., OK

  kSchemaError,

  kHostResolveError,
  kEndpointConnectError,
  kHandshakeError,  // HTTPS handshake
  kSocketReadError,
  kSocketWriteError,

  // HTTP error.
  // E.g., failed to parse HTTP response (invalid content length, etc.).
  kHttpError,

  // Server error.
  // E.g., HTTP status 500 + SOAP Fault element.
  kServerError,

  // File read/write error.
  kFileIOError,

  // XML parsing error.
  kXmlError,
};

// Return a descriptive message for the given error code.
const char* DescribeError(Error error);

class Exception : public std::exception {
public:
  explicit Exception(Error error, const std::string& details = "",
                     bool timeout = false);

  Error error() const { return error_; }

  // Note that `noexcept` is required by GCC.
  const char* what() const noexcept override {
    return msg_.c_str();
  }

  bool timeout() const { return timeout_; }

private:
  Error error_;

  std::string msg_;

  // If the error was caused by timeout or not.
  bool timeout_;
};

}  // namespace webcc

#endif  // WEBCC_GLOBALS_H_
