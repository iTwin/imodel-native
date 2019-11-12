/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <BeHttp/HttpError.h>
#include <BeHttp/HttpStatusHelper.h>

#include "HttpError.xliff.h"

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpError::HttpError() :
m_connectionStatus(ConnectionStatus::None),
m_httpStatus(HttpStatus::None)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpError::HttpError(Response httpResponse) :
AsyncError(
CreateMessage(httpResponse.GetConnectionStatus(), httpResponse.GetHttpStatus(), &httpResponse),
CreateDescription(httpResponse.GetConnectionStatus(), httpResponse.GetHttpStatus())
),
m_connectionStatus(httpResponse.GetConnectionStatus()),
m_httpStatus(httpResponse.GetHttpStatus())
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpError::HttpError(ConnectionStatus connectionStatus, HttpStatus httpStatus) :
AsyncError
(
CreateMessage(connectionStatus, httpStatus, nullptr),
CreateDescription(connectionStatus, httpStatus)
),
m_connectionStatus(connectionStatus),
m_httpStatus(httpStatus)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool HttpError::IsValid() const
    {
    return m_connectionStatus != ConnectionStatus::None || !m_message.empty();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Julius.Cepukenas      02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpError::CreateMessage(ConnectionStatus connectionStatus, HttpStatus httpStatus, ResponseCP response)
    {
    if (connectionStatus != ConnectionStatus::OK)
        {
        return GetConnectionErrorDisplayMessage(connectionStatus);
        }
    else
        {
        return GetHttpDisplayMessage(httpStatus, response);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpError::GetConnectionErrorDisplayMessage(ConnectionStatus connectionStatus)
    {
    switch (connectionStatus)
        {
        case ConnectionStatus::CouldNotConnect:     return HttpErrorLocalizedString(MSG_ConnectionStatus_CouldNotConnect);
        case ConnectionStatus::CouldNotResolveProxy: return HttpErrorLocalizedString(MSG_ConnectionStatus_CouldNotResolveProxy);
        case ConnectionStatus::Timeout:             return HttpErrorLocalizedString(MSG_ConnectionStatus_Timeout);
        case ConnectionStatus::ConnectionLost:      return HttpErrorLocalizedString(MSG_ConnectionStatus_ConnectionLost);
        case ConnectionStatus::CertificateError:    return HttpErrorLocalizedString(MSG_ConnectionStatus_CertificateError);
        case ConnectionStatus::None:                return HttpErrorLocalizedString(MSG_ConnectionStatus_UnknownStatus);
        case ConnectionStatus::UnknownError:        return HttpErrorLocalizedString(MSG_ConnectionStatus_UnknownStatus);

        case ConnectionStatus::Canceled:
            return "";

        case ConnectionStatus::OK:
            return "";

        default:
            BeAssert(false && "Unexpected ConnectionStatus");
            return HttpErrorLocalizedString(MSG_ConnectionStatus_UnknownStatus);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpError::GetHttpDisplayMessage(HttpStatus httpStatus, const Response* response)
    {
    HttpStatusType statusType = HttpStatusHelper::GetType(httpStatus);

    if (statusType != HttpStatusType::ServerError &&
        statusType != HttpStatusType::ClientError)
        {
        return "";
        }

    if (httpStatus == HttpStatus::ProxyAuthenticationRequired && response)
        {
        AuthenticationChallengeValue challenge;
        AuthenticationChallengeValue::Parse(response->GetHeaders().GetProxyAuthenticate(), challenge);
        if (!challenge.GetRealm().empty())
            return Utf8PrintfString(HttpErrorLocalizedString(STATUS_HttpStatus_407_WithRealm).c_str(), challenge.GetRealm().c_str());
        }

    switch (httpStatus)
        {
        case HttpStatus::Unauthorized:      return HttpErrorLocalizedString(STATUS_HttpStatus_401);
        case HttpStatus::Forbidden:         return HttpErrorLocalizedString(STATUS_HttpStatus_403);
        case HttpStatus::NotFound:          return HttpErrorLocalizedString(STATUS_HttpStatus_404);
        case HttpStatus::ProxyAuthenticationRequired: return HttpErrorLocalizedString(STATUS_HttpStatus_407);
        case HttpStatus::TooManyRequests:   return HttpErrorLocalizedString(STATUS_HttpStatus_429);
        }

    if (statusType == HttpStatusType::ServerError)
        return HttpErrorLocalizedString(STATUS_ServerError);

    return HttpErrorLocalizedString(STATUS_UnexpectedStatus);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Julius.Cepukenas      02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpError::CreateDescription(ConnectionStatus connectionStatus, HttpStatus httpStatus)
    {
    // We don't have description if connection failed.
    if (connectionStatus != ConnectionStatus::OK)
        return "";

    HttpStatusType statusType = HttpStatusHelper::GetType(httpStatus);

    if (statusType != HttpStatusType::ServerError &&
        statusType != HttpStatusType::ClientError)
        {
        return "";
        }

    if (HttpStatus::ProxyAuthenticationRequired == httpStatus)
        return "";

    return Utf8PrintfString(HttpErrorLocalizedString(MSG_HttpErrorDescription).c_str(), static_cast<int>(httpStatus));
    }
