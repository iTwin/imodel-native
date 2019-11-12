/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BeHttp/HttpRequest.h>
#include <BeHttp/DefaultHttpHandler.h>
#include <Bentley/Tasks/ThreadlessTaskScheduler.h>
#include <Bentley/Tasks/AsyncTaskFollyAdapter.h>

#include "WebLogging.h"

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
    if (m_url.empty())
        {
        LOG.errorv("Request object received an invalid URI: \"%s\"", url.c_str());
        BeAssert(false);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  12/2016
//----------------------------------------------------------------------------------------
folly::Future<Response> Request::Perform()
    {
    return AsyncTaskFollyAdapter::ToFolly(PerformAsync());
    }
