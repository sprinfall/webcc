#include "webcc/http_request_handler.h"

#include <sstream>

#include "webcc/globals.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"
#include "webcc/logger.h"

namespace webcc {

void HttpRequestHandler::Enqueue(HttpConnectionPtr connection) {
  queue_.Push(connection);
}

void HttpRequestHandler::Start(std::size_t count) {
  assert(count > 0 && workers_.size() == 0);

  for (std::size_t i = 0; i < count; ++i) {
    workers_.emplace_back(std::bind(&HttpRequestHandler::WorkerRoutine, this));
  }
}

void HttpRequestHandler::Stop() {
  LOG_INFO("Stopping workers...");

  // Close pending connections.
  for (HttpConnectionPtr conn = queue_.Pop(); conn; conn = queue_.Pop()) {
    LOG_INFO("Closing pending connection...");
    conn->Close();
  }

  // Enqueue a null connection to trigger the first worker to stop.
  queue_.Push(HttpConnectionPtr());

  for (auto& worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }

  LOG_INFO("All workers have been stopped.");
}

void HttpRequestHandler::WorkerRoutine() {
  LOG_INFO("Worker is running.");

  for (;;) {
    HttpConnectionPtr connection = queue_.PopOrWait();

    if (!connection) {
      LOG_INFO("Worker is going to stop.");

      // For stopping next worker.
      queue_.Push(HttpConnectionPtr());

      // Stop the worker.
      break;
    }

    HandleConnection(connection);
  }
}

}  // namespace webcc
