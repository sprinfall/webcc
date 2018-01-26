#include "csoap/http_request_handler.h"

#include <sstream>
#if CSOAP_DEBUG_OUTPUT
#include <iostream>
#endif

#include "csoap/common.h"
#include "csoap/http_request.h"
#include "csoap/http_response.h"
#include "csoap/soap_request.h"
#include "csoap/soap_response.h"

namespace csoap {

bool HttpRequestHandler::RegisterService(SoapServicePtr soap_service) {
  assert(soap_service);

  if (std::find(soap_services_.begin(), soap_services_.end(), soap_service) !=
      soap_services_.end()) {
    return false;
  }

  soap_services_.push_back(soap_service);
  return true;
}

void HttpRequestHandler::Enqueue(ConnectionPtr conn) {
  queue_.Push(conn);
}

void HttpRequestHandler::Start(std::size_t count) {
  assert(count > 0 && workers_.size() == 0);

  for (std::size_t i = 0; i < count; ++i) {
#if CSOAP_DEBUG_OUTPUT
    boost::thread* worker =
#endif
    workers_.create_thread(std::bind(&HttpRequestHandler::WorkerRoutine, this));

#if CSOAP_DEBUG_OUTPUT
    std::cout << "Worker is running (thread: " << worker->get_id() << ")\n";
#endif
  }
}

void HttpRequestHandler::Stop() {
#if CSOAP_DEBUG_OUTPUT
  std::cout << "Stopping workers...\n";
#endif

  // Close pending connections.
  for (ConnectionPtr conn = queue_.Pop(); conn; conn = queue_.Pop()) {
#if CSOAP_DEBUG_OUTPUT
    std::cout << "Closing pending connection...\n";
#endif
    conn->Stop();
  }

  // Enqueue a null connection to trigger the first worker to stop.
  queue_.Push(ConnectionPtr());

  workers_.join_all();

#if CSOAP_DEBUG_OUTPUT
  std::cout << "All workers have been stopped.\n";
#endif
}

void HttpRequestHandler::WorkerRoutine() {
#if CSOAP_DEBUG_OUTPUT
  boost::thread::id thread_id = boost::this_thread::get_id();
#endif

  for (;;) {
    ConnectionPtr conn = queue_.PopOrWait();

    if (!conn) {
#if CSOAP_DEBUG_OUTPUT
      std::cout << "Worker is going to stop (thread: " << thread_id << ")\n";
#endif
      // For stopping next worker.
      queue_.Push(ConnectionPtr());

      // Stop the worker.
      break;
    }

    // Parse the SOAP request XML.
    SoapRequest soap_request;
    if (!soap_request.FromXml(conn->request_.content())) {
      conn->response_.set_status(HttpStatus::BAD_REQUEST);
      conn->DoWrite();
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
