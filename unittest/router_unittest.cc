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

  router.Route(webcc::R{ "/instance/(\\d+)" }, std::make_shared<MyView>());

  std::string url = "/instance/12345";
  webcc::UrlArgs args;

  webcc::ViewPtr view = router.FindView("GET", url, &args);

  ASSERT_NE(view, nullptr);

  EXPECT_EQ(args.size(), 1);
  EXPECT_EQ(args[0], "12345");

  url = "/instance/abcde";
  args.clear();
  view = router.FindView("GET", url, &args);

  EXPECT_EQ(view, nullptr);
}

TEST(RouterTest, URL_RegexMultiple) {
  webcc::Router router;

  router.Route(webcc::R{ "/study/(\\d+)/series/(\\d+)/instance/(\\d+)" },
               std::make_shared<MyView>());

  std::string url = "/study/1/series/2/instance/3";
  webcc::UrlArgs args;

  webcc::ViewPtr view = router.FindView("GET", url, &args);

  ASSERT_NE(view, nullptr);

  EXPECT_EQ(args.size(), 3);
  EXPECT_EQ(args[0], "1");
  EXPECT_EQ(args[1], "2");
  EXPECT_EQ(args[2], "3");

  url = "/study/a/series/b/instance/c";
  args.clear();
  view = router.FindView("GET", url, &args);

  EXPECT_EQ(view, nullptr);
}

TEST(RouterTest, URL_NonRegex) {
  webcc::Router router;

  router.Route("/instances", std::make_shared<MyView>());

  std::string url = "/instances";
  webcc::UrlArgs args;

  webcc::ViewPtr view = router.FindView("GET", url, &args);

  ASSERT_NE(view, nullptr);
  EXPECT_TRUE(args.empty());
}
