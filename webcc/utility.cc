#include "webcc/utility.h"

#include <algorithm>
#include <ostream>
#include <sstream>

#include "webcc/logger.h"

using tcp = boost::asio::ip::tcp;

namespace webcc {

void AdjustBufferSize(std::size_t content_length, std::vector<char>* buffer) {
  const std::size_t kMaxTimes = 10;

  // According to test, a client never read more than 200000 bytes a time.
  // So it doesn't make sense to set any larger size, e.g., 1MB.
  const std::size_t kMaxBufferSize = 200000;

  LOG_INFO("Adjust buffer size according to content length.");

  std::size_t min_buffer_size = content_length / kMaxTimes;
  if (min_buffer_size > buffer->size()) {
    buffer->resize(std::min(min_buffer_size, kMaxBufferSize));
    LOG_INFO("Resize read buffer to %u.", buffer->size());
  } else {
    LOG_INFO("Keep the current buffer size: %u.", buffer->size());
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

}  // namespace webcc
