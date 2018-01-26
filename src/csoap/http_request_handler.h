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

  HttpRequestHandler() = default;

  bool RegisterService(SoapServicePtr soap_service);

  // Put the connection into the queue.
  void Enqueue(ConnectionPtr conn);

  // Start worker threads.
  void Start(std::size_t count);

  // Close pending connections, stop worker threads.
  void Stop();

private:
  void WorkerRoutine();

private:
  std::vector<SoapServicePtr> soap_services_;

  Queue<ConnectionPtr> queue_;
  boost::thread_group workers_;
};

}  // namespace csoap

#endif  // CSOAP_HTTP_REQUEST_HANDLER_H_
