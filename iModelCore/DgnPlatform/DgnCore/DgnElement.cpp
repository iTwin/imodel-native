/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnElement.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/QvElemSet.h>

BEGIN_UNNAMED_NAMESPACE

enum class InstanceChangeType {NoChange=0, Write, Delete};
enum class InstanceUpdateOutcome{Nop, Inserted, Updated, Deleted};

//=======================================================================================
//! @bsiclass                                                     Sam.Wilson      04/2015
//=======================================================================================
struct CachedInstance
    {
    InstanceChangeType  m_changeType;
    IECInstancePtr m_instance;

    CachedInstance() : m_changeType(InstanceChangeType::NoChange) {}
    CachedInstance(IECInstancePtr instance, InstanceChangeType ct = InstanceChangeType::NoChange) : m_instance(instance), m_changeType(ct) {}
    void Clear() {m_changeType = InstanceChangeType::NoChange; m_instance = nullptr;}

    BentleyStatus ApplyScheduledDelete(DgnDbR);
    BentleyStatus ApplyScheduledReplace(DgnDbR);
    BentleyStatus ApplyScheduledInsert(DgnDbR);
    BentleyStatus ApplyScheduledChange(InstanceUpdateOutcome&, DgnDbR);
    BentleyStatus ApplyScheduledChangeToElementInstance(DgnElementR);
    BentleyStatus ApplyScheduledChangesToItem(DgnElementR);
    BentleyStatus ApplyScheduledChangesToAspect(DgnElementR);
    };

//=======================================================================================
//! @bsiclass                                                     Sam.Wilson      04/2015
//=======================================================================================
struct CachedInstances : DgnElementAppData
    {
    static Key s_key;

    bool m_itemChecked;
    CachedInstance m_element;
    CachedInstance m_item;
    bmap<DgnClassId, bvector<CachedInstance>> m_otherAspects;

    virtual void _OnCleanup(DgnElementCP host) override;

    CachedInstances() : m_itemChecked(false) {}
    void Clear() {m_element.Clear(); m_item.Clear(); m_otherAspects.clear(); m_itemChecked=false;}

    CachedInstance* FindAspectInstanceAccessor(ECClassId classId, WStringCR instanceId);

    static CachedInstances* Find(DgnElementCR);
    static CachedInstances& Get(DgnElementCR);
    };

END_UNNAMED_NAMESPACE

CachedInstances::Key CachedInstances::s_key;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CachedInstances* CachedInstances::Find(DgnElementCR el)
    {
    return static_cast<CachedInstances*>(el.FindAppData(s_key));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CachedInstances& CachedInstances::Get(DgnElementCR el)
    {
    CachedInstances* d = Find(el);
    if (nullptr == d)
        {
        // *** WIP_ELEMENT_INSTANCES - use HeapZone to avoid malloc
        d = new CachedInstances;
        el.AddAppData(s_key, d);
        }
    return *d;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CachedInstances::_OnCleanup(DgnElementCP host)
    {
    delete this; // *** WIP_ELEMENT_INSTANCES - use HeapZone to avoid malloc
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    02/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DgnElements::QueryModelId(DgnElementId elementId)
    {
    CachedStatementPtr stmt;
    GetStatement(stmt, "SELECT ModelId FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?");
    stmt->BindId(1, elementId);
    return (BE_SQLITE_ROW != stmt->Step()) ? DgnModelId() : stmt->GetValueId<DgnModelId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DgnElement::AddRef() const
    {
    if (0 == m_refCount && IsPersistent())
        GetDgnDb().Elements().OnReclaimed(*this);

    return ++m_refCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DgnElement::Release() const
    {
    if (1 < m_refCount--)
        return m_refCount.load();

    BeAssert(m_refCount==0);

    if (IsPersistent())
        {
        // add to the DgnFile's unreferenced element count
        GetDgnDb().Elements().OnUnreferenced(*this);
        return 0;
        }

    // is not in the pool, just delete it
    delete this;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppDataEntry* DgnElement::FreeAppDataEntry(AppDataEntry* prev, AppDataEntry& thisEntry) const
    {
    AppDataEntry* next = thisEntry.m_next;

    if (prev)
        prev->m_next = next;
    else
        m_appData = next;

    thisEntry.ClearEntry(this);
    GetHeapZone().Free(&thisEntry, sizeof(AppDataEntry));

    return  next;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementAppData* DgnElement::FindAppData(DgnElementAppData::Key const& key) const
    {
    for (AppDataEntry* thisEntry = m_appData; thisEntry; thisEntry = thisEntry->m_next)
        {
        if (thisEntry->m_key < &key) // entries are sorted by key
            continue;

        return(thisEntry->m_key == &key) ? thisEntry->m_obj : NULL;
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnElement::AddAppData(DgnElementAppData::Key const& key, DgnElementAppData* obj) const
    {
    AppDataEntry* prevEntry = NULL;
    AppDataEntry* nextEntry = m_appData;
    for (; nextEntry; prevEntry=nextEntry, nextEntry=nextEntry->m_next)
        {
        if (nextEntry->m_key < &key) // sort them by key
            continue;

        if (nextEntry->m_key != &key)
            break;

        nextEntry->SetEntry(obj, this);      // already exists, just change it
        return SUCCESS;
        }

    AppDataEntry* newEntry = (AppDataEntry*) GetHeapZone().Alloc(sizeof(AppDataEntry));
    newEntry->Init(key, obj, nextEntry);

    if (prevEntry)
        prevEntry->m_next = newEntry;
    else
        m_appData = newEntry;

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnElement::DropAppData(DgnElementAppData::Key const& key) const
    {
    for (AppDataEntry* prev=NULL, *thisEntry=m_appData; thisEntry; prev=thisEntry, thisEntry=thisEntry->m_next)
        {
        if (thisEntry->m_key < &key) // entries are sorted by key
            continue;

        if (thisEntry->m_key != &key)
            break;

        FreeAppDataEntry(prev, *thisEntry);
        return  SUCCESS;
        }

    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::ClearAllAppData()
    {
    for (AppDataEntry* thisEntry=m_appData; thisEntry; )
        thisEntry = FreeAppDataEntry(NULL, *thisEntry);
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
void DgnElement::ForceElemChanged(bool qvCacheCleared, DgnElementChangeReason reason)
    {
    for (AppDataEntry* prev=NULL, *next, *thisEntry=m_appData; thisEntry; thisEntry=next)
        {
        next = thisEntry->m_next;
        if (thisEntry->m_obj->_OnElemChanged(this, qvCacheCleared, reason))
            FreeAppDataEntry(prev, *thisEntry);
        else
            prev = thisEntry;
        }
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
    static DgnElementAppData::Key s_qvElemsKey;
    T_QvElemSet* qvElems = (T_QvElemSet*) FindAppData(s_qvElemsKey);
    if (qvElems)
        return  qvElems;

    if (!createIfNotPresent)
        return  NULL;

    HeapZone& zone = GetHeapZone();
    qvElems = new((T_QvElemSet*) zone.Alloc(sizeof(T_QvElemSet))) T_QvElemSet(zone);

    AddAppData(s_qvElemsKey, qvElems);
    return  qvElems;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::SetInSelectionSet(bool yesNo) const
    {
    m_flags.m_inSelectionSet = yesNo;
    SetHilited(yesNo ? HILITED_Normal : HILITED_None);
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
    enum Column : int       {ElementId=1,ECClassId=2,ModelId=3,CategoryId=4,Code=5,ParentId=6};
    GetDgnDb().Elements().GetStatement(stmt, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Element) " (Id,ECClassId,ModelId,CategoryId,Code,ParentId) VALUES(?,?,?,?,?,?)");

    stmt->BindId(Column::ElementId, m_elementId);
    stmt->BindId(Column::ECClassId, m_classId);
    stmt->BindId(Column::ModelId, m_dgnModel.GetModelId());
    stmt->BindId(Column::CategoryId, m_categoryId);
    stmt->BindText(Column::Code, m_code.c_str(), Statement::MakeCopy::No);
    stmt->BindId(Column::ParentId, m_parentId);

    return stmt->Step() != BE_SQLITE_DONE ? DGNMODEL_STATUS_ElementWriteError : DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement::_UpdateInDb()
    {
    CachedStatementPtr stmt;
    enum Column : int       {CategoryId=1,Code=2,ParentId=3,ElementId=4};
    GetDgnDb().Elements().GetStatement(stmt, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Element) " SET CategoryId=?,Code=?,ParentId=? WHERE Id=?");

    // note: ECClassId and ModelId cannot be modified.
    stmt->BindId(Column::CategoryId, m_categoryId);
    stmt->BindText(Column::Code, m_code.c_str(), Statement::MakeCopy::No);
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

    CachedStatementPtr stmt;
    GetDgnDb().Elements().GetStatement(stmt, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_ElementGeom) "(Geom,Placement,ElementId) VALUES(?,?,?)");
    stmt->BindId(3, m_elementId);

    stat = _BindPlacement(*stmt);
    if (DGNMODEL_STATUS_NoGeometry == stat)
        return DGNMODEL_STATUS_Success;

    return (DGNMODEL_STATUS_Success != stat) ? stat : DoInsertOrUpdate(*stmt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus GeometricElement::_UpdateInDb()
    {
    DgnModelStatus stat = T_Super::_UpdateInDb();
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    CachedStatementPtr stmt;
    GetDgnDb().Elements().GetStatement(stmt, "UPDATE " DGN_TABLE(DGN_CLASSNAME_ElementGeom) " SET Geom=?,Placement=? WHERE ElementId=?");
    stmt->BindId(3, m_elementId);

    stat = _BindPlacement(*stmt);
    if (DGNMODEL_STATUS_NoGeometry == stat)
        return DGNMODEL_STATUS_Success;

    return (DGNMODEL_STATUS_Success != stat) ? stat : DoInsertOrUpdate(*stmt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus GeometricElement::DoInsertOrUpdate(Statement& stmt)
    {
    SnappyToBlob& snappy = GetDgnDb().Elements().GetSnappyTo();

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

    StatusInt status = snappy.SaveToRow(GetDgnDb(), DGN_TABLE(DGN_CLASSNAME_ElementGeom), GEOM_Column, m_elementId.GetValue());
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
    return qvElems ? qvElems->Find(id) : NULL;
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
    DgnElementPtr newEl = GetElementHandler()._CreateInstance(DgnElement::CreateParams(m_dgnModel, m_classId, m_categoryId, m_code.c_str(), m_elementId, m_parentId));
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      04/2015
//---------------------------------------------------------------------------------------
static BentleyStatus deleteInstance(DgnDbR db, IECInstanceCR instance)
    {
    BeAssert(!instance.GetInstanceId().empty());
    ECInstanceDeleter deleter(db, instance.GetClass());
    return deleter.Delete(instance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      04/2015
//---------------------------------------------------------------------------------------
BentleyStatus CachedInstance::ApplyScheduledDelete(DgnDbR db)
    {
    if (m_instance->GetInstanceId().empty())
        {
        BeAssert(false);
        return BSIERROR;
        }

    return deleteInstance(db, *m_instance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      04/2015
//---------------------------------------------------------------------------------------
BentleyStatus CachedInstance::ApplyScheduledReplace(DgnDbR db)
    {
    if (m_instance->GetInstanceId().empty())
        {
        //WIP_ASPECT_API: Remove the assertion as we don't have control over the ECInstances passed by apps. They can indeed
        //have an empty instance id if they were created from scratch as standalone instances
        BeAssert(false);
        return BSIERROR;
        }
    
    ECInstanceUpdater updater(db, *m_instance);
    if (!updater.IsValid())
        return BSIERROR;

    return updater.Update(*m_instance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      04/2015
//---------------------------------------------------------------------------------------
BentleyStatus CachedInstance::ApplyScheduledInsert(DgnDbR db)
    {
    ECInstanceInserter inserter(db, m_instance->GetClass());
    if (!inserter.IsValid())
        return BSIERROR;

    //if valid instance id is set on the ECInstance use it (-> instance is an item). Otherwise let ECDb generate a new one
    //WIP_ASPECT_API: ECInstanceID string conversion is called twice in the workflow. Once here and once in the Insert method.
    //Maybe a better way is to be explicit and add a flag on CachedInstance when the id on the instance is expected to be a
    //valid one (item/elementgeom...) and when not (other aspects).
    ECInstanceId extractedECInstanceId;
    const bool extractedIdIsValid = ECInstanceIdHelper::FromString(extractedECInstanceId, m_instance->GetInstanceId().c_str());
    const bool autogenerateECInstanceId = !extractedIdIsValid;
    //generated id is set as instance id in m_instance
    return inserter.Insert(*m_instance, autogenerateECInstanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      04/2015
//---------------------------------------------------------------------------------------
BentleyStatus CachedInstance::ApplyScheduledChange(InstanceUpdateOutcome& outcome, DgnDbR db)
    {
    if (m_changeType == InstanceChangeType::NoChange)
        {
        outcome = InstanceUpdateOutcome::Nop;
        return BSISUCCESS;
        }

    if (!m_instance.IsValid())
        {
        BeAssert(false);
        outcome = InstanceUpdateOutcome::Nop;
        return BSIERROR;
        }

    if (m_changeType == InstanceChangeType::Delete)
        {
        outcome = InstanceUpdateOutcome::Deleted;
        return ApplyScheduledDelete(db);
        }

    // write the item.
    if (ApplyScheduledReplace(db) == BSISUCCESS)
        {
        outcome = InstanceUpdateOutcome::Updated;
        return BSISUCCESS;
        }

    outcome = InstanceUpdateOutcome::Inserted;
    return ApplyScheduledInsert(db);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
static void appendPropertiesToECSqlSelectClause(Utf8StringR ecsql, ECClassCR ecclass, bset<Utf8String> const& ignoreList, Utf8CP classAlias = nullptr)
    {
    Utf8String classAliasStr;
    if (classAlias != nullptr)
        classAliasStr.append(classAlias).append(".");

    //ECClassId needed in case of polymorphic selects
    ecsql.append(classAliasStr).append("ECInstanceId, ").append(classAliasStr).append(".GetECClassId() AS ECClassId");

    for (auto ecprop : ecclass.GetProperties())
        {
        Utf8String propName(ecprop->GetName());
        if (ignoreList.find(propName) != ignoreList.end())
            continue;

        ecsql.append(", ").append(classAliasStr).append(propName.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
static void setInstanceId(IECInstanceR instance, EC::ECInstanceId id)
    {
    wchar_t buf[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH];
    ECInstanceIdHelper::ToString(buf, _countof(buf), id);
    instance.SetInstanceId(buf);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
static void setInstanceId(IECInstanceR instance, DgnElementId elementId)
    {
    setInstanceId(instance, EC::ECInstanceId(elementId.GetValue()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
static IECInstancePtr queryInstance(DgnDbR db, ECClassCP ecclass, EC::ECInstanceId instanceId, bset<Utf8String> const& ignoreList = bset<Utf8String>())
    {
    if (nullptr == ecclass)
        return nullptr;

    Utf8String ecsql("SELECT ");
    appendPropertiesToECSqlSelectClause(ecsql, *ecclass, ignoreList);
    ecsql.append(" FROM ONLY ").append(ECSqlBuilder::ToECSqlSnippet(*ecclass)).append(" WHERE ECInstanceId=?");

    ECSqlStatement stmt;
    stmt.Prepare(db, ecsql.c_str());
    stmt.BindId(1, instanceId);
    stmt.Step();

    ECInstanceECSqlSelectAdapter selector(stmt);
    return selector.GetInstance();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
static IECInstancePtr getDirectlyLinkedInstance(DgnDbR db, ECClassCP ecclass, DgnElementId elementId, bset<Utf8String> const& ignoreList)
    {
    return queryInstance(db, ecclass, EC::ECInstanceId(elementId.GetValue()), ignoreList);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
IECInstanceR DgnElement::GetSubclassProperties(bool setModified) const
    {
    CachedInstances& instances = CachedInstances::Get(*this);
    if (!instances.m_element.m_instance.IsValid())
        {
        if (!GetElementId().IsValid())
            {
            if (setModified)
                instances.m_element.m_instance = GetDgnDb().Schemas().GetECClass(GetElementClassId().GetValue())->GetDefaultStandaloneEnabler()->CreateInstance();
            }
        else
            {
            bset<Utf8String> ignoreList;
            ignoreList.insert("ModelId");
            ignoreList.insert("CategoryId");
            ignoreList.insert("Code");
            ignoreList.insert("LastMod");
            instances.m_element = CachedInstance(getDirectlyLinkedInstance(GetDgnDb(), GetElementClass(), GetElementId(), ignoreList));
            }
        }
    if (setModified)
        instances.m_element.m_changeType = InstanceChangeType::Write;
    return *instances.m_element.m_instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
IECInstanceCR DgnElement::GetSubclassProperties() const {return GetSubclassProperties(false);}
IECInstanceR DgnElement::GetSubclassPropertiesR() {return GetSubclassProperties(true);}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
IECInstanceP DgnElement::GetItem(bool setModified) const
    {
    if (!GetElementId().IsValid())
        return nullptr;

    CachedInstances& instances = CachedInstances::Get(*this);
    if (!instances.m_itemChecked)
        {
        instances.m_itemChecked = true;

#if defined (NEEDS_WORK_ELEMENTS_API)
        if (GetItemClassId().IsValid())
            {
            ECClassCP itemClass = GetDgnDb().Schemas().GetECClass(GetItemClassId().GetValue());
            instances.m_item = CachedInstance(getDirectlyLinkedInstance(GetDgnDb(), itemClass, GetElementId(), bset<Utf8String>()));
            }
#endif
        }

    if (instances.m_item.m_changeType == InstanceChangeType::Delete)
        return nullptr;

    if (setModified)
        instances.m_item.m_changeType = InstanceChangeType::Write;

    return instances.m_item.m_instance.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
IECInstanceCP DgnElement::GetItem() const {return GetItem(false);}
IECInstanceP  DgnElement::GetItemP()      {return GetItem(true);}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
void DgnElement::SetItem(IECInstanceR instance)
    {
    CachedInstances& instances = CachedInstances::Get(*this);
    instances.m_itemChecked = true;
    instances.m_item = CachedInstance(&instance, InstanceChangeType::Write);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
void DgnElement::RemoveItem()
    {
    if (nullptr == GetItem()) // make sure we have the existing item cached.
        return; // no item => nop

    CachedInstances& instances = CachedInstances::Get(*this);
    instances.m_item.m_changeType = InstanceChangeType::Delete;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
bvector<IECInstancePtr> DgnElement::GetAspects(ECClassCP aspectClass) const
    {
    bvector<IECInstancePtr> instances;

    //WIP_ASPECT_API: If Element hasn't been persisted yet, it is assumed that no persisted aspects should exist either.
    //WIP_ASPECT_API: Sam, is this assumption correct? And if yes, it is correct to return an empty vector?
    if (!IsPersistent() || nullptr == aspectClass)
        return instances;

    ECRelationshipClassCP elementOwnsAspectRelationship = (ECRelationshipClassCP)GetDgnDb().Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_RELNAME_ElementOwnsAspects);
    if (nullptr == elementOwnsAspectRelationship)
        {
        BeAssert(false && "missing or invalid dgn schema");
        return instances;
        }

    Utf8CP aspectAlias = "a";

    Utf8String ecsql("SELECT ");
    appendPropertiesToECSqlSelectClause(ecsql, *aspectClass, bset<Utf8String>(), aspectAlias);
    ecsql.append(" FROM ONLY ").append(ECSqlBuilder::ToECSqlSnippet(*aspectClass)).append(" ").append(aspectAlias);
    ecsql.append(" JOIN ONLY ").append(ECSqlBuilder::ToECSqlSnippet(*GetElementClass())).append(" e USING ").append(ECSqlBuilder::ToECSqlSnippet(*elementOwnsAspectRelationship));
    ecsql.append(" WHERE SourceECInstanceId=?");
    
    ECSqlStatement stmt;
    stmt.Prepare(GetDgnDb(), ecsql.c_str());
    stmt.BindId(1, GetElementId());
    while(stmt.Step() == ECSqlStepStatus::HasRow)
        {
        ECInstanceECSqlSelectAdapter selector(stmt);
        IECInstancePtr instance = selector.GetInstance();
        if (!instance.IsValid())
            {
            BeAssert(false);
            continue;
            }

        BeAssert(!instance->GetInstanceId().empty());
        instances.push_back(instance);
        }

    return instances;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
template<typename RTYPE, bool SETMODIFIED>
bvector<RTYPE> DgnElement::GetAspects(DgnClassId aspectClassId) const
    {
    CachedInstances& instances = CachedInstances::Get(*this);
    if (instances.m_otherAspects.find(aspectClassId) == instances.m_otherAspects.end())
        {
        // Always create an entry for the requested class. That tells us that we have checked. Its vector will be empty if we find no instances.
        auto& instancesOfClass = instances.m_otherAspects[aspectClassId];
        for (auto inst : GetAspects(GetDgnDb().Schemas().GetECClass(aspectClassId.GetValue())))
            instancesOfClass.push_back(CachedInstance(inst));
        }

    bvector<RTYPE> vec;
    for (auto& accs : instances.m_otherAspects[aspectClassId])
        {
        if (accs.m_changeType == InstanceChangeType::Delete)
            continue;

        if (SETMODIFIED)
            accs.m_changeType = InstanceChangeType::Write;

        vec.push_back(accs.m_instance.get());
        }

    return vec;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
bvector<IECInstanceCP> DgnElement::GetAspects(DgnClassId aspectClassId) const {return GetAspects<IECInstanceCP,false>(aspectClassId);}
bvector<IECInstanceP>  DgnElement::GetAspectsP(DgnClassId aspectClassId)      {return GetAspects<IECInstanceP, true>(aspectClassId);}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
void DgnElement::AddAspect(IECInstanceR aspectInstance)
    {
    DgnClassId aspectClassId(aspectInstance.GetClass().GetId());

    GetAspects(aspectClassId); // make sure we have read the aspects that are there.

    CachedInstances& instances = CachedInstances::Get(*this);
    CachedInstance* aspect = instances.FindAspectInstanceAccessor(aspectInstance.GetClass().GetId(), aspectInstance.GetInstanceId());
    if (nullptr != aspect)
        {
        aspect->m_instance = &aspectInstance;
        aspect->m_changeType = InstanceChangeType::Write;
        return;
        }

    // No existing aspect found => insert a new one.
    instances.m_otherAspects[aspectClassId].push_back(CachedInstance(&aspectInstance, InstanceChangeType::Write));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
void DgnElement::RemoveAspect(DgnClassId aspectClassId, ECInstanceId aspectId)
    {
    GetAspects(aspectClassId); // make sure we have read the aspects that are there.

    CachedInstances& instances = CachedInstances::Get(*this);

    wchar_t aspectIdStr[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH];
    ECInstanceIdHelper::ToString(aspectIdStr, _countof(aspectIdStr), aspectId);

    CachedInstance* aspect = instances.FindAspectInstanceAccessor(aspectClassId.GetValue(), aspectIdStr);
    if (nullptr != aspect)
        aspect->m_changeType = InstanceChangeType::Delete;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedInstance::ApplyScheduledChangeToElementInstance(DgnElementR el)
    {
    if (m_changeType == InstanceChangeType::NoChange)
        return BSISUCCESS;

    if (!m_instance.IsValid())
        {
        BeAssert(false && "Missing instance for change");
        return BSIERROR;
        }

    if (m_changeType == InstanceChangeType::Delete)
        {
        BeAssert(false && "You can't delete the element instance");
        return BSIERROR;
        }

    if (m_instance->GetClass().GetId() != el.GetElementClassId().GetValue())
        {
        BeAssert(false && "You can't change the class of the element instance");
        return BSIERROR;
        }

    setInstanceId(*m_instance, el.GetElementId());

    //  Apply the proposed change.
    return ApplyScheduledReplace(el.GetDgnDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                    03/2015
//---------------------------------------------------------------------------------------
static BentleyStatus insertRelationship(DgnDbR db, DgnElementKeyCR sourceElement, Utf8CP relName, IECInstanceR targetInstance)
    {
    if (!sourceElement.IsValid() || targetInstance.GetInstanceId().empty())
        return BentleyStatus::ERROR;


    CachedECSqlStatementPtr statementPtr = db.GetPreparedECSqlStatement(Utf8PrintfString(
        "INSERT INTO dgn.%s (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES (?,?,?,?)", relName));

    if (!statementPtr.IsValid())
        return BentleyStatus::ERROR;

    statementPtr->BindInt64(1, sourceElement.GetECClassId());
    statementPtr->BindId  (2, sourceElement.GetECInstanceId());
    statementPtr->BindInt64(3, targetInstance.GetClass().GetId());
    Utf8String targetInstanceIdStr(targetInstance.GetInstanceId()); //save string copy in ECSql if the Utf8String is a local var instead of a temp variable
    statementPtr->BindText(4, targetInstanceIdStr.c_str(), IECSqlBinder::MakeCopy::No);

    if (ECSqlStepStatus::Done != statementPtr->Step())
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedInstance::ApplyScheduledChangesToItem(DgnElementR el)
    {
    if (m_changeType == InstanceChangeType::NoChange)
        return BSISUCCESS;

    if (!m_instance.IsValid())
        {
        BeAssert(false && "Missing instance for change");
        return BSIERROR;
        }

    //  Ensure that the proposed item has the correct Id
    setInstanceId(*m_instance, el.GetElementId());

    if (m_changeType == InstanceChangeType::Write)
        {
#if defined (NEEDS_WORK_ELEMENTS_API)
        //  There is 1 item. If we want to change its class, we must delete the existing one.
        if (el.GetItemClassId().IsValid() && el.GetItemClassId().GetValue() != m_instance->GetClass().GetId())
            el.GetDgnDb().Items().DeleteItem(el.GetElementId());
#endif
        }

    //  Insert, update, or delete the item.
    InstanceUpdateOutcome outcome;
    if (BSISUCCESS != ApplyScheduledChange(outcome, el.GetDgnDb()))
        return BSIERROR;

#if defined (NEEDS_WORK_ELEMENTS_API)
    //  Update the DgnElement's ItemClassId member variable
    if (outcome == InstanceUpdateOutcome::Deleted)
        el.SetItemClassId(DgnClassId());
    else
        el.SetItemClassId(DgnClassId(m_instance->GetClass().GetId()));
#endif

    // Note: the ElementOwnsItem relationship is created implicitly when we create the item with its ElementId column set to match the owning element's ID.

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CachedInstance* CachedInstances::FindAspectInstanceAccessor(ECClassId classId, WStringCR instanceId)
    {
    CachedInstance originalAspectAccessor;
    auto ioriginal = m_otherAspects.find(DgnClassId(classId));
    if (ioriginal == m_otherAspects.end())
        return nullptr;

    for (CachedInstance& oaccs : ioriginal->second)
        {
        if (oaccs.m_instance->GetInstanceId() == instanceId)
            {
            return &oaccs;
            }
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedInstance::ApplyScheduledChangesToAspect(DgnElementR el)
    {
    if (m_changeType == InstanceChangeType::NoChange)
        return BSISUCCESS;

    if (!m_instance.IsValid())
        {
        BeAssert(false && "Missing instance for change");
        return BSIERROR;
        }

    InstanceUpdateOutcome outcome;
    if (BSISUCCESS != ApplyScheduledChange(outcome, el.GetDgnDb()))
        return BSIERROR;

    //  For an insert, we must create a DGN_RELNAME_ElementOwnsAspects relationship between the element and each new aspect
    if (outcome == InstanceUpdateOutcome::Inserted)
        {
        if (BSISUCCESS != insertRelationship(el.GetDgnDb(), el.GetElementKey(), DGN_RELNAME_ElementOwnsAspects, *m_instance))
            return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnElement::_ApplyScheduledChangesToInstances(DgnElement& modified)
    {
    CachedInstances* modifiedInstances = CachedInstances::Find(modified);
    if (nullptr == modifiedInstances)
        return BSISUCCESS;

    if (BSISUCCESS != modifiedInstances->m_element.ApplyScheduledChangeToElementInstance(*this))
        return BSIERROR;
    
    if (BSISUCCESS != modifiedInstances->m_item.ApplyScheduledChangesToItem(*this))
        return BSIERROR;

    //  Check for changes to other aspects
    for (bmap<DgnClassId, bvector<CachedInstance>>::value_type& proposedAspectChanges : modifiedInstances->m_otherAspects)
        {
        for (CachedInstance& proposedAspect : proposedAspectChanges.second)
            {
            if (BSISUCCESS != proposedAspect.ApplyScheduledChangesToAspect(*this))
                return BSIERROR;
            }
        }

    return BSISUCCESS;
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_ClearScheduledChangesToInstances()
    {
    CachedInstances* thisInstances = CachedInstances::Find(*this);
    if (nullptr != thisInstances)
        thisInstances->Clear();
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
