#include "csoap/soap_response.h"

#include "csoap/xml.h"

namespace csoap {

SoapResponse::SoapResponse() {
}

bool SoapResponse::Parse(const std::string& content,
                         const std::string& message_name,
                         const std::string& element_name,
                         std::string* element_value) {
  pugi::xml_document xdoc;
  pugi::xml_parse_result result = xdoc.load_string(content.c_str());

  if (!result) {
    //std::cout << "Error: " << result.description() << std::endl;
    return false;
  }

  pugi::xml_node xroot = xdoc.document_element();

  soapenv_ns_ = csoap::xml::GetNsPrefix(xroot);

  pugi::xml_node xbody = csoap::xml::GetChild(xroot, soapenv_ns_, "Body");
  if (!xbody) {
    return false;
  }

  pugi::xml_node xmessage = csoap::xml::GetChildNoNS(xbody, message_name);
  if (!xmessage) {
    return false;
  }

  pugi::xml_node xelement = csoap::xml::GetChildNoNS(xmessage, element_name);
  if (!xelement) {
    return false;
  }

  *element_value = xelement.text().get();

  return true;
}

}  // namespace csoap
