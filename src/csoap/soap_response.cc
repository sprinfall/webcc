#include "csoap/soap_response.h"

#include "csoap/xml.h"

namespace csoap {

SoapResponse::SoapResponse() {
}

bool SoapResponse::Parse(const std::string& content,
                         const std::string& message_name,
                         const std::string& element_name,
                         std::string* element_value) {
#ifdef CSOAP_USE_TINYXML

  TiXmlDocument xdoc;
  xdoc.Parse(content.c_str());

  if (xdoc.Error()) {
    return false;
  }

  TiXmlElement* xroot = xdoc.RootElement();

  soapenv_ns_ = xml::GetNsPrefix(xroot);

  TiXmlElement* xbody = xml::GetChild(xroot, soapenv_ns_, "Body");
  if (xbody == NULL) {
    return false;
  }

  TiXmlElement* xmessage = xml::GetChildNoNS(xbody, message_name);
  if (xmessage == NULL) {
    return false;
  }

  TiXmlElement* xelement = xml::GetChildNoNS(xmessage, element_name);
  if (xelement == NULL) {
    return false;
  }

  const char* text = xelement->GetText();
  if (text != NULL) {
    *element_value = text;
  } else {
    *element_value = "";
  }

#else

  pugi::xml_document xdoc;
  pugi::xml_parse_result result = xdoc.load_string(content.c_str());

  if (!result) {
    return false;
  }

  pugi::xml_node xroot = xdoc.document_element();

  soapenv_ns_ = xml::GetNsPrefix(xroot);

  pugi::xml_node xbody = xml::GetChild(xroot, soapenv_ns_, "Body");
  if (!xbody) {
    return false;
  }

  pugi::xml_node xmessage = xml::GetChildNoNS(xbody, message_name);
  if (!xmessage) {
    return false;
  }

  pugi::xml_node xelement = xml::GetChildNoNS(xmessage, element_name);
  if (!xelement) {
    return false;
  }

  *element_value = xelement.text().get();

#endif  // CSOAP_USE_TINYXML

  return true;
}

}  // namespace csoap
