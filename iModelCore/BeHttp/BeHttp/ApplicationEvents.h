/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/ApplicationEvents.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeHttp/Http.h>
#include <Bentley/bset.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                 Pranciskus.Ambrazas   03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct IApplicationEventsListener
    {
    virtual void _OnApplicationSentToBackground() = 0;
    virtual void _OnApplicationSentToForeground() = 0;
    virtual ~IApplicationEventsListener() {}
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                 Pranciskus.Ambrazas   03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct ApplicationEventsManager
    {
private:
    bset<IApplicationEventsListener*> m_applicationEventsListeners;

private:
    ApplicationEventsManager() { InitializeApplicationEventsListening(); }
    void InitializeApplicationEventsListening();

public:
    static ApplicationEventsManager& GetInstance();

    //! Registers the application events' listener.
    void AddApplicationEventsListener(IApplicationEventsListener& listener);

    //! Unregisters the application events' listener.
    void RemoveApplicationEventsListener(IApplicationEventsListener& listener);

    //! Start OS specific background task for extended execution
    static void StartBackgroundTask(Utf8CP name, std::function<void()> task, std::function<void()> onExpired);

    //! Indicate network activity in OS specific way
    static void SetNetworkActivityIndicatorVisible(bool visible);
    };

END_BENTLEY_HTTP_NAMESPACE
