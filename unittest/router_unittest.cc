#include "gtest/gtest.h"

#include "webcc/router.h"
#include "webcc/response_builder.h"

// -----------------------------------------------------------------------------

class MyView : public webcc::View {
public:
  webcc::ResponsePtr Handle(webcc::RequestPtr request) override {
    return webcc::ResponseBuilder{}.OK()();
  }
};

// -----------------------------------------------------------------------------

TEST(RouterTest, URL_RegexBasic) {
  webcc::Router router;

  router.Route(webcc::R("/instance/(\\d+)"),
               std::make_shared<MyView>());

  std::string url = "/instance/12345";
  webcc::UrlArgs args;

  webcc::ViewPtr view = router.FindView("GET", url, &args);

  EXPECT_TRUE(!!view);

  EXPECT_EQ(1, args.size());
  EXPECT_EQ("12345", args[0]);

  url = "/instance/abcde";
  args.clear();
  view = router.FindView("GET", url, &args);

  EXPECT_TRUE(!view);
}

TEST(RouterTest, URL_RegexMultiple) {
  webcc::Router router;

  router.Route(webcc::R("/study/(\\d+)/series/(\\d+)/instance/(\\d+)"),
               std::make_shared<MyView>());

  std::string url = "/study/1/series/2/instance/3";
  webcc::UrlArgs args;

  webcc::ViewPtr view = router.FindView("GET", url, &args);

  EXPECT_TRUE(!!view);

  EXPECT_EQ(3, args.size());
  EXPECT_EQ("1", args[0]);
  EXPECT_EQ("2", args[1]);
  EXPECT_EQ("3", args[2]);

  url = "/study/a/series/b/instance/c";
  args.clear();
  view = router.FindView("GET", url, &args);

  EXPECT_TRUE(!view);
}

TEST(RouterTest, URL_NonRegex) {
  webcc::Router router;

  router.Route("/instances", std::make_shared<MyView>());

  std::string url = "/instances";
  webcc::UrlArgs args;

  webcc::ViewPtr view = router.FindView("GET", url, &args);

  EXPECT_TRUE(!!view);
  EXPECT_TRUE(args.empty());
}
