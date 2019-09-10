static void Sleep(int seconds) {
  if (seconds > 0) {
    LOG_INFO("Sleep %d seconds...", seconds);
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
  }
}

std::cout << std::endl;
    std::cout << "If |seconds| is provided, the server will sleep, for testing "
              << "timeout, before " << std::endl
              << "send back each response." << std::endl;