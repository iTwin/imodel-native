/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Connect/ConnectSpaces.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! LEGACY CODE - CONSIDER REVIEWING !!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <MobileDgn/MobileDgnCommon.h>
#include <MobileDgn/Utils/Http/Credentials.h>
#include <MobileDgn/Utils/Http/HttpClient.h>
#include <MobileDgn/Utils/Threading/WorkerThreadPool.h>
#include <WebServices/Connect/SamlToken.h>

// Work --> UI
// These messages are reacting to virtual function callbacks from ConnectSpaces worker threads
#define CS_MESSAGE_StatusReport     "ConnectSpaces.Message.StatusReport"
#define CS_MESSAGE_TokenUpdate      "ConnectSpaces.Message.TokenUpdate"
#define CS_MESSAGE_EulaStatus       "ConnectSpaces.Message.EulaStatus"
#define CS_MESSAGE_EulaDownloaded   "ConnectSpaces.Message.EulaDownloaded"
#define CS_MESSAGE_EulaTokenUpdate  "ConnectSpaces.Message.EulaTokenUpdate"

// UI --> Work
// These messages are requests for the ConnectSpaces worker threads to do work

#define CS_MESSAGE_SetCredentials   "ConnectSpaces.Message.SetCredentials"
#define CS_MESSAGE_ResetEula        "ConnectSpaces.Message.ResetEula"
#define CS_MESSAGE_CheckEula        "ConnectSpaces.Message.CheckEula"
#define CS_MESSAGE_AcceptEula       "ConnectSpaces.Message.AcceptEula"
#define CS_MESSAGE_SetEulaToken     "ConnectSpaces.Message.SetEulaToken"

// Message fields
#define CS_MESSAGE_FIELD_username   "username"
#define CS_MESSAGE_FIELD_password   "password"
#define CS_MESSAGE_FIELD_token      "token"
#define CS_MESSAGE_FIELD_EULA       "EULA"
#define CS_MESSAGE_FIELD_accepted   "accepted"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE 

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS
//=======================================================================================
//! @deprecated
//! @private
// @bsiclass                                                     Travis.Cobbs   05/2013
//=======================================================================================
struct ConnectSpaces
    {
    public:
        //! Initialize once in app lifetime
        WSCLIENT_EXPORT static void Initialize(ClientInfoPtr clientInfo);
        WSCLIENT_EXPORT static bool IsInitialized();
        WSCLIENT_EXPORT static void Uninitialize();

        WSCLIENT_EXPORT bool OnMessageReceived(Utf8CP messageType, JsonValueCR messageObj);

    public:
        enum StatusCode
            {
            OK = 0,
            UnknownError = 1,
            NetworkError = 2,
            UnexpectedResponseError = 3,
            CredentialsError = 4,
            };

        enum StatusAction
            {
            SetCredentialsAction = 4,
            CheckEulaAction = 6,
            AcceptEulaAction = 7,
            SetEulaTokenAction = 8,
            ResetEulaAction = 9
            };

        WSCLIENT_EXPORT ConnectSpaces();
        WSCLIENT_EXPORT ConnectSpaces(const ConnectSpaces& other);
        WSCLIENT_EXPORT ~ConnectSpaces();

    private:
        void ResetEula(bool getNewToken = false);
        void CheckEula(bool getNewToken = false);
        void AcceptEula(bool getNewToken = false);
        bool DownloadEula(Utf8StringR eulaString, bool getNewToken = false);

        BentleyStatus GetNewTokenIfNeeded(bool getNewToken, StatusAction action, SamlTokenR token, Utf8CP appliesToUrl = nullptr);
        HttpRequest CreateGetRequest(Utf8StringCR url, bool acceptJson = true, bool includeToken = true);

        void SendStatusToUIThread(StatusAction action, StatusCode statusCode, JsonValueCR data = Json::Value());
        void SendJsonMessageToUiThread(Utf8CP messageType, JsonValueCR response = Json::Value());

        void SetCredentials(Credentials credentials, Utf8StringCR token);
        void SetEulaToken(Utf8StringCR token);

        void ResetEulaAsync();
        void CheckEulaAsync();
        void AcceptEulaAsync();

    private:
        static std::map<Utf8String, StatusAction> sm_actionMap;
        static Utf8String sm_eulaUrlBase;

        BeMutex m_credentialsCriticalSection;
        Credentials m_credentials;
        SamlToken m_token;
        SamlToken m_eulaToken;

        SimpleCancellationTokenPtr m_cancelToken;
        HttpClient m_client;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
