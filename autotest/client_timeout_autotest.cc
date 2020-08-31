#include <chrono>
#include <thread>

#include "gtest/gtest.h"

#include "webcc/client_session.h"
#include "webcc/response_builder.h"
#include "webcc/server.h"

namespace {

const char* kData = "Hello, World!";

const std::uint16_t kPort = 8080;

std::shared_ptr<webcc::Server> g_server;
std::shared_ptr<std::thread> g_thread;

class HelloView : public webcc::View {
public:
  webcc::ResponsePtr Handle(webcc::RequestPtr request) override {
    if (request->method() == "GET") {
      if (request->args().size() != 1) {
        return webcc::ResponseBuilder{}.BadRequest()();
      }

      int seconds = std::stoi(request->args()[0]);
      if (seconds > 0) {
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
      }

      return webcc::ResponseBuilder{}.OK().Body(kData)();
    }

    return {};
  }
};

}  // namespace

class ClientTimeoutTest : public testing::Test {
public:
  static void SetUpTestCase() {
    g_server.reset(new webcc::Server{ boost::asio::ip::tcp::v4(), kPort });

    g_server->Route(webcc::R{ "/sleep/(\\d+)" },
                    std::make_shared<HelloView>());

    // Run the server in a separate thread.
    g_thread.reset(new std::thread{ []() { g_server->Run(); } });
  }

  static void TearDownTestCase() {
    if (g_server) {
      g_server->Stop();
    }
    if (g_thread) {
      g_thread->join();
    }
  }
};

TEST_F(ClientTimeoutTest, NoTimeout) {
  webcc::ClientSession session;

  // Default timeout is 30s.

  try {
    auto r = session.Send(webcc::RequestBuilder{}.
                          Get("http://localhost/sleep/0").Port(kPort)
                          ());

    EXPECT_EQ(webcc::Status::kOK, r->status());
    EXPECT_EQ("OK", r->reason());

    EXPECT_EQ(kData, r->data());

  } catch (const webcc::Error& error) {
    std::cerr << error << std::endl;
  }
}

TEST_F(ClientTimeoutTest, Timeout) {
  webcc::ClientSession session;

  // Change timeout to 1s.
  session.set_timeout(1);

  webcc::ResponsePtr r;
  bool timeout = false;

  try {
    r = session.Send(webcc::RequestBuilder{}.
                     Get("http://localhost/sleep/3").Port(kPort)
                     ());

  } catch (const webcc::Error& error) {
    timeout = error.timeout();
  }

  EXPECT_TRUE(!r);
  EXPECT_TRUE(timeout);
}
