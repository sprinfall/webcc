#ifndef WEBCC_SOAP_RESPONSE_H_
#define WEBCC_SOAP_RESPONSE_H_

#include <functional>
#include <memory>
#include <string>
#include <utility>  // for move()

#include "webcc/soap_message.h"
#include "webcc/soap_xml.h"

namespace webcc {

class SoapResponse : public SoapMessage {
 public:
  SoapResponse() : is_cdata_(false) {}

  // Response result parser.
  // Called on each child of the response node.
  // Example:
  //   - SOAP action: xxx
  //   - Given SOAP response:
  //     <soap:Body>
  //         <n:xxxResponse xmlns:n="...">
  //             <n:aaa>Blaah</aaa>
  //             <n:bbb>Blaah</bbb>
  //             <n:ccc>Blaah</ccc>
  //         </n:xxxResponse>
  //     <soap:Body>
  // The parser will be called in the following sequence:
  //   - parser(xaaa);  // return true
  //   - parser(xbbb);  // return true
  //   - parser(xccc);
  // If any of the parser returns false, the call sequence will be stopped:
  //   - parser(xaaa);  // return false
  //     <stopped>
  // Then you can get the expected result by parsing the node one by one.
  // When you implement the parser, you normally need to check the node name,
  // the following helper can extract the name without any namespace prefix:
  //   webcc::soap_xml::GetNameNoPrefix(xnode);
  typedef std::function<bool(pugi::xml_node)> Parser;

  void set_parser(Parser parser) {
    parser_ = parser;
  }

  std::shared_ptr<SoapFault> fault() const {
    return fault_;
  }

  // Could be "Price" for an operation/method like "GetXyzPrice".
  // Really depend on the service.
  // Most services use a general name "Result".
  void set_result_name(const std::string& result_name) {
    result_name_ = result_name;
  }

  // Server only.
  void set_result(const std::string& result, bool is_cdata) {
    result_ = result;
    is_cdata_ = is_cdata;
  }

  // Server only.
  void set_result_moved(std::string&& result, bool is_cdata) {
    result_ = std::move(result);
    is_cdata_ = is_cdata;
  }

 protected:
  void ToXmlBody(pugi::xml_node xbody) override;

  bool FromXmlBody(pugi::xml_node xbody) override;

 private:
  // Fault element if any.
  std::shared_ptr<SoapFault> fault_;

  // Response result parser.
  Parser parser_;

  // Result XML node name.
  // Used to parse the response XML from client side.
  // TODO
  std::string result_name_;

  // Result value.
  // TODO
  std::string result_;

  // CDATA result.
  bool is_cdata_;
};

}  // namespace webcc

#endif  // WEBCC_SOAP_RESPONSE_H_
