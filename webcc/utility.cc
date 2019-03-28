#include "webcc/utility.h"

#include <algorithm>
#include <ctime>
#include <iomanip>  // for put_time
#include <ostream>
#include <sstream>

#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/uuid_io.hpp"

#include "webcc/logger.h"

using tcp = boost::asio::ip::tcp;

namespace webcc {

void PrintEndpoint(std::ostream& ostream, const TcpEndpoint& endpoint) {
  ostream << endpoint;
  if (endpoint.protocol() == tcp::v4()) {
    ostream << ", v4";
  } else if (endpoint.protocol() == tcp::v6()) {
    ostream << ", v6";
  }
}

void PrintEndpoints(std::ostream& ostream, const TcpEndpoints& endpoints) {
  ostream << "Endpoints: " << endpoints.size() << std::endl;
  tcp::resolver::results_type::iterator it = endpoints.begin();
  for (; it != endpoints.end(); ++it) {
    ostream << "  - ";
    PrintEndpoint(ostream, it->endpoint());
    ostream << std::endl;
  }
}

std::string EndpointToString(const TcpEndpoint& endpoint) {
  std::stringstream ss;
  PrintEndpoint(ss, endpoint);
  return ss.str();
}

std::string GetHttpDateTimestamp() {
  std::time_t t = std::time(nullptr);
  std::stringstream ss;
  ss << std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S") << " GMT";
  return ss.str();
}

std::string RandomUuid() {
  boost::uuids::uuid u = boost::uuids::random_generator()();
  std::stringstream ss;
  ss << u;
  return ss.str();
}

}  // namespace webcc
