/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ApplicationEvents.h"

USING_NAMESPACE_BENTLEY_HTTP

#if !defined (BENTLEYCONFIG_OS_APPLE_IOS)
void ApplicationEventsManager::InitializeApplicationEventsListening() {}
void ApplicationEventsManager::StartBackgroundTask(Utf8CP name, std::function<void()> task, std::function<void()> onExpired) {}
void ApplicationEventsManager::SetNetworkActivityIndicatorVisible(bool visible) { /* Do nothing on non-iOS platforms. */ }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Keith.Bentley                   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
ApplicationEventsManager& ApplicationEventsManager::GetInstance()
    {
    // NOTE: This is not thread safe!
    static ApplicationEventsManager* s_instance;

    if (s_instance == nullptr)
        s_instance = new ApplicationEventsManager();

    return *s_instance;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplicationEventsManager::AddApplicationEventsListener(IApplicationEventsListener& listener)
    {
    m_applicationEventsListeners.insert(&listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplicationEventsManager::RemoveApplicationEventsListener(IApplicationEventsListener& listener)
    {
    m_applicationEventsListeners.erase(&listener);
    }
