#include "webcc/utility.h"

#include <algorithm>
#include <ctime>
#include <iomanip>  // for put_time
#include <ostream>
#include <sstream>

#include "webcc/logger.h"

using tcp = boost::asio::ip::tcp;

namespace webcc {

void AdjustHostPort(std::string& host, std::string& port) {
  if (port.empty()) {
    std::size_t i = host.find_last_of(':');
    if (i != std::string::npos) {
      port = host.substr(i + 1);
      host = host.substr(0, i);
    }
  }
}

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

std::string GetHttpDateTimestamp() {
  std::time_t t = std::time(nullptr);
  std::stringstream ss;
  ss << std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S") << " GMT";
  return ss.str();
}

}  // namespace webcc
