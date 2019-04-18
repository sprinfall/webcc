#include "webcc/request_handler.h"

#include <sstream>

#include "webcc/globals.h"
#include "webcc/request.h"
#include "webcc/response.h"
#include "webcc/logger.h"

namespace webcc {

void RequestHandler::Enqueue(ConnectionPtr connection) {
  queue_.Push(connection);
}

void RequestHandler::Start(std::size_t count) {
  assert(count > 0 && workers_.size() == 0);

  for (std::size_t i = 0; i < count; ++i) {
    workers_.emplace_back(std::bind(&RequestHandler::WorkerRoutine, this));
  }
}

void RequestHandler::Stop() {
  LOG_INFO("Stopping workers...");

  // Clear pending connections.
  // The connections will be closed later (see Server::DoAwaitStop).
  LOG_INFO("Clear pending connections...");
  queue_.Clear();

  // Enqueue a null connection to trigger the first worker to stop.
  queue_.Push(ConnectionPtr());

  for (auto& worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }

  LOG_INFO("All workers have been stopped.");
}

void RequestHandler::WorkerRoutine() {
  LOG_INFO("Worker is running.");

  for (;;) {
    ConnectionPtr connection = queue_.PopOrWait();

    if (!connection) {
      LOG_INFO("Worker is going to stop.");

      // For stopping next worker.
      queue_.Push(ConnectionPtr());

      // Stop the worker.
      break;
    }

    HandleConnection(connection);
  }
}

}  // namespace webcc
