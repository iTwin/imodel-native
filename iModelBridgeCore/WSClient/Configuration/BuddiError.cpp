/*--------------------------------------------------------------------------------------+
|
|     $Source: Configuration/BuddiError.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Configuration/BuddiError.h>

#include <MobileDgn/Utils/Http/HttpStatusHelper.h>
#include <WebServices/Connect/Connect.h>
#include "BuddiError.xliff.h"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Julija.Semenenko   06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BuddiError::BuddiError() :
m_status(Status::UnxpectedError)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Julija.Semenenko   06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BuddiError::BuddiError(Status status)
    {
    m_status = status;
    if (Status::UrlNotConfigured == status)
        {
        m_message = BuddiErrorLocalizedString(MESSAGE_UrlConfigurationError);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Julija.Semenenko   06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BuddiError::BuddiError(HttpResponseCR httpResponse) : BuddiError()
    {
    if (ConnectionStatus::OK != httpResponse.GetConnectionStatus())
        {
        m_message = HttpError(httpResponse).GetDisplayMessage();
        m_description.clear();
        m_status = Status::ConnectionError;
        return;
        }

    if (HttpStatus::OK != httpResponse.GetHttpStatus())
        {
        m_message = HttpError(httpResponse).GetDisplayMessage();
        m_description.clear();
        m_status = Status::UnxpectedError;
        return;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Julija.Semenenko   06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BuddiError::Status BuddiError::GetStatus() const
    {
    return m_status;
    }