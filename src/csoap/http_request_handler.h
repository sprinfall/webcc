#ifndef CSOAP_HTTP_REQUEST_HANDLER_H_
#define CSOAP_HTTP_REQUEST_HANDLER_H_

#include <list>
#include <vector>

#include "boost/thread/thread.hpp"

#include "csoap/http_session.h"
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

  virtual ~HttpRequestHandler() {
  }

  // Put the session into the queue.
  void Enqueue(HttpSessionPtr session);

  // Start worker threads.
  void Start(std::size_t count);

  // Close pending sessions and stop worker threads.
  void Stop();

private:
  void WorkerRoutine();

  // Called by the worker routine.
  virtual HttpStatus::Enum HandleSession(HttpSessionPtr session) = 0;

private:
  Queue<HttpSessionPtr> queue_;
  boost::thread_group workers_;
};

}  // namespace csoap

#endif  // CSOAP_HTTP_REQUEST_HANDLER_H_
