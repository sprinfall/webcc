#include "webcc/soap_response.h"

#include <cassert>

#include "webcc/soap_xml.h"

namespace webcc {

void SoapResponse::ToXmlBody(pugi::xml_node xbody) {
  pugi::xml_node xresponse = soap_xml::AddChild(xbody, service_ns_.name,
                                                operation_ + "Response");
  soap_xml::AddNSAttr(xresponse, service_ns_.name, service_ns_.url);

  if (simple_result_) {
    pugi::xml_node xresult = soap_xml::AddChild(xresponse, service_ns_.name,
                                                simple_result_->name);
    soap_xml::SetText(xresult, simple_result_->value, simple_result_->is_cdata);
  } else {
    assert(composer_);
    (*composer_)(xresponse);
  }
}

bool SoapResponse::FromXmlBody(pugi::xml_node xbody) {
  assert(parser_);

  // Check Fault element.

  pugi::xml_node xfault = soap_xml::GetChildNoNS(xbody, "Fault");

  // TODO: service_ns_.url

  if (xfault) {
    fault_.reset(new SoapFault);

    pugi::xml_node xfaultcode = soap_xml::GetChildNoNS(xfault, "faultcode");
    pugi::xml_node xfaultstring = soap_xml::GetChildNoNS(xfault, "faultstring");
    pugi::xml_node xdetail = soap_xml::GetChildNoNS(xfault, "detail");

    if (xfaultcode) {
      fault_->faultcode = xfaultcode.text().as_string();
    }
    if (xfaultstring) {
      fault_->faultstring = xfaultstring.text().as_string();
    }
    if (xdetail) {
      fault_->detail = xdetail.text().as_string();
    }

    return false;
  }

  // Check Response element.

  pugi::xml_node xresponse = soap_xml::GetChildNoNS(xbody,
                                                    operation_ + "Response");

  if (!xresponse) {
    return false;
  }

  soap_xml::SplitName(xresponse, &service_ns_.name, nullptr);
  service_ns_.url = soap_xml::GetNSAttr(xresponse, service_ns_.name);

  // Call result parser on each child of the response node.
  pugi::xml_node xchild = xresponse.first_child();
  while (xchild) {
    if (!parser_(xchild)) {
      break;
    }
    xchild = xchild.next_sibling();
  }

  return true;
}

}  // namespace webcc
