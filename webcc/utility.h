#ifndef WEBCC_UTILITY_H_
#define WEBCC_UTILITY_H_

#include <iosfwd>
#include <string>

#include "boost/asio/ip/tcp.hpp"

namespace webcc {

typedef boost::asio::ip::tcp::endpoint TcpEndpoint;
typedef boost::asio::ip::tcp::resolver::results_type TcpEndpoints;

void PrintEndpoint(std::ostream& ostream, const TcpEndpoint& endpoint);

void PrintEndpoints(std::ostream& ostream, const TcpEndpoints& endpoints);

std::string EndpointToString(const TcpEndpoint& endpoint);

// Get the timestamp for HTTP Date header field.
// E.g., Wed, 21 Oct 2015 07:28:00 GMT
// See: https://tools.ietf.org/html/rfc7231#section-7.1.1.2
std::string GetHttpDateTimestamp();

std::string RandomUuid();

}  // namespace webcc

#endif  // WEBCC_UTILITY_H_
