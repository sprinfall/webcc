#include "webcc/utility.h"

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>  // for put_time
#include <sstream>

#include "boost/algorithm/string.hpp"
#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/uuid_io.hpp"

#include "webcc/version.h"

namespace webcc {
namespace utility {

std::string RandomUuid() {
  boost::uuids::uuid u = boost::uuids::random_generator()();
  std::stringstream ss;
  ss << u;
  return ss.str();
}

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

bool SplitKV(const std::string& str, char delimiter, std::string* key,
             std::string* value, bool trim) {
  std::size_t pos = str.find(delimiter);
  if (pos == std::string::npos) {
    return false;
  }

  *key = str.substr(0, pos);
  *value = str.substr(pos + 1);

  if (trim) {
    boost::trim(*key);
    boost::trim(*value);
  }

  return true;
}

bool ToSize(const std::string& str, int base, std::size_t* size) {
  try {
    *size = static_cast<std::size_t>(std::stoul(str, 0, base));
  } catch (const std::exception&) {
    return false;
  }
  return true;
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
  boost::split(lines, data, boost::is_any_of("\n"));

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

}  // namespace utility
}  // namespace webcc
