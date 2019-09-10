
#include "boost/asio/ip/tcp.hpp"


using Endpoint = boost::asio::ip::tcp::endpoint;
using Endpoints = boost::asio::ip::tcp::resolver::results_type;

void PrintEndpoint(std::ostream& ostream, const Endpoint& endpoint);

void PrintEndpoints(std::ostream& ostream, const Endpoints& endpoints);

std::string EndpointToString(const Endpoint& endpoint);



using tcp = boost::asio::ip::tcp;

void PrintEndpoint(std::ostream& ostream, const Endpoint& endpoint) {
  ostream << endpoint;
  if (endpoint.protocol() == tcp::v4()) {
    ostream << ", v4";
  } else if (endpoint.protocol() == tcp::v6()) {
    ostream << ", v6";
  }
}

void PrintEndpoints(std::ostream& ostream, const Endpoints& endpoints) {
  ostream << "Endpoints: " << endpoints.size() << std::endl;
  tcp::resolver::results_type::iterator it = endpoints.begin();
  for (; it != endpoints.end(); ++it) {
    ostream << "  - ";
    PrintEndpoint(ostream, it->endpoint());
    ostream << std::endl;
  }
}

std::string EndpointToString(const Endpoint& endpoint) {
  std::stringstream ss;
  PrintEndpoint(ss, endpoint);
  return ss.str();
}
