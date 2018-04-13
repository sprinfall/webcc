#include "webcc/rest_server.h"
#include "gtest/gtest.h"

using namespace webcc;

////////////////////////////////////////////////////////////////////////////////

class TestRestService : public RestService {
public:
  bool Handle(const std::string& http_method,
              const std::vector<std::string>& url_sub_matches,
              const std::string& request_content,
              std::string* response_content) override {
    return true;
  }
};

////////////////////////////////////////////////////////////////////////////////

TEST(RestServiceManager, URL_RegexBasic) {
  RestServiceManager service_manager;

  {
    RestServicePtr service = std::make_shared<TestRestService>();

    service_manager.AddService(service, "/instances/(\\d+)");
  }

  std::vector<std::string> sub_matches;

  std::string url = "/instances/12345";
  RestServicePtr service = service_manager.GetService(url, &sub_matches);

  EXPECT_TRUE(!!service);

  EXPECT_EQ(1, sub_matches.size());
  EXPECT_EQ("12345", sub_matches[0]);

  url = "/instances/abcde";
  sub_matches.clear();
  service = service_manager.GetService(url, &sub_matches);

  EXPECT_FALSE(!!service);
}
