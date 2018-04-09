#include "webcc/http_server.h"

#include <signal.h>

#if WEBCC_DEBUG_OUTPUT
#include <iostream>
#endif

#include "webcc/http_request_handler.h"
#include "webcc/soap_service.h"
#include "webcc/utility.h"

using tcp = boost::asio::ip::tcp;

namespace webcc {

HttpServer::HttpServer(unsigned short port, std::size_t workers)
    : signals_(io_context_)
    , workers_(workers)
    , timeout_seconds_(0) {

  // Register to handle the signals that indicate when the server should exit.
  // It is safe to register for the same signal multiple times in a program,
  // provided all registration for the specified signal is made through asio.
  // TODO: Verify if this works for Windows.
  signals_.add(SIGINT);
  signals_.add(SIGTERM);
#if defined(SIGQUIT)
  signals_.add(SIGQUIT);
#endif

  DoAwaitStop();

  // NOTE:
  // "reuse_addr=true" means option SO_REUSEADDR will be set.
  // For more details about SO_REUSEADDR, see:
  // https://msdn.microsoft.com/en-us/library/windows/desktop/ms740621(v=vs.85).aspx
  // http://www.andy-pearce.com/blog/posts/2013/Feb/so_reuseaddr-on-windows/
  // TODO: SO_EXCLUSIVEADDRUSE
  acceptor_.reset(new tcp::acceptor(io_context_,
                                    tcp::endpoint(tcp::v4(), port),
                                    true));  // reuse_addr

  DoAccept();
}

HttpServer::~HttpServer() {
}

void HttpServer::Run() {
  assert(GetRequestHandler() != NULL);

#if WEBCC_DEBUG_OUTPUT
  boost::thread::id thread_id = boost::this_thread::get_id();
  std::cout << "Server main thread: " << thread_id << std::endl;
#endif

  // Start worker threads.
  GetRequestHandler()->Start(workers_);

  // The io_context::run() call will block until all asynchronous operations
  // have finished. While the server is running, there is always at least one
  // asynchronous operation outstanding: the asynchronous accept call waiting
  // for new incoming connections.
  io_context_.run();
}

void HttpServer::DoAccept() {
  acceptor_->async_accept(
      [this](boost::system::error_code ec, tcp::socket socket) {
        // Check whether the server was stopped by a signal before this
        // completion handler had a chance to run.
        if (!acceptor_->is_open()) {
          return;
        }

        if (!ec) {
          HttpSessionPtr session{
            new HttpSession(std::move(socket), GetRequestHandler())
          };
          session->Start(timeout_seconds_);
        }

        DoAccept();
      });
}

void HttpServer::DoAwaitStop() {
  signals_.async_wait(
      [this](boost::system::error_code, int /*signo*/) {
        // The server is stopped by canceling all outstanding asynchronous
        // operations. Once all operations have finished the io_context::run()
        // call will exit.
        acceptor_->close();
        GetRequestHandler()->Stop();
      });
}

}  // namespace webcc
