#ifndef WEBCC_GLOBALS_H_
#define WEBCC_GLOBALS_H_

#include <string>
#include <vector>

// -----------------------------------------------------------------------------
// Macros

// Explicitly declare the copy constructor and assignment operator as deleted.
#define DELETE_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete; \
  TypeName& operator=(const TypeName&) = delete;

namespace webcc {

// -----------------------------------------------------------------------------
// Constants

// Buffer size for sending HTTP request and receiving HTTP response.
// TODO: Configurable
const std::size_t kBufferSize = 1024;

const std::size_t kInvalidLength = static_cast<std::size_t>(-1);

// Timeout seconds.
const int kMaxConnectSeconds = 10;
const int kMaxSendSeconds = 10;
const int kMaxReceiveSeconds = 30;

extern const std::string kContentType;
extern const std::string kContentLength;
extern const std::string kSoapAction;
extern const std::string kHost;

extern const std::string kTextXmlUtf8;
extern const std::string kTextJsonUtf8;

// HTTP methods (verbs) in string ("HEAD", "GET", etc.).
// NOTE: Don't use enum to avoid converting back and forth.
extern const std::string kHttpHead;
extern const std::string kHttpGet;
extern const std::string kHttpPost;
extern const std::string kHttpPatch;
extern const std::string kHttpPut;
extern const std::string kHttpDelete;

// HTTP response status.
// This is not a full list.
// Full list: https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
// NTOE: Don't use enum class because we want to convert to/from int easily.
struct HttpStatus {
  enum Enum {
    kOK = 200,
    kCreated = 201,
    kAccepted = 202,
    kNoContent = 204,
    kNotModified = 304,
    kBadRequest = 400,
    kNotFound = 404,
    InternalServerError = 500,
    kNotImplemented = 501,
    kServiceUnavailable = 503,
  };
};

// Error codes.
enum Error {
  kNoError = 0,

  kHostResolveError,
  kEndpointConnectError,

  kSocketTimeoutError,

  kSocketReadError,
  kSocketWriteError,

  // Invalid start line in the HTTP response.
  kHttpStartLineError,

  // Status is not 200 in the HTTP response.
  kHttpStatusError,

  // Invalid or missing Content-Length in the HTTP response.
  kHttpContentLengthError,

  kXmlError,
};

// Return a descriptive message for the given error code.
const char* DescribeError(Error error);

// -----------------------------------------------------------------------------

// Key-value parameter.
class Parameter {
 public:
  Parameter() = default;
  Parameter(const Parameter&) = default;
  Parameter& operator=(const Parameter&) = default;

  Parameter(const std::string& key, const char* value);
  Parameter(const std::string& key, const std::string& value);
  Parameter(const std::string& key, std::string&& value);
  Parameter(const std::string& key, int value);
  Parameter(const std::string& key, double value);
  Parameter(const std::string& key, bool value);

  // Use "= default" if drop the support of VS 2013.
  Parameter(Parameter&& rhs);

  // Use "= default" if drop the support of VS 2013.
  Parameter& operator=(Parameter&& rhs);

  const std::string& key() const {
    return key_;
  }

  const std::string& value() const {
    return value_;
  }

  const char* c_key() const {
    return key_.c_str();
  }

  const char* c_value() const {
    return value_.c_str();
  }

  // Return "key=value" string.
  std::string ToString() const;

 private:
  std::string key_;
  std::string value_;
};

}  // namespace webcc

#endif  // WEBCC_GLOBALS_H_
