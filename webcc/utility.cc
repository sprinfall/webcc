#include "webcc/utility.h"

#include <sstream>

#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/uuid_io.hpp"

namespace webcc {

std::string RandomUuid() {
  boost::uuids::uuid u = boost::uuids::random_generator()();
  std::stringstream ss;
  ss << u;
  return ss.str();
}

}  // namespace webcc
