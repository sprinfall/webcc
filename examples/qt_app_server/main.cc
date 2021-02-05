#include <thread>

#include <QApplication>

#include "examples/qt_app_server/main_window.h"

#include "webcc/logger.h"
#include "webcc/response_builder.h"
#include "webcc/server.h"
#include "webcc/view.h"

class ApiView : public webcc::View {
public:
  explicit ApiView(Context* context) : context_(context) {
  }

  webcc::ResponsePtr Handle(webcc::RequestPtr request) override {
    if (request->method() == "GET") {
      emit context_->SomeApiSignal("A GET request has been handled");
      return webcc::ResponseBuilder{}.OK().Body("Hello, World!")();
    }

    return {};
  }

private:
  Context* context_;
};

int main(int argc, char** argv) {
  QApplication app{ argc, argv };

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  Context context;

  // Create a server and bind view.
  webcc::Server server{ boost::asio::ip::tcp::v4(), 8000 };
  server.Route("/", std::make_shared<ApiView>(&context));

  // Run the server in a thread to avoid blocking the GUI.
  std::thread server_thread{ [&server]() { server.Run(); } };

  // Now show our Qt main window and execute the application loop.
  MainWindow main_window{ &context };
  main_window.resize(400, 300);
  main_window.showNormal();

  int code = app.exec();

  // Stop the server and wait for it to finish.
  server.Stop();
  server_thread.join();

  return code;
}
