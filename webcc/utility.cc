#include "webcc/utility.h"

#include <sstream>

#include "boost/algorithm/string.hpp"
#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/uuid_io.hpp"

namespace webcc {

std::string RandomUuid() {
  boost::uuids::uuid u = boost::uuids::random_generator()();
  std::stringstream ss;
  ss << u;
  return ss.str();
}

bool SplitKV(const std::string& str, char delimiter,
             std::string* part1, std::string* part2) {
  std::size_t pos = str.find(delimiter);
  if (pos == std::string::npos) {
    return false;
  }

  *part1 = str.substr(0, pos);
  *part2 = str.substr(pos + 1);

  boost::trim(*part1);
  boost::trim(*part2);

  return true;
}

}  // namespace webcc
