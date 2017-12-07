/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ECInstanceChangeEvents.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/ECInstanceChangeEvents.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceChangeEventSource::NotifyECInstanceChanged(ECDbCR db, ChangedECInstance change) const
    {
    bvector<ChangedECInstance> changes;
    changes.push_back(change);
    NotifyECInstancesChanged(db, changes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceChangeEventSource::NotifyECInstancesChanged(ECDbCR db, bvector<ChangedECInstance> changes) const
    {
    for (IEventHandler* handler : m_eventHandlers)
        handler->_OnECInstancesChanged(db, changes);
    }
