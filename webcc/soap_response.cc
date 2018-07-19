#include "webcc/soap_response.h"

#include <cassert>

#include "webcc/soap_xml.h"

namespace webcc {

void SoapResponse::ToXmlBody(pugi::xml_node xbody) {
  pugi::xml_node xop = soap_xml::AddChild(xbody, service_ns_.name,
                                          operation_ + "Response");
  soap_xml::AddNSAttr(xop, service_ns_.name, service_ns_.url);

  pugi::xml_node xresult = soap_xml::AddChild(xop, service_ns_.name,
                                              result_name_);

  // xresult.text().set() also works for PCDATA.
  xresult.append_child(is_cdata_ ? pugi::node_cdata : pugi::node_pcdata)
      .set_value(result_.c_str());
}

bool SoapResponse::FromXmlBody(pugi::xml_node xbody) {
  assert(!result_name_.empty());

  pugi::xml_node xresponse = xbody.first_child();
  if (xresponse) {
    soap_xml::SplitName(xresponse, &service_ns_.name, nullptr);
    service_ns_.url = soap_xml::GetNSAttr(xresponse, service_ns_.name);

    pugi::xml_node xresult = soap_xml::GetChildNoNS(xresponse, result_name_);
    if (xresult) {
      // The value of the first child node of type PCDATA/CDATA.
      // xresult.text().get/as_string() also works.
      result_ = xresult.child_value();
      return true;
    }
  }

  return false;
}

}  // namespace webcc
