/*--------------------------------------------------------------------------------------+
|
|     $Source: ConnectC/CWSCCPrivate.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <DgnClientFx/DgnClientFxL10N.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/L10N.h>
#include <WebServices/Client/WSClient.h>
#include <WebServices/Client/WSRepositoryClient.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/IMS/SolrClient.h>
#include <WebServices/Connect/ImsClient.h>
#include <WebServices/ConnectC/CWSCCPublic.h>

USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BENTLEY_WEBSERVICES

//Common code to verify API reference
#define VERIFY_API \
if(NULL == apiHandle) \
    return CALLSTATUS{INVALID_PARAMETER, "Invalid apiHandle parameter.", "The api handle passed to the function is NULL."}; \
LPCWSCC api = (LPCWSCC) apiHandle;

class WSPathProvider : public IApplicationPathsProvider
    {
    private:
        BeFileName m_rootDirectory;
        BeFileName m_tempDirectory;

    protected:
        virtual BeFileNameCR _GetDocumentsDirectory() const { return m_nullPath; }
        virtual BeFileNameCR _GetTemporaryDirectory()  const { return m_tempDirectory; }
        virtual BeFileNameCR _GetCachesDirectory() const { return m_nullPath; }
        virtual BeFileNameCR _GetLocalStateDirectory() const { return m_nullPath; }
        virtual BeFileNameCR _GetAssetsRootDirectory() const { return m_rootDirectory; }
        virtual BeFileNameCR _GetMarkupSeedFilePath() const { return m_nullPath; }

    public:
        WSPathProvider()
            {
            m_rootDirectory = BeFileName(R"(D:\dev\dgndb0601dev\out\Winx64\Product\DgnClientSdk-Winx64\assets)");
            m_tempDirectory = BeFileName(R"(C:\Users\David.Jones\AppData\Local\Bentley\WSApi)");
            }
    };

class ConnectWebServicesClientC_internal
    {
    private:
        WSPathProvider m_pathProv;
        static WSLocalState m_localState;

    public:
        ConnectWebServicesClientC_internal(Utf8String authenticatedToken, uint32_t productId);
        ConnectWebServicesClientC_internal(Utf8String username, Utf8String password, uint32_t productId);
        shared_ptr<WSRepositoryClient> m_wsRepositoryClientPtr;
        shared_ptr<SolrClient> m_solrClientPtr;
    };

typedef ConnectWebServicesClientC_internal* LPCWSCC;
