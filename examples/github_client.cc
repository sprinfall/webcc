#include <iostream>

#include "json/json.h"

#include "webcc/client_session.h"
#include "webcc/logger.h"

// -----------------------------------------------------------------------------

// Change to 1 to print response JSON.
#define PRINT_RESPONSE 0

#if (defined(_WIN32) || defined(_WIN64))
// You need to set environment variable SSL_CERT_FILE properly to enable
// SSL verification.
bool kSslVerify = false;
#else
bool kSslVerify = true;
#endif

const std::string kUrlRoot = "https://api.github.com";

// -----------------------------------------------------------------------------
// JSON helper functions (based on jsoncpp).

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
void ListEvents(webcc::ClientSession& session) {
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
void ListUserFollowers(webcc::ClientSession& session, const std::string& user) {
  try {
    auto r = session.Get(kUrlRoot + "/users/" + user + "/followers");

    PRINT_JSON_STRING(r->content());

  } catch (const webcc::Exception& e) {
    std::cout << e.what() << std::endl;
  }
}

// List the followers of the current authorized user using Basic authorization.
// E.g., list my own followers:
//   ListAuthUserFollowers(session, "sprinfall@gmail.com", "<MyPassword>");
void ListAuthUserFollowers(webcc::ClientSession& session,
                           const std::string& login,
                           const std::string& password) {
  try {
    auto r = session.Request(webcc::RequestBuilder{}.Get().
                             Url(kUrlRoot + "/user/followers").
                             AuthBasic(login, password)
                             ());

    PRINT_JSON_STRING(r->content());

  } catch (const webcc::Exception& e) {
    std::cout << e.what() << std::endl;
  }
}

void CreateAuthorization(webcc::ClientSession& session,
                         const std::string& login,
                         const std::string& password) {
  try {
    std::string data =
      "{\n"
      "  'note': 'Webcc test',\n"
      "  'scopes': ['public_repo', 'repo', 'repo:status', 'user']\n"
      "}";

    auto r = session.Request(webcc::RequestBuilder{}.Post().
                             Url(kUrlRoot + "/authorizations").
                             Data(std::move(data)).
                             Json(true).
                             AuthBasic(login, password)
                             ());

    std::cout << r->content() << std::endl;

  } catch (const webcc::Exception& e) {
    std::cout << e.what() << std::endl;
  }
}

// -----------------------------------------------------------------------------

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  webcc::ClientSession session;

  session.set_ssl_verify(kSslVerify);

  ListEvents(session);

  return 0;
}
