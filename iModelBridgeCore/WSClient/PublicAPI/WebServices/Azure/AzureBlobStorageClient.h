/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Azure/AzureBlobStorageClient.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "../Client/WebServicesClient.h"
#include <WebServices/Azure/AzureError.h>
#include <BeHttp/HttpResponse.h>
#include <Bentley/Tasks/AsyncResult.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

//--------------------------------------------------------------------------------------+
// WebServices Client API for connecting to Azure blob storage.
//--------------------------------------------------------------------------------------+
typedef std::shared_ptr<struct IAzureBlobStorageClient> IAzureBlobStorageClientPtr;
typedef std::shared_ptr<struct AzureBlobStorageClient> AzureBlobStorageClientPtr;
// Return success values or error Http::Response. Convert Http::Response to HttpError if simple information is needed.
typedef AsyncResult<struct AzureFileResponse, AzureError> AzureResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct AzureFileResponse
    {
    private:
        Utf8String m_eTag;

    public:
        AzureFileResponse() {};
        AzureFileResponse(Utf8String eTag) : m_eTag(eTag) {};
        Utf8String GetETag() const { return m_eTag; };
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Andrius.Zonys   01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct IAzureBlobStorageClient
    {
    public:
        WSCLIENT_EXPORT virtual ~IAzureBlobStorageClient();

        virtual AsyncTaskPtr<AzureResult> SendGetFileRequest
            (
            Utf8StringCR url,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR progressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<AzureResult> SendUpdateFileRequest
            (
            Utf8StringCR url,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR progressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Andrius.Zonys   01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct AzureBlobStorageClient : public IAzureBlobStorageClient
    {
    private:
        IHttpHandlerPtr m_customHandler;

    private:
        AzureBlobStorageClient(IHttpHandlerPtr customHandler);
        static AzureResult ResolveFinalResponse(Http::ResponseCR httpResponse);
        AsyncTaskPtr<AzureResult> SendChunkAndContinue
            (
            Utf8StringCR url,
            Utf8String blockIds,
            HttpBodyPtr httpBody,
            uint64_t fileSize,
            uint64_t chunkSize,
            int chunkNumber,
            Http::Request::ProgressCallbackCR progressCallback,
            ICancellationTokenPtr ct
            ) const;

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

        //! @param[in] customHandler - custom http handler for testing purposes.
        WSCLIENT_EXPORT static AzureBlobStorageClientPtr Create
            (
            IHttpHandlerPtr customHandler = nullptr
            );

        //! Download file from Azure blob
        WSCLIENT_EXPORT AsyncTaskPtr<AzureResult> SendGetFileRequest
            (
            Utf8StringCR url,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR progressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        //! Update file in Azure blob. Does chunked upload in blocks.
        WSCLIENT_EXPORT AsyncTaskPtr<AzureResult> SendUpdateFileRequest
            (
            Utf8StringCR url,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR progressCallback = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
