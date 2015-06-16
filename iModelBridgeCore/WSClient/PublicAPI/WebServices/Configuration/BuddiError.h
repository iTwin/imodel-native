/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Configuration/BuddiError.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <MobileDgn/Utils/Threading/AsyncError.h>
#include <MobileDgn/Utils/Http/HttpError.h>
#include <MobileDgn/Utils/Http/HttpResponse.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

struct BuddiError;
typedef BuddiError& BuddiErrorR;
typedef const BuddiError& BuddiErrorCR;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                               Julija.Semenenko   06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct BuddiError : public AsyncError
    {
    public:
        enum Status
            {
            //! Request ConnectionStatus != OK
            ConnectionError,
            //! Response httpStatus != OK / XML is invadil / unable to parse value 
            UnxpectedError,
            //! Check urlName and make sure it is registered in the BUDDI - buddi.bentley.com
            UrlNotConfigured
            };

    private:
        Status m_status;

    public:
        //! Default error with status UnexpectedError
        WSCLIENT_EXPORT BuddiError();
        WSCLIENT_EXPORT BuddiError(Status status);
        WSCLIENT_EXPORT BuddiError(HttpResponseCR httpResponse);

        WSCLIENT_EXPORT Status GetStatus() const;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE