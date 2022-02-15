#include "webcc/utility.h"

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>  // for put_time
#include <iostream>
#include <sstream>

#include "boost/algorithm/string.hpp"

#include "webcc/string.h"
#include "webcc/version.h"

using boost::asio::ip::tcp;

namespace webcc {
namespace utility {

const std::string& UserAgent() {
  static const std::string s_user_agent = std::string("Webcc/") + WEBCC_VERSION;
  return s_user_agent;
}

std::string HttpDate() {
  std::time_t t = std::time(nullptr);
  std::tm gmt = *std::gmtime(&t);

  std::ostringstream date;
  date.imbue(std::locale::classic());  // Use classic C locale
  date << std::put_time(&gmt, "%a, %d %b %Y %H:%M:%S GMT");
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

void DumpByLine(const std::string& data, std::ostream& os, string_view prefix) {
  std::vector<string_view> lines;
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

void PrintEndpoint(std::ostream& ostream, const tcp::endpoint& endpoint) {
  ostream << endpoint;
  if (endpoint.protocol() == tcp::v4()) {
    ostream << ", v4";
  } else if (endpoint.protocol() == tcp::v6()) {
    ostream << ", v6";
  }
}

std::string EndpointToString(const tcp::endpoint& endpoint) {
  std::ostringstream ss;
  PrintEndpoint(ss, endpoint);
  return ss.str();
}

fs::path TranslatePath(const std::string& utf8_url_path) {
#if (defined(_WIN32) || defined(_WIN64))
  std::wstring url_path = Utf8To16(utf8_url_path);
  std::vector<std::wstring> words;
  boost::split(words, url_path, boost::is_any_of(L"/"),
               boost::token_compress_on);
#else
  std::vector<std::string> words;
  boost::split(words, utf8_url_path, boost::is_any_of("/"),
               boost::token_compress_on);
#endif  // defined(_WIN32) || defined(_WIN64)

  fs::path path;
  for (auto& word : words) {
    // Ignore . and ..
#if (defined(_WIN32) || defined(_WIN64))
    if (word == L"." || word == L"..") {
#else
    if (word == "." || word == "..") {
#endif
      continue;
    }

    fs::path p{ word };

    // Ignore C:\\, C:, path\\sub, ...
    // parent_path() is similar to Python os.path.dirname().
    if (!p.parent_path().empty()) {
      continue;
    }

    path /= p;
  }

  return path;
}

}  // namespace utility
}  // namespace webcc
