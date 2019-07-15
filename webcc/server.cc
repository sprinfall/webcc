#include "webcc/server.h"

#include <algorithm>
#include <csignal>
#include <utility>

#include "boost/algorithm/string.hpp"
#include "boost/filesystem/fstream.hpp"

#include "webcc/body.h"
#include "webcc/logger.h"
#include "webcc/request.h"
#include "webcc/response.h"
#include "webcc/url.h"
#include "webcc/utility.h"

namespace bfs = boost::filesystem;

using tcp = boost::asio::ip::tcp;

namespace webcc {

Server::Server(std::uint16_t port, const Path& doc_root)
    : acceptor_(io_context_), signals_(io_context_), doc_root_(doc_root) {
  RegisterSignals();

  boost::system::error_code ec;
  tcp::endpoint endpoint(tcp::v4(), port);

  // Open the acceptor.
  acceptor_.open(endpoint.protocol(), ec);
  if (ec) {
    LOG_ERRO("Acceptor open error (%s).", ec.message().c_str());
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
    LOG_ERRO("Acceptor bind error (%s).", ec.message().c_str());
    return;
  }

  // Start listening for connections.
  // After listen, the client is able to connect to the server even the server
  // has not started to accept the connection yet.
  acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
  if (ec) {
    LOG_ERRO("Acceptor listen error (%s).", ec.message().c_str());
    return;
  }
}

bool Server::Route(const std::string& url, ViewPtr view,
                   const Strings& methods) {
  assert(view);

  // TODO: More error check

  routes_.push_back({ url, {}, view, methods });

  return true;
}

bool Server::Route(const UrlRegex& regex_url, ViewPtr view,
                   const Strings& methods) {
  assert(view);

  // TODO: More error check

  try {

    routes_.push_back({ "", regex_url(), view, methods });

  } catch (const std::regex_error& e) {
    LOG_ERRO("Not a valid regular expression: %s", e.what());
    return false;
  }

  return true;
}

void Server::Start(std::size_t workers) {
  assert(workers > 0);
  assert(worker_threads_.empty());

  if (!acceptor_.is_open()) {
    LOG_ERRO("Server is NOT going to run.");
    return;
  }

  LOG_INFO("Server is going to run...");

  DoAwaitStop();

  DoAccept();

  // Create worker threads.
  for (std::size_t i = 0; i < workers; ++i) {
    worker_threads_.emplace_back(std::bind(&Server::WorkerRoutine, this));
  }

  // Run the loop.
  // The io_context::run() call will block until all asynchronous operations
  // have finished. While the server is running, there is always at least one
  // asynchronous operation outstanding: the asynchronous accept call waiting
  // for new incoming connections.
  io_context_.run();
}

void Server::Stop() {
  // Stop listener.
  acceptor_.close();

  // Stop worker threads.
  StopWorkers();

  // Close all pending connections.
  pool_.CloseAll();
}

void Server::Enqueue(ConnectionPtr connection) {
  queue_.Push(connection);
}

void Server::RegisterSignals() {
  signals_.add(SIGINT);  // Ctrl+C
  signals_.add(SIGTERM);

#if defined(SIGQUIT)
  signals_.add(SIGQUIT);
#endif
}

void Server::DoAccept() {
  acceptor_.async_accept(
      [this](boost::system::error_code ec, tcp::socket socket) {
        // Check whether the server was stopped by a signal before this
        // completion handler had a chance to run.
        if (!acceptor_.is_open()) {
          return;
        }

        if (!ec) {
          LOG_INFO("Accepted a connection.");

          auto connection = std::make_shared<Connection>(
              std::move(socket), &pool_, this);

          pool_.Start(connection);
        }

        DoAccept();
      });
}

void Server::DoAwaitStop() {
  signals_.async_wait(
      [this](boost::system::error_code, int signo) {
        // The server is stopped by canceling all outstanding asynchronous
        // operations. Once all operations have finished the io_context::run()
        // call will exit.
        LOG_INFO("On signal %d, stopping the server...", signo);

        Stop();
      });
}

void Server::WorkerRoutine() {
  LOG_INFO("Worker is running.");

  for (;;) {
    auto connection = queue_.PopOrWait();

    if (!connection) {
      LOG_INFO("Worker is going to stop.");

      // For stopping next worker.
      queue_.Push(ConnectionPtr());

      // Stop the worker.
      break;
    }

    Handle(connection);
  }
}

void Server::StopWorkers() {
  LOG_INFO("Stopping workers...");

  // Clear pending connections.
  // The connections will be closed later (see Server::DoAwaitStop).
  LOG_INFO("Clear pending connections...");
  queue_.Clear();

  // Enqueue a null connection to trigger the first worker to stop.
  queue_.Push(ConnectionPtr());

  for (auto& t : worker_threads_) {
    if (t.joinable()) {
      t.join();
    }
  }

  LOG_INFO("All workers have been stopped.");
}

void Server::Handle(ConnectionPtr connection) {
  auto request = connection->request();

  const Url& url = request->url();
  UrlArgs args;

  LOG_INFO("Request URL path: %s", url.path().c_str());

  auto view = FindView(request->method(), url.path(), &args);

  if (!view) {
    LOG_WARN("No view matches the URL path: %s", url.path().c_str());
    if (!ServeStatic(connection)) {
      connection->SendResponse(Status::kNotFound);
    }
    return;
  }

  // Save the (regex matched) URL args to request object.
  request->set_args(args);

  // Ask the matched view to process the request.
  ResponsePtr response = view->Handle(request);

  // Send the response back.
  if (response) {
    connection->SendResponse(response);
  } else {
    connection->SendResponse(Status::kNotImplemented);
  }
}

ViewPtr Server::FindView(const std::string& method, const std::string& url,
                         UrlArgs* args) {
  assert(args != nullptr);

  for (auto& route : routes_) {
    if (std::find(route.methods.begin(), route.methods.end(), method) ==
        route.methods.end()) {
      continue;
    }

    if (route.url.empty()) {
      std::smatch match;

      if (std::regex_match(url, match, route.url_regex)) {
        // Any sub-matches?
        // Start from 1 because match[0] is the whole string itself.
        for (size_t i = 1; i < match.size(); ++i) {
          args->push_back(match[i].str());
        }

        return route.view;
      }
    } else {
      if (boost::iequals(route.url, url)) {
        return route.view;
      }
    }
  }

  return ViewPtr();
}

bool Server::ServeStatic(ConnectionPtr connection) {
  if (doc_root_.empty()) {
    LOG_INFO("The doc root was not specified.");
    return false;
  }

  auto request = connection->request();
  std::string path = request->url().path();

  // If path ends in slash (i.e. is a directory) then add "index.html".
  if (path[path.size() - 1] == '/') {
    path += "index.html";  // TODO
  }

  Path p = doc_root_ / path;

  try {
    auto body = std::make_shared<FileBody>(p);

    auto response = std::make_shared<Response>(Status::kOK);

    std::string extension = p.extension().string();
    response->SetContentType(media_types::FromExtension(extension), "");

    // NOTE: Gzip compression is not supported.
    response->SetBody(body, true);

    // Send response back to client.
    connection->SendResponse(response);

    return true;

  } catch (const Error& error) {
    LOG_ERRO("File error: %s.", error.message().c_str());
    return false;
  }
}

}  // namespace webcc
