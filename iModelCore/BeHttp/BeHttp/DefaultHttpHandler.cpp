/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <BeHttp/DefaultHttpHandler.h>
#include <Bentley/Tasks/AsyncTasksManager.h>
#include "Curl/ThreadCurlHttpHandler.h"
#include "WebLogging.h"

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
    if (m_handler != nullptr)
        {
        //LOG.fatal("Call HttpClient::Uninitialize() before shutting down application.");
        BeAssert(false && "Call HttpClient::Uninitialize() before shutting down application.");

        // Windows process is now running one thread. Clean shut down impossible at this point.
        // Destructor is called from static singleton variable - process shutdown.
        // All threads are killed off - web threads are destroyed and stuck in "running" state.
        
        // Disable waiting to avoid hanging process in destructor.
        // Crash could still occur as CURL is unitialized too late in process lifetime.
        CurlHttpHandler::SetWaitOnDestroy(false);
        }
    
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
