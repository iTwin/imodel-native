/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Client/ChunkedUploadRequest.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "ChunkedUploadRequest.h"

const uint64_t ChunkedUploadRequest::DefaultChunkSize = 4 * 1024 * 1024;

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChunkedUploadRequest::TransferData
    {
    public:
        HttpResponse response;
        uint64_t contentLength;
        uint64_t rangeFrom;
        HttpRequest::ProgressCallback onProgress;
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ChunkedUploadRequest::ChunkedUploadRequest(Utf8StringCR method, Utf8StringCR url, HttpClientCR client) :
m_handshakeRequest(client.CreateRequest(url, method)),
m_client(client),
m_method(method),
m_url(url),

m_handshakeBody(nullptr),
m_mainBody(nullptr),

m_chunkSizeBytes(ChunkedUploadRequest::DefaultChunkSize)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ChunkedUploadRequest::SetHandshakeRequestBody(HttpBodyPtr body, Utf8StringCR contentType)
    {
    m_handshakeBody = body;
    m_handshakeContentType = contentType;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ChunkedUploadRequest::SetRequestBody(HttpBodyPtr body, Utf8String fileName)
    {
    m_mainBody = body;
    m_mainBodyFileName = fileName;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ChunkedUploadRequest::SetChunkSize(uint64_t chunkSizeBytes)
    {
    m_chunkSizeBytes = chunkSizeBytes;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ChunkedUploadRequest::SetETag(Utf8StringCR etag)
    {
    m_etag = etag;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ChunkedUploadRequest::SetETagRetrievedCallback(ETagRetrievedCallbackCR etagCallback)
    {
    m_etagRetrievedCallback = etagCallback;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ChunkedUploadRequest::SetCancellationToken(ICancellationTokenPtr token)
    {
    m_cancellationToken = token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ChunkedUploadRequest::SetUploadProgressCallback(HttpRequest::ProgressCallbackCR onProgress)
    {
    m_progressCallback = onProgress;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequest& ChunkedUploadRequest::GetHandshakeRequest()
    {
    return m_handshakeRequest;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<HttpResponse> ChunkedUploadRequest::PerformAsync()
    {
    return PerformAsync(std::make_shared<ChunkedUploadRequest>(*this));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<HttpResponse> ChunkedUploadRequest::PerformAsync(std::shared_ptr<ChunkedUploadRequest> cuRequest)
    {
    cuRequest->m_data = std::make_shared<TransferData>();
    if (cuRequest->m_progressCallback)
        {
        cuRequest->m_progressCallback(0, 0);
        }
    return SendHandshakeAndContinue(cuRequest)->Then<HttpResponse>([=]
        {
        auto data = cuRequest->m_data;
        cuRequest->m_data = nullptr;
        return data->response;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> ChunkedUploadRequest::SendHandshakeAndContinue(std::shared_ptr<ChunkedUploadRequest> cuRequest)
    {
    HttpRequest request = cuRequest->m_handshakeRequest;

    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::Upload);

    // Setup Query/Handshake request
    if (!cuRequest->m_mainBody.IsNull())
        { 
        request.GetHeaders().SetIfMatch(cuRequest->m_etag);
        request.GetHeaders().SetContentRange(Utf8PrintfString("bytes */%llu", cuRequest->m_mainBody->GetLength()).c_str());

        // Format based on https://www.ietf.org/rfc/rfc1806.txt, https://tools.ietf.org/html/rfc6266#ref-ISO-8859-1
        request.GetHeaders().SetContentDisposition(Utf8PrintfString("attachment; filename=\"%s\"", cuRequest->m_mainBodyFileName.c_str()));
        }

    // Setup Handshake request
    if (cuRequest->m_etag.empty())
        {
        request.GetHeaders().SetContentType(cuRequest->m_handshakeContentType);
        request.SetRequestBody(cuRequest->m_handshakeBody);
        }

    request.SetCancellationToken(cuRequest->m_cancellationToken);

    LOG.debugv("ChunkedUpload::SendHandshake %s", cuRequest->m_etag.c_str());
    return request.PerformAsync()
        ->Then([=] (HttpResponse& response)
        {
        cuRequest->m_data->response = response;
        if (response.GetHttpStatus() != HttpStatus::ResumeIncomplete || cuRequest->m_mainBody.IsNull())
            {
            // End request
            return;
            }

        cuRequest->m_etag = response.GetHeaders().GetETag();
        if (cuRequest->m_etagRetrievedCallback)
            {
            cuRequest->m_etagRetrievedCallback(cuRequest->m_etag);
            }

        // Setup Upload request
        cuRequest->m_data->contentLength = cuRequest->m_mainBody->GetLength();
        cuRequest->m_data->rangeFrom = 0;

        cuRequest->m_data->onProgress =
            [=] (double bytesTransfered, double bytesTotal)
            {
            if (cuRequest->m_progressCallback)
                {
                cuRequest->m_progressCallback((double) cuRequest->m_data->rangeFrom + bytesTransfered, (double) cuRequest->m_data->contentLength);
                }
            };

        SendChunkAndContinue(cuRequest);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChunkedUploadRequest::SendChunkAndContinue(std::shared_ptr<ChunkedUploadRequest> cuRequest)
    {
    cuRequest->m_data->rangeFrom = 0;
    Utf8CP rangeHeader = cuRequest->m_data->response.GetHeaders().GetRange();
    if (nullptr != rangeHeader)
        {
        RangeHeaderValue range;
        if (SUCCESS != RangeHeaderValue::Parse(rangeHeader, range))
            {
            BeAssert(false);
            cuRequest->m_data->response = HttpResponse();
            // End request
            return;
            }
        cuRequest->m_data->rangeFrom = range.to + 1;
        }

    uint64_t rangeTo = cuRequest->m_data->rangeFrom + cuRequest->m_chunkSizeBytes - 1;
    if (rangeTo >= cuRequest->m_data->contentLength)
        {
        rangeTo = cuRequest->m_data->contentLength - 1;
        }

    HttpRequest request = cuRequest->m_client.CreateRequest(cuRequest->m_url, cuRequest->m_method);

    // TODO VRA: calling ResetConnection() is a workaround to what seems to be CURL bug:
    // (logged as https://sourceforge.net/p/curl/bugs/1275/)
    // Sending multiple upload requests using same CURL handle with curl_easy_reset causes it to stuck on specific number of chunks sent.
    // That causes timeout and CURL connection reset until same pattern happens again.
    // 4MB requests stuck on every second request.
    request.SetUseNewConnection(true);

    request.GetHeaders().SetIfMatch(cuRequest->m_etag);
    request.GetHeaders().SetContentRange(Utf8PrintfString("bytes %llu-%llu/%llu", cuRequest->m_data->rangeFrom, rangeTo, cuRequest->m_data->contentLength));
    request.SetRequestBody(HttpRangeBody::Create(cuRequest->m_mainBody, cuRequest->m_data->rangeFrom, rangeTo));

    request.SetCancellationToken(cuRequest->m_cancellationToken);
    request.SetUploadProgressCallback(cuRequest->m_data->onProgress);

    request.SetRetryOptions(HttpRequest::RetryOption::ResetTransfer, 0);

    request.SetConnectionTimeoutSeconds(WSRepositoryClient::Timeout::Connection::Default);
    request.SetTransferTimeoutSeconds(WSRepositoryClient::Timeout::Transfer::Upload);

    LOG.debugv("ChunkedUpload::SendChunk %s", cuRequest->m_etag.c_str());
    request.PerformAsync()->Then([=] (HttpResponse& response)
        {
        cuRequest->m_data->response = response;
        if (response.GetHttpStatus() == HttpStatus::ResumeIncomplete)
            {
            SendChunkAndContinue(cuRequest);
            return;
            }

        cuRequest->m_etag.clear();
        if (response.GetHttpStatus() == HttpStatus::PreconditionFailed)
            {
            SendHandshakeAndContinue(cuRequest);
            return;
            }
        // End request
        });
    }
