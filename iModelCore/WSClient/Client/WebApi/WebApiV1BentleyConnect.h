/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WebApi/WebApiV1BentleyConnect.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "WebApiV1.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WebApiV1BentleyConect : public WebApiV1
    {
    protected:
        virtual Utf8String GetSchemaUrl () const override;

    public:
        WebApiV1BentleyConect (std::shared_ptr<const ClientConfiguration> configuration, WSInfoCR info);
        virtual ~WebApiV1BentleyConect ();

        virtual MobileDgn::Utils::AsyncTaskPtr<WSFileResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag = nullptr,
            MobileDgn::Utils::HttpRequest::ProgressCallbackCR downloadProgressCallback = nullptr,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
