/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Curl/ThreadCurlHttpHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Bentley/Tasks/LimitingTaskQueue.h>
#include <Bentley/Tasks/WorkerThreadPool.h>
#include "CurlHttpHandler.h"

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* This code workarounds issue described in http://curl.haxx.se/mail/lib-2015-05/0063.html
* Reusing same CULRM handle in CurlTaskRunner causes TCP layer to misbehave on uploads
* This problem only occurs when using Windows Server 2008 (IIS 7.5) and HTTPS. 
* Using non secure HTTP or Windows Server 2012 (IIS 8.0 both HTTP and HTTPS) works fine. 
* Curl version does not seem to affect anything. Downloads and small uploads work fine. 
* @bsiclass                                                     Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct ThreadCurlHttpHandler : public CurlHttpHandler
{
protected:
    std::shared_ptr<Tasks::WorkerThreadPool> m_threadPool;
    Tasks::LimitingTaskQueue<Response>   m_threadQueue;

public:
    ThreadCurlHttpHandler();
    virtual ~ThreadCurlHttpHandler();
    virtual Tasks::AsyncTaskPtr<Response> _PerformRequest(RequestCR request) override;
};

END_BENTLEY_HTTP_NAMESPACE
