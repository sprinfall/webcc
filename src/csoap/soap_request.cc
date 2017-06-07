#include "csoap/soap_request.h"
#include "csoap/xml.h"

namespace csoap {

////////////////////////////////////////////////////////////////////////////////

#ifdef CSOAP_USE_TINYXML

// Append "xmlns" attribute.
static void AddNSAttr(TiXmlElement* xnode, const Namespace& ns) {
  xml::AddAttr(xnode, "xmlns", ns.name, ns.url);
}

#else

// Append "xmlns" attribute.
static void AddNSAttr(pugi::xml_node& xnode, const Namespace& ns) {
  xml::AddAttr(xnode, "xmlns", ns.name, ns.url);
}

#endif  // CSOAP_USE_TINYXML

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
#ifdef CSOAP_USE_TINYXML

  TiXmlDocument xdoc;
  TiXmlElement* xroot = xml::AppendChild(&xdoc, soapenv_ns_.name, "Envelope");

  AddNSAttr(xroot, soapenv_ns_);
  AddNSAttr(xroot, service_ns_);

  xml::AppendChild(xroot, soapenv_ns_.name, "Header");

  TiXmlElement* xbody = xml::AppendChild(xroot, soapenv_ns_.name, "Body");
  TiXmlElement* xop = xml::AppendChild(xbody, service_ns_.name, operation_);

  for (Parameter& p : parameters_) {
    TiXmlElement* xparam = xml::AppendChild(xop, service_ns_.name, p.key());
    xml::SetText(xparam, p.value());
  }

  TiXmlPrinter printer;
  xdoc.Accept(&printer);
  *xml_string = printer.CStr();

#else

  pugi::xml_document xdoc;
  pugi::xml_node xroot = xml::AppendChild(xdoc, soapenv_ns_.name, "Envelope");

  AddNSAttr(xroot, soapenv_ns_);
  AddNSAttr(xroot, service_ns_);

  xml::AppendChild(xroot, soapenv_ns_.name, "Header");

  pugi::xml_node xbody = xml::AppendChild(xroot, soapenv_ns_.name, "Body");
  pugi::xml_node xop = xml::AppendChild(xbody, service_ns_.name, operation_);

  for (Parameter& p : parameters_) {
    pugi::xml_node xparam = xml::AppendChild(xop, service_ns_.name, p.key());
    xparam.text().set(p.value().c_str());
  }

  xml::XmlStrRefWriter writer(xml_string);
  xdoc.print(writer, "\t", pugi::format_default, pugi::encoding_utf8);

#endif  // #ifdef CSOAP_USE_TINYXML
}

}  // namespace csoap
