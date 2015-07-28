/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Network/ResponseGuard.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "ResponseGuard.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ResponseGuard::ResponseGuard(ICancellationTokenPtr tokenToWrap, HttpRequest::ProgressCallbackCR onProgress) :
m_tokenEnabled(true),
m_token(tokenToWrap),
m_onProgress(onProgress)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ResponseGuardPtr ResponseGuard::Create(ICancellationTokenPtr tokenToWrap, HttpRequest::ProgressCallbackCR onProgress)
    {
    return std::make_shared<ResponseGuard>(tokenToWrap, onProgress);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ResponseGuard::IsCanceled()
    {
    return m_tokenEnabled && m_token && m_token->IsCanceled();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ResponseGuard::Register(std::weak_ptr<ICancellationListener> listener)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequest::ProgressCallback ResponseGuard::GetProgressCallback()
    {
    ResponseGuardPtr guard = shared_from_this();
    return [guard] (double bytesUploaded, double bytesToUpload)
        {
        if (bytesUploaded != 0 && bytesUploaded == bytesToUpload)
            {
            // Getting server result, wait for response no matter what
            guard->m_tokenEnabled = false;
            }
        if (guard->m_onProgress)
            {
            guard->m_onProgress(bytesUploaded, bytesToUpload);
            }
        };
    }
