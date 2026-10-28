/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// The per-class information of a BulkElementAspectWriter.
//=======================================================================================
struct BulkElementAspectWriterClassInfo
    {
    bool m_isMulti = false;
    bool m_isGeneric = false;
    bool m_isGenericComputed = false;
    Utf8String m_insertECSql;   // INSERT INTO [instance class] (Element.Id,Element.RelECClassId) VALUES (?,?)
    Utf8String m_deleteECSql;   // DELETE FROM [key class] WHERE Element.Id=? (unique aspects)
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct BulkElementAspectWriter::Impl
    {
    DgnDbR m_dgndb;
    ECClassId m_multiRelClassId;
    ECClassId m_uniqueRelClassId;
    std::map<ECClassId, BulkElementAspectWriterClassInfo> m_classes;

    explicit Impl(DgnDbR db) : m_dgndb(db)
        {
        m_multiRelClassId = ECClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsMultiAspects).GetValue());
        m_uniqueRelClassId = ECClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsUniqueAspect).GetValue());
        }

    BulkElementAspectWriterClassInfo& GetClassInfo(ECClassCR cls)
        {
        auto found = m_classes.find(cls.GetId());
        if (m_classes.end() != found)
            return found->second;

        auto& info = m_classes[cls.GetId()];
        info.m_isMulti = cls.Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect);
        info.m_insertECSql.Sprintf("INSERT INTO %s (Element.Id,Element.RelECClassId) VALUES (?,?)", cls.GetECSqlName().c_str());
        info.m_deleteECSql.Sprintf("DELETE FROM %s WHERE Element.Id=?", cls.GetECSqlName().c_str());
        return info;
        }

    // The same as the fix-up done by GenericUniqueAspect::_UpdateProperties and GenericMultiAspect::_UpdateProperties.
    void SetElement(IECInstanceR instance, DgnElementId ownerId, bool isMulti)
        {
        ECValue v;
        instance.GetValue(v, "Element");
        if (v.IsNull() || !v.IsNavigation() || !v.GetNavigationInfo().GetRelationshipClassId().IsValid())
            instance.SetValue("Element", ECValue(ownerId, isMulti ? m_multiRelClassId : m_uniqueRelClassId));
        }

    static void SetInstanceId(IECInstanceR instance, ECInstanceId id)
        {
        Utf8Char idStr[32];
        BeStringUtilities::FormatUInt64(idStr, id.GetValue());
        instance.SetInstanceId(idStr);
        }

    // The same as GenericUniqueAspect::_UpdateProperties and GenericMultiAspect::_UpdateProperties.
    DgnDbStatus UpdateProperties(IECInstanceR instance, ECInstanceId id, DgnElementId ownerId, bool isMulti)
        {
        auto updater = BulkElementAspectWriter::GetUpdater(m_dgndb, instance.GetClass());
        if (nullptr == updater)
            return DgnDbStatus::BadSchema;

        SetInstanceId(instance, id);
        SetElement(instance, ownerId, isMulti);
        return (BE_SQLITE_OK == updater->Update(instance)) ? DgnDbStatus::Success : DgnDbStatus::WriteError;
        }

    DgnDbStatus Insert(IECInstanceR instance, ECInstanceId& id, DgnElementId ownerId, bool isMulti);
    ECInstanceKey QueryExistingUniqueAspect(ECClassCR keyClass, DgnElementId ownerId);
    void Write(BulkElementAspectWrite& write, DgnElementId ownerId);
    };

/*---------------------------------------------------------------------------------**//**
* The same as DgnElement::Aspect::InsertThis for a generic aspect: the INSERT of the owner (see MultiAspect::_InsertInstance and
* UniqueAspect::_InsertInstance) followed by the UPDATE of the properties.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus BulkElementAspectWriter::Impl::Insert(IECInstanceR instance, ECInstanceId& id, DgnElementId ownerId, bool isMulti)
    {
    ECClassCR cls = instance.GetClass();
    auto& info = GetClassInfo(cls);

    // The INSERT uses the relationship class of the Element property, so setting it first does not change what is written.
    SetElement(instance, ownerId, isMulti);

    CachedECSqlStatementPtr stmt = m_dgndb.GetNonSelectPreparedECSqlStatement(info.m_insertECSql.c_str(), m_dgndb.GetECCrudWriteToken());
    if (!stmt.IsValid())
        return DgnDbStatus::WriteError;

    ECValue v;
    instance.GetValue(v, "Element");
    ECClassId relClassId = v.IsNull() ? ECClassId() : v.GetNavigationInfo().GetRelationshipClassId();
    if (!relClassId.IsValid())
        relClassId = isMulti ? m_multiRelClassId : m_uniqueRelClassId;

    if ((ECSqlStatus::Success != stmt->BindId(1, ownerId)) || (ECSqlStatus::Success != stmt->BindId(2, relClassId)))
        return DgnDbStatus::WriteError;

    ECInstanceKey key;
    if (BE_SQLITE_DONE != stmt->Step(key))
        return DgnDbStatus::WriteError;

    id = key.GetInstanceId();
    return UpdateProperties(instance, id, ownerId, isMulti);
    }

/*---------------------------------------------------------------------------------**//**
* The same as UniqueAspect::_QueryExistingInstanceKey.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey BulkElementAspectWriter::Impl::QueryExistingUniqueAspect(ECClassCR keyClass, DgnElementId ownerId)
    {
    CachedStatementPtr stmt = m_dgndb.GetCachedStatement("SELECT Id, ECClassId from bis_ElementUniqueAspect WHERE ECClassId in ( SELECT ClassId from ec_cache_ClassHierarchy WHERE BaseClassId = ?) AND ElementId =? ");
    if (!stmt.IsValid())
        return ECInstanceKey();

    if ((BE_SQLITE_OK != stmt->BindId(1, keyClass.GetId())) || (BE_SQLITE_OK != stmt->BindId(2, ownerId)))
        return ECInstanceKey();

    if (BE_SQLITE_ROW != stmt->Step())
        return ECInstanceKey();

    return ECInstanceKey(stmt->GetValueId<ECClassId>(1), stmt->GetValueId<ECInstanceId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* The same as DgnElement::Aspect::_OnUpdated for a changed generic aspect.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BulkElementAspectWriter::Impl::Write(BulkElementAspectWrite& write, DgnElementId ownerId)
    {
    if (!write.instance.IsValid())
        {
        write.status = DgnDbStatus::BadArg;
        return;
        }

    IECInstanceR instance = *write.instance;
    ECClassCR cls = instance.GetClass();
    bool isMulti = GetClassInfo(cls).m_isMulti;

    if (isMulti)
        {
        // A multi aspect exists if and only if its id is known (see MultiAspect::_QueryExistingInstanceKey)
        if (!write.aspectId.IsValid())
            write.status = Insert(instance, write.aspectId, ownerId, true);
        else
            write.status = UpdateProperties(instance, write.aspectId, ownerId, true);
        return;
        }

    ECClassCR keyClass = (nullptr != write.keyClass) ? *write.keyClass : cls;
    ECInstanceKey existing = QueryExistingUniqueAspect(keyClass, ownerId);
    write.status = DgnDbStatus::Success;
    if (existing.IsValid() && (existing.GetClassId() != cls.GetId()))
        {
        // Replace the existing aspect, which is of another class (see UniqueAspect::_DeleteInstance)
        write.status = DgnDbStatus::WriteError;
        CachedECSqlStatementPtr stmt = m_dgndb.GetNonSelectPreparedECSqlStatement(GetClassInfo(keyClass).m_deleteECSql.c_str(), m_dgndb.GetECCrudWriteToken());
        if (stmt.IsValid() && (ECSqlStatus::Success == stmt->BindId(1, ownerId)) && (BE_SQLITE_DONE == stmt->Step()))
            write.status = DgnDbStatus::Success;
        existing = ECInstanceKey();
        }

    if (!existing.IsValid())
        {
        ECInstanceId id;
        auto status = Insert(instance, id, ownerId, false);
        if (id.IsValid())
            write.aspectId = id;
        if (DgnDbStatus::Success == write.status)
            write.status = status;
        return;
        }

    if (!write.aspectId.IsValid())
        write.aspectId = existing.GetInstanceId();
    write.status = UpdateProperties(instance, write.aspectId, ownerId, false);
    }

//=======================================================================================
// Writes the aspects when the element is updated, at the point where DgnElement::Aspect objects are written.
//=======================================================================================
struct BulkElementAspectWriter::WriterAppData : DgnElement::AppData
    {
    static Key const& GetKey() { static Key s_key; return s_key; }

    BulkElementAspectWriter::Impl& m_impl;
    bvector<BulkElementAspectWrite>& m_writes;

    WriterAppData(BulkElementAspectWriter::Impl& impl, bvector<BulkElementAspectWrite>& writes) : m_impl(impl), m_writes(writes) {}

    DropMe _OnInserted(DgnElementCR) override { return DropMe::Yes; }
    DropMe _OnDeleted(DgnElementCR) override { return DropMe::Yes; }
    DropMe _OnUpdated(DgnElementCR modified, DgnElementCR original, bool isOriginal) override
        {
        if (isOriginal)
            return DropMe::Yes;

        for (auto& write : m_writes)
            m_impl.Write(write, modified.GetElementId());
        return DropMe::Yes;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BulkElementAspectWriter::BulkElementAspectWriter(DgnDbR db) : m_impl(new Impl(db)) {}
BulkElementAspectWriter::~BulkElementAspectWriter() { delete m_impl; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceUpdater* BulkElementAspectWriter::GetUpdater(DgnDbR db, ECClassCR aspectClass)
    {
    return db.Elements().m_updaterCache.GetUpdater(db, aspectClass);
    }

/*---------------------------------------------------------------------------------**//**
* The same tests as GenericUniqueAspect::GetAspectP and GenericMultiAspect::GetAspectP.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BulkElementAspectWriter::IsGenericAspectClass(ECClassCR aspectClass)
    {
    auto& info = m_impl->GetClassInfo(aspectClass);
    if (!info.m_isGenericComputed)
        {
        info.m_isGenericComputed = true;
        dgn_AspectHandler::Aspect* handler = dgn_AspectHandler::Aspect::FindHandler(m_impl->m_dgndb, DgnClassId(aspectClass.GetId()));
        info.m_isGeneric = ((nullptr == handler) || (handler == &dgn_AspectHandler::Aspect::GetHandler())) && aspectClass.GetCustomAttributeLocal("ClassHasHandler").IsNull();
        }
    return info.m_isGeneric;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus BulkElementAspectWriter::UpdateElement(DgnElementR elementEdit, bvector<BulkElementAspectWrite>& writes)
    {
    if (!writes.empty())
        elementEdit.AddAppData(WriterAppData::GetKey(), new WriterAppData(*m_impl, writes));

    DgnDbStatus status = elementEdit.Update();
    if (DgnDbStatus::Success != status)
        {
        elementEdit.DropAppData(WriterAppData::GetKey());
        for (auto& write : writes)
            write.status = status;
        }
    return status;
    }

END_BENTLEY_DGN_NAMESPACE
