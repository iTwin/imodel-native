/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Connect/ConnectSpaces.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/ConnectSpaces.h>

#include <Bentley/BeTimeUtilities.h>
#include <Bentley/DateTime.h>
#include <MobileDgn/MobileDgnApplication.h>
#include <MobileDgn/Utils/Http/HttpRequest.h>
#include <MobileDgn/Utils/Http/HttpStatusHelper.h>
#include <MobileDgn/Utils/Threading/WorkerThreadPool.h>
#include <WebServices/Connect/Connect.h>

#ifdef DEBUG
#include <Bentley/BeDirectoryIterator.h>
#endif
//#include <Bentley/BeDebugLog.h>

#ifdef QA_URLS
#define EULA_STS_AUTH_URI "https://ims-testing.bentley.com/rest/ActiveSTSService/json/IssueEx"
#else // QA_URLS
#define EULA_STS_AUTH_URI "https://ims.bentley.com/rest/ActiveSTSService/json/IssueEx"
#endif // !QA_URLS

#define EULA_HEADER "<html><head><meta charset=\"UTF-8\">\r\n</head><body>"
#define EULA_FOOTER "</body></html>"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_MOBILEDGN
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

#define MESSAGE_STRING_FIELD(x) messageObj[(x)].asCString()

#define HTTP_FILE_DOWNLOAD_TIMEOUT 6
#define HTTP_DEFAULT_TIMEOUT 10

ClientInfoPtr s_clientInfo;

static std::shared_ptr<WorkerThreadPool> s_threadPool;
static std::shared_ptr<WorkerThreadPool> s_dlThreadPool;

std::map<Utf8String, ConnectSpaces::StatusAction> ConnectSpaces::sm_actionMap;
std::map<std::string, int> ConnectSpaces::sm_monthMap;
std::set<std::string> ConnectSpaces::sm_dayNames;
Utf8String ConnectSpaces::sm_eulaUrlBase;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::Initialize (ClientInfoPtr clientInfo)
    {
    BeAssert (nullptr != clientInfo);

    s_clientInfo = clientInfo;
    s_threadPool   = WorkerThreadPool::Create (1, "ConnectSpaces::web");
    s_dlThreadPool = WorkerThreadPool::Create (2, "ConnectSpaces::downloads");

    if (sm_actionMap.empty())
        {
        sm_actionMap[CS_MESSAGE_FetchDatasourceList] = FetchDatasourceListAction;
        sm_actionMap[CS_MESSAGE_FetchFileList] = FetchFileListAction;
        sm_actionMap[CS_MESSAGE_FetchObjectList] = FetchObjectListAction;
        sm_actionMap[CS_MESSAGE_DownloadFile] = DownloadFileAction;
        sm_actionMap[CS_MESSAGE_SetCredentials] = SetCredentialsAction;
        sm_actionMap[CS_MESSAGE_GetFileStatus] = GetFileStatusAction;
        sm_actionMap[CS_MESSAGE_ResetEula] = ResetEulaAction;
        sm_actionMap[CS_MESSAGE_CheckEula] = CheckEulaAction;
        sm_actionMap[CS_MESSAGE_AcceptEula] = AcceptEulaAction;
        sm_actionMap[CS_MESSAGE_SetEulaToken] = SetEulaTokenAction;
#ifdef DEBUG
        sm_actionMap["ConnectSpaces.Message.DecreaseDates"] = DecreaseDatesAction;
#endif
        }
    if (sm_monthMap.empty())
        {
        sm_monthMap["Jan"] = 1;
        sm_monthMap["Feb"] = 2;
        sm_monthMap["Mar"] = 3;
        sm_monthMap["Apr"] = 4;
        sm_monthMap["May"] = 5;
        sm_monthMap["Jun"] = 6;
        sm_monthMap["Jul"] = 7;
        sm_monthMap["Aug"] = 8;
        sm_monthMap["Sep"] = 9;
        sm_monthMap["Oct"] = 10;
        sm_monthMap["Nov"] = 11;
        sm_monthMap["Dec"] = 12;
        }
    if (sm_dayNames.empty())
        {
        sm_dayNames.insert("Sun");
        sm_dayNames.insert("Mon");
        sm_dayNames.insert("Tue");
        sm_dayNames.insert("Wed");
        sm_dayNames.insert("Thu");
        sm_dayNames.insert("Fri");
        sm_dayNames.insert("Sat");
        }
    if (sm_eulaUrlBase.empty())
        {
        Utf8String urlFormat = Connect::GetEulaUrl() + "/Agreements/%d/Types/%s";
        sm_eulaUrlBase.Sprintf(urlFormat.c_str(), 1, "EULA");
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectSpaces::IsInitialized()
    {
    return s_threadPool.get() != nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::Uninitialize()
    {
    if (s_threadPool != nullptr)
        {
        s_threadPool->OnEmpty ()->Wait ();
        s_threadPool = nullptr;        
        }

    if (s_dlThreadPool)
        {
        s_dlThreadPool = nullptr;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSpaces::ConnectSpaces() : 
m_client (s_clientInfo),
m_cancelToken (SimpleCancellationToken::Create())
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSpaces::ConnectSpaces(const ConnectSpaces& other) : 
m_credentials(other.m_credentials), 
m_token(other.m_token), 
m_eulaToken(other.m_eulaToken), 
m_client (other.m_client),
// NOTE: Create a new m_cancelToken
m_cancelToken(SimpleCancellationToken::Create(other.m_cancelToken->IsCanceled()))
// NOTE: DO NOT copy m_credentialsCriticalSection.
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSpaces::~ConnectSpaces()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::SetCredentials(Credentials credentials, Utf8StringCR token)
    {
    m_credentialsCriticalSection.Enter();
    m_credentials = std::move (credentials);
    m_token = SamlToken (token);
    m_credentialsCriticalSection.Leave();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::SetEulaToken(Utf8StringCR token)
    {
    m_credentialsCriticalSection.Enter();
    m_eulaToken = token;
    m_credentialsCriticalSection.Leave();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::Cancel()
    {
    // Note: m_cancelToken is a ref-counted pointer, but it is created in the constructor,
    // so there's no need to check if it's non-null.
    m_cancelToken->SetCanceled ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequest ConnectSpaces::CreateGetRequest (Utf8StringCR url, bool acceptJson /*= true*/, bool includeToken /*= true*/)
    {
    HttpRequest request = m_client.CreateGetRequest (url);

    if (includeToken)
        {
        request.GetHeaders ().SetAuthorization (m_token.ToAuthorizationString ());
        }

    request.SetTimeoutSeconds (HTTP_DEFAULT_TIMEOUT);
    request.SetCancellationToken(m_cancelToken);

    if (acceptJson)
        {
        request.GetHeaders().SetValue("Accept", "application/json");
        }

    return request;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsRedirectToStsLogin (HttpResponseCR response)
    {
    return Connect::IsImsLoginRedirect (response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::SendJsonMessageToUiThread(Utf8CP messageType, JsonValueCR response)
    {
    Json::Value credentials;

    m_credentialsCriticalSection.Enter();
    credentials["username"] = m_credentials.GetUsername ();
    credentials["password"] = m_credentials.GetPassword ();
    m_credentialsCriticalSection.Leave();

    Json::Value wrapper;
    wrapper["credentials"] = credentials;
    if (!response.isNull())
        {
        wrapper["data"] = response;
        }

    MobileDgnApplication::App ().Messages ().Send (JsonMessage (messageType, wrapper));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::SendStatusToUIThread(StatusAction action, StatusCode statusCode, JsonValueCR data)
    {
    Json::Value statusData(data);
    statusData["statusAction"] = (int)action;
    statusData["statusCode"] = (int)statusCode;
    SendJsonMessageToUiThread(CS_MESSAGE_StatusReport, statusData);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ConnectSpaces::GetNewTokenIfNeeded (bool getNewToken, StatusAction action, SamlTokenR token, Utf8CP appliesToUrl /*= NULL*/,
                                        Utf8CP stsUrl /*= NULL*/)
    {
    SamlToken newToken;

    m_credentialsCriticalSection.Enter();
    if (!getNewToken && !token.IsEmpty ())
        {
        m_credentialsCriticalSection.Leave();
        return SUCCESS;
        }
    StatusInt status = Connect::Login(m_credentials, newToken, appliesToUrl, stsUrl);
    m_credentialsCriticalSection.Leave();

    if (SUCCESS == status)
        {
        m_credentialsCriticalSection.Enter();
        token = newToken;
        m_credentialsCriticalSection.Leave();
        // Note: even though the below access members (m_token and m_eulaToken) protected
        // by the critical section, it doesn't access their data, just their address,
        // which cannot change, since they are non-pointer member variables.
        if (&token == &m_token)
            {
            SendJsonMessageToUiThread (CS_MESSAGE_TokenUpdate, token.AsString());
            }
        else if (&token == &m_eulaToken)
            {
            SendJsonMessageToUiThread (CS_MESSAGE_EulaTokenUpdate, token.AsString ());
            }
        return SUCCESS;
        }
    else
        {
        SendStatusToUIThread(action, CredentialsError);
        return ERROR;
        }
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::FetchFileList(Utf8StringCR dsId, Utf8StringCR folderId, bool getNewToken)
    {
    if (SUCCESS != GetNewTokenIfNeeded(getNewToken, FetchFileListAction, m_token))
        {
        // Note: error sent to UI thread in GetNewTokenIfNeeded().
        return;
        }
    Utf8String navUrl = Connect::GetWsgUrl() + "/Datasources/" + dsId + "/Navigation";
    if (!folderId.empty())
        {
        navUrl += "/Folder/" + folderId;
        }

    HttpResponse httpResponse = CreateGetRequest(navUrl).Perform();

    if (IsRedirectToStsLogin (httpResponse))
        {
        if (getNewToken)
            {
            // We already got a new token, but it's not working.
            SendStatusToUIThread(FetchFileListAction, CredentialsError);
            }
        else
            {
            // Try again, but get a new token this time.
            FetchFileList(dsId, folderId, true);
            }
        }
    else
        {
        switch (httpResponse.GetHttpStatus())
            {
            case HttpStatus::OK:
                {
                Json::Value navValue = httpResponse.GetBody().AsJson();
                navValue["DatasourceId"] = dsId.c_str();
                if (!folderId.empty())
                    {
                    navValue["FolderId"] = folderId;
                    }
                for (JsonValueR item : navValue["Document"])
                    {
                    item = AppendFileStatus(item);
                    }
                SendJsonMessageToUiThread(CS_MESSAGE_FileListFetched, navValue);
                }
                break;
            case HttpStatus::Unauthorized:
                if (getNewToken)
                    {
                    SendStatusToUIThread(FetchFileListAction, CredentialsError);
                    }
                else
                    {
                    // Try again, but get a new token this time.
                    FetchFileList(dsId, folderId, true);
                    }
                break;
            default:
                SendStatusToUIThread(FetchFileListAction, UnknownError);
                break;
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::FetchObjectList(Utf8StringCR dsId, Utf8StringCR objectClass, bool getNewToken)
    {
    if (SUCCESS != GetNewTokenIfNeeded(getNewToken, FetchObjectListAction, m_token))
        {
        // Note: error sent to UI thread in GetNewTokenIfNeeded().
        return;
        }
    Utf8String navUrl = Connect::GetWsgUrl() + "/Datasources/" + dsId + "/Objects";
    if (!objectClass.empty())
        {
        navUrl += "/" + objectClass;
        }

    HttpResponse httpResponse = CreateGetRequest(navUrl).Perform();

    switch (httpResponse.GetHttpStatus())
        {
        case HttpStatus::OK:
            {
            Json::Value navValue = httpResponse.GetBody().AsJson();
//            Utf8String debug("Object list(" + dsId + ", " + objectClass + "):\n");
//            debug += navValue.toStyledString();
//            debug += "\nEffective URL: " + httpResponse.GetEffectiveUrl();
//            BeDebugLog(debug.c_str());
            navValue["DatasourceId"] = dsId.c_str();
            if (!objectClass.empty())
                {
                navValue["ObjectClass"] = objectClass;
                }
            for (JsonValueR item : navValue["Document"])
                {
                item = AppendFileStatus(item);
                }
            SendJsonMessageToUiThread(CS_MESSAGE_ObjectListFetched, navValue);
            }
            break;
        case HttpStatus::Unauthorized:
            if (getNewToken)
                {
                SendStatusToUIThread(FetchObjectListAction, CredentialsError);
                }
            else
                {
                // Try again, but get a new token this time.
                FetchObjectList(dsId, objectClass, true);
                }
            break;
        default:
            SendStatusToUIThread(FetchObjectListAction, UnknownError);
            break;
        }
    }

bool ConnectSpaces::ParseFixedNumber(const std::string &numberString, int &result, int min /*= 0*/, int max /*= 0*/)
    {
    for (size_t i = 0; i < numberString.size(); ++i)
        {
        if (!isdigit(numberString[i]))
            {
            return false;
            }
        }
    result = atoi(numberString.c_str());
    if (min != max)
        {
        if (result < min || result > max)
            {
            return false;
            }
        }
    return true;
    }

bool ConnectSpaces::ParseLastModified(const std::string &lastModified, BentleyApi::DateTime &dateTime)
    {
    // 0         1         2
    // 01234567890123456789012345678
    // ddd, DD mmm YYYY HH:MM:SS GMT
    if (lastModified.size() == 29 &&
        lastModified.substr(3, 2) == ", " &&
        lastModified[7] == ' ' &&
        lastModified[11] == ' ' &&
        lastModified[16] == ' ' &&
        lastModified[19] == ':' &&
        lastModified[22] == ':' &&
        lastModified.substr(25, 4) == " GMT")
        {
        // Length is correct and all constant tokens are correct.
        if (sm_dayNames.find(lastModified.substr(0, 3)) == sm_dayNames.end())
            {
            return false;
            }
        int day;
        if (!ParseFixedNumber(lastModified.substr(5, 2), day, 1, 31))
            {
            return false;
            }
        auto itMonth = sm_monthMap.find(lastModified.substr(8, 3));
        if (itMonth == sm_monthMap.end())
            {
            return false;
            }
        int month = itMonth->second;
        int year;
        if (!ParseFixedNumber(lastModified.substr(12, 4), year))
            {
            return false;
            }
        int hour;
        if (!ParseFixedNumber(lastModified.substr(17, 2), hour, 0, 23))
            {
            return false;
            }
        int minute;
        if (!ParseFixedNumber(lastModified.substr(20, 2), minute, 0, 59))
            {
            return false;
            }
        int second;
        if (!ParseFixedNumber(lastModified.substr(23, 2), second, 0, 59))
            {
            return false;
            }
        dateTime = DateTime(DateTime::Kind::Utc, (int16_t) year, (uint8_t) month, (uint8_t) day, (uint8_t) hour, (uint8_t) minute, (uint8_t) second);
        return true;
        }
    return false;
    }

// The commented out code below works fine, but it requires std::regex in order to work,
// and GCC (used for Android) doesn't yet support std::regex.
//bool ConnectSpaces::ParseLastModified(const std::string &lastModified, BentleyApi::DateTime &dateTime)
//    {
//    std::string regexStr(
//        "^"
//        "(Sun|Mon|Tue|Wed|Thu|Fri|Sat), "                    // Day of Week
//        "(0[1-9]|1[012]) "                                   // Day of Month
//        "(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) " // Month
//        "([0-9]{4}) "                                        // Year
//        "([01][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9]) "      // Time of Day
//        "GMT"                                                // Must be GMT
//        "$"
//        );
//    std::regex expr(regexStr);
//    std::smatch piecesMatch;
//    if (std::regex_match(lastModified, piecesMatch, expr))
//        {
//        dateTime = DateTime(
//            DateTime::DATETIMEKIND_Utc,
//            atoi(piecesMatch[4].str().c_str()),  // Year
//            sm_monthMap[piecesMatch[3].str()],   // Month
//            atoi(piecesMatch[2].str().c_str()),  // Day
//            atoi(piecesMatch[5].str().c_str()),  // Hour
//            atoi(piecesMatch[6].str().c_str()),  // Minute
//            atoi(piecesMatch[7].str().c_str())); // Second
//        return true;
//        }
//    return false;
//    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::FetchDatasourceList(bool getNewToken)
    {
    if (SUCCESS != GetNewTokenIfNeeded(getNewToken, FetchDatasourceListAction, m_token))
        {
        // Note: error sent to UI thread in GetNewTokenIfNeeded().
        return;
        }

    Utf8String url = Connect::GetWsgUrl() + "/Datasources/";
    HttpResponse httpResponse = CreateGetRequest(url).Perform();

    if (IsRedirectToStsLogin(httpResponse))
        {
        if (getNewToken)
            {
            // We already got a new token, but it's not working.
            SendStatusToUIThread(FetchDatasourceListAction, CredentialsError);
            }
        else
            {
            FetchDatasourceList(true);
            }
        }
    else
        {
        if (httpResponse.GetConnectionStatus() != ConnectionStatus::OK)
            {
            SendStatusToUIThread(FetchDatasourceListAction, NetworkError);
            return;
            }

        if (httpResponse.GetHttpStatus() == HttpStatus::OK)
            {
            Json::Value dsData = httpResponse.GetBody().AsJson();
            try
                {
                if (dsData.type() == Json::arrayValue)
                    {
                    SendJsonMessageToUiThread(CS_MESSAGE_DatasourceListFetched, dsData);
                    }
                else
                    {
                    SendStatusToUIThread(FetchDatasourceListAction, UnexpectedResponseError);
                    }
                }
            catch (...)
                {
                SendStatusToUIThread(FetchDatasourceListAction, UnknownError);
                }
            }
        else if (httpResponse.GetHttpStatus() == HttpStatus::Unauthorized)
            {
            if (getNewToken)
                {
                SendStatusToUIThread(FetchDatasourceListAction, UnknownError);
                }
            else
                {
                // Note: a corrupt security token can result in a server error, so
                // throw out our token and grab a new if we get a ServerError.
                // Try again, but get a new token this time.
                FetchDatasourceList(true);
                }
            }
        else
            {
            SendStatusToUIThread(FetchDatasourceListAction, UnknownError);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Rolandas.Rimkus    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::ResetEula(bool getNewToken)
    {    
    if (SUCCESS != GetNewTokenIfNeeded(getNewToken, ResetEulaAction, m_eulaToken, Connect::GetEulaUrl().c_str(), Connect::GetStsUrl().c_str()))
        {
        // Note: error sent to UI thread in GetNewTokenIfNeeded().
        return;
        }
    Utf8String url = Connect::GetEulaUrl() + "/Agreements/RevokeAgreementService/" + m_credentials.GetUsername ();
    HttpRequest request = m_client.CreatePostRequest (url);
    request.GetHeaders().SetValue("Content-Type", "application/json");
    m_credentialsCriticalSection.Enter();
    request.GetHeaders().SetAuthorization(m_eulaToken.ToAuthorizationString());
    bmap<Utf8String, Utf8String> attributes;
    BentleyStatus attributeStatus = m_eulaToken.GetAttributes(attributes);
    m_credentialsCriticalSection.Leave();
    
    if (SUCCESS != attributeStatus)
        {
        // The token we got is invalid.
        SendStatusToUIThread(ResetEulaAction, CredentialsError);
        return;
        }
    request.SetTimeoutSeconds (HTTP_DEFAULT_TIMEOUT);
    request.SetCancellationToken(m_cancelToken);
    HttpResponse httpResponse = request.Perform();
    if (IsRedirectToStsLogin(httpResponse))
        {
        if (getNewToken)
            {
            // We already got a new token, but it's not working.
            SendStatusToUIThread(ResetEulaAction, CredentialsError);
            }
        else
            {
            ResetEula(true);
            }
        }
    else
        {
        if (httpResponse.GetConnectionStatus() != ConnectionStatus::OK)
            {
            SendStatusToUIThread(ResetEulaAction, NetworkError);
            }

        if (httpResponse.GetHttpStatus() == HttpStatus::OK)
            {
            SendStatusToUIThread(ResetEulaAction, OK);
            Json::Value dsData = httpResponse.GetBody().AsJson();
            SendJsonMessageToUiThread(CS_MESSAGE_ResetEula, httpResponse.GetBody().AsJson().asString());  
            }
        else
            {
            SendStatusToUIThread(ResetEulaAction, UnknownError);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::CheckEula(bool getNewToken)
    {
    if (SUCCESS != GetNewTokenIfNeeded(getNewToken, CheckEulaAction, m_eulaToken, Connect::GetEulaUrl().c_str() , Connect::GetStsUrl().c_str())) 
        {
        // Note: error sent to UI thread in GetNewTokenIfNeeded().
        return;
        }

    Utf8String url = sm_eulaUrlBase + "/state";
    HttpRequest request = CreateGetRequest(url);
    m_credentialsCriticalSection.Enter();
    request.GetHeaders().SetAuthorization (m_eulaToken.ToAuthorizationString());
    m_credentialsCriticalSection.Leave();
    HttpResponse httpResponse = request.Perform();
    if (IsRedirectToStsLogin(httpResponse))
        {
        if (getNewToken)
            {
            // We already got a new token, but it's not working.
            SendStatusToUIThread(CheckEulaAction, CredentialsError);
            }
        else
            {
            CheckEula(true);
            }
        }
    else
        {
        if (httpResponse.GetConnectionStatus() != ConnectionStatus::OK)
            {
            SendStatusToUIThread(CheckEulaAction, NetworkError);
            return;
            }

        if (httpResponse.GetHttpStatus() == HttpStatus::OK)
            {
            Json::Value dsData = httpResponse.GetBody().AsJson();
            try
                {
                const Json::Value &accepted = dsData["accepted"];
                
                if (accepted.isBool())
                    {
                    Json::Value messageData;
                    messageData[CS_MESSAGE_FIELD_accepted] = accepted;
                    if (accepted.asBool())
                        {
                        SendJsonMessageToUiThread(CS_MESSAGE_EulaStatus, messageData);
                        }
                    else
                        {
                        Utf8String eulaString;
                        
                        if (DownloadEula(eulaString))
                            {
                            messageData[CS_MESSAGE_FIELD_EULA] = eulaString;
                            SendJsonMessageToUiThread(CS_MESSAGE_EulaStatus, messageData);
                            }
                        }
                    
                    }
                else
                    {
                    throw false;
                    }
                }
            catch (...)
                {
                SendStatusToUIThread(CheckEulaAction, UnexpectedResponseError);
                }
            }
        // TODO:
        // Should check for concrete HttpStatus code instead of all ServerErrors possible.
        else if (httpResponse.GetHttpStatus() == HttpStatus::Forbidden ||
                 httpResponse.GetHttpStatus() == HttpStatus::Unauthorized ||
                 HttpStatusHelper::GetType(httpResponse.GetHttpStatus()) == HttpStatusType::ServerError)
            {
            if (getNewToken)
                {
                SendStatusToUIThread(CheckEulaAction, UnknownError);
                }
            else
                {
                // Note: a corrupt security token can result in a server error, so
                // throw out our token and grab a new if we get a ServerError.
                // Try again, but get a new token this time.
                CheckEula(true);
                }
            }
        else
            {
            SendStatusToUIThread(CheckEulaAction, UnknownError);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectSpaces::DownloadEula(Utf8StringR eulaString, bool getNewToken)
    {
    if (SUCCESS != GetNewTokenIfNeeded(getNewToken, CheckEulaAction, m_eulaToken, Connect::GetEulaUrl().c_str() , Connect::GetStsUrl().c_str()))
        {
        // Note: error sent to UI thread in GetNewTokenIfNeeded().
        return false;
        }
    BeFileName tempPathName = MobileDgnApplication::App ().GetApplicationPaths ().GetTemporaryDirectory ();
    tempPathName.AppendToPath(L"eula.html");
    Utf8String tempPath(tempPathName);
    Utf8String url = sm_eulaUrlBase;
    HttpRequest request = CreateGetRequest(url);
    m_credentialsCriticalSection.Enter();
    request.GetHeaders().SetAuthorization(m_eulaToken.ToAuthorizationString());
    m_credentialsCriticalSection.Leave();
    HttpResponse httpResponse = request.Perform();
    bool retValue = false;
    if (IsRedirectToStsLogin(httpResponse))
        {
        if (getNewToken)
            {
            // We already got a new token, but it's not working.
            SendStatusToUIThread(CheckEulaAction, CredentialsError);
            }
        else
            {
            return DownloadEula(eulaString, true);
            }
        }
    else
        {
        if (httpResponse.GetConnectionStatus() != ConnectionStatus::OK)
            {
            SendStatusToUIThread(CheckEulaAction, NetworkError);
            return retValue;
            }

        if (httpResponse.GetHttpStatus() == HttpStatus::OK)
            {
            Json::Value dsData = httpResponse.GetBody().AsJson();
            try
                {
                const Json::Value &text = dsData["text"];
                if (text.isString())
                    {
                    eulaString = text.asString();
                    retValue = true;
                    }
                else
                    {
                    throw false;
                    }
                }
            catch (...)
                {
                SendStatusToUIThread(CheckEulaAction, UnknownError);
                }
            }
        else
            {
            SendStatusToUIThread(CheckEulaAction, UnknownError);
            }
        }
    
    return retValue;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::AcceptEula(bool getNewToken)
    {
    if (SUCCESS != GetNewTokenIfNeeded(getNewToken, AcceptEulaAction, m_eulaToken, Connect::GetEulaUrl().c_str(), Connect::GetStsUrl().c_str()))
        {
        // Note: error sent to UI thread in GetNewTokenIfNeeded().
        return;
        }
    Utf8String url = sm_eulaUrlBase + "/state";
    HttpRequest request = m_client.CreatePostRequest (url);
    request.GetHeaders().SetValue("Content-Type", "application/json");
    m_credentialsCriticalSection.Enter();
    request.GetHeaders().SetAuthorization(m_eulaToken.ToAuthorizationString());

    bmap<Utf8String, Utf8String> attributes;
    BentleyStatus attributeStatus = m_eulaToken.GetAttributes(attributes);
    m_credentialsCriticalSection.Leave();
    if (SUCCESS != attributeStatus)
        {
        // The token we got is invalid.
        SendStatusToUIThread(AcceptEulaAction, CredentialsError);
        return;
        }
    request.SetTimeoutSeconds (HTTP_DEFAULT_TIMEOUT);
    request.SetCancellationToken(m_cancelToken);
    Json::Value params;
    params["accepted"] = true;
    HttpStringBodyPtr requestBody = HttpStringBody::Create(Json::FastWriter().write(params));
    request.SetRequestBody(requestBody);
    HttpResponse httpResponse = request.Perform();
    if (IsRedirectToStsLogin(httpResponse))
        {
        if (getNewToken)
            {
            // We already got a new token, but it's not working.
            SendStatusToUIThread(AcceptEulaAction, CredentialsError);
            }
        else
            {
            AcceptEula(true);
            }
        }
    else
        {
        if (httpResponse.GetConnectionStatus() != ConnectionStatus::OK)
            {
            SendStatusToUIThread(AcceptEulaAction, NetworkError);
            }

        if (httpResponse.GetHttpStatus() == HttpStatus::OK)
            {
            SendStatusToUIThread(AcceptEulaAction, OK);
            }
        else
            {
            SendStatusToUIThread(AcceptEulaAction, UnknownError);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::FetchFileListAsync(Utf8StringCR dsId, Utf8StringCR folderId)
    {
    m_credentialsCriticalSection.Enter();
    ConnectSpaces* spaces = new ConnectSpaces(*this);
    m_credentialsCriticalSection.Leave();
    s_threadPool->ExecuteAsync(
        [=]()
            {
            spaces->FetchFileList(dsId, folderId);
            delete spaces;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::FetchObjectListAsync(Utf8StringCR dsId, Utf8StringCR objectClass)
    {
    m_credentialsCriticalSection.Enter();
    ConnectSpaces* spaces = new ConnectSpaces(*this);
    m_credentialsCriticalSection.Leave();
    s_threadPool->ExecuteAsync(
        [=]()
            {
            spaces->FetchObjectList(dsId, objectClass);
            delete spaces;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::FetchDatasourceListAsync()
    {
    m_credentialsCriticalSection.Enter();
    ConnectSpaces* spaces = new ConnectSpaces(*this);
    m_credentialsCriticalSection.Leave();
    s_threadPool->ExecuteAsync(
        [=]()
            {
            spaces->FetchDatasourceList();
            delete spaces;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Rolandas.Rimkus    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::ResetEulaAsync()
    {
    m_credentialsCriticalSection.Enter();
    ConnectSpaces* spaces = new ConnectSpaces(*this);
    m_credentialsCriticalSection.Leave();
    s_threadPool->ExecuteAsync(
        [=]()
            {
            spaces->ResetEula();
            delete spaces;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::CheckEulaAsync()
    {
    m_credentialsCriticalSection.Enter();
    ConnectSpaces* spaces = new ConnectSpaces(*this);
    m_credentialsCriticalSection.Leave();
    s_threadPool->ExecuteAsync(
        [=]()
            {
            spaces->CheckEula();
            delete spaces;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::AcceptEulaAsync()
    {
    m_credentialsCriticalSection.Enter();
    ConnectSpaces* spaces = new ConnectSpaces(*this);
    m_credentialsCriticalSection.Leave();
    s_threadPool->ExecuteAsync(
        [=]()
            {
            spaces->AcceptEula();
            delete spaces;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::DownloadFile(JsonValueCR messageObj)
    {
    DownloadFile(messageObj, false);
    }

void ConnectSpaces::SetJsonDocData(JsonValueR data, Utf8StringCR datasourceId, Utf8StringCR docId, Utf8StringCR filename)
    {
    data["datasourceId"] = datasourceId.c_str();
    data["docId"] = docId.c_str();
    data["filename"] = filename.c_str();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::DownloadFile(JsonValueCR messageObj, bool getNewToken)
    {
    Utf8String filename(MESSAGE_STRING_FIELD("Name"));
    Utf8String datasourceId(MESSAGE_STRING_FIELD(CS_MESSAGE_FIELD_datasourceId));
    Utf8String docId(MESSAGE_STRING_FIELD("$id"));
    Utf8String lastModified(MESSAGE_STRING_FIELD("FileLastModifiedDate"));
    WString wFilename(filename.c_str(), true);
    BeFileName tempPathName = MobileDgnApplication::App ().GetApplicationPaths ().GetTemporaryDirectory ();
    BeFileName docsPathName = MobileDgnApplication::App ().GetApplicationPaths ().GetDocumentsDirectory ();
    tempPathName.AppendToPath(wFilename.c_str());
    docsPathName.AppendToPath(wFilename.c_str());

    if (SUCCESS != GetNewTokenIfNeeded(getNewToken, DownloadFileAction, m_token))
        {
        // Note: error sent to UI thread in GetNewTokenIfNeeded().
        return;
        }
    Utf8String docUrl = Connect::GetWsgUrl() + "/Datasources/" + datasourceId + "/Files/Document/" + docId;

    HttpRequest firstRequest = CreateGetRequest(docUrl, false);
    firstRequest.SetFollowRedirects (false);
    HttpResponse response = firstRequest.Perform();
    Json::Value data;
    SetJsonDocData(data, datasourceId, docId, filename);

    if (response.GetHttpStatus() != HttpStatus::Found)
        {
        SendStatusToUIThread(DownloadFileAction, UnexpectedResponseError, data);
        return;
        }

    if (response.GetHeaders ().GetValue ("Location") == nullptr)
        {
        SendStatusToUIThread(DownloadFileAction, UnexpectedResponseError, data);
        return;
        }
    docUrl = response.GetHeaders ().GetValue ("Location");

    HttpRequest httpRequest = CreateGetRequest (docUrl, false, false);

    httpRequest.SetResponseBody (HttpFileBody::Create (tempPathName));
    httpRequest.SetDownloadProgressCallback (
         [=] (double bytesDownloaded, double bytesTotal)
             {
             bool isProgressKnown = 0 != bytesTotal;
             if (isProgressKnown)
                 {
                 Json::Value data;
                 SetJsonDocData(data, datasourceId, docId, filename);
                 data["isProgressKnown"] = isProgressKnown;
                 data["bytesDownloaded"] = bytesDownloaded;
                 data["bytesTotal"] = bytesTotal;
                 SendJsonMessageToUiThread(CS_MESSAGE_DownloadProgress, data);
                 }
             });
    httpRequest.SetRetryOptions(HttpRequest::ResumeTransfer);
    httpRequest.SetTimeoutSeconds (HTTP_FILE_DOWNLOAD_TIMEOUT);

    HttpResponse httpResponse = httpRequest.Perform();

//    if (httpResponse.Status() != HttpResponse::OK)
//        {
//        printf("Download error: %s\n", Json::FastWriter().write(httpResponse.Body().AsJson()).c_str());
//        }
//    printf("Download headers: \n");
//    for (auto it : httpResponse.Headers().Map())
//        {
//        printf("%s: %s\n", it.first.c_str(), it.second.c_str());
//        }

    if (IsRedirectToStsLogin (httpResponse))
        {
        if (getNewToken)
            {
            // We already got a new token, but it's not working.
            SendStatusToUIThread(DownloadFileAction, CredentialsError, data);
            }
        else
            {
            // Try again, but get a new token this time.
            DownloadFile(messageObj, true);
            }
        }
    else 
        {
        StatusCode result = UnknownError;
        
        if (httpResponse.GetConnectionStatus() != ConnectionStatus::OK)
            {
            result = NetworkError;
            }
        else if (httpResponse.GetHttpStatus() == HttpStatus::OK)
            {
            result = OK;
            }
        
        if (result == OK)
            {
            BeFileName::BeMoveFile(tempPathName, docsPathName);
            DateTime remoteModTime;
            bool haveHeaderRemoteModTime = false;
            Utf8CP lastModified = httpResponse.GetHeaders().GetValue("Last-Modified");

            if (lastModified != nullptr)
                {
                if (ParseLastModified(lastModified, remoteModTime))
                    {
                    haveHeaderRemoteModTime = true;
                    }
#ifdef DEBUG
                else
                    {
                    printf("Could not parse Last-Modified Date!\n");
                    }
#endif // DEBUG
                }
#if defined(DEBUG)
            else
                {
                BeAssert(false && "Server didn't specify Last-Modified Date!");
                }
#endif // DEBUG
            if (haveHeaderRemoteModTime || SUCCESS == DateTime::FromString(remoteModTime, messageObj["FileLastModifiedDate"].asString().c_str()))
                {
                int64_t localUnixModTime;
                remoteModTime.ToUnixMilliseconds(localUnixModTime);
                time_t localFileTime = (time_t)(localUnixModTime / 1000);
                BeFileName::SetFileTime(docsPathName, NULL, &localFileTime);
                }
            SendJsonMessageToUiThread(CS_MESSAGE_FileDownloaded, messageObj);
            }
        else
            {
            SendStatusToUIThread(DownloadFileAction, result, data);
            BeFileName::BeDeleteFile(tempPathName);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::DownloadFileAsync(JsonValueCR messageObj)
    {
    m_credentialsCriticalSection.Enter();
    ConnectSpaces* spaces = new ConnectSpaces(*this);
    m_credentialsCriticalSection.Leave();
    s_dlThreadPool->ExecuteAsync(
        [=]()
            {
            spaces->DownloadFile(messageObj);
            delete spaces;
            }/*FIXME,
        [=]()
            {
            spaces->Cancel();
            delete spaces;
            }*/);
    }

Json::Value ConnectSpaces::AppendFileStatus(JsonValueCR itemObj)
    {
    Utf8String filename(itemObj["Name"].asString());
    WString wFilename(filename.c_str(), true);
    BeFileName docsPathName = MobileDgnApplication::App ().GetApplicationPaths ().GetDocumentsDirectory ();
    docsPathName.AppendToPath(wFilename.c_str());
    FileStatus status = MissingStatus;
    DateTime remoteModTime;

    if (SUCCESS == DateTime::FromString(remoteModTime, itemObj["FileLastModifiedDate"].asString().c_str()))
        {
        time_t localUnixModTime;
        if (BeFileNameStatus::Success == BeFileName::GetFileTime(NULL, NULL, &localUnixModTime, docsPathName))
            {
            int64_t remoteUnixMillisModTime;

            remoteModTime.ToUnixMilliseconds(remoteUnixMillisModTime);
            time_t remoteUnixModTime = (time_t)(remoteUnixMillisModTime / 1000);
            if (remoteUnixModTime > localUnixModTime)
                {
                status = NeedsUpdateStatus;
                }
            else
                {
                status = UpToDateStatus;
                }
            }
        }

    Json::Value withStatus(itemObj);
    withStatus[CS_MESSAGE_FIELD_fileStatus] = (int)status;

    return withStatus;
    }

void ConnectSpaces::GetFileStatus(JsonValueCR messageObj)
    {
    Json::Value statusData(AppendFileStatus(messageObj));
    SendJsonMessageToUiThread(CS_MESSAGE_FileStatus, statusData);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectSpaces::OnMessageReceived(Utf8CP messageType, JsonValueCR messageObj)
    {
    auto it = sm_actionMap.find(messageType);

    if (it != sm_actionMap.end())
        {
        switch (it->second)
            {
            case FetchDatasourceListAction:
                FetchDatasourceListAsync();
                break;
            case FetchFileListAction:
                {
                Utf8String folderId;
                if (messageObj.isMember(CS_MESSAGE_FIELD_folderId))
                    {
                    JsonValueCR folderIdValue = messageObj[CS_MESSAGE_FIELD_folderId];
                    if (folderIdValue.isString())
                        {
                        folderId = folderIdValue.asString();
                        }
                    }
                FetchFileListAsync(MESSAGE_STRING_FIELD(CS_MESSAGE_FIELD_datasourceId), folderId);
                }
                break;
            case FetchObjectListAction:
                {
                Utf8String objectClass;
                if (messageObj.isMember(CS_MESSAGE_FIELD_objectClass))
                    {
                    JsonValueCR objectClassValue = messageObj[CS_MESSAGE_FIELD_objectClass];
                    if (objectClassValue.isString())
                        {
                        objectClass = objectClassValue.asString();
                        }
                    }
                FetchObjectListAsync(MESSAGE_STRING_FIELD(CS_MESSAGE_FIELD_datasourceId), objectClass);
                }
                break;
            case DownloadFileAction:
                DownloadFileAsync(messageObj);
                break;
            case SetCredentialsAction:
                SetCredentials
                    ({
                    MESSAGE_STRING_FIELD(CS_MESSAGE_FIELD_username), 
                    MESSAGE_STRING_FIELD(CS_MESSAGE_FIELD_password)}, 
                    MESSAGE_STRING_FIELD(CS_MESSAGE_FIELD_token)
                    );
                break;
            case GetFileStatusAction:
                GetFileStatus(messageObj);
                break;
            case ResetEulaAction:
                ResetEulaAsync();
                break;    
            case CheckEulaAction:
                CheckEulaAsync();
                break;
            case AcceptEulaAction:
                AcceptEulaAsync();
                break;
            case SetEulaTokenAction:
                SetEulaToken(MESSAGE_STRING_FIELD(CS_MESSAGE_FIELD_token));
                break;
#ifdef DEBUG
            case DecreaseDatesAction:
                DecreaseDates();
                break;
#endif
            }
        }
    return true;
    }

#ifdef DEBUG

void ConnectSpaces::DecreaseDates()
    {
    BeFileName docsPathName = MobileDgnApplication::App ().GetApplicationPaths ().GetDocumentsDirectory();
    bvector<BeFileName> imodelPaths;
    BeDirectoryIterator::WalkDirsAndMatch(imodelPaths, docsPathName, L"*.imodel", false);
    int i = 0;
    for (auto it = imodelPaths.begin(); it != imodelPaths.end(); ++it)
        {
        if ((++i % 2) == 0)
            {
            time_t fileTime;
            if (BeFileNameStatus::Success == BeFileName::GetFileTime(NULL, NULL, &fileTime, it->c_str()))
                {
                fileTime -= 3600;
                BeFileName::SetFileTime(it->c_str(), NULL, &fileTime);
                printf("Date moved back one hour for %s\n", Utf8String (*it).c_str());
                }
            }
        }
    }

#endif

