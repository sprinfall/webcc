#include "webcc/soap_request.h"

#include "webcc/soap_xml.h"

namespace webcc {

void SoapRequest::AddParameter(const SoapParameter& parameter) {
  parameters_.push_back(parameter);
}

void SoapRequest::AddParameter(SoapParameter&& parameter) {
  parameters_.push_back(std::move(parameter));
}

const std::string& SoapRequest::GetParameter(const std::string& key) const {
  for (const SoapParameter& p : parameters_) {
    if (p.key() == key) {
      return p.value();
    }
  }

  static const std::string kEmptyValue;
  return kEmptyValue;
}

void SoapRequest::ToXmlBody(pugi::xml_node xbody) {
  pugi::xml_node xop = soap_xml::AddChild(xbody, service_ns_.name, operation_);
  soap_xml::AddNSAttr(xop, service_ns_.name, service_ns_.url);

  for (SoapParameter& p : parameters_) {
    pugi::xml_node xparam = soap_xml::AddChild(xop, service_ns_.name, p.key());

    // xparam.text().set() also works for PCDATA.
    xparam.append_child(p.as_cdata() ? pugi::node_cdata : pugi::node_pcdata)
        .set_value(p.c_value());
  }
}

bool SoapRequest::FromXmlBody(pugi::xml_node xbody) {
  pugi::xml_node xoperation = xbody.first_child();
  if (!xoperation) {
    return false;
  }

  soap_xml::SplitName(xoperation, &service_ns_.name, &operation_);
  service_ns_.url = soap_xml::GetNSAttr(xoperation, service_ns_.name);

  pugi::xml_node xparameter = xoperation.first_child();
  while (xparameter) {
    parameters_.push_back({
      soap_xml::GetNameNoPrefix(xparameter),
      // xparameter.text().get/as_string() also works.
      std::string(xparameter.child_value())
    });

    xparameter = xparameter.next_sibling();
  }

  return true;
}

}  // namespace webcc
