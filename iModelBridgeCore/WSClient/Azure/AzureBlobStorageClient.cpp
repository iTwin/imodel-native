/*--------------------------------------------------------------------------------------+
|
|     $Source: Azure/AzureBlobStorageClient.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
IAzureBlobStorageClient::~IAzureBlobStorageClient()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Andrius.Zonys   01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AzureBlobStorageClient::AzureBlobStorageClient(IHttpHandlerPtr customHandler) :
m_customHandler(customHandler)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Andrius.Zonys   01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<AzureBlobStorageClient> AzureBlobStorageClient::Create
(
IHttpHandlerPtr customHandler
)
    {
    return std::shared_ptr<AzureBlobStorageClient>(new AzureBlobStorageClient(customHandler));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Andrius.Zonys   03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SetCommonRequestOptions
(
HttpRequestR                    request,
HttpRequest::RetryOption        retryOption,
uint32_t                        transferType,
HttpRequest::ProgressCallbackCR progressCallback,
ICancellationTokenPtr           ct
)
    {
    request.SetRetryOptions(retryOption, 0);
    request.SetConnectionTimeoutSeconds(AzureBlobStorageClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(transferType);
    request.SetCancellationToken(ct);
    request.SetUploadProgressCallback(progressCallback);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Andrius.Zonys   01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<AzureResult> AzureBlobStorageClient::SendGetFileRequest
(
Utf8StringCR url,
BeFileNameCR filePath,
HttpRequest::ProgressCallbackCR progressCallback,
ICancellationTokenPtr ct
) const
    {
    HttpRequest request(url, "GET", m_customHandler);

    request.SetResponseBody(HttpFileBody::Create(filePath));
    SetCommonRequestOptions(request, HttpRequest::ResumeTransfer, AzureBlobStorageClient::Timeout::Transfer::FileDownload, progressCallback, ct);

    return request.PerformAsync()
        ->Then<AzureResult>([=] (HttpResponse& httpResponse)
        {
        if (httpResponse.IsSuccess ())
            return AzureResult::Success();
        else
            return AzureResult::Error(httpResponse);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Andrius.Zonys   01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<AzureResult> AzureBlobStorageClient::SendChunkAndContinue
(
Utf8StringCR url,
Utf8String blockIds,
HttpBodyPtr httpBody,
uint64_t fileSize,
uint64_t chunkSize,
int chunkNumber,
HttpRequest::ProgressCallbackCR progressCallback,
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

    HttpRequest request(blockUrl, "PUT", m_customHandler);
    request.GetHeaders().SetValue("x-ms-blob-type", "BlockBlob");
    request.SetRequestBody(HttpRangeBody::Create(httpBody, chunkSize * chunkNumber, bytesTo));
    SetCommonRequestOptions(request, HttpRequest::ResetTransfer, AzureBlobStorageClient::Timeout::Transfer::Upload, progressCallback, ct);

    std::shared_ptr<AzureResult> finalResult = std::make_shared<AzureResult>();
    return request.PerformAsync()
        ->Then([=] (const HttpResponse& httpResponse)
        {
        if (!httpResponse.IsSuccess())
            {
            finalResult->SetError (httpResponse);
            return;
            }

        if (chunkNumber + 1 < ceil((double) fileSize / chunkSize))
            {
            SendChunkAndContinue(url, blockIds, httpBody, fileSize, chunkSize, chunkNumber + 1, progressCallback, ct)
                ->Then([=] (const AzureResult& result)
                {
                if (result.IsSuccess())
                    finalResult->SetSuccess ();
                else
                    finalResult->SetError(result.GetError());
                });
            return;
            }

        Utf8String finalBody = Utf8PrintfString("<?xml version=\"1.0\" encoding=\"utf-8\"?><BlockList>%s</BlockList>", blockIds.c_str());
        Utf8String blockListUrl = Utf8PrintfString("%s&comp=blocklist", url.c_str());

        HttpRequest finalRequest(blockListUrl, "PUT", m_customHandler);
        finalRequest.SetRequestBody(HttpStringBody::Create(finalBody));
        SetCommonRequestOptions(finalRequest, HttpRequest::ResetTransfer, AzureBlobStorageClient::Timeout::Transfer::Upload, progressCallback, ct);

        finalRequest.PerformAsync()
            ->Then([=] (const HttpResponse& httpResponse)
            {
            if (httpResponse.IsSuccess())
                finalResult->SetSuccess();
            else
                finalResult->SetError (httpResponse);
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
HttpRequest::ProgressCallbackCR progressCallback,
ICancellationTokenPtr ct
) const
    {
    BeFile beFile;
    beFile.Open(filePath, BeFileAccess::Read);

    uint64_t fileSize;
    if (BeFileStatus::Success != beFile.GetSize(fileSize))
        {
        HttpResponse response(HttpResponseContent::Create(HttpStringBody::Create("Invalid file.")), "", ConnectionStatus::None, HttpStatus::BadRequest);
        return CreateCompletedAsyncTask<AzureResult>(AzureResult::Error(HttpError(response)));
        }
    beFile.Close();

    uint64_t chunkSize = 4 * 1024 * 1024;   // Max 4MB.

    HttpBodyPtr httpBody = HttpFileBody::Create(filePath);
    Utf8String blockIds = "";

    return SendChunkAndContinue(url, blockIds, httpBody, fileSize, chunkSize, 0, progressCallback, ct);
    }
