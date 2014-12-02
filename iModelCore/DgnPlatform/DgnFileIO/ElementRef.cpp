/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnFileIO/ElementRef.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/QvElemSet.h>
#include <DgnPlatform/DgnCore/QueryModel.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 PersistentElementRef::_AddRef() const 
    {
    BeAssert (NULL != &T_HOST);
    if (0 == m_refCount)
        GetDgnProject()->Models().ElementPool().OnReclaimed (*this);

    return ++m_refCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 PersistentElementRef::_Release() const 
    {
    BeAssert (NULL != &T_HOST);
    if (0 == m_refCount)
        {
        BeAssert (false);
        return  0;
        }

    if (0 != --m_refCount)
        return  m_refCount;

    // add to the DgnFile's unreferenced element count
    GetDgnProject()->Models().ElementPool().OnUnreferenced (*this);
    return  0;
    }

static ElementRefAppData::Key s_derivedRangeKey;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PersistentElementRef::AddDerivedRange (DerivedElementRange& cder)
    {
    if (NULL != FindAppData (s_derivedRangeKey))
        return ERROR;

    AddAppData (s_derivedRangeKey, &cder, _GetHeapZone());
    m_flags.hasComputedRange = COMPUTED_RANGE_FLAG_Derived;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PersistentElementRef::RemoveDerivedRange()
    {
    DropAppData (s_derivedRangeKey);    // calls _OnCleanup
    m_flags.hasComputedRange = COMPUTED_RANGE_FLAG_None;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
DerivedElementRange* PersistentElementRef::GetDerivedElementRange() const
    {
    if (!HasDerivedRange())
        return NULL;

    DerivedElementRange* cder = (DerivedElementRange*) const_cast<PersistentElementRef*>(this)->FindAppData (s_derivedRangeKey);
    BeAssert (NULL != cder); // "If ElementRefP::m_flags.hasDerivedFlag is set, ref must have derived range app data");
    return cder;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void PersistentElementRef::UpdateDerivedRange()
    {
    DerivedElementRange* cder = GetDerivedElementRange();
    if (NULL != cder)
        cder->_UpdateDerivedRange (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void PersistentElementRef::SetDynamicRange (bool b)
    {
    // WARNING: Modify only the dynamic range status of this element.
    // WARNING: m_flags.hasComputedRange can have 4 mutually exclusive values.
    // WARNING: COMPUTED_RANGE_FLAG_Dynamic is NOT simply a bit. It is one of 4 possible values.
    if (b)
        {
        if (m_flags.hasComputedRange != COMPUTED_RANGE_FLAG_Dynamic)
            {
            RemoveDerivedRange();
            m_flags.hasComputedRange =  COMPUTED_RANGE_FLAG_Dynamic;
            }
        }
    else
        {
        if (m_flags.hasComputedRange == COMPUTED_RANGE_FLAG_Dynamic)
            m_flags.hasComputedRange =  COMPUTED_RANGE_FLAG_None;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 DgnElementRef::GetTotalSizeBytes()
    {
    if (IsDeleted())
        return  0;

    return (UInt32) GetMemorySize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRef::AppDataEntry* ElementRef::FreeAppDataEntry (AppDataEntry* prev, AppDataEntry& thisEntry, HeapZone& zone, bool cacheUnloading)
    {
    AppDataEntry* next = thisEntry.m_next;

    if (prev)
        prev->m_next = next;
    else
        m_appData = next;

    thisEntry.ClearEntry (this, cacheUnloading, zone);
    if (!cacheUnloading)
        zone.Free (&thisEntry, sizeof(AppDataEntry));

    return  next;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefAppData* ElementRef::FindAppData (ElementRefAppData::Key const& key)
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
StatusInt ElementRef::AddAppData (ElementRefAppData::Key const& key, ElementRefAppData* obj, HeapZone& zone, bool allowOnDeleted)
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
StatusInt ElementRef::DropAppData (ElementRefAppData::Key const& key)
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
void ElementRef::ClearAllAppData (HeapZone& zone, bool cacheUnloading)
    {
    for (AppDataEntry* thisEntry=m_appData; thisEntry; )
        thisEntry = FreeAppDataEntry (NULL, *thisEntry, zone, cacheUnloading);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    02/01
+---------------+---------------+---------------+---------------+---------------+------*/
HeapZone& PersistentElementRef::_GetHeapZone()  {return GetDgnProject()->Models().GetHeapZone();}
QvCache*  PersistentElementRef::_GetMyQvCache() {return GetDgnModelP()->GetQvCache();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementRef::~DgnElementRef() 
    {
    BeAssert (NULL == m_elm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementP DgnElementRef::ReserveMemory (UInt32 size)
    {
    if (size > 20*1024*1024) // 20 meg to check for bogus calls
        {
        BeAssert (false);
        return NULL;
        }

    if (NULL == m_elm)
        {
        m_elm = (DgnElementP) malloc (size);
        GetDgnProject()->Models().ElementPool().AllocatedMemory (size);
        m_allocSize = size;
        }
    else if (m_allocSize < size)
        {
        m_elm = (DgnElementP) realloc (m_elm, size);
        GetDgnProject()->Models().ElementPool().AllocatedMemory (size - m_allocSize);
        m_allocSize = size;
        }

    return m_elm;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementRef::SaveElement (DgnElementCP el)
    {
    UInt32 size = el->Size();
    memcpy ((void*) ReserveMemory (size), el, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementRef::MarkRefDeleted (DgnModelR dgnModel)
    {
    SetDeletedRef();

    SetHilited (HILITED_None);
    dgnModel.ElementChanged (*this, ELEMREF_CHANGE_REASON_Delete);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void PersistentElementRef::DeleteElementAndComponents (DgnModelR dgnModel)
    {
    MarkRefDeleted(dgnModel);
    SetDirtyFlags(ElementRef::DIRTY_Both);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    03/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PersistentElementRef::_DeleteElement()
    {
    if (m_dgnModel->IsReadOnly())
        return  DGNMODEL_STATUS_ReadOnly;

    // let handler reject deletion
    BeAssert (m_handler != NULL);
    if (Handler::PRE_ACTION_Ok != m_handler->_OnDelete (*this))
        return  DGNMODEL_STATUS_BadRequest;

    DeleteElementAndComponents (*m_dgnModel);

    DbResult rc = m_dgnModel->GetDgnProject().Models().DeleteElementFromDb (GetElementId());
    if (rc == BE_SQLITE_CONSTRAINT_FOREIGNKEY)
        {
        UnDeleteElement();
        return  DGNMODEL_STATUS_ForeignKeyConstraint;
        }

    PersistentElementRefList* list = m_dgnModel->m_graphicElems;
    if (list)
        list->_OnElementDeletedFromDb(*this, false);

    if (rc != BE_SQLITE_DONE)
        {
        BeAssert (false);
        return ERROR;
        }

    m_handler->_OnDeleted (*GetDgnProject(), m_elementId);
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Walk through all elements of a deleted element and mark each component as not-deleted.
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementRef::UnDeleteElement()
    {
    ClearDeletedRef();
    SetDirtyFlags(DIRTY_Both);
    }

/*---------------------------------------------------------------------------------**//**
* undelete a deleted element (undeletes components as well, if complex).
* @bsimethod                                                    KeithBentley    03/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PersistentElementRef::_UndeleteElement()
    {
    if (m_dgnModel->IsReadOnly())
        return  DGNMODEL_STATUS_ReadOnly;

    if (!IsDeleted())
        return  DGNMODEL_STATUS_BadRequest;

    UnDeleteElement();

    GraphicElementRefList* graphics = m_dgnModel->GetGraphicElementsP();
    if (NULL != graphics)
        graphics->InsertRangeElement (this, true);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* add this element's ID into its cache's id tree. If the ID already exists, return DGNMODEL_STATUS_IdExists.
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PersistentElementRefList::RegisterId (PersistentElementRefR elRef)  
    {
    ElementId  id = elRef.GetElementId();
    if (!id.IsValid())
        {
        BeAssert (false);
        return ERROR;
        }

    auto pair = m_ownedElems.Insert(id,&elRef);
    return pair.second ? SUCCESS : DGNMODEL_STATUS_IdExists;
    }

/*---------------------------------------------------------------------------------**//**
* "canceled" means that we're abandoning a rolled-back transaction. In that case, remove it from the list even if the list is filled.
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void PersistentElementRefList::_OnElementDeletedFromDb (PersistentElementRefR elRef, bool canceled)
    {
    // if this list is marked as "filled", then we keep the deleted elements in the list so they 
    // will remain owned by the list if the delete is subsequently undone.
    if (!canceled && m_wasFilled)
        return;

    ElementId  id = elRef.GetElementId();
    if (!id.IsValid())
        {
        BeAssert (false);
        return;
        }

    m_ownedElems.erase(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void GraphicElementRefList::_OnElementDeletedFromDb (PersistentElementRefR elRef, bool canceled) 
    {
    T_Super::_OnElementDeletedFromDb (elRef, canceled);

    // if this is a cancel, then the element was dropped from the range index when it was deleted.
    if (canceled || NULL == m_rangeIndex)
        return;

    RemoveRangeElement (elRef);
    }

/*---------------------------------------------------------------------------------**//**
* @return   true if this element can be written out to the file. Returns false for deleted elements.
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
bool PersistentElementRef::CanBeSavedToFile()
    {
    return !IsDeleted() && (NULL != GetUnstableMSElementCP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementRef::ForceElemChanged (bool qvCacheCleared, ElemRefChangeReason reason)
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
bool ElementRef::_SetQvElem (QvElem* qvElem, UInt32 index)
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
T_QvElemSet* ElementRef::GetQvElems (bool createIfNotPresent)
    {
    static ElementRefAppData::Key s_qvElemsKey;
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
void ElementRef::SetInSelectionSet (bool yesNo)
    {
    m_flags.inSelectionSet = yesNo;
    SetHilited (yesNo ? HILITED_Normal : HILITED_None);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementRef::IsInSelectionSet() const
    {
    return (m_flags.inSelectionSet); // i don't know why we used to test this too: && (HILITED_None != IsHilited()));
    }

/*---------------------------------------------------------------------------------**//**
* @return   number of bytes actually copied
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ElementRef::_GetElement (DgnElementP out, size_t outSize) const
    {
    size_t size = GetMemorySize();
    if (size > outSize)
        size = outSize;

    memcpy (out, GetUnstableMSElementCP(), size);
    return  size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementRef::DeallocateRef(DgnElementPool& pool, bool fileUnloading)
    {
    ClearAllAppData (pool.GetHeapZone(), fileUnloading);

    FREE_AND_CLEAR (m_elm);

    if (!fileUnloading)  // if the file is unloading, the pool is about to become useless. Don't bother to keep it correct.
        pool.ReturnedMemory (m_allocSize);

    delete this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult XAttributeCollection::PrepareQuery() const
    {
    if (0 == m_elRef)
        return  BE_SQLITE_ERROR;

    if (!m_sql.IsValid())
        {
        DgnProjectP dgnFile = m_elRef ? m_elRef->GetDgnProject() : NULL;
        if (NULL == dgnFile)
            return BE_SQLITE_ERROR;

        Utf8CP sql = "SELECT rowid,Size,HandlerId,Id,Flags FROM " DGN_TABLE_ElmXAtt " WHERE ElementId=?";

        Utf8String  withHandlerId;
        if (m_searchId.IsValid())
            {
            withHandlerId.assign (sql);
            withHandlerId += " AND HandlerId=?";
            sql = withHandlerId.c_str();
            }

        DbResult result = dgnFile->GetCachedStatement (m_sql, sql);
        if (BE_SQLITE_OK != result)
            {
            BeAssert (false);
            return  result;
            }
        }
    else
        {
        m_sql->Reset();
        }

    m_sql->ClearBindings();
    m_sql->BindId(1, m_elRef->GetElementId());
    if (m_searchId.IsValid())
        m_sql->BindInt(2, m_searchId.GetId());

    return  BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XAttributeCollection::Reset (ElementRefP elRef, XAttributeHandlerId searchId)
    {
    m_sql = 0;
    m_elRef = elRef;
    m_searchId = searchId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeCollection::Entry XAttributeCollection::begin() const
    {
    return (BE_SQLITE_OK == PrepareQuery()) ? Entry (m_sql.get(), m_elRef) : Entry();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XAttributeCollection::Entry::Step()
    {
    if (NULL == m_sql)
        return;

    Invalidate();

    DbResult result = m_sql->Step();
    if (BE_SQLITE_ROW != result)
        {
        BeAssert (result == BE_SQLITE_DONE);
        return;
        }

    m_rowid = m_sql->GetValueInt64(0);
    m_size  = m_sql->GetValueInt(1);
    m_handlerId = XAttributeHandlerId((UInt32)m_sql->GetValueInt(2));
    m_id = m_sql->GetValueInt(3);
    m_flags = DgnModels::XAttributeFlags(m_sql->GetValueInt(4));
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/10
//=======================================================================================
struct XAttributeFullId
    {
    XAttributeHandlerId  m_handlerId;
    UInt32               m_id;

    XAttributeFullId (XAttributeHandlerId handlerId, UInt32 id) {m_handlerId=handlerId; m_id=id;}
    bool operator==(XAttributeFullId const& other) const {return (m_handlerId == other.m_handlerId && m_id == other.m_id);}
    bool operator!=(XAttributeFullId const& other) const {return !(*this == other);}
    bool operator< (XAttributeFullId const& other) const {return (m_handlerId < other.m_handlerId) || ((m_handlerId == other.m_handlerId) && (m_id < other.m_id));}
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/12
//=======================================================================================
struct XAttrSet : ElementRefAppData
{
    struct  Entry
        {
        Entry*              m_next;
        mutable void*       m_data;
        Int64               m_rowid;
        XAttributeFullId    m_fullId;
        UInt32              m_size;
        DgnModels::XAttributeFlags     m_flags;

        void Init (XAttributeFullId fullId, Int64 rowid, DgnModels::XAttributeFlags flags, UInt32 size, Entry* next) {m_fullId=fullId; m_rowid=rowid; m_flags=flags; m_next=next; m_size=size; m_data = NULL;}
        void Clear(DgnElementPool& pool) {if (NULL != m_data) {free(m_data); pool.ReturnedMemory(m_size);}}
        void AssignValue (DgnElementPool& pool, void* data) 
            {
            BeAssert (NULL == m_data); 
            BeAssert (NULL != data);

            pool.AllocatedMemory(m_size);
            m_data=data;
            }
        };

private:
    DgnElementPool&  m_pool;
    Entry*           m_entry;
    virtual WCharCP _GetName() override {return L"XAttrSet";}
    virtual void _OnCleanup (ElementRefP host, bool unloadingCache, HeapZone& zone) override {FreeAll(); if (!unloadingCache) zone.Free (this, sizeof *this);}
    virtual bool _OnElemChanged (ElementRefP host, bool qvCacheDeleted, ElemRefChangeReason) override {FreeAll(); return true;}
    HeapZone& GetHeapZone() {return m_pool.GetHeapZone();}

public:
    DgnElementPool& ElementPool() {return m_pool;}
    void FreeAll();
    void Add (XAttributeHandlerId handlerId, UInt32 id, Int64 rowid, DgnModels::XAttributeFlags flags, UInt32 size);
    StatusInt Drop (XAttributeHandlerId handlerId, UInt32 id);
    Entry* Find (XAttributeHandlerId handlerId, UInt32 id);
    XAttrSet (DgnElementPool& pool) : m_pool(pool) {m_entry = NULL;}
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void XAttrSet::FreeAll()
    {
    for (Entry* thisEntry=m_entry, *next; thisEntry; thisEntry=next)
        {
        next = thisEntry->m_next;
        thisEntry->Clear(m_pool);
        GetHeapZone().Free (thisEntry, sizeof(Entry));
        }
    m_entry = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/06
+---------------+---------------+---------------+---------------+---------------+------*/
void XAttrSet::Add (XAttributeHandlerId handlerId, UInt32 id, Int64 rowid, DgnModels::XAttributeFlags flags, UInt32 size)
    {
    XAttributeFullId fullId (handlerId, id);

    Entry* prevEntry = NULL;
    Entry* nextEntry = m_entry;

    for (; nextEntry; prevEntry=nextEntry, nextEntry=nextEntry->m_next)
        {
        if (nextEntry->m_fullId < fullId)
            continue;

        if (nextEntry->m_fullId == fullId)
            {
            BeAssert (false);
            return;
            }

        break;
        }

    Entry* newEntry = (Entry*) GetHeapZone().Alloc (sizeof(Entry));
    newEntry->Init (fullId, rowid, flags, size, nextEntry);

    if (prevEntry)
        prevEntry->m_next = newEntry;
    else
        m_entry = newEntry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XAttrSet::Drop (XAttributeHandlerId handlerId, UInt32 id)
    {
    XAttributeFullId fullId (handlerId, id);
    for (Entry* prev=NULL, *thisEntry=m_entry; thisEntry; prev=thisEntry, thisEntry=thisEntry->m_next)
        {
        if (thisEntry->m_fullId < fullId)
            continue;

        if (thisEntry->m_fullId != fullId)
            break;

        if (prev)
            prev->m_next = thisEntry->m_next;
        else
            m_entry = thisEntry->m_next;

        thisEntry->Clear(m_pool);
        GetHeapZone().Free (thisEntry, sizeof(Entry));
        return  SUCCESS;
        }

    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/06
+---------------+---------------+---------------+---------------+---------------+------*/
XAttrSet::Entry* XAttrSet::Find (XAttributeHandlerId handlerId, UInt32 id)
    {
    XAttributeFullId fullId (handlerId, id);
    for (Entry* thisEntry = m_entry; thisEntry; thisEntry = thisEntry->m_next)
        {
        if (thisEntry->m_fullId < fullId)
            continue;

        return thisEntry->m_fullId == fullId ? thisEntry : NULL;
        }

    return  NULL;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandle::XAttributeHandle() : m_handlerId(0,0), m_flags(DgnModels::XAttributeFlags::Compress::No)
    {
    m_cached = false; 
    m_data = NULL; 
    m_size = 0;
    m_id = 0;
    m_rowid = -1;
    m_elRef = 0;
    m_project = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandle::XAttributeHandle (ElementRefP elRef, XAttributeHandlerId handlerId, UInt32 id, Int64 rowid, UInt32 size, DgnModels::XAttributeFlags flags) : m_handlerId(handlerId), m_flags(flags)
    {
    m_cached = false; 
    m_data = NULL;
    m_size = size;
    m_id    = id;
    m_elRef = elRef;
    m_rowid = rowid;
    if (NULL == elRef)
        {
        m_project = NULL;
        return;
        }
    m_project = elRef->GetDgnProject();
    m_elementId = elRef->GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandle::XAttributeHandle (ElementRefP elRef, XAttributeHandlerId handlerId, UInt32 id) : m_handlerId(handlerId)
    {
    m_cached = false; 
    m_data = NULL;
    m_size = 0;
    m_id    = id;
    m_elRef = elRef;
    m_rowid = -1;

    if (NULL == elRef)
        {
        m_project = NULL;
        return;
        }

    if (id == XAttributeHandle::MATCH_ANY_ID)
        {
        *this = XAttributeCollection(elRef, handlerId).begin();
        return;
        }

    m_project = elRef->GetDgnProject();
    m_elementId = elRef->GetElementId();
    DoSelect();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandle::XAttributeHandle (ElementId elId, DgnProjectR dgnFile, XAttributeHandlerId handlerId, UInt32 id) : m_handlerId(handlerId)
    {
    m_cached = false; 
    m_data = NULL;
    m_size = 0;
    m_id    = id;
    m_elRef = NULL;
    m_rowid = -1;
    m_project = &dgnFile;
    m_elementId = elId;
    DoSelect();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XAttributeHandle::FreeData() const
    {
    if (m_cached)
        {
        m_data = NULL;
        m_cached = false; 
        return;
        }

    FREE_AND_CLEAR (m_data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XAttributeHandle::Copy(XAttributeHandleCR other)
    {
    memcpy (this, &other, sizeof(*this));
    if (!m_cached)
        m_data = NULL;   // copy all of the members, but strip the data pointer, if not cached (since that means this handle owns it).
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandle::XAttributeHandle (XAttributeHandleCR other)
    {
    Copy(other);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandleR XAttributeHandle::operator= (XAttributeHandleCR rhs)
    {
    if (this != &rhs)
        {
        Invalidate();
        Copy (rhs);
        }
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* find or create an xattribute set for an elementref
* @bsimethod                                    Keith.Bentley                   12/06
+---------------+---------------+---------------+---------------+---------------+------*/
static XAttrSet* getXattrSet (ElementRefP elRef, bool createIfNotPresent)
    {
    if (NULL == elRef) // you can use XAttributeHandle without an ElementRef, so this can happen
        return  NULL;

    static ElementRefAppData::Key s_xattrsKey;
    XAttrSet* xattrs = (XAttrSet*) elRef->FindAppData (s_xattrsKey);
    if (xattrs)
        return  xattrs;

    if (!createIfNotPresent)
        return  NULL;

    HeapZone& zone = elRef->GetHeapZone();
    xattrs = new ((XAttrSet*) zone.Alloc (sizeof(XAttrSet))) XAttrSet (elRef->GetDgnProject()->Models().ElementPool());

    elRef->AddAppData (s_xattrsKey, xattrs, zone);
    return  xattrs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XAttributeHandle::DoSelect()
    {
    Invalidate();

    if (0 == m_project || !m_handlerId.IsValid() || XAttributeHandle::MATCH_ANY_ID==m_id)
        return;

    // before reading it from the db, see if this xattribute is cached on the ElementRef
    XAttrSet* cachedXatts = getXattrSet (m_elRef, true);
    XAttrSet::Entry* entry = cachedXatts ? cachedXatts->Find (m_handlerId, m_id) : NULL;
    if (entry)
        {
        if (entry->m_rowid < 0)         // Previous miss.
            return;

        m_rowid = entry->m_rowid;
        m_size  = entry->m_size;
        m_data  = entry->m_data;
        m_flags = entry->m_flags;
        if (m_data)
            m_cached = true;

        return; // we have everything we need.
        }

    HighPriorityOperationBlock highPriority;  //  see comments in BeSQLite.h
    CachedStatementPtr queryXAttStmt;
    m_project->GetCachedStatement(queryXAttStmt, "SELECT rowid,Size,Flags FROM " DGN_TABLE_ElmXAtt " WHERE ElementId=? AND HandlerId=? AND Id=?");

    queryXAttStmt->BindId(1, m_elementId);
    queryXAttStmt->BindInt(2, m_handlerId.GetId());
    queryXAttStmt->BindInt(3, m_id);

    DbResult stepResult = queryXAttStmt->Step();

    if (BE_SQLITE_ROW != stepResult)
        {
        if (cachedXatts) // if we have an elementref, save this xattr "miss" on it so we don't attempt the lookup again.
            cachedXatts->Add(m_handlerId, m_id, -1, DgnModels::XAttributeFlags(), 0);

        return;
        }

    m_rowid = queryXAttStmt->GetValueInt64(0);
    m_size  = queryXAttStmt->GetValueInt(1);
    m_flags = DgnModels::XAttributeFlags(queryXAttStmt->GetValueInt(2));

    if (cachedXatts) // if we have an elementref, save this xattr on it
        cachedXatts->Add(m_handlerId, m_id, m_rowid, m_flags, m_size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void XAttributeHandle::ClearElemRefCache() const
    {
    XAttrSet* cachedXatts = getXattrSet (m_elRef, false);
    if (cachedXatts)
        cachedXatts->Drop (m_handlerId, m_id);

    FreeData();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementRef::ClearXAttCache (XAttributeHandlerId handlerId, UInt32 id)
    {
    XAttrSet* cachedXatts = getXattrSet (this, false);
    if (cachedXatts)
        cachedXatts->Drop (handlerId, id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult XAttributeHandle::ReadData (void* data, UInt32 size) const
    {
    if (NULL == m_project || m_rowid==-1 || size>m_size)
        return  BE_SQLITE_ERROR;

    if ((UInt32) DgnModels::XAttributeFlags::Compress::Snappy == m_flags.m_compress)
        {
        SnappyFromBlob snapper;

        ZipErrors stat;
        UInt32 actuallyRead;
            {
            //  See comments in BeSQLite.h regarding a HighPriorityOperationBlock
            HighPriorityOperationBlock highPriority;

            stat = snapper.Init (*m_project, DGN_TABLE_ElmXAtt, "Data", m_rowid);
            BeAssert (stat==ZIP_SUCCESS);

            stat = snapper._Read ((byte*) data, size, actuallyRead);
            BeAssert (stat==ZIP_SUCCESS);
            BeAssert (actuallyRead==size);
            }

        return  (actuallyRead==size) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
        }

    BlobIO blob;
    if (BE_SQLITE_OK != blob.Open (*m_project, DGN_TABLE_ElmXAtt, "Data", m_rowid, false))
        return  BE_SQLITE_ERROR;

    //  See comments in BeSQLite.h regarding a HighPriorityOperationBlock
    HighPriorityOperationBlock highPriority;
    return blob.Read(data, size, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult XAttributeHandle::ModifyData (void const* data, UInt32 offset, UInt32 size, DgnModels::XAttributeFlags flags) 
    {
    if (NULL == m_project || m_rowid==-1 || (size+offset)>m_size)
        return  BE_SQLITE_ERROR;

    if (((UInt32)DgnModels::XAttributeFlags::Compress::No == flags.m_compress) && ((UInt32)DgnModels::XAttributeFlags::Compress::No == m_flags.m_compress))
        {
        ClearElemRefCache();

        BlobIO blob;
        if (BE_SQLITE_OK != blob.Open (*m_project, DGN_TABLE_ElmXAtt, "Data", m_rowid, true))
            return  BE_SQLITE_ERROR;

        return blob.Write(data, size, offset);
        }

    // we've been asked to modify a piece of a compressed XAttribute. We will just read the whole thing into memory,
    // then modify it in place and re-write the whole thing.
    if (NULL == PeekData())
        return  BE_SQLITE_ERROR;

    memcpy ((byte*)m_data+offset, data, size);
    return ReplaceData (m_data, m_size, flags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult XAttributeHandle::ReplaceData (void const* xaData, UInt32 xaSize, DgnModels::XAttributeFlags flags) 
    {
    if (NULL == m_project || m_rowid==-1)
        return  BE_SQLITE_ERROR;

    // update the cached version.
    if (NULL != m_data && xaData != m_data)
        memcpy ((byte*)m_data, xaData, xaSize);

    CachedStatementPtr updateStmt;
    m_project->GetCachedStatement(updateStmt, "UPDATE " DGN_TABLE_ElmXAtt " SET Size=?,Flags=?,Data=? WHERE rowid=?");

    m_size = xaSize;
    updateStmt->BindInt (1, xaSize);
    updateStmt->BindInt64 (4, m_rowid);

    bool multiChunkBlob = false;
    SnappyToBlob snapper;

    if (xaSize < 40)
        flags.m_compress = (UInt32)DgnModels::XAttributeFlags::Compress::No;

    if (flags.m_compress == (UInt32)DgnModels::XAttributeFlags::Compress::Snappy)
        {
        snapper.DoSnappy((ByteCP) xaData, xaSize);
        UInt32 compressedSize = snapper.GetCompressedSize();
        if (compressedSize >= xaSize)
           flags.m_compress = (UInt32)DgnModels::XAttributeFlags::Compress::No;
        else
            {
            updateStmt->BindInt (2, flags.GetUInt32());

            if (1 == snapper.GetCurrChunk())
                updateStmt->BindBlob (3, snapper.GetChunkData(0), compressedSize, Statement::MAKE_COPY_No);
            else
                {
                updateStmt->BindZeroBlob (3, compressedSize);
                multiChunkBlob = true;
                }
            }
        }

    if (flags.m_compress == (UInt32)DgnModels::XAttributeFlags::Compress::No)
        {
        updateStmt->BindBlob(3, xaData, xaSize, Statement::MAKE_COPY_No);
        updateStmt->BindInt (2, 0);
        }

    DbResult rc = updateStmt->Step();
    BeAssert (rc==BE_SQLITE_DONE);

    if (rc == BE_SQLITE_DONE && multiChunkBlob)
        {
        StatusInt status = snapper.SaveToRow (*m_project, DGN_TABLE_ElmXAtt, "Data", m_rowid);
        if (SUCCESS != status)
            { BeAssert(false); }
        }

    return rc;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
bool XAttributeHandle::IsXAttributeCached (ElementRefP elRef, XAttributeHandlerId handlerId, UInt32 id)
    {
    if (NULL == elRef)
        return false;

    XAttrSet* cachedXatts = getXattrSet (elRef, false);
    XAttrSet::Entry* entry = cachedXatts ? cachedXatts->Find (handlerId, id) : NULL;
    return entry && entry->m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/04
+---------------+---------------+---------------+---------------+---------------+------*/
void const* XAttributeHandle::PeekData() const
    {
    if (NULL==m_data && m_size>0 && m_rowid != -1)
        {
        XAttrSet* cachedXatts = getXattrSet (m_elRef, false);
        XAttrSet::Entry* entry = cachedXatts ? cachedXatts->Find (m_handlerId, m_id) : NULL;
        if (entry && entry->m_data)
            {
            m_data = entry->m_data;
            m_cached = true;
            }
        else
            {
            m_data = (void*) malloc(m_size);
            if (BE_SQLITE_OK != ReadData (m_data, m_size))
                {
                BeAssert (false);
                FreeData();
                }
            else if (entry)
                {
                entry->AssignValue (cachedXatts->ElementPool(), m_data);
                m_cached = true;
                }
            }
        }

    return  m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult XAttributeHandle::DeleteFromFile()
    {
    if (!IsValid() || NULL == m_project)
        return  BE_SQLITE_ERROR;

    Int64 rowid = m_rowid; // save this, Invalidate will clear it.
    ClearElemRefCache();
    Invalidate();
    return m_project->Models().DeleteXAttribute (rowid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/04
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnModels::InsertXAttribute (Int64& rowid, ElementId elementId, XAttributeHandlerId handlerId, UInt32& xAttId, UInt32 xaSize, void const* xaData, DgnModels::XAttributeFlags& flags)
    {
    rowid = -1;

    if (XAttributeHandle::INVALID_XATTR_ID == xAttId)
        {
        // find the current xattribute with the highest id for this element/handlerId
        CachedStatementPtr stmt;
        DbResult rc = m_project.GetCachedStatement(stmt, "SELECT Id FROM " DGN_TABLE_ElmXAtt " WHERE ElementId=? AND HandlerId=? ORDER BY Id DESC LIMIT 1");
        if (BE_SQLITE_OK != rc)
            return  rc;

        stmt->BindId (1,elementId);
        stmt->BindInt (2,handlerId.GetId());
        rc = stmt->Step();
        xAttId = (rc==BE_SQLITE_ROW) ? stmt->GetValueInt(0)+1 : 0;
        }

    CachedStatementPtr insertXAttStmt;
    DbResult rc = m_project.GetCachedStatement(insertXAttStmt, "INSERT INTO " DGN_TABLE_ElmXAtt " (ElementId,HandlerId,Id,Size,Data,Flags) VALUES (?,?,?,?,?,?)");
    if (BE_SQLITE_OK != rc)
        return  rc;

    insertXAttStmt->BindId (1,elementId);
    insertXAttStmt->BindInt (2,handlerId.GetId());
    insertXAttStmt->BindInt (3,xAttId);
    insertXAttStmt->BindInt (4,xaSize);

    bool multiChunkBlob = false;
    SnappyToBlob snapper;

    if (xaSize < 40)
        flags.m_compress = (UInt32)DgnModels::XAttributeFlags::Compress::No;

    if (flags.m_compress == (UInt32)XAttributeFlags::Compress::Snappy)
        {
        snapper.DoSnappy((ByteCP) xaData, xaSize);
        UInt32 compressedSize = snapper.GetCompressedSize();
        if (compressedSize >= xaSize)
           flags.m_compress = (UInt32)XAttributeFlags::Compress::No;
        else
            {
            insertXAttStmt->BindInt(6, flags.GetUInt32());

            if (1 == snapper.GetCurrChunk())
                insertXAttStmt->BindBlob (5, snapper.GetChunkData(0), compressedSize, Statement::MAKE_COPY_No);
            else
                {
                insertXAttStmt->BindZeroBlob (5, compressedSize);
                multiChunkBlob = true;
                }
            }
        }

    if (flags.m_compress == (UInt32)XAttributeFlags::Compress::No)
        {
        insertXAttStmt->BindBlob(5, xaData, xaSize, Statement::MAKE_COPY_No);
        insertXAttStmt->BindInt(6, 0);
        }

    rc = insertXAttStmt->Step();
    BeAssert (rc==BE_SQLITE_DONE);

    if (rc == BE_SQLITE_DONE)
        {
        rowid = m_project.GetLastInsertRowId();
        if (multiChunkBlob)
            {
            StatusInt status = snapper.SaveToRow (m_project, DGN_TABLE_ElmXAtt, "Data", rowid);
            if (SUCCESS != status)
                { BeAssert(false); }
            }
        }

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnModels::DeleteXAttribute (Int64 rowid)
    {
    if (rowid==-1)
        return  BE_SQLITE_ERROR;

    CachedStatementPtr deleteXAttStmt;
    DbResult status = m_project.GetCachedStatement(deleteXAttStmt, "DELETE FROM " DGN_TABLE_ElmXAtt " WHERE rowid=?");
    if (BE_SQLITE_OK != status)
        return  status;

    deleteXAttStmt->BindInt64 (1, rowid);
    return deleteXAttStmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRef::ElementRef (DgnModelP dgnModel)
    {
    memset (&m_flags, 0, sizeof(m_flags));
    m_handler  = NULL;
    m_appData  = NULL;
    m_dgnModel = dgnModel;
    m_allocSize = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/06
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem* ElementRef::GetQvElem (UInt32 id)
    {
    T_QvElemSet* qvElems = GetQvElems(false);
    return qvElems ? qvElems->Find (id) : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElementRef::IsSameSizeAndType (MSElementDescrCR descr)
    {
    if ((descr.Element().Size() != GetMemorySize()) || (descr.GetElementHandler() != GetHandler()))
        return  false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementRef::TransferIdToDescr (MSElementDescrR descr)
    {
    descr.ElementR().SetElementId(m_elementId);
    descr.SetElementRef(this);    // set the ElementRefP in his descriptor
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementRef::_GetHeaderFieldsFrom(MSElementDescrCR descr)
    {
    m_handler    = descr.GetElementHandler();
    BeAssert (NULL != m_handler);
    m_itemId  = descr.GetItemId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/00
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementRef::ModifyFromDescr (DgnModelR dgnModel, MSElementDescrR newDescr)
    {
    if (NULL != m_elm)
        dgnModel.ElementChanged (*this, ELEMREF_CHANGE_REASON_Modify);

    _GetHeaderFieldsFrom(newDescr);

    // copy the new element into this element's data
    SaveElement(&newDescr.Element());
    _SetDirtyFlags(ElementRef::DIRTY_ElemData);
    }

static ElementRefAppData::Key s_symbolStampRefPinAppDataKey;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
void DgnSymbolStampPinner::_OnCleanup (ElementRefP host, bool unloadingCache, HeapZone& zone)
    {
    this->~DgnSymbolStampPinner();
    if (!unloadingCache)
        zone.Free (this, sizeof *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
XGraphicsSymbolStampP DgnSymbolStampPinner::GetAndPin (ElementRefP appDataElm, DgnStampId stampToPin)
    {
    DgnSymbolStampPinner* appData = (DgnSymbolStampPinner*) appDataElm->FindAppData (s_symbolStampRefPinAppDataKey);

    if (NULL == appData)
        {
        HeapZoneR zone = appDataElm->GetHeapZone();

        appData = new ((DgnSymbolStampPinner*) zone.Alloc (sizeof (DgnSymbolStampPinner))) DgnSymbolStampPinner ();

        if (SUCCESS != appDataElm->AddAppData (s_symbolStampRefPinAppDataKey, appData, zone))
            {
            zone.Free (appData, sizeof (*appData));
            appData = NULL;
            return NULL;
            }
        }

    auto iter = appData->m_pinnedSymbols.find (stampToPin);
    if (iter != appData->m_pinnedSymbols.end())
        return iter->second.get();

    XGraphicsSymbolStampPtr stampPtr = XGraphicsSymbolStamp::Get(*appDataElm->GetDgnProject(), stampToPin);

    if (stampPtr.IsValid())
        appData->m_pinnedSymbols.insert (make_bpair (stampToPin, stampPtr));

    return stampPtr.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
void DgnSymbolStampPinner::Release (ElementRefP appDataElm, DgnStampId stampToRelease)
    {
    DgnSymbolStampPinner* appData = (DgnSymbolStampPinner*) appDataElm->FindAppData (s_symbolStampRefPinAppDataKey);
    if (!appData)
        return;

    appData->m_pinnedSymbols.erase (stampToRelease);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
void DgnSymbolStampPinner::ReleaseAll (ElementRefP appDataElm)
    {
    DgnSymbolStampPinner* appData = (DgnSymbolStampPinner*) appDataElm->FindAppData (s_symbolStampRefPinAppDataKey);
    if (!appData)
        return;

    appData->m_pinnedSymbols.clear();
    }
