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

USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WebApiV1BentleyConect : public WebApiV1
    {
    protected:
        virtual Utf8String GetSchemaUrl() const override;

    public:
        WebApiV1BentleyConect(std::shared_ptr<const ClientConfiguration> configuration, WSInfoCR info);
        virtual ~WebApiV1BentleyConect();

        static bool IsSupported(WSInfoCR info);

        virtual AsyncTaskPtr<WSFileResult> SendGetFileRequest
            (
            ObjectIdCR objectId,
            BeFileNameCR filePath,
            Utf8StringCR eTag = nullptr,
            HttpRequest::ProgressCallbackCR downloadProgressCallback = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
