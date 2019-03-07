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
  //   - parser(aaa);  // return true
  //   - parser(bbb);  // return true
  //   - parser(ccc);
  // If any of the parser returns false, the call sequence will be stopped:
  //   - parser(aaa);  // return false
  //     <stopped>
  // Then you can get the expected result by parsing the node one by one.
  // When you implement the parser, you normally need to check the node name,
  // the following helper can extract the name without any namespace prefix:
  //   webcc::soap_xml::GetNameNoPrefix(node);
  typedef std::function<bool(pugi::xml_node)> Parser;

  // Response result composer.
  // Called on the response node.
  // Example:
  //   - SOAP action: xxx
  //   - Given SOAP response:
  //     <soap:Body>
  //         <n:xxxResponse xmlns:n="..." />
  //     <soap:Body>
  // The composer will be called as:
  //   - composer(xxxResponse);
  // The composer then add proper children to xxxResponse as the result.
  class Composer {
 public:
    void operator()(pugi::xml_node xresponse) {
      Compose(xresponse);
    }
  private:
    virtual void Compose(pugi::xml_node xresponse) = 0;
  };

  typedef std::shared_ptr<Composer> ComposerPtr;

  // Simple result means there's only one child of the response node.
  // Normally, the value of the result is a string (could be CDATA to embed an
  // XML string).
  // Given SOAP action "xxx", the response could be:
  // - Plain text as result value:
  //     <soap:Body>
  //         <n:xxxResponse xmlns:n="...">
  //             <n:Result>2.0</n:Result>
  //         </n:xxxResponse>
  //     <soap:Body>
  // - CDATA as result value:
  //     <soap:Body>
  //         <n:xxxResponse xmlns:n="...">
  //             <n:Result>
  //                 <![CDATA[
  //                 <webcc type = "response">
  //                   <status code = "0" message = "ok">
  //                   <book>
  //                     <id>1</id>
  //                   </book>
  //                 </webcc>
  //                 ]]>
  //             </n:Result>
  //         </n:xxxResponse>
  //     <soap:Body>
  struct SimpleResult {
    std::string name;    // Result XML node name.
    std::string value;   // Result value.
    bool is_cdata;       // CDATA result.
  };

  typedef std::shared_ptr<SimpleResult> SimpleResultPtr;

  SoapResponse() : parser_(nullptr) {}

  // Set the parser to parse the result.
  // Client only.
  void set_parser(Parser parser) {
    parser_ = parser;
  }

  // Set the composer to compose the result.
  // Composer will be ignored if the simple result is provided.
  void set_composer(ComposerPtr composer) {
    composer_ = composer;
  }

  // Set to compose from simple result.
  void set_simple_result(SimpleResultPtr simple_result) {
    simple_result_ = simple_result;  
  }

  // Set to compose from simple result (a shortcut).
  void set_simple_result(const std::string& name,
                         std::string&& value,
                         bool is_cdata) {
    simple_result_.reset(new SimpleResult{
      name, std::move(value), is_cdata
    });
  }

  std::shared_ptr<SoapFault> fault() const {
    return fault_;
  }

  // TODO: Set fault from server.

public:
  void ToXmlBody(pugi::xml_node xbody) override;

  bool FromXmlBody(pugi::xml_node xbody) override;

private:
  // Fault element if any.
  std::shared_ptr<SoapFault> fault_;

  // Result parser (for client).
  Parser parser_;

  // Result composer (for server).
  // Ignored if |simple_result_| is provided.
  ComposerPtr composer_;

  // Simple result (for server).
  SimpleResultPtr simple_result_;
};

}  // namespace webcc

#endif  // WEBCC_SOAP_RESPONSE_H_
