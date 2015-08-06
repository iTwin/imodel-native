/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Client/ChunkedUploadRequest.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <MobileDgn/Utils/Http/HttpClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
// Upload big files in resumable/chunked manner. Compatible with BWSG server
struct ChunkedUploadRequest
    {
    public:
        typedef std::function<void(Utf8StringCR etag)> ETagRetrievedCallback;
        typedef const ETagRetrievedCallback& ETagRetrievedCallbackCR;

    private:
        uint64_t m_chunkSizeBytes;

        HttpClient m_client;

        Utf8String m_url;
        Utf8String m_method;

        Utf8String  m_handshakeContentType;
        HttpBodyPtr m_handshakeBody;
        HttpBodyPtr m_mainBody;
        Utf8String  m_mainBodyFileName;
        Utf8String  m_etag;

        ETagRetrievedCallback           m_etagRetrievedCallback;
        ICancellationTokenPtr           m_cancellationToken;
        HttpRequest::ProgressCallback   m_progressCallback;

        struct TransferData;
        std::shared_ptr<TransferData>   m_data;

    private:
        static AsyncTaskPtr<HttpResponse> PerformAsync(std::shared_ptr<ChunkedUploadRequest> cuRequest);
        static AsyncTaskPtr<void> SendHandshakeAndContinue(std::shared_ptr<ChunkedUploadRequest> cuRequest);
        static void SendChunkAndContinue(std::shared_ptr<ChunkedUploadRequest> cuRequest);

    public:
        // Default chunk size in bytes
        WSCLIENT_EXPORT static const uint64_t DefaultChunkSize;

        // Create request with url, specified HTTP method (POST, PUT) and client to use for seperate request creation
        WSCLIENT_EXPORT ChunkedUploadRequest(Utf8StringCR method, Utf8StringCR url, HttpClientCR client);

        // Set body for chunked upload. FileName for Content-Disposition
        WSCLIENT_EXPORT void SetRequestBody(HttpBodyPtr body, Utf8String fileName);

        // Set body for first request. Empty by default
        WSCLIENT_EXPORT void SetHandshakeRequestBody(HttpBodyPtr body, Utf8StringCR contentType);

        // Set custom chunk size. Default is ChunkedUploadRequest::DefaultChunkSize
        WSCLIENT_EXPORT void SetChunkSize(uint64_t chunkSizeBytes);

        // Set ETag from previously interrupted upload to resume it
        WSCLIENT_EXPORT void SetETag(Utf8StringCR etag);

        // When called, save ETag in order to resume upload later
        WSCLIENT_EXPORT void SetETagRetrievedCallback(ETagRetrievedCallbackCR etagCallback);

        // Return true from callback when request needs to be canceled
        WSCLIENT_EXPORT void SetCancellationToken(ICancellationTokenPtr token);

        // Progress callback for whole upload
        WSCLIENT_EXPORT void SetUploadProgressCallback(HttpRequest::ProgressCallbackCR onProgress);

        // Send required requests and return final response
        WSCLIENT_EXPORT AsyncTaskPtr<HttpResponse> PerformAsync();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
