/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/CrawlerLib/PageContent.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
//=======================================================================================
struct PageContent : public RefCountedBase
    {
    public:
    PageContent(Url const& p_Url, WString const& p_Text)
        : RefCountedBase(), m_Url(p_Url), m_Text(p_Text) {}
    PageContent() = delete;

    virtual ~PageContent() {}

    Url const& GetUrl() const {return m_Url;}
    void SetUrl(Url url) {m_Url = url;}

    WString const& GetText() const {return m_Text;}
    void SetText(WString const& text) {m_Text = text;}

    void AddLink(UrlPtr const& link) {m_Links.push_back(link);}
    bvector<UrlPtr> const& GetLinks() {return m_Links;}

    private:
    //Member attributes
    Url m_Url;
    bvector<UrlPtr> m_Links;
    WString m_Text;
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
