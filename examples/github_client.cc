#include <iostream>

#include "json/json.h"

#include "webcc/http_client_session.h"
#include "webcc/logger.h"

// -----------------------------------------------------------------------------

// Change to 1 to print response JSON.
#define PRINT_RESPONSE 0

#if (defined(WIN32) || defined(_WIN64))
// You need to set environment variable SSL_CERT_FILE properly to enable
// SSL verification.
bool kSslVerify = false;
#else
bool kSslVerify = true;
#endif

const std::size_t kBufferSize = 1500;

const std::string kUrlRoot = "https://api.github.com";

// -----------------------------------------------------------------------------
// JSON helper functions (based on cppjson).

// Parse a string to JSON object.
Json::Value StringToJson(const std::string& str) {
  Json::Value json;

  Json::CharReaderBuilder builder;
  std::stringstream stream(str);
  std::string errors;
  if (!Json::parseFromStream(builder, stream, &json, &errors)) {
    std::cerr << errors << std::endl;
  }

  return json;
}

// Print the JSON string in pretty format.
void PrettyPrintJsonString(const std::string& str) {
  Json::Value json = StringToJson(str);

  Json::StreamWriterBuilder builder;
  builder["indentation"] = "  ";

  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  writer->write(json, &std::cout);

  std::cout << std::endl;
}

// -----------------------------------------------------------------------------

#if PRINT_RESPONSE
#define PRINT_JSON_STRING(str) PrettyPrintJsonString(str)
#else
#define PRINT_JSON_STRING(str)
#endif  // PRINT_RESPONSE

// -----------------------------------------------------------------------------

// List public events.
void ListEvents(webcc::HttpClientSession& session) {
  try {
    auto r = session.Get(kUrlRoot + "/events");
    PRINT_JSON_STRING(r->content());
  } catch (const webcc::Exception& e) {
    std::cout << e.what() << std::endl;
  }
}

// List the followers of the given user.
// Example:
//   ListUserFollowers(session, "<login>")
void ListUserFollowers(webcc::HttpClientSession& session,
                       const std::string& user) {
  try {
    auto r = session.Get(kUrlRoot + "/users/" + user + "/followers");
    PRINT_JSON_STRING(r->content());
  } catch (const webcc::Exception& e) {
    std::cout << e.what() << std::endl;
  }
}

// List the followers of the current authorized user.
// Header syntax: Authorization: <type> <credentials>
// Example:
//   ListAuthUserFollowers(session, "Basic <base64 encoded login:password>")
//   ListAuthUserFollowers(session, "Token <token>")
void ListAuthUserFollowers(webcc::HttpClientSession& session,
                           const std::string& auth) {
  try {
    auto r = session.Get(kUrlRoot + "/user/followers", {},
                        { "Authorization", auth });

    PRINT_JSON_STRING(r->content());

  } catch (const webcc::Exception& e) {
    std::cout << e.what() << std::endl;
  }
}

// -----------------------------------------------------------------------------

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  webcc::HttpClientSession session;

  session.set_ssl_verify(kSslVerify);

  ListEvents(session);

  return 0;
}
