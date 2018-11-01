/*--------------------------------------------------------------------------------------+
|
|     $Source: Azure/AzureBlobStorageClient.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Azure/AzureBlobStorageClient.h>
#include <iomanip>
#include <Bentley/Base64Utilities.h>

const uint32_t AzureBlobStorageClient::Timeout::Connection::Default = 30;
const uint32_t AzureBlobStorageClient::Timeout::Transfer::FileDownload = 30;
const uint32_t AzureBlobStorageClient::Timeout::Transfer::Upload = 30;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Andrius.Zonys   01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AzureBlobStorageClient::AzureBlobStorageClient(IHttpHandlerPtr customHandler) :
m_customHandler(customHandler)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Andrius.Zonys   01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IAzureBlobStorageClient::~IAzureBlobStorageClient()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Andrius.Zonys   01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AzureBlobStorageClientPtr AzureBlobStorageClient::Create(IHttpHandlerPtr customHandler)
    {
    return AzureBlobStorageClientPtr(new AzureBlobStorageClient(customHandler));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Andrius.Zonys   03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SetCommonRequestOptions
(
Http::RequestR                    request,
Http::Request::RetryOption        retryOption,
uint32_t                          transferType,
Http::Request::ProgressCallbackCR downloadProgressCallback,
Http::Request::ProgressCallbackCR uploadProgressCallback,
ICancellationTokenPtr             ct
)
    {
    request.SetRetryOptions(retryOption, 0);
    request.SetConnectionTimeoutSeconds(AzureBlobStorageClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(transferType);
    request.SetCancellationToken(ct);
    request.SetDownloadProgressCallback(downloadProgressCallback);
    request.SetUploadProgressCallback(uploadProgressCallback);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AzureResult AzureBlobStorageClient::ResolveFinalResponse(Http::ResponseCR httpResponse)
    {
    if (!httpResponse.IsSuccess())
        return AzureResult::Error(httpResponse);

    return AzureResult::Success({httpResponse.GetHeaders().GetETag()});
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Andrius.Zonys   01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<AzureResult> AzureBlobStorageClient::SendGetFileRequest
(
Utf8StringCR url,
BeFileNameCR filePath,
Http::Request::ProgressCallbackCR progressCallback,
ICancellationTokenPtr ct
) const
    {
    Http::Request request(url, "GET", m_customHandler);

    request.SetResponseBody(HttpFileBody::Create(filePath));
    SetCommonRequestOptions(request, Http::Request::RetryOption::ResumeTransfer, AzureBlobStorageClient::Timeout::Transfer::FileDownload, progressCallback, nullptr, ct);

    return request.PerformAsync()
        ->Then<AzureResult>([=] (Http::ResponseCR httpResponse)
        {
        return ResolveFinalResponse(httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Andrius.Zonys   01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<AzureResult> AzureBlobStorageClient::SendChunkAndContinue
(
Utf8StringCR url,
Utf8String blockIds,
HttpBodyPtr body,
uint64_t fileSize,
uint64_t chunkSize,
int chunkNumber,
Http::Request::ProgressCallbackCR progressCallback,
ICancellationTokenPtr ct
) const
    {
    std::stringstream blockIdStream;
    blockIdStream << std::setw(5) << std::setfill('0') << chunkNumber;
    std::string blockId = blockIdStream.str();
    Utf8String encodedBlockId = Base64Utilities::Encode(blockId.c_str()).c_str();
    blockIds += Utf8PrintfString("<Latest>%s</Latest>", encodedBlockId.c_str());

    // Update URL
    Utf8String blockUrl = Utf8PrintfString("%s&comp=block&blockid=%s", url.c_str(), encodedBlockId.c_str());
    uint64_t bytesTo = chunkSize * chunkNumber + chunkSize - 1; // -1 because ranges are inclusive.
    if (bytesTo >= fileSize)
        bytesTo = fileSize - 1;

    auto onBlockProgress = [=] (double bytesTransfered, double bytesTotal)
        {
        if (progressCallback)
            progressCallback(chunkNumber * (double) chunkSize + bytesTransfered, (double) fileSize);
        };

    Http::Request request(blockUrl, "PUT", m_customHandler);
    request.GetHeaders().SetValue("x-ms-blob-type", "BlockBlob");
    request.SetRequestBody(HttpRangeBody::Create(body, chunkSize * chunkNumber, bytesTo));
    SetCommonRequestOptions(request, Http::Request::RetryOption::ResetTransfer, AzureBlobStorageClient::Timeout::Transfer::Upload, nullptr, onBlockProgress, ct);

    std::shared_ptr<AzureResult> finalResult = std::make_shared<AzureResult>();
    return request.PerformAsync()
        ->Then([=] (Http::ResponseCR httpResponse)
        {
        if (!httpResponse.IsSuccess())
            {
            finalResult->SetError(httpResponse);
            return;
            }

        if (chunkNumber + 1 < ceil(fileSize / (double) chunkSize)) // Cast is required to get double result
            {
            SendChunkAndContinue(url, blockIds, body, fileSize, chunkSize, chunkNumber + 1, progressCallback, ct)
                ->Then([=] (const AzureResult& result)
                {
                *finalResult = result;
                });
            return;
            }

        if (progressCallback)
            progressCallback((double) fileSize, (double) fileSize);

        Utf8String finalBody = Utf8PrintfString("<?xml version=\"1.0\" encoding=\"utf-8\"?><BlockList>%s</BlockList>", blockIds.c_str());
        Utf8String blockListUrl = Utf8PrintfString("%s&comp=blocklist", url.c_str());

        Http::Request finalRequest(blockListUrl, "PUT", m_customHandler);
        finalRequest.SetRequestBody(HttpStringBody::Create(finalBody));
        SetCommonRequestOptions(finalRequest, Http::Request::RetryOption::ResetTransfer, AzureBlobStorageClient::Timeout::Transfer::Upload, nullptr, nullptr, ct);

        finalRequest.PerformAsync()
            ->Then([=] (Http::ResponseCR httpResponse)
            {
            *finalResult = ResolveFinalResponse(httpResponse);
            });
        })->Then<AzureResult>([=]
            {
            return *finalResult;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Andrius.Zonys   01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<AzureResult> AzureBlobStorageClient::SendUpdateFileRequest
(
Utf8StringCR url,
BeFileNameCR filePath,
Http::Request::ProgressCallbackCR progressCallback,
ICancellationTokenPtr ct
) const
    {
    BeFile file;
    file.Open(filePath, BeFileAccess::Read);

    uint64_t fileSize;
    if (BeFileStatus::Success != file.GetSize(fileSize))
        {
        Http::Response response(HttpResponseContent::Create(HttpStringBody::Create("Invalid file.")), "", ConnectionStatus::None, HttpStatus::BadRequest);
        return CreateCompletedAsyncTask<AzureResult>(AzureResult::Error(response));
        }
    file.Close();

    uint64_t chunkSize = 4 * 1024 * 1024;   // Max 4MB.

    HttpBodyPtr body = HttpFileBody::Create(filePath);
    Utf8String blockIds = "";

    if (progressCallback)
        progressCallback(0, (double) fileSize);

    return SendChunkAndContinue(url, blockIds, body, fileSize, chunkSize, 0, progressCallback, ct);
    }
