/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/iModelBank/StorageServiceClient.h>

#define HEADER_XCorrelationId "X-Correlation-Id"
#define HEADER_TransferEncoding "Transfer-Encoding"
#define HEADER_VALUE_Chunked "Chunked"

const uint32_t StorageServiceClient::Timeout::Connection::Default = 60 * 60;
const uint32_t StorageServiceClient::Timeout::Transfer::FileDownload = 60 * 60;
const uint32_t StorageServiceClient::Timeout::Transfer::Upload = 60 * 60;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
StorageServiceClient::StorageServiceClient(IHttpHandlerPtr httpHandler) :
    m_httpHandler(httpHandler) {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
const IAzureBlobStorageClientFactory StorageServiceClient::Factory = []()
    {
    return std::static_pointer_cast<IAzureBlobStorageClient>(StorageServiceClient::Create(nullptr));
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
StorageServiceClientPtr StorageServiceClient::Create(IHttpHandlerPtr httpHandler)
    {
    return StorageServiceClientPtr(new StorageServiceClient(httpHandler));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void StorageServiceClient::SetCommonRequestOptions
(
Http::RequestR                             request,
IAzureBlobStorageClient::RequestOptionsPtr options,
uint32_t                                   transferTimeout,
ICancellationTokenPtr                      ct
) const
    {
    request.SetRetryOptions(Http::Request::RetryOption::ResumeTransfer, 0);
    request.SetConnectionTimeoutSeconds(StorageServiceClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(transferTimeout);
    request.SetCancellationToken(ct);

    if (nullptr != options && options->GetActivityOptions()->HasActivityId())
        request.GetHeaders().AddValue(HEADER_XCorrelationId, options->GetActivityOptions()->GetActivityId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
AzureResult StorageServiceClient::ResolveFinalResponse(Http::ResponseCR httpResponse) const
    {
    if (!httpResponse.IsSuccess())
        return AzureResult::Error(httpResponse);

    return AzureResult::Success({httpResponse.GetHeaders().GetETag()});
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void StorageServiceClient::FinishProgressCallback(Http::Request::ProgressCallbackCR progressCallback, Http::ResponseCR httpResponse, BeFileNameCR filePath) const
    {
    if (httpResponse.IsSuccess() && progressCallback)
        {
        uint64_t size = 0;
        if (filePath.GetFileSize(size) == BeFileNameStatus::Success)
            progressCallback(size, size);
        else
            progressCallback(1, 1);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<AzureResult> StorageServiceClient::SendGetFileRequest
    (
    Utf8StringCR url,
    BeFileNameCR filePath,
    Http::Request::ProgressCallbackCR progressCallback,
    IAzureBlobStorageClient::RequestOptionsPtr options,
    ICancellationTokenPtr ct
    ) const
    {
    Http::Request request(url, "GET", m_httpHandler);
    request.SetResponseBody(HttpFileBody::Create(filePath));
    SetCommonRequestOptions(request, options, StorageServiceClient::Timeout::Transfer::FileDownload, ct);
    request.SetDownloadProgressCallback(progressCallback);

    return request.PerformAsync()->Then<AzureResult>([=] (Http::ResponseCR httpResponse)
        {
        FinishProgressCallback(progressCallback, httpResponse, filePath);
        return ResolveFinalResponse(httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<AzureResult> StorageServiceClient::SendUpdateFileRequest
    (
    Utf8StringCR url,
    BeFileNameCR filePath,
    Http::Request::ProgressCallbackCR progressCallback,
    IAzureBlobStorageClient::RequestOptionsPtr options,
    ICancellationTokenPtr ct
    ) const
    {
    Http::Request request(url, "PUT", m_httpHandler);
    
    request.GetHeaders().SetValue(HEADER_TransferEncoding, HEADER_VALUE_Chunked);
    request.SetRequestBody(HttpFileBody::Create(filePath));
    SetCommonRequestOptions(request, options, StorageServiceClient::Timeout::Transfer::Upload, ct);
    request.SetUploadProgressCallback(progressCallback);

    return request.PerformAsync()->Then<AzureResult>([=](Http::ResponseCR httpResponse)
        {
        FinishProgressCallback(progressCallback, httpResponse, filePath);
        return ResolveFinalResponse(httpResponse);
        });
    }
