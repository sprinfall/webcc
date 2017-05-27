#include "csoap/soap_request.h"
#include "csoap/xml.h"

namespace csoap {

////////////////////////////////////////////////////////////////////////////////

// Append "xmlns" attribute.
static void AppendAttrNS(pugi::xml_node& xnode, const Namespace& ns) {
  xml::AppendAttr(xnode, "xmlns", ns.name, ns.url);
}

////////////////////////////////////////////////////////////////////////////////

SoapRequest::SoapRequest(const std::string& operation)
    : operation_(operation) {
  soapenv_ns_.name = "soapenv";
  soapenv_ns_.url = "http://schemas.xmlsoap.org/soap/envelope/";
}

void SoapRequest::AddParameter(const std::string& key,
                               const std::string& value) {
  parameters_.push_back(Parameter(key, value));
}

void SoapRequest::AddParameter(const Parameter& parameter) {
  parameters_.push_back(parameter);
}

void SoapRequest::ToXmlString(std::string* xml_string) {
  pugi::xml_document xdoc;
  pugi::xml_node xroot = xml::AppendChild(xdoc, soapenv_ns_.name, "Envelope");

  AppendAttrNS(xroot, soapenv_ns_);
  AppendAttrNS(xroot, service_ns_);

  xml::AppendChild(xroot, soapenv_ns_.name, "Header");

  pugi::xml_node xbody = xml::AppendChild(xroot, soapenv_ns_.name, "Body");
  pugi::xml_node xop = xml::AppendChild(xbody, service_ns_.name, operation_);

  for (Parameter& p : parameters_) {
    pugi::xml_node xparam = xml::AppendChild(xop, service_ns_.name, p.c_key());
    xparam.text().set(p.c_value());
  }

  xml::XmlStrRefWriter writer(xml_string);
  xdoc.print(writer, "\t", pugi::format_default, pugi::encoding_utf8);
}

}  // namespace csoap
