#include "csoap/xml.h"

namespace csoap {
namespace xml {

std::string GetNsPrefix(const pugi::xml_node& xnode) {
  std::string node_name = xnode.name();

  size_t pos = node_name.find(':');

  if (pos != std::string::npos) {
    return node_name.substr(0, pos);
  }

  return "";
}

pugi::xml_node AppendChild(pugi::xml_node& xnode,
                           const std::string& ns,
                           const std::string& name) {
  std::string ns_name = ns + ":" + name;
  return xnode.append_child(ns_name.c_str());
}

pugi::xml_node GetChild(pugi::xml_node& xnode,
                        const std::string& ns,
                        const std::string& name) {
  return xnode.child((ns + ":" + name).c_str());
}

pugi::xml_node GetChildNoNS(pugi::xml_node& xnode,
                            const std::string& name) {
  pugi::xml_node xchild = xnode.first_child();
  while (xchild) {
    std::string child_name = xchild.name();

    // Remove NS prefix.
    size_t pos = child_name.find(':');
    if (pos != std::string::npos) {
      child_name = child_name.substr(pos + 1);
    }

    if (child_name == name) {
      return xchild;
    }

    xchild = xchild.next_sibling();
  }

  return pugi::xml_node();
}

void AppendAttr(pugi::xml_node& xnode,
                const std::string& ns,
                const std::string& name,
                const std::string& value) {
  std::string ns_name = ns + ":" + name;
  xnode.append_attribute(ns_name.c_str()) = value.c_str();
}

}  // namespace xml
}  // namespace csoap
