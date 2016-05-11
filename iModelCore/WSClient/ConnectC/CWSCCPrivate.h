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
#include <DgnClientFx/Utils/Http/ProxyHttpHandler.h>
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
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
USING_NAMESPACE_BENTLEY_WEBSERVICES

//Common code to verify API reference
#define VERIFY_API \
if(NULL == apiHandle) \
    return INVALID_PARAMETER; \
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
        WSPathProvider(BeFileName tempDir, BeFileName rootDir)
            {
            m_tempDirectory = tempDir;
            m_rootDirectory = rootDir;
            }
    };

class ConnectWebServicesClientC_internal
    {
    private:
        WSPathProvider                  m_pathProvider;
        static WSLocalState             m_localState;
        Utf8String                      m_lastStatusDescription;
        Utf8String                      m_lastStatusMessage;
        WSCreateObjectResponse          m_lastCreatedObjectResponse;
        WSObjectsResponse               m_lastObjectsResponse;
        shared_ptr<ProxyHttpHandler>    m_proxy;
        ConnectSignInManagerPtr         m_connectSignInManager;
        ClientInfoPtr                   m_clientInfo;

    public:
        bmap<Utf8String, shared_ptr<WSRepositoryClient>> m_repositoryClients;
        shared_ptr<SolrClient> m_solrClientPtr;

    private:
        void Initialize
            (
            BeFileName temporaryDirectory,
            BeFileName assetsRootDirectory,
            Utf8String applicationName,
            BeVersion applicationVersion,
            Utf8String applicationGUID,
            Utf8String applicationProductId
            );

    public:
        ConnectWebServicesClientC_internal
            (
            Utf8String authenticatedToken,
            BeFileName temporaryDirectory,
            BeFileName assetsRootDirectory,
            Utf8String applicationName,
            BeVersion applicationVersion,
            Utf8String applicationGUID,
            Utf8String applicationProductId,
            Utf8StringP proxyUrl = nullptr,
            Utf8StringP proxyUsername = nullptr,
            Utf8StringP proxyPassword = nullptr
            );

        ConnectWebServicesClientC_internal
            (
            Utf8String username,
            Utf8String password,
            BeFileName temporaryDirectory,
            BeFileName assetsRootDirectory,
            Utf8String applicationName,
            BeVersion applicationVersion,
            Utf8String applicationGUID,
            Utf8String applicationProductId,
            Utf8StringP proxyUrl = nullptr,
            Utf8StringP proxyUsername = nullptr,
            Utf8StringP proxyPassword = nullptr
            );

        ~ConnectWebServicesClientC_internal ();

        void CreateProxyHttpClient
            (
            Utf8String proxyUrl,
            Utf8String username = "",
            Utf8String password = ""
            );

        void CreateWSRepositoryClient
            (
            Utf8String serverUrl,
            Utf8String repositoryId
            );

        Utf8StringCR GetLastStatusMessage();
        Utf8StringCR GetLastStatusDescription();
        CharCP       GetLastCreatedObjectInstanceId ();
        void SetStatusMessage(Utf8String message);
        void SetStatusDescription(Utf8String desc);
        void SetCreatedObjectResponse (WSCreateObjectResponse response);
        void SetObjectsResponse (WSObjectsResponse response);
    };

typedef ConnectWebServicesClientC_internal* LPCWSCC;
