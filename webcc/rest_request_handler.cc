#include "webcc/rest_request_handler.h"

#include <utility>  // for move()
#include <vector>

#include "webcc/logger.h"
#include "webcc/url.h"
#include "webcc/zlib_wrapper.h"

namespace webcc {

bool RestRequestHandler::Bind(RestServicePtr service, const std::string& url,
                              bool is_regex) {
  return service_manager_.AddService(service, url, is_regex);
}

void RestRequestHandler::HandleConnection(HttpConnectionPtr connection) {
  HttpRequestPtr http_request = connection->request();
  assert(http_request);

  const Url& url = http_request->url();

  RestRequest rest_request{
    http_request->method(), http_request->content(), url.query()
  };

  // Get service by URL path.
  std::string path = "/" + url.path();
  auto service = service_manager_.GetService(path, &rest_request.url_matches);

  if (!service) {
    LOG_WARN("No service matches the URL path: %s", url.path().c_str());
    connection->SendResponse(http::Status::kNotFound);
    return;
  }

  RestResponse rest_response;
  service->Handle(rest_request, &rest_response);

  auto http_response = std::make_shared<HttpResponse>(rest_response.status);

  if (!rest_response.content.empty()) {
    if (!rest_response.media_type.empty()) {
      http_response->SetContentType(rest_response.media_type,
                                    rest_response.charset);
    }

    // Only support gzip for response compression.
    if (rest_response.content.size() > kGzipThreshold &&
        http_request->AcceptEncodingGzip()) {
      std::string compressed;
      if (Compress(rest_response.content, &compressed)) {
        http_response->SetHeader(http::headers::kContentEncoding, "gzip");
        http_response->SetContent(std::move(compressed), true);
      }
    } else {
      http_response->SetContent(std::move(rest_response.content), true);
    }
  }

  // Send response back to client.
  connection->SendResponse(http_response);
}

}  // namespace webcc
