#include "csoap/http_request_handler.h"

#include <sstream>
#if CSOAP_ENABLE_OUTPUT
#include <iostream>
#endif

#include "csoap/common.h"
#include "csoap/http_request.h"
#include "csoap/http_response.h"
#include "csoap/soap_request.h"
#include "csoap/soap_response.h"

namespace csoap {

HttpRequestHandler::HttpRequestHandler() {
  // Create worker threads.
  for (std::size_t i = 0; i < 2; ++i) {
    workers_.create_thread(std::bind(&HttpRequestHandler::WorkerRoutine, this));
  }
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

#if 0
void HttpRequestHandler::HandleRequest(const HttpRequest& http_request,
                                       HttpResponse* http_response) {
  // Parse the SOAP request XML.
  SoapRequest soap_request;
  if (!soap_request.FromXml(http_request.content())) {
    http_response->set_status(HttpStatus::BAD_REQUEST);
    return;
  }

  SoapResponse soap_response;

  // TODO: Get service by URL.

  for (SoapServicePtr& service : soap_services_) {
    service->Handle(soap_request, &soap_response);
  }

  std::string content;
  soap_response.ToXml(&content);

  http_response->set_status(HttpStatus::OK);
  http_response->SetContentType(kTextXmlUtf8);
  http_response->SetContentLength(content.length());
  http_response->set_content(std::move(content));
}
#endif

void HttpRequestHandler::Enqueue(ConnectionPtr conn) {
  queue_.Put(conn);
}

void HttpRequestHandler::StopWorkers() {
#if CSOAP_ENABLE_OUTPUT
  std::cout << "Stopping workers...\n";
#endif

  // Enqueue an null connection to trigger the first worker to stop.
  queue_.Put(ConnectionPtr());

  workers_.join_all();

#if CSOAP_ENABLE_OUTPUT
  std::cout << "Workers have been stopped.\n";
#endif
}

// TODO: How and when to exit?
void HttpRequestHandler::WorkerRoutine() {
  for (;;) {
    ConnectionPtr conn = queue_.Get();

    if (!conn) {
#if CSOAP_ENABLE_OUTPUT
      boost::thread::id thread_id = boost::this_thread::get_id();
      std::cout << "Worker is going to stop (thread: " << thread_id << ")\n";
#endif
      // For stopping next worker.
      queue_.Put(ConnectionPtr());
      // Stop the worker.
      break;
    }

    // Parse the SOAP request XML.
    SoapRequest soap_request;
    if (!soap_request.FromXml(conn->request_.content())) {
      conn->response_.set_status(HttpStatus::BAD_REQUEST);
      conn->DoWrite();  // TODO
      continue;
    }

    SoapResponse soap_response;
     
    // TODO: Get service by URL.
    for (SoapServicePtr& service : soap_services_) {
      service->Handle(soap_request, &soap_response);
    }

    std::string content;
    soap_response.ToXml(&content);

    conn->response_.set_status(HttpStatus::OK);
    conn->response_.SetContentType(kTextXmlUtf8);
    conn->response_.SetContentLength(content.length());
    conn->response_.set_content(std::move(content));

    conn->DoWrite();
  }
}

}  // namespace csoap
