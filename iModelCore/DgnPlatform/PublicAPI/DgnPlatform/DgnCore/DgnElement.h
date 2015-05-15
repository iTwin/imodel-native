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

Classes for working with DgnElements in memory.

*/

BENTLEY_API_TYPEDEFS (HeapZone);

#include <Bentley/BeAssert.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef RefCountedPtr<ElementGeometry> ElementGeometryPtr;

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
    friend struct DgnElements;
    friend struct DgnModel;
    friend struct ElemIdTree;
    friend struct EditElementHandle;
    friend struct ElementHandler;

    //! Parameters for creating new DgnElements
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
        void SetParentId(DgnElementId parent) {m_parentId=parent;}
    };

    enum class Hilited : uint8_t
    {
        None         = 0,
        Normal       = 1,
        Bold         = 2,
        Dashed       = 3,
        Background   = 4,
    };

    //! Create a subclass of this to store non-persistent information on a DgnElement.
    struct AppData
    {
        virtual ~AppData() {}

        //! A unique identifier for this type of DgnElementAppData. Use a static instance of this class.
        struct Key : NonCopyableClass {};

        //! Called to clean up owned resources and delete the app data.
        //! @param[in]  el the DgnElement holding this AppData 
        virtual void _OnCleanup(DgnElementCR el) = 0;
        virtual bool _OnInsert(DgnElementCR el)  {return false;}
        virtual bool _OnInserted(DgnElementCR el){return false;}
        virtual bool _OnUpdate(DgnElementCR orig, DgnElementCR update)  {return false;}
        virtual bool _OnUpdated(DgnElementCR el) {return false;}
        virtual bool _OnDelete(DgnElementCR el)  {return false;}
        virtual bool _OnDeleted(DgnElementCR el) {return false;}
    };

protected:
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

    struct AppDataEntry
        {
        AppData::Key const* m_key;
        AppData*            m_obj;
        AppDataEntry*       m_next;

        void Init(AppData::Key const& key, AppData* obj, AppDataEntry* next) {m_key = &key; m_obj = obj; m_next = next;}
        void ClearEntry(DgnElementCR el) {if (nullptr == m_obj) return; m_obj->_OnCleanup(el); m_obj=nullptr;}
        void SetEntry(AppData* obj, DgnElementCR el) {ClearEntry(el); m_obj = obj;}
        };

    struct Flags
        {
        uint32_t m_lockHeld:1;
        uint32_t m_editable:1;
        uint32_t m_inPool:1;
        uint32_t m_inSelectionSet:1;
        uint32_t m_hilited:3;
        uint32_t m_undisplayed:1;
        uint32_t m_mark1:1;                        // used by applications
        uint32_t m_mark2:1;                        // used by applications
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

    AppDataEntry* FreeAppDataEntry(AppDataEntry* prev, AppDataEntry& thisEntry) const;

    void SetInPool(bool val) const {m_flags.m_inPool = val;}
                                                
    DGNPLATFORM_EXPORT virtual ~DgnElement();
                       virtual DgnModelStatus _LoadFromDb() {return DGNMODEL_STATUS_Success;}
    DGNPLATFORM_EXPORT virtual DgnModelStatus _InsertInDb();
    DGNPLATFORM_EXPORT virtual DgnModelStatus _UpdateInDb();
    DGNPLATFORM_EXPORT virtual DgnModelStatus _DeleteInDb() const;
                       virtual uint32_t _GetMemSize() const {return sizeof(*this);}

    //! Virtual assignment operator.  If your subclass has member variables, it \em must override this method
    DGNPLATFORM_EXPORT virtual DgnModelStatus _CopyFrom(DgnElementCR other);

    virtual DgnModelStatus _OnInsert() {return DGNMODEL_STATUS_Success;}
    virtual void _OnInserted() {}
    virtual GeometricElementCP _ToGeometricElement() const {return nullptr;}
    virtual DgnElement3dCP _ToElement3d() const {return nullptr;}
    virtual DgnElement2dCP _ToElement2d() const {return nullptr;}
    virtual PhysicalElementCP _ToPhysicalElement() const {return nullptr;}
    virtual DrawingElementCP _ToDrawingElement() const {return nullptr;}

    explicit DgnElement(CreateParams const& params) : m_refCount(0), m_elementId(params.m_id), m_dgnModel(params.m_model), m_classId(params.m_classId),
             m_categoryId(params.m_categoryId), m_code(params.m_code), m_parentId(params.m_parentId), m_appData(nullptr) {}

public:
    DgnModelStatus CopyFrom(DgnElementCR rhs) {return _CopyFrom(rhs);}
    DGNPLATFORM_EXPORT void AddRef() const;
    DGNPLATFORM_EXPORT void Release() const;
    uint32_t GetRefCount() const {return m_refCount.load();}

    DGNPLATFORM_EXPORT virtual Utf8String _GenerateDefaultCode();

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
    bool IsPersistent() const {return m_flags.m_inPool;}
    bool IsSameType(DgnElementCR other) {return m_classId == other.m_classId;}
    void SetMark1(bool yesNo) const {if (m_flags.m_mark1==yesNo) return; m_flags.m_mark1 = yesNo;}
    void SetMark2(bool yesNo) const {if (m_flags.m_mark2==yesNo) return; m_flags.m_mark2 = yesNo;}
    bool IsMarked1() const {return true == m_flags.m_mark1;}
    bool IsMarked2() const {return true == m_flags.m_mark2;}
    Hilited IsHilited() const {return (Hilited) m_flags.m_hilited;}
    void SetHilited(Hilited newState) const {m_flags.m_hilited = (uint8_t) newState;}
    DGNPLATFORM_EXPORT void SetInSelectionSet(bool yesNo) const;

    void SetCategoryId(DgnCategoryId categoryId) {m_categoryId = categoryId;}

    DGNPLATFORM_EXPORT void ClearAllAppData();

    //! Test if the element is in the selection set
    bool IsInSelectionSet() const {return m_flags.m_inSelectionSet;}

    //! Test if the element is not displayed
    bool IsUndisplayed() const {return m_flags.m_undisplayed;}

    //! Set this element's undisplayed flag
    void SetUndisplayedFlag(bool yesNo) {m_flags.m_undisplayed = yesNo;}

    //! Make a writable copy of this DgnElement so that the copy may be edited.
    //! @return a DgnElementPtr that holds the copy of this element.
    //! @note This method may only be used on a DgnElement this is the readonly persistent element returned by DgnElements::GetElement, and then
    //! only one editing copy of this element at a time may exist. If another copy is extant, this method will return an invalid DgnElementPtr.
    DGNPLATFORM_EXPORT DgnElementPtr CopyForEdit() const;

    //! Make a writable copy of this DgnElement so that the copy may be edited.
    //! This is merely a templated shortcut to dynamic_cast the return of #CopyForEdit to a subclass of DgnElement.
    template<class T> RefCountedPtr<T> MakeCopy() const {return dynamic_cast<T*>(CopyForEdit().get());}

    //! Update the persistent state of a DgnElement in the DgnDb from a modified copy of it.
    //! This is merely a shortcut for el.GetDgnDb().Elements().Update(el, stat);
    DGNPLATFORM_EXPORT DgnElementCPtr Update(DgnModelStatus* stat=nullptr);

    //! Insert this DgnElement into the DgnDb.
    //! This is merely a shortcut for el.GetDgnDb().Elements().Insert(el, stat);
    DGNPLATFORM_EXPORT DgnElementCPtr Insert(DgnModelStatus* stat=nullptr);

    //! Delete this DgnElement from the DgnDb,
    //! This is merely a shortcut for el.GetDgnDb().Elements().Delete(el);
    DGNPLATFORM_EXPORT DgnModelStatus Delete() const;

    //! Get the ElementHandler for this DgnElement.
    DGNPLATFORM_EXPORT ElementHandlerR GetElementHandler() const;

    /** @name AppData Management */
    /** @{ */
    //! Get the HeapZone for the this element.
    DGNPLATFORM_EXPORT HeapZoneR GetHeapZone() const;

    //! Add Application Data to this element.
    //! @param[in] key The AppData's key. If AppData with this key already exists on this element, it is dropped and
    //! replaced with \a appData.
    //! @param[in] appData The appData object to attach to this element.
    DGNPLATFORM_EXPORT StatusInt AddAppData(AppData::Key const& key, AppData* appData) const;

    //! Drop Application data from this element.
    //! @param[in] key the key for the AppData to drop.
    //! @return SUCCESS if an entry with \a key is found and dropped.
    DGNPLATFORM_EXPORT StatusInt DropAppData(AppData::Key const& key) const;

    //! Find DgnElementAppData on this element by key.
    //! @param[in] key The key for the AppData of interest.
    //! @return the AppData for key \a key, or nullptr.
    DGNPLATFORM_EXPORT AppData* FindAppData(AppData::Key const& key) const;
    /** @} */

    //! Get the DgnModel of this element.
    DgnModelR GetDgnModel() const {return m_dgnModel;}

    //! Get the DgnDb of this element.
    DGNPLATFORM_EXPORT DgnDbR GetDgnDb() const;

    //! Get the DgnElementId for this DgnElement
    DgnElementId GetElementId() const {return m_elementId;}

    //! Invalidate the ElementId of this element. This can be used to insert a copy of this element.
    void InvalidateElementId() {m_elementId = DgnElementId();}

    //! Get the DgnClassId for this DgnElement
    //! @see DgnElement::QueryClassId
    DgnClassId GetElementClassId() const {return m_classId;}

    //! Get the DgnElementKey (the element DgnClassId and DgnElementId) for this DgnElement
    DgnElementKey GetElementKey() const {return DgnElementKey(GetElementClassId(), GetElementId());}

    //! Get a pointer to the ECClass for this DgnElement
    DGNPLATFORM_EXPORT ECN::ECClassCP GetElementClass() const;

    //! Query the DgnClassId for the dgn.Element ECClass in the specified DgnDb.
    //! @see DgnElement::GetElementClassId
    DGNPLATFORM_EXPORT static DgnClassId QueryClassId(DgnDbR db);

    //! Get the DgnElementId for the parent of this element
    //! @return Id will be invalid if this element does not have a parent element
    DgnElementId GetParentId() const {return m_parentId;}

    //! Change the parent of this DgnElement
    void SetParentId(DgnElementId parent) {m_parentId = parent;}

    //! Get the category of this DgnElement.
    DgnCategoryId GetCategoryId() const {return m_categoryId;}

    //! Get the code (business key) of this DgnElement.
    Utf8CP GetCode() const {return m_code.c_str();}

    //! Set the code of this DgnElement.
    void SetCode(Utf8CP code) {m_code.AssignOrClear(code);}

    static DgnElementPtr Create(CreateParams const& params) {return new DgnElement(params);}
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
    //! Get a writable pointer to the GeomStream
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

    //! Get a writable reference to the placement origin
    DPoint3dR GetOriginR() {return m_origin;}

    //! Get the placement angles
    YawPitchRollAnglesCR GetAngles() const {return m_angles;}

    //! Get a writable reference to the placement angles
    YawPitchRollAnglesR GetAnglesR() {return m_angles;}

    //! Get the element-aligned bounding box
    ElementAlignedBox3d const& GetElementBox() const {return m_boundingBox;}

    //! Get a writable reference to the element-aligned bounding box
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
    //! Get a writable reference to the placement origin
    DPoint2dR GetOriginR() {return m_origin;}

    //! Get the placement angle of the element
    double GetAngle() const {return m_angle;}
    //! Get a writable reference to the placement angle
    double& GetAngleR() {return m_angle;}

    //! Get the element-aligned bounding box
    ElementAlignedBox2d const& GetElementBox() const {return m_boundingBox;}

    //! Get a writable reference to the element-aligned bounding box
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

    uint32_t _GetMemSize() const override {return sizeof(*this) + m_geom.GetAllocSize();}
    DGNPLATFORM_EXPORT DgnModelStatus _LoadFromDb() override;
    DGNPLATFORM_EXPORT DgnModelStatus _InsertInDb() override;
    DGNPLATFORM_EXPORT DgnModelStatus _UpdateInDb() override;
    DGNPLATFORM_EXPORT DgnModelStatus _CopyFrom(DgnElementCR) override;
    virtual DgnModelStatus _BindPlacement(BeSQLite::Statement&) = 0;
    GeometricElementCP _ToGeometricElement() const override {return this;}
    DgnModelStatus DoInsertOrUpdate(BeSQLite::Statement&);
    explicit GeometricElement(CreateParams const& params) : T_Super(params) {}

public:
    virtual AxisAlignedBox3d _GetRange3d() const = 0;

    DGNPLATFORM_EXPORT QvCache* GetMyQvCache() const;
    DGNPLATFORM_EXPORT QvElem* GetQvElem(uint32_t index) const;
    DGNPLATFORM_EXPORT bool SetQvElem(QvElem* qvElem, uint32_t index);
    T_QvElemSet* GetQvElems(bool createIfNotPresent) const;
    DGNPLATFORM_EXPORT void SaveGeomStream(GeomStreamCP);
    DGNPLATFORM_EXPORT virtual void _Draw(ViewContextR) const;
    DGNPLATFORM_EXPORT virtual bool _DrawHit(HitPathCR, ViewContextR) const;

    //! Get the GeomStream for this element.
    GeomStreamCR GetGeomStream() const {return m_geom;}

    //! Get a writable reference to the GeomStream for this element.
    GeomStreamR GetGeomStreamR() {return m_geom;}

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
    DGNPLATFORM_EXPORT DgnModelStatus _LoadFromDb() override;
    DGNPLATFORM_EXPORT DgnModelStatus _BindPlacement(BeSQLite::Statement&) override;
    DGNPLATFORM_EXPORT DgnModelStatus _CopyFrom(DgnElementCR) override;
    explicit DgnElement3d(CreateParams const& params) : T_Super(params), m_placement(params.m_placement) {}

public:
    DgnElement3dCP _ToElement3d() const override {return this;}
    AxisAlignedBox3d _GetRange3d() const override {return m_placement.CalculateRange();}
    Placement3dCR GetPlacement() const {return m_placement;}
    void SetPlacement(Placement3dCR placement) {m_placement=placement;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalElement : DgnElement3d
{
    DEFINE_T_SUPER(DgnElement3d);
protected:
    friend struct PhysicalElementHandler;

    DGNPLATFORM_EXPORT DgnModelStatus _InsertInDb() override;
    DGNPLATFORM_EXPORT DgnModelStatus _UpdateInDb() override;
    PhysicalElementCP _ToPhysicalElement() const override {return this;}
    explicit PhysicalElement(CreateParams const& params) : T_Super(params) {}

public:
    static PhysicalElementPtr Create(CreateParams const& params) {return new PhysicalElement(params);}

    //! Create an instance of a PhysicalElement from a model and DgnCategoryId
    //! @param[in] model Create the new element in this PhysicalModel
    //! @param[in] categoryId The category for the PhysicalElement.
    DGNPLATFORM_EXPORT static PhysicalElementPtr Create(PhysicalModelR model, DgnCategoryId categoryId);

    //! Query the DgnClassId for the dgn.PhysicalElement class in the specified DgnDb.
    DGNPLATFORM_EXPORT static DgnClassId QueryClassId(DgnDbR db);
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
    DGNPLATFORM_EXPORT DgnModelStatus _LoadFromDb() override;
    DGNPLATFORM_EXPORT DgnModelStatus _BindPlacement(BeSQLite::Statement&) override;
    DGNPLATFORM_EXPORT DgnModelStatus _CopyFrom(DgnElementCR) override;
    explicit DgnElement2d(CreateParams const& params) : T_Super(params) {}

public:
    DgnElement2dCP _ToElement2d() const override {return this;}
    AxisAlignedBox3d _GetRange3d() const override {return m_placement.CalculateRange();}
    Placement2dCR GetPlacement() const {return m_placement;}
    void SetPlacement(Placement2dCR placement) {m_placement=placement;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DrawingElement : DgnElement2d
{
    DEFINE_T_SUPER(DgnElement2d);
protected:
    friend struct DrawingElementHandler;
    DrawingElementCP _ToDrawingElement() const override {return this;}
    explicit DrawingElement(CreateParams const& params) : T_Super(params) {}

public:
    static DrawingElementPtr Create(CreateParams const& params) {return new DrawingElement(params);}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
