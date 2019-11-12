/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/ObservableECDb.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ObservableECDb::_OnDbChangedByOtherConnection()
    {
    ECDb::_OnDbChangedByOtherConnection();
    NotifyOnSchemaChangedListeners();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ObservableECDb::RegisterSchemaChangeListener(IECDbSchemaChangeListener* listener)
    {
    m_cacheSchemaChangeListeners.insert(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ObservableECDb::UnRegisterSchemaChangeListener(IECDbSchemaChangeListener* listener)
    {
    m_cacheSchemaChangeListeners.erase(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ObservableECDb::NotifyOnSchemaChangedListeners()
    {
    auto listenersCopy = m_cacheSchemaChangeListeners; // Safe guard against added listeners
    for (IECDbSchemaChangeListener* listener : listenersCopy)
        {
        if (m_cacheSchemaChangeListeners.find(listener) != m_cacheSchemaChangeListeners.end()) // Safe guard agains removed listeners
            {
            listener->OnSchemaChanged();
            }
        }
    }
