/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WebApi/WebApiV1BentleyConnect.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "WebApiV1BentleyConnect.h"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WebApiV1BentleyConect::WebApiV1BentleyConect(std::shared_ptr<const ClientConfiguration> configuration, WSInfoCR info) :
WebApiV1(configuration, info)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WebApiV1BentleyConect::~WebApiV1BentleyConect()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool WebApiV1BentleyConect::IsSupported(WSInfoCR info)
    {
    return info.GetWebApiVersion() == BeVersion(1, 1) && info.GetType() == WSInfo::Type::BentleyConnect;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WebApiV1BentleyConect::GetSchemaUrl() const
    {
    return GetUrl(SERVICE_Schema, "", nullptr, "v1.1");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSFileResult> WebApiV1BentleyConect::SendGetFileRequest
(
ObjectIdCR objectId,
BeFileNameCR filePath,
Utf8StringCR eTag,
HttpRequest::ProgressCallbackCR downloadProgressCallback,
ICancellationTokenPtr cancellationToken
) const
    {
    if (SchemaInfo::IsDummySchemaId(objectId))
        {
        return GetSchema(filePath, eTag, downloadProgressCallback, cancellationToken);
        }

    Utf8String url = GetUrl(SERVICE_Files, CreateObjectIdParam(objectId));
    HttpRequest request = m_configuration->GetHttpClient().CreateGetRequest(url);

    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::FileDownload);
    request.SetCancellationToken(cancellationToken);

    request.SetFollowRedirects(false);

    auto result = std::make_shared<WSFileResult>();

    return request.PerformAsync()
        ->Then([=] (HttpResponse& redirectResponse)
        {
        Utf8CP location = redirectResponse.GetHeaders().GetLocation();

        if (redirectResponse.GetHttpStatus() != HttpStatus::Found ||
            Utf8String::IsNullOrEmpty(location))
            {
            result->SetError(redirectResponse);
            return;
            }

        HttpRequest fileRequest = m_configuration->GetHttpClient().CreateGetRequest(location);

        fileRequest.SetResponseBody(HttpFileBody::Create(filePath));
        fileRequest.SetRetryOptions(HttpRequest::ResumeTransfer, 0);
        fileRequest.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
        fileRequest.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::FileDownload);
        fileRequest.SetDownloadProgressCallback(downloadProgressCallback);
        fileRequest.SetCancellationToken(cancellationToken);
        fileRequest.GetHeaders().SetIfNoneMatch(eTag);

        fileRequest.PerformAsync()
            ->Then([result, filePath] (HttpResponse& fileResponse)
            {
            *result = ResolveFileResponse(fileResponse, filePath);
            });
        })
            ->Then<WSFileResult>([=]
            {
            return *result;
            });
    }
