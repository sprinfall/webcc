#ifndef WEBCC_REQUEST_HANDLER_H_
#define WEBCC_REQUEST_HANDLER_H_

#include <list>
#include <regex>
#include <thread>
#include <vector>

#include "webcc/connection.h"
#include "webcc/queue.h"
#include "webcc/view.h"

namespace webcc {

// -----------------------------------------------------------------------------

// Wrapper for regular expression URL.
class RegexUrl {
public:
  explicit RegexUrl(const std::string& url) : url_(url) {
  }

  std::regex operator()() const {
    std::regex::flag_type flags = std::regex::ECMAScript | std::regex::icase;

    return std::regex(url_, flags);
  }

private:
  std::string url_;
};

using R = RegexUrl;  // A shortcut

// -----------------------------------------------------------------------------

class RequestHandler {
public:
  explicit RequestHandler(const Path& doc_root);

  virtual ~RequestHandler() = default;

  RequestHandler(const RequestHandler&) = delete;
  RequestHandler& operator=(const RequestHandler&) = delete;

  bool Route(const std::string& url, ViewPtr view, const Strings& methods);

  bool Route(const RegexUrl& regex_url, ViewPtr view, const Strings& methods);

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
  virtual void Handle(ConnectionPtr connection);

  // Find the view by HTTP method and URL.
  ViewPtr FindView(const std::string& method, const std::string& url,
                   UrlArgs* args);

  // TODO
  bool ServeStatic(ConnectionPtr connection);

private:
  struct RouteInfo {
    std::string url;
    std::regex url_regex;
    ViewPtr view;
    Strings methods;
  };

private:
  // The directory containing the files to be served.
  Path doc_root_;

  Queue<ConnectionPtr> queue_;

  std::vector<std::thread> workers_;

  std::vector<RouteInfo> routes_;
};

}  // namespace webcc

#endif  // WEBCC_REQUEST_HANDLER_H_
