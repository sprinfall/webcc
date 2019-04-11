#include <iostream>

#include "boost/filesystem.hpp"

#include "webcc/http_client_session.h"
#include "webcc/logger.h"

#if (defined(WIN32) || defined(_WIN64))
// You need to set environment variable SSL_CERT_FILE properly to enable
// SSL verification.
bool kSslVerify = false;
#else
bool kSslVerify = true;
#endif

void Help(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <upload_dir>" << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << "E:/github/webcc/data/upload" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    Help(argv[0]);
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  const webcc::Path upload_dir(argv[1]);

  namespace bfs = boost::filesystem;

  if (!bfs::is_directory(upload_dir) || !bfs::exists(upload_dir)) {
    std::cerr << "Invalid upload dir!" << std::endl;
    return 1;
  }

  webcc::HttpClientSession session;

  //std::string url = "http://httpbin.org/post";
  std::string url = "http://localhost:8080/upload";

  try {
    auto r = session.PostFile(url, "file",
                              upload_dir / "remember.txt");

    //std::cout << r->content() << std::endl;

  } catch (const webcc::Exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
