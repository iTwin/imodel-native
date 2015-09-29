/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnElement.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/QvElemSet.h>
#include <DgnPlatform/DgnCore/DgnScript.h>

#define DGN_ELEMENT_PROPNAME_ECINSTANCEID "ECInstanceId"
#define DGN_ELEMENT_PROPNAME_MODELID "ModelId"
#define DGN_ELEMENT_PROPNAME_CODE "Code"
#define DGN_ELEMENT_PROPNAME_CODEAUTHORITYID "CodeAuthorityId"
#define DGN_ELEMENT_PROPNAME_CODENAMESPACE "CodeNameSpace"
#define DGN_ELEMENT_PROPNAME_PARENTID "ParentId"
#define DGN_ELEMENT_PROPNAME_LASTMOD "LastMod"
#define DGN_GEOMETRICELEMENT_PROPNAME_CATEGORYID "CategoryId"

DgnElement::Item::Key&  DgnElement::Item::GetKey() {static Key s_key; return s_key;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::AddRef() const
    {
    if (0 == m_refCount && IsPersistent())
        GetDgnDb().Elements().OnReclaimed(*this); // someone just requested this previously unreferenced element

    ++m_refCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::Release() const
    {
    if (1 < m_refCount--)
        return;

    BeAssert(m_refCount==0);

    if (IsPersistent())  // is this element in the pool?
        GetDgnDb().Elements().OnUnreferenced(*this); // yes, the last reference was just released, add to the unreferenced element count
    else
        delete this;  // no, just delete it
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnElement::GetModel() const
    {
    return m_dgndb.Models().GetModel(m_modelId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData* DgnElement::FindAppData(AppData::Key const& key) const
    {
    auto entry = m_appData.find(&key);
    return entry==m_appData.end() ? nullptr : entry->second.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::AddAppData(AppData::Key const& key, AppData* obj) const
    {
    auto entry = m_appData.Insert(&key, obj);
    if (entry.second)
        return;

    // we already had appdata for this key. Clean up old and save new.
    entry.first->second = obj;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnElement::DropAppData(AppData::Key const& key) const
    {
    return 0==m_appData.erase(&key) ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::ClearAllAppData()
    {
    m_appData.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    02/01
+---------------+---------------+---------------+---------------+---------------+------*/
HeapZone& DgnElement::GetHeapZone()  const {return GetDgnDb().Elements().GetHeapZone();}
QvCache*  GeometricElement::GetMyQvCache() const {return GetDgnDb().Models().GetQvCache();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/01
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::SaveGeomStream(GeomStreamCP stream)
    {
    m_geom = *stream;    // assign the new steam (overwrites or reallocates)
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_DeleteInDb() const
    {
    CachedStatementPtr stmt=GetDgnDb().Elements().GetStatement("DELETE FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?");
    stmt->BindId(1, m_elementId);

    switch (stmt->Step())
        {
        case BE_SQLITE_CONSTRAINT_FOREIGNKEY:
            return  DgnDbStatus::ForeignKeyConstraint;

        case BE_SQLITE_DONE:
            return DgnDbStatus::Success;
        }

    return DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometricElement::SetQvElem(QvElem* qvElem, uint32_t index)
    {
    GetQvElems(true)->Add(index, qvElem);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void QvKey32::DeleteQvElem(QvElem* qvElem)
    {
    if (qvElem && qvElem != INVALID_QvElem)
        T_HOST.GetGraphicsAdmin()._DeleteQvElem(qvElem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/06
+---------------+---------------+---------------+---------------+---------------+------*/
T_QvElemSet* GeometricElement::GetQvElems(bool createIfNotPresent) const
    {
    static AppData::Key s_qvElemsKey;
    T_QvElemSet* qvElems = (T_QvElemSet*) FindAppData(s_qvElemsKey);
    if (qvElems)
        return  qvElems;

    if (!createIfNotPresent)
        return  nullptr;

    HeapZone& zone = GetHeapZone();
    qvElems = new T_QvElemSet(zone);

    AddAppData(s_qvElemsKey, qvElems);
    return  qvElems;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::SetInSelectionSet(bool yesNo) const
    {
    m_flags.m_inSelectionSet = yesNo;
    SetHilited(yesNo ? DgnElement::Hilited::Normal : DgnElement::Hilited::None);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId DgnElement::QueryClassId(DgnDbR db)
    {
    return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Element));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP DgnElement::GetElementClass() const
    {
    return GetDgnDb().Schemas().GetECClass(GetElementClassId().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::Code DgnElement::_GenerateDefaultCode()
    {
    return DgnAuthority::GenerateDefaultCode(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime DgnElement::QueryTimeStamp() const
    {
    ECSqlStatement stmt;
    stmt.Prepare(GetDgnDb(), "SELECT " DGN_ELEMENT_PROPNAME_LASTMOD " FROM " DGN_SCHEMA(DGN_CLASSNAME_Element) " WHERE " DGN_ELEMENT_PROPNAME_ECINSTANCEID "=?");
    stmt.BindId(1, m_elementId);
    stmt.Step();
    return stmt.GetValueDateTime(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T> void DgnElement::CallAppData(T const& caller) const
    {
    for (auto entry=m_appData.begin(); entry!=m_appData.end(); )
        {
        if (DgnElement::AppData::DropMe::Yes == caller(*entry->second, *this))
            entry = m_appData.erase(entry);
        else
            ++entry;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnInsert()
    {
    if (!m_code.IsValid())
        m_code = _GenerateDefaultCode();

    for (auto entry=m_appData.begin(); entry!=m_appData.end(); ++entry)
        {
        DgnDbStatus stat = entry->second->_OnInsert(*this);
        if (DgnDbStatus::Success != stat)
            return stat;
        }

    return GetModel()->_OnInsertElement(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_OnInsert()
    {
    return !m_categoryId.IsValid() ? DgnDbStatus::InvalidCategory : T_Super::_OnInsert();
    }

struct OnInsertedCaller
    {
    DgnElementCR m_newEl;
    OnInsertedCaller(DgnElementCR newEl) : m_newEl(newEl){}
    DgnElement::AppData::DropMe operator()(DgnElement::AppData& app, DgnElementCR el) const {return app._OnInserted(m_newEl);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_OnInserted(DgnElementP copiedFrom) const
    {
    if (copiedFrom)
        copiedFrom->CallAppData(OnInsertedCaller(*this));

    GetModel()->_OnInsertedElement(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_OnReversedDelete() const
    {
    GetModel()->_OnReversedDeleteElement(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnUpdate(DgnElementCR original)
    {
    if (m_classId != original.m_classId)
        return DgnDbStatus::WrongClass;

    for (auto entry=m_appData.begin(); entry!=m_appData.end(); ++entry)
        {
        DgnDbStatus stat = entry->second->_OnUpdate(*this, original);
        if (DgnDbStatus::Success != stat)
            return stat;
        }

    return GetModel()->_OnUpdateElement(*this, original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_OnUpdate(DgnElementCR original)
    {
    return !m_categoryId.IsValid() ? DgnDbStatus::InvalidCategory : T_Super::_OnUpdate(original);
    }

struct OnUpdatedCaller
    {
    DgnElementCR m_updated, m_original;
    OnUpdatedCaller(DgnElementCR updated, DgnElementCR original) : m_updated(updated), m_original(original){}
    DgnElement::AppData::DropMe operator()(DgnElement::AppData& app, DgnElementCR el) const {return app._OnUpdated(m_updated, m_original);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_OnUpdated(DgnElementCR original) const
    {
    // We need to call the events on both sets of AppData. Start by calling the appdata on this (the replacement)
    // element. NOTE: This is where Aspects, etc. actually update the database.
    CallAppData(OnUpdatedCaller(*this, original));

    // All done. This gives appdata on the *original* element a notification that the update has happened
    original.CallAppData(OnUpdatedCaller(*this, original));

    // now tell the model that one of its elements has been changed.
    GetModel()->_OnUpdatedElement(*this, original);
    }

struct OnUpdateReversedCaller
    {
    DgnElementCR m_updated, m_original;
    OnUpdateReversedCaller(DgnElementCR updated, DgnElementCR original) : m_updated(updated), m_original(original){}
    DgnElement::AppData::DropMe operator()(DgnElement::AppData& app, DgnElementCR el) const {return app._OnReversedUpdate(m_updated, m_original);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_OnReversedUpdate(DgnElementCR original) const
    {
    // we need to call the events on BOTH sets of appdata
    original.CallAppData(OnUpdateReversedCaller(*this, original));
    CallAppData(OnUpdateReversedCaller(*this, original));

    GetModel()->_OnReversedUpdateElement(*this, original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnDelete() const
    {
    for (auto entry=m_appData.begin(); entry!=m_appData.end(); ++entry)
        {
        DgnDbStatus stat = entry->second->_OnDelete(*this);
        if (DgnDbStatus::Success != stat)
            return stat;
        }

    return GetModel()->_OnDeleteElement(*this);
    }

struct OnDeletedCaller  {DgnElement::AppData::DropMe operator()(DgnElement::AppData& app, DgnElementCR el) const {return app._OnDeleted(el);}};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_OnDeleted() const
    {
    CallAppData(OnDeletedCaller());
    GetDgnDb().Elements().DropFromPool(*this);
    DgnModelPtr model = GetModel();
    if (model.IsValid())
        model->_OnDeletedElement(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_OnReversedAdd() const
    {
    CallAppData(OnDeletedCaller());
    GetDgnDb().Elements().DropFromPool(*this);
    DgnModelPtr model = GetModel();
    if (model.IsValid())
        model->_OnReversedAddElement(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::Element::_GetClassParams(ECSqlClassParams& params)
    {
    params.Add(DGN_ELEMENT_PROPNAME_ECINSTANCEID, ECSqlClassParams::StatementType::Insert);
    params.Add(DGN_ELEMENT_PROPNAME_MODELID, ECSqlClassParams::StatementType::Insert);
    params.Add(DGN_ELEMENT_PROPNAME_CODE, ECSqlClassParams::StatementType::InsertUpdate);
    params.Add(DGN_ELEMENT_PROPNAME_CODEAUTHORITYID, ECSqlClassParams::StatementType::InsertUpdate);
    params.Add(DGN_ELEMENT_PROPNAME_CODENAMESPACE, ECSqlClassParams::StatementType::InsertUpdate);
    params.Add(DGN_ELEMENT_PROPNAME_PARENTID, ECSqlClassParams::StatementType::InsertUpdate);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus DgnElement::BindParams(ECSqlStatement& statement, bool isForUpdate)
    {
    BeAssert(m_code.IsValid());

    if ((ECSqlStatus::Success != statement.BindText(statement.GetParameterIndex(DGN_ELEMENT_PROPNAME_CODE), m_code.GetValue().c_str(), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != statement.BindId(statement.GetParameterIndex(DGN_ELEMENT_PROPNAME_CODEAUTHORITYID), m_code.GetAuthority())) ||
        (ECSqlStatus::Success != statement.BindText(statement.GetParameterIndex(DGN_ELEMENT_PROPNAME_CODENAMESPACE), m_code.GetNameSpace().c_str(), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != statement.BindId(statement.GetParameterIndex(DGN_ELEMENT_PROPNAME_PARENTID), m_parentId)))
        {
        return DgnDbStatus::BadArg;
        }

    if (!isForUpdate)
        {
        if ((ECSqlStatus::Success != statement.BindId(statement.GetParameterIndex(DGN_ELEMENT_PROPNAME_ECINSTANCEID), m_elementId)) ||
            (ECSqlStatus::Success != statement.BindId(statement.GetParameterIndex(DGN_ELEMENT_PROPNAME_MODELID), m_modelId)))
            return DgnDbStatus::BadArg;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::Physical::_GetClassParams(ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(DGN_GEOMETRICELEMENT_PROPNAME_CATEGORYID, ECSqlClassParams::StatementType::InsertUpdate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::Drawing::_GetClassParams(ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(DGN_GEOMETRICELEMENT_PROPNAME_CATEGORYID, ECSqlClassParams::StatementType::InsertUpdate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_BindInsertParams(ECSqlStatement& statement)
    {
    return BindParams(statement, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_BindInsertParams(ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindInsertParams(stmt);
    if (DgnDbStatus::Success == status && ECSqlStatus::Success != stmt.BindId(stmt.GetParameterIndex(DGN_GEOMETRICELEMENT_PROPNAME_CATEGORYID), m_categoryId))
        status = DgnDbStatus::BadArg;

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus DgnElement::_InsertInDb()
    {
    CachedECSqlStatementPtr statement = GetDgnDb().Elements().GetPreparedInsertStatement(*this);
    if (statement.IsNull())
        return DgnDbStatus::WriteError;

    auto status = _BindInsertParams(*statement);
    if (DgnDbStatus::Success != status || BE_SQLITE_DONE == statement->Step())
        return status;

    // SQLite doesn't tell us which constraint failed - check if it's the Code.
    auto existingElemWithCode = GetDgnDb().Elements().QueryElementIdByCode(m_code);
    return existingElemWithCode.IsValid() ? DgnDbStatus::DuplicateCode : DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_BindUpdateParams(ECSqlStatement& statement)
    {
    return BindParams(statement, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_BindUpdateParams(ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindUpdateParams(stmt);
    if (DgnDbStatus::Success == status && ECSqlStatus::Success != stmt.BindId(stmt.GetParameterIndex(DGN_GEOMETRICELEMENT_PROPNAME_CATEGORYID), m_categoryId))
        status = DgnDbStatus::BadArg;

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus DgnElement::_UpdateInDb()
    {
    CachedECSqlStatementPtr stmt = GetDgnDb().Elements().GetPreparedUpdateStatement(*this);
    if (stmt.IsNull())
        return DgnDbStatus::WriteError;

    DgnDbStatus status = _BindUpdateParams(*stmt);
    if (DgnDbStatus::Success == status && BE_SQLITE_DONE != stmt->Step())
        status = DgnDbStatus::WriteError;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_LoadFromDb()
    {
    DgnElements::ElementSelectStatement select = GetDgnDb().Elements().GetPreparedSelectStatement(*this);
    if (select.m_statement.IsNull())
        return DgnDbStatus::Success;
    else if (BE_SQLITE_ROW != select.m_statement->Step())
        return DgnDbStatus::ReadError;
    else
        return _ExtractSelectParams(*select.m_statement, select.m_params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet DgnElement::QueryChildren() const
    {
    CachedStatementPtr stmt=GetDgnDb().Elements().GetStatement("SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE ParentId=?");
    stmt->BindId(1, GetElementId());

    DgnElementIdSet elementIdSet;
    while (BE_SQLITE_ROW == stmt->Step())
        elementIdSet.insert(stmt->GetValueId<DgnElementId>(0));

    return elementIdSet;
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   11/10
//=======================================================================================
struct GeomBlobHeader
{
    enum {Signature = 0x0600,}; // DgnDb06

    uint32_t m_signature;    // write this so we can detect errors on read
    uint32_t m_size;
    GeomBlobHeader(GeomStream const& geom) {m_signature = Signature; m_size=geom.GetSize();}
    GeomBlobHeader(SnappyReader& in) {uint32_t actuallyRead; in._Read((Byte*) this, sizeof(*this), actuallyRead);}
};

static Utf8CP GEOM_Column = "Geom";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_LoadFromDb()
    {
    DgnDbStatus stat = T_Super::_LoadFromDb();
    if (DgnDbStatus::Success != stat)
        return stat;

    return m_geom.ReadGeomStream(GetDgnDb(), DGN_TABLE(DGN_CLASSNAME_ElementGeom), GEOM_Column, m_elementId.GetValue());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus GeometricElement::_InsertInDb()
    {
    DgnDbStatus stat;

    if (DgnDbStatus::Success != (stat = T_Super::_InsertInDb()))
        return stat;

    DgnDbR dgnDb = GetDgnDb();
    CachedStatementPtr stmt=dgnDb.Elements().GetStatement("INSERT INTO " DGN_TABLE(DGN_CLASSNAME_ElementGeom) "(Geom,Placement,ElementId) VALUES(?,?,?)");
    stmt->BindId(3, m_elementId);

    stat = _BindPlacement(*stmt);
    if (DgnDbStatus::NoGeometry == stat)
        return DgnDbStatus::Success;

    if (DgnDbStatus::Success == stat)
        stat = WriteGeomStream(*stmt, dgnDb);

    if (DgnDbStatus::Success != stat)
        return stat;

    // Insert element uses geom part relationships for geom parts referenced in geom stream...
    IdSet<DgnGeomPartId> parts;
    ElementGeomIO::Collection(m_geom.GetData(), m_geom.GetSize()).GetGeomPartIds(parts, dgnDb);
    for (DgnGeomPartId partId : parts)
        {
        if (BentleyStatus::SUCCESS != dgnDb.GeomParts().InsertElementGeomUsesParts(m_elementId, partId))
            stat = DgnDbStatus::WriteError;
        }

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_UpdateInDb()
    {
    DgnDbStatus stat = T_Super::_UpdateInDb();
    if (DgnDbStatus::Success != stat)
        return stat;

    DgnDbR dgnDb = GetDgnDb();
    CachedStatementPtr stmt=dgnDb.Elements().GetStatement("INSERT OR REPLACE INTO " DGN_TABLE(DGN_CLASSNAME_ElementGeom) " (Geom,Placement,ElementId) VALUES(?,?,?)");
    stmt->BindId(3, m_elementId);

    stat = _BindPlacement(*stmt);
    if (DgnDbStatus::NoGeometry == stat)
        return DgnDbStatus::Success;

    if (DgnDbStatus::Success == stat)
        stat = WriteGeomStream(*stmt, dgnDb);

    if (DgnDbStatus::Success != stat)
        return stat;

    // Update element uses geom part relationships for geom parts referenced in geom stream...
    stmt = dgnDb.Elements().GetStatement("SELECT GeomPartId FROM " DGN_TABLE(DGN_RELNAME_ElementGeomUsesParts) " WHERE ElementId=?");
    stmt->BindId(1, m_elementId);

    IdSet<DgnGeomPartId> partsOld;
    while (BE_SQLITE_ROW == stmt->Step())
        partsOld.insert(stmt->GetValueId<DgnGeomPartId>(0));

    IdSet<DgnGeomPartId> partsNew;
    ElementGeomIO::Collection(m_geom.GetData(), m_geom.GetSize()).GetGeomPartIds(partsNew, dgnDb);

    if (partsOld.empty() && partsNew.empty())
        return stat;

    bset<DgnGeomPartId> partsToRemove;
    std::set_difference(partsOld.begin(), partsOld.end(), partsNew.begin(), partsNew.end(), std::inserter(partsToRemove, partsToRemove.end()));

    if (!partsToRemove.empty())
        {
        stmt = dgnDb.Elements().GetStatement("DELETE FROM " DGN_TABLE(DGN_RELNAME_ElementGeomUsesParts) " WHERE ElementId=? AND GeomPartId=?");
        stmt->BindId(1, m_elementId);

        for (DgnGeomPartId partId : partsToRemove)
            {
            stmt->BindId(2, partId);
            if (BE_SQLITE_DONE != stmt->Step())
                stat = DgnDbStatus::BadRequest;
            }
        }

    bset<DgnGeomPartId> partsToAdd;
    std::set_difference(partsNew.begin(), partsNew.end(), partsOld.begin(), partsOld.end(), std::inserter(partsToAdd, partsToAdd.end()));

    for (DgnGeomPartId partId : partsToAdd)
        {
        if (BentleyStatus::SUCCESS != dgnDb.GeomParts().InsertElementGeomUsesParts(m_elementId, partId))
            stat = DgnDbStatus::WriteError;
        }

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeomStream::WriteGeomStreamAndStep(DgnDbR dgnDb, Utf8CP table, Utf8CP colname, uint64_t rowId, Statement& stmt, int stmtcolidx) const
    {
    SnappyToBlob& snappy = dgnDb.Elements().GetSnappyTo();

    snappy.Init();
    if (0 < GetSize())
        {
        GeomBlobHeader header(*this);
        snappy.Write((Byte const*) &header, sizeof(header));
        snappy.Write(GetData(), GetSize());
        }

    uint32_t zipSize = snappy.GetCompressedSize();
    if (0 < zipSize)
        {
        if (1 == snappy.GetCurrChunk())
            stmt.BindBlob(stmtcolidx, snappy.GetChunkData(0), zipSize, Statement::MakeCopy::No);
        else
            stmt.BindZeroBlob(stmtcolidx, zipSize); // more than one chunk in geom stream
        }

    if (BE_SQLITE_DONE != stmt.Step())
        return DgnDbStatus::WriteError;

    if (1 == snappy.GetCurrChunk())
        return DgnDbStatus::Success;

    StatusInt status = snappy.SaveToRow(dgnDb, table, colname, rowId);
    return SUCCESS != status ? DgnDbStatus::WriteError : DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeomStream::ReadGeomStream(DgnDbR dgnDb, Utf8CP table, Utf8CP colname, uint64_t rowId)
    {
    auto& pool = dgnDb.Elements();
    SnappyFromBlob& snappy = pool.GetSnappyFrom();

    if (ZIP_SUCCESS != snappy.Init(pool.GetDgnDb(), table, colname, rowId))
        return DgnDbStatus::Success; // this row has no geometry

    GeomBlobHeader header(snappy);
    if ((GeomBlobHeader::Signature != header.m_signature) || 0 == header.m_size)
        {
        BeAssert(false);
        return DgnDbStatus::ReadError;
        }

    ReserveMemory(header.m_size);

    uint32_t actuallyRead;
    snappy.ReadAndFinish(GetDataR(), GetSize(), actuallyRead);

    if (actuallyRead != GetSize())
        {
        BeAssert(false);
        return DgnDbStatus::ReadError;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::WriteGeomStream(Statement& stmt, DgnDbR dgnDb)
    {
    return m_geom.WriteGeomStreamAndStep(dgnDb, DGN_TABLE(DGN_CLASSNAME_ElementGeom), GEOM_Column, m_elementId.GetValue(), stmt, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement3d::_BindPlacement(Statement& stmt)
    {
    if (!m_placement.IsValid())
        return DgnDbStatus::NoGeometry;

    stmt.BindBlob(2, &m_placement, sizeof(m_placement), Statement::MakeCopy::No);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement3d::_LoadFromDb()
    {
    DgnDbStatus stat = T_Super::_LoadFromDb();
    if (DgnDbStatus::Success != stat)
        return stat;

    CachedStatementPtr stmt=GetDgnDb().Elements().GetStatement("SELECT Placement FROM " DGN_TABLE(DGN_CLASSNAME_ElementGeom) " Where ElementId=?");
    stmt->BindId(1, m_elementId);

    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::Success; // it is legal to have an element with no geometry.

    if (stmt->GetColumnBytes(0) != sizeof(m_placement))
        {
        BeAssert(false); 
        return DgnDbStatus::ReadError;
        }

    memcpy(&m_placement, stmt->GetValueBlob(0), sizeof(m_placement));
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    04/15
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalElementPtr PhysicalElement::Create(PhysicalModelR model, DgnCategoryId categoryId)
    {
    if (!categoryId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return new PhysicalElement(CreateParams(model.GetDgnDb(), model.GetModelId(), PhysicalElement::QueryClassId(model.GetDgnDb()), categoryId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId PhysicalElement::QueryClassId(DgnDbR db)
    {
    return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId DrawingElement::QueryClassId(DgnDbR db)
    {
    return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_DrawingElement));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement2d::_BindPlacement(Statement& stmt)
    {
    if (!m_placement.IsValid())
        return DgnDbStatus::NoGeometry;

    stmt.BindBlob(2, &m_placement, sizeof(m_placement), Statement::MakeCopy::No);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement2d::_LoadFromDb()
    {
    DgnDbStatus stat = T_Super::_LoadFromDb();
    if (DgnDbStatus::Success != stat)
        return stat;

    CachedStatementPtr stmt=GetDgnDb().Elements().GetStatement("SELECT Placement FROM " DGN_TABLE(DGN_CLASSNAME_ElementGeom) " Where ElementId=?");
    stmt->BindId(1, m_elementId);

    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::Success; // it is legal to have an element with no geometry.

    if (stmt->GetColumnBytes(0) != sizeof(m_placement))
        {
        BeAssert(false);
        return DgnDbStatus::ReadError;
        }

    memcpy(&m_placement, stmt->GetValueBlob(0), sizeof(m_placement));
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/06
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem* GeometricElement::GetQvElem(uint32_t id) const
    {
    T_QvElemSet* qvElems = GetQvElems(false);
    return qvElems ? qvElems->Find(id) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::CreateParams::RelocateToDestinationDb(DgnImportContext& importer)
    {
    m_modelId = importer.FindModelId(m_modelId);
    m_classId = importer.RemapClassId(m_classId);
    }

/*---------------------------------------------------------------------------------**//**
* NEEDSWORK: Not clear in what contexts an element's code should be copied, or not.
* GetCreateParamsForImport() only copies the code if we're copying between DBs.
* But _CopyFrom() always copies it. Which is what we want when called from CopyForEdit(),
* but not what we want from _CloneForImport() or Clone(), which both use their own CreateParams
* to specify the desired code.
* So - Clone-like methods use this to ensure the code is copied or not copied appropriately.
* @bsistruct                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::CopyForCloneFrom(DgnElementCR src)
    {
    DgnElement::Code code = GetCode();
    _CopyFrom(src);
    SetCode(code);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnElement::_Clone(DgnDbStatus* inStat, DgnElement::CreateParams const* params) const
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);

    // Perform input params validation. Code must be different and element id should be invalid...
    if (nullptr != params)
        {
        if (params->m_id.IsValid())
            {
            stat = DgnDbStatus::InvalidId;
            return nullptr;
            }
            
        if (params->m_code == GetCode())
            {
            stat = DgnDbStatus::InvalidName;
            return nullptr;
            }
        }

    DgnElementPtr cloneElem = GetElementHandler().Create(nullptr != params ? *params : DgnElement::CreateParams(GetDgnDb(), GetModelId(), GetElementClassId(), Code(), DgnElementId()));
    if (!cloneElem.IsValid())
        {
        stat = DgnDbStatus::BadRequest;
        return nullptr;
        }

    cloneElem->CopyForCloneFrom(*this);

    stat = DgnDbStatus::Success;
    return cloneElem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnElement::_CloneForImport(DgnDbStatus* inStat, DgnModelR destModel, DgnImportContext& importer) const
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);

    DgnElement::CreateParams params = GetCreateParamsForImport(destModel, importer);
    params.m_modelId = destModel.GetModelId();

    DgnElementPtr cloneElem = GetElementHandler().Create(params);

    if (!cloneElem.IsValid())
        {
        stat = DgnDbStatus::BadRequest;
        return nullptr;
        }

    cloneElem->CopyForCloneFrom(*this);

    if (importer.IsBetweenDbs())
        {
        cloneElem->_RemapIds(importer);
        cloneElem->_AdjustPlacementForImport(importer);
        }

    stat = DgnDbStatus::Success;
    return cloneElem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_CopyFrom(DgnElementCR other)
    {
    if (&other == this)
        return;

    // Copying between DgnDbs is allowed. Caller must do Id remapping.
    m_code       = other.m_code;
    m_parentId   = other.m_parentId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::Code::RelocateToDestinationDb(DgnImportContext& importer)
    {
    m_authority = importer.RemapAuthorityId(m_authority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_RemapIds(DgnImportContext& importer)
    {
    BeAssert(importer.IsBetweenDbs());
    m_code.RelocateToDestinationDb(importer);
    m_parentId   = importer.FindElementId(m_parentId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::_CopyFrom(DgnElementCR other)
    {
    T_Super::_CopyFrom(other);

    GeometricElementCP otherGeom = other.ToGeometricElement();
    if (otherGeom)
        {
        m_categoryId = otherGeom->GetCategoryId();
        SaveGeomStream(&otherGeom->m_geom);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::_RemapIds(DgnImportContext& importer)
    {
    BeAssert(importer.IsBetweenDbs());
    T_Super::_RemapIds(importer);
    m_categoryId = importer.RemapCategory(m_categoryId);
    ElementGeomIO::Import(m_geom, m_geom, importer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::CreateParams DgnElement::GetCreateParamsForImport(DgnModelR destModel, DgnImportContext& importer) const
    {
    CreateParams parms(importer.GetDestinationDb(), GetModelId(), GetElementClassId());
    DgnAuthorityCPtr authority = GetCode().IsValid() ? GetDgnDb().Authorities().GetAuthority(GetCode().GetAuthority()) : nullptr;
    if (authority.IsValid())
        parms.m_code = authority->CloneCodeForImport(*this, destModel, importer);

    if (importer.IsBetweenDbs())
        parms.RelocateToDestinationDb(importer);

    return parms;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElement::Import(DgnDbStatus* stat, DgnModelR destModel, DgnImportContext& importer) const
    {
    if (nullptr != stat)
        *stat = DgnDbStatus::Success;

    DgnElementPtr cc = _CloneForImport(stat, destModel, importer); // (also calls _CopyFrom and _RemapIds)
    if (!cc.IsValid())
        return DgnElementCPtr();

    DgnElementCPtr ccp = cc->Insert(stat);
    if (!ccp.IsValid())
        return ccp;

    importer.AddElementId(GetElementId(), ccp->GetElementId());
    return ccp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement3d::_CopyFrom(DgnElementCR other)
    {
    T_Super::_CopyFrom(other);

    DgnElement3dCP el3d = other.ToElement3d();
    if (el3d)
        m_placement = el3d->m_placement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement3d::_AdjustPlacementForImport(DgnImportContext const& importer)
    {
    m_placement.GetOriginR().Add(DPoint3d::From(importer.GetOriginOffset()));
    m_placement.GetAnglesR().AddYaw(importer.GetYawAdjustment());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement2d::_CopyFrom(DgnElementCR other)
    {
    T_Super::_CopyFrom(other);

    DgnElement2dCP el2d = other.ToElement2d();
    if (el2d)
        m_placement = el2d->m_placement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandlerR DgnElement::GetElementHandler() const
    {
    return *dgn_ElementHandler::Element::FindHandler(GetDgnDb(), m_classId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings  09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDomain::Handler* DgnElement::GetItemOrElementHandler() const
    {
    DgnClassId classId = DgnElement::Item::QueryExistingItemClass(*this);

    if (classId.IsValid())
        return dgn_AspectHandler::Aspect::FindHandler(GetDgnDb(), classId);

    return &GetElementHandler();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnElement::CopyForEdit() const
    {
    DgnElementPtr newEl = GetElementHandler()._CreateInstance(DgnElement::CreateParams(GetDgnDb(), m_modelId, m_classId, GetCode(), m_elementId, m_parentId));
    BeAssert(typeid(*newEl) == typeid(*this)); // this means the ClassId of the element does not match the type of the element. Caller should find out why.
    newEl->_CopyFrom(*this);
    return newEl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
GeomStream::GeomStream(GeomStream const& other)
    {
    m_size = m_allocSize = 0;
    m_data = nullptr;
    SaveData(other.m_data, other.m_size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
void GeomStream::ReserveMemory(uint32_t size)
    {
    m_size = size;
    if (size<=m_allocSize)
        return;

    m_data = (uint8_t*) realloc(m_data, size);
    m_allocSize = size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
GeomStream::~GeomStream()
    {
    FREE_AND_CLEAR(m_data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
GeomStream& GeomStream::operator= (GeomStream const& other)
    {
    if (this != &other)
        SaveData(other.m_data, other.m_size);

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
void GeomStream::SaveData(uint8_t const* data, uint32_t size)
    {
    ReserveMemory(size);
    if (data)
        memcpy(m_data, data, size);
    }


static const double s_smallVal = .0005;
inline static void fixRange(double& low, double& high) {if (low==high) {low-=s_smallVal; high+=s_smallVal;}}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d Placement3d::CalculateRange() const
    {
    if (!IsValid())
        return AxisAlignedBox3d();

    AxisAlignedBox3d range;
    GetTransform().Multiply(range, m_boundingBox);

    // low and high are not allowed to be equal
    fixRange(range.low.x, range.high.x);
    fixRange(range.low.y, range.high.y);
    fixRange(range.low.z, range.high.z);

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d Placement2d::CalculateRange() const
    {
    if (!IsValid())
        return AxisAlignedBox3d();

    AxisAlignedBox3d range;
    GetTransform().Multiply(range, DRange3d::From(&m_boundingBox.low, 2, 0.0));

    // low and high are not allowed to be equal
    fixRange(range.low.x, range.high.x);
    fixRange(range.low.y, range.high.y);
    range.low.z = -s_smallVal;
    range.high.z = s_smallVal;

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElement::Update(DgnDbStatus* stat) {return GetDgnDb().Elements().Update(*this, stat);}
DgnElementCPtr DgnElement::Insert(DgnDbStatus* stat) {return GetDgnDb().Elements().Insert(*this, stat);}
DgnDbStatus DgnElement::Delete() const {return GetDgnDb().Elements().Delete(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ElementGroup::InsertMember(DgnElementCR member) const
    {
    if (!GetElementId().IsValid() || !member.GetElementId().IsValid())
        return DgnDbStatus::InvalidId;

    DgnDbStatus status = _OnMemberInsert(member); // give subclass a chance to reject member insert
    if (DgnDbStatus::Success != status)
        return status;

    CachedECSqlStatementPtr statement = GetDgnDb().GetPreparedECSqlStatement("INSERT INTO " DGN_SCHEMA(DGN_RELNAME_ElementGroupHasMembers) "(SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");

    statement->BindId(1, GetElementClassId());
    statement->BindId(2, GetElementId());
    statement->BindId(3, member.GetElementClassId());
    statement->BindId(4, member.GetElementId());

    if (BE_SQLITE_DONE != statement->Step())
        return DgnDbStatus::BadRequest;

    _OnMemberInserted(member); // notify subclass that member was inserted
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ElementGroup::DeleteMember(DgnElementCR member) const
    {
    if (!GetElementId().IsValid() || !member.GetElementId().IsValid())
        return DgnDbStatus::InvalidId;

    DgnDbStatus status = _OnMemberDelete(member); // give subclass a chance to reject member delete
    if (DgnDbStatus::Success != status)
        return status;

    CachedStatementPtr statement=GetDgnDb().Elements().GetStatement("DELETE FROM " DGN_TABLE(DGN_RELNAME_ElementGroupHasMembers) " WHERE GroupId=? AND MemberId=?");
    statement->BindId(1, GetElementId());
    statement->BindId(2, member.GetElementId());

    if (BE_SQLITE_DONE != statement->Step())
        return DgnDbStatus::BadRequest;

    _OnMemberDeleted(member); // notify subclass that member was deleted
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ElementGroup::DeleteAllMembers() const
    {
    DgnElementIdSet memberIds = QueryMembers();
    for (DgnElementId memberId : memberIds)
        {
        DgnElementCPtr member = GetDgnDb().Elements().GetElement(memberId);
        if (!member.IsValid())
            {
            BeAssert(false); // indicates an orphaned relationship
            continue;
            }

        DgnDbStatus deleteMemberStatus = DeleteMember(*member);
        if (DgnDbStatus::Success != deleteMemberStatus)
            return deleteMemberStatus;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet ElementGroup::_QueryMembers() const
    {
    CachedStatementPtr statement=GetDgnDb().Elements().GetStatement("SELECT MemberId FROM " DGN_TABLE(DGN_RELNAME_ElementGroupHasMembers) " WHERE GroupId=?");
    statement->BindId(1, GetElementId());

    DgnElementIdSet elementIdSet;
    while (BE_SQLITE_ROW == statement->Step())
        elementIdSet.insert(statement->GetValueId<DgnElementId>(0));

    return elementIdSet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ElementGroup::QueryFromMember(DgnDbR db, DgnClassId groupClassId, DgnElementId memberId)
    {
    if (!groupClassId.IsValid() || !memberId.IsValid())
        return DgnElementId();

    CachedStatementPtr statement=db.Elements().GetStatement("SELECT Rel.GroupId FROM " DGN_TABLE(DGN_RELNAME_ElementGroupHasMembers) " Rel INNER JOIN " DGN_TABLE(DGN_CLASSNAME_Element) " Elm ON Elm.Id=Rel.GroupId WHERE Elm.ECClassId=? AND Rel.MemberId=?");
    statement->BindId(1, groupClassId);
    statement->BindId(2, memberId);

    return (BE_SQLITE_ROW != statement->Step()) ? DgnElementId() : statement->GetValueId<DgnElementId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                    06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId ElementGroup::QueryClassId(DgnDbR db)
    {
    return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ElementGroup));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_AspectHandler::Aspect* dgn_AspectHandler::Aspect::FindHandler(DgnDbR db, DgnClassId handlerId) 
    {
    // quick check for a handler already known
    DgnDomain::Handler* handler = db.Domains().LookupHandler(handlerId);
    if (nullptr != handler)
        return dynamic_cast<dgn_AspectHandler::Aspect*>(handler);

    // not there, check via base classes
    handler = db.Domains().FindHandler(handlerId, db.Domains().GetClassId(GetHandler()));
    return handler ? dynamic_cast<dgn_AspectHandler::Aspect*>(handler) : nullptr;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::Aspect::Aspect()
    {
    m_changeType = ChangeType::None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::Aspect::InsertThis(DgnElementCR el)
    {
    DgnDbStatus status = _InsertInstance(el);
    if (DgnDbStatus::Success != status)
        return status;
    return _UpdateProperties(el); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId  DgnElement::Aspect::GetECClassId(DgnDbR db) const
    {
    return DgnClassId(db.Schemas().GetECClassId(_GetECSchemaName(), _GetECClassName()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP DgnElement::Aspect::GetECClass(DgnDbR db) const
    {
    return db.Schemas().GetECClass(_GetECSchemaName(), _GetECClassName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData::DropMe DgnElement::Aspect::_OnInserted(DgnElementCR el)
    {
    if (ChangeType::Delete != m_changeType) // (caller can cancel an add by calling Delete)
        InsertThis(el);
    
    m_changeType = ChangeType::None; // (Just in case)

    return DropMe::Yes;     // this scheduled change has been processed, so remove it.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData::DropMe DgnElement::Aspect::_OnUpdated(DgnElementCR modified, DgnElementCR original)
    {
    if (ChangeType::None == m_changeType)
        return DropMe::Yes;     // Was just a cached instance? Drop it now, so that it does not become stale.

    if (ChangeType::Delete == m_changeType)
        {
        _DeleteInstance(modified);
        }
    else
        {
        DgnDbR db = modified.GetDgnDb();
        ECInstanceKey existing = _QueryExistingInstanceKey(modified);
        if (existing.IsValid() && (existing.GetECClassId() != GetECClassId(db).GetValue()))
            {
            _DeleteInstance(modified);
            existing = ECInstanceKey();  //  trigger an insert below
            }
            
        if (!existing.IsValid())
            InsertThis(modified);
        else
            _UpdateProperties(modified);
        }

    m_changeType = ChangeType::None; // (Just in case)

    return DropMe::Yes; // this scheduled change has been processed, so remove it.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<DgnElement::Aspect> DgnElement::Aspect::_CloneForImport(DgnElementCR el, DgnImportContext& importer) const
    {
    DgnClassId classid = GetECClassId(el.GetDgnDb());
    if (!el.GetElementId().IsValid() || !classid.IsValid())
        return nullptr;

    dgn_AspectHandler::Aspect* handler = dgn_AspectHandler::Aspect::FindHandler(el.GetDgnDb(), classid);
    if (nullptr == handler)
        return nullptr;

    RefCountedPtr<DgnElement::Aspect> aspect = handler->_CreateInstance().get();
    if (!aspect.IsValid())
        return nullptr;

    // *** WIP_IMPORTER - we read the *persistent* properties -- we don't copy the in-memory copies -- pending changes are ignored!

    if (DgnDbStatus::Success != aspect->_LoadProperties(el))
        return nullptr;

    // Assume that there are no IDs to be remapped.

    return aspect;
    }

/*=================================================================================**//**
* @bsimethod                                    Sam.Wilson      06/15
+===============+===============+===============+===============+===============+======*/
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
struct MultiAspectMux : DgnElement::AppData
{
    ECClassCR m_ecclass;
    bvector<RefCountedPtr<DgnElement::MultiAspect>> m_instances;

    static Key& GetKey(ECClassCR cls) {return *(Key*)&cls;}
    Key& GetKey(DgnDbR db) {return GetKey(m_ecclass);}

    static MultiAspectMux* Find(DgnElementCR, ECClassCR);
    static MultiAspectMux& Get(DgnElementCR, ECClassCR);

    MultiAspectMux(ECClassCR cls) : m_ecclass(cls) {;}
    DropMe _OnInserted(DgnElementCR el) override;
    DropMe _OnUpdated(DgnElementCR modified, DgnElementCR original) override;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
MultiAspectMux* MultiAspectMux::Find(DgnElementCR el, ECClassCR cls)
    {
    AppData* appData = el.FindAppData(GetKey(cls));
    if (nullptr == appData)
        return nullptr;
    MultiAspectMux* mux = dynamic_cast<MultiAspectMux*>(appData);
    BeAssert(nullptr != mux && "The same ECClass cannot have both Unique and MultiAspects");
    return mux;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
MultiAspectMux& MultiAspectMux::Get(DgnElementCR el, ECClassCR cls)
    {
    MultiAspectMux* mux = Find(el,cls);
    if (nullptr == mux)
        el.AddAppData(GetKey(cls), mux = new MultiAspectMux(cls));
    return *mux;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData::DropMe MultiAspectMux::_OnInserted(DgnElementCR el)
    {
    for (auto aspect : m_instances)
        aspect->_OnInserted(el);

    return DropMe::Yes; // all scheduled changes have been processed, so remove them.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData::DropMe MultiAspectMux::_OnUpdated(DgnElementCR modified, DgnElementCR original)
    {
    for (auto aspect : m_instances)
        aspect->_OnUpdated(modified, original);

    return DropMe::Yes; // all scheduled changes have been processed, so remove them.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::MultiAspect::_DeleteInstance(DgnElementCR el)
    {
    // I am assuming that the ElementOwnsAspects ECRelationship is either just a foreign key column on the aspect or that ECSql somehow deletes the relationship instance automatically.
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("DELETE FROM %s WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindId(1, m_instanceId);
    BeSQLite::DbResult status = stmt->Step();
    return (BeSQLite::BE_SQLITE_DONE == status) ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::MultiAspect::_InsertInstance(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("INSERT INTO %s ([ElementId]) VALUES (?)", GetFullEcSqlClassName().c_str()));
    stmt->BindId(1, el.GetElementId());

    ECInstanceKey key;
    if (BeSQLite::BE_SQLITE_DONE != stmt->Step(key))
        return DgnDbStatus::WriteError;

    m_instanceId = key.GetECInstanceId();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::MultiAspect* DgnElement::MultiAspect::GetAspectP(DgnElementR el, ECClassCR cls, ECInstanceId id)
    {
    //  First, see if we alrady have this particular MultiAspect cached
    MultiAspectMux* mux = MultiAspectMux::Find(el,cls);
    if (nullptr != mux)
        {
        for (RefCountedPtr<MultiAspect> aspect : mux->m_instances)
            {
            if (aspect->GetAspectInstanceId() != id)
                continue;
            if (aspect->m_changeType == ChangeType::Delete)
                return nullptr;
            aspect->m_changeType = ChangeType::Write; // caller intends to modify the aspect
            return aspect.get();
            }
        }

    //  First time we've been asked for this particular aspect. Cache it.
    dgn_AspectHandler::Aspect* handler = dgn_AspectHandler::Aspect::FindHandler(el.GetDgnDb(), DgnClassId(cls.GetId()));
    if (nullptr == handler)
        return nullptr;

    RefCountedPtr<MultiAspect> aspect = dynamic_cast<MultiAspect*>(handler->_CreateInstance().get());
    if (!aspect.IsValid())
        return nullptr;

    aspect->m_instanceId = id;

    if (DgnDbStatus::Success != aspect->_LoadProperties(el))
        return nullptr;

    MultiAspectMux::Get(el,cls).m_instances.push_back(aspect);

    aspect->m_changeType = ChangeType::Write; // caller intends to modify the aspect

    return aspect.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::MultiAspect::AddAspect(DgnElementR el, MultiAspect& aspect)
    {
    ECClassCP cls = aspect.GetECClass(el.GetDgnDb());
    if (nullptr == cls)
        {
        BeAssert(false && "aspect must know its class");
        return;
        }
    MultiAspectMux::Get(el,*cls).m_instances.push_back(&aspect);
    aspect.m_changeType = ChangeType::Write;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Sam.Wilson                    03/2015
//---------------------------------------------------------------------------------------
ECInstanceKey DgnElement::MultiAspect::_QueryExistingInstanceKey(DgnElementCR el)
    {
    // My m_instanceId field is valid if and only if I was just inserted or was loaded from an existing instance.
    return ECInstanceKey(GetECClassId(el.GetDgnDb()).GetValue(), m_instanceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::UniqueAspect* DgnElement::UniqueAspect::Find(DgnElementCR el, ECClassCR cls)
    {
    AppData* appData = el.FindAppData(GetKey(cls));
    if (nullptr == appData)
        return nullptr;
    UniqueAspect* aspect = dynamic_cast<UniqueAspect*>(appData);
    BeAssert(nullptr != aspect && "The same ECClass cannot have both Unique and MultiAspects");
    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::UniqueAspect::SetAspect(DgnElementR el, UniqueAspect& newAspect)
    {
    if (nullptr != dynamic_cast<Item*>(&newAspect))
        {
        BeAssert(false && "You must use the DgnElement::Item class to work with Items");
        return;
        }

    SetAspect0(el, newAspect);
    newAspect.m_changeType = ChangeType::Write;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::UniqueAspect const* DgnElement::UniqueAspect::GetAspect(DgnElementCR el, ECClassCR cls)
    {
    UniqueAspect const* aspect = Find(el,cls);
    if (nullptr == aspect)
        {
        aspect = Load(el,DgnClassId(cls.GetId()));
        }
    else
        {
        if (aspect->m_changeType == ChangeType::Delete)
            aspect = nullptr;
        }

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::UniqueAspect* DgnElement::UniqueAspect::GetAspectP(DgnElementR el, ECClassCR cls)
    {
    UniqueAspect* aspect = const_cast<UniqueAspect*>(GetAspect(el,cls));
    if (nullptr == aspect)
        return aspect;
    aspect->m_changeType = ChangeType::Write;
    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::UniqueAspect::SetAspect0(DgnElementCR el, UniqueAspect& newItem)
    {
    Key& key = newItem.GetKey(el.GetDgnDb());
    el.DropAppData(key);  // remove any existing cached Item
    el.AddAppData(key, &newItem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<DgnElement::UniqueAspect> DgnElement::UniqueAspect::Load0(DgnElementCR el, DgnClassId classid)
    {
    if (!el.GetElementId().IsValid() || !classid.IsValid())
        return nullptr;

    dgn_AspectHandler::Aspect* handler = dgn_AspectHandler::Aspect::FindHandler(el.GetDgnDb(), classid);
    if (nullptr == handler)
        return nullptr;

    RefCountedPtr<DgnElement::UniqueAspect> aspect = dynamic_cast<DgnElement::UniqueAspect*>(handler->_CreateInstance().get());
    if (!aspect.IsValid())
        return nullptr;

    if (DgnDbStatus::Success != aspect->_LoadProperties(el))
        return nullptr;

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::UniqueAspect* DgnElement::UniqueAspect::Load(DgnElementCR el, DgnClassId classid)
    {
    RefCountedPtr<DgnElement::UniqueAspect> aspect = Load0(el, classid);
    if (!aspect.IsValid())
        return nullptr;

    if (nullptr != dynamic_cast<Item*>(aspect.get()))
        {
        BeAssert(false); // You must use the DgnElement::Item class to load Items
        return nullptr;
        }

    SetAspect0(el, *aspect);
    aspect->m_changeType = ChangeType::None; // aspect starts out clean
    return aspect.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::UniqueAspect::_InsertInstance(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("INSERT INTO %s (ECInstanceId) VALUES(?)", GetFullEcSqlClassName().c_str()));
    stmt->BindId(1, GetAspectInstanceId(el));
    DbResult status = stmt->Step();
    return (BE_SQLITE_DONE == status) ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::UniqueAspect::_DeleteInstance(DgnElementCR el)
    {
    // I am assuming that the ElementOwnsAspects ECRelationship is either just a foreign key column on the aspect or that ECSql somehow deletes the relationship instance automatically.
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("DELETE FROM %s WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindId(1, GetAspectInstanceId(el));
    DbResult status = stmt->Step();
    return (BE_SQLITE_DONE == status) ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::Item::_DeleteInstance(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement("DELETE FROM " DGN_SCHEMA(DGN_CLASSNAME_ElementItem) " WHERE ECInstanceId=?");
    stmt->BindId(1, GetAspectInstanceId(el));
    DbResult status = stmt->Step();
    return (BE_SQLITE_DONE == status) ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Sam.Wilson                    03/2015
//---------------------------------------------------------------------------------------
ECInstanceKey DgnElement::UniqueAspect::_QueryExistingInstanceKey(DgnElementCR el)
    {
    // We know what the class and the ID of an instance *would be* if it exists. See if such an instance actually exists.
    DgnClassId classId = GetECClassId(el.GetDgnDb());

    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT ECInstanceId FROM %s WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindId(1, el.GetElementId());
    if (BE_SQLITE_ROW != stmt->Step())
        return ECInstanceKey();

    // And we know the ID. See if such an instance actually exists.
    return ECInstanceKey(classId.GetValue(), GetAspectInstanceId(el));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Sam.Wilson                    03/2015
//---------------------------------------------------------------------------------------
ECInstanceKey DgnElement::Item::_QueryExistingInstanceKey(DgnElementCR el)
    {
    // We know the ID, and we know that the instance will be in the dgn.ElementItem table if it exists. See if it's there.
    DgnClassId classId = QueryExistingItemClass(el);
    if (!classId.IsValid())
        return ECInstanceKey();
    return ECInstanceKey(classId.GetValue(), el.GetElementId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Sam.Wilson                    03/2015
//---------------------------------------------------------------------------------------
DgnClassId DgnElement::Item::QueryExistingItemClass(DgnElementCR el)
    {
    // We know the ID, and we know that the instance will be in the dgn.ElementItem table if it exists. See if it's there.
    CachedStatementPtr getItemClass;
    el.GetDgnDb().GetCachedStatement(getItemClass, "SELECT ECClassId FROM " DGN_TABLE(DGN_CLASSNAME_ElementItem) " WHERE ElementId=?");
    getItemClass->BindId(1, el.GetElementId());
    if (BE_SQLITE_ROW != getItemClass->Step())
        return DgnClassId();
    return DgnClassId(getItemClass->GetValueId<DgnClassId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::Item::LoadPropertiesIntoInstance(IECInstancePtr& instance, DgnElementCR el)
    {
    DgnDbR db = el.GetDgnDb();

    ECInstanceKey key = _QueryExistingInstanceKey(el);
    ECClassCP ecclass = db.Schemas().GetECClass(key.GetECClassId());
    if (nullptr == ecclass)
        return DgnDbStatus::NotFound;

    ECSqlSelectBuilder b;
    b.Select("*").From(*ecclass).Where("ECInstanceId=?");
    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement(b.ToString().c_str());
    stmt->BindId(1, el.GetElementId());
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::ReadError;

    ECInstanceECSqlSelectAdapter reader(*stmt);     // *** NEEDS WORK: Use a cached ECInstanceECSqlSelectAdapter!!!!!
    instance = reader.GetInstance();
    if (!instance.IsValid())
        return DgnDbStatus::ReadError;
    
    Utf8Char idStrBuffer[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH];
    ECInstanceIdHelper::ToString(idStrBuffer, ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH, el.GetElementId());
    instance->SetInstanceId(idStrBuffer);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP DgnElement::Item::GetECSchemaNameOfInstance(IECInstanceCP instance)
    {
    if (nullptr == instance)
        {
        BeAssert(false && "Item has no instance");
        return nullptr;
        }
    
    return instance->GetClass().GetSchema().GetName().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP DgnElement::Item::GetECClassNameOfInstance(IECInstanceCP instance)
    {
    if (nullptr == instance)
        {
        BeAssert(false && "Item has no instance");
        return nullptr;
        }
    
    return instance->GetClass().GetName().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::Item* DgnElement::Item::Find(DgnElementCR el)
    {
    return (DgnElement::Item*)el.FindAppData(GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::Item::SetItem0(DgnElementCR el, Item& newItem)
    {
    el.DropAppData(GetKey());  // remove any existing cached Item
    el.AddAppData(GetKey(), &newItem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::Item::SetItem(DgnElementR el, Item& newItem)
    {
    SetItem0(el, newItem);
    newItem.m_changeType = ChangeType::Write;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::Item const* DgnElement::Item::GetItem(DgnElementCR el)
    {
    Item const* item = Find(el);
    if (nullptr == item)
        {
        item = Load(el);
        }
    else
        {
        if (item->m_changeType == ChangeType::Delete)
            item = nullptr;
        }
    return item;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::Item* DgnElement::Item::GetItemP(DgnElementR el)
    {
    Item* item = const_cast<Item*>(GetItem(el));
    if (nullptr == item)
        return item;
    item->m_changeType = ChangeType::Write;
    return item;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::Item* DgnElement::Item::Load(DgnElementCR el)
    {
    RefCountedPtr<DgnElement::UniqueAspect> aspect = Load0(el, QueryExistingItemClass(el));
    if (!aspect.IsValid())
        return nullptr;

    DgnElement::Item* item = dynamic_cast<DgnElement::Item*>(aspect.get());
    if (nullptr == item)
        {
        BeAssert(false && "You may use the DgnElement::Item class to load Items only");
        return nullptr;
        }

    SetItem0(el, *item);
    return item;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::Item::CallGenerateGeometry(DgnElementR el, GenerateReason reason)
    {
    GeometricElementP gel = el.ToGeometricElementP();
    return nullptr == gel ? DgnDbStatus::Success : _GenerateElementGeometry(*gel, reason);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::Item::GenerateElementGeometry(GeometricElementR el, GenerateReason reason)
    {
    Item* item = GetItemP(el);
    return nullptr == item ? DgnDbStatus::NotFound : item->_GenerateElementGeometry(el, reason);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnElement::Item::ExecuteEGA(DgnElementR el, DPoint3dCR origin, YawPitchRollAnglesCR angles, IECInstanceCR egaInstance)
    {
    ECClassCR ecclass = egaInstance.GetClass();
    IECInstancePtr ca = ecclass.GetCustomAttribute("EGASpecifier");
    if (!ca.IsValid())
        return DgnDbStatus::NotEnabled;

    ECValue egaType, egaName, egaInputs;
    ca->GetValue(egaType, "Type");
    ca->GetValue(egaName, "Name");
    ca->GetValue(egaInputs, "Inputs");

    Utf8String tsName(egaName.GetUtf8CP());

    if (0 == BeStringUtilities::Stricmp("JavaScript", egaType.GetUtf8CP()))
        {
        //  ----------------------------------------------------------------------------------
        //  JavaScript EGA
        //  ----------------------------------------------------------------------------------
        Json::Value json(Json::objectValue);
        if (BSISUCCESS != ECUtils::ToJsonPropertiesFromECProperties(json, egaInstance, Utf8String(egaInputs.GetUtf8CP())))
            return DgnDbStatus::BadArg;

        int retval;
        DgnDbStatus xstatus = DgnScript::ExecuteEga(retval, el, tsName.c_str(), origin, angles, json);
        if (xstatus != DgnDbStatus::Success)
            return xstatus;

        return (0 == retval) ? DgnDbStatus::Success : DgnDbStatus::WriteError;
        }

    if (0 == BeStringUtilities::Stricmp("ComponentModel", egaType.GetUtf8CP()))
        {
        return ExecuteComponentSolutionEGA(el, origin, angles, egaInstance, tsName, Utf8String(egaInputs.GetUtf8CP()), *this);
        }

    BeAssert(false && "TBD - Unrecognized EGA type.");
    return DgnDbStatus::NotEnabled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus InstanceBackedItem::_GenerateElementGeometry(GeometricElementR el, GenerateReason)
    {
    Placement3d placement;
    DgnElement3dP e3d = el.ToElement3dP();
    if (nullptr != e3d)
        placement = e3d->GetPlacement();
    else
        {
        DgnElement2dP e2d = el.ToElement2dP();
        Placement2d p2d = e2d->GetPlacement();
        DPoint3d o3d = DPoint3d::From(p2d.GetOrigin().x, p2d.GetOrigin().y, 0);
        YawPitchRollAngles a3d = YawPitchRollAngles::FromDegrees(p2d.GetAngle().Degrees(), 0, 0);
        ElementAlignedBox2d b2d = p2d.GetElementBoxR();
        ElementAlignedBox3d b3d;
        b3d.low = DPoint3d::From(b2d.low.x, b2d.low.y, 0);
        b3d.high = DPoint3d::From(b2d.high.x, b2d.high.y, 0);
        placement = Placement3d(o3d, a3d, b3d);
        }
    return ExecuteEGA(el, placement.GetOrigin(), placement.GetAngles(), *m_instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceBackedItem::SetInstanceId(ECInstanceId eid)
    {
    Utf8Char idStrBuffer[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH];
    ECInstanceIdHelper::ToString(idStrBuffer, ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH, eid);
    m_instance->SetInstanceId(idStrBuffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus InstanceBackedItem::_LoadProperties(DgnElementCR el)
    {
    ECInstanceId eid(el.GetElementId().GetValue());

    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement("SELECT * FROM " DGN_TABLE(DGN_CLASSNAME_ElementItem) " WHERE ECInstanceId=?");
    stmt->BindId(1, eid);
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::ReadError;

    ECInstanceECSqlSelectAdapter reader(*stmt);
    m_instance = reader.GetInstance();
    if (!m_instance.IsValid())
        return DgnDbStatus::ReadError;
    
    SetInstanceId(eid);

    return DgnDbStatus::Success;
    }

BEGIN_UNNAMED_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct CachedECInstanceUpdaters : Db::AppData
{
    bmap<DgnClassId, ECInstanceUpdater*> m_cache;

    ~CachedECInstanceUpdaters()
        {
        DeleteAll();
        }

    void DeleteAll()
        {
        for (auto e : m_cache)
            delete e.second;    
          
        m_cache.clear();
        }

    static CachedECInstanceUpdaters& Get(DgnDbR db)
        {
        static Key s_key;
        auto ad = dynamic_cast<CachedECInstanceUpdaters*>(db.FindAppData(s_key));
        if (nullptr == ad)
            db.AddAppData(s_key, (ad = new CachedECInstanceUpdaters));
        return *ad;
        }

    void TrimCache()
        {
        if (m_cache.size() < 10)
            return;

        DeleteAll();
        }

    ECInstanceUpdater& GetECInstanceUpdater0(DgnDbR db, ECClassCR ecClass)
        {
        DgnClassId clsid(ecClass.GetId());
        auto i = m_cache.find(clsid);
        if (i != m_cache.end())
            return *i->second;
        TrimCache();
        ECInstanceUpdater* newUpdater = new ECInstanceUpdater(db, ecClass);
        m_cache[clsid] = newUpdater;
        return *newUpdater;
        }

    static ECInstanceUpdater& GetECInstanceUpdater(DgnDbR db, ECClassCR ecClass)
        {
        return Get(db).GetECInstanceUpdater0(db, ecClass);
        }
};

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus InstanceBackedItem::_UpdateProperties(DgnElementCR el)
    {
    SetInstanceId(ECInstanceId(el.GetElementId().GetValue()));
    ECInstanceUpdater& updater = CachedECInstanceUpdaters::GetECInstanceUpdater(el.GetDgnDb(), m_instance->GetClass());
    return (BSISUCCESS != updater.Update(*m_instance)) ? DgnDbStatus::WriteError : DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlClassParams::Add(Utf8CP name, StatementType type)
    {
    BeAssert(!Utf8String::IsNullOrEmpty(name));
    if (!Utf8String::IsNullOrEmpty(name))
        {
        BeAssert(m_entries.end() == std::find_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return 0 == ::strcmp(name, arg.m_name); }));
        Entry entry(name, type);
        if (StatementType::Select == (type & StatementType::Select) && 0 < m_entries.size())
            {
            // We want to be able to quickly look up the index for a name for SELECT query results...so group them together at the front of the list.
            m_entries.insert(m_entries.begin(), entry);
            }
        else
            {
            m_entries.push_back(entry);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
int ECSqlClassParams::GetSelectIndex(Utf8CP name) const
    {
    // NB: All parameters valid for SELECT statements are grouped at the beginning of the list.
    BeAssert(!Utf8String::IsNullOrEmpty(name));
    if (!Utf8String::IsNullOrEmpty(name))
        {
        auto found = std::find_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return arg.m_name == name; });
        if (m_entries.end() == found)
            {
            // Ideally callers always pass the same static string we originally stored...fallback to string comparison...
            found = std::find_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return 0 == ::strcmp(arg.m_name, name); });
            BeAssert(m_entries.end() == found && "Prefer to pass the same string with static storage duration to GetSelectIndex() as was previously passed to Add()");
            }

        BeAssert(m_entries.end() != found);
        if (m_entries.end() != found)
            {
            BeAssert(StatementType::Select == (found->m_type & StatementType::Select));
            if (StatementType::Select == (found->m_type & StatementType::Select))
                return static_cast<int>(found - m_entries.begin());
            }
        }

    return -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlClassParams::RemoveAllButSelect()
    {
    // Once we've constructed the handler info, we need only retain those property names which are used in SELECT statements.
    auto removeAt = std::remove_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return StatementType::Select != (arg.m_type & StatementType::Select); });
    m_entries.erase(removeAt, m_entries.end());
    }

