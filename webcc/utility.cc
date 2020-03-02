#include "webcc/utility.h"

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>  // for put_time
#include <iostream>
#include <sstream>

#include "webcc/string.h"
#include "webcc/version.h"

namespace webcc {
namespace utility {

const std::string& UserAgent() {
  static auto s_user_agent = std::string("Webcc/") + WEBCC_VERSION;
  return s_user_agent;
}

std::string GetTimestamp() {
  std::time_t t = std::time(nullptr);
  std::stringstream ss;
  ss << std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S") << " GMT";
  return ss.str();
}

std::size_t TellSize(const std::filesystem::path& path) {
  // Flag "ate": seek to the end of stream immediately after open.
  std::ifstream stream{ path, std::ios::binary | std::ios::ate };
  if (stream.fail()) {
    return kInvalidLength;
  }
  return static_cast<std::size_t>(stream.tellg());
}

bool ReadFile(const std::filesystem::path& path, std::string* output) {
  // Flag "ate": seek to the end of stream immediately after open.
  std::ifstream stream{ path, std::ios::binary | std::ios::ate };
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
  std::vector<std::string> lines;
  split(lines, data, '\n');

  std::size_t size = 0;

  for (const std::string& line : lines) {
    os << prefix;

    if (line.size() + size > kMaxDumpSize) {
      os.write(line.c_str(), kMaxDumpSize - size);
      os << "..." << std::endl;
      break;
    } else {
      os << line << std::endl;
      size += line.size();
    }
  }
}

void PrintEndpoint(std::ostream& ostream,
                   const asio::ip::tcp::endpoint& endpoint) {
  ostream << endpoint;
  if (endpoint.protocol() == asio::ip::tcp::v4()) {
    ostream << ", v4";
  } else if (endpoint.protocol() == asio::ip::tcp::v6()) {
    ostream << ", v6";
  }
}

std::string EndpointToString(const asio::ip::tcp::endpoint& endpoint) {
  std::stringstream ss;
  PrintEndpoint(ss, endpoint);
  return ss.str();
}

}  // namespace utility
}  // namespace webcc
