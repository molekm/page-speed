// Copyright 2009 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>

#include "base/scoped_ptr.h"
#include "pagespeed/core/pagespeed_input.h"
#include "pagespeed/core/resource.h"
#include "pagespeed/core/result_provider.h"
#include "pagespeed/proto/pagespeed_output.pb.h"
#include "pagespeed/rules/specify_charset_early.h"
#include "pagespeed/testing/pagespeed_test.h"

using pagespeed::rules::SpecifyCharsetEarly;
using pagespeed::PagespeedInput;
using pagespeed::Resource;
using pagespeed::Result;
using pagespeed::Results;
using pagespeed::ResultProvider;

namespace {

const int kLateThresholdBytes = 1024;


class SpecifyCharsetEarlyTest : public ::pagespeed_testing::PagespeedTest {
 protected:
  void AddTestResource(const std::string &url,
                       const std::string &header_name,
                       const std::string &header_value,
                       const std::string &body) {
    Resource* resource = new Resource;
    resource->SetResponseStatusCode(200);
    resource->SetRequestUrl(url);
    if (!header_name.empty()) {
      resource->AddResponseHeader(header_name, header_value);
    }
    resource->SetResponseBody(body);
    AddResource(resource);
  }

  void CheckNoViolations() {
    SpecifyCharsetEarly charset_rule;
    Results results;
    ResultProvider provider(charset_rule, &results);
    ASSERT_TRUE(charset_rule.AppendResults(*input(), &provider));
    ASSERT_EQ(results.results_size(), 0);
  }

  void CheckOneViolation(const std::string &url) {
    SpecifyCharsetEarly charset_rule;
    Results results;
    ResultProvider provider(charset_rule, &results);
    ASSERT_TRUE(charset_rule.AppendResults(*input(), &provider));
    ASSERT_EQ(results.results_size(), 1);
    const Result& result = results.results(0);
    ASSERT_EQ(result.savings().page_reflows_saved(), 1);
    ASSERT_EQ(result.resource_urls_size(), 1);
    ASSERT_EQ(result.resource_urls(0), url);
  }
};

TEST_F(SpecifyCharsetEarlyTest, CharsetInHeader) {
  std::string html = "Hello world";
  html.append(kLateThresholdBytes, ' ');
  AddTestResource("http://www.example.com/hello.html",
                  "Content-Type",
                  "text/html; charset=utf-8",
                  html);
  Freeze();
  CheckNoViolations();
}

TEST_F(SpecifyCharsetEarlyTest, CharsetEarlyInHtml) {
  // Use mixed case to test case-insensitive matching.
  std::string html = "<html><head><META hTtP-eQuIv=\"content-TYPE\" "
                     "conTENT=\"text/html;   charSET= utf-8\"></head><body>"
                     "Hello world"
                     "</body></html>";

  // pad spaces to make the html big
  html.append(kLateThresholdBytes, ' ');

  AddTestResource("http://www.example.com/hello.html",
                  "",
                  "",
                  html);
  Freeze();
  CheckNoViolations();
}

TEST_F(SpecifyCharsetEarlyTest, CharsetSecondInHtml) {
  std::string html = "<html><head><meta foo=\"bar\">"
                     "<meta http-equiv=\"Content-Type\" "
                     "content=\"text/html;   charset= utf-8\"></head><body>"
                     "Hello world"
                     "</body></html>";

  // pad spaces to make the html big
  html.append(kLateThresholdBytes, ' ');

  AddTestResource("http://www.example.com/hello.html",
                  "",
                  "",
                  html);
  Freeze();
  CheckNoViolations();
}

TEST_F(SpecifyCharsetEarlyTest, TwoResourcesSecondIsViolation) {
  std::string html = "<html><head><meta http-equiv=\"Content-Type\" "
                     "content=\"text/html;   charset= utf-8\"></head><body>"
                     "Hello world"
                     "</body></html>";

  // pad spaces to make the html big
  html.append(kLateThresholdBytes, ' ');

  AddTestResource("http://www.example.com/hello.html",
                  "",
                  "",
                  html);

  std::string html2 = "<html><head></head><body></body></html>";
  // pad spaces to make the html big
  html2.append(kLateThresholdBytes, ' ');

  AddTestResource("http://www.example.com/hello2.html",
                  "",
                  "",
                  html2);

  Freeze();
  CheckOneViolation("http://www.example.com/hello2.html");
}

TEST_F(SpecifyCharsetEarlyTest, NoSpaceCharsetEarlyInHtml) {
  std::string html = "<html><head><meta http-equiv=\"Content-Type\" "
                     "content=\"text/html;charset= utf-8\"></head><body>"
                     "Hello world"
                     "</body></html>";
  // pad spaces to make the html big
  html.append(kLateThresholdBytes, ' ');
  AddTestResource("http://www.example.com/hello.html",
                  "",
                  "",
                  html);
  Freeze();
  CheckNoViolations();
}

TEST_F(SpecifyCharsetEarlyTest, CharsetLateInHtml) {
  std::string html = "<html><body>"
                     "Hello world";

  // pad spaces to push the meta tag late
  html.append(kLateThresholdBytes, ' ');
  html.append("<meta http-equiv=\"Content-Type\" "
              "content=\"text/html; charset=utf-8\">");
  html.append("</body></html>");
  AddTestResource("http://www.example.com/hello.html",
                  "",
                  "",
                  html);
  Freeze();
  CheckOneViolation("http://www.example.com/hello.html");
}

TEST_F(SpecifyCharsetEarlyTest, NotHtmlContent) {
  std::string html = "Hello world";
  html.append(kLateThresholdBytes, ' ');
  AddTestResource("http://www.example.com/hello.html",
                  "Content-Type",
                  "text/javascript",
                  html);
  Freeze();
  CheckNoViolations();
}

TEST_F(SpecifyCharsetEarlyTest, MissingCharset) {
  std::string html = "<html><body>"
                     "Hello world";

  // pad spaces to push the meta tag late
  html.append(kLateThresholdBytes, ' ');
  html.append("</body></html>");

  AddTestResource("http://www.example.com/hello.html",
                  "Content-Type",
                  "text/html",
                  html);
  Freeze();
  CheckOneViolation("http://www.example.com/hello.html");
}

TEST_F(SpecifyCharsetEarlyTest, MissingContentType) {
  std::string html = "<html><body>"
                     "Hello world";

  // pad spaces to push the meta tag late
  html.append(kLateThresholdBytes, ' ');
  html.append("</body></html>");

  AddTestResource("http://www.example.com/hello.html",
                  "",
                  "",
                  html);
  Freeze();
  CheckOneViolation("http://www.example.com/hello.html");
}

TEST_F(SpecifyCharsetEarlyTest, EmptyContentType) {
  std::string html = "<html><body>"
                     "Hello world";

  // pad spaces to push the meta tag late
  html.append(kLateThresholdBytes, ' ');
  html.append("</body></html>");


  AddTestResource("http://www.example.com/hello.html",
                  "Content-Type",
                  "",
                  html);
  Freeze();
  CheckOneViolation("http://www.example.com/hello.html");
}

TEST_F(SpecifyCharsetEarlyTest, SmallHtmlMissingCharset) {
  std::string html = "<html><body>"
                     "Hello world";

  // pad spaces to push the meta tag late
  html.append(900, ' ');
  html.append("</body></html>");

  AddTestResource("http://www.example.com/hello.html",
                  "Content-Type",
                  "text/html",
                  html);
  Freeze();
  CheckNoViolations();
}


TEST_F(SpecifyCharsetEarlyTest, TwoResourcesMissingCharset) {
  std::string html1 = "<html><body>"
                     "Hello world";

  // pad spaces to push the meta tag late
  html1.append(900, ' ');
  html1.append("</body></html>");

  AddTestResource("http://www.example.com/hello.html",
                  "Content-Type",
                  "text/html",
                  html1);

  std::string html = "<html><body>"
                     "Hello world";

  // pad spaces to push the meta tag late
  html.append(kLateThresholdBytes, ' ');
  html.append("</body></html>");


  AddTestResource("http://www.example.com/hello2.html",
                  "Content-Type",
                  "",
                  html);

  Freeze();
  CheckOneViolation("http://www.example.com/hello2.html");
}

}  // namespace
