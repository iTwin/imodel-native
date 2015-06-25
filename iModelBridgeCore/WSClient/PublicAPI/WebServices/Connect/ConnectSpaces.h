/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Connect/ConnectSpaces.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <MobileDgn/MobileDgnCommon.h>
#include <MobileDgn/Utils/Threading/WorkerThreadPool.h>
#include <WebServices/Connect/SamlToken.h>
#include <MobileDgn/Utils/Http/Credentials.h>
#include <MobileDgn/Utils/Http/HttpClient.h>
#include <MobileDgn/Utils/Http/HttpRequest.h>
#include <functional>
#include <set>

// Work --> UI
// These messages are reacting to virtual function callbacks from ConnectSpaces worker threads
#define CS_MESSAGE_DatasourceListFetched "ConnectSpaces.Message.DatasourceListFetched"
#define CS_MESSAGE_FileListFetched "ConnectSpaces.Message.FileListFetched"
#define CS_MESSAGE_ObjectListFetched "ConnectSpaces.Message.ObjectListFetched"
#define CS_MESSAGE_StatusReport "ConnectSpaces.Message.StatusReport"
#define CS_MESSAGE_TokenUpdate "ConnectSpaces.Message.TokenUpdate"
#define CS_MESSAGE_DownloadProgress "ConnectSpaces.Message.DownloadProgress"
#define CS_MESSAGE_FileStatus "ConnectSpaces.Message.FileStatus"
#define CS_MESSAGE_FileDownloaded "ConnectSpaces.Message.FileDownloaded"
#define CS_MESSAGE_EulaStatus "ConnectSpaces.Message.EulaStatus"
#define CS_MESSAGE_EulaDownloaded "ConnectSpaces.Message.EulaDownloaded"
#define CS_MESSAGE_EulaTokenUpdate "ConnectSpaces.Message.EulaTokenUpdate"

// UI --> Work
// These messages are requests for the ConnectSpaces worker threads to do work
#define CS_MESSAGE_FetchDatasourceList "ConnectSpaces.Message.FetchDatasourceList"
#define CS_MESSAGE_FetchFileList "ConnectSpaces.Message.FetchFileList"
#define CS_MESSAGE_FetchObjectList "ConnectSpaces.Message.FetchObjectList"
#define CS_MESSAGE_DownloadFile "ConnectSpaces.Message.DownloadFile"
#define CS_MESSAGE_SetCredentials "ConnectSpaces.Message.SetCredentials"
#define CS_MESSAGE_GetFileStatus "ConnectSpaces.Message.GetFileStatus"
#define CS_MESSAGE_ResetEula "ConnectSpaces.Message.ResetEula"
#define CS_MESSAGE_CheckEula "ConnectSpaces.Message.CheckEula"
#define CS_MESSAGE_AcceptEula "ConnectSpaces.Message.AcceptEula"
#define CS_MESSAGE_SetEulaToken "ConnectSpaces.Message.SetEulaToken"

#define CS_MESSAGE_FIELD_username "username"
#define CS_MESSAGE_FIELD_password "password"
#define CS_MESSAGE_FIELD_token "token"
#define CS_MESSAGE_FIELD_datasourceId "datasourceId"
#define CS_MESSAGE_FIELD_docId "docId"
#define CS_MESSAGE_FIELD_EULA "EULA"
#define CS_MESSAGE_FIELD_accepted "accepted"
#define CS_MESSAGE_FIELD_fileStatus "fileStatus"
#define CS_MESSAGE_FIELD_folderId "folderId"
#define CS_MESSAGE_FIELD_objectClass "objectClass"

USING_NAMESPACE_BENTLEY_WEBSERVICES
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

    WSCLIENT_EXPORT static bool ParseLastModified(const std::string &lastModified, BentleyApi::DateTime &dateTime);

    WSCLIENT_EXPORT bool OnMessageReceived(Utf8CP messageType, JsonValueCR messageObj);

public:
    enum StatusCode
        {
        OK,
        UnknownError,
        NetworkError,
        UnexpectedResponseError,
        CredentialsError,
        };
    enum StatusAction
        {
        FetchFileListAction,
        FetchDatasourceListAction,
        FetchObjectListAction,
        DownloadFileAction,
        SetCredentialsAction,
        GetFileStatusAction,
        CheckEulaAction,
        AcceptEulaAction,
        SetEulaTokenAction,
        ResetEulaAction,
#ifdef DEBUG
        DecreaseDatesAction,
#endif
        };
    enum FileStatus
        {
        UpToDateStatus,
        NeedsUpdateStatus,
        MissingStatus,
        };
    WSCLIENT_EXPORT ConnectSpaces();
    WSCLIENT_EXPORT ConnectSpaces(const ConnectSpaces& other);
    WSCLIENT_EXPORT ~ConnectSpaces();
   
private:
    static bool ParseFixedNumber(const std::string &numberString, int &result, int min = 0, int max = 0);
    void FetchFileList(Utf8StringCR dsId, Utf8StringCR folderId, bool getNewToken = false);
    void FetchObjectList(Utf8StringCR dsId, Utf8StringCR objectClass, bool getNewToken = false);
    void FetchDatasourceList(bool getNewToken = false);
    void ResetEula(bool getNewToken = false);
    void CheckEula(bool getNewToken = false);
    void AcceptEula(bool getNewToken = false);
    bool DownloadEula(Utf8StringR eulaString, bool getNewToken = false);
    void DownloadFile(JsonValueCR messageObj, bool getNewToken);
    void Cancel();
    BentleyStatus GetNewTokenIfNeeded (bool getNewToken, StatusAction action, SamlTokenR token, Utf8CP appliesToUrl = nullptr, Utf8CP stsUrl = nullptr);
    HttpRequest CreateGetRequest (Utf8StringCR url, bool acceptJson = true, bool includeToken = true);
    void SetJsonDocData(JsonValueR data, Utf8StringCR datasourceId, Utf8StringCR docId, Utf8StringCR filename);
    void SendStatusToUIThread(StatusAction action, StatusCode statusCode, JsonValueCR data = Json::Value());
    void SendJsonMessageToUiThread(Utf8CP messageType, JsonValueCR response = Json::Value());

    void SetCredentials(Credentials credentials, Utf8StringCR token);
    void SetEulaToken(Utf8StringCR token);
    void FetchDatasourceListAsync();
    void ResetEulaAsync();
    void CheckEulaAsync();
    void AcceptEulaAsync();
    void FetchFileListAsync(Utf8StringCR dsId, Utf8StringCR folderId);
    void FetchObjectListAsync(Utf8StringCR dsId, Utf8StringCR objectClass);
    void DownloadFile(JsonValueCR messageObj);
    void DownloadFileAsync(JsonValueCR messageObj);
    void GetFileStatus(JsonValueCR messageObj);
    Json::Value AppendFileStatus(JsonValueCR itemObj);

#ifdef DEBUG
    void DecreaseDates();
#endif

    Credentials m_credentials;
    SamlToken m_token;
    SamlToken m_eulaToken;

    SimpleCancellationTokenPtr m_cancelToken;
    HttpClient m_client;

    BeMutex m_credentialsCriticalSection;
    static std::map<Utf8String, StatusAction> sm_actionMap;
    static std::map<std::string, int> sm_monthMap;
    static std::set<std::string> sm_dayNames;
    static Utf8String sm_eulaUrlBase;
    };
