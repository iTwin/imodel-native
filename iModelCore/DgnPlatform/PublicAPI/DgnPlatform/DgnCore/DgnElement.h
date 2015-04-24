/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnElement.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

/** @addtogroup DgnElementGroup

Classes for working with elements in memory.

Elements must be loaded from @ref DgnProjectGroup and cached in memory before they can be accessed.
Element are loaded and cached using DgnDb::Elements() methods or using QueryModel's.

*/

BENTLEY_API_TYPEDEFS (HeapZone);

#include <Bentley/BeAssert.h>
#include <ECObjects/ECInstance.h>
#include <ECObjects/ECSchema.h>

enum ElementHiliteState
{
    HILITED_None         = 0,
    HILITED_Normal       = 1,
    HILITED_Bold         = 2,
    HILITED_Dashed       = 3,
    HILITED_Background   = 4,
};

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef RefCountedPtr<ElementGeometry> ElementGeometryPtr;

//=======================================================================================
// @bsiclass                                                     Keith.Bentley   06/08
//=======================================================================================
enum DgnElementChangeReason
{
    ELEMREF_CHANGE_REASON_Delete        = 1,
    ELEMREF_CHANGE_REASON_Modify        = 2,
    ELEMREF_CHANGE_REASON_ClearQVData   = 5,
};

//=======================================================================================
//! Create a subclass of this to store non-persistent information on a DgnElement.
//! @bsiclass
//=======================================================================================
struct DgnElementAppData
{
    virtual ~DgnElementAppData() {}

    //=======================================================================================
    //! A unique identifier for this type of DgnElementAppData. A static instance of
    //! DgnElementAppData::Key should be declared to hold the identifier.
    //! @bsiclass
    //=======================================================================================
    struct Key : BeSQLite::AppDataKey {};

    //! Return a name for this type of app data.
    //! @remarks Strictly for debugging, does not need to be implemented or localized.
    //! @return The name string or NULL.
    virtual WCharCP _GetName() {return NULL;}

    //! Called to clean up owned resources and delete the app data.
    //! @param[in]  host            DgnElementP that app data was added to.
    //! @param[in]  unloadingModel  If DgnModel containing host is being unloaded.
    //! @param[in]  zone            HeapZone for the DgnElementP holding the app data.
    //! @remarks If the app data was allocated using DgnElement::GetHeapZone(), there is
    //! nothing to free when unloadingCache is true as the entire heap zone will be freed with
    //! the cache. If unloadingCache is false, call HeapZone::Free. If a heap zone was not used, call delete/release/free as appropriate.
    //! @note If the appData was allocated using placement new with DgnElement::GetHeapZone(), the appData's
    //! destructor should be manually called in this method.
    virtual void _OnCleanup(DgnElementCP host, bool unloadingModel, HeapZoneR zone) = 0;

    //! Called to allow app data to react to changes to the persistent element it was added to.
    //! @param[in]  host            DgnElementP that app data was added to.
    //! @param[in]  qvCacheDeleted  Specific to app data used to cache the display representation.
    //!                             of the element. Clearing the qvCache invalidates QvElems stored in app data.
    //! @param[in]  reason          Why _OnElementChanged is being called.
    //! @return true to drop this app data entry from the element.
    virtual bool _OnElemChanged(DgnElementP host, bool qvCacheDeleted, DgnElementChangeReason reason) {return false;}
};

template <class _QvKey> struct QvElemSet;
//=======================================================================================
//! @bsiclass
//=======================================================================================
struct QvKey32
{
private:
    uint32_t m_key;

public:
    inline bool LessThan(QvKey32 const& other) const {return m_key < other.m_key;}
    inline bool Equal(QvKey32 const& other) const    {return m_key == other.m_key;}
    void DeleteQvElem(QvElem* qvElem);
    QvKey32(uint32_t key) {m_key = key;} // allow non-explicit!
};

typedef QvElemSet<QvKey32> T_QvElemSet;

//=======================================================================================
//! An instance of a element in memory
//! @bsiclass                                                     KeithBentley    10/00
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnElement : NonCopyableClass
{
public:
    enum DirtyFlags {DIRTY_ElemData = 1<<0, DIRTY_Aspects = 1<<1, DIRTY_Both = (DIRTY_ElemData|DIRTY_Aspects)};
    friend struct DgnElementPool;
    friend struct DgnModel;
    friend struct ITxn;
    friend struct ElemIdTree;
    friend struct EditElementHandle;
    friend struct ElementHandler;

    struct CreateParams
    {
    DgnModelR       m_model;
    DgnClassId      m_classId;
    DgnCategoryId   m_categoryId;
    Utf8CP          m_code;
    DgnElementId    m_id;
    DgnElementId    m_parentId;
    CreateParams(DgnModelR model, DgnClassId classId, DgnCategoryId category, Utf8CP code=nullptr, DgnElementId id=DgnElementId(), DgnElementId parent=DgnElementId()) :
                m_model(model), m_classId(classId), m_categoryId(category), m_code(code), m_id(id), m_parentId(parent) {}

    void SetCode(Utf8CP code) {m_code = code;}
    void SetElementId(DgnElementId id) {m_id=id;}
    void SetParentId(DgnElementId parent) {m_parentId=parent;}
    };

protected:
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

    struct AppDataEntry
        {
        DgnElementAppData::Key const* m_key;
        DgnElementAppData*            m_obj;
        AppDataEntry*                 m_next;

        void Init(DgnElementAppData::Key const& key, DgnElementAppData* obj, AppDataEntry* next) {m_key = &key; m_obj = obj; m_next = next;}
        void ClearEntry(DgnElementCP el, bool unloading, HeapZoneR zone) {if (NULL == m_obj) return; m_obj->_OnCleanup(el, unloading, zone); m_obj=NULL;}
        void SetEntry(DgnElementAppData* obj, DgnElementCP el, HeapZoneR zone) {ClearEntry(el, false, zone); m_obj = obj;}
        };

    struct Flags
        {
        uint32_t m_lockHeld:1;
        uint32_t m_editable:1;
        uint32_t m_inPool:1;
        uint32_t m_dirtyFlag:2;
        uint32_t m_deletedRef:1;
        uint32_t m_inSelectionSet:1;
        uint32_t m_hiliteState:3;
        uint32_t m_undisplayed:1;
        uint32_t m_mark1:1;                        // used by applications
        uint32_t m_mark2:1;                        // used by applications
        uint32_t m_elFlags:4;                      // used by element type specific code
        Flags() {memset(this, 0, sizeof(*this));}
        };

    mutable BeAtomic<uint32_t> m_refCount;
    DgnElementId    m_elementId;
    DgnElementId    m_parentId;
    DgnModelR       m_dgnModel;
    DgnClassId      m_classId;
    DgnCategoryId   m_categoryId;
    Utf8String      m_code;
    mutable Flags   m_flags;
    mutable AppDataEntry* m_appData;

    AppDataEntry* FreeAppDataEntry(AppDataEntry* prev, AppDataEntry& thisEntry, HeapZoneR zone, bool modelUnloading) const;

    void ClearDirty() {m_flags.m_dirtyFlag = 0;}
    void SetDeletedRef() {m_flags.m_deletedRef = true;}
    void ClearDeletedRef() {m_flags.m_deletedRef = false;}
    void SetInPool() {m_flags.m_inPool = true;}
    void MarkAsDeleted();
    void UnDeleteElement();
    virtual uint32_t _GetMemSize() const {return sizeof(*this);}
    void DeallocateRef(struct DgnElementPool&, bool fileUnloading);
    DgnModelStatus ReloadFromDb();
    ECN::IECInstanceR GetSubclassProperties(bool setModifiedFlag) const;

    explicit DgnElement(CreateParams const& params) : m_refCount(0), m_elementId(params.m_id), m_dgnModel(params.m_model), m_classId(params.m_classId), 
             m_categoryId(params.m_categoryId), m_code(params.m_code), m_parentId(params.m_parentId)
        {
        m_appData  = nullptr;
        }

    virtual ~DgnElement() {}
    DGNPLATFORM_EXPORT virtual uint32_t _AddRef() const;
    DGNPLATFORM_EXPORT virtual uint32_t _Release() const;
    DGNPLATFORM_EXPORT virtual void _GenerateDefaultCode();
    DGNPLATFORM_EXPORT virtual DgnModelStatus _LoadFromDb(DgnElementPool&);
    DGNPLATFORM_EXPORT virtual DgnModelStatus _InsertInDb(DgnElementPool&);
    DGNPLATFORM_EXPORT virtual DgnModelStatus _UpdateInDb(DgnElementPool&);
    DGNPLATFORM_EXPORT virtual DgnModelStatus _DeleteInDb(DgnElementPool&);
    //! Virtual copy constructor. If your subclass has member variables, it \em must override _InitFrom. @see _SwapWithModified
    virtual void _InitFrom(DgnElementCR other) {}
    //! Virtual move assignment operator. If your subclass has member variables, it \em must override _SwapWithModified. @see _InitFrom
    DGNPLATFORM_EXPORT virtual DgnModelStatus _SwapWithModified(DgnElementR);
    virtual DgnModelStatus _OnAdd() {return DGNMODEL_STATUS_Success;}
    virtual void _OnAdded() {}
    virtual GeometricElementCP _ToGeometricElement() const {return nullptr;}
    virtual DgnElement3dCP _ToElement3d() const {return nullptr;}
    virtual DgnElement2dCP _ToElement2d() const {return nullptr;}
    virtual PhysicalElementCP _ToPhysicalElement() const {return nullptr;}
    virtual DrawingElementCP _ToDrawingElement() const {return nullptr;}

public:
    void InitFrom(DgnElementCR rhs) {_InitFrom(rhs);}

    DGNPLATFORM_EXPORT virtual BentleyStatus _ApplyScheduledChangesToInstances(DgnElementR);
    DGNPLATFORM_EXPORT void _ClearScheduledChangesToInstances();

    GeometricElementCP ToGeometricElement() const {return _ToGeometricElement();}
    DgnElement3dCP ToElement3d() const {return _ToElement3d();}
    DgnElement2dCP ToElement2d() const {return _ToElement2d();}
    PhysicalElementCP ToPhysicalElement() const {return _ToPhysicalElement();}
    DrawingElementCP ToDrawingElement() const {return _ToDrawingElement();}

    GeometricElementP ToGeometricElementP() {return const_cast<GeometricElementP>(_ToGeometricElement());}
    DgnElement3dP ToElement3dP() {return const_cast<DgnElement3dP>(_ToElement3d());}
    DgnElement2dP ToElement2dP() {return const_cast<DgnElement2dP>(_ToElement2d());}
    PhysicalElementP ToPhysicalElementP() {return const_cast<PhysicalElementP>(_ToPhysicalElement());}
    DrawingElementP ToDrawingElementP() {return const_cast<DrawingElementP>(_ToDrawingElement());}

    bool Is3d() const {return nullptr != _ToElement3d();}
    bool IsInPool() const {return m_flags.m_inPool;}

    void SetElementClassId(DgnClassId classId) {m_classId = classId;}
    void SetCode(Utf8CP code) {m_code.AssignOrClear(code);}
    bool IsSameType(DgnElementCR other) {return m_classId == other.m_classId;}
    bool IsAnyDirty() const {return 0 != m_flags.m_dirtyFlag;}
    bool IsElemDirty()const {return 0 != (DIRTY_ElemData & m_flags.m_dirtyFlag);}
    void SetMark1(bool yesNo) const {if (m_flags.m_mark1==yesNo) return; m_flags.m_mark1 = yesNo;}
    void SetMark2(bool yesNo) const {if (m_flags.m_mark2==yesNo) return; m_flags.m_mark2 = yesNo;}
    void SetElFlags(int flags) {m_flags.m_elFlags = flags;}
    bool IsMarked1() const {return true == m_flags.m_mark1;}
    bool IsMarked2() const {return true == m_flags.m_mark2;}
    int GetElFlags() const {return m_flags.m_elFlags;}
    ElementHiliteState IsHilited() const {return (ElementHiliteState) m_flags.m_hiliteState;}
    void SetHilited(ElementHiliteState newState) const {m_flags.m_hiliteState = newState;}
    DGNPLATFORM_EXPORT void SetInSelectionSet(bool yesNo);

    void SetCategoryId(DgnCategoryId categoryId) {m_categoryId = categoryId;}
    uint32_t GetRefCount() const {return m_refCount.load();}
    void SetDirtyFlags(DirtyFlags flags) {m_flags.m_dirtyFlag |= flags;}

    DGNPLATFORM_EXPORT void ForceElemChanged(bool qvCacheCleared, DgnElementChangeReason);
    DGNPLATFORM_EXPORT void ForceElemChangedPost();
    DGNPLATFORM_EXPORT void ClearAllAppData(HeapZoneR, bool cacheUnloading);

    //! Test if the element is in the selection set
    bool IsInSelectionSet() const {return m_flags.m_inSelectionSet;}

    //! Test if the element is not displayed
    bool IsUndisplayed() const {return m_flags.m_undisplayed;}

    //! Set the element to be displayed or not displayed
    void SetUndisplayedFlag(bool yesNo) {m_flags.m_undisplayed = yesNo;}

    //! Determine whether this DgnElement is an element that once existed, but has been deleted.
    bool IsDeleted() const {return m_flags.m_deletedRef;}

    static DgnElementPtr Create(CreateParams const& params) {return new DgnElement(params);}

    DGNPLATFORM_EXPORT virtual DgnElementPtr Duplicate() const;

    DGNPLATFORM_EXPORT ElementHandlerR GetElementHandler() const;

    /// @name DgnElementAppData Management
    //@{
    //! Get the HeapZone for the this element.
    DGNPLATFORM_EXPORT HeapZoneR GetHeapZone() const;

    //! Add Application Data to this element.
    //! @param[in] key The AppData's key. If an DgnElementAppData with this key already exists on this element, it is dropped and
    //! replaced with \a appData.
    //! @param[in] appData The appData object to attach to this element.
    //! @param[in] heapZone HeapZone for this element. \b Must be the HeapZone returned by #GetHeapZone.
    //! @param[in] allowOnDeleted if false (the default), this method will reject adds if the element is deleted.
    DGNPLATFORM_EXPORT StatusInt AddAppData(DgnElementAppData::Key const& key, DgnElementAppData* appData, HeapZoneR heapZone, bool allowOnDeleted=false) const;

    //! Drop Application data from this element.
    //! @param[in] key the key for the DgnElementAppData to drop.
    //! @return SUCCESS if an entry with \a key is found and dropped.
    DGNPLATFORM_EXPORT StatusInt DropAppData(DgnElementAppData::Key const& key) const;

    //! Find DgnElementAppData on this element by key.
    //! @param[in] key The key for the DgnElementAppData of interest.
    //! @return the DgnElementAppData for key \a key, or NULL.
    DGNPLATFORM_EXPORT DgnElementAppData* FindAppData(DgnElementAppData::Key const& key) const;
    //@}

    //! Get the DgnModel of this element.
    DgnModelR GetDgnModel() const {return m_dgnModel;}

    //! Get the DgnDb of this element.
    DGNPLATFORM_EXPORT DgnDbR GetDgnDb() const;

    //! Get the DgnElementId for this DgnElement
    DgnElementId GetElementId() const {return m_elementId;}

    //! Get the DgnClassId for this DgnElement
    //! @see DgnElement::GetClassId
    DgnClassId GetElementClassId() const {return m_classId;}

    //! Get the DgnElementKey (the element DgnClassId and DgnElementId) for this DgnElement
    DgnElementKey GetElementKey() const {return DgnElementKey(GetElementClassId(), GetElementId());}

    //! Get the ECClass for this DgnElement
    DGNPLATFORM_EXPORT ECN::ECClassCP GetElementClass() const;

    //! Get the DgnClassId for the dgn.Element ECClass in the specified DgnDb.
    //! @see DgnElement::GetElementClassId
    DGNPLATFORM_EXPORT static DgnClassId GetClassId(DgnDbR db);

    //! Get the category of the element held by this element.
    DgnCategoryId GetCategoryId() const {return m_categoryId;}

    //! Get the code (business key) of the element held by this element.
    Utf8CP GetCode() const {return m_code.c_str();}

    //! Increment the reference count of this DgnElement
    //! @note This call should always be paired with a corresponding subsequent call to #Release when the element is no longer referenced.
    uint32_t AddRef() const {return _AddRef();}

    //! Decrement the reference count of this DgnElement. When the reference count goes to zero, the element becomes "garbage" and may be reclaimed later.
    //! @note This call must always follow a previous call to #AddRef.
    uint32_t Release() const {return _Release();}

    //! Add this DgnElement to its DgnModel. 
    //! @return DGNMODEL_STATUS_Success if the element was successfully added, error status otherwise.
    //! @see EditElementHandle::ReplaceInModel for how to update an existing element in its DgnModel.
    DGNPLATFORM_EXPORT DgnModelStatus AddToModel();

    /// @name Element Properties
    //@{

    //! Get the properties of this Element, other than the properties defined by dgn.Element.
    //! Note that the base class properties, including ElementId, ClassId, CategoryId, and Code, are accessed directly through member functions on DgnElement.
    //! @return an instance that holds the element's subclass properties. The instance will be empty if there are no subclass properties.
    //! @note This DgnElement controls the lifetime of the returned instance. Do not attempt to delete it.
    //! @note The returned instance is read-only.
    //! @see GetSubclassPropertiesR
    DGNPLATFORM_EXPORT ECN::IECInstanceCR GetSubclassProperties() const;

    //! Get the properties of this Element, other than the properties defined by dgn.Element.
    //! Note that the base class properties, including ElementId, ClassId, CategoryId, and Code, are accessed directly through member functions on DgnElement.
    //! @return an instance that holds the element's subclass properties. The instance will be empty if there are no subclass properties.
    //! @note This DgnElement controls the lifetime of the returned instance. Do not attempt to delete it.
    //! @note The returned instance is read-write. You can modify its properties.
    //! @see GetSubclassProperties, CancelSubclassPropertiesChange
    DGNPLATFORM_EXPORT ECN::IECInstanceR GetSubclassPropertiesR();

    //! Discard all scheduled changes to subclass properties.
    //! @note This method invalidates the object returned by GetSubclassPropertiesR and GetSubclassProperties
    DGNPLATFORM_EXPORT void CancelSubclassPropertiesChange();

    //@}
};

//=======================================================================================
//! A stream of geometry
// @bsiclass                                                    Keith.Bentley   12/14
//=======================================================================================
struct GeomStream
{
private:
    uint32_t m_size;
    uint32_t m_allocSize;
    uint8_t* m_data;

    void swap(GeomStream& rhs) {std::swap(m_size,rhs.m_size); std::swap(m_allocSize,rhs.m_allocSize); std::swap(m_data,rhs.m_data);}

public:
    GeomStream() {m_size=m_allocSize=0; m_data=nullptr;}
    DGNPLATFORM_EXPORT GeomStream(GeomStream const&);
    DGNPLATFORM_EXPORT ~GeomStream();
    GeomStream(GeomStream&& rhs) : m_size(rhs.m_size), m_allocSize(rhs.m_allocSize), m_data(rhs.m_data) {rhs.m_size=rhs.m_allocSize=0; rhs.m_data=nullptr;}
    DGNPLATFORM_EXPORT GeomStream& operator=(GeomStream const&);
    GeomStream& operator=(GeomStream&& rhs) {GeomStream(std::move(rhs)).swap(*this); return *this;}

    //! Get the size in bytes of the data in this GeomStream
    uint32_t GetSize() const {return m_size;}
    //! Get the size in bytes of the buffer for this GeomStream
    uint32_t GetAllocSize() const {return m_allocSize;}
    //! Get a const pointer to the GeomStream
    uint8_t const* GetData() const {return m_data;}
    //! Get a writeable pointer to the GeomStream
    uint8_t* GetDataR() const {return m_data;}
    //! Reserve memory for this GeomStream
    //! @param[in] size the number of bytes to reserve
    DGNPLATFORM_EXPORT void ReserveMemory(uint32_t size);
    //! Save a graphics stream
    //! @param[in] data the data to save
    //! @param[in] size number of bytes in data
    DGNPLATFORM_EXPORT void SaveData(uint8_t const* data, uint32_t size);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
struct Placement3d
{
protected:
    DPoint3d            m_origin;
    YawPitchRollAngles  m_angles;
    ElementAlignedBox3d m_boundingBox;

public:
    Placement3d() : m_origin(DPoint3d::FromZero())  {}
    Placement3d(DPoint3dCR origin, YawPitchRollAngles angles, ElementAlignedBox3dCR box): m_origin(origin), m_angles(angles), m_boundingBox(box) {}
    Placement3d(Placement3d const& rhs) : m_origin(rhs.m_origin), m_angles(rhs.m_angles), m_boundingBox(rhs.m_boundingBox) {}
    Placement3d(Placement3d&& rhs) : m_origin(rhs.m_origin), m_angles(rhs.m_angles), m_boundingBox(rhs.m_boundingBox) {}
    Placement3d& operator=(Placement3d&& rhs) {m_origin=rhs.m_origin; m_angles=rhs.m_angles; m_boundingBox=rhs.m_boundingBox; return *this;}
    Placement3d& operator=(Placement3d const& rhs) {m_origin=rhs.m_origin; m_angles=rhs.m_angles; m_boundingBox=rhs.m_boundingBox; return *this;}

    //! Get the placement origin
    DPoint3dCR GetOrigin() const {return m_origin;}
    //! Get a writeable reference to the placement origin
    DPoint3dR GetOriginR() {return m_origin;}

    //! Get the placement angles
    YawPitchRollAnglesCR GetAngles() const {return m_angles;}
    //! Get a writeable reference to the placement angles
    YawPitchRollAnglesR GetAnglesR() {return m_angles;}

    //! Get the element-aligned bounding box 
    ElementAlignedBox3d const& GetElementBox() const {return m_boundingBox;}

    //! Get a writeable reference to the element-aligned bounding box 
    ElementAlignedBox3d& GetElementBoxR() {return m_boundingBox;}

    Transform GetTransform() const {return m_angles.ToTransform(m_origin);}

    //! Calculate the axis-aligned range of this placement
    DGNPLATFORM_EXPORT AxisAlignedBox3d CalculateRange() const;
    
    //! Determine whether the range of this placement is valid 
    bool IsValid() const {return m_boundingBox.IsValid();}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
struct Placement2d
{
protected:
    DPoint2d            m_origin;
    double              m_angle;
    ElementAlignedBox2d m_boundingBox;

public:
    Placement2d() : m_origin(DPoint2d::FromZero()), m_angle(0.0)  {}
    Placement2d(DPoint2dCR origin, double angle, ElementAlignedBox2dCR box) : m_origin(origin), m_angle(angle), m_boundingBox(box){}
    Placement2d(Placement2d const& rhs) : m_origin(rhs.m_origin), m_angle(rhs.m_angle), m_boundingBox(rhs.m_boundingBox) {}
    Placement2d(Placement2d&& rhs) : m_origin(rhs.m_origin), m_angle(rhs.m_angle), m_boundingBox(rhs.m_boundingBox) {}
    Placement2d& operator=(Placement2d&& rhs) {m_origin=rhs.m_origin; m_angle=rhs.m_angle; m_boundingBox=rhs.m_boundingBox; return *this;}
    Placement2d& operator=(Placement2d const& rhs) {m_origin=rhs.m_origin; m_angle=rhs.m_angle; m_boundingBox=rhs.m_boundingBox; return *this;}

    //! Get the placement origin 
    DPoint2dCR GetOrigin() const {return m_origin;}
    //! Get a writeable reference to the placement origin 
    DPoint2dR GetOriginR() {return m_origin;}

    //! Get the placement angle of the element
    double GetAngle() const {return m_angle;}
    //! Get a writeable reference to the placement angle 
    double& GetAngleR() {return m_angle;}

    //! Get the element-aligned bounding box 
    ElementAlignedBox2d const& GetElementBox() const {return m_boundingBox;}

    //! Get a writeable reference to the element-aligned bounding box 
    ElementAlignedBox2d& GetElementBoxR() {return m_boundingBox;}

    Transform GetTransform() const {Transform t; t.InitFromOriginAngleAndLengths(m_origin, m_angle, 1.0, 1.0); return t;}

    //! Calculate the axis-aligned range of this placement
    DGNPLATFORM_EXPORT AxisAlignedBox3d CalculateRange() const;
    
    //! Determine whether the range of this placement is valid 
    bool IsValid() const {return m_boundingBox.IsValid();}

};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometricElement : DgnElement
{
    DEFINE_T_SUPER(DgnElement);

protected:
    GeomStream m_geom;
    mutable DgnClassId m_itemClassId;
    mutable ElementItemHandler* m_itemHandler;

    uint32_t _GetMemSize() const override {return sizeof(*this) + m_geom.GetAllocSize();}
    DGNPLATFORM_EXPORT void _InitFrom(DgnElementCR other) override;
    DGNPLATFORM_EXPORT DgnModelStatus _LoadFromDb(DgnElementPool&) override;
    DGNPLATFORM_EXPORT DgnModelStatus _InsertInDb(DgnElementPool&) override;
    DGNPLATFORM_EXPORT DgnModelStatus _UpdateInDb(DgnElementPool&) override;
    DGNPLATFORM_EXPORT DgnModelStatus _SwapWithModified(DgnElementR) override;
    virtual DgnModelStatus _BindInsertGeom(BeSQLite::Statement&) = 0;
    DGNPLATFORM_EXPORT virtual BentleyStatus _ApplyScheduledChangesToInstances(DgnElementR) override;
    GeometricElementCP _ToGeometricElement() const override {return this;}
    explicit GeometricElement(CreateParams const& params) : T_Super(params) {m_itemHandler=nullptr;} 
    
    DgnModelStatus DoInsertOrUpdate(BeSQLite::Statement&, DgnElementPool&);
     
    bvector<ECN::IECInstancePtr> GetAspects(ECN::ECClassCP ecclass) const;
    template<typename RTYPE, bool SETMODIFIED>
    bvector<RTYPE> GetAspects(DgnClassId aspectClass) const;
    ECN::IECInstanceP GetItem(bool setModifiedFlag) const;

public:
    virtual AxisAlignedBox3d _GetRange3d() const = 0;
    virtual DPoint3d _GetOrigin3d() const = 0;
    virtual ElementAlignedBox3d _GetElementBox3d() const = 0;
    virtual Transform _GetPlacementTrans() const = 0;

    DGNPLATFORM_EXPORT QvCache* GetMyQvCache() const;
    DGNPLATFORM_EXPORT QvElem* GetQvElem(uint32_t index) const;
    DGNPLATFORM_EXPORT bool SetQvElem(QvElem* qvElem, uint32_t index);
    T_QvElemSet* GetQvElems(bool createIfNotPresent) const;
    void SetItemClassId(DgnClassId classId) {m_itemClassId = classId;}
    DGNPLATFORM_EXPORT void SaveGeomStream(GeomStreamCP);
    DGNPLATFORM_EXPORT virtual void _Draw(ViewContextR) const;

    //! Get the GeomStream for this element.
    GeomStreamCR GetGeomStream() const {return m_geom;}

    //! Get a writeable reference to the GeomStream for this element.
    GeomStreamR GetGeomStreamR() {return m_geom;}

    //! Get the ElementGeom's DgnClassId (the form/geometry of the element)
    DGNPLATFORM_EXPORT DgnClassId GetItemClassId() const;

    ElementItemKey GetItemKey() const {return ElementItemKey(GetItemClassId(), GetElementId());}

    //! Get the handler that generated the ElementGeom
    DGNPLATFORM_EXPORT ElementItemHandler& GetItemHandler() const;

    /// @name Element Aspects
    //@{

    //! Get a copy of the ElementItem associated with this element, if any.
    //! @return a pointer to a read-only instance that holds the ElementItem's properties, or nullptr if the element has no Item.
    //! @note This DgnElement controls the lifetime of the returned instance. Do not attempt to delete it.
    //! @see GetItemP, SetItem, RemoveItem
    DGNPLATFORM_EXPORT ECN::IECInstanceCP GetItem() const;

    //! Get a writable copy of the ElementItem associated with this element, if any.
    //! @return a pointer to a read-write instance that holds the ElementItem's properties, or nullptr if the element has no Item.
    //! @note The returned instance can be used to read and/or modify the ElementItem's properties.
    //! @note This DgnElement controls the lifetime of the returned instance. Do not attempt to delete it.
    //! @see GetItem, SetItem, RemoveItem, CancelItemChange
    DGNPLATFORM_EXPORT ECN::IECInstanceP GetItemP();

    //! Get copies of all existing or pending ElementAspects of the specified class that are associated with this element.
    //! @param[in] aspectClass      The ECClass of the ElementAspect to query
    //! @return zero or more ElementAspect instances. The vector will be empty if the element has no ElementAspect of the specified class. The vector
    //! will hold multiple instances if the element has more than aspect of the specified class.
    //! @note This DgnElement controls the lifetime of the returned instances. Do not attempt to delete them.
    //! @see GetItemcp for a direct way to access the ElementItem.
    //! @see GetAspectsP, SetAspect, RemoveAspect
    DGNPLATFORM_EXPORT bvector<ECN::IECInstanceCP> GetAspects(DgnClassId aspectClass) const;

    //! Get writable copies of all existing or pending ElementAspects of the specified class that are associated with this element.
    //! @param[in] aspectClass      The ECClass of the ElementAspect to query
    //! @return zero or more ElementAspect instances. The vector will be empty if the element has no ElementAspect of the specified class. The vector
    //! will hold multiple instances if the element has more than aspect of the specified class.
    //! @note GetAspectsP returns instances that can be used to read and/or modify the aspects' properties.
    //! @note This DgnElement controls the lifetime of the returned instances. Do not attempt to delete them.
    //! @see GetItemP for a direct way to access the ElementItem.
    //! @see GetAspects, SetAspect, RemoveAspect, CancelAspectChange
    DGNPLATFORM_EXPORT bvector<ECN::IECInstanceP> GetAspectsP(DgnClassId aspectClass);

    //! Set the ElementItem associated with this element.
    //! If the element does not currently have an ElementItem, then the supplied instance will be used to insert one.
    //! Otherwise, the supplied instance will be used to update the existing ElementItem.
    //! The change is buffered in memory and is applied to the database when the element itself is inserted or replaced.
    //! @note DgnElement will increment the reference count on the supplied instance and then hold onto it.
    //! @note This method invalidates pointers returned by GetItemP and GetItem
    //! @see GetItemP, CancelItemChange
    DGNPLATFORM_EXPORT void SetItem(ECN::IECInstanceR itemInstance);

    //! Specify that the element's ElementItem should be deleted.
    //! The deletion is buffered in memory and is applied to the database when the element itself is replaced.
    //! @note This DgnElement will release its reference to the ElementItem instance that it is currently holding, if any.
    //! @note This method invalidates pointers returned by GetItemP and GetItem
    DGNPLATFORM_EXPORT void RemoveItem();

    //! Cancel a pending request to insert, update, or delete the ElementItem.
    //! @note This method invalidates pointers returned by GetItemP and GetItem
    //! @see GetItemP, SetItem, RemoveItem
    DGNPLATFORM_EXPORT void CancelItemChange();

    //! Add or update an aspect of this element.
    //! The change is buffered in memory and is applied to the database when the element itself is inserted or replaced.
    //! @param[in] instance      The new state of the aspect
    //! @note DgnElement will increment the reference count on the supplied instance and then hold onto it.
    //! @note This method invalidates pointers returned by GetAspectsP and GetAspects
    //! @see SetItem for a direct way to insert or update the ElementItem.
    //! @see GetAspectsP, CancelAspectChange, RemoveAspect
    DGNPLATFORM_EXPORT void SetAspect(ECN::IECInstanceR instance);

    //! Specify that an aspect of this element should be deleted.
    //! The deletion is buffered in memory and is applied to the database when the element itself is replaced.
    //! @param[in] aspectClass      The ECClass of the ElementAspect to be deleted
    //! @param[in] aspectId         The ID of the ElementAspect to be deleted
    //! @note This DgnElement will release its reference to the ElementAspect instance that it is currently holding, if any.
    //! @note This method invalidates pointers returned by GetAspectsP and GetAspects
    //! @see RemoveItem for a direct way to delete the ElementItem.
    DGNPLATFORM_EXPORT void RemoveAspect(DgnClassId aspectClass, BeSQLite::EC::ECInstanceId aspectId);

    //! Cancel a pending request to insert, update, or delete the aspects of this element.
    //! @note This method invalidates pointers returned by GetAspectsP and GetAspects
    //! @see RemoveAspect, SetAspect, GetAspectsP
    DGNPLATFORM_EXPORT void CancelAspectChange(DgnClassId aspectClass, BeSQLite::EC::ECInstanceId aspectId);

    //@}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnElement3d : GeometricElement
{
    DEFINE_T_SUPER(GeometricElement);

    struct CreateParams : DgnElement::CreateParams
    {
    DEFINE_T_SUPER(DgnElement::CreateParams);

    Placement3dCR m_placement;
    CreateParams(DgnModelR model, DgnClassId classId, DgnCategoryId category, Placement3dCR placement=Placement3d(), Utf8CP code=nullptr, DgnElementId id=DgnElementId(), DgnElementId parent=DgnElementId()) :
        T_Super(model, classId, category, code, id, parent), m_placement(placement) {}

    explicit CreateParams(DgnElement::CreateParams const& params, Placement3dCR placement=Placement3d()) : T_Super(params), m_placement(placement){}
    };

protected:
    Placement3d m_placement;

    explicit DgnElement3d(CreateParams const& params) : T_Super(params), m_placement(params.m_placement) {} 
    DGNPLATFORM_EXPORT DgnModelStatus _LoadFromDb(DgnElementPool&) override;
    DGNPLATFORM_EXPORT DgnModelStatus _BindInsertGeom(BeSQLite::Statement&) override; // This injects DgnElement3d's members into both _InsertInDb and _UpdateInDb 
    DGNPLATFORM_EXPORT DgnModelStatus _SwapWithModified(DgnElementR) override;
    DGNPLATFORM_EXPORT void _InitFrom(DgnElementCR) override;

public:
    DgnElement3dCP _ToElement3d() const override {return this;}
    AxisAlignedBox3d _GetRange3d() const override {return m_placement.CalculateRange();}
    DPoint3d _GetOrigin3d() const override {return m_placement.GetOrigin();}
    Placement3dCR GetPlacement() const {return m_placement;}
    void SetPlacement(Placement3dCR placement) {m_placement=placement;}
    ElementAlignedBox3d GetElementBox() const {return m_placement.GetElementBox();}
    ElementAlignedBox3d _GetElementBox3d() const override {return GetElementBox();}
    Transform _GetPlacementTrans() const override {return m_placement.GetAngles().ToTransform(m_placement.GetOrigin());}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalElement : DgnElement3d
{
    DEFINE_T_SUPER(DgnElement3d);
protected:
    friend struct PhysicalElementHandler;

    DGNPLATFORM_EXPORT DgnModelStatus _InsertInDb(DgnElementPool& pool) override;
    DGNPLATFORM_EXPORT DgnModelStatus _UpdateInDb(DgnElementPool&) override;

    PhysicalElementCP _ToPhysicalElement() const override {return this;}
    explicit PhysicalElement(CreateParams const& params) : T_Super(params) {} 

public:
    //! Factory method that creates an instance of a PhysicalElement
    //! @param[in] model Create the PhysicalElement in this PhysicalModel
    //! @param[in] categoryId specifies the category for the PhysicalElement.
    DGNPLATFORM_EXPORT static PhysicalElementPtr Create(PhysicalModelR model, DgnCategoryId categoryId);

    //! Get the DgnClassId for the dgn.PhysicalElement ECClass in the specified DgnDb.
    DGNPLATFORM_EXPORT static DgnClassId GetClassId(DgnDbR db);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnElement2d : GeometricElement
{
    DEFINE_T_SUPER(GeometricElement);
    struct CreateParams : DgnElement::CreateParams
    {
    DEFINE_T_SUPER(DgnElement::CreateParams);

    Placement2dCR m_placement;
    CreateParams(DgnModelR model, DgnClassId classId, DgnCategoryId category, Placement2dCR placement=Placement2d(), Utf8CP code=nullptr, DgnElementId id=DgnElementId(), DgnElementId parent=DgnElementId()) :
        T_Super(model, classId, category, code, id, parent), m_placement(placement) {}

    explicit CreateParams(DgnElement::CreateParams const& params, Placement2dCR placement=Placement2d()) : T_Super(params), m_placement(placement){}
    };

protected:
    Placement2d m_placement;
    explicit DgnElement2d(CreateParams const& params) : T_Super(params) {} 
    DGNPLATFORM_EXPORT DgnModelStatus _LoadFromDb(DgnElementPool&) override;
    DGNPLATFORM_EXPORT DgnModelStatus _BindInsertGeom(BeSQLite::Statement&) override; // This injects DgnElement2d's members into both _InsertInDb and _UpdateInDb 
    DGNPLATFORM_EXPORT DgnModelStatus _SwapWithModified(DgnElementR) override;
    DGNPLATFORM_EXPORT void _InitFrom(DgnElementCR) override;

public:
    DgnElement2dCP _ToElement2d() const override {return this;}
    AxisAlignedBox3d _GetRange3d() const override {return m_placement.CalculateRange();}
    DPoint3d _GetOrigin3d() const override {return DPoint3d::From(m_placement.GetOrigin());}
    void SetPlacement(Placement2dCR placement) {m_placement=placement;}
    ElementAlignedBox2d GetElementBox() const {return m_placement.GetElementBox();}
    ElementAlignedBox3d _GetElementBox3d() const override {return ElementAlignedBox3d(GetElementBox());}
    Transform _GetPlacementTrans() const override {Transform t; t.InitFromOriginAngleAndLengths(m_placement.GetOrigin(), m_placement.GetAngle(), 1.0, 1.0); return t;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DrawingElement : DgnElement2d
{
    DEFINE_T_SUPER(DgnElement2d);
protected:
    friend struct DrawingElementHandler;
    explicit DrawingElement(CreateParams const& params) : T_Super(params) {} 

    DrawingElementCP _ToDrawingElement() const override {return this;}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
