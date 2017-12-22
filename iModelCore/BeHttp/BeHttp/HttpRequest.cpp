/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/HttpRequest.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeHttp/HttpRequest.h>
#include <BeHttp/DefaultHttpHandler.h>
#include <folly/BeFolly.h>

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Request::Request(Utf8String url, Utf8String method, IHttpHandlerPtr customHandler) :
    m_url(url), m_method(method), m_responseBody(HttpStringBody::Create()),
    m_httpHandler(customHandler == nullptr ? DefaultHttpHandler::GetInstance() : customHandler),
    m_compressionOptions()
    {
    BeUri::EscapeUnsafeCharactersInUrl(m_url);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  12/2016
//----------------------------------------------------------------------------------------
folly::Future<Response> Request::Perform()
    {
    auto follyPromise = std::make_shared<folly::Promise<Response>>();

    PerformAsync()->Then([=] (Response& response)
        {
        follyPromise->setValue(response);
        });

    // We want all f.then(...) to execute on the CPU pool unless overridden with f.then(&otherExec, ...)
    return follyPromise->getFuture().via(&BeFolly::ThreadPool::GetCpuPool());
    }
