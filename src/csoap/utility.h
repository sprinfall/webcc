#ifndef CSOAP_UTILITY_H_
#define CSOAP_UTILITY_H_

#include "boost/asio/ip/tcp.hpp"

namespace csoap {

// Print the resolved endpoints.
// NOTE: Endpoint is one word, don't use "end point".
void DumpEndpoints(boost::asio::ip::tcp::resolver::results_type& endpoints);

}  // namespace csoap

#endif  // CSOAP_UTILITY_H_
