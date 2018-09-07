#include "webcc/http_server.h"

#include <csignal>
#include <utility>

#include "webcc/http_request_handler.h"
#include "webcc/logger.h"
#include "webcc/soap_service.h"
#include "webcc/utility.h"

using tcp = boost::asio::ip::tcp;

namespace webcc {

HttpServer::HttpServer(std::uint16_t port, std::size_t workers)
    : acceptor_(io_context_), signals_(io_context_), workers_(workers) {
  RegisterSignals();

  boost::system::error_code ec;
  tcp::endpoint endpoint(tcp::v4(), port);

  // Open the acceptor.
  acceptor_.open(endpoint.protocol(), ec);
  if (ec) {
    LOG_ERRO("Acceptor open error: %s", ec.message().c_str());
    return;
  }

  // Set option SO_REUSEADDR on.
  // When SO_REUSEADDR is set, multiple servers can listen on the same port.
  // This is necessary for restarting the server on the same port.
  // More details:
  // - https://stackoverflow.com/a/3233022
  // - http://www.andy-pearce.com/blog/posts/2013/Feb/so_reuseaddr-on-windows/
  acceptor_.set_option(tcp::acceptor::reuse_address(true));

  // Bind to the server address.
  acceptor_.bind(endpoint, ec);
  if (ec) {
    LOG_ERRO("Acceptor bind error: %s", ec.message().c_str());
    return;
  }

  // Start listening for connections.
  // After listen, the client is able to connect to the server even the server
  // has not started to accept the connection yet.
  acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
  if (ec) {
    LOG_ERRO("Acceptor listen error: %s", ec.message().c_str());
    return;
  }
}

void HttpServer::Run() {
  assert(GetRequestHandler() != nullptr);

  if (!acceptor_.is_open()) {
    LOG_ERRO("Server is NOT going to run.");
    return;
  }

  LOG_INFO("Server is going to run...");

  AsyncAwaitStop();

  AsyncAccept();

  // Start worker threads.
  GetRequestHandler()->Start(workers_);

  // The io_context::run() call will block until all asynchronous operations
  // have finished. While the server is running, there is always at least one
  // asynchronous operation outstanding: the asynchronous accept call waiting
  // for new incoming connections.
  io_context_.run();
}

void HttpServer::RegisterSignals() {
  signals_.add(SIGINT);  // Ctrl+C
  signals_.add(SIGTERM);

#if defined(SIGQUIT)
  signals_.add(SIGQUIT);
#endif
}

void HttpServer::AsyncAccept() {
  acceptor_.async_accept(
      [this](boost::system::error_code ec, tcp::socket socket) {
        // Check whether the server was stopped by a signal before this
        // completion handler had a chance to run.
        if (!acceptor_.is_open()) {
          return;
        }

        if (!ec) {
          LOG_INFO("Accepted a connection.");

          HttpConnectionPtr connection{
            new HttpConnection(std::move(socket), GetRequestHandler())
          };
          connection->Start();
        }

        AsyncAccept();
      });
}

void HttpServer::AsyncAwaitStop() {
  signals_.async_wait(
      [this](boost::system::error_code, int signo) {
        // The server is stopped by canceling all outstanding asynchronous
        // operations. Once all operations have finished the io_context::run()
        // call will exit.
        LOG_INFO("On signal %d, stopping the server...", signo);
        acceptor_.close();
        GetRequestHandler()->Stop();
      });
}

}  // namespace webcc
