#include "csoap/common.h"

namespace csoap {

////////////////////////////////////////////////////////////////////////////////

Parameter::Parameter(const std::string& key, const std::string& value)
    : key_(key) {
  value_ = LexicalCast<std::string>(value, "");
}

Parameter::Parameter(const std::string& key, int value)
    : key_(key) {
  value_ = LexicalCast<std::string>(value, "");
}

Parameter::Parameter(const std::string& key, float value)
    : key_(key) {
  value_ = LexicalCast<std::string>(value, "");
}

Parameter::Parameter(const std::string& key, bool value)
    : key_(key) {
  value_ = LexicalCast<std::string>(value, "");
}

}  // namespace csoap
