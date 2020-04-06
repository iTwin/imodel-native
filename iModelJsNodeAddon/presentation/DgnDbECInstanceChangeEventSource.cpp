/*--------------------------------------------------------------------------------------+
|
|     $Source: Presentation/DgnDbECInstanceChangeEventSource.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformLib.h>
#include "DgnDbECInstanceChangeEventSource.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbECInstanceChangeEventSource::DgnDbECInstanceChangeEventSource()
    {
    T_HOST.GetTxnAdmin().AddTxnMonitor(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbECInstanceChangeEventSource::~DgnDbECInstanceChangeEventSource()
    {
    T_HOST.GetTxnAdmin().DropTxnMonitor(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static ChangeType GetChangeTypeFromTxnChangeType(TxnTable::ChangeType changeType)
    {
    switch (changeType)
        {
        case TxnTable::ChangeType::Insert: return ChangeType::Insert;
        case TxnTable::ChangeType::Update: return ChangeType::Update;
        case TxnTable::ChangeType::Delete: return ChangeType::Delete;
        }
    BeAssert(false);
    return ChangeType::Insert;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static ECInstanceChangeEventSource::ChangedECInstance GetChangedECInstance(ECClassCR ecClass, ECInstanceId id, TxnTable::ChangeType changeType)
    {
    return ECInstanceChangeEventSource::ChangedECInstance(ecClass, id, GetChangeTypeFromTxnChangeType(changeType));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bset<ECInstanceChangeEventSource::ChangedECInstance> DgnDbECInstanceChangeEventSource::FindChanges(bvector<ChangedECInstance> const& changes, bset<ECInstanceKey> const& keys)
    {
    bset<ChangedECInstance> filteredChanges;
    for (ChangedECInstance const& change : changes)
        {
        for (ECInstanceKeyCR key : keys)
            {
            if (change.GetClass()->GetId() == key.GetClassId() && change.GetInstanceId() == key.GetInstanceId())
                filteredChanges.insert(change);
            }
        }
    return filteredChanges;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbECInstanceChangeEventSource::FillWithRelationshipChanges(bvector<ChangedECInstance>& changes, TxnManager& txns) const
    {
    typedef Dgn::TxnRelationshipLinkTables RLT;
    static Utf8String sql;
    if (sql.empty())
        {
        sql.append("SELECT ");
        sql.append("src_el.ECClassId").append(",");
        sql.append(RLT::COLNAME_SourceECInstanceId).append(",");
        sql.append("dst_el.ECClassId").append(",");
        sql.append(RLT::COLNAME_TargetECInstanceId);
        sql.append(" FROM ").append(TEMP_TABLE(TXN_TABLE_RelationshipLinkTables)).append(" rlt");
        sql.append(" LEFT JOIN ").append(BIS_TABLE(BIS_CLASS_Element)).append(" src_el");
        sql.append(" ON src_el.Id = ").append(RLT::COLNAME_SourceECInstanceId);
        sql.append(" LEFT JOIN ").append(BIS_TABLE(BIS_CLASS_Element)).append(" dst_el");
        sql.append(" ON dst_el.Id = ").append(RLT::COLNAME_TargetECInstanceId);
        }
    CachedStatementPtr stmt = txns.GetDgnDb().GetCachedStatement(sql.c_str());

    while (BE_SQLITE_ROW == stmt->Step())
        {
        ECClassId sourceClassId = stmt->GetValueId<ECClassId>(0);
        ECInstanceId sourceElementId = stmt->GetValueId<ECInstanceId>(1);
        ECClassId targetClassId = stmt->GetValueId<ECClassId>(2);
        ECInstanceId targetElementId = stmt->GetValueId<ECInstanceId>(3);

        bset<ECInstanceKey> keys;
        keys.insert(ECInstanceKey(sourceClassId, sourceElementId));
        keys.insert(ECInstanceKey(targetClassId, targetElementId));

        bset<ChangedECInstance> foundChanges = FindChanges(changes, keys);
        BeAssert(foundChanges.size() <= 2);

        for (ECInstanceKeyCR key : keys)
            {
            bool hasChange = false;
            for (ChangedECInstance const& change : foundChanges)
                {
                if (change.GetClass()->GetId() == key.GetClassId() && change.GetInstanceId() == key.GetInstanceId())
                    {
                    hasChange = true;
                    break;
                    }
                }
            if (!hasChange)
                {
                ECClassCP changeECClass = txns.GetDgnDb().Schemas().GetClass(key.GetClassId());
                if (nullptr == changeECClass)
                    {
                    BeAssert(false);
                    continue;
                    }
                changes.push_back(ChangedECInstance(*changeECClass, key.GetInstanceId(), ChangeType::Update));
                }
            }
        }
    }

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                07/2016
//=======================================================================================
struct ChangedInstanceIds : bmap<ECInstanceId, TxnTable::ChangeType>, VirtualSet
{
    bool _IsInSet(int nVals, DbValue const* vals) const override
        {
        BeAssert(nVals == 1);
        return end() != find(ECInstanceId(vals[0].GetValueUInt64()));
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddElementChanges(bvector<ECInstanceChangeEventSource::ChangedECInstance>& changes, Dgn::TxnManager& txns)
    {
    dgn_TxnTable::Element::Iterator elementsIter = txns.Elements().MakeIterator();
    for (auto const& element : elementsIter)
        {
        ECInstanceId instanceId(element.GetElementId().GetValue());
        ECClassId classId(element.GetECClassId().GetValue());
        ECClassCP ecClass = txns.GetDgnDb().Schemas().GetClass(classId);
        if (nullptr == ecClass)
            {
            BeAssert(false);
            continue;
            }
        changes.push_back(GetChangedECInstance(*ecClass, instanceId, element.GetChangeType()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddModelChanges(bvector<ECInstanceChangeEventSource::ChangedECInstance>& changes, Dgn::TxnManager& txns)
    {
    dgn_TxnTable::Model::Iterator modelsIter = txns.Models().MakeIterator();
    for (auto const& model : modelsIter)
        {
        ECInstanceId instanceId(model.GetModelId().GetValue());
        ECClassId classId(model.GetECClassId().GetValue());
        ECClassCP ecClass = txns.GetDgnDb().Schemas().GetClass(classId);
        if (nullptr == ecClass)
            {
            BeAssert(false);
            continue;
            }
        changes.push_back(GetChangedECInstance(*ecClass, instanceId, model.GetChangeType()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbECInstanceChangeEventSource::_OnCommit(Dgn::TxnManager& txns)
    {
    m_changes.clear();
    AddElementChanges(m_changes, txns);
    AddModelChanges(m_changes, txns);
    FillWithRelationshipChanges(m_changes, txns);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbECInstanceChangeEventSource::_OnCommitted(Dgn::TxnManager& txns)
    {
    NotifyECInstancesChanged(txns.GetDgnDb(), m_changes);
    m_changes.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbECInstanceChangeEventSource::_OnClassUsed(ECDbCR db, ECN::ECClassCR ecClass, bool)
    {
    if (!ecClass.IsRelationshipClass())
        return;

    DgnDbCP dgndb = dynamic_cast<DgnDbCP>(&db);
    if (nullptr == dgndb)
        return;

    const_cast<DgnDbP>(dgndb)->Txns().BeginTrackingRelationship(ecClass);
    }
