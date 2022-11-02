/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatform/PlatformLib.h>
#include "DgnDbECInstanceChangeEventSource.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbECInstanceChangeEventSource::DgnDbECInstanceChangeEventSource()
    {
    TxnManager::AddTxnMonitor(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbECInstanceChangeEventSource::~DgnDbECInstanceChangeEventSource()
    {
    TxnManager::DropTxnMonitor(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ChangeType GetChangeTypeFromTxnChangeType(TxnTable::ChangeType changeType)
    {
    switch (changeType)
        {
        case TxnTable::ChangeType::Insert: return ChangeType::Insert;
        case TxnTable::ChangeType::Update: return ChangeType::Update;
        case TxnTable::ChangeType::Delete: return ChangeType::Delete;
        }
    DIAGNOSTICS_DEV_LOG(ECPresentation::DiagnosticsCategory::Default, NativeLogging::LOG_ERROR, Utf8PrintfString("Unhandled TxnTable::ChangeType: %d", (int)changeType));
    return ChangeType::Insert;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECInstanceChangeEventSource::ChangedECInstance GetChangedECInstance(ECClassCR ecClass, ECInstanceId id, TxnTable::ChangeType changeType)
    {
    return ECInstanceChangeEventSource::ChangedECInstance(ecClass, id, GetChangeTypeFromTxnChangeType(changeType));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ECInstanceChangeEventSource::ChangedECInstance> FindChangesForInstanceKeys(bvector<ECInstanceChangeEventSource::ChangedECInstance> const& changes, bvector<ECInstanceKey> const& keys)
    {
    bvector<ECInstanceChangeEventSource::ChangedECInstance> filteredChanges;
    for (auto const& change : changes)
        {
        for (ECInstanceKeyCR key : keys)
            {
            if (change.GetClass()->GetId() == key.GetClassId() && change.GetInstanceId() == key.GetInstanceId())
                filteredChanges.push_back(change);
            }
        if (filteredChanges.size() == keys.size())
            break;
        }
    return filteredChanges;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddRelationshipChanges(bvector<ECInstanceChangeEventSource::ChangedECInstance>& changes, TxnManager& txns)
    {
    typedef Dgn::TxnRelationshipLinkTables RLT;
    Utf8String sql;
    sql.append("SELECT ")
        .append("src_el.ECClassId").append(",").append(RLT::COLNAME_SourceECInstanceId).append(",")
        .append("dst_el.ECClassId").append(",").append(RLT::COLNAME_TargetECInstanceId).append(" ")
        .append("FROM ").append(TEMP_TABLE(TXN_TABLE_RelationshipLinkTables)).append(" rlt")
        .append(" LEFT JOIN ").append(BIS_TABLE(BIS_CLASS_Element)).append(" src_el").append(" ON src_el.Id = ").append(RLT::COLNAME_SourceECInstanceId)
        .append(" LEFT JOIN ").append(BIS_TABLE(BIS_CLASS_Element)).append(" dst_el").append(" ON dst_el.Id = ").append(RLT::COLNAME_TargetECInstanceId);

    CachedStatementPtr stmt = txns.GetDgnDb().GetCachedStatement(sql.c_str());

    while (BE_SQLITE_ROW == stmt->Step())
        {
        ECClassId sourceClassId = stmt->GetValueId<ECClassId>(0);
        ECInstanceId sourceElementId = stmt->GetValueId<ECInstanceId>(1);
        ECClassId targetClassId = stmt->GetValueId<ECClassId>(2);
        ECInstanceId targetElementId = stmt->GetValueId<ECInstanceId>(3);

        bvector<ECInstanceKey> keys;
        keys.push_back(ECInstanceKey(sourceClassId, sourceElementId));
        keys.push_back(ECInstanceKey(targetClassId, targetElementId));

        bvector<ECInstanceChangeEventSource::ChangedECInstance> foundChanges = FindChangesForInstanceKeys(changes, keys);
        for (ECInstanceKeyCR key : keys)
            {
            auto iter = std::find_if(foundChanges.begin(), foundChanges.end(), [&key](auto const& change){return change.GetClass()->GetId() == key.GetClassId() && change.GetInstanceId() == key.GetInstanceId();});
            if (iter == foundChanges.end())
                {
                ECClassCP changeECClass = txns.GetDgnDb().Schemas().GetClass(key.GetClassId());
                if (nullptr == changeECClass)
                    DIAGNOSTICS_HANDLE_FAILURE(ECPresentation::DiagnosticsCategory::Default, Utf8PrintfString("Detected changed ECInstance with invalid ECClassId: %" PRIu64, key.GetClassId().GetValue()));
                changes.push_back(ECInstanceChangeEventSource::ChangedECInstance(*changeECClass, key.GetInstanceId(), ChangeType::Update));
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
            DIAGNOSTICS_HANDLE_FAILURE(ECPresentation::DiagnosticsCategory::Default, Utf8PrintfString("Detected changed Element with invalid ECClassId: %" PRIu64, classId.GetValue()));
        changes.push_back(GetChangedECInstance(*ecClass, instanceId, element.GetChangeType()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
            DIAGNOSTICS_HANDLE_FAILURE(ECPresentation::DiagnosticsCategory::Default, Utf8PrintfString("Detected changed Model with invalid ECClassId: %" PRIu64, classId.GetValue()));
        changes.push_back(GetChangedECInstance(*ecClass, instanceId, model.GetChangeType()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddChanges(bvector<ECInstanceChangeEventSource::ChangedECInstance>& changes, Dgn::TxnManager& txns)
    {
    AddElementChanges(changes, txns);
    AddModelChanges(changes, txns);
    AddRelationshipChanges(changes, txns);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbECInstanceChangeEventSource::_OnCommit(Dgn::TxnManager& txns)
    {
    AddChanges(m_changes, txns);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbECInstanceChangeEventSource::_OnAppliedChanges(TxnManager& txns)
    {
    bvector<ECInstanceChangeEventSource::ChangedECInstance> changes;
    AddChanges(changes, txns);
    NotifyECInstancesChanged(txns.GetDgnDb(), changes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbECInstanceChangeEventSource::_OnCommitted(Dgn::TxnManager& txns)
    {
    NotifyECInstancesChanged(txns.GetDgnDb(), m_changes);
    m_changes.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
