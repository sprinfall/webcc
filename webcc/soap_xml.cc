#include "webcc/soap_xml.h"

namespace webcc {
namespace soap_xml {

void SplitName(const pugi::xml_node& xnode, std::string* prefix,
               std::string* name) {
  std::string full_name = xnode.name();

  size_t pos = full_name.find(':');

  if (pos != std::string::npos) {
    if (prefix != nullptr) {
      *prefix = full_name.substr(0, pos);
    }
    if (name != nullptr) {
      *name = full_name.substr(pos + 1);
    }
  } else {
    if (prefix != nullptr) {
      *prefix = "";
    }
    if (name != nullptr) {
      *name = full_name;
    }
  }
}

std::string GetPrefix(const pugi::xml_node& xnode) {
  std::string ns_prefix;
  SplitName(xnode, &ns_prefix, nullptr);
  return ns_prefix;
}

std::string GetNameNoPrefix(const pugi::xml_node& xnode) {
  std::string name;
  SplitName(xnode, nullptr, &name);
  return name;
}

pugi::xml_node AddChild(pugi::xml_node xnode,
                        const std::string& ns, const std::string& name) {
  return xnode.append_child((ns + ":" + name).c_str());
}

pugi::xml_node GetChild(const pugi::xml_node& xnode, const std::string& ns,
                        const std::string& name) {
  return xnode.child((ns + ":" + name).c_str());
}

pugi::xml_node GetChildNoNS(const pugi::xml_node& xnode,
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

void AddAttr(pugi::xml_node xnode, const std::string& ns,
             const std::string& name, const std::string& value) {
  std::string ns_name = ns + ":" + name;
  xnode.append_attribute(ns_name.c_str()) = value.c_str();
}

void AddNSAttr(pugi::xml_node xnode, const std::string& ns_name,
               const std::string& ns_url) {
  AddAttr(xnode, "xmlns", ns_name, ns_url);
}

std::string GetNSAttr(const pugi::xml_node& xnode, const std::string& ns_name) {
  std::string attr_name = "xmlns:" + ns_name;
  return xnode.attribute(attr_name.c_str()).as_string();
}

bool PrettyPrint(std::ostream& os, const std::string& xml_string,
                 const char* indent) {
  pugi::xml_document xdoc;
  if (!xdoc.load_string(xml_string.c_str())) {
    os << "Invalid XML" << std::endl;
    return false;
  }

  xdoc.print(os, indent);
  return true;
}

}  // namespace soap_xml
}  // namespace webcc
