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
  static const std::string s_user_agent = std::string("Webcc/") + WEBCC_VERSION;
  return s_user_agent;
}

std::string HttpDate() {
  std::time_t t = std::time(nullptr);
  tm* gmt = std::gmtime(&t);

  // Either put_time() or strftime() could format the date as expected, but they
  // are both locale dependent!
  //
  //   std::stringstream ss;
  //   ss << std::put_time(gmt, "%a, %d %b %Y %H:%M:%S") << " GMT";
  //   return ss.str();
  // 
  //   char buf[26];
  //   std::strftime(buf, 26, "%a, %d %b %Y %H:%M:%S", gmt);

  static const char* const kDays[7] = { "Sun", "Mon", "Tue", "Wed",
                                        "Thu", "Fri", "Sat" };

  static const char* const kMonths[12] = { "Jan", "Feb", "Mar", "Apr",
                                           "May", "Jun", "Jul", "Aug",
                                           "Sep", "Oct", "Nov", "Dec" };

  char buf[26];

  std::snprintf(buf, 26, "%s, %.2i %s %i %.2i:%.2i:%.2i", kDays[gmt->tm_wday],
               gmt->tm_mday, kMonths[gmt->tm_mon], gmt->tm_year + 1900,
               gmt->tm_hour, gmt->tm_min, gmt->tm_sec);

  return std::string(buf) + " GMT";
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
