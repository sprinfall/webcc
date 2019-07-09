#include "webcc/request_handler.h"

#include <algorithm>
#include <fstream>
#include <utility>

#include "boost/algorithm/string.hpp"
#include "boost/filesystem/fstream.hpp"

#include "webcc/logger.h"
#include "webcc/request.h"
#include "webcc/response.h"
#include "webcc/url.h"
#include "webcc/utility.h"

#if WEBCC_ENABLE_GZIP
#include "webcc/gzip.h"
#endif

namespace bfs = boost::filesystem;

namespace webcc {

RequestHandler::RequestHandler(const Path& doc_root) : doc_root_(doc_root) {
}

bool RequestHandler::Route(const std::string& url, ViewPtr view,
                           const Strings& methods) {
  assert(view);

  // TODO: More error check

  routes_.push_back({ url, {}, view, methods });

  return true;
}

bool RequestHandler::Route(const RegexUrl& regex_url, ViewPtr view,
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

void RequestHandler::Handle(ConnectionPtr connection) {
  auto request = connection->request();

  const Url& url = request->url();
  UrlArgs args;

  LOG_INFO("Request URL path: %s", url.path().c_str());

  // Find view
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

ViewPtr RequestHandler::FindView(const std::string& method,
                                 const std::string& url, UrlArgs* args) {
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

bool RequestHandler::ServeStatic(ConnectionPtr connection) {
  auto request = connection->request();
  std::string path = request->url().path();

  // If path ends in slash (i.e. is a directory) then add "index.html".
  if (path[path.size() - 1] == '/') {
    path += "index.html";
  }

  Path p = doc_root_ / path;

  std::string data;
  if (!utility::ReadFile(p, &data)) {
    connection->SendResponse(Status::kNotFound);
    return false;
  }
  
  auto response = std::make_shared<Response>(Status::kOK);

  if (!data.empty()) {
    std::string extension = p.extension().string();
    response->SetContentType(media_types::FromExtension(extension), "");

    // TODO: Use FileBody instead for streaming.
    // TODO: gzip
    auto body = std::make_shared<StringBody>(std::move(data));
    response->SetBody(body, true);
  }

  // Send response back to client.
  connection->SendResponse(response);

  return true;
}

}  // namespace webcc
