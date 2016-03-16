/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/CrawlerLib/PageContent.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <CrawlerLib/CrawlerLib.h>
#include <CrawlerLib/Url.h>

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/bvector.h>
#include <Bentley/RefCounted.h>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

//=======================================================================================
//! @bsiclass
// This class implements a WEB page content. A page content is composed of a Url object,
// a textual content and a series of links from this page. 
//=======================================================================================
struct PageContent : public RefCountedBase
    {
    public:

    //=======================================================================================
    // Creates a page content. Note that after create a page content has the Url and text
    // set yet the links are not set and must be added using the AddLink() method.
    //=======================================================================================
    CRAWLERLIB_EXPORT static PageContentPtr Create(UrlCR url, WString const& text);

    virtual ~PageContent() {}

    UrlCR GetUrl() const { return m_url; }
    void SetUrl(UrlCR url) { m_url = url; }

    WString const& GetText() const { return m_text; }
    void SetText(WString const& text) { m_text = text; }

    //=======================================================================================
    // Adds a link to the page content. There is no validation performed and thus an existing
    // link can be added more than once.
    //=======================================================================================
    void AddLink(UrlCR link) { m_links.push_back(&link); }
    bvector<UrlCPtr> const& GetLinks() const { return m_links; }

    private:
    PageContent(UrlCR url, WString const& text);
    PageContent() = delete;

    //Member attributes
    Url m_url;
    bvector<UrlCPtr> m_links;
    WString m_text;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
