#include "webcc/utility.h"

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>  // for put_time
#include <iostream>
#include <sstream>

#include "webcc/string.h"
#include "webcc/version.h"

namespace webcc {
namespace utility {

const std::string& UserAgent() {
  static const std::string s_user_agent = std::string("Webcc/") + WEBCC_VERSION;
  return s_user_agent;
}

std::string HttpDate() {
  std::time_t t = std::time(nullptr);
  std::tm* gmt = std::gmtime(&t);

  std::stringstream date;
  date.imbue(std::locale::classic());  // Use classic C locale
  date << std::put_time(gmt, "%a, %d %b %y %h:%m:%s") << " GMT";
  return date.str();
}

std::size_t TellSize(const fs::path& path) {
  // Flag "ate": seek to the end of stream immediately after open.
  fs::ifstream stream{ path, std::ios::binary | std::ios::ate };
  if (stream.fail()) {
    return kInvalidLength;
  }
  return static_cast<std::size_t>(stream.tellg());
}

bool ReadFile(const fs::path& path, std::string* output) {
  // Flag "ate": seek to the end of stream immediately after open.
  fs::ifstream stream{ path, std::ios::binary | std::ios::ate };
  if (stream.fail()) {
    return false;
  }

  auto size = stream.tellg();
  output->resize(static_cast<std::size_t>(size), '\0');
  stream.seekg(std::ios::beg);
  stream.read(&(*output)[0], size);
  if (stream.fail()) {
    return false;
  }
  return true;
}

void DumpByLine(const std::string& data, std::ostream& os,
                const std::string& prefix) {
  std::vector<boost::string_view> lines;
  Split(data, '\n', false, &lines);

  std::size_t size = 0;

  for (const auto& line : lines) {
    os << prefix;

    if (line.size() + size > kMaxDumpSize) {
      os.write(line.data(), kMaxDumpSize - size);
      os << "..." << std::endl;
      break;
    } else {
      os << line << std::endl;
      size += line.size();
    }
  }
}

void PrintEndpoint(std::ostream& ostream,
                   const boost::asio::ip::tcp::endpoint& endpoint) {
  ostream << endpoint;
  if (endpoint.protocol() == boost::asio::ip::tcp::v4()) {
    ostream << ", v4";
  } else if (endpoint.protocol() == boost::asio::ip::tcp::v6()) {
    ostream << ", v6";
  }
}

std::string EndpointToString(const boost::asio::ip::tcp::endpoint& endpoint) {
  std::stringstream ss;
  PrintEndpoint(ss, endpoint);
  return ss.str();
}

}  // namespace utility
}  // namespace webcc
