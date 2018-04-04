#ifndef WEBCC_XML_H_
#define WEBCC_XML_H_

// XML utilities.

#include <string>
#include "pugixml/pugixml.hpp"

namespace webcc {
namespace xml {

// Split the node name into namespace prefix and real name.
// E.g., if the node name is "soapenv:Envelope", it will be splited to
// "soapenv" and "Envelope".
void SplitName(const pugi::xml_node& xnode,
               std::string* prefix = NULL,
               std::string* name = NULL);

// Get the namespace prefix from node name.
// E.g., if the node name is "soapenv:Envelope", NS prefix will be "soapenv".
std::string GetPrefix(const pugi::xml_node& xnode);

// Get the node name without namespace prefix.
std::string GetNameNoPrefix(const pugi::xml_node& xnode);

// Add a child with the given name which is prefixed by a namespace.
// E.g., AppendChild(xnode, "soapenv", "Envelope") will append a child with
// name "soapenv:Envelope".
pugi::xml_node AddChild(pugi::xml_node& xnode,
                        const std::string& ns,
                        const std::string& name);

pugi::xml_node GetChild(pugi::xml_node& xnode,
                        const std::string& ns,
                        const std::string& name);

// TODO: Remove
pugi::xml_node GetChildNoNS(pugi::xml_node& xnode,
                            const std::string& name);

// Add an attribute with the given name which is prefixed by a namespace.
void AddAttr(pugi::xml_node& xnode,
             const std::string& ns,
             const std::string& name,
             const std::string& value);

// Append "xmlns" attribute.
// E.g., if the namespace is
//   { "soapenv", "http://schemas.xmlsoap.org/soap/envelope/" }
// the attribute added will be
//   xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/"
void AddNSAttr(pugi::xml_node& xnode,
               const std::string& ns_name,
               const std::string& ns_url);

// Get namespace attribute value.
// E.g., if the given namespace name is "soapenv", the value of
// attribute "xmlns:soapenv" will be returned.
std::string GetNSAttr(pugi::xml_node& xnode,
                      const std::string& ns_name);

// An XML writer writing to a referenced string.
// Example:
//   pugi::xml_document xdoc;
//   ...
//   std::string xml_string;
//   XmlStrRefWriter writer(&xml_string);
//   xdoc.save(writer, "\t", pugi::format_default, pugi::encoding_utf8);
class XmlStrRefWriter : public pugi::xml_writer {
public:
  explicit XmlStrRefWriter(std::string* result)
      : result_(result) {
    result_->clear();
  }

  virtual void write(const void* data, size_t size) override {
    result_->append(static_cast<const char*>(data), size);
  }

private:
  std::string* result_;
};

bool PrettyPrintXml(std::ostream& os,
                    const std::string& xml_string,
                    const char* indent = "\t");

}  // namespace xml
}  // namespace webcc

#endif  // WEBCC_XML_H_
