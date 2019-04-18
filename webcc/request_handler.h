#ifndef WEBCC_REQUEST_HANDLER_H_
#define WEBCC_REQUEST_HANDLER_H_

#include <list>
#include <thread>
#include <vector>

#include "webcc/connection.h"
#include "webcc/queue.h"

namespace webcc {

class Request;
class Response;

// The common handler for all incoming requests.
class RequestHandler {
public:
  RequestHandler() = default;
  virtual ~RequestHandler() = default;

  RequestHandler(const RequestHandler&) = delete;
  RequestHandler& operator=(const RequestHandler&) = delete;

  // Put the connection into the queue.
  void Enqueue(ConnectionPtr connection);

  // Start worker threads.
  void Start(std::size_t count);

  // Clear pending connections from the queue and stop worker threads.
  void Stop();

private:
  void WorkerRoutine();

  // Called by the worker routine.
  virtual void HandleConnection(ConnectionPtr connection) = 0;

private:
  Queue<ConnectionPtr> queue_;

  std::vector<std::thread> workers_;
};

}  // namespace webcc

#endif  // WEBCC_REQUEST_HANDLER_H_
