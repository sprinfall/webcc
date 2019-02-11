#include <iostream>

#include "json/json.h"

#include "webcc/rest_ssl_client.h"
#include "webcc/logger.h"

const bool kSslVerify = false;

#define PRINT_CONTENT 0


static Json::Value StringToJson(const std::string& str) {
  Json::Value json;

  Json::CharReaderBuilder builder;
  std::stringstream stream(str);
  std::string errs;
  if (!Json::parseFromStream(builder, stream, &json, &errs)) {
    std::cerr << errs << std::endl;
  }

  return json;
}

void ListPublicEvents() {
  webcc::RestSslClient client("api.github.com", "", kSslVerify);

  if (client.Get("/events")) {
#if PRINT_CONTENT
    Json::Value json = StringToJson(client.response()->content());

    // Pretty print the JSON.

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  ";

    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(json, &std::cout);

    std::cout << std::endl;
#endif  // PRINT_CONTENT
  } else {
    std::cout << webcc::DescribeError(client.error());
    if (client.timed_out()) {
      std::cout << " (timed out)";
    }
    std::cout << std::endl;
  }
}

void ListUserFollowers() {
  webcc::RestSslClient client("api.github.com", "", kSslVerify);

  if (client.Get("/users/sprinfall/followers")) {
#if PRINT_CONTENT
    Json::Value json = StringToJson(client.response()->content());

    // Pretty print the JSON.

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  ";

    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(json, &std::cout);

    std::cout << std::endl;
#endif  // PRINT_CONTENT
  } else {
    std::cout << webcc::DescribeError(client.error());
    if (client.timed_out()) {
      std::cout << " (timed out)";
    }
    std::cout << std::endl;
  }
}

int main() {
  WEBCC_LOG_INIT("", webcc::LOG_CONSOLE);

  //ListPublicEvents();

  ListUserFollowers();

  return 0;
}
