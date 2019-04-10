#include "webcc/http_request_builder.h"

#include "webcc/base64.h"
#include "webcc/logger.h"
#include "webcc/utility.h"
#include "webcc/zlib_wrapper.h"

namespace webcc {

HttpRequestPtr HttpRequestBuilder::Build() {
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

HttpRequestBuilder& HttpRequestBuilder::File(const std::string& name,
                                             const Path& path,
                                             const std::string& mime_type) {
  assert(!name.empty());

  files_[name] = HttpFile(path, mime_type);

  return *this;
}

HttpRequestBuilder& HttpRequestBuilder::FileData(const std::string& name,
                                                 std::string&& file_data,
                                                 const std::string& file_name,
                                                 const std::string& mime_type) {
  assert(!name.empty());

  files_[name] = HttpFile(std::move(file_data), file_name, mime_type);

  return *this;
}

HttpRequestBuilder& HttpRequestBuilder::Auth(const std::string& type,
                                             const std::string& credentials) {
  headers_.push_back(http::headers::kAuthorization);
  headers_.push_back(type + " " + credentials);
  return *this;
}

HttpRequestBuilder& HttpRequestBuilder::AuthBasic(const std::string& login,
                                                  const std::string& password) {
  auto credentials = Base64Encode(login + ":" + password);
  return Auth("Basic", credentials);
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
  for (auto& pair : files_) {
    data->append("--" + boundary + kCRLF);

    // Content-Disposition header
    data->append("Content-Disposition: form-data");
    if (!pair.first.empty()) {
      data->append("; name=\"" + pair.first + "\"");
    }
    if (!pair.second.file_name().empty()) {
      data->append("; filename=\"" + pair.second.file_name() + "\"");
    }
    data->append(kCRLF);

    // Content-Type header
    if (!pair.second.mime_type().empty()) {
      data->append("Content-Type: " + pair.second.mime_type());
      data->append(kCRLF);
    }

    data->append(kCRLF);

    // Payload
    data->append(pair.second.data());

    data->append(kCRLF);
  }

  data->append("--" + boundary + "--");
  data->append(kCRLF);
}

}  // namespace webcc
