#include "webcc/rest_service_manager.h"
#include "gtest/gtest.h"

class TestRestService : public webcc::RestService {
 public:
  void Handle(const webcc::RestRequest& request,
              webcc::RestResponse* response) final {
    response->status = webcc::http::Status::kOK;
  }
};

TEST(RestServiceManager, URL_RegexBasic) {
  webcc::RestServiceManager service_manager;

  service_manager.AddService(std::make_shared<TestRestService>(),
                             "/instance/(\\d+)", true);

  std::vector<std::string> sub_matches;

  std::string url = "/instance/12345";
  webcc::RestServicePtr service = service_manager.GetService(url, &sub_matches);

  EXPECT_TRUE(!!service);

  EXPECT_EQ(1, sub_matches.size());
  EXPECT_EQ("12345", sub_matches[0]);

  url = "/instance/abcde";
  sub_matches.clear();
  service = service_manager.GetService(url, &sub_matches);

  EXPECT_FALSE(!!service);
}

TEST(RestServiceManager, URL_RegexMultiple) {
  webcc::RestServiceManager service_manager;

  service_manager.AddService(std::make_shared<TestRestService>(),
                             "/study/(\\d+)/series/(\\d+)/instance/(\\d+)",
                             true);

  std::vector<std::string> sub_matches;

  std::string url = "/study/1/series/2/instance/3";
  webcc::RestServicePtr service = service_manager.GetService(url, &sub_matches);

  EXPECT_TRUE(!!service);

  EXPECT_EQ(3, sub_matches.size());
  EXPECT_EQ("1", sub_matches[0]);
  EXPECT_EQ("2", sub_matches[1]);
  EXPECT_EQ("3", sub_matches[2]);

  url = "/study/a/series/b/instance/c";
  sub_matches.clear();
  service = service_manager.GetService(url, &sub_matches);

  EXPECT_FALSE(!!service);
}

TEST(RestServiceManager, URL_NonRegex) {
  webcc::RestServiceManager service_manager;

  service_manager.AddService(std::make_shared<TestRestService>(), "/instances",
                             false);

  std::vector<std::string> sub_matches;
  std::string url = "/instances";
  webcc::RestServicePtr service = service_manager.GetService(url, &sub_matches);

  EXPECT_TRUE(!!service);
  EXPECT_EQ(0, sub_matches.size());
}
