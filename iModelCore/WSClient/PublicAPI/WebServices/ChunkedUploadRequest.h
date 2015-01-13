/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/ChunkedUploadRequest.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Common.h>
#include <MobileDgn/Utils/Http/HttpClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
// Upload big files in resumable/chunked manner. Compatible with BWSG server
struct ChunkedUploadRequest
    {
    public:
        typedef std::function<void (Utf8StringCR etag)> ETagRetrievedCallback;
        typedef const ETagRetrievedCallback& ETagRetrievedCallbackCR;

    private:
        uint64_t m_chunkSizeBytes;

        MobileDgn::Utils::HttpClient m_client;

        Utf8String m_url;
        Utf8String m_method;

        Utf8String  m_handshakeContentType;
        MobileDgn::Utils::HttpBodyPtr m_handshakeBody;
        MobileDgn::Utils::HttpBodyPtr m_mainBody;
        Utf8String  m_mainBodyFileName;
        Utf8String  m_etag;

        ETagRetrievedCallback           m_etagRetrievedCallback;
        MobileDgn::Utils::ICancellationTokenPtr           m_cancellationToken;
        MobileDgn::Utils::HttpRequest::ProgressCallback   m_progressCallback;

        struct TransferData;
        std::shared_ptr<TransferData>   m_data;

    private:
        static MobileDgn::Utils::AsyncTaskPtr<MobileDgn::Utils::HttpResponse> PerformAsync (std::shared_ptr<ChunkedUploadRequest> cuRequest);
        static MobileDgn::Utils::AsyncTaskPtr<void> SendHandshakeAndContinue (std::shared_ptr<ChunkedUploadRequest> cuRequest);
        static void SendChunkAndContinue (std::shared_ptr<ChunkedUploadRequest> cuRequest);

    public:
        // Default chunk size in bytes
        WS_EXPORT static const uint64_t DefaultChunkSize;

        // Create request with url, specified HTTP method (POST, PUT) and client to use for seperate request creation
        WS_EXPORT ChunkedUploadRequest (Utf8StringCR method, Utf8StringCR url, MobileDgn::Utils::HttpClientCR client);

        // Set body for chunked upload. FileName for Content-Disposition 
        WS_EXPORT void SetRequestBody (MobileDgn::Utils::HttpBodyPtr body, Utf8String fileName);

        // Set body for first request. Empty by default
        WS_EXPORT void SetHandshakeRequestBody (MobileDgn::Utils::HttpBodyPtr body, Utf8StringCR contentType);

        // Set custom chunk size. Default is ChunkedUploadRequest::DefaultChunkSize
        WS_EXPORT void SetChunkSize (uint64_t chunkSizeBytes);

        // Set ETag from previously interrupted upload to resume it
        WS_EXPORT void SetETag (Utf8StringCR etag);

        // When called, save ETag in order to resume upload later
        WS_EXPORT void SetETagRetrievedCallback (ETagRetrievedCallbackCR etagCallback);

        // Return true from callback when request needs to be canceled
        WS_EXPORT void SetCancellationToken (MobileDgn::Utils::ICancellationTokenPtr token);

        // Progress callback for whole upload
        WS_EXPORT void SetUploadProgressCallback (MobileDgn::Utils::HttpRequest::ProgressCallbackCR onProgress);

        // Send required requests and return final response
        WS_EXPORT MobileDgn::Utils::AsyncTaskPtr<MobileDgn::Utils::HttpResponse> PerformAsync ();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
