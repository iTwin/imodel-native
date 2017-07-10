/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/PerformanceTestsHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnDb.h>
#include <BeHttp/Credentials.h>
#include <WebServices/iModelHub/Client/iModelInfo.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Client/ClientInfo.h>

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGN

struct PerformanceTestSettings
    {
    private:
        int64_t m_codePostSize;
        int64_t m_codeGetSize;
        int64_t m_codeGetByIdSize;
        int64_t m_codeGetSizeSecondCall;
        int64_t m_codeGetByIdSizeSecondCall;
        int64_t m_lockPostSize;
        int64_t m_lockGetSize;
        int64_t m_lockGetByIdSize;
        int64_t m_lockGetSizeSecondCall;
        int64_t m_lockGetByIdSizeSecondCall;
        
        void ReadSettings(BeFileNameCR settingsFile);
    public:
        static PerformanceTestSettings& Instance();
        int64_t& GetCodePostRequestSize() {return m_codePostSize;};
        int64_t& GetCodeGetRequestSize() {return m_codeGetSize;};
        int64_t& GetCodeGetByIdRequestSize() {return m_codeGetByIdSize;};
        int64_t& GetCodeGetRequestSizeSecondCall() {return m_codeGetSizeSecondCall;};
        int64_t& GetCodeGetByIdRequestSizeSecondCall() {return m_codeGetByIdSizeSecondCall;};
        int64_t& GetLockPostRequestSize() {return m_lockPostSize;};
        int64_t& GetLockGetRequestSize() {return m_lockGetSize;};
        int64_t& GetLockGetByIdRequestSize() {return m_lockGetByIdSize;};
        int64_t& GetLockGetRequestSizeSecondCall() {return m_lockGetSizeSecondCall;};
        int64_t& GetLockGetByIdRequestSizeSecondCall() {return m_lockGetByIdSizeSecondCall;};
    };
