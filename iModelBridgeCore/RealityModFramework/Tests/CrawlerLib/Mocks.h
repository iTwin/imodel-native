/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CrawlerLib/Mocks.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <CrawlerLib/Url.h>
#include "../../CrawlerLib/Politeness.h"

BEGIN_BENTLEY_CRAWLERLIB_NAMESPACE

struct UrlMock : public Url //Skips the url validation
    {
    public:
    UrlMock() : Url()
        {
        m_IsExternalPage = false;
        m_Depth = 1;
        }

    UrlMock(WString const& url)
        : Url()
        {
        m_Url = url;
        m_IsExternalPage = false;
        m_Depth = 1;
        }
    virtual ~UrlMock() { }

    inline void SetUrlWString(WString const& url) {m_Url = url;}
    inline void SetParent(UrlPtr const& parent) {m_Parent = parent;}
    inline void SetDepth(uint32_t depth) {m_Depth = depth;}
    inline void SetIsExternalPage(bool isExternal) {m_IsExternalPage = isExternal;}
    };

class PolitenessMock : public IPoliteness
    {
    public:
    MOCK_METHOD1(SetMaxCrawlDelay, void(uint32_t delay));
    MOCK_METHOD1(SetUserAgent, void(UserAgent const& agent));
    MOCK_METHOD1(SetRespectRobotTxt, void(bool respect));
    MOCK_METHOD1(SetRespectRobotTxtIfDisallowRoot, void(bool respect));
    MOCK_METHOD1(CanDownloadUrl, bool(UrlCR url));
    MOCK_METHOD1(GetCrawlDelay, uint32_t(UrlCR url));
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
