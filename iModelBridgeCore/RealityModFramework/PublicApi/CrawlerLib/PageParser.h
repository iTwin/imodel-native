/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/CrawlerLib/PageParser.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/Url.h>
#include <CrawlerLib/PageContent.h>

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>

#include <regex>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

class IPageParser
    {
    public:
    CRAWLERLIB_EXPORT virtual ~IPageParser() {}
    virtual PageContentPtr ParsePage(WString const& text, UrlPtr const& url) const = 0;
    virtual void SetParseLinksRelNoFollow(bool parse) = 0;
    virtual void SetParsePagesWithNoFollowMetaTag(bool parse) = 0;
    };


class PageParser : public IPageParser
    {
    public:
    CRAWLERLIB_EXPORT PageParser();
    CRAWLERLIB_EXPORT virtual ~PageParser() {}
    CRAWLERLIB_EXPORT PageContentPtr ParsePage(WString const& text, UrlPtr const& url) const override;
    CRAWLERLIB_EXPORT void SetParseLinksRelNoFollow(bool parse) override {m_ParseLinksRelNoFollow = parse;}
    CRAWLERLIB_EXPORT void SetParsePagesWithNoFollowMetaTag(bool parse) override {m_ParsePagesWithNoFollowMetaTag = parse;}

    private:
    //Helper functions
    void  AddLinksFromText(WString const& text, UrlPtr const& url, PageContentPtr content) const;

    bool m_ParseLinksRelNoFollow;
    bool m_ParsePagesWithNoFollowMetaTag;

    static const std::wregex s_LinkRegex;
    static const std::wregex s_RelNoFollowRegex;
    static const std::wregex s_NoFollowMetaTagRegex;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
