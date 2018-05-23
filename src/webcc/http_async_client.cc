#include "webcc/http_async_client.h"

#if 0
#include "boost/asio.hpp"
#else
#include "boost/asio/connect.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"
#endif

#include "webcc/http_response_parser.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"

using boost::asio::ip::tcp;

namespace webcc {

HttpAsyncClient::HttpAsyncClient(boost::asio::io_context& io_context)
    : socket_(io_context) {

  resolver_.reset(new tcp::resolver(io_context));

  response_.reset(new HttpResponse());
  parser_.reset(new HttpResponseParser(response_.get()));
}

Error HttpAsyncClient::SendRequest(std::shared_ptr<HttpRequest> request,
                                   HttpResponseHandler response_handler) {
  request_ = request;

  std::string port = request->port();
  if (port.empty()) {
    port = "80";
  }

  auto handler = std::bind(&HttpAsyncClient::HandleResolve,
                           this,
                           std::placeholders::_1,
                           std::placeholders::_2);

  resolver_->async_resolve(tcp::v4(), request->host(), port, handler);

  return kNoError;
}

void HttpAsyncClient::HandleResolve(boost::system::error_code ec,
                                    tcp::resolver::results_type results) {
  if (ec) {
    std::cerr << "Resolve: " << ec.message() << std::endl;
    //  return kHostResolveError;
  } else {
    endpoints_ = results;
    DoConnect(endpoints_.begin());
  }
}

void HttpAsyncClient::DoConnect(tcp::resolver::results_type::iterator endpoint_it) {
  if (endpoint_it != endpoints_.end()) {
    socket_.async_connect(endpoint_it->endpoint(),
                          std::bind(&HttpAsyncClient::HandleConnect,
                          this,
                          std::placeholders::_1,
                          endpoint_it));
  }
}

void HttpAsyncClient::HandleConnect(boost::system::error_code ec,
                                    tcp::resolver::results_type::iterator endpoint_it) {
  if (ec) {
    // Will be here if the end point is v6.
    std::cout << "Connect error: " << ec.message() << std::endl;
    //  return kEndpointConnectError;

    socket_.close();

    // Try the next available endpoint.
    DoConnect(++endpoint_it);
  } else {
    DoWrite();
  }
}

// Send HTTP request.
void HttpAsyncClient::DoWrite() {
  boost::asio::async_write(socket_,
                           request_->ToBuffers(),
                           std::bind(&HttpAsyncClient::HandleWrite,
                           this,
                           std::placeholders::_1));
}

void HttpAsyncClient::HandleWrite(boost::system::error_code ec) {
  if (ec) {
    //return kSocketWriteError;
    return;
  }

  DoRead();
}

void HttpAsyncClient::DoRead() {
  socket_.async_read_some(boost::asio::buffer(buffer_),
                          std::bind(&HttpAsyncClient::HandleRead,
                          this,
                          std::placeholders::_1,
                          std::placeholders::_2));
}

void HttpAsyncClient::HandleRead(boost::system::error_code ec,
                                 std::size_t length) {
  if (ec || length == 0) {
    //return kSocketReadError;
    return;
  }

  // Parse the response piece just read.
  // If the content has been fully received, next time flag "finished_"
  // will be set.
  Error error = parser_->Parse(buffer_.data(), length);

  if (error != kNoError) {
    //return error;
  }

  if (parser_->finished()) {
    return;
  }

  // Read and parse HTTP response.

  // NOTE:
  // We must stop trying to read once all content has been received,
  // because some servers will block extra call to read_some().
  //while (!parser_.finished()) {
  //  size_t length = socket_.read_some(boost::asio::buffer(buffer_), ec);

    //if (length == 0 || ec) {
    //  return kSocketReadError;
    //}

    // Parse the response piece just read.
    // If the content has been fully received, next time flag "finished_"
    // will be set.
    //Error error = parser_.Parse(buffer_.data(), length);

    //if (error != kNoError) {
    //  return error;
    //}
  //}
}

}  // namespace webcc
