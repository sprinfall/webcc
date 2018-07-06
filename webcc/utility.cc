#include "webcc/utility.h"

#include <ostream>
#include <sstream>

using tcp = boost::asio::ip::tcp;

namespace webcc {

void PrintEndpoint(std::ostream& ostream,
                   const boost::asio::ip::tcp::endpoint& endpoint) {
  ostream << endpoint;
  if (endpoint.protocol() == tcp::v4()) {
    ostream << ", v4";
  } else if (endpoint.protocol() == tcp::v6()) {
    ostream << ", v6";
  }
}

void PrintEndpoints(std::ostream& ostream,
                    const tcp::resolver::results_type& endpoints) {
  ostream << "Endpoints: " << endpoints.size() << std::endl;
  tcp::resolver::results_type::iterator it = endpoints.begin();
  for (; it != endpoints.end(); ++it) {
    ostream << "  - ";
    PrintEndpoint(ostream, it->endpoint());
    ostream << std::endl;
  }
}

std::string EndpointToString(const boost::asio::ip::tcp::endpoint& endpoint) {
  std::stringstream ss;
  PrintEndpoint(ss, endpoint);
  return ss.str();
}

}  // namespace webcc
