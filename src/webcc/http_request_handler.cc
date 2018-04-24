#include "webcc/http_request_handler.h"

#include <sstream>

#include "webcc/logger.h"
#include "webcc/common.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"

namespace webcc {

void HttpRequestHandler::Enqueue(HttpSessionPtr session) {
  queue_.Push(session);
}

void HttpRequestHandler::Start(std::size_t count) {
  assert(count > 0 && workers_.size() == 0);

  for (std::size_t i = 0; i < count; ++i) {
    workers_.create_thread(std::bind(&HttpRequestHandler::WorkerRoutine, this));
  }
}

void HttpRequestHandler::Stop() {
  LOG_VERB("Stopping workers...");

  // Close pending sessions.
  for (HttpSessionPtr conn = queue_.Pop(); conn; conn = queue_.Pop()) {
    LOG_VERB("Closing pending session...");
    conn->Stop();
  }

  // Enqueue a null session to trigger the first worker to stop.
  queue_.Push(HttpSessionPtr());

  workers_.join_all();

  LOG_VERB("All workers have been stopped.");
}

void HttpRequestHandler::WorkerRoutine() {
  LOG_VERB("Worker is running.");

  for (;;) {
    HttpSessionPtr session = queue_.PopOrWait();

    if (!session) {
      LOG_VERB("Worker is going to stop.");

      // For stopping next worker.
      queue_.Push(HttpSessionPtr());

      // Stop the worker.
      break;
    }

    HandleSession(session);
  }
}

}  // namespace webcc
