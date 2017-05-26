#ifndef CSOAP_XML_H_
#define CSOAP_XML_H_

#include <string>
#include "pugixml/pugixml.hpp"

// XML utilities.

namespace csoap {
namespace xml {

// Get the namespace prefix from node name.
// Example:
//   Node name: soapenv:Envelope
//   NS prefix: soapenv
std::string GetNsPrefix(const pugi::xml_node& xnode);

// Append a child with the given name which is prefixed by a namespace.
// E.g., AppendChild(xnode, "soapenv", "Envelope") will append a child with
// name "soapenv:Envelope".
pugi::xml_node AppendChild(pugi::xml_node& xnode,
                           const std::string& ns,
                           const std::string& name);

pugi::xml_node GetChild(pugi::xml_node& xnode,
                        const std::string& ns,
                        const std::string& name);

pugi::xml_node GetChildNoNS(pugi::xml_node& xnode,
                            const std::string& name);

// Append a attribute with the given name which is prefixed by a namespace.
void AppendAttr(pugi::xml_node& xnode,
                const std::string& ns,
                const std::string& name,
                const std::string& value);

// An XML writer writing to a referenced string.
// Example:
//   pugi::xml_document xdoc;
//   ...
//   std::string xml_string;
//   XmlStrRefWriter writer(&xml_string);
//   xdoc.print(writer, "  ", pugi::format_default, pugi::encoding_utf8);
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

}  // namespace xml
}  // namespace csoap

#endif  // CSOAP_XML_H_
