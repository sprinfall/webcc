#ifndef CSOAP_COMMON_H_
#define CSOAP_COMMON_H_

// Common definitions.

#include <string>
#include <vector>

namespace csoap {

////////////////////////////////////////////////////////////////////////////////

// API decorators.
// For a given class, e.g., SoapRequest, some APIs are for client while others
// are for server. In order to make it clear to the user, use the following
// macros to decorate.
#define SERVER_API
#define CLIENT_API

////////////////////////////////////////////////////////////////////////////////

// TODO

// Buffer size for sending HTTP request and receiving HTTP response.
const std::size_t BUF_SIZE = 1024;

static const std::string kCRLF = "\r\n";

extern const std::string kContentTypeName;
extern const std::string kContentLengthName;

extern const std::string kTextXmlUtf8;

const std::size_t kInvalidLength = std::string::npos;

////////////////////////////////////////////////////////////////////////////////

// Error codes.
enum Error {
  kNoError = 0,  // OK

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
const char* GetErrorMessage(Error error);

////////////////////////////////////////////////////////////////////////////////

// TODO: No 1.1 feature has been used or supported yet.
enum HttpVersion {
  kHttpV10,
  kHttpV11,
};

// HTTP response status.
// NOTE: Only support the listed status codes.
enum HttpStatus {
  OK = 200,
  BAD_REQUEST = 400,
  INTERNAL_SERVER_ERROR = 500,
  NOT_IMPLEMENTED = 501,
  SERVICE_UNAVAILABLE = 503,
};

enum HeaderField {
  kHeaderContentType,
  kHeaderContentLength,
  kHeaderHost,
};

////////////////////////////////////////////////////////////////////////////////

// XML namespace name/url pair.
// E.g., { "soap", "http://schemas.xmlsoap.org/soap/envelope/" }
class Namespace {
public:
  std::string name;
  std::string url;

  bool IsValid() const {
    return !name.empty() && !url.empty();
  }
};

// CSoap's default namespace for SOAP Envelope.
extern const Namespace kSoapEnvNamespace;

////////////////////////////////////////////////////////////////////////////////

// Parameter in the SOAP request envelope.
class Parameter {
public:
  Parameter(const std::string& key, const std::string& value);
  Parameter(const std::string& key, int value);
  Parameter(const std::string& key, double value);
  Parameter(const std::string& key, bool value);

  const std::string& key() const {
    return key_;
  }

  const std::string& value() const {
    return value_;
  }

private:
  std::string key_;
  std::string value_;
};

}  // namespace csoap

#endif  // CSOAP_COMMON_H_
