#pragma once

#include <CrawlerLib/Url.h>

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

struct UrlMock : public Url //Skips the url validation
    {
    public:
    UrlMock() : Url() { }

    UrlMock(WString const& url)
        : Url()
        {
        m_Url = url;
        }

    virtual ~UrlMock() { }
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
