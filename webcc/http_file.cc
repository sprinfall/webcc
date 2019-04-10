#include "webcc/http_file.h"

#include "boost/filesystem/fstream.hpp"

namespace bfs = boost::filesystem;

namespace webcc {

// -----------------------------------------------------------------------------

// Read entire file into string.
static bool ReadFile(const Path& path, std::string* output) {
  bfs::ifstream ifs{path, std::ios::binary | std::ios::ate};
  if (!ifs) {
    return false;
  }

  auto size = ifs.tellg();
  output->resize(static_cast<std::size_t>(size), '\0');
  ifs.seekg(0);
  ifs.read(&(*output)[0], size);  // TODO: Error handling

  return true;
}

// -----------------------------------------------------------------------------

HttpFile::HttpFile(const Path& path, const std::string& mime_type) {
  if (!ReadFile(path, &data_)) {
    throw Exception(kFileIOError, "Cannot read the file.");
  }

  // Determine file name from file path.
  // TODO: Encoding
  file_name_ = path.filename().string();

  // Determine content type from file extension.
  if (mime_type.empty()) {
    std::string extension = path.extension().string();
    mime_type_ = http::media_types::FromExtension(extension, false);
  } else {
    mime_type_ = mime_type;
  }
}

HttpFile::HttpFile(std::string&& data, const std::string& file_name,
                   const std::string& mime_type) {
  data_ = std::move(data);

  file_name_ = file_name;

  mime_type_ = mime_type;

  // Determine content type from file extension.
  if (mime_type_.empty()) {
    std::size_t pos = file_name_.find_last_of('.');
    if (pos != std::string::npos) {
      std::string extension = file_name_.substr(pos + 1);
      mime_type_ = http::media_types::FromExtension(extension, false);
    }
  }
}

}  // namespace webcc
