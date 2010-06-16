// Copyright 2010 and onwards Google Inc.
// Author: jmarantz@google.com (Joshua Marantz)

#ifndef NET_INSTAWEB_REWRITER_PUBLIC_CSS_TAG_SCANNER_H_
#define NET_INSTAWEB_REWRITER_PUBLIC_CSS_TAG_SCANNER_H_

#include <vector>

#include "net/instaweb/htmlparse/public/html_parser_types.h"
#include "net/instaweb/htmlparse/public/html_element.h"
#include "net/instaweb/util/public/atom.h"
#include <string>

namespace net_instaweb {

class CssTagScanner {
 public:
  explicit CssTagScanner(HtmlParse* html_parse);

  // Examines an HTML element to determine if it's a CSS link,
  // extracting out the HREF and the media-type.
  bool ParseCssElement(
      HtmlElement* element, HtmlElement::Attribute** href, const char** media);

  // Scans the contents of a CSS file, looking for the pattern url(xxx).
  // If xxx is a relative URL, it absolutifies it based on the passed-in base
  // path.  If xxx is quoted with single-quotes or double-quotes, those are
  // retained and the URL inside is absolutified.
  static bool AbsolutifyUrls(const StringPiece& contents,
                             const std::string& base_url,
                             Writer* writer, MessageHandler* handler);

 private:
  Atom s_link_;
  Atom s_href_;
  Atom s_type_;
  Atom s_rel_;
  Atom s_media_;
};

}  // namespace net_instaweb

#endif  // NET_INSTAWEB_REWRITER_PUBLIC_CSS_TAG_SCANNER_H_