#include "webcc/ssl_server.h"

#include "webcc/ssl_connection.h"

namespace webcc {

SslServer::SslServer(boost::asio::ip::tcp protocol, std::uint16_t port,
                     const sfs::path& doc_root, ssl::context::method method)
    : Server(protocol, port, doc_root), ssl_context_(method) {
}

ConnectionPtr SslServer::NewConnection() {
  using namespace std::placeholders;

  auto view_matcher = std::bind(&Server::MatchView, this, _1, _2, _3);

  return std::make_shared<SslConnection>(io_context_, ssl_context_, &pool_,
                                         &queue_, std::move(view_matcher),
                                         buffer_size_);
}

}  // namespace webcc
