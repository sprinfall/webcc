#ifndef CSOAP_HTTP_REQUEST_HANDLER_H_
#define CSOAP_HTTP_REQUEST_HANDLER_H_

#include <list>
#include <vector>

#include "boost/thread/thread.hpp"

#include "csoap/connection.h"
#include "csoap/queue.h"
#include "csoap/soap_service.h"

namespace csoap {

class HttpRequest;
class HttpResponse;

// The common handler for all incoming requests.
class HttpRequestHandler {
public:
  HttpRequestHandler(const HttpRequestHandler&) = delete;
  HttpRequestHandler& operator=(const HttpRequestHandler&) = delete;

  HttpRequestHandler();

  bool RegisterService(SoapServicePtr soap_service);

  // Handle a request and produce a response.
#if 0
  void HandleRequest(const HttpRequest& http_request,
                     HttpResponse* http_response);
#endif

  // Put the connection into the queue.
  void Enqueue(ConnectionPtr conn);

  // Stop all worker threads.
  void StopWorkers();

private:
  void WorkerRoutine();

private:
  std::vector<SoapServicePtr> soap_services_;

  Queue<ConnectionPtr> queue_;
  boost::thread_group workers_;
};

}  // namespace csoap

#endif  // CSOAP_HTTP_REQUEST_HANDLER_H_
