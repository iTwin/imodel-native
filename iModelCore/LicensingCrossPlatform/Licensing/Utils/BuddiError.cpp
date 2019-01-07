/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Utils/BuddiError.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//#include "ClientInternal.h"
//#include <Licensing/Utils/BuddiError.h>
#include "../../PublicAPI/Licensing/Utils/BuddiError.h"

#include <BeHttp/HttpStatusHelper.h>
//#include "BuddiError.xliff.h"
#include "../../PublicAPI/Licensing/Utils/BuddiError.xliff.h"

USING_NAMESPACE_BENTLEY_LICENSING

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
        m_message = BuddiErrorL10N::GetString(BuddiErrorL10N::UrlConfigurationError());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Julija.Semenenko   06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BuddiError::BuddiError(Http::ResponseCR httpResponse) : BuddiError()
    {
    if (Http::ConnectionStatus::OK != httpResponse.GetConnectionStatus())
        {
        m_message = Http::HttpError(httpResponse).GetDisplayMessage();
        m_description.clear();
        m_status = Status::ConnectionError;
        return;
        }

    if (Http::HttpStatus::OK != httpResponse.GetHttpStatus())
        {
        m_message = Http::HttpError(httpResponse).GetDisplayMessage();
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
