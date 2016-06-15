/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Curl/CurlHttpRequest.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
        static BeMutex s_numberCS;
        static uint64_t s_number;

    private:
        HttpRequest m_httpRequest;
        uint64_t m_number;

        CurlPool&   m_curlPool;
        CURL*       m_curl;
        curl_slist* m_headers;

        std::shared_ptr<TransferInfo> m_transferInfo;

    private:
        static void RegisterTransferInfo (TransferInfo* transfer);
        static void UnregisterTransferInfo (TransferInfo* transfer);
        static uint64_t GetNextNumber ();

        static size_t CurlWriteHeaderCallback (void* buffer, size_t size, size_t count, CurlHttpRequest* request);
        static size_t CurlWriteDataCallback (void* buffer, size_t size, size_t count, CurlHttpRequest* request);
        static size_t CurlReadDataCallback (void* buffer, size_t size, size_t count, CurlHttpRequest* request);
        static int    CurlProgressCallback (CurlHttpRequest* request, double dltotal, double dlnow, double ultotal, double ulnow);
        static int    CurlDebugCallback(CURL* handle, curl_infotype type, char* data, size_t size, CurlHttpRequest* request);

        void SetupCurl ();
        void SetupHeaders ();
        void SetupCurlCallbacks ();

        HttpStatus ResolveHttpStatus (int httpStatusInt);

        void SendProgressCallback (double dltotal, double dlnow, double ultotal, double ulnow);

        size_t OnWriteData (void* buffer, size_t size, size_t count);
        size_t OnWriteHeaders (void* buffer, size_t size, size_t count);
        size_t OnReadData (void* buffer, size_t size, size_t count);
        ProgressResult OnProgress (double dltotal, double dlnow, double ultotal, double ulnow);

    public:
        //! Call this method when HttpRequests are stuck. On iOS this happens when application switches from background to foreground.
        static void ResetAllRequests ();

        CurlHttpRequest (HttpRequestCR httpRequest, CurlPool& curlPool);
        ~CurlHttpRequest ();

        CURL* GetCurlHandle ();
        HttpRequestCR GetHttpRequest () const;
        uint64_t GetNumber () const;

        void PrepareRequest ();

        ConnectionStatus GetConnectionStatus (CURLcode curlStatus);

        bool ShouldRetry (ConnectionStatus curlStatus);

        HttpResponse ResolveResponse (ConnectionStatus curlStatus);

        static Utf8String GetDebugStatusString (ConnectionStatus status, HttpStatus httpStatus);
    };

END_BENTLEY_HTTP_NAMESPACE
