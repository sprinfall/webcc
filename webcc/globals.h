#ifndef WEBCC_GLOBALS_H_
#define WEBCC_GLOBALS_H_

#include <cassert>
#include <exception>
#include <filesystem>
#include <iosfwd>
#include <limits>
#include <string>
#include <vector>

#include "boost/asio/buffer.hpp"  // for const_buffer

#include "webcc/config.h"
#include "webcc/string.h"

namespace webcc {

namespace sfs = std::filesystem;

// -----------------------------------------------------------------------------

// Regex sub-matches of the URL (usually resource ID's).
// Could also be considered as arguments, so named as UrlArgs.
using UrlArgs = std::vector<std::string>;

using Payload = std::vector<boost::asio::const_buffer>;

// -----------------------------------------------------------------------------

constexpr std::size_t kInvalidSize = std::numeric_limits<std::size_t>::max();

// Default timeout for reading response.
constexpr int kMaxReadSeconds = 30;

// Max size of the HTTP body to dump/log.
// If the HTTP, e.g., response, has a very large content, it will be truncated
// when dumped/logged.
constexpr std::size_t kMaxDumpSize = 2048;

// Default buffer size for socket reading.
constexpr std::size_t kBufferSize = 1024;

// Why 1400? See the following page:
// https://www.itworld.com/article/2693941/why-it-doesn-t-make-sense-to-
// gzip-all-content-from-your-web-server.html
constexpr std::size_t kGzipThreshold = 1400;

// -----------------------------------------------------------------------------

namespace methods {

// HTTP methods (verbs) in string.
// Don't use enum to avoid converting back and forth.

const char* const kGet = "GET";
const char* const kHead = "HEAD";
const char* const kPost = "POST";
const char* const kPut = "PUT";
const char* const kDelete = "DELETE";
const char* const kConnect = "CONNECT";
const char* const kOptions = "OPTIONS";
const char* const kTrace = "TRACE";
const char* const kPatch = "PATCH";

}  // namespace methods

namespace status_codes {

// HTTP status codes.
// Don't use "enum class" for converting to/from int easily.
// The full list is available here:
//   https://en.wikipedia.org/wiki/List_of_HTTP_status_codes

constexpr int kOK = 200;
constexpr int kCreated = 201;
constexpr int kAccepted = 202;
constexpr int kCallbackFailed = 203;
constexpr int kNoContent = 204;
constexpr int kNotModified = 304;
constexpr int kBadRequest = 400;
constexpr int kForbidden = 403;
constexpr int kNotFound = 404;
constexpr int kInternalServerError = 500;
constexpr int kNotImplemented = 501;
constexpr int kServiceUnavailable = 503;

}  // namespace status_codes

namespace headers {

// NOTE: Field names are case-insensitive.
//   See https://stackoverflow.com/a/5259004 for more details.

const char* const kHost = "Host";
const char* const kDate = "Date";
const char* const kAuthorization = "Authorization";
const char* const kContentType = "Content-Type";
const char* const kContentLength = "Content-Length";
const char* const kContentEncoding = "Content-Encoding";
const char* const kContentMD5 = "Content-MD5";
const char* const kContentDisposition = "Content-Disposition";
const char* const kConnection = "Connection";
const char* const kTransferEncoding = "Transfer-Encoding";
const char* const kAccept = "Accept";
const char* const kAcceptEncoding = "Accept-Encoding";
const char* const kUserAgent = "User-Agent";
const char* const kServer = "Server";

}  // namespace headers

namespace media_types {

// See the following link for the full list of media types:
//   https://www.iana.org/assignments/media-types/media-types.xhtml

const char* const kApplicationJson = "application/json";
const char* const kApplicationSoapXml = "application/soap+xml";
const char* const kApplicationFormUrlEncoded =
    "application/x-www-form-urlencoded";
const char* const kTextPlain = "text/plain";
const char* const kTextXml = "text/xml";

// Get media type from file extension.
std::string FromExtension(const std::string& ext);

}  // namespace media_types

namespace charsets {

const char* const kUtf8 = "utf-8";

}  // namespace charsets

enum class ContentEncoding {
  kUnknown,
  kGzip,
  kDeflate,
};

// -----------------------------------------------------------------------------

namespace error_codes {

constexpr int kUnknownError = -1;
constexpr int kOK = 0;
constexpr int kStateError = 1;
constexpr int kSyntaxError = 2;
constexpr int kResolveError = 3;
constexpr int kConnectError = 4;
constexpr int kHandshakeError = 5;
constexpr int kSocketReadError = 6;
constexpr int kSocketWriteError = 7;
constexpr int kParseError = 8;
constexpr int kFileError = 9;
constexpr int kDataError = 10;

}  // namespace error_codes

// Error or exception (for client only).
class Error : public std::exception {
public:
  explicit Error(int code = error_codes::kOK, std::string_view message = "")
      : code_(code), message_(message) {
  }

  // Note that `noexcept` is required by GCC.
  const char* what() const noexcept override {
    return message_.c_str();
  }

  int code() const {
    return code_;
  }

  const std::string& message() const {
    return message_;
  }

  void Set(int code, std::string_view message) {
    code_ = code;
    message_ = message;
  }

  void Clear() {
    code_ = error_codes::kOK;
    message_.clear();
    timeout_ = false;
  }

  bool timeout() const {
    return timeout_;
  }

  void set_timeout(bool timeout) {
    timeout_ = timeout;
  }

  bool failed() const {
    return code_ != error_codes::kOK;
  }

  operator bool() const {
    return failed();
  }

private:
  int code_;
  std::string message_;
  bool timeout_ = false;
};

std::ostream& operator<<(std::ostream& os, const Error& error);

}  // namespace webcc

#endif  // WEBCC_GLOBALS_H_
