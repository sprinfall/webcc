#include "webcc/http_request_handler.h"

#include <sstream>

#if WEBCC_DEBUG_OUTPUT
#include <iostream>
#endif

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
#if WEBCC_DEBUG_OUTPUT
    boost::thread* worker =
#endif
    workers_.create_thread(std::bind(&HttpRequestHandler::WorkerRoutine, this));

#if WEBCC_DEBUG_OUTPUT
    std::cout << "Worker is running (thread: " << worker->get_id() << ")\n";
#endif
  }
}

void HttpRequestHandler::Stop() {
#if WEBCC_DEBUG_OUTPUT
  std::cout << "Stopping workers...\n";
#endif

  // Close pending sessions.
  for (HttpSessionPtr conn = queue_.Pop(); conn; conn = queue_.Pop()) {
#if WEBCC_DEBUG_OUTPUT
    std::cout << "Closing pending session...\n";
#endif
    conn->Stop();
  }

  // Enqueue a null session to trigger the first worker to stop.
  queue_.Push(HttpSessionPtr());

  workers_.join_all();

#if WEBCC_DEBUG_OUTPUT
  std::cout << "All workers have been stopped.\n";
#endif
}

void HttpRequestHandler::WorkerRoutine() {
#if WEBCC_DEBUG_OUTPUT
  boost::thread::id thread_id = boost::this_thread::get_id();
#endif

  for (;;) {
    HttpSessionPtr session = queue_.PopOrWait();

    if (!session) {
#if WEBCC_DEBUG_OUTPUT
      std::cout << "Worker is going to stop (thread: " << thread_id << ")\n";
#endif
      // For stopping next worker.
      queue_.Push(HttpSessionPtr());

      // Stop the worker.
      break;
    }

    HandleSession(session);
  }
}

}  // namespace webcc
