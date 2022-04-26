// examples/ssl_hello_server.cc
// A simple HTTPS server which returns "Hello, World!" to the client.
// See `ssl_hello_client` for the client program.

#include <iostream>

#include "webcc/logger.h"
#include "webcc/response_builder.h"
#include "webcc/ssl_server.h"

namespace ssl = boost::asio::ssl;

class HelloView : public webcc::View {
public:
  webcc::ResponsePtr Handle(webcc::RequestPtr request) override {
    if (request->method() == "GET") {
      return webcc::ResponseBuilder{}.OK().Body("Hello, World!")();
    }

    return {};
  }
};

// This function is called to obtain password info about an encrypted key.
// See: https://stackoverflow.com/a/13986949/6825348
std::string MyPasswordCallback(std::size_t max_length,
                               ssl::context::password_purpose purpose) {
  std::string password;
  // security warning: !! DO NOT hard-code the password here !!
  // read it from a SECURE location on your system
  password = "test";
  return password;
}

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: ssl_hello_server <cert_dir>" << std::endl;
    std::cout << "Example: " << std::endl;
    std::cout << "  $ ssl_hello_server <webcc_root>/examples/cert/"
              << std::endl;
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  std::string cert_dir = argv[1];

  try {
    webcc::SslServer server{ boost::asio::ip::tcp::v4(), 8080 };

    // Configure SSL context
    auto& ssl_context = server.ssl_context();

    ssl_context.set_options(ssl::context::default_workarounds |
                            ssl::context::no_sslv2 |
                            ssl::context::single_dh_use);

    // Set the callback before you load the protected key.
    ssl_context.set_password_callback(MyPasswordCallback);

    ssl_context.use_certificate_chain_file(cert_dir + "server.pem");

    // This will call MyPasswordCallback if a password is required.
    ssl_context.use_private_key_file(cert_dir + "server.pem",
                                     ssl::context::pem);

    ssl_context.use_tmp_dh_file(cert_dir + "dh4096.pem");
  
    auto view = std::make_shared<HelloView>();
    server.Route("/", view);
    server.Route("/hello", view);

    server.Run();

  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
