#include "webcc/http_client.h"

#if 0
#include "boost/asio.hpp"
#else
#include "boost/asio/connect.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"
#endif

#include "webcc/logger.h"
#include "webcc/http_response_parser.h"
#include "webcc/http_request.h"
#include "webcc/http_response.h"

namespace webcc {

////////////////////////////////////////////////////////////////////////////////

// See https://stackoverflow.com/a/9079092
static void SetTimeout(boost::asio::ip::tcp::socket& socket,
                       int timeout_seconds) {
#if defined _WINDOWS

  int ms = timeout_seconds * 1000;

  const char* optval = reinterpret_cast<const char*>(&ms);
  std::size_t optlen = sizeof(ms);

  setsockopt(socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, optval, optlen);
  setsockopt(socket.native_handle(), SOL_SOCKET, SO_SNDTIMEO, optval, optlen);

#else  // POSIX

  // TODO: This doesn't work! Consider to control timeout in server side.

  struct timeval tv;
  tv.tv_sec = timeout_seconds;
  tv.tv_usec = 0;
  setsockopt(socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  setsockopt(socket.native_handle(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

#endif
}

////////////////////////////////////////////////////////////////////////////////

HttpClient::HttpClient()
    : timeout_seconds_(15) {
}

Error HttpClient::SendRequest(const HttpRequest& request,
                              HttpResponse* response) {
  assert(response != NULL);

  using boost::asio::ip::tcp;

  tcp::socket socket(io_context_);

  tcp::resolver resolver(io_context_);

  std::string port = request.port();
  if (port.empty()) {
    port = "80";
  }

  boost::system::error_code ec;
  // tcp::resolver::results_type
  auto endpoints = resolver.resolve(tcp::v4(), request.host(), port, ec);

  if (ec) {
    LOG_ERRO("cannot resolve host: %s, %s",
             request.host().c_str(),
             port.c_str());

    return kHostResolveError;
  }

  boost::asio::connect(socket, endpoints, ec);
  if (ec) {
    return kEndpointConnectError;
  }

  SetTimeout(socket, timeout_seconds_);

  // Send HTTP request.

  LOG_VERB("http request:\n{\n%s}", request.Dump().c_str());

  try {
    boost::asio::write(socket, request.ToBuffers());
  } catch (boost::system::system_error&) {
    return kSocketWriteError;
  }

  // Read and parse HTTP response.

  HttpResponseParser parser(response);

  // NOTE:
  // We must stop trying to read once all content has been received,
  // because some servers will block extra call to read_some().
  while (!parser.finished()) {
    // read_some() will block until one or more bytes of data has been
    // read successfully, or until an error occurs.
    std::size_t length = socket.read_some(boost::asio::buffer(buffer_), ec);

    if (length == 0 || ec) {
#if defined _WINDOWS
      if (ec.value() == WSAETIMEDOUT) {
        return kSocketTimeoutError;
      }
#endif
      return kSocketReadError;
    }

    // Parse the response piece just read.
    // If the content has been fully received, next time flag "finished_"
    // will be set.
    Error error = parser.Parse(buffer_.data(), length);

    if (error != kNoError) {
      LOG_ERRO("failed to parse http response.");
      return error;
    }
  }

  LOG_VERB("http response:\n{\n%s}", response->Dump().c_str());

  return kNoError;
}

}  // namespace webcc
