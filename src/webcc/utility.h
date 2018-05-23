#ifndef WEBCC_UTILITY_H_
#define WEBCC_UTILITY_H_

#include "boost/asio/ip/tcp.hpp"

namespace webcc {

// Print the resolved endpoints.
// NOTE: Endpoint is one word, don't use "end point".
void DumpEndpoints(
    const boost::asio::ip::tcp::resolver::results_type& endpoints);

}  // namespace webcc

#endif  // WEBCC_UTILITY_H_
