#include <iostream>

#include "boost/filesystem.hpp"

#include "webcc/client_session.h"
#include "webcc/logger.h"

#if (defined(_WIN32) || defined(_WIN64))
// You need to set environment variable SSL_CERT_FILE properly to enable
// SSL verification.
bool kSslVerify = false;
#else
bool kSslVerify = true;
#endif

void Help(const char* argv0) {
  std::cout << "Usage: " << argv0 << " <upload_dir> [url]" << std::endl;
  std::cout << "Default Url: http://httpbin.org/post" << std::endl;
  std::cout << "  E.g.," << std::endl;
  std::cout << "    " << argv0 << "E:/github/webcc/data/upload" << std::endl;
  std::cout << "    " << argv0
            << "E:/github/webcc/data/upload http://httpbin.org/post"
            << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    Help(argv[0]);
    return 1;
  }

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  const webcc::Path upload_dir(argv[1]);

  std::string url;
  if (argc == 3) {
    url = argv[2];
  } else {
    url = "http://httpbin.org/post";
  }

  namespace bfs = boost::filesystem;

  if (!bfs::is_directory(upload_dir) || !bfs::exists(upload_dir)) {
    std::cerr << "Invalid upload dir!" << std::endl;
    return 1;
  }

  webcc::ClientSession session;

  try {
    auto r = session.Request(webcc::RequestBuilder{}.Post().
                             Url(url).
                             File("file", upload_dir / "remember.txt").
                             Form("json", "{}", "application/json")
                             ());

    std::cout << r->status() << std::endl;

  } catch (const webcc::Error& error) {
    std::cout << error << std::endl;
  }

  return 0;
}
