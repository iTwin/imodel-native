/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECInstanceChangeEvents.h>
#include "ECPresentationTest.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestECInstanceChangeEventsSource : ECInstanceChangeEventSource
{
private:
    void NotifyECInstanceChanged(ECDbCR db, IECInstanceCR instance, ChangeType change) const
        {
        ECInstanceId id;
        ECInstanceId::FromString(id, instance.GetInstanceId().c_str());
        ECInstanceChangeEventSource::NotifyECInstanceChanged(db, ECInstanceChangeEventSource::ChangedECInstance(instance.GetClass(), id, change));
        }
public:
    void NotifyECInstancesChanged(ECDbCR db, bvector<ChangedECInstance> changes) const
        {
        ECInstanceChangeEventSource::NotifyECInstancesChanged(db, changes);
        }
    void NotifyECInstancesChanged(ECDbCR db, bvector<IECInstanceCP> instances, ChangeType change) const
        {
        bvector<ECInstanceChangeEventSource::ChangedECInstance> changes;
        for (IECInstanceCP instance : instances)
            {
            ECInstanceId id;
            ECInstanceId::FromString(id, instance->GetInstanceId().c_str());
            changes.push_back(ECInstanceChangeEventSource::ChangedECInstance(instance->GetClass(), id, change));
            }
        NotifyECInstancesChanged(db, changes);
        }
    void NotifyECInstancesChanged(ECDbCR db, bvector<ECClassInstanceKey> const& keys, ChangeType change) const
        {
        bvector<ECInstanceChangeEventSource::ChangedECInstance> changes;
        for (auto const& key : keys)
            changes.push_back(ECInstanceChangeEventSource::ChangedECInstance(*key.GetClass(), key.GetId(), change));
        NotifyECInstancesChanged(db, changes);
        }

    void NotifyECInstanceInserted(ECDbCR db, IECInstanceCR instance) const { NotifyECInstanceChanged(db, instance, ChangeType::Insert); }
    void NotifyECInstanceDeleted(ECDbCR db, IECInstanceCR instance) const { NotifyECInstanceChanged(db, instance, ChangeType::Delete); }
    void NotifyECInstanceUpdated(ECDbCR db, IECInstanceCR instance) const { NotifyECInstanceChanged(db, instance, ChangeType::Update); }
    void NotifyECInstanceUpdated(ECDbCR db, ECClassInstanceKeyCR key) const { NotifyECInstancesChanged(db, bvector<ECClassInstanceKey>{ key }, ChangeType::Update); }

    void NotifyECInstancesInserted(ECDbCR db, bvector<IECInstanceCP> instances) const { NotifyECInstancesChanged(db, instances, ChangeType::Insert); }
    void NotifyECInstancesDeleted(ECDbCR db, bvector<IECInstanceCP> instances) const { NotifyECInstancesChanged(db, instances, ChangeType::Delete); }
    void NotifyECInstancesUpdated(ECDbCR db, bvector<IECInstanceCP> instances) const { NotifyECInstancesChanged(db, instances, ChangeType::Update); }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestECInstanceChangeEventsHandler : ECInstanceChangeEventSource::IEventHandler
{
private:
    std::function<void(ECDbCR, bvector<ECInstanceChangeEventSource::ChangedECInstance>)> m_callback;
protected:
    void _OnECInstancesChanged(ECDbCR db, bvector<ECInstanceChangeEventSource::ChangedECInstance> changes) override
        {
        if (m_callback)
            m_callback(db, changes);
        }
public:
    void SetCallback(std::function<void(ECDbCR, bvector<ECInstanceChangeEventSource::ChangedECInstance>)> callback) { m_callback = callback; }
};

END_ECPRESENTATIONTESTS_NAMESPACE
