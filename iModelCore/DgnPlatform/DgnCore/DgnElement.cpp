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
    ECN::IECInstancePtr m_instance;

    CachedInstance() : m_changeType(InstanceChangeType::NoChange) {;}
    CachedInstance(ECN::IECInstancePtr instance, InstanceChangeType ct = InstanceChangeType::NoChange) : m_instance(instance), m_changeType(ct) {;}
    void Clear() {m_changeType = InstanceChangeType::NoChange; m_instance = nullptr;}

    BentleyStatus ApplyScheduledDelete(DgnDbR);
    BentleyStatus ApplyScheduledReplace(DgnDbR);
    BentleyStatus ApplyScheduledInsert(DgnDbR);
    BentleyStatus ApplyScheduledChange(InstanceUpdateOutcome&, DgnDbR);
    BentleyStatus ApplyScheduledChangeToElementInstance(DgnElementR);
    BentleyStatus ApplyScheduledChangesToItem(GeometricElementR);
    BentleyStatus ApplyScheduledChangesToAspect(GeometricElementR);
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

    virtual void _OnCleanup(DgnElementCP host, bool unloadingModel, HeapZoneR zone) override;

    CachedInstances() : m_itemChecked(false) {;}
    void Clear() {m_element.Clear(); m_item.Clear(); m_otherAspects.clear(); m_itemChecked=false;}

    CachedInstance* FindAspectInstanceAccessor(ECN::ECClassId classId, WStringCR instanceId);

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
        el.AddAppData(s_key, d, el.GetHeapZone());
        }
    return *d;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CachedInstances::_OnCleanup(DgnElementCP host, bool unloadingModel, HeapZoneR zone)
    {
    delete this; // *** WIP_ELEMENT_INSTANCES - use HeapZone to avoid malloc
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId GeometricElement::GetItemClassId() const
    {
    if (!m_itemClassId.IsValid())
        {
        ElementItemKey itemKey = GetDgnDb().Items().QueryItemKey(GetElementId());
        m_itemClassId = DgnClassId(itemKey.GetECClassId());
        }
    return m_itemClassId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementItemHandler& GeometricElement::GetItemHandler() const
    {
    if (nullptr == m_itemHandler)
        m_itemHandler = ElementItemHandler::GetHandler().GetItemHandler(GetDgnDb(), GetItemClassId());

    return *m_itemHandler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DgnElement::_AddRef() const
    {
    if (0 == m_refCount && IsInPool())
        GetDgnDb().Elements().GetPool().OnReclaimed(*this);

    return ++m_refCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DgnElement::_Release() const
    {
    if (1 < m_refCount--)
        return m_refCount.load();

    BeAssert(m_refCount==0);

    if (IsInPool())
        {
        // add to the DgnFile's unreferenced element count
        GetDgnDb().Elements().GetPool().OnUnreferenced(*this);
        return  0;
        }

    // is not in the pool, just delete it
    delete this;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppDataEntry* DgnElement::FreeAppDataEntry(AppDataEntry* prev, AppDataEntry& thisEntry, HeapZone& zone, bool cacheUnloading) const
    {
    AppDataEntry* next = thisEntry.m_next;

    if (prev)
        prev->m_next = next;
    else
        m_appData = next;

    thisEntry.ClearEntry(this, cacheUnloading, zone);
    if (!cacheUnloading)
        zone.Free(&thisEntry, sizeof(AppDataEntry));

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

        return (thisEntry->m_key == &key) ? thisEntry->m_obj : NULL;
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnElement::AddAppData (DgnElementAppData::Key const& key, DgnElementAppData* obj, HeapZone& zone, bool allowOnDeleted) const
    {
    if (!allowOnDeleted && IsDeleted())
        {
        BeAssert (0);
        return ERROR;
        }

    AppDataEntry* prevEntry = NULL;
    AppDataEntry* nextEntry = m_appData;
    for (; nextEntry; prevEntry=nextEntry, nextEntry=nextEntry->m_next)
        {
        if (nextEntry->m_key < &key) // sort them by key
            continue;

        if (nextEntry->m_key != &key)
            break;

        nextEntry->SetEntry (obj, this, zone);      // already exists, just change it
        return SUCCESS;
        }

    AppDataEntry* newEntry = (AppDataEntry*) zone.Alloc (sizeof(AppDataEntry));
    newEntry->Init (key, obj, nextEntry);

    if (prevEntry)
        prevEntry->m_next = newEntry;
    else
        m_appData = newEntry;

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnElement::DropAppData (DgnElementAppData::Key const& key) const
    {
    for (AppDataEntry* prev=NULL, *thisEntry=m_appData; thisEntry; prev=thisEntry, thisEntry=thisEntry->m_next)
        {
        if (thisEntry->m_key < &key) // entries are sorted by key
            continue;

        if (thisEntry->m_key != &key)
            break;

        FreeAppDataEntry (prev, *thisEntry, GetHeapZone(), false);
        return  SUCCESS;
        }

    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::ClearAllAppData (HeapZone& zone, bool cacheUnloading)
    {
    for (AppDataEntry* thisEntry=m_appData; thisEntry; )
        thisEntry = FreeAppDataEntry (NULL, *thisEntry, zone, cacheUnloading);
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
    uint32_t oldSize = _GetMemSize(); // save current size
    m_geom = *stream;     // assign the new element (overwrites or reallocates)
    int32_t sizeChange = _GetMemSize() - oldSize; // figure out whether the element data is larger now than before
    BeAssert(0 <= sizeChange); // we never shrink

    if (0 < sizeChange) // report the number or bytes the element grew.
        GetDgnDb().Elements().GetPool().AllocatedMemory(sizeChange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::MarkAsDeleted()
    {
    SetDeletedRef();
    SetDirtyFlags(DgnElement::DIRTY_Both);

    SetHilited(HILITED_None);
    m_dgnModel.ElementChanged(*this, ELEMREF_CHANGE_REASON_Delete);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement::_DeleteInDb(DgnElementPool& pool)
    {
#if defined (NEEDS_WORK_ELEMDSCR_REWORK)
    if (m_dgnModel.IsReadOnly())
        return  DGNMODEL_STATUS_ReadOnly;

    // let handler reject deletion
    if (ElementHandler::PRE_ACTION_Ok != m_elementHandler._OnElementDelete(*this))
        return  DGNMODEL_STATUS_BadRequest;

    MarkRefDeleted(m_dgnModel);

    // to delete an element, we delete it from the Element table. Foreign keys and triggers cause all of the related
    // tables to be cleaned up.
    CachedStatementPtr deleteStmt;
    DbResult status = m_dgndb.GetCachedStatement(deleteStmt, "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?");
    deleteStmt->BindId(1, elementId);
    DbResult rc = deleteStmt->Step();

    if (rc == BE_SQLITE_CONSTRAINT_FOREIGNKEY)
        {
        UnDeleteElement();
        return  DGNMODEL_STATUS_ForeignKeyConstraint;
        }

    m_dgnModel.OnElementDeletedFromDb(*this, false);

    if (rc != BE_SQLITE_DONE)
        {
        BeAssert (false);
        return ERROR;
        }

    m_elementHandler._OnElementDeleted(GetDgnDb(), m_elementId);
#endif
    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* undelete a deleted element
* @bsimethod                                                    KeithBentley    03/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::UnDeleteElement()
    {
    if (m_dgnModel.IsReadOnly())
        return;

    if (!IsDeleted())
        return;

    ClearDeletedRef();
    SetDirtyFlags(DIRTY_Both);

#if defined (NEEDS_WORK_ELEMDSCR_REWORK)
    m_dgnModel.InsertRangeElement(_ToGeometricElement());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement::ReloadFromDb ()
    {
#if defined (NEEDS_WORK_ELEMDSCR_REWORK)
    m_dgnModel.ElementChanged (*this, ELEMREF_CHANGE_REASON_Modify);

    DbElementReloader reloader (*this);
    DgnModelStatus stat = reloader.ReloadElement();

    if (DGNMODEL_STATUS_Success == stat)
        m_dgnModel.OnElementModified(*this);

    return  stat;
#endif
    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::ForceElemChanged (bool qvCacheCleared, DgnElementChangeReason reason)
    {
    HeapZone& zone = GetHeapZone();
    for (AppDataEntry* prev=NULL, *next, *thisEntry=m_appData; thisEntry; thisEntry=next)
        {
        next = thisEntry->m_next;
        if (thisEntry->m_obj->_OnElemChanged (this, qvCacheCleared, reason))
            FreeAppDataEntry (prev, *thisEntry, zone, false);
        else
            prev = thisEntry;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometricElement::SetQvElem (QvElem* qvElem, uint32_t index)
    {
    GetQvElems(true)->Add (index, qvElem);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void QvKey32::DeleteQvElem (QvElem* qvElem)
    {
    if (qvElem && qvElem != INVALID_QvElem)
        T_HOST.GetGraphicsAdmin()._DeleteQvElem(qvElem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/06
+---------------+---------------+---------------+---------------+---------------+------*/
T_QvElemSet* GeometricElement::GetQvElems (bool createIfNotPresent) const
    {
    static DgnElementAppData::Key s_qvElemsKey;
    T_QvElemSet* qvElems = (T_QvElemSet*) FindAppData (s_qvElemsKey);
    if (qvElems)
        return  qvElems;

    if (!createIfNotPresent)
        return  NULL;

    HeapZone& zone = GetHeapZone();
    qvElems = new ((T_QvElemSet*) zone.Alloc (sizeof(T_QvElemSet))) T_QvElemSet (zone);

    AddAppData (s_qvElemsKey, qvElems, zone);
    return  qvElems;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::SetInSelectionSet (bool yesNo)
    {
    m_flags.m_inSelectionSet = yesNo;
    SetHilited (yesNo ? HILITED_Normal : HILITED_None);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::DeallocateRef(DgnElementPool& pool, bool dbUnloading)
    {
    ClearAllAppData (pool.GetHeapZone(), dbUnloading);
    pool.ReturnedMemory(_GetMemSize());

    delete this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement::AddToModel()
    {
    if (m_elementId.IsValid())
        return DGNMODEL_STATUS_IdExists;

    DgnModelStatus stat = _OnAdd();
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    stat = m_dgnModel._OnAddElement(*this);
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    DgnElements& elements =m_dgnModel.GetDgnDb().Elements();
    m_elementId = elements.MakeNewElementId();

    stat = _InsertInDb(elements.GetPool());
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    _ApplyScheduledChangesToInstances(*this);
    _ClearScheduledChangesToInstances();

    m_dgnModel._OnAddedElement(*this);
    _OnAdded();

    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId DgnElement::GetClassId(DgnDbR db)
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
* @bsimethod                                    Shaun.Sewall                    04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_GenerateDefaultCode()
    {
    if (m_elementId.IsValid())
        {
        Utf8String className(GetElementClass()->GetName());
        m_code = Utf8PrintfString("%s%lld", className.c_str(), m_elementId.GetValue());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement::_LoadFromDb(DgnElementPool& pool)
    {
    // Note: DgnElementPool::FindOrLoadElement takes care of populating DgnElement's member variables by calling the handler's _CreateInstance method with CreateParams set up from a DB query
    pool.AddDgnElement(*this);
    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement::_InsertInDb(DgnElementPool& pool)
    {
    if (m_code.empty())
        _GenerateDefaultCode();

    CachedStatementPtr stmt;
    pool.GetStatement(stmt, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Element) " (Id,ECClassId,ModelId,CategoryId,Code,ParentId) VALUES(?,?,?,?,?,?)");

    stmt->BindId(1, m_elementId);
    stmt->BindId(2, m_classId);
    stmt->BindId(3, m_dgnModel.GetModelId());
    stmt->BindId(4, m_categoryId);
    stmt->BindText(5, m_code.c_str(), Statement::MakeCopy::No);
    stmt->BindId(6, m_parentId);

    if (stmt->Step() != BE_SQLITE_DONE)
        return DGNMODEL_STATUS_ElementWriteError;

    pool.AddDgnElement(*this);
    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement::_UpdateInDb(DgnElementPool& pool)
    {
    CachedStatementPtr stmt;
    pool.GetStatement(stmt, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Element) " SET ECClassId=?,ModelId=?,CategoryId=?,Code=?,ParentId=? WHERE Id=?");

    stmt->BindId(1, m_classId);
    stmt->BindId(2, m_dgnModel.GetModelId());
    stmt->BindId(3, m_categoryId);
    stmt->BindText(4, m_code.c_str(), Statement::MakeCopy::No);
    stmt->BindId(5, m_parentId);
    stmt->BindId(6, m_elementId);

    if (stmt->Step() != BE_SQLITE_DONE)
        return DGNMODEL_STATUS_ElementWriteError;

    return DGNMODEL_STATUS_Success;
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
    GeomBlobHeader(BeSQLite::SnappyReader& in) {uint32_t actuallyRead; in._Read ((Byte*) this, sizeof(*this), actuallyRead);}
};

static Utf8CP GEOM_Column = "Geom";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus GeometricElement::_LoadFromDb(DgnElementPool& pool)
    {
    DgnModelStatus stat = T_Super::_LoadFromDb(pool);
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    SnappyFromBlob& snappy = pool.GetSnappyFrom();

    if (ZIP_SUCCESS != snappy.Init(pool.GetDgnDb(), DGN_TABLE(DGN_CLASSNAME_ElementGeom), GEOM_Column, m_elementId.GetValue()))
        {
        return DGNMODEL_STATUS_Success; // this element has no geometry
        }

    GeomBlobHeader header (snappy);
    if ((GeomBlobHeader::Signature != header.m_signature) || 0 == header.m_size)
        {
        BeAssert (false);
        return DGNMODEL_STATUS_ElementReadError;
        }

    // determine how much memory is to be allocated for this element's geom (new size - old size)
    uint32_t oldSize = m_geom.GetAllocSize();
    m_geom.ReserveMemory(header.m_size);
    int32_t sizeChange = m_geom.GetAllocSize() - oldSize;
    if (0 < sizeChange)
        pool.AllocatedMemory(sizeChange);

    uint32_t actuallyRead;
    snappy._Read (m_geom.GetDataR(), m_geom.GetSize(), actuallyRead);

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
DgnModelStatus GeometricElement::_InsertInDb(DgnElementPool& pool)
    {
    DgnModelStatus stat = T_Super::_InsertInDb(pool);
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    CachedStatementPtr stmt;
    pool.GetStatement(stmt, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_ElementGeom) "(Geom,Range,Box,Origin,Rotation,ElementId) VALUES(?,?,?,?,?,?)");
    stmt->BindId(6, m_elementId);

    stat = _BindInsertGeom(*stmt);
    if (DGNMODEL_STATUS_NoGeometry == stat)
        return DGNMODEL_STATUS_Success;

    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    return DoInsertOrUpdate(*stmt, pool);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus GeometricElement::_UpdateInDb(DgnElementPool& pool)
    {
    DgnModelStatus stat = T_Super::_UpdateInDb(pool);
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    CachedStatementPtr stmt;
    pool.GetStatement(stmt, "UPDATE " DGN_TABLE(DGN_CLASSNAME_ElementGeom) " SET Geom=?,Range=?,Box=?,Origin=?,Rotation=? WHERE ElementId=?");
    stmt->BindId(6, m_elementId);

    stat = _BindInsertGeom(*stmt);
    if (DGNMODEL_STATUS_NoGeometry == stat)
        return DGNMODEL_STATUS_Success;

    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    return DoInsertOrUpdate(*stmt, pool);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus GeometricElement::DoInsertOrUpdate(Statement& stmt, DgnElementPool& pool)
    {
    SnappyToBlob& snappy = pool.GetSnappyTo();

    snappy.Init();
    if (0 < m_geom.GetSize())
        {
        GeomBlobHeader header(m_geom);
        snappy.Write((ByteCP) &header, sizeof (header));
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

    StatusInt status = snappy.SaveToRow (pool.GetDgnDb(), DGN_TABLE(DGN_CLASSNAME_ElementGeom), GEOM_Column, m_elementId.GetValue());
    return (SUCCESS != status) ? DGNMODEL_STATUS_ElementWriteError : DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement3d::_BindInsertGeom(Statement& stmt)
    {
    if (!m_placement.IsValid())
        DGNMODEL_STATUS_NoGeometry;

    int col=2;
    stmt.BindBlob(col++, &m_placement.GetRange(), sizeof(DRange3d), Statement::MakeCopy::No);
    stmt.BindBlob(col++, &m_placement.GetElementBox(), sizeof(DRange3d), Statement::MakeCopy::No);
    stmt.BindBlob(col++, &m_placement.GetOrigin(), sizeof(DPoint3d), Statement::MakeCopy::No);
    stmt.BindBlob(col++, &m_placement.GetAngles(), sizeof(YawPitchRollAngles), Statement::MakeCopy::No);
    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement3d::_LoadFromDb(DgnElementPool& pool)
    {
    DgnModelStatus stat = T_Super::_LoadFromDb(pool);
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    if (IsUndisplayed()) // has no geometry, and therefore no placement
        return DGNMODEL_STATUS_Success;

    CachedStatementPtr stmt;
    pool.GetStatement(stmt, "SELECT Range,Box,Origin,Rotation FROM " DGN_TABLE(DGN_CLASSNAME_ElementGeom) " Where ElementId=?");
    stmt->BindId(1, m_elementId);

    if (BE_SQLITE_ROW != stmt->Step())
        {
        BeAssert(false);
        return DGNMODEL_STATUS_ElementReadError;
        }

    if (stmt->GetColumnBytes(0) != sizeof(DRange3d) ||
        stmt->GetColumnBytes(1) != sizeof(DRange3d) ||
        stmt->GetColumnBytes(2) != sizeof(DPoint3d) ||
        stmt->GetColumnBytes(3) != sizeof(YawPitchRollAngles))
        {
        BeAssert(false);
        return DGNMODEL_STATUS_ElementReadError;
        }

    memcpy(&m_placement.GetRangeR(), stmt->GetValueBlob(0), sizeof(DRange3d));
    memcpy(&m_placement.GetElementBoxR(), stmt->GetValueBlob(1), sizeof(DRange3d));
    memcpy(&m_placement.GetOriginR(), stmt->GetValueBlob(2), sizeof(DPoint3d));
    memcpy(&m_placement.GetAnglesR(), stmt->GetValueBlob(3), sizeof(YawPitchRollAngles));

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

    PhysicalElementPtr elementPtr = new PhysicalElement(CreateParams(model, PhysicalElement::GetClassId(model.GetDgnDb()), categoryId));
    elementPtr->SetItemClassId(ElementItemHandler::GetHandler().GetItemClassId(model.GetDgnDb()));
    return elementPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId PhysicalElement::GetClassId(DgnDbR db)
    {
    return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus PhysicalElement::_InsertInDb(DgnElementPool& pool)
    {
    DgnModelStatus stat = T_Super::_InsertInDb(pool);
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    if (!m_placement.IsValid()) // this is an invisible element
        return DGNMODEL_STATUS_Success;

    CachedStatementPtr stmt;
    pool.GetStatement(stmt, "INSERT INTO " DGN_VTABLE_PrjRTree " (ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ) VALUES (?,?,?,?,?,?,?)");

    stmt->BindId(1, m_elementId);
    AxisAlignedBox3dCR range = m_placement.GetRange();
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
DgnModelStatus PhysicalElement::_UpdateInDb(DgnElementPool& pool)
    {
    DgnModelStatus stat = T_Super::_UpdateInDb(pool);
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    if (!m_placement.IsValid()) // this is an invisible element
        return DGNMODEL_STATUS_Success;

    CachedStatementPtr stmt;
    pool.GetStatement(stmt, "UPDATE " DGN_VTABLE_PrjRTree " SET MinX=?,MaxX=?,MinY=?,MaxY=?,MinZ=?,MaxZ=? WHERE ElementId=?");

    AxisAlignedBox3dCR range = m_placement.GetRange();
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
DgnModelStatus DgnElement2d::_BindInsertGeom(Statement& stmt)
    {
#if defined (NEEDS_WORK_ELEMDSCR_REWORK)
#endif
    return DGNMODEL_STATUS_Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement2d::_LoadFromDb(DgnElementPool& pool)
    {
#if defined (NEEDS_WORK_ELEMDSCR_REWORK)
#endif
    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/06
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem* GeometricElement::GetQvElem (uint32_t id) const
    {
    T_QvElemSet* qvElems = GetQvElems(false);
    return qvElems ? qvElems->Find (id) : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement::_SwapWithModified(DgnElementR other)
    {
    if (m_elementId != other.m_elementId || &m_dgnModel!=&other.m_dgnModel || !IsSameType(other))
        return DGNMODEL_STATUS_BadElement;

    std::swap(m_categoryId, other.m_categoryId);
    std::swap(m_code, other.m_code);
    other.SetDeletedRef();

    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::_InitFrom(DgnElementCR other)
    {
    T_Super::_InitFrom(other);

    GeometricElementCP otherGeom = other.ToGeometricElement();
    if (nullptr == otherGeom)
        return;

    SaveGeomStream(&otherGeom->m_geom);
    m_itemClassId = otherGeom->m_itemClassId;
    m_itemHandler = otherGeom->m_itemHandler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement3d::_InitFrom(DgnElementCR other)
    {
    T_Super::_InitFrom(other);

    DgnElement3dCP other3d = other.ToElement3d();
    if (nullptr == other3d)
        return;

    m_placement = other3d->m_placement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement2d::_InitFrom(DgnElementCR other)
    {
    T_Super::_InitFrom(other);

    DgnElement2dCP other2d = other.ToElement2d();
    if (nullptr == other2d)
        return;

    m_placement = other2d->m_placement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus GeometricElement::_SwapWithModified(DgnElementR other)
    {
    DgnModelStatus stat = T_Super::_SwapWithModified(other);
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    GeometricElementP geom = (GeometricElementP) other.ToGeometricElement();
    if (nullptr == geom)
        return DGNMODEL_STATUS_BadElement;

    std::swap(m_geom, geom->m_geom);
    std::swap(m_itemClassId, geom->m_itemClassId);
    std::swap(m_itemHandler, geom->m_itemHandler);

    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement3d::_SwapWithModified(DgnElementR other)
    {
    DgnModelStatus stat = T_Super::_SwapWithModified(other);
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    DgnElement3dP el3d = (DgnElement3dP) other.ToElement3d();
    if (nullptr == el3d)
        return DGNMODEL_STATUS_BadElement;

    std::swap(m_placement, el3d->m_placement);
    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnElement2d::_SwapWithModified(DgnElementR other)
    {
    DgnModelStatus stat = T_Super::_SwapWithModified(other);
    if (DGNMODEL_STATUS_Success != stat)
        return stat;

    DgnElement2dP el2d = (DgnElement2dP) other.ToElement2d();
    if (nullptr == el2d)
        return DGNMODEL_STATUS_BadElement;

    std::swap(m_placement, el2d->m_placement);
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
DgnElementPtr DgnElement::Duplicate() const
    {
    DgnElementPtr newEl = GetElementHandler()._CreateInstance(DgnElement::CreateParams(m_dgnModel, m_classId, m_categoryId, m_code.c_str(), m_elementId, m_parentId));
    newEl->_InitFrom(*this);
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

    m_data = (uint8_t*) realloc (m_data, size);
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
    ElementLoadedEventCaller (DgnElementR elRef) : m_elRef (elRef) {}
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
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static BentleyStatus deleteInstance(DgnDbR db, ECN::IECInstanceCR instance)
    {
    BeAssert(!instance.GetInstanceId().empty());
    BeSQLite::EC::ECInstanceDeleter deleter(db, instance.GetClass());
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
        BeAssert(false);
        return BSIERROR;
        }
    
    BeSQLite::EC::ECInstanceUpdater updater(db, *m_instance);
    return updater.Update(*m_instance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      04/2015
//---------------------------------------------------------------------------------------
BentleyStatus CachedInstance::ApplyScheduledInsert(DgnDbR db)
    {
    ECInstanceId newId;
    ECInstanceId* useNewId = ECInstanceIdHelper::FromString(newId, m_instance->GetInstanceId().c_str())? &newId: nullptr;

    BeSQLite::EC::ECInstanceKey newkey;
    BeSQLite::EC::ECInstanceInserter inserter(db, m_instance->GetClass());
    return inserter.Insert(newkey, *m_instance, (useNewId == nullptr), useNewId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
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
static Utf8String getPropertiesList(ECN::ECClassCR ecclass, bset<Utf8String> const& ignoreList)
    {
    Utf8String props;

    props.append("ECInstanceId");   // so that I'll always have at least one valid property

    for (auto ecprop : ecclass.GetProperties())
        {
        Utf8String propName(ecprop->GetName());
        if (ignoreList.find(propName) != ignoreList.end())
            continue;

        if (!props.empty())
            props.append(",");

        props.append(propName.c_str());
        }

    return props;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
static void setInstanceId(ECN::IECInstanceR instance, EC::ECInstanceId id)
    {
    wchar_t buf[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH];
    ECInstanceIdHelper::ToString(buf, _countof(buf), id);
    instance.SetInstanceId(buf);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
static void setInstanceId(ECN::IECInstanceR instance, DgnElementId elementId)
    {
    setInstanceId(instance, EC::ECInstanceId(elementId.GetValue()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
static ECN::IECInstancePtr queryInstance(DgnDbR db, ECN::ECClassCP ecclass, EC::ECInstanceId instanceId, bset<Utf8String> const& ignoreList = bset<Utf8String>())
    {
    if (nullptr == ecclass)
        return nullptr;

    Utf8String props = getPropertiesList(*ecclass, ignoreList);

    ECSqlSelectBuilder b;
    b.Select(props.c_str()).From(*ecclass, false).Where("ECInstanceId=?");
    ECSqlStatement stmt;
    stmt.Prepare(db, b.ToString().c_str());
    stmt.BindId(1, instanceId);
    stmt.Step();
    ECInstanceECSqlSelectAdapter selector(stmt);
    ECN::IECInstancePtr instance = selector.GetInstance();
    if (!instance.IsValid())
        return nullptr;

    setInstanceId(*instance, instanceId);

    return instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
static ECN::IECInstancePtr getDirectlyLinkedInstance(DgnDbR db, ECN::ECClassCP ecclass, DgnElementId elementId, bset<Utf8String> const& ignoreList)
    {
    return queryInstance(db, ecclass, EC::ECInstanceId(elementId.GetValue()), ignoreList);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
ECN::IECInstanceR DgnElement::GetSubclassProperties(bool setModified) const
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
ECN::IECInstanceCR DgnElement::GetSubclassProperties()  const {return GetSubclassProperties(false);}
ECN::IECInstanceR  DgnElement::GetSubclassPropertiesR()       {return GetSubclassProperties(true);}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
void DgnElement::CancelSubclassPropertiesChange()
    {
    CachedInstances* instances = CachedInstances::Find(*this);
    if (nullptr == instances)
        return;
    instances->m_element.Clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
ECN::IECInstanceP GeometricElement::GetItem(bool setModified) const
    {
    if (!GetElementId().IsValid())
        return nullptr;

    CachedInstances& instances = CachedInstances::Get(*this);
    if (!instances.m_itemChecked)
        {
        ECClassCP itemClass = GetDgnDb().Schemas().GetECClass(GetItemClassId().GetValue());

        instances.m_itemChecked = true;
        instances.m_item = CachedInstance(getDirectlyLinkedInstance(GetDgnDb(), itemClass, GetElementId(), bset<Utf8String>()));
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
ECN::IECInstanceCP GeometricElement::GetItem() const {return GetItem(false);}
ECN::IECInstanceP  GeometricElement::GetItemP()        {return GetItem(true);}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
void GeometricElement::SetItem(ECN::IECInstanceR instance)
    {
    CachedInstances& instances = CachedInstances::Get(*this);
    instances.m_itemChecked = true;
    instances.m_item = CachedInstance(&instance, InstanceChangeType::Write);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
void GeometricElement::RemoveItem()
    {
    if (nullptr == GetItem()) // make sure we have the existing item cached.
        return; // no item => nop

    CachedInstances& instances = CachedInstances::Get(*this);
    instances.m_item.m_changeType = InstanceChangeType::Delete;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
void GeometricElement::CancelItemChange()
    {
    CachedInstances* instances = CachedInstances::Find(*this);
    if (nullptr == instances)
        return;
    instances->m_item.Clear();
    instances->m_itemChecked = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
bvector<ECN::IECInstancePtr> GeometricElement::GetAspects(ECN::ECClassCP aspectClass) const
    {
    bvector<ECN::IECInstancePtr> instances;

    if (nullptr == aspectClass)
        return instances;

    ECN::ECRelationshipClassCP elementOwnsAspectRelationship = (ECN::ECRelationshipClassCP)GetDgnDb().Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_RELNAME_ElementOwnsAspects);
    if (nullptr == elementOwnsAspectRelationship)
        {
        BeAssert(false && "missing or invalid dgn schema");
        return instances;
        }

    Utf8String aspectProps = getPropertiesList(*aspectClass, bset<Utf8String>());

    ECSqlSelectBuilder b;
    b.Select(aspectProps.c_str()).From(*aspectClass, "a", false).Join(*GetElementClass(), "e", false).Using(*elementOwnsAspectRelationship).WhereSourceEndIs("e", "?");
    ECSqlStatement stmt;
    stmt.Prepare(GetDgnDb(), b.ToString().c_str());
    stmt.BindId(1, GetElementId());
    while (stmt.Step() == ECSqlStepStatus::HasRow)
        {
        ECInstanceECSqlSelectAdapter selector(stmt);
        ECN::IECInstancePtr instance = selector.GetInstance();
        if (!instance.IsValid())
            {
            BeAssert(false);
            continue;
            }

        // ***WIP - how to find ECInstanceId of selected instance??
        ECN::ECValue idValue;
        instance->GetValue(idValue, L"ECInstanceId");
        setInstanceId(*instance, EC::ECInstanceId(idValue.GetLong()));

        instances.push_back(instance);
        }

    return instances;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
template<typename RTYPE, bool SETMODIFIED>
bvector<RTYPE> GeometricElement::GetAspects(DgnClassId aspectClassId) const
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
bvector<ECN::IECInstanceCP> GeometricElement::GetAspects(DgnClassId aspectClassId) const {return GetAspects<IECInstanceCP,false>(aspectClassId);}
bvector<ECN::IECInstanceP>  GeometricElement::GetAspectsP(DgnClassId aspectClassId)      {return GetAspects<IECInstanceP, true> (aspectClassId);}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
void GeometricElement::SetAspect(ECN::IECInstanceR aspectInstance)
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
void GeometricElement::RemoveAspect(DgnClassId aspectClassId, ECInstanceId aspectId)
    {
    GetAspects(aspectClassId); // make sure we have read the aspects that are there.

    CachedInstances& instances = CachedInstances::Get(*this);

    wchar_t aspectIdStr[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH];
    ECInstanceIdHelper::ToString(aspectIdStr, _countof(aspectIdStr), aspectId);

    CachedInstance* aspect = instances.FindAspectInstanceAccessor(aspectClassId.GetValue(), aspectIdStr);
    if (nullptr != aspect)
        aspect->m_changeType = InstanceChangeType::Delete;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  04/2015
//---------------------------------------------------------------------------------------
void GeometricElement::CancelAspectChange(DgnClassId aspectClass, EC::ECInstanceId aspectId)
    {
    CachedInstances* instances = CachedInstances::Find(*this);
    if (nullptr == instances)
        return;

    wchar_t aspectIdString[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH];
    ECInstanceIdHelper::ToString(aspectIdString, _countof(aspectIdString), aspectId);
    CachedInstance* aspect = instances->FindAspectInstanceAccessor(aspectClass.GetValue(), aspectIdString);
    if (nullptr == aspect)
        return;

    aspect->m_changeType = InstanceChangeType::NoChange;
    aspect->m_instance = queryInstance(GetDgnDb(), GetDgnDb().Schemas().GetECClass(aspectClass.GetValue()), aspectId);
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
static BentleyStatus insertRelationship(DgnDbR db, DgnElementKeyCR sourceElement, Utf8CP relName, ECN::IECInstanceR targetInstance)
    {
    if (!sourceElement.IsValid() || targetInstance.GetInstanceId().empty())
        return BentleyStatus::ERROR;


    CachedECSqlStatementPtr statementPtr = db.GetPreparedECSqlStatement(Utf8PrintfString(
        "INSERT INTO dgn.%s (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES (?,?,?,?)", relName));

    if (!statementPtr.IsValid())
        return BentleyStatus::ERROR;

    statementPtr->BindInt64(1, sourceElement.GetECClassId());
    statementPtr->BindId   (2, sourceElement.GetECInstanceId());
    statementPtr->BindInt64(3, targetInstance.GetClass().GetId());
    statementPtr->BindText (4, Utf8String(targetInstance.GetInstanceId()).c_str(), EC::IECSqlBinder::MakeCopy::Yes);

    if (ECSqlStepStatus::Done != statementPtr->Step())
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedInstance::ApplyScheduledChangesToItem(GeometricElementR el)
    {
    if (m_changeType == InstanceChangeType::NoChange)
        return BSISUCCESS;

    if (!m_instance.IsValid())
        {
        BeAssert(false && "Missing instance for change");
        return BSIERROR;
        }

    //  Ensure that the proposed item has the correct ID
    setInstanceId(*m_instance, el.GetElementId());

    if (m_changeType == InstanceChangeType::Write)
        {
        //  There is 1 item. If we want to change its class, we must delete the existing one.
        if (el.GetItemClassId().GetValue() != m_instance->GetClass().GetId())
            el.GetDgnDb().Items().DeleteItem(el.GetElementId());
        }

    //  Insert, update, or delete the item.
    InstanceUpdateOutcome outcome;
    if (BSISUCCESS != ApplyScheduledChange(outcome, el.GetDgnDb()))
        return BSIERROR;

    //  Update the DgnElement's ItemClassId member variable
    if (outcome == InstanceUpdateOutcome::Deleted)
        el.SetItemClassId(DgnClassId());
    else
        el.SetItemClassId(DgnClassId(m_instance->GetClass().GetId()));

    // Note: the ElementOwnsItem relationship is created implicitly when we create the item with its ElementId column set to match the owning element's ID.

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CachedInstance* CachedInstances::FindAspectInstanceAccessor(ECN::ECClassId classId, WStringCR instanceId)
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
BentleyStatus CachedInstance::ApplyScheduledChangesToAspect(GeometricElementR el)
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

    return modifiedInstances->m_element.ApplyScheduledChangeToElementInstance(*this);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GeometricElement::_ApplyScheduledChangesToInstances(DgnElement& modified)
    {
    CachedInstances* modifiedInstances = CachedInstances::Find(modified);
    if (nullptr == modifiedInstances)
        return BSISUCCESS;

    if (BSISUCCESS != T_Super::_ApplyScheduledChangesToInstances(modified))
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

static const double s_smallVal = .0005;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
Placement3d::Placement3d(DPoint3dCR origin, YawPitchRollAngles angles, ElementAlignedBox3dCR box) : m_origin(origin), m_angles(angles), m_boundingBox(box)
    {
    angles.ToTransform(origin).Multiply(m_range, box);

    // low and high are no longer allowed to be equal...
    if (m_range.low.x == m_range.high.x)
        {
        m_range.low.x -= s_smallVal;
        m_range.high.x += s_smallVal;
        }

    if (m_range.low.y == m_range.high.y)
        {
        m_range.low.y -= s_smallVal;
        m_range.high.y += s_smallVal;
        }

    if (m_range.low.z == m_range.high.z)
        {
        m_range.low.z -= s_smallVal;
        m_range.high.z += s_smallVal;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
Placement2d::Placement2d(DPoint2dCR origin, double angle, ElementAlignedBox2dCR box) : m_origin(origin), m_angle(angle), m_boundingBox(box)
    {
    Transform t;
    t.InitFromOriginAngleAndLengths(origin, angle, 1.0, 1.0);

    DRange3d box3d = DRange3d::From(&box.low, 2, 0.0);
    DRange3d range3d;
    t.Multiply(range3d, box3d);

    m_range.From(&range3d.low,2);

    // low and high are no longer allowed to be equal...
    if (m_range.low.x == m_range.high.x)
        {
        m_range.low.x -= s_smallVal;
        m_range.high.x += s_smallVal;
        }

    if (m_range.low.y == m_range.high.y)
        {
        m_range.low.y -= s_smallVal;
        m_range.high.y += s_smallVal;
        }
    }
