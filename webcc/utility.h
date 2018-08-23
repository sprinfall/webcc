#ifndef WEBCC_UTILITY_H_
#define WEBCC_UTILITY_H_

#include <iosfwd>
#include <string>
#include <vector>

#include "boost/asio/ip/tcp.hpp"

namespace webcc {

// Adjust buffer size according to content length.
// This is to avoid reading too many times.
void AdjustBufferSize(std::size_t content_length, std::vector<char>* buffer);

void PrintEndpoint(std::ostream& ostream,
                   const boost::asio::ip::tcp::endpoint& endpoint);

void PrintEndpoints(
    std::ostream& ostream,
    const boost::asio::ip::tcp::resolver::results_type& endpoints);

std::string EndpointToString(const boost::asio::ip::tcp::endpoint& endpoint);

}  // namespace webcc

#endif  // WEBCC_UTILITY_H_
