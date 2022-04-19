// examples/heartbeat_client.cc
// This example creates a client running in a thread which periodically
// communicates with the server. It behaves like a heartbeat.

// Another purpose of this example is to check if webcc::ClientSession could
// handle cancel, stop and restart operations properly without any crash. And it
// did help me find out that ClientSession::Stop() mistakenly called
// `io_context_.stop()` during the stop process.

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

#include "webcc/client_session.h"
#include "webcc/logger.h"

// Just use `examples/hello_world_server` as the server.
static const std::string kUrl = "http://localhost:8080/hello";

class Heartbeat {
public:
  Heartbeat(int connect_timeout_seconds = 5, int interval_seconds = 5)
      : connect_timeout_seconds_(connect_timeout_seconds),
        interval_seconds_(interval_seconds) {
    session_.set_connect_timeout(connect_timeout_seconds_);
  }

  ~Heartbeat() = default;

  void Start() {
    if (running_) {
      return;
    }

    session_.Start();

    std::cout << "Heartbeat started" << std::endl;

    thread_.reset(new std::thread{ &Heartbeat::Routine, this });

    running_ = true;
    stop_ = false;
  }

  void Stop() {
    if (!running_) {
      return;
    }

    std::cout << "Heartbeat stop..." << std::endl;
    stop_ = true;

    session_.Stop();

    Resume();

    thread_->join();

    std::cout << "Heartbeat stopped" << std::endl;

    service_available_ = false;
    running_ = false;
  }

  bool service_available() const {
    return service_available_;
  }

private:
  void Routine() {
    while (!stop_) {
      std::cout << "Heartbeat check begin" << std::endl;

      webcc::Error error = Request();

      if (error.code() == webcc::error_codes::kStateError) {
        std::cout << "Asio loop is not running!" << std::endl;
        break;
      }

      std::cout << "Heartbeat check end" << std::endl;

      bool available = !error.failed();

      if (available ^ service_available_) {
        std::cout << "Service available: " << available << std::endl;
        service_available_ = available;
      }

      // Wait for seconds then request again.
      WaitForSeconds();
    }
  }

  webcc::Error Request() {
    try {
      session_.Send(WEBCC_GET(kUrl)());
    } catch (const webcc::Error& error) {
      std::cerr << "Error: " << error.what() << std::endl;
      return error;
    }
    return webcc::Error{};
  }

  void WaitForSeconds() {
    std::unique_lock<std::mutex> lock{ pause_mutex_ };

    paused_ = true;
    pause_cv_.wait_for(lock, std::chrono::seconds(interval_seconds_),
                       [=]() { return !paused_; });
  }

  void Resume() {
    std::lock_guard<std::mutex> lock{ pause_mutex_ };

    if (paused_) {
      paused_ = false;
      pause_cv_.notify_one();
    }
  }

private:
  int connect_timeout_seconds_;
  int interval_seconds_;

  std::unique_ptr<std::thread> thread_;

  webcc::ClientSession session_;

  std::atomic<bool> running_ = false;
  std::atomic<bool> stop_ = false;

  bool paused_ = false;
  std::mutex pause_mutex_;
  std::condition_variable pause_cv_;

  bool service_available_ = false;
};

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  Heartbeat heartbeat;

  heartbeat.Start();
  std::getchar();
  heartbeat.Stop();

  // Enable the following code to test restart.
#if 0
  heartbeat.Start();
  std::getchar();
  heartbeat.Stop();
#endif

  return 0;
}
