// Copyright 2010 Google Inc. All Rights Reserved.
// Author: sligocki@google.com (Shawn Ligocki)

#include "html_rewriter/apache_rewrite_driver_factory.h"

#include "html_rewriter/apr_file_system.h"
#include "html_rewriter/apr_mutex.h"
#include "html_rewriter/apr_timer.h"
#include "html_rewriter/html_parser_message_handler.h"
#include "html_rewriter/html_rewriter_config.h"
#include "html_rewriter/md5_hasher.h"
#include "html_rewriter/serf_url_async_fetcher.h"
#include "html_rewriter/serf_url_fetcher.h"
#include "net/instaweb/htmlparse/public/html_parse.h"
#include "net/instaweb/util/public/file_cache.h"
#include "third_party/apache/apr/src/include/apr_pools.h"

using html_rewriter::SerfUrlAsyncFetcher;

namespace net_instaweb {

ApacheRewriteDriverFactory::ApacheRewriteDriverFactory() {
  apr_pool_create(&pool_, NULL);
  set_filename_prefix(html_rewriter::GetFileCachePath());
  set_url_prefix(html_rewriter::GetUrlPrefix());
}

ApacheRewriteDriverFactory::~ApacheRewriteDriverFactory() {
  apr_pool_destroy(pool_);
}

FileSystem* ApacheRewriteDriverFactory::NewFileSystem() {
  return new html_rewriter::AprFileSystem(NULL);
}

Hasher* ApacheRewriteDriverFactory::NewHasher() {
  return new html_rewriter::Md5Hasher();
}

Timer* ApacheRewriteDriverFactory::NewTimer() {
  return new html_rewriter::AprTimer();
}

MessageHandler* ApacheRewriteDriverFactory::NewHtmlParseMessageHandler() {
  return new html_rewriter::HtmlParserMessageHandler();
}

CacheInterface* ApacheRewriteDriverFactory::NewCacheInterface() {
  return new FileCache(html_rewriter::GetFileCachePath(),
                       file_system(),
                       html_parse_message_handler());
}

UrlFetcher* ApacheRewriteDriverFactory::DefaultUrlFetcher() {
  SerfUrlAsyncFetcher* async_fetcher =
      reinterpret_cast<SerfUrlAsyncFetcher*>(url_async_fetcher());
  return new html_rewriter::SerfUrlFetcher(async_fetcher);
}
UrlAsyncFetcher* ApacheRewriteDriverFactory::DefaultAsyncUrlFetcher() {
  return new SerfUrlAsyncFetcher(html_rewriter::GetFetcherProxy().c_str(),
                                 pool_);
}


HtmlParse* ApacheRewriteDriverFactory::NewHtmlParse() {
  return new HtmlParse(html_parse_message_handler());
}

AbstractMutex* ApacheRewriteDriverFactory::NewMutex() {
  return new html_rewriter::AprMutex(pool_);
}

}  // namespace net_instaweb
