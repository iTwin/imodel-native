/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/CrawlerLib/PageParser.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/Url.h>
#include <CrawlerLib/PageContent.h>

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/RefCounted.h>

#include <regex>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

class IPageParser
    {
    public:
    CRAWLERLIB_EXPORT virtual ~IPageParser() {}
    virtual PageContentPtr ParsePage(UrlPtr const& url, WString const& text) const = 0;
    };


class PageParser : public IPageParser
    {
    public:
    CRAWLERLIB_EXPORT virtual ~PageParser() {}
    CRAWLERLIB_EXPORT PageContentPtr ParsePage(UrlPtr const& url, WString const& text) const override;

    private:
    //Helper functions
    void  AddLinksFromText(UrlPtr const& url, WString const& text, PageContentPtr content) const;

    static const std::wregex s_LinkRegex; 
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
