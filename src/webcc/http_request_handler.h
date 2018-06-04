#ifndef WEBCC_HTTP_REQUEST_HANDLER_H_
#define WEBCC_HTTP_REQUEST_HANDLER_H_

#include <list>
#include <vector>

#include "boost/thread/thread.hpp"

#include "webcc/http_session.h"
#include "webcc/queue.h"
#include "webcc/soap_service.h"

namespace webcc {

class HttpRequest;
class HttpResponse;

// The common handler for all incoming requests.
class HttpRequestHandler {
 public:
  HttpRequestHandler() = default;
  virtual ~HttpRequestHandler() = default;

  DELETE_COPY_AND_ASSIGN(HttpRequestHandler);

  // Put the session into the queue.
  void Enqueue(HttpSessionPtr session);

  // Start worker threads.
  void Start(std::size_t count);

  // Close pending sessions and stop worker threads.
  void Stop();

 private:
  void WorkerRoutine();

  // Called by the worker routine.
  virtual void HandleSession(HttpSessionPtr session) = 0;

  Queue<HttpSessionPtr> queue_;
  boost::thread_group workers_;
};

}  // namespace webcc

#endif  // WEBCC_HTTP_REQUEST_HANDLER_H_
