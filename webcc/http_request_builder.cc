#include "webcc/http_request_builder.h"

#include <fstream>

#include "webcc/logger.h"
#include "webcc/utility.h"
#include "webcc/zlib_wrapper.h"

namespace webcc {

// -----------------------------------------------------------------------------

// Read entire file into string.
static bool ReadFile(const std::string& path, std::string* output) {
  std::ifstream ifs{path, std::ios::binary | std::ios::ate};
  if (!ifs) {
    return false;
  }

  auto size = ifs.tellg();
  output->resize(size, '\0');
  ifs.seekg(0);
  ifs.read(&(*output)[0], size);  // TODO: Error handling

  return true;
}

// -----------------------------------------------------------------------------

HttpRequestPtr HttpRequestBuilder::operator()() {
  assert(parameters_.size() % 2 == 0);
  assert(headers_.size() % 2 == 0);

  auto request = std::make_shared<HttpRequest>(method_, url_);

  for (std::size_t i = 1; i < parameters_.size(); i += 2) {
    request->AddParameter(parameters_[i - 1], parameters_[i]);
  }

  for (std::size_t i = 1; i < headers_.size(); i += 2) {
    request->SetHeader(std::move(headers_[i - 1]), std::move(headers_[i]));
  }

  // No keep-alive?
  if (!keep_alive_) {
    request->SetHeader(http::headers::kConnection, "Close");
  }

  if (!data_.empty()) {
    SetContent(request, std::move(data_));

    // TODO: Request-level charset.
    if (json_) {
      request->SetContentType(http::media_types::kApplicationJson, "");
    }
  } else if (!files_.empty()) {
    // Another choice to generate the boundary is what Apache does, see:
    //   https://stackoverflow.com/a/5686863
    const std::string boundary = RandomUuid();

    request->SetContentType("multipart/form-data; boundary=" + boundary);

    std::string data;
    CreateFormData(&data, boundary);

    // Ingore gzip since most servers don't support it. 
    request->SetContent(std::move(data), true);
  }

  return request;
}

HttpRequestBuilder& HttpRequestBuilder::file(const std::string& name,
                                             const std::string& file_name,
                                             const std::string& file_path,
                                             const std::string& content_type) {
  std::string file_data;
  if (!ReadFile(file_path, &file_data)) {
    throw Exception(kFileIOError, "Cannot read the file.");
  }

  files_.push_back({name, file_name, std::move(file_data), content_type});

  return *this;
}

void HttpRequestBuilder::SetContent(HttpRequestPtr request,
                                    std::string&& data) {
  if (gzip_ && data.size() > kGzipThreshold) {
    std::string compressed;
    if (Compress(data, &compressed)) {
      request->SetContent(std::move(compressed), true);
      request->SetHeader(http::headers::kContentEncoding, "gzip");
      return;
    }

    LOG_WARN("Cannot compress the content data!");
  }

  request->SetContent(std::move(data), true);
}

void HttpRequestBuilder::CreateFormData(std::string* data,
                                        const std::string& boundary) {
  for (File& file : files_) {
    data->append("--" + boundary + kCRLF);

    // Content-Disposition header
    data->append("Content-Disposition: form-data");
    if (!file.name.empty()) {
      data->append("; name=\"" + file.name + "\"");
    }
    if (!file.file_name.empty()) {
      data->append("; filename=\"" + file.file_name + "\"");
    }
    data->append(kCRLF);

    // Content-Type header
    if (!file.content_type.empty()) {
      data->append("Content-Type: " + file.content_type);
      data->append(kCRLF);
    }

    data->append(kCRLF);

    // Payload
    data->append(file.file_data);

    data->append(kCRLF);
  }

  data->append("--" + boundary + "--");
  data->append(kCRLF);
}

}  // namespace webcc
