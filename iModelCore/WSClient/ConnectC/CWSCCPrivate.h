/*--------------------------------------------------------------------------------------+
|
|     $Source: ConnectC/CWSCCPrivate.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <DgnClientFx/DgnClientFxL10N.h>
#include <DgnClientFx/Utils/Http/ProxyHttpHandler.h>
#include <WebServices/Client/WSRepositoryClient.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/IMS/SolrClient.h>
#include <WebServices/ConnectC/CWSCC.h>
#include "WSLocalState.h"

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
        BeFileName m_tempDirectory;
        BeFileName m_assetDirectory;

    protected:
        virtual BeFileNameCR _GetDocumentsDirectory() const { return m_nullPath; }
        virtual BeFileNameCR _GetTemporaryDirectory()  const { return m_tempDirectory; }
        virtual BeFileNameCR _GetCachesDirectory() const { return m_nullPath; }
        virtual BeFileNameCR _GetLocalStateDirectory() const { return m_nullPath; }
        virtual BeFileNameCR _GetAssetsRootDirectory() const { return m_assetDirectory; }
        virtual BeFileNameCR _GetMarkupSeedFilePath() const { return m_nullPath; }
                
    public:
        WSPathProvider(BeFileName tempDir, BeFileName assetDir)
            {
            m_tempDirectory = tempDir;
            m_assetDirectory = assetDir;
            }
    };

class ConnectWebServicesClientC_internal
    {
    private:
        WSPathProvider                  m_pathProvider;
        static WSLocalState             s_localState;
        Utf8String                      m_lastStatusDescription;
        Utf8String                      m_lastStatusMessage;
        WSCreateObjectResponse          m_lastCreatedObjectResponse;
        WSObjectsResponse               m_lastObjectsResponse;
        ConnectSignInManagerPtr         m_connectSignInManager;
        ClientInfoPtr                   m_clientInfo;
        IHttpHandlerPtr                 m_customHandler;

    public:
        bmap<Utf8String, shared_ptr<WSRepositoryClient>> m_repositoryClients;
        bmap<Utf8String, shared_ptr<SolrClient>> m_solrClients;

    private:
        void Initialize
            (
            BeFileName temporaryDirectory,
            BeFileName assetDirectory,
            Utf8String applicationName,
            BeVersion applicationVersion,
            Utf8String applicationGUID,
            Utf8String applicationProductId
            );

    public:
        ConnectWebServicesClientC_internal
            (
            BeFileName temporaryDirectory,
            BeFileName assetDirectory,
            Utf8String applicationName,
            BeVersion applicationVersion,
            Utf8String applicationGUID,
            Utf8String applicationProductId,
            Utf8StringP proxyUrl = nullptr,
            Utf8StringP proxyUsername = nullptr,
            Utf8StringP proxyPassword = nullptr,
            IHttpHandlerPtr customHandler = nullptr
            );

        ~ConnectWebServicesClientC_internal ();

        bool AttemptLoginUsingCredentials(Credentials credentials);
        bool AttemptLoginUsingToken(SamlTokenPtr token);

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

        void CreateSolrClient
            (
            Utf8String serverUrl,
            Utf8String collection
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

/*
* Convert WSResults to CallStatus messages.
* NOTE: Used in pyApiGen tool for error message conversion.
*/
CallStatus wsresultToConnectWebServicesClientCStatus (LPCWSCC api, WSError::Id errorId, Utf8StringCR errorMessage, Utf8StringCR errorDescription);
