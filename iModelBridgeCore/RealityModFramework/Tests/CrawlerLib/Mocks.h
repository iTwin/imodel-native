/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CrawlerLib/Mocks.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <CrawlerLib/Url.h>
#include <CrawlerLib/PageParser.h>
#include <CrawlerLib/Downloader.h>
#include <CrawlerLib/UrlQueue.h>

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

    inline void SetDomainName(WString const& domainName) {m_DomainName = domainName;}
    inline void SetUrlWString(WString const& url) {m_Url = url;}
    inline void SetParent(UrlPtr const& parent) {m_Parent = parent;}
    inline void SetDepth(uint32_t depth) {m_Depth = depth;}
    inline void SetIsExternalPage(bool isExternal) {m_IsExternalPage = isExternal;}
    };

class DownloaderMock : public IPageDownloader
    {
    public:
    MOCK_METHOD2(DownloadPage, StatusInt(WString& buffer, UrlPtr const& p_Url));
    MOCK_METHOD1(SetUserAgent, void(WString const& agent));
    MOCK_METHOD1(SetRequestTimeoutInSeconds, void(long timeout));
    MOCK_METHOD1(SetFollowAutoRedirects, void(bool follow));
    MOCK_METHOD1(SetMaxAutoRedirectCount, void(long count));
    MOCK_METHOD1(SetMaxHttpConnectionCount, void(long count));
    MOCK_METHOD1(ValidateSslCertificates, void(bool validate));
    MOCK_METHOD1(ValidateContentType, void(bool validate));
    MOCK_METHOD1(SetListOfValidContentType, void(bvector<WString> const& types));
    };

END_BENTLEY_CRAWLERLIB_NAMESPACE
