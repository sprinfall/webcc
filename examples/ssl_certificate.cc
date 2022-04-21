// examples/ssl_certificate.cc
// This example shows different ways to specify the SSL certificates for
// ClientSession to validate the peer.

#include <cassert>
#include <iostream>

#include "webcc/client_session.h"
#include "webcc/logger.h"

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: ssl_certificate <cert_file>" << std::endl;
    std::cerr << "Example:" << std::endl;
    std::cerr << "  $ ssl_certificate path/to/httpbin-org-chain.pem" << std::endl;
    return 1;
  }

  std::string cert_file = argv[1];

  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

#define USE_FILEPATH_OR_BUFFER 1
#define OVERWRITE_DEFAULT 0

#if OVERWRITE_DEFAULT
  const std::string ssl_context_key = "default";
#else
  const std::string ssl_context_key = "httpbin-org";
#endif

#if USE_FILEPATH_OR_BUFFER
  if (!webcc::ClientSession::AddSslContext(ssl_context_key, cert_file)) {
    std::cerr << "Failed to add the certificate!" << std::endl;
    return 1;
  }
#else
  std::string cert_content;
  webcc::utility::ReadFile(cert_file, &cert_content);
  auto cert_buffer = boost::asio::buffer(cert_content);
  if (!ClientSession::AddCertificate(ssl_context_key, cert_buffer)) {
    std::cerr << "Failed to add the certificate!" << std::endl;
    return 1;
  }
#endif

#if OVERWRITE_DEFAULT
  webcc::ClientSession session{ /*"default"*/ };
#else
  webcc::ClientSession session{ ssl_context_key };
#endif

  try {
    auto r = session.Send(WEBCC_GET("https://httpbin.org/get")());

    assert(r->status() == webcc::status_codes::kOK);
    assert(!r->data().empty());

  } catch (const webcc::Error& error) {
    std::cerr << error << std::endl;
    return 1;
  }

  return 0;
}
