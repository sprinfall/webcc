#include "csoap/soap_request_envelope.h"

#include "csoap/xml.h"

namespace csoap {

////////////////////////////////////////////////////////////////////////////////

// Append "xmlns" attribute.
static void AppendAttrNS(pugi::xml_node& xnode, const Namespace& ns) {
  xml::AppendAttr(xnode, "xmlns", ns.name, ns.url);
}

////////////////////////////////////////////////////////////////////////////////

SoapRequestEnvelope::SoapRequestEnvelope(const std::string& operation)
    : operation_(operation) {
}

void SoapRequestEnvelope::SetNamespace(NSType ns_type, const Namespace& ns) {
  assert(ns_type < kCountNS);
  namespaces_[ns_type] = ns;
}

void SoapRequestEnvelope::SetNamespace(NSType ns_type,
                                       const std::string& name,
                                       const std::string& url) {
  assert(ns_type < kCountNS);

  namespaces_[ns_type].name = name;
  namespaces_[ns_type].url = url;
}

void SoapRequestEnvelope::AddParameter(const std::string& key,
                                       const std::string& value) {
  parameters_.push_back(Parameter(key, value));
}

void SoapRequestEnvelope::AddParameter(const Parameter& parameter) {
  parameters_.push_back(parameter);
}

void SoapRequestEnvelope::ToXmlString(std::string* xml_string) {
  std::string& soapenv_ns = namespaces_[kSoapEnvelopeNS].name;
  std::string& srv_ns = namespaces_[kServiceNS].name;

  pugi::xml_document xdoc;
  pugi::xml_node xroot = xml::AppendChild(xdoc, soapenv_ns, "Envelope");

  AppendAttrNS(xroot, namespaces_[kSoapEnvelopeNS]);
  AppendAttrNS(xroot, namespaces_[kServiceNS]);

  xml::AppendChild(xroot, soapenv_ns, "Header");

  pugi::xml_node xbody = xml::AppendChild(xroot, soapenv_ns, "Body");
  pugi::xml_node xoperation = xml::AppendChild(xbody, srv_ns, operation_);

  for (Parameter& p : parameters_) {
    pugi::xml_node xparam = xml::AppendChild(xoperation, srv_ns, p.c_key());
    xparam.text().set(p.c_value());
  }

  xml::XmlStrRefWriter writer(xml_string);
  xdoc.print(writer, "\t", pugi::format_default, pugi::encoding_utf8);
}

}  // namespace csoap
