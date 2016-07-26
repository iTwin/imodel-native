/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Connect/ConnectSpaces.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/ConnectSpaces.h>

#include <DgnClientFx/DgnClientApp.h>
#include <BeHttp/HttpStatusHelper.h>
#include <Bentley/Tasks/WorkerThreadPool.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ImsClient.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BENTLEY_WEBSERVICES

#define MESSAGE_STRING_FIELD(x) messageObj[(x)].asCString()
#define HTTP_DEFAULT_TIMEOUT 10

static ClientInfoPtr s_clientInfo;
static IHttpHandlerPtr s_customHandler;
static std::shared_ptr<WorkerThreadPool> s_threadPool;

std::map<Utf8String, ConnectSpaces::StatusAction> ConnectSpaces::sm_actionMap;
Utf8String ConnectSpaces::sm_eulaUrlBase;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::Initialize(ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler)
    {
    BeAssert(nullptr != clientInfo);

    s_clientInfo = clientInfo;
    s_customHandler = customHandler;
    s_threadPool = WorkerThreadPool::Create(1, "ConnectSpaces::web");

    sm_actionMap[CS_MESSAGE_SetCredentials] = SetCredentialsAction;
    sm_actionMap[CS_MESSAGE_ResetEula] = ResetEulaAction;
    sm_actionMap[CS_MESSAGE_CheckEula] = CheckEulaAction;
    sm_actionMap[CS_MESSAGE_AcceptEula] = AcceptEulaAction;
    sm_actionMap[CS_MESSAGE_SetEulaToken] = SetEulaTokenAction;

    sm_eulaUrlBase = UrlProvider::Urls::ConnectEula.Get() + "/Agreements/1/Types/EULA";
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
        s_threadPool->OnEmpty()->Wait();
        s_threadPool = nullptr;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSpaces::ConnectSpaces() :
m_client(s_clientInfo, UrlProvider::GetSecurityConfigurator(s_customHandler)),
m_cancelToken(SimpleCancellationToken::Create())
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSpaces::ConnectSpaces(const ConnectSpaces& other) :
m_credentials(other.m_credentials),
m_token(other.m_token),
m_eulaToken(other.m_eulaToken),
m_client(other.m_client),
// NOTE: Create a new m_cancelToken
m_cancelToken(SimpleCancellationToken::Create(other.m_cancelToken->IsCanceled()))
// NOTE: DO NOT copy m_credentialsCriticalSection.
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSpaces::~ConnectSpaces()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::SetCredentials(Credentials credentials, Utf8StringCR token)
    {
    m_credentialsCriticalSection.Enter();
    m_credentials = std::move(credentials);
    m_token = SamlToken(token);
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
Http::Request ConnectSpaces::CreateGetRequest(Utf8StringCR url, bool acceptJson, bool includeToken)
    {
    Http::Request request = m_client.CreateGetRequest(url);

    if (includeToken)
        {// TODO: clean this - it's always overridden
        request.GetHeaders().SetAuthorization(m_token.ToAuthorizationString());
        }

    request.SetTimeoutSeconds(HTTP_DEFAULT_TIMEOUT);
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
bool IsRedirectToStsLogin(Http::ResponseCR response)
    {
    return ImsClient::IsLoginRedirect(response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::SendJsonMessageToUiThread(Utf8CP messageType, JsonValueCR response)
    {
    Json::Value credentials;

    m_credentialsCriticalSection.Enter();
    credentials["username"] = m_credentials.GetUsername();
    credentials["password"] = m_credentials.GetPassword();
    m_credentialsCriticalSection.Leave();

    Json::Value wrapper;
    wrapper["credentials"] = credentials;
    if (!response.isNull())
        {
        wrapper["data"] = response;
        }

    DgnClientApp::App().Messages().Send(JsonMessage(messageType, wrapper));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::SendStatusToUIThread(StatusAction action, StatusCode statusCode, JsonValueCR data)
    {
    Json::Value statusData(data);
    statusData["statusAction"] = (int) action;
    statusData["statusCode"] = (int) statusCode;
    SendJsonMessageToUiThread(CS_MESSAGE_StatusReport, statusData);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ConnectSpaces::GetNewTokenIfNeeded(bool getNewToken, StatusAction action, SamlTokenR token, Utf8CP appliesToUrl)
    {
    m_credentialsCriticalSection.Enter();
    if (!getNewToken && !token.IsEmpty())
        {
        m_credentialsCriticalSection.Leave();
        return SUCCESS;
        }

    auto result = ImsClient::GetShared()->RequestToken(m_credentials, appliesToUrl)->GetResult();
    m_credentialsCriticalSection.Leave();

    if (result.IsSuccess())
        {
        m_credentialsCriticalSection.Enter();
        token = *result.GetValue();
        m_credentialsCriticalSection.Leave();
        // Note: even though the below access members (m_token and m_eulaToken) protected
        // by the critical section, it doesn't access their data, just their address,
        // which cannot change, since they are non-pointer member variables.
        if (&token == &m_token)
            {
            SendJsonMessageToUiThread(CS_MESSAGE_TokenUpdate, token.AsString());
            }
        else if (&token == &m_eulaToken)
            {
            SendJsonMessageToUiThread(CS_MESSAGE_EulaTokenUpdate, token.AsString());
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
* @bsimethod                                                    Rolandas.Rimkus    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::ResetEula(bool getNewToken)
    {
    Utf8String eulaUrl = UrlProvider::Urls::ConnectEula.Get();

    m_eulaToken = *ConnectAuthenticationPersistence::GetShared()->GetToken();

    if (m_eulaToken.IsEmpty())
        {
        SendStatusToUIThread(ResetEulaAction, CredentialsError);
        return;
        }

    // The agreement service stores usernames (email addresses) in lower case and performs case-sensitive checks on them,
    // so we must map accordingly.
    Utf8String usernameLowerCase(ConnectAuthenticationPersistence::GetShared()->GetCredentials().GetUsername());
    usernameLowerCase.ToLower();
    Utf8String url = eulaUrl + "/Agreements/RevokeAgreementService/" + usernameLowerCase;
    Http::Request request = m_client.CreatePostRequest(url);
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
    request.SetTimeoutSeconds(HTTP_DEFAULT_TIMEOUT);
    request.SetCancellationToken(m_cancelToken);
    Http::Response httpResponse = request.Perform();
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
            Json::Value dsData = Json::Reader::DoParse(httpResponse.GetBody().AsString());
            SendJsonMessageToUiThread(CS_MESSAGE_ResetEula, dsData.asString());
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
    Utf8String eulaUrl = UrlProvider::Urls::ConnectEula.Get();
    m_eulaToken = *ConnectAuthenticationPersistence::GetShared()->GetToken();

    if (m_eulaToken.IsEmpty())
        {
        SendStatusToUIThread(CheckEulaAction, CredentialsError);
        return;
        }

    Utf8String url = sm_eulaUrlBase + "/state";
    Http::Request request = CreateGetRequest(url);
    m_credentialsCriticalSection.Enter();
    request.GetHeaders().SetAuthorization(m_eulaToken.ToAuthorizationString());
    m_credentialsCriticalSection.Leave();
    Http::Response httpResponse = request.Perform();
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
            Json::Value dsData = Json::Reader::DoParse(httpResponse.GetBody().AsString());
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
    Utf8String eulaUrl = UrlProvider::Urls::ConnectEula.Get();

    m_eulaToken = *ConnectAuthenticationPersistence::GetShared()->GetToken();

    if (m_eulaToken.IsEmpty())
        {
        SendStatusToUIThread(CheckEulaAction, CredentialsError);
        return false;
        }

    BeFileName tempPathName = DgnClientApp::App().GetApplicationPaths().GetTemporaryDirectory();
    tempPathName.AppendToPath(L"eula.html");
    Utf8String tempPath(tempPathName);
    Utf8String url = sm_eulaUrlBase;
    Http::Request request = CreateGetRequest(url);
    m_credentialsCriticalSection.Enter();
    request.GetHeaders().SetAuthorization(m_eulaToken.ToAuthorizationString());
    m_credentialsCriticalSection.Leave();
    Http::Response httpResponse = request.Perform();
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
            Json::Value dsData = Json::Reader::DoParse(httpResponse.GetBody().AsString());
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
    Utf8String eulaUrl = UrlProvider::Urls::ConnectEula.Get();
    m_eulaToken = *ConnectAuthenticationPersistence::GetShared()->GetToken();

    if (m_eulaToken.IsEmpty())
        {
        SendStatusToUIThread(AcceptEulaAction, CredentialsError);
        return;
        }

    Utf8String url = sm_eulaUrlBase + "/state";
    Http::Request request = m_client.CreatePostRequest(url);
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
    request.SetTimeoutSeconds(HTTP_DEFAULT_TIMEOUT);
    request.SetCancellationToken(m_cancelToken);
    Json::Value params;
    params["accepted"] = true;
    HttpStringBodyPtr requestBody = HttpStringBody::Create(Json::FastWriter().write(params));
    request.SetRequestBody(requestBody);
    Http::Response httpResponse = request.Perform();
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
* @bsimethod                                                    Rolandas.Rimkus    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSpaces::ResetEulaAsync()
    {
    m_credentialsCriticalSection.Enter();
    ConnectSpaces* spaces = new ConnectSpaces(*this);
    m_credentialsCriticalSection.Leave();
    s_threadPool->ExecuteAsync(
        [=] ()
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
        [=] ()
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
        [=] ()
        {
        spaces->AcceptEula();
        delete spaces;
        });
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
            case SetCredentialsAction:
                SetCredentials
                    ({
                    MESSAGE_STRING_FIELD(CS_MESSAGE_FIELD_username),
                    MESSAGE_STRING_FIELD(CS_MESSAGE_FIELD_password)},
                    MESSAGE_STRING_FIELD(CS_MESSAGE_FIELD_token)
                    );
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
            }
        }
    return true;
    }
