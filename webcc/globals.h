#ifndef WEBCC_GLOBALS_H_
#define WEBCC_GLOBALS_H_

#include <string>

#include "webcc/version.h"

// -----------------------------------------------------------------------------
// Macros

// Does the compiler support "= default" for move copy constructor and
// move assignment operator?
#ifdef _MSC_VER
  #if _MSC_VER <= 1800  // VS 2013
    #define WEBCC_DEFAULT_MOVE_COPY_ASSIGN 0
  #else
    #define WEBCC_DEFAULT_MOVE_COPY_ASSIGN 1
  #endif  // _MSC_VER <= 1800
#else
  #define WEBCC_DEFAULT_MOVE_COPY_ASSIGN 1
#endif  // _MSC_VER

// Explicitly declare the copy constructor and assignment operator as deleted.
#define WEBCC_DELETE_COPY_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete; \
  TypeName& operator=(const TypeName&) = delete;

namespace webcc {

// -----------------------------------------------------------------------------
// Constants

const char* const CRLF = "\r\n";

// Default buffer size for socket reading.
const std::size_t kBufferSize = 1024;

const std::size_t kInvalidLength = std::string::npos;

// Default timeout for reading response.
const int kMaxReadSeconds = 30;

// Max size of the HTTP body to dump/log.
// If the HTTP, e.g., response, has a very large content, it will be truncated
// when dumped/logged.
const std::size_t kMaxDumpSize = 2048;

// HTTP headers.
namespace http {

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
const char* const kContentType = "Content-Type";
const char* const kContentLength = "Content-Length";
const char* const kConnection = "Connection";
const char* const kTransferEncoding = "Transfer-Encoding";
const char* const kAccept = "Accept";
const char* const kAcceptEncoding = "Accept-Encoding";
const char* const kUserAgent = "User-Agent";

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

}  // namespace http

// Default ports.
const char* const kHttpPort = "80";
const char* const kHttpSslPort = "443";

// HTTP methods (verbs) in string ("HEAD", "GET", etc.).
// NOTE: Don't use enum to avoid converting back and forth.
// TODO: Add enum.
extern const std::string kHttpHead;
extern const std::string kHttpGet;
extern const std::string kHttpPost;
extern const std::string kHttpPatch;
extern const std::string kHttpPut;
extern const std::string kHttpDelete;

// Client side error codes.
enum Error {
  kNoError = 0,  // i.e., OK

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

  // XML parsing error.
  kXmlError,
};

// Return a descriptive message for the given error code.
const char* DescribeError(Error error);

}  // namespace webcc

#endif  // WEBCC_GLOBALS_H_
