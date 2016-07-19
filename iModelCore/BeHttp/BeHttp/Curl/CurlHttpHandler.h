/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Curl/CurlHttpHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeHttp/HttpRequest.h>
#include <BeHttp/IHttpHandler.h>
#include <Bentley/Tasks/WorkerThreadPool.h>
#include "CurlPool.h"
#include "NotificationPipe.h"

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                 Pranciskus.Ambrazas   03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct IApplicationEventsListener
    {
    virtual void _OnApplicationResume() = 0;
    virtual ~IApplicationEventsListener() {}
    };
 
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                 Pranciskus.Ambrazas   03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct ApplicationEventsManager
    {
    private:
        BEHTTP_EXPORT static ApplicationEventsManager* s_instance;
        bset<IApplicationEventsListener*> m_applicationEventsListeners; 

    private:
        ApplicationEventsManager() { InitializeApplicationEventsListening(); }
        BEHTTP_EXPORT void InitializeApplicationEventsListening();

    public:
        static ApplicationEventsManager& GetInstance()
            {
            if (s_instance == nullptr)
                s_instance = new ApplicationEventsManager();
            return *s_instance;
            }
        //! Registers the application events' listener.
        void AddApplicationEventsListener(IApplicationEventsListener& listener);

        //! Unregisters the application events' listener.
        void RemoveApplicationEventsListener(IApplicationEventsListener& listener);
    };


/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CurlHttpHandler : public IHttpHandler, IApplicationEventsListener
    {
protected:
    // NotificationPipe for wake up from idle when request is added. Will not work for cancellation of existing requests.
    NotificationPipe                                        m_notifier;
    
    std::shared_ptr<Tasks::WorkerThreadPool>                       m_webThreadPool;

    CurlPool                                                m_curlPool;
    
private:
    virtual void _OnApplicationResume() override;

public:
    CurlHttpHandler();
    virtual ~CurlHttpHandler();

    virtual Tasks::AsyncTaskPtr<Response> _PerformRequest(RequestCR request) override;
    };


END_BENTLEY_HTTP_NAMESPACE
