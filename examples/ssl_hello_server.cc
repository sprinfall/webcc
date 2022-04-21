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
  explicit HelloView(int sleep_seconds) : sleep_seconds_(sleep_seconds) {
  }

  webcc::ResponsePtr Handle(webcc::RequestPtr request) override {
    if (sleep_seconds_ > 0) {
      std::this_thread::sleep_for(std::chrono::seconds(sleep_seconds_));
    }

    if (request->method() == "GET") {
      return webcc::ResponseBuilder{}.OK().Body("Hello, World!")();
    }

    return {};
  }

private:
  int sleep_seconds_;
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
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  int workers = 1;
  int loops = 1;
  int sleep_seconds = 0;

  if (argc > 1) {
    workers = std::stoi(argv[1]);
    if (argc > 2) {
      loops = std::stoi(argv[2]);
      if (argc > 3) {
        sleep_seconds = std::stoi(argv[3]);
      }
    }
  }

  LOG_USER("Workers: %d, loops: %d, sleep: %ds", workers, loops, sleep_seconds);

  try {
    webcc::SslServer server{ boost::asio::ip::tcp::v4(), 8080 };

    // Configure SSL context
    auto& ssl_context = server.ssl_context();

    ssl_context.set_options(ssl::context::default_workarounds |
                            ssl::context::no_sslv2 |
                            ssl::context::single_dh_use);

    // Set the callback before you load the protected key.
    ssl_context.set_password_callback(MyPasswordCallback);

    ssl_context.use_certificate_chain_file("server.pem");

    // This will call MyPasswordCallback if a password is required.
    ssl_context.use_private_key_file("server.pem", ssl::context::pem);

    ssl_context.use_tmp_dh_file("dh4096.pem");
  
    auto view = std::make_shared<HelloView>(sleep_seconds);
    server.Route("/", view);
    server.Route("/hello", view);

    server.Run(workers, loops);

  } catch (const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
    return 1;
  }

  return 0;
}
