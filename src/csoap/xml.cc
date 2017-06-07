#include "csoap/xml.h"

namespace csoap {
namespace xml {

#ifdef CSOAP_USE_TINYXML

std::string GetNsPrefix(const TiXmlElement* xnode) {
  std::string node_name = xnode->Value();
#else
std::string GetNsPrefix(const pugi::xml_node& xnode) {
  std::string node_name = xnode.name();
#endif

  size_t pos = node_name.find(':');

  if (pos != std::string::npos) {
    return node_name.substr(0, pos);
  }

  return "";
}

#ifdef CSOAP_USE_TINYXML

TiXmlElement* AppendChild(TiXmlNode* xnode,
                          const std::string& ns,
                          const std::string& name) {
  std::string ns_name = ns + ":" + name;
  TiXmlElement* xchild = new TiXmlElement(ns_name.c_str());
  xnode->LinkEndChild(xchild);
  return xchild;
}

TiXmlElement* GetChild(TiXmlElement* xnode,
                       const std::string& ns,
                       const std::string& name) {
  return xnode->FirstChildElement((ns + ":" + name).c_str());
}

TiXmlElement* GetChildNoNS(TiXmlElement* xnode, const std::string& name) {
  TiXmlElement* xchild = xnode->FirstChildElement();
  while (xchild != NULL) {
    std::string child_name = xchild->Value();

    // Remove NS prefix.
    size_t pos = child_name.find(':');
    if (pos != std::string::npos) {
      child_name = child_name.substr(pos + 1);
    }

    if (child_name == name) {
      return xchild;
    }

    xchild = xchild->NextSiblingElement();
  }

  return NULL;
}

void AddAttr(TiXmlElement* xnode,
             const std::string& ns,
             const std::string& name,
             const std::string& value) {
  std::string ns_name = ns + ":" + name;
  xnode->SetAttribute(ns_name.c_str(), value.c_str());
}

void SetText(TiXmlElement* xnode, const std::string& text) {
  if (xnode->FirstChild() == NULL) {
    xnode->LinkEndChild(new TiXmlText(text.c_str()));
  } else {
    xnode->ReplaceChild(xnode->FirstChild(), TiXmlText(text.c_str()));
  }
}

bool PrettyPrintXml(std::ostream& os,
                    const std::string& xml_string,
                    const char* indent) {
  TiXmlDocument xdoc;
  xdoc.Parse(xml_string.c_str());

  if (xdoc.Error()) {
    os << "Invalid XML" << std::endl;
    return false;
  }

  TiXmlPrinter xprinter;
  xprinter.SetIndent(indent);
  xdoc.Accept(&xprinter);
  os << xprinter.CStr();

  return true;
}

#else

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

pugi::xml_node GetChildNoNS(pugi::xml_node& xnode, const std::string& name) {
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

void AddAttr(pugi::xml_node& xnode,
             const std::string& ns,
             const std::string& name,
             const std::string& value) {
  std::string ns_name = ns + ":" + name;
  xnode.append_attribute(ns_name.c_str()) = value.c_str();
}

bool PrettyPrintXml(std::ostream& os,
                    const std::string& xml_string,
                    const char* indent) {
  pugi::xml_document xdoc;
  if (!xdoc.load_string(xml_string.c_str())) {
    os << "Invalid XML" << std::endl;
    return false;
  }

  xdoc.print(os, indent);
  return true;
}

#endif

}  // namespace xml
}  // namespace csoap
