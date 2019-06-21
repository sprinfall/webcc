#include "gtest/gtest.h"

#include "webcc/service_manager.h"

// -----------------------------------------------------------------------------

class MyService : public webcc::Service {
public:
  webcc::ResponsePtr Handle(webcc::RequestPtr request,
                            const webcc::UrlArgs& args) override {
    return webcc::ResponseBuilder{}.OK()();
  }
};

// -----------------------------------------------------------------------------

TEST(ServiceManagerTest, URL_RegexBasic) {
  webcc::ServiceManager service_manager;

  service_manager.Add(std::make_shared<MyService>(), "/instance/(\\d+)", true);

  std::string url = "/instance/12345";
  webcc::UrlArgs args;

  webcc::ServicePtr service = service_manager.Get(url, &args);

  EXPECT_TRUE(!!service);

  EXPECT_EQ(1, args.size());
  EXPECT_EQ("12345", args[0]);

  url = "/instance/abcde";
  args.clear();
  service = service_manager.Get(url, &args);

  EXPECT_FALSE(!!service);
}

TEST(RestServiceManagerTest, URL_RegexMultiple) {
  webcc::ServiceManager service_manager;

  service_manager.Add(std::make_shared<MyService>(),
                      "/study/(\\d+)/series/(\\d+)/instance/(\\d+)", true);

  std::string url = "/study/1/series/2/instance/3";
  webcc::UrlArgs args;

  webcc::ServicePtr service = service_manager.Get(url, &args);

  EXPECT_TRUE(!!service);

  EXPECT_EQ(3, args.size());
  EXPECT_EQ("1", args[0]);
  EXPECT_EQ("2", args[1]);
  EXPECT_EQ("3", args[2]);

  url = "/study/a/series/b/instance/c";
  args.clear();
  service = service_manager.Get(url, &args);

  EXPECT_FALSE(!!service);
}

TEST(RestServiceManagerTest, URL_NonRegex) {
  webcc::ServiceManager service_manager;

  service_manager.Add(std::make_shared<MyService>(), "/instances", false);

  std::string url = "/instances";
  webcc::UrlArgs args;

  webcc::ServicePtr service = service_manager.Get(url, &args);

  EXPECT_TRUE(!!service);
  EXPECT_TRUE(args.empty());
}
