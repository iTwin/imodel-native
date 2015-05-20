/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnElement.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/QvElemSet.h>

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
    m_geom = *stream;     // assign the new element (overwrites or reallocates)
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement::_DeleteInDb() const
    {
    CachedStatementPtr stmt;
    GetDgnDb().Elements().GetStatement(stmt, "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?");
    stmt->BindId(1, m_elementId);

    switch (stmt->Step())
        {
        case BE_SQLITE_CONSTRAINT_FOREIGNKEY:
            return  DGNMODEL_STATUS_ForeignKeyConstraint;

        case BE_SQLITE_DONE:
            return DGNMODEL_STATUS_Success;
        }

    return DGNMODEL_STATUS_ElementWriteError;
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
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::~DgnElement()
    {
    BeAssert(!IsPersistent());
    ClearAllAppData();
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
Utf8String DgnElement::_GenerateDefaultCode()
    {
    if (!m_elementId.IsValid())
        return "";

    Utf8String className(GetElementClass()->GetName());
    return Utf8PrintfString("%s%lld", className.c_str(), m_elementId.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement::_InsertInDb()
    {
    if (m_code.empty())
        m_code = _GenerateDefaultCode();

    CachedStatementPtr stmt;
    enum Column : int       {ElementId=1,ECClassId=2,ModelId=3,CategoryId=4,Label=5,Code=6,ParentId=7};
    GetDgnDb().Elements().GetStatement(stmt, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Element) " (Id,ECClassId,ModelId,CategoryId,Label,Code,ParentId) VALUES(?,?,?,?,?,?,?)");

    stmt->BindId(Column::ElementId, m_elementId);
    stmt->BindId(Column::ECClassId, m_classId);
    stmt->BindId(Column::ModelId, m_dgnModel.GetModelId());
    stmt->BindId(Column::CategoryId, m_categoryId);
    stmt->BindText(Column::Label, GetLabel(), Statement::MakeCopy::No);
    stmt->BindText(Column::Code, GetCode(), Statement::MakeCopy::No);
    stmt->BindId(Column::ParentId, m_parentId);

    return stmt->Step() != BE_SQLITE_DONE ? DGNMODEL_STATUS_ElementWriteError : DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement::_UpdateInDb()
    {
    CachedStatementPtr stmt;
    enum Column : int       {CategoryId=1,Label=2,Code=3,ParentId=4,ElementId=5};
    GetDgnDb().Elements().GetStatement(stmt, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Element) " SET CategoryId=?,Label=?,Code=?,ParentId=? WHERE Id=?");

    // note: ECClassId and ModelId cannot be modified.
    stmt->BindId(Column::CategoryId, m_categoryId);
    stmt->BindText(Column::Label, GetLabel(), Statement::MakeCopy::No);
    stmt->BindText(Column::Code, GetCode(), Statement::MakeCopy::No);
    stmt->BindId(Column::ParentId, m_parentId);
    stmt->BindId(Column::ElementId, m_elementId);

    return (stmt->Step() != BE_SQLITE_DONE) ? DGNMODEL_STATUS_ElementWriteError : DGNMODEL_STATUS_Success;
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
    GeomBlobHeader(BeSQLite::SnappyReader& in) {uint32_t actuallyRead; in._Read((Byte*) this, sizeof(*this), actuallyRead);}
};

static Utf8CP GEOM_Column = "Geom";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus GeometricElement::_LoadFromDb()
    {
    DgnModelStatus stat = T_Super::_LoadFromDb();
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    auto& pool = GetDgnDb().Elements();
    SnappyFromBlob& snappy = pool.GetSnappyFrom();

    if (ZIP_SUCCESS != snappy.Init(pool.GetDgnDb(), DGN_TABLE(DGN_CLASSNAME_ElementGeom), GEOM_Column, m_elementId.GetValue()))
        return DGNMODEL_STATUS_Success; // this element has no geometry

    GeomBlobHeader header(snappy);
    if ((GeomBlobHeader::Signature != header.m_signature) || 0 == header.m_size)
        {
        BeAssert(false);
        return DGNMODEL_STATUS_ElementReadError;
        }

    m_geom.ReserveMemory(header.m_size);

    uint32_t actuallyRead;
    snappy.ReadAndFinish(m_geom.GetDataR(), m_geom.GetSize(), actuallyRead);

    if (actuallyRead != m_geom.GetSize())
        {
        BeAssert(false);
        return DGNMODEL_STATUS_ElementReadError;
        }

    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus GeometricElement::_InsertInDb()
    {
    DgnModelStatus stat = T_Super::_InsertInDb();
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    DgnDbR dgnDb = GetDgnDb();
    CachedStatementPtr stmt;
    dgnDb.Elements().GetStatement(stmt, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_ElementGeom) "(Geom,Placement,ElementId) VALUES(?,?,?)");
    stmt->BindId(3, m_elementId);

    stat = _BindPlacement(*stmt);
    if (DGNMODEL_STATUS_NoGeometry == stat)
        return DGNMODEL_STATUS_Success;

    if (DGNMODEL_STATUS_Success == stat)
        stat = WriteGeomStream(*stmt, dgnDb);

    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    // Insert element uses geom part relationships for geom parts referenced in geom stream...
    ECIdSet<DgnGeomPartId> parts;
    ElementGeomIO::Collection(m_geom.GetData(), m_geom.GetSize()).GetGeomPartIds(parts, dgnDb);

    for (DgnGeomPartId partId : parts)
        {
        if (BentleyStatus::SUCCESS != dgnDb.GeomParts().InsertElementGeomUsesParts(m_elementId, partId))
            stat = DGNMODEL_STATUS_ElementWriteError;
        }

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus GeometricElement::_UpdateInDb()
    {
    DgnModelStatus stat = T_Super::_UpdateInDb();
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    DgnDbR dgnDb = GetDgnDb();
    CachedStatementPtr stmt;
    dgnDb.Elements().GetStatement(stmt, "UPDATE " DGN_TABLE(DGN_CLASSNAME_ElementGeom) " SET Geom=?,Placement=? WHERE ElementId=?");
    stmt->BindId(3, m_elementId);

    stat = _BindPlacement(*stmt);
    if (DGNMODEL_STATUS_NoGeometry == stat)
        return DGNMODEL_STATUS_Success;

    if (DGNMODEL_STATUS_Success == stat)
        stat = WriteGeomStream(*stmt, dgnDb);

    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    // Update element uses geom part relationships for geom parts referenced in geom stream...
    dgnDb.Elements().GetStatement(stmt, "SELECT GeomPartId FROM " DGN_TABLE(DGN_RELNAME_ElementGeomUsesParts) " WHERE ElementId=?");
    stmt->BindId(1, m_elementId);

    ECIdSet<DgnGeomPartId> partsOld;
    while (BE_SQLITE_ROW == stmt->Step())
        partsOld.insert(stmt->GetValueId<DgnGeomPartId>(0));

    ECIdSet<DgnGeomPartId> partsNew;
    ElementGeomIO::Collection(m_geom.GetData(), m_geom.GetSize()).GetGeomPartIds(partsNew, dgnDb);

    if (partsOld.empty() && partsNew.empty())
        return stat;

    bset<DgnGeomPartId> partsToRemove;
    std::set_difference(partsOld.begin(), partsOld.end(), partsNew.begin(), partsNew.end(), std::inserter(partsToRemove, partsToRemove.end()));

    if (!partsToRemove.empty())
        {
        dgnDb.Elements().GetStatement(stmt, "DELETE FROM " DGN_TABLE(DGN_RELNAME_ElementGeomUsesParts) " WHERE ElementId=? AND GeomPartId=?");
        stmt->BindId(1, m_elementId);

        for (DgnGeomPartId partId : partsToRemove)
            {
            stmt->BindId(2, partId);
            if (BE_SQLITE_DONE != stmt->Step())
                stat = DGNMODEL_STATUS_BadRequest;
            }
        }

    bset<DgnGeomPartId> partsToAdd;
    std::set_difference(partsNew.begin(), partsNew.end(), partsOld.begin(), partsOld.end(), std::inserter(partsToAdd, partsToAdd.end()));

    for (DgnGeomPartId partId : partsToAdd)
        {
        if (BentleyStatus::SUCCESS != dgnDb.GeomParts().InsertElementGeomUsesParts(m_elementId, partId))
            stat = DGNMODEL_STATUS_ElementWriteError;
        }

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus GeometricElement::WriteGeomStream(Statement& stmt, DgnDbR dgnDb)
    {
    SnappyToBlob& snappy = dgnDb.Elements().GetSnappyTo();

    snappy.Init();
    if (0 < m_geom.GetSize())
        {
        GeomBlobHeader header(m_geom);
        snappy.Write((ByteCP) &header, sizeof(header));
        snappy.Write(m_geom.GetData(), m_geom.GetSize());
        }

    uint32_t zipSize = snappy.GetCompressedSize();
    if (0 < zipSize)
        {
        if (1 == snappy.GetCurrChunk())
            stmt.BindBlob(1, snappy.GetChunkData(0), zipSize, Statement::MakeCopy::No);
        else
            stmt.BindZeroBlob(1, zipSize); // more than one chunk in geom stream
        }

    if (BE_SQLITE_DONE != stmt.Step())
        return DGNMODEL_STATUS_ElementWriteError;

    if (1 == snappy.GetCurrChunk())
        return DGNMODEL_STATUS_Success;

    StatusInt status = snappy.SaveToRow(dgnDb, DGN_TABLE(DGN_CLASSNAME_ElementGeom), GEOM_Column, m_elementId.GetValue());
    return(SUCCESS != status) ? DGNMODEL_STATUS_ElementWriteError : DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement3d::_BindPlacement(Statement& stmt)
    {
    if (!m_placement.IsValid())
        return DGNMODEL_STATUS_NoGeometry;

    stmt.BindBlob(2, &m_placement, sizeof(m_placement), Statement::MakeCopy::No);
    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement3d::_LoadFromDb()
    {
    DgnModelStatus stat = T_Super::_LoadFromDb();
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    CachedStatementPtr stmt;
    GetDgnDb().Elements().GetStatement(stmt, "SELECT Placement FROM " DGN_TABLE(DGN_CLASSNAME_ElementGeom) " Where ElementId=?");
    stmt->BindId(1, m_elementId);

    if (BE_SQLITE_ROW != stmt->Step())
        return DGNMODEL_STATUS_Success; // it is legal to have an element with no geometry.

    if (stmt->GetColumnBytes(0) != sizeof(m_placement))
        {
        BeAssert(false);
        return DGNMODEL_STATUS_ElementReadError;
        }

    memcpy(&m_placement, stmt->GetValueBlob(0), sizeof(m_placement));
    return DGNMODEL_STATUS_Success;
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

    return new PhysicalElement(CreateParams(model, PhysicalElement::QueryClassId(model.GetDgnDb()), categoryId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId PhysicalElement::QueryClassId(DgnDbR db)
    {
    return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus PhysicalElement::_InsertInDb()
    {
    DgnModelStatus stat = T_Super::_InsertInDb();
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    if (!m_placement.IsValid()) // this is an invisible element
        return DGNMODEL_STATUS_Success;

    CachedStatementPtr stmt;
    GetDgnDb().Elements().GetStatement(stmt, "INSERT INTO " DGN_VTABLE_PrjRTree " (ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ) VALUES (?,?,?,?,?,?,?)");

    stmt->BindId(1, m_elementId);
    AxisAlignedBox3dCR range = m_placement.CalculateRange();
    stmt->BindDouble(2, range.low.x);
    stmt->BindDouble(3, range.high.x);
    stmt->BindDouble(4, range.low.y);
    stmt->BindDouble(5, range.high.y);
    stmt->BindDouble(6, range.low.z);
    stmt->BindDouble(7, range.high.z);

    return stmt->Step() == BE_SQLITE_DONE ? DGNMODEL_STATUS_Success : DGNMODEL_STATUS_ElementWriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus PhysicalElement::_UpdateInDb()
    {
    DgnModelStatus stat = T_Super::_UpdateInDb();
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    if (!m_placement.IsValid()) // this is an invisible element
        return DGNMODEL_STATUS_Success;

    CachedStatementPtr stmt;
    GetDgnDb().Elements().GetStatement(stmt, "UPDATE " DGN_VTABLE_PrjRTree " SET MinX=?,MaxX=?,MinY=?,MaxY=?,MinZ=?,MaxZ=? WHERE ElementId=?");

    AxisAlignedBox3dCR range = m_placement.CalculateRange();
    stmt->BindDouble(1, range.low.x);
    stmt->BindDouble(2, range.high.x);
    stmt->BindDouble(3, range.low.y);
    stmt->BindDouble(4, range.high.y);
    stmt->BindDouble(5, range.low.z);
    stmt->BindDouble(6, range.high.z);
    stmt->BindId(7, m_elementId);

    return stmt->Step() == BE_SQLITE_DONE ? DGNMODEL_STATUS_Success : DGNMODEL_STATUS_ElementWriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement2d::_BindPlacement(Statement& stmt)
    {
    if (!m_placement.IsValid())
        return DGNMODEL_STATUS_NoGeometry;

    stmt.BindBlob(2, &m_placement, sizeof(m_placement), Statement::MakeCopy::No);
    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement2d::_LoadFromDb()
    {
    DgnModelStatus stat = T_Super::_LoadFromDb();
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    CachedStatementPtr stmt;
    GetDgnDb().Elements().GetStatement(stmt, "SELECT Placement FROM " DGN_TABLE(DGN_CLASSNAME_ElementGeom) " Where ElementId=?");
    stmt->BindId(1, m_elementId);

    if (BE_SQLITE_ROW != stmt->Step())
        return DGNMODEL_STATUS_Success; // it is legal to have an element with no geometry.

    if (stmt->GetColumnBytes(0) != sizeof(m_placement))
        {
        BeAssert(false);
        return DGNMODEL_STATUS_ElementReadError;
        }

    memcpy(&m_placement, stmt->GetValueBlob(0), sizeof(m_placement));
    return DGNMODEL_STATUS_Success;
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
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement::_CopyFrom(DgnElementCR other)
    {
    if (&other == this)
        return DGNMODEL_STATUS_Success;

    if (&m_dgnModel != &other.m_dgnModel)
        return DGNMODEL_STATUS_BadElement;

    m_categoryId = other.m_categoryId;
    m_code       = other.m_code;
    m_label      = other.m_label;
    m_parentId   = other.m_parentId;

    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus GeometricElement::_CopyFrom(DgnElementCR other)
    {
    auto stat = T_Super::_CopyFrom(other);
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    GeometricElementCP otherGeom = other.ToGeometricElement();
    if (otherGeom)
        SaveGeomStream(&otherGeom->m_geom);
    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement3d::_CopyFrom(DgnElementCR other)
    {
    auto stat = T_Super::_CopyFrom(other);
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    DgnElement3dCP el3d = other.ToElement3d();
    if (el3d)
        m_placement = el3d->m_placement;

    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement2d::_CopyFrom(DgnElementCR other)
    {
    auto stat = T_Super::_CopyFrom(other);
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    DgnElement2dCP el2d = other.ToElement2d();
    if (el2d)
        m_placement = el2d->m_placement;
    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandlerR DgnElement::GetElementHandler() const
    {
    return *ElementHandler::FindHandler(GetDgnDb(), m_classId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnElement::CopyForEdit() const
    {
    DgnElementPtr newEl = GetElementHandler()._CreateInstance(DgnElement::CreateParams(m_dgnModel, m_classId, m_categoryId, GetLabel(), GetCode(), m_elementId, m_parentId));
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

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  07/2014
//=======================================================================================
struct ElementLoadedEventCaller
{
    DgnElementR m_elRef;
    ElementLoadedEventCaller(DgnElementR elRef) : m_elRef(elRef) {}
    void CallHandler(DgnElements::Listener& handler) const {handler._OnElementLoaded(m_elRef);}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::SendOnLoadedEvent(DgnElementR elRef) const
    {
    if (nullptr != m_listeners)
        m_listeners->CallAllHandlers(ElementLoadedEventCaller(elRef));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings   07/2014
//---------------------------------------------------------------------------------------
void DgnElements::AddListener(Listener* listener)
    {
    if (nullptr == m_listeners)
        m_listeners = new EventHandlerList<Listener>;

    m_listeners->AddHandler(listener);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings   07/2014
//---------------------------------------------------------------------------------------
void DgnElements::DropListener(Listener* listener)
    {
    if (nullptr != m_listeners)
        m_listeners->DropHandler(listener);
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
    range.low.z = range.high.z = 0.0;

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElement::Update(DgnModelStatus* stat) {return GetDgnDb().Elements().Update(*this, stat);}
DgnElementCPtr DgnElement::Insert(DgnModelStatus* stat) {return GetDgnDb().Elements().Insert(*this, stat);}
DgnModelStatus DgnElement::Delete() const {return GetDgnDb().Elements().Delete(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus ElementGroup::InsertMember(DgnElementCR member) const
    {
    if (!GetElementId().IsValid() || !member.GetElementId().IsValid())
        return DGNMODEL_STATUS_InvalidId;

    DgnModelStatus status = _OnMemberInsert(member); // give subclass a chance to reject member insert
    if (DGNMODEL_STATUS_Success != status)
        return status;

    CachedECSqlStatementPtr statement = GetDgnDb().GetPreparedECSqlStatement
        ("INSERT INTO " DGN_SCHEMA(DGN_RELNAME_ElementGroupHasMembers) 
        " (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES (?,?,?,?)");

    if (!statement.IsValid())
        return DGNMODEL_STATUS_BadRequest;

    statement->BindId(1, GetElementClassId());
    statement->BindId(2, GetElementId());
    statement->BindId(3, member.GetElementClassId());
    statement->BindId(4, member.GetElementId());

    if (ECSqlStepStatus::Done != statement->Step())
        return DGNMODEL_STATUS_BadRequest;
    
    _OnMemberInserted(member); // notify subclass that member was inserted
    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus ElementGroup::DeleteMember(DgnElementCR member) const
    {
    if (!GetElementId().IsValid() || !member.GetElementId().IsValid())
        return DGNMODEL_STATUS_InvalidId;

    DgnModelStatus status = _OnMemberDelete(member); // give subclass a chance to reject member delete
    if (DGNMODEL_STATUS_Success != status)
        return status;

    CachedStatementPtr statement;
    GetDgnDb().Elements().GetStatement(statement, "DELETE FROM " DGN_TABLE(DGN_RELNAME_ElementGroupHasMembers) " WHERE GroupId=? AND MemberId=?");

    if (!statement.IsValid())
        return DGNMODEL_STATUS_BadRequest;

    statement->BindId(1, GetElementId());
    statement->BindId(2, member.GetElementId());

    if (BE_SQLITE_DONE != statement->Step())
        return DGNMODEL_STATUS_BadRequest;
    
    _OnMemberDeleted(member); // notify subclass that member was deleted
    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet ElementGroup::QueryMembers() const
    {
    CachedStatementPtr statement;
    GetDgnDb().Elements().GetStatement(statement, "SELECT MemberId FROM " DGN_TABLE(DGN_RELNAME_ElementGroupHasMembers) " WHERE GroupId=?");

    if (!statement.IsValid())
        return DgnElementIdSet();

    statement->BindId(1, GetElementId());

    DgnElementIdSet elementIdSet;
    while (BE_SQLITE_ROW == statement->Step())
        elementIdSet.insert(statement->GetValueId<DgnElementId>(0));

    return elementIdSet;
    }
