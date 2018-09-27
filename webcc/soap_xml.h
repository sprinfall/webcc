#ifndef WEBCC_SOAP_XML_H_
#define WEBCC_SOAP_XML_H_

// XML helpers for SOAP messages.

#include <string>

#include "pugixml/pugixml.hpp"

namespace webcc {
namespace soap_xml {

// Split the node name into namespace prefix and real name.
// E.g., if the node name is "soapenv:Envelope", it will be splitted to
// "soapenv" and "Envelope".
void SplitName(const pugi::xml_node& xnode, std::string* prefix = nullptr,
               std::string* name = nullptr);

// Get the namespace prefix from node name.
// E.g., if the node name is "soapenv:Envelope", NS prefix will be "soapenv".
std::string GetPrefix(const pugi::xml_node& xnode);

// Get the node name without namespace prefix.
std::string GetNameNoPrefix(const pugi::xml_node& xnode);

// Get node text (applicable for both PCDATA and CDATA).
// E.g., given the following node:
//   <Name>Chunting Gu</Name>
// GetText returns "Chunting Gu".
std::string GetText(const pugi::xml_node& xnode);

// Output parameter version GetText.
void GetText(const pugi::xml_node& xnode, std::string* text);

// Set node text.
void SetText(pugi::xml_node xnode, const std::string& text, bool is_cdata);

// Add a child with the given name which is prefixed by a namespace.
// E.g., AppendChild(xnode, "soapenv", "Envelope") will append a child with
// name "soapenv:Envelope".
pugi::xml_node AddChild(pugi::xml_node xnode,
                        const std::string& ns, const std::string& name);

pugi::xml_node GetChild(const pugi::xml_node& xnode, const std::string& ns,
                        const std::string& name);

pugi::xml_node GetChildNoNS(const pugi::xml_node& xnode,
                            const std::string& name);

// Add an attribute with the given name which is prefixed by a namespace.
void AddAttr(pugi::xml_node xnode, const std::string& ns,
             const std::string& name, const std::string& value);

// Append "xmlns" attribute.
// E.g., if the namespace is
//   { "soapenv", "http://schemas.xmlsoap.org/soap/envelope/" }
// the attribute added will be
//   xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/"
void AddNSAttr(pugi::xml_node xnode, const std::string& ns_name,
               const std::string& ns_url);

// Get namespace attribute value.
// E.g., if the given namespace name is "soapenv", the value of
// attribute "xmlns:soapenv" will be returned.
std::string GetNSAttr(const pugi::xml_node& xnode,
                      const std::string& ns_name);

// An XML writer writing to a referenced string.
// Example:
//   pugi::xml_document xdoc;
//   ...
//   std::string xml_string;
//   XmlStringWriter writer(&xml_string);
//   xdoc.save/print(writer);
class XmlStringWriter : public pugi::xml_writer {
 public:
  explicit XmlStringWriter(std::string* result) : result_(result) {
    result_->clear();
  }

  void write(const void* data, std::size_t size) override {
    result_->append(static_cast<const char*>(data), size);
  }

 private:
  std::string* result_;
};

// Print the XML string to output stream in pretty format.
bool PrettyPrint(std::ostream& os, const std::string& xml_string,
                 const char* indent = "\t");

}  // namespace soap_xml
}  // namespace webcc

#endif  // WEBCC_SOAP_XML_H_
