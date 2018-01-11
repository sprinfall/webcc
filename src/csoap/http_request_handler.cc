#include "csoap/http_request_handler.h"

#include <sstream>

#include "csoap/common.h"
#include "csoap/http_request.h"
#include "csoap/http_response.h"
#include "csoap/soap_request.h"
#include "csoap/soap_response.h"
#include "csoap/soap_service.h"

namespace csoap {

#if 0
// Perform URL-decoding on a string. Returns false if the encoding was invalid.
static bool UrlDecode(const std::string& in, std::string& out) {
  out.clear();
  out.reserve(in.size());

  for (std::size_t i = 0; i < in.size(); ++i) {
    if (in[i] == '%') {
      if (i + 3 <= in.size()) {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value) {
          out += static_cast<char>(value);
          i += 2;
        } else {
          return false;
        }
      } else {
        return false;
      }
    } else if (in[i] == '+') {
      out += ' ';
    } else {
      out += in[i];
    }
  }

  return true;
}
#endif

HttpRequestHandler::HttpRequestHandler() {
}

bool HttpRequestHandler::RegisterService(SoapServicePtr soap_service) {
  assert(soap_service);

  if (std::find(soap_services_.begin(), soap_services_.end(), soap_service) !=
      soap_services_.end()) {
    return false;
  }

  soap_services_.push_back(soap_service);
  return true;
}

void HttpRequestHandler::HandleRequest(const HttpRequest& request,
                                       HttpResponse& response) {
  // Parse the SOAP request XML.
  SoapRequest soap_request;
  if (!soap_request.FromXml(request.content())) {
    // TODO: Bad request
    return;
  }

  // TEST

  SoapResponse soap_response;

  // Get service by URL.

  for (SoapServicePtr& service : soap_services_) {
    service->Handle(soap_request, &soap_response);
  }

  std::string content;
  soap_response.ToXml(&content);

  response.set_status(HttpStatus::OK);
  response.AddHeader(kContentTypeName, kTextXmlUtf8);
  response.AddHeader(kContentLengthName, std::to_string(content.length()));
  response.set_content(content);

#if 0
  // Decode URL to path.
  std::string request_path;
  if (!UrlDecode(request.uri, request_path)) {
    reply = HttpReply::StockReply(HttpReply::BAD_REQUEST);
    return;
  }

  // Request path must be absolute and not contain "..".
  if (request_path.empty() ||
      request_path[0] != '/' ||
      request_path.find("..") != std::string::npos) {
    reply = HttpReply::StockReply(HttpReply::BAD_REQUEST);
    return;
  }

  // If path ends in slash (i.e. is a directory) then add "index.html".
  if (request_path[request_path.size() - 1] == '/') {
    request_path += "index.html";
  }
#endif
}

}  // namespace csoap
