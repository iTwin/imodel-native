/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/PageParser.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/PageContent.h>
#include <CrawlerLib/Url.h>

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>

#include <regex>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

//=======================================================================================
//! @bsiclass
// A page parser parses a HTML page provided. It generates a page content.
//=======================================================================================
struct PageParser : public RefCountedBase
    {
    public:
    CRAWLERLIB_EXPORT static PageParserPtr Create();
    CRAWLERLIB_EXPORT void SetParseLinksRelNoFollow(bool parse);
    CRAWLERLIB_EXPORT void SetParsePagesWithNoFollowMetaTag(bool parse);

    CRAWLERLIB_EXPORT PageContentPtr ParsePage(WString const& text, UrlCR url) const;
    CRAWLERLIB_EXPORT PageContentPtr GetEmptyPageContent(UrlCR url) const;

    private:
    PageParser();

    //Helper functions
    void  AddLinksFromText(WString const& text, UrlCR url, PageContentR content) const;

    bool m_ParseLinksRelNoFollow;
    bool m_ParsePagesWithNoFollowMetaTag;

    static const std::wregex s_LinkRegex;
    static const std::wregex s_RelNoFollowRegex;
    static const std::wregex s_NoFollowMetaTagRegex;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
