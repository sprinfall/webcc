#include "webcc/utility.h"
#include <iostream>

using tcp = boost::asio::ip::tcp;

namespace webcc {

// Print the resolved endpoints.
// NOTE: Endpoint is one word, don't use "end point".
// TODO
void DumpEndpoints(tcp::resolver::results_type& endpoints) {
  std::cout << "Endpoints: " << endpoints.size() << std::endl;

  tcp::resolver::results_type::iterator it = endpoints.begin();
  for (; it != endpoints.end(); ++it) {
    std::cout << "  - " << it->endpoint();

    if (it->endpoint().protocol() == tcp::v4()) {
      std::cout << ", v4";
    } else if (it->endpoint().protocol() == tcp::v6()) {
      std::cout << ", v6";
    }

    std::cout << std::endl;
  }
}

}  // namespace webcc
