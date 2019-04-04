/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Curl/CurlHttpRequest.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/RefCounted.h>
#include <Bentley/WString.h>
#include <Bentley/bmap.h>
#include <functional>

#include <BeHttp/IHttpHandler.h>
#include <BeHttp/HttpResponse.h>
#include <BeHttp/HttpRequest.h>
#include <Bentley/Tasks/AsyncTask.h>

#include <curl/curl.h>
#include <openssl/ssl.h>

#include "CurlPool.h"

BEGIN_BENTLEY_HTTP_NAMESPACE

USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct CurlHttpRequest
    {
    private:
        struct TransferInfo;
        enum ProgressResult
            {
            Continue = 0,
            Pause = CURL_READFUNC_PAUSE,
            Abort = CURL_READFUNC_ABORT
            };

    private:
        static BeMutex s_transfersCS;
        static bset<TransferInfo*> s_transfers;
        static std::function<void()> s_transfersChangeListener;

        static BeMutex s_numberCS;
        static uint64_t s_number;

        static ICancellationTokenPtr s_globalCt;

    private:
        Request m_httpRequest;
        uint64_t m_number;

        CurlPool&   m_curlPool;
        CURL*       m_curl;
        curl_slist* m_headers;

        std::shared_ptr<TransferInfo> m_transferInfo;

        char m_errorBuffer[CURL_ERROR_SIZE];

    private:
        bool ShouldCompressRequestBody(Request request);

        static void RegisterTransferInfo(TransferInfo* transfer);
        static void UnregisterTransferInfo(TransferInfo* transfer);

        static uint64_t GetNextNumber();
        static Utf8String GetDebugStatusString(ConnectionStatus status, HttpStatus httpStatus);

        static CURLcode CurlSslCtxCallback(CURL* curl, void* sslctx, CurlHttpRequest* request);
        static size_t CurlWriteHeaderCallback(void* buffer, size_t size, size_t count, CurlHttpRequest* request);
        static size_t CurlWriteDataCallback(void* buffer, size_t size, size_t count, CurlHttpRequest* request);
        static size_t CurlReadDataCallback(void* buffer, size_t size, size_t count, CurlHttpRequest* request);
        static int    CurlProgressCallback(CurlHttpRequest* request, double dltotal, double dlnow, double ultotal, double ulnow);
        static int    CurlDebugCallback(CURL* handle, curl_infotype type, char* data, size_t size, CurlHttpRequest* request);

        void SetPrematureError(ConnectionStatus status);
        BentleyStatus SetupCurl ();
        void SetupHeaders();
        void SetupCurlCallbacks();

        ConnectionStatus ResolveConnectionStatus(CURLcode curlStatus);
        HttpStatus ResolveHttpStatus(int httpStatusInt);
        bool GetShouldRetry();

        void SendProgressCallback(double dltotal, double dlnow, double ultotal, double ulnow);

        size_t OnWriteData(void* buffer, size_t size, size_t count);
        size_t OnWriteHeaders(void* buffer, size_t size, size_t count);
        size_t OnReadData(void* buffer, size_t size, size_t count);
        ProgressResult OnProgress(double dltotal, double dlnow, double ultotal, double ulnow);

    public:
        //! Call this method when HttpRequests are stuck. On iOS this happens when application switches from background to foreground.
        static void ResetAllActiveRequests();
        static size_t GetActiveRequestCount();
        static void SetOnActiveRequestCountChanged(std::function<void()> listener);

        static void SetGlobalCancellationToken(ICancellationTokenPtr ct);

        CurlHttpRequest(RequestCR httpRequest, CurlPool& curlPool);
        ~CurlHttpRequest();

        RequestCR GetHttpRequest() const;
        uint64_t GetNumber() const;

        //! Should be called before request execution. GetCurlHandle() should be called after this.
        void PrepareRequest();
        //! Get CURL handle to execute request. 
        //! When non NULL value is returned - CURL handle should be used to execute request and called after FinalizeRequest().
        //! When NULL is returned - ResolveResponse() should be called to handle error.
        CURL* GetCurlHandle();
        //! Should be called after request execution. ShouldRetry() should be called after this.
        void FinalizeRequest(CURLcode curlStatus);
        //! Check if request should be re-executed. ResolveResponse() should be called if false. PrepareRequest() should be called if true.
        bool ShouldRetry();
        //! Get final request result - response
        Response ResolveResponse();
    };

END_BENTLEY_HTTP_NAMESPACE
