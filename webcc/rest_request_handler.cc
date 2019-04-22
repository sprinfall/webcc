#include "webcc/rest_request_handler.h"

#include <utility>  // for move()
#include <vector>

#include "webcc/logger.h"
#include "webcc/url.h"

#if WEBCC_ENABLE_GZIP
#include "webcc/gzip.h"
#endif

namespace webcc {

bool RestRequestHandler::Bind(RestServicePtr service, const std::string& url,
                              bool is_regex) {
  return service_manager_.AddService(service, url, is_regex);
}

void RestRequestHandler::HandleConnection(ConnectionPtr connection) {
  RequestPtr request = connection->request();

  const Url& url = request->url();

  RestRequest rest_request{ request };

  // Get service by URL path.
  std::string path = "/" + url.path();
  auto service = service_manager_.GetService(path, &rest_request.url_matches);

  if (!service) {
    LOG_WARN("No service matches the URL path: %s", url.path().c_str());
    connection->SendResponse(Status::kNotFound);
    return;
  }

  RestResponse rest_response;
  service->Handle(rest_request, &rest_response);

  auto response = std::make_shared<Response>(rest_response.status);

  if (!rest_response.content.empty()) {
    if (!rest_response.media_type.empty()) {
      response->SetContentType(rest_response.media_type, rest_response.charset);
    }
    SetContent(request, response, std::move(rest_response.content));
  }

  // Send response back to client.
  connection->SendResponse(response);
}

void RestRequestHandler::SetContent(RequestPtr request, ResponsePtr response,
                                    std::string&& content) {
#if WEBCC_ENABLE_GZIP
  // Only support gzip (no deflate) for response compression.
  if (content.size() > kGzipThreshold && request->AcceptEncodingGzip()) {
    std::string compressed;
    if (gzip::Compress(content, &compressed)) {
      response->SetHeader(headers::kContentEncoding, "gzip");
      response->SetContent(std::move(compressed), true);
      return;
    }
  }
#endif  // WEBCC_ENABLE_GZIP

  response->SetContent(std::move(content), true);
}

}  // namespace webcc
