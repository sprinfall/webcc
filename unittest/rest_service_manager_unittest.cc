#include "gtest/gtest.h"

#include "webcc/rest_service_manager.h"

// -----------------------------------------------------------------------------

class MyRestService : public webcc::RestService {
public:
  void Handle(const webcc::RestRequest& request,
              webcc::RestResponse* response) final {
    response->status = webcc::Status::kOK;
  }
};

// -----------------------------------------------------------------------------

TEST(RestServiceManagerTest, URL_RegexBasic) {
  webcc::RestServiceManager service_manager;

  service_manager.AddService(std::make_shared<MyRestService>(),
                             "/instance/(\\d+)", true);

  std::vector<std::string> matches;

  std::string url = "/instance/12345";
  webcc::RestServicePtr service = service_manager.GetService(url, &matches);

  EXPECT_TRUE(!!service);

  EXPECT_EQ(1, matches.size());
  EXPECT_EQ("12345", matches[0]);

  url = "/instance/abcde";
  matches.clear();
  service = service_manager.GetService(url, &matches);

  EXPECT_FALSE(!!service);
}

TEST(RestServiceManagerTest, URL_RegexMultiple) {
  webcc::RestServiceManager service_manager;

  service_manager.AddService(std::make_shared<MyRestService>(),
                             "/study/(\\d+)/series/(\\d+)/instance/(\\d+)",
                             true);

  std::vector<std::string> matches;

  std::string url = "/study/1/series/2/instance/3";
  webcc::RestServicePtr service = service_manager.GetService(url, &matches);

  EXPECT_TRUE(!!service);

  EXPECT_EQ(3, matches.size());
  EXPECT_EQ("1", matches[0]);
  EXPECT_EQ("2", matches[1]);
  EXPECT_EQ("3", matches[2]);

  url = "/study/a/series/b/instance/c";
  matches.clear();
  service = service_manager.GetService(url, &matches);

  EXPECT_FALSE(!!service);
}

TEST(RestServiceManagerTest, URL_NonRegex) {
  webcc::RestServiceManager service_manager;

  service_manager.AddService(std::make_shared<MyRestService>(), "/instances",
                             false);

  std::vector<std::string> matches;
  std::string url = "/instances";
  webcc::RestServicePtr service = service_manager.GetService(url, &matches);

  EXPECT_TRUE(!!service);
  EXPECT_EQ(0, matches.size());
}
