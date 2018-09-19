#include "webcc/soap_globals.h"

namespace webcc {

const std::string kSoapAction = "SOAPAction";

// According to www.w3.org when placing SOAP messages in HTTP bodies, the HTTP
// Content-type header must be chosen as "application/soap+xml" [RFC 3902].
// But in practice, many web servers cannot understand it.
// See: https://www.w3.org/TR/2007/REC-soap12-part0-20070427/#L26854
const std::string kTextXmlUtf8 = "text/xml; charset=utf-8";
const std::string kAppSoapXmlUtf8 = "application/soap+xml; charset=utf-8";

// NOTE:
// According to HTTP 1.1 RFC7231, the following examples are all equivalent,
// but the first is preferred for consistency:
//    text/html;charset=utf-8
//    text/html;charset=UTF-8
//    Text/HTML;Charset="utf-8"
//    text/html; charset="utf-8"
// See: https://tools.ietf.org/html/rfc7231#section-3.1.1.1

const SoapNamespace kSoapEnvNamespaceV11{
  "soap",
  "http://schemas.xmlsoap.org/soap/envelope/"
};

const SoapNamespace kSoapEnvNamespaceV12{
  "soap",
  "http://www.w3.org/2003/05/soap-envelope"
};

}  // namespace webcc
