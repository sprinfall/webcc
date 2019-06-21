#include "webcc/request_handler.h"

#include <fstream>
#include <utility>  // for move()
#include <vector>

#include "webcc/logger.h"
#include "webcc/request.h"
#include "webcc/response.h"
#include "webcc/url.h"

#if WEBCC_ENABLE_GZIP
#include "webcc/gzip.h"
#endif

namespace webcc {

RequestHandler::RequestHandler(const std::string& doc_root)
    : doc_root_(doc_root) {
}

bool RequestHandler::Bind(ServicePtr service, const std::string& url,
                          bool is_regex) {
  return service_manager_.Add(service, url, is_regex);
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

    HandleConnection(connection);
  }
}

void RequestHandler::HandleConnection(ConnectionPtr connection) {
  auto request = connection->request();

  const Url& url = request->url();
  UrlArgs args;

  LOG_INFO("Request URL path: %s", url.path().c_str());

  // Get service by URL path.
  auto service = service_manager_.Get(url.path(), &args);

  if (!service) {
    LOG_WARN("No service matches the URL path: %s", url.path().c_str());

    if (!ServeStatic(connection)) {
      connection->SendResponse(Status::kNotFound);
    }

    return;
  }

  ResponsePtr response = service->Handle(request, args);
  
  // Send response back to client.
  if (response) {
    connection->SendResponse(response);
  } else {
    connection->SendResponse(Status::kNotImplemented);
  }
}

bool RequestHandler::ServeStatic(ConnectionPtr connection) {
  auto request = connection->request();
  std::string path = request->url().path();

  // If path ends in slash (i.e. is a directory) then add "index.html".
  if (path[path.size() - 1] == '/') {
    path += "index.html";
  }

  // Determine the file extension.
  std::string extension;
  std::size_t last_slash_pos = path.find_last_of("/");
  std::size_t last_dot_pos = path.find_last_of(".");
  if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos) {
    extension = path.substr(last_dot_pos + 1);
  }

  // Open the file to send back.
  std::string full_path = doc_root_ + path;
  std::ifstream ifs(full_path.c_str(), std::ios::in | std::ios::binary);
  if (!ifs) {
    // The file doesn't exist.
    connection->SendResponse(Status::kNotFound);
    return false;
  }

  // Fill out the content to be sent to the client.
  std::string content;
  char buf[512];
  while (ifs.read(buf, sizeof(buf)).gcount() > 0) {
    content.append(buf, ifs.gcount());
  }

  auto response = std::make_shared<Response>(Status::kOK);

  if (!content.empty()) {
    response->SetContentType(media_types::FromExtension(extension), "");
    response->SetContent(std::move(content), true);
  }

  // Send response back to client.
  connection->SendResponse(response);

  return true;
}

void RequestHandler::SetContent(RequestPtr request, ResponsePtr response,
                                std::string&& content) {
#if WEBCC_ENABLE_GZIP
  // Only support gzip (no deflate) for response compression.
  if (content.size() > kGzipThreshold && request->AcceptEncodingGzip()) {
    std::string compressed;
    if (gzip::Compress(content, &compressed)) {
      response->SetHeader(headers::kContentEncoding, "gzip");
      response->SetContent(std::move(compressed), true);
      return;
    }
  }
#endif  // WEBCC_ENABLE_GZIP

  response->SetContent(std::move(content), true);
}

}  // namespace webcc
