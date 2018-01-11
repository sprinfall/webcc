#include "csoap/http_response.h"

#include "csoap/common.h"
#include "csoap/xml.h"

namespace csoap {

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const HttpResponse& response) {
  // TODO
  os << response.status() << std::endl;

  // Pretty print the SOAP response XML.
  if (!xml::PrettyPrintXml(os, response.content())) {
    os << response.content();
  }

  return os;
}

////////////////////////////////////////////////////////////////////////////////

namespace status_strings {

const std::string OK = "HTTP/1.1 200 OK\r\n";
const std::string BAD_REQUEST = "HTTP/1.1 400 Bad Request\r\n";
const std::string INTERNAL_SERVER_ERROR = "HTTP/1.1 500 Internal Server Error\r\n";
const std::string NOT_IMPLEMENTED = "HTTP/1.1 501 Not Implemented\r\n";
const std::string SERVICE_UNAVAILABLE = "HTTP/1.1 503 Service Unavailable\r\n";

boost::asio::const_buffer ToBuffer(int status) {
  switch (status) {
    case HttpStatus::OK:
      return boost::asio::buffer(OK);

    case HttpStatus::BAD_REQUEST:
      return boost::asio::buffer(BAD_REQUEST);

    case HttpStatus::INTERNAL_SERVER_ERROR:
      return boost::asio::buffer(INTERNAL_SERVER_ERROR);

    case HttpStatus::NOT_IMPLEMENTED:
      return boost::asio::buffer(NOT_IMPLEMENTED);

    case HttpStatus::SERVICE_UNAVAILABLE:
      return boost::asio::buffer(SERVICE_UNAVAILABLE);

    default:
      return boost::asio::buffer(SERVICE_UNAVAILABLE);
  }
}

}  // namespace status_strings

namespace misc_strings {

const char NAME_VALUE_SEPARATOR[] = { ':', ' ' };
const char CRLF[] = { '\r', '\n' };

}  // misc_strings

////////////////////////////////////////////////////////////////////////////////

// ATTENTION: The buffers don't hold the memory!
std::vector<boost::asio::const_buffer> HttpResponse::ToBuffers() const {
  std::vector<boost::asio::const_buffer> buffers;

  buffers.push_back(status_strings::ToBuffer(status_));

  // Header fields
  for (const HttpHeader& h : headers_) {
    buffers.push_back(boost::asio::buffer(h.name));
    buffers.push_back(boost::asio::buffer(misc_strings::NAME_VALUE_SEPARATOR));
    buffers.push_back(boost::asio::buffer(h.value));
    buffers.push_back(boost::asio::buffer(misc_strings::CRLF));
  }

  buffers.push_back(boost::asio::buffer(misc_strings::CRLF));
  buffers.push_back(boost::asio::buffer(content_));

  return buffers;
}

// TODO: Move to SoapResponse
static void CreateSoapFaultResponse(HttpStatus status,
                                    std::string* xml_string) {
  Namespace soapenv_ns{
    "soap",
    "http://schemas.xmlsoap.org/soap/envelope/"
  };

  pugi::xml_document xdoc;
  pugi::xml_node xroot = xml::AddChild(xdoc, soapenv_ns.name, "Envelope");

  xml::AddNSAttr(xroot, soapenv_ns.name, soapenv_ns.url);

  // FIXME: Body
  // See https://www.w3schools.com/XML/xml_soap.asp

  pugi::xml_node xfault = xml::AddChild(xroot, soapenv_ns.name, "Fault");

  pugi::xml_node xfaultcode = xfault.append_child("faultcode");
  xfaultcode.text().set(std::to_string(HttpStatus::BAD_REQUEST).c_str());  // TODO

  pugi::xml_node xfaultstring = xfault.append_child("faultstring");
  xfaultstring.text().set("Bad Request");  // TODO

  // TODO: faultactor

  xml::XmlStrRefWriter writer(xml_string);
  xdoc.save(writer, "\t", pugi::format_default, pugi::encoding_utf8);
}

HttpResponse HttpResponse::Fault(HttpStatus status) {
  assert(status != HttpStatus::OK);

  HttpResponse response;

  std::string content;
  CreateSoapFaultResponse(status, &content);

  response.set_content(content);
  response.set_content_length(content.length());
  response.set_content_type("text/xml");

  response.set_status(status);

  return response;  // TODO: Output parameter?
}

}  // namespace csoap
