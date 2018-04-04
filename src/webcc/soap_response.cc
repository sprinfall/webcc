#include "webcc/soap_response.h"

#include <cassert>
#include "webcc/xml.h"

namespace webcc {

void SoapResponse::ToXmlBody(pugi::xml_node xbody) {
  std::string rsp_operation = operation_ + "Response";
  pugi::xml_node xop = xml::AddChild(xbody, service_ns_.name, rsp_operation);
  xml::AddNSAttr(xop, service_ns_.name, service_ns_.url);

  pugi::xml_node xresult = xml::AddChild(xop, service_ns_.name, result_name_);
  xresult.text().set(result_.c_str());
}

bool SoapResponse::FromXmlBody(pugi::xml_node xbody) {
  assert(!result_name_.empty());

  pugi::xml_node xresponse = xbody.first_child();
  if (xresponse) {
    xml::SplitName(xresponse, &service_ns_.name, NULL);
    service_ns_.url = xml::GetNSAttr(xresponse, service_ns_.name);

    pugi::xml_node xresult = xml::GetChildNoNS(xresponse, result_name_);
    if (xresult) {
      result_ = xresult.text().get();
      return true;
    }
  }

  return false;
}

}  // namespace webcc
