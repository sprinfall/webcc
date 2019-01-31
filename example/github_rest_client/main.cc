#include <iostream>

#include "json/json.h"

#include "webcc/rest_ssl_client.h"
#include "webcc/logger.h"

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

void Test() {
  webcc::RestSslClient client("api.github.com");

  if (client.Get("/events")) {
    Json::Value json = StringToJson(client.response()->content());

    // Pretty print the JSON.

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  ";

    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(json, &std::cout);

    std::cout << std::endl;
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

  Test();

  return 0;
}
