#ifndef CSOAP_COMMON_H_
#define CSOAP_COMMON_H_

// Common definitions.

#include <string>

namespace csoap {

////////////////////////////////////////////////////////////////////////////////

enum ErrorCode {
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
const char* GetErrorMessage(ErrorCode error_code);

////////////////////////////////////////////////////////////////////////////////

// XML namespace name/url pair.
// E.g., { "soapenv", "http://schemas.xmlsoap.org/soap/envelope/" }
class Namespace {
public:
  std::string name;
  std::string url;
};

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
