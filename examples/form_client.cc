// examples/form_client.cc
// A client posting multipart form data.

#include <fstream>
#include <iostream>

#include "webcc/client_session.h"
#include "webcc/logger.h"

namespace sfs = std::filesystem;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: form_client <upload_dir> [url]" << std::endl;
    std::cout << "  (default url: http://httpbin.org/post)" << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  (Post to httpbin.org)" << std::endl;
    std::cout << "  $ form_client <webcc_root>/data/upload" << std::endl;
    std::cout << "  $ form_client <webcc_root>/data/upload "
              << "http://httpbin.org/post" << std::endl;
    std::cout << "  (Post to 'form_server' example)" << std::endl;
    std::cout << "  $ form_client <webcc_root>/data/upload "
                 "http://localhost:8080/upload"
              << std::endl;
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  const sfs::path upload_dir{ argv[1] };

  std::string url;
  if (argc == 3) {
    url = argv[2];
  } else {
    url = "http://httpbin.org/post";
  }

  if (!sfs::is_directory(upload_dir) || !sfs::exists(upload_dir)) {
    std::cerr << "Invalid upload dir!" << std::endl;
    return 1;
  }

  webcc::ClientSession session;

  try {
    auto r = session.Send(WEBCC_POST(url)
                              .FormFile("file", upload_dir / "remember.txt")
                              .FormData("json", "{}", "application/json")());

    std::cout << r->status() << std::endl;

  } catch (const webcc::Error& error) {
    std::cout << error << std::endl;
  }

  return 0;
}
