/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/DefaultHttpHandler.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BeHttp/DefaultHttpHandler.h>
#include <Bentley/Tasks/AsyncTasksManager.h>
#include "Curl/ThreadCurlHttpHandler.h"

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static IHttpHandlerPtr CreateDefaultHandler()
    {
    return std::make_shared<ThreadCurlHttpHandler> ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultHttpHandlerPtr DefaultHttpHandler::GetInstance()
    {
    static DefaultHttpHandlerPtr instance = std::shared_ptr<DefaultHttpHandler>(new DefaultHttpHandler());
    return instance;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultHttpHandler::DefaultHttpHandler()
    {
    CurlHttpHandler::ProcessInitialize();
    SetEnabled(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultHttpHandler::~DefaultHttpHandler()
    {
    BeAssert(m_handler == nullptr && "Call HttpClient::Uninitialize() before shutting down application");
    m_handler = nullptr;
    CurlHttpHandler::ProcessUninitialize();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultHttpHandler::SetEnabled(bool enabled)
    {
    BeMutexHolder lock(m_mutex);

    if (enabled && m_handler == nullptr)
        m_handler = CreateDefaultHandler();
    else if (!enabled)
        m_handler.reset();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IHttpHandlerPtr DefaultHttpHandler::GetInternalHandler() const
    {
    return m_handler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<Response> DefaultHttpHandler::_PerformRequest(const Request& request)
    {
    BeMutexHolder lock(m_mutex);

    if (!m_handler)
        return CreateCompletedAsyncTask(Response(ConnectionStatus::Canceled));

    return m_handler->_PerformRequest(request);
    }
