/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECInstanceChangeEvents.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceChangeEventSource::NotifyECInstanceChanged(ECDbCR db, ChangedECInstance change) const
    {
    bvector<ChangedECInstance> changes;
    changes.push_back(change);
    NotifyECInstancesChanged(db, changes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceChangeEventSource::NotifyECInstancesChanged(ECDbCR db, bvector<ChangedECInstance> changes) const
    {
    BeMutexHolder lock(m_mutex);
    for (IEventHandler* handler : m_eventHandlers)
        handler->_OnECInstancesChanged(db, changes);
    }
