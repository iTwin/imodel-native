/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/CancellationToken.h>
#include <WebServices/Azure/AzureBlobStorageClient.h>
#include <BeHttp/HttpRequest.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_TASKS

//--------------------------------------------------------------------------------------+
// WebServices Client API for connecting to Azure blob storage.
//--------------------------------------------------------------------------------------+
typedef std::shared_ptr<struct StorageServiceClient> StorageServiceClientPtr;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
struct StorageServiceClient : public IAzureBlobStorageClient
{
private:
    const IHttpHandlerPtr m_httpHandler;

    void SetCommonRequestOptions
        (
        Http::RequestR                             request,
        IAzureBlobStorageClient::RequestOptionsPtr options,
        uint32_t                                   transferTimeout,
        ICancellationTokenPtr                      ct
        ) const;
    AzureResult ResolveFinalResponse(Http::ResponseCR httpResponse) const;
    StorageServiceClient(IHttpHandlerPtr httpHandler);
public:
    struct Timeout
        {
        struct Connection
            {
            static const uint32_t Default;
            };
        struct Transfer
            {
            static const uint32_t FileDownload;
            static const uint32_t Upload;
            };
        };

    //! Factory for creating this client instances.
    WSCLIENT_EXPORT static const IAzureBlobStorageClientFactory Factory;

    //! Creates instance of this storage client.
    WSCLIENT_EXPORT static StorageServiceClientPtr Create(IHttpHandlerPtr httpHandler);

    //! Download file from Azure blob that is stored locally
    WSCLIENT_EXPORT AsyncTaskPtr<AzureResult> SendGetFileRequest
        (
        Utf8StringCR url,
        BeFileNameCR filePath,
        Http::Request::ProgressCallbackCR progressCallback = nullptr,
        RequestOptionsPtr options = nullptr,
        ICancellationTokenPtr ct = nullptr
        ) const override;

    //! Update file in Azure blob that is stored locally.
    WSCLIENT_EXPORT AsyncTaskPtr<AzureResult> SendUpdateFileRequest
        (
        Utf8StringCR url,
        BeFileNameCR filePath,
        Http::Request::ProgressCallbackCR progressCallback = nullptr,
        RequestOptionsPtr options = nullptr,
        ICancellationTokenPtr ct = nullptr
        ) const override;
};

END_BENTLEY_WEBSERVICES_NAMESPACE
