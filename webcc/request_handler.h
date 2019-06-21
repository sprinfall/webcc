#ifndef WEBCC_REQUEST_HANDLER_H_
#define WEBCC_REQUEST_HANDLER_H_

#include <list>
#include <thread>
#include <vector>

#include "webcc/connection.h"
#include "webcc/queue.h"
#include "webcc/service_manager.h"

namespace webcc {

// The common handler for all incoming requests.
class RequestHandler {
public:
  explicit RequestHandler(const std::string& doc_root);

  virtual ~RequestHandler() = default;

  RequestHandler(const RequestHandler&) = delete;
  RequestHandler& operator=(const RequestHandler&) = delete;

  bool Bind(ServicePtr service, const std::string& url, bool is_regex);

  // Put the connection into the queue.
  void Enqueue(ConnectionPtr connection);

  // Start worker threads.
  void Start(std::size_t count);

  // Clear pending connections from the queue and stop worker threads.
  void Stop();

private:
  void WorkerRoutine();

  // Handle a connection (or more precisely, the request inside it).
  // Get the request from the connection, process it, prepare the response,
  // then send the response back to the client.
  // The connection will keep alive if it's a persistent connection. When next
  // request comes, this connection will be put back to the queue again.
  virtual void HandleConnection(ConnectionPtr connection);

  // TODO
  bool ServeStatic(ConnectionPtr connection);

  void SetContent(RequestPtr request, ResponsePtr response,
                  std::string&& content);

private:
  // The directory containing the files to be served.
  std::string doc_root_;

  Queue<ConnectionPtr> queue_;

  std::vector<std::thread> workers_;

  ServiceManager service_manager_;
};

}  // namespace webcc

#endif  // WEBCC_REQUEST_HANDLER_H_
