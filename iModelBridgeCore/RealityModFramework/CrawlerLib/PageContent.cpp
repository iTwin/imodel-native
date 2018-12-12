/*--------------------------------------------------------------------------------------+
|
|     $Source: CrawlerLib/PageContent.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <CrawlerLib/PageContent.h>


USING_NAMESPACE_BENTLEY_CRAWLERLIB

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
PageContentPtr PageContent::Create(UrlCR url, WString const& text)
    {
    return new PageContent(url, text);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    2/2016
//-------------------------------------------------------------------------------------
PageContent::PageContent(UrlCR url, WString const& text)
    : RefCountedBase(), m_url(url), m_text(text)
    {}
