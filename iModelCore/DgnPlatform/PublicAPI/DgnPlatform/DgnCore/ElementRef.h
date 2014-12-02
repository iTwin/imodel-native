/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ElementRef.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnElements.h"
#include "DgnCore.h"

/** @addtogroup ElementRefGroup

Classes for working with elements in memory.

Elements must be loaded from @ref DgnProjectGroup and cached in memory before they can be accessed.
Element are loaded and cached using @ref DgnFileGroup or using QueryModel's.

*/

BENTLEY_API_TYPEDEFS (HeapZone);
DGNPLATFORM_TYPEDEFS (DgnElementRef)
DGNPLATFORM_TYPEDEFS (PersistentElementRef)

//__PUBLISH_SECTION_END__
#include <Bentley/BeAssert.h>

DGNPLATFORM_TYPEDEFS (PersistentElementRefList)
DGNPLATFORM_TYPEDEFS (ElementListHandler)
DGNPLATFORM_CLASS_TYPEDEFS(Handler);

//__PUBLISH_SECTION_START__

enum ElementRefType
{
    ELEMENT_REF_TYPE_Persistent     = 1,
};

enum ElementHiliteState
{
    HILITED_None                = 0,
    HILITED_Normal              = 1,
    HILITED_Bold                = 2,
    HILITED_Dashed              = 3,
    HILITED_Background          = 4,
};

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                     Keith.Bentley   06/08
//=======================================================================================
enum ElemRefChangeReason
{
    ELEMREF_CHANGE_REASON_Delete        = 1,
    ELEMREF_CHANGE_REASON_Modify        = 2,
    ELEMREF_CHANGE_REASON_ClearQVData   = 5,
};

//=======================================================================================
//! Create a subclass of this to store non-persistent information on an element.
//! @bsiclass
//=======================================================================================
struct ElementRefAppData
{
    virtual ~ElementRefAppData() {}

    //=======================================================================================
    //! A unique identifier for this type of ElementRefAppData. A static instance of
    //! ElementRefAppData::Key should be declared to hold the identifier.
    //! @bsiclass
    //=======================================================================================
    struct Key : BeSQLite::AppDataKey {};

    //! Return a name for this type of app data.
    //! @remarks Strictly for debugging, does not need to be implemented or localized.
    //! @return The name string or NULL.
    virtual WCharCP _GetName () {return NULL;}

    //! Called to clean up owned resources and delete the app data.
    //! @param[in]  host            ElementRefP that app data was added to.
    //! @param[in]  unloadingModel  If DgnModel containing host is being unloaded.
    //! @param[in]  zone            HeapZone for the ElementRefP holding the app data.
    //! @remarks If the app data was allocated using ElementRefP::GetHeapZone(), there is
    //! nothing to free when unloadingCache is true as the entire heap zone will be freed with
    //! the cache. If unloadingCache is false, call HeapZone::Free. If a heap zone was not used, call delete/release/free as appropriate.
    //! @note If the appData was allocated using placement new with ElementRefP::GetHeapZone(), the appData's
    //! destructor should be manually called in this method.
    virtual void _OnCleanup (ElementRefP host, bool unloadingModel, HeapZoneR zone) = 0;

    //! Called to allow app data to react to changes to the persistent element it was added to.
    //! @param[in]  host            ElementRefP that app data was added to.
    //! @param[in]  qvCacheDeleted  Specific to app data used to cache the display representation.
    //!                             of the element. Clearing the qvCache invalidates QvElems stored in app data.
    //! @param[in]  reason          Why _OnElementChanged is being called.
    //! @return true to drop this app data entry from the element.
    virtual bool _OnElemChanged (ElementRefP host, bool qvCacheDeleted, ElemRefChangeReason reason) {return false;}
};

//__PUBLISH_SECTION_END__

//=======================================================================================
//! @bsiclass                                                    JohnGooding     04/14
//=======================================================================================
struct DgnSymbolStampPinner : ElementRefAppData
{
private:
    bmap <DgnStampId, XGraphicsSymbolStampPtr> m_pinnedSymbols;

    DgnSymbolStampPinner() {}

protected:
//! @cond DONTINCLUDEINDOC
    virtual ~DgnSymbolStampPinner () {}
    virtual WCharCP _GetName() override { return L"DgnStampPinner"; }
    virtual void _OnCleanup (ElementRefP host, bool unloadingModel, HeapZoneR zone) override;
//! @endcond

public:
    static XGraphicsSymbolStampP GetAndPin (ElementRefP elementWithPin, DgnStampId);
    static void Release (ElementRefP elementWithPin, DgnStampId stampToRelease);
    static void ReleaseAll (ElementRefP elementWithPin);
};

struct DgnModel;
struct DerivedElementRangeUtils;
struct ElementTableHandler;

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      04/2008
+===============+===============+===============+===============+===============+======*/
struct DerivedElementRange : private ElementRefAppData
{
private:
    friend struct PersistentElementRef;

protected:
    DRange3d   m_range;

//! @cond DONTINCLUDEINDOC
    virtual void _OnCleanup (ElementRefP host, bool unloadingModel, HeapZone& zone) override {_OnRemoveDerivedRange (*(PersistentElementRef*)host, unloadingModel, zone);}
//! @endcond

    //! The host element is being deallocated or the DerivedElementRange is being removed from the host.
    //! This DerivedElementRange object should deallocate itself.
    virtual void _OnRemoveDerivedRange (PersistentElementRef const&, bool, HeapZone&) = 0;

    //! The host element has been modified. This DerivedElementRange object should update itself.
    virtual void _UpdateDerivedRange (PersistentElementRef&) = 0;

protected:
    //! Return the scan range that should be used for the specified host element.
    DRange3dCR GetDerivedRange () {return m_range;}

    //! Call this before adding or changing a derived range
    DGNPLATFORM_EXPORT static void OnDerivedRangeChangePre (PersistentElementRef&);

    //! Call this after adding or changing a derived range
    DGNPLATFORM_EXPORT static void OnDerivedRangeChangePost (PersistentElementRef&);
};

template <class _QvKey> struct QvElemSet;
//=======================================================================================
//! @bsiclass
//=======================================================================================
struct QvKey32
{
private:
    UInt32  m_key;

public:
    inline bool LessThan (QvKey32 const& other) const {return m_key < other.m_key;}
    inline bool Equal (QvKey32 const& other) const    {return m_key == other.m_key;}
    void DeleteQvElem (QvElem* qvElem);
    QvKey32 (UInt32 key) {m_key = key;} // allow non-explicit!
};

typedef QvElemSet<QvKey32> T_QvElemSet;

//__PUBLISH_SECTION_START__
//=======================================================================================
//! An instance of a DgnElement.
//! @bsiclass                                                     KeithBentley    10/00
//=======================================================================================
struct ElementRef
    {
//__PUBLISH_SECTION_END__
public:
    enum DirtyFlags {DIRTY_ElemData = 1<<0, DIRTY_XAttr = 1<<1, DIRTY_Both = (DIRTY_ElemData|DIRTY_XAttr)};
    enum ComputedRangeFlags {COMPUTED_RANGE_FLAG_None=0, COMPUTED_RANGE_FLAG_Derived = 1, COMPUTED_RANGE_FLAG_Dynamic = 2, COMPUTED_RANGE_FLAG_Unused_ = 3};
    friend struct PersistentElementRefList;
    friend struct DbElementReader;
    friend struct ITxn;

protected:
    ElementId       m_elementId;
    DgnItemId       m_itemId;
    HandlerP        m_handler;
    DgnModelP       m_dgnModel;

    struct AppDataEntry
        {
        ElementRefAppData::Key const* m_key;
        ElementRefAppData*            m_obj;
        AppDataEntry*                 m_next;

        void Init (ElementRefAppData::Key const& key, ElementRefAppData* obj, AppDataEntry* next) {m_key = &key; m_obj = obj; m_next = next;}
        void ClearEntry (ElementRefP el, bool unloading, HeapZoneR zone) {if (NULL == m_obj) return; m_obj->_OnCleanup (el, unloading, zone); m_obj=NULL;}
        void SetEntry (ElementRefAppData* obj, ElementRefP el, HeapZoneR zone) {ClearEntry (el, false, zone); m_obj = obj;}
        };

    AppDataEntry*  m_appData;

    struct
        {
        UInt32     dirtyFlag:2;
        UInt32     deletedRef:1;
        UInt32     inSelectionSet:1;
        UInt32     failedAssoc:1;
        UInt32     hiliteState:3;
        UInt32     undisplayed:1;                  // don't display this element.
        UInt32     mark1:1;                        // used by applications
        UInt32     mark2:1;                        // used by applications
        UInt32     elFlags:4;                      // used by element type specific code
        UInt32     hasComputedRange:2;             // the element has no fixed range or has a computed range.
        } m_flags;

    UInt32 m_allocSize;

    void SetItemId(DgnItemId val) {m_itemId = val;}
    T_QvElemSet* GetQvElems (bool createIfNotPresent);
    AppDataEntry* FreeAppDataEntry (AppDataEntry* prev, AppDataEntry& thisEntry, HeapZoneR zone, bool modelUnloading);

    virtual QvCache* _GetMyQvCache () = 0;
    DGNPLATFORM_EXPORT virtual bool _SetQvElem (QvElem* qvElem, UInt32 index);
    virtual ElementRefType _GetRefType () = 0;
    virtual HeapZoneR _GetHeapZone () = 0;
    virtual DgnElementCP _GetElemPtrC () const = 0;
    DGNPLATFORM_EXPORT virtual size_t _GetElement (DgnElementP out, size_t outSize) const;
    virtual void _SetDirtyFlags (DirtyFlags flags) {m_flags.dirtyFlag |= flags;}
    virtual StatusInt _DeleteElement () {return ERROR;}
    virtual StatusInt _UndeleteElement () {return ERROR;}
    virtual UInt32 _AddRef() const = 0;
    virtual UInt32 _Release() const = 0;
    DGNPLATFORM_EXPORT virtual void _GetHeaderFieldsFrom(MSElementDescrCR);
    void ClearXAttCache (XAttributeHandlerId handlerId, UInt32 id);

public:
    DGNPLATFORM_EXPORT ElementRef(DgnModelP);
    virtual ~ElementRef () {}

    inline void ClearDirty () {m_flags.dirtyFlag = 0;}
    inline bool IsAnyDirty () const {return 0 != m_flags.dirtyFlag;}
    inline bool IsElemDirty ()const {return 0 != (DIRTY_ElemData & m_flags.dirtyFlag);}
    inline bool IsXAttrDirty () const{return 0 != (DIRTY_XAttr & m_flags.dirtyFlag);}
    inline void SetDeletedRef () {m_flags.deletedRef = true;}
    inline void ClearDeletedRef () {m_flags.deletedRef = false;}
    inline void SetFailedAssoc (bool yesNo) {m_flags.failedAssoc = yesNo;}
    inline void SetMark1 (bool yesNo) {if (m_flags.mark1==yesNo) return; m_flags.mark1 = yesNo;}
    inline void SetMark2 (bool yesNo) {if (m_flags.mark2==yesNo) return; m_flags.mark2 = yesNo;}
    inline void SetElFlags (int flags) {m_flags.elFlags = flags;}
    inline bool IsFailedAssoc () const {return true == m_flags.failedAssoc;}
    inline bool IsMarked1() const {return true == m_flags.mark1;}
    inline bool IsMarked2() const {return true == m_flags.mark2;}
    inline int GetElFlags() const {return m_flags.elFlags;}
    inline bool HasDerivedRange () const {return m_flags.hasComputedRange == COMPUTED_RANGE_FLAG_Derived;}
    inline bool IsDynamicRange () const {return m_flags.hasComputedRange == COMPUTED_RANGE_FLAG_Dynamic;}
    inline HandlerP GetHandler () const {return m_handler;}
    inline void SetHandler (HandlerP handler) {m_handler = handler;}
    inline void SetHilited (ElementHiliteState newState) {m_flags.hiliteState = newState;}

    DGNPLATFORM_EXPORT void SetInSelectionSet (bool yesNo);
    void SetDirtyFlags (DirtyFlags flags);

    DGNPLATFORM_EXPORT StatusInt UndeleteElement ();
    DGNPLATFORM_EXPORT QvElem* GetQvElem (UInt32 index);
    DGNPLATFORM_EXPORT bool SetQvElem (QvElem* qvElem, UInt32 index);
    DGNPLATFORM_EXPORT void ForceElemChanged (bool qvCacheCleared, ElemRefChangeReason);
    DGNPLATFORM_EXPORT void ForceElemChangedPost ();
    DGNPLATFORM_EXPORT void ClearAllAppData (HeapZoneR, bool cacheUnloading);
    DGNPLATFORM_EXPORT QvCache* GetMyQvCache ();
    DGNPLATFORM_EXPORT ElementRefType GetRefType ();

    //! Direct element I/O. To be called only by Itxn!
    //! Mark the element held by this ElementRef as deleted. The ElementRef remains in the DgnModel, but the element is marked as
    //! deleted and will be removed from the persistent file when the next save occurs.
    //! @param[in]  wholeComplex    If true, the entire element is deleted. If false, and if the element is a complex header, the header
    //!                             is deleted and the components are "dropped". This is reserved for MicroStation internals only.
    //! @param[in]  movingElemntRef If true, the element's data is being moved to a different ElementRef. This is reserved for MicroStation
    //!                             internals only.
    DGNPLATFORM_EXPORT StatusInt DeleteElement();

    // Get a const pointer to the element held by this ElementRef.
    //! @note For this pointer to be stable, you must call #PinElementData before calling this method.
    //! @note It is not valid or legal to cast away the const-ness of this pointer and use it to modify the element.
    //! Doing so will corrupt the DgnModel and possibly the DgnFile.
    DGNPLATFORM_EXPORT DgnElementCP GetUnstableMSElementCP() const;

    //! Get a const pointer to the 'ehdr' of the element held by this ElementRef.
    //! @see #GetUnstableMSElementCP.
    DgnElementHeaderCP GetElementHeaderCP () const {return GetUnstableMSElementCP();}

    //! Allocate an MSElementDescr to hold a copy of the element for this ElementRef.
    //! @param[out] ed the allocated MSElementDescr. Caller must free.
    //! @param[in] allowDeleted Read deleted elements? Normally, GetElementDescr will fail if this ElementRef was deleted.
    DGNPLATFORM_EXPORT MSElementDescrPtr GetElementDescr (bool allowDeleted=false);

    //! Get a copy of the data for this ElementRef.
    //! @param[out] el A Buffer to receive the element's data.
    //! @param[in] outSize the maximum number of bytes in \a el.
    //! @return the actual number of bytes copied to \a el.
    DGNPLATFORM_EXPORT size_t GetElement (DgnElementP el, size_t outSize) const;

    //! Determine whether this ElementRef is an element that once existed, but has been deleted.
    DGNPLATFORM_EXPORT bool IsDeleted () const;

    //! Get the size (in bytes) of the element held by this ElementRef.
    DGNPLATFORM_EXPORT size_t GetMemorySize () const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    /// @name ElementRefAppData Management
    //@{
    //! Get the HeapZone for the this ElementRef.
    DGNPLATFORM_EXPORT HeapZoneR GetHeapZone();

    //! Add Application Data to this ElementRef.
    //! @param[in] key The AppData's key. If an ElementRefAppData with this key already exists on this ElementRef, it is dropped and
    //! replaced with \a appData.
    //! @param[in] appData The appData object to attach to this ElementRef.
    //! @param[in] heapZone HeapZone for this ElementRef. \b Must be the HeapZone returned by #GetHeapZone.
    //! @param[in] allowOnDeleted if false (the default), this method will reject adds if the ElementRef is deleted.
    DGNPLATFORM_EXPORT StatusInt AddAppData (ElementRefAppData::Key const& key, ElementRefAppData* appData, HeapZoneR heapZone, bool allowOnDeleted=false);

    //! Drop Application data from this ElementRef.
    //! @param[in] key the key for the ElementRefAppData to drop.
    //! @return SUCCESS if an entry with \a key is found and dropped.
    DGNPLATFORM_EXPORT StatusInt DropAppData (ElementRefAppData::Key const& key);

    //! Find ElementRefAppData on this ElementRef by key.
    //! @param[in] key The key for the ElementRefAppData of interest.
    //! @return the ElementRefAppData for key \a key, or NULL.
    DGNPLATFORM_EXPORT ElementRefAppData* FindAppData (ElementRefAppData::Key const& key);
    //@}

    //! Get the DgnModel holding this ElementRef.
    //! @note Some types of ElementRefs are not in any DgnModel, and this function will return NULL.
    DGNPLATFORM_EXPORT DgnModelP GetDgnModelP () const;
    DGNPLATFORM_EXPORT DgnProjectP GetDgnProject() const;

    //! Get the ElementId of the element held by this ElementRef.
    DGNPLATFORM_EXPORT ElementId GetElementId () const;

    //! Get the DgnItemId of this ElementRef
    DGNPLATFORM_EXPORT DgnItemId GetItemId() const;

    //! Get the level of the element held by this ElementRef.
    DGNPLATFORM_EXPORT LevelId GetLevel () const;

    //! Test if the element is not displayed
    DGNPLATFORM_EXPORT bool IsUndisplayed () const;

    //! Set the element to be displayed or not displayed
    DGNPLATFORM_EXPORT void SetUndisplayedFlag (bool);

    //! Test if the element is in the selection set
    DGNPLATFORM_EXPORT bool IsInSelectionSet () const;

    //! Test if the element is highlighted in the view
    DGNPLATFORM_EXPORT ElementHiliteState IsHilited () const;

    //! Increment the reference count of this ElementRef
    //! @note This call should always be paired with a corresponding subsequent call to #Release when the element is no longer referenced.
    DGNPLATFORM_EXPORT UInt32 AddRef() const;

    //! Decrement the reference count of this ElementRef. When the reference count goes to zero, the ElementRef becomes "garbage" and may be reclaimed later.
    //! @note This call must always follow a previous call to #AddRef.
    DGNPLATFORM_EXPORT UInt32 Release() const;
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/11
//=======================================================================================
struct DgnElementRef : ElementRef
{
//__PUBLISH_SECTION_END__
    friend struct ElementListHandler;
    friend struct PersistentElementRefList;
    friend struct GraphicElementRefList;
    friend struct DbElementReloader;
    friend struct DbElementReader;
    friend struct ElemIdTree;

protected:
    DgnElementP       m_elm;

    DgnElementRef(DgnModelP model) : ElementRef(model) {m_elm=NULL;}
    ~DgnElementRef();

    void TransferIdToDescr (MSElementDescrR descr);
    bool IsSameSizeAndType (MSElementDescrCR descr);
    void ModifyFromDescr (DgnModelR dgnModel, MSElementDescrR newDescr);

    virtual DgnElementCP _GetElemPtrC () const override {return (DgnElementCP) m_elm;}

    DgnElementP ReserveMemory (UInt32 size);
    UInt32 GetTotalSizeBytes();
    void MarkRefDeleted(DgnModelR);
    void UnDeleteElement();
    DgnElementP WriteableElement () {return m_elm;}
    void DeallocateRef (struct DgnElementPool&, bool fileUnloading);

public:
    DGNPLATFORM_EXPORT void SaveElement (DgnElementCP);
//__PUBLISH_SECTION_START__
};

//=======================================================================================
//! An ElementRef for an element that is in a DgnModel. PersistentElementRefs are sometimes referred to as "persistent elements".
//! @ingroup ElementRefGroup
//! @bsiclass                                                     KeithBentley    10/00
//=======================================================================================
struct PersistentElementRef : public DgnElementRef
{
//__PUBLISH_SECTION_END__
    friend struct DgnModel;
    friend struct ElementListHandler;
    friend struct PersistentElementRefList;
    friend struct GraphicElementRefList;
    friend struct ElementTableHandler;
    friend struct DwgConversionDataAccessor;
    friend struct DbElementReader;
    friend struct DgnElementPool;

    DEFINE_T_SUPER(DgnElementRef)

protected:
    mutable UInt32 m_refCount;

private:
    void DeleteElementAndComponents (DgnModelR);

protected:
    void SetDynamicRange (bool yesNo);

    virtual QvCache* _GetMyQvCache () override;
    virtual ElementRefType _GetRefType () override {return ELEMENT_REF_TYPE_Persistent;}
    virtual HeapZone& _GetHeapZone () override;
    virtual StatusInt _DeleteElement () override;
    virtual StatusInt _UndeleteElement () override;
    virtual UInt32 _AddRef() const override;
    virtual UInt32 _Release() const override;

    DgnModelStatus ReloadFromDb ();

public:
    explicit PersistentElementRef (DgnModelP dgnModel, ElementId id) : DgnElementRef(dgnModel) {m_elementId=id; m_refCount=0;}
    UInt32 GetRefCount() const {return m_refCount;}

    DgnElementCP ElemPtrC () const {return (DgnElementCP) m_elm;}
    DRange3dCR GetRange() const {return ElemPtrC()->GetRange();}
    bool CanBeSavedToFile ();
    void AddDgnModelToDisplaySet (DgnModelP);
    StatusInt AddXAttribute (XAttributeHandleR, DgnPlatform::XAttributeHandlerId, UInt32 attrId, UInt32 size, void const* attrData, UInt32*);
    StatusInt DeleteXAttribute (XAttributeHandleR);
    StatusInt ReplaceXAttribute (XAttributeHandleCR, void const* data, UInt32 newSize);
    StatusInt ModifyXAttribute (XAttributeHandleCR, void const* data, UInt32 start, UInt32 length);
    DGNPLATFORM_EXPORT ElementListHandlerR GetListHandler() const;

    void UpdateDerivedRange ();
    DGNPLATFORM_EXPORT StatusInt AddDerivedRange (DerivedElementRange&);
    DGNPLATFORM_EXPORT StatusInt RemoveDerivedRange ();
    DGNPLATFORM_EXPORT DerivedElementRange* GetDerivedElementRange () const;

    //! Call this method if you know that the element is graphical and does NOT have a dynamic range.
    //! This is the quickest way to get a scanrange for an element that is already in the range index.
    DRange3dCR GetIndexRange () const {return !HasDerivedRange()? GetRange(): GetDerivedElementRange()->GetDerivedRange();}

    //! Call this method if you know that the element is graphical and does NOT have a dynamic range.
    //! Use this function instead of GetIndexRange if you already have a pointer to the element's data. It is more efficient.
    DRange3dCR GetIndexRangeOfElement (DgnElementCR el) const {return !HasDerivedRange()? el.GetRange(): GetDerivedElementRange()->GetDerivedRange();}

    //! Call this method if you know the element is graphical but you DON'T know if it has a dynamic range.
    DRange3dCP CheckIndexRange () const {return IsDynamicRange()? NULL: &GetIndexRange(); }

    DRange3dCP CheckIndexRangeOfElement (DgnElementCR el) const {return !HasDerivedRange() ? &el.GetRange() : &GetDerivedElementRange()->GetDerivedRange(); }

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
//__PUBLISH_SECTION_END__

