/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/HttpError.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <BeHttp/HttpError.h>
#include <BeHttp/HttpStatusHelper.h>

#include "HttpError.xliff.h"

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpError::HttpError () :
HttpError(ConnectionStatus::None, HttpStatus::None)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpError::HttpError (HttpResponse httpResponse) :
HttpError(httpResponse.GetConnectionStatus(), httpResponse.GetHttpStatus())
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpError::HttpError(ConnectionStatus connectionStatus, HttpStatus httpStatus) :   
m_connectionStatus(connectionStatus),
m_httpStatus(httpStatus),
AsyncError(GetMessage(connectionStatus, httpStatus), GetDescription(connectionStatus, httpStatus))
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectionStatus HttpError::GetConnectionStatus() const
    {
    return m_connectionStatus;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpStatus HttpError::GetHttpStatus() const
    {
    return m_httpStatus;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpError::GetDisplayMessage() const
    {
    return m_message;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Julius.Cepukenas      02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpError::GetMessage(ConnectionStatus connectionStatus, HttpStatus httpStatus)
    {
    if (connectionStatus != ConnectionStatus::OK)
        {
        return GetConnectionErrorDisplayMessage (connectionStatus);
        }
    else
        {
        return GetHttpDisplayMessage (httpStatus);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpError::GetConnectionErrorDisplayMessage (ConnectionStatus connectionStatus)
    {
    switch (connectionStatus)
        {
        case ConnectionStatus::CouldNotConnect:     return HttpErrorLocalizedString (MSG_ConnectionStatus_CouldNotConnect);
        case ConnectionStatus::Timeout:             return HttpErrorLocalizedString (MSG_ConnectionStatus_Timeout);
        case ConnectionStatus::ConnectionLost:      return HttpErrorLocalizedString (MSG_ConnectionStatus_ConnectionLost);
        case ConnectionStatus::CertificateError:    return HttpErrorLocalizedString (MSG_ConnectionStatus_CertificateError);
        case ConnectionStatus::None:                return HttpErrorLocalizedString (MSG_ConnectionStatus_UnknownStatus);
        case ConnectionStatus::UnknownError:        return HttpErrorLocalizedString (MSG_ConnectionStatus_UnknownStatus);
        
        case ConnectionStatus::Canceled:
            BeAssert (false && "User should not get errors about canceled connections");
            return "";
        
        case ConnectionStatus::OK:
            BeAssert (false && "Not an error");
            return "";

        default:
            BeAssert (false && "Unexpected ConnectionStatus");
            return HttpErrorLocalizedString (MSG_ConnectionStatus_UnknownStatus);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpError::GetHttpDisplayMessage (HttpStatus httpStatus)
    {
    HttpStatusType statusType = HttpStatusHelper::GetType (httpStatus);
    
    if (statusType != HttpStatusType::ServerError &&
        statusType != HttpStatusType::ClientError)
        {
        BeAssert (false && "Not an error");
        return "";
        }
    
    switch (httpStatus)
        {
        case HttpStatus::Unauthorized:      return HttpErrorLocalizedString (STATUS_HttpStatus_401);
        case HttpStatus::Forbidden:         return HttpErrorLocalizedString (STATUS_HttpStatus_403);
        case HttpStatus::NotFound:          return HttpErrorLocalizedString (STATUS_HttpStatus_404);
        case HttpStatus::TooManyRequests:   return HttpErrorLocalizedString (STATUS_HttpStatus_429);
        
        default:
            if (statusType == HttpStatusType::ServerError)
                {
                return HttpErrorLocalizedString (STATUS_ServerError);
                }
            else
                {
                return HttpErrorLocalizedString (STATUS_UnexpectedStatus);
                }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpError::GetDisplayDescription() const
    {
    return m_description;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Julius.Cepukenas      02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpError::GetDescription(ConnectionStatus connectionStatus, HttpStatus httpStatus)
    {
    // We don't have description if connection failed.
    if (connectionStatus != ConnectionStatus::OK)
        {
        return "";
        }
        
    HttpStatusType statusType = HttpStatusHelper::GetType (httpStatus);
    
    if (statusType != HttpStatusType::ServerError &&
        statusType != HttpStatusType::ClientError)
        {
        BeAssert (false && "Not an error");
        return "";
        }
        
    return Utf8PrintfString (HttpErrorLocalizedString (MSG_HttpErrorDescription).c_str(), static_cast<int>(httpStatus));
    }
