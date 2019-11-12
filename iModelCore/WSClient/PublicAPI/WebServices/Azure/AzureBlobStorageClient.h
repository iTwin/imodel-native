/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
        struct RequestOptions;
        typedef std::shared_ptr<RequestOptions> RequestOptionsPtr;
        struct ActivityOptions;
        typedef std::shared_ptr<ActivityOptions> ActivityOptionsPtr;

        WSCLIENT_EXPORT virtual ~IAzureBlobStorageClient();

        virtual AsyncTaskPtr<AzureResult> SendGetFileRequest
            (
            Utf8StringCR url,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR progressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<AzureResult> SendUpdateFileRequest
            (
            Utf8StringCR url,
            BeFileNameCR filePath,
            Http::Request::ProgressCallbackCR progressCallback = nullptr,
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Mantas.Smicius    11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
//! Request options that can be passed to individual requests to define their properties
struct IAzureBlobStorageClient::RequestOptions
    {
    private:
        ActivityOptionsPtr m_activityOptions;

    public:
        RequestOptions() : m_activityOptions(std::make_shared<ActivityOptions>()) {}
        virtual ~RequestOptions() {}

        //! Retrieve options for activity
        ActivityOptionsPtr GetActivityOptions() { return m_activityOptions; }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Mantas.Smicius    11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct IAzureBlobStorageClient::ActivityOptions
    {
    private:
        Utf8String m_activityId;

    public:
        //! Set activity id for all Http requests in current Api method.
        //! If activity id is not set then it won't be included in Http requests
        void SetActivityId(Utf8StringCR activityId) { m_activityId = activityId; }

        //! Get activity id for all Http requests in current Api method
        Utf8StringCR GetActivityId() const { return m_activityId; }

        //! Check if activity id is set
        bool HasActivityId() const { return !m_activityId.empty(); }
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
        static void SetActivityIdToRequest(Http::RequestR request, RequestOptionsPtr options);
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
            RequestOptionsPtr options,
            ICancellationTokenPtr ct
            ) const;
        AsyncTaskPtr<AzureResult> SendAsOneChunk
            (
            Utf8StringCR url,
            HttpBodyPtr httpBody,
            uint64_t fileSize,
            Http::Request::ProgressCallbackCR progressCallback,
            RequestOptionsPtr options,
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
            RequestOptionsPtr options = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) const override;

        //! Update file in Azure blob. Does chunked upload in blocks.
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
