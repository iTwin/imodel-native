/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnModel.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDomain.h"
#include "DgnElement.h"
#include <Bentley/ValueFormat.h>
#include <DgnPlatform/DgnProperties.h>

DGNPLATFORM_TYPEDEFS (DgnModel2d)
DGNPLATFORM_TYPEDEFS (DgnModel3d)
DGNPLATFORM_TYPEDEFS (DgnRangeTree)
DGNPLATFORM_TYPEDEFS (ICheckStop)
DGNPLATFORM_TYPEDEFS (PlanarPhysicalModel)
DGNPLATFORM_TYPEDEFS (SectioningViewController)
DGNPLATFORM_TYPEDEFS (SheetModel)
DGNPLATFORM_REF_COUNTED_PTR(SectioningViewController)
DGNPLATFORM_REF_COUNTED_PTR(SheetModel)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct SheetModelHandler;

//=======================================================================================
//! A map whose key is DgnElementId and whose data is DgnElementCPtr
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DgnElementMap : bmap<DgnElementId, DgnElementCPtr>
    {
    void Add(DgnElementCR el)
        {
        DgnElementId  id = el.GetElementId();
        if (!id.IsValid())
            {
            BeAssert(false);
            return;
            }
        Insert(id, &el);
        }
    };

/** @addtogroup DgnModelGroup DgnModels
@ref PAGE_ModelOverview
*/

//=======================================================================================
//! A DgnModel represents a model in memory and may hold references to elements that belong to it.
//! @ingroup DgnModelGroup
// @bsiclass                                                     KeithBentley    10/00
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnModel : RefCountedBase
    {
    friend struct DgnModels;
    friend struct DgnElement;
    friend struct DgnElements;
    friend struct QueryModel;

    //========================================================================================
    //! Application data attached to a DgnModel. Create a subclass of this to store non-persistent information on a DgnModel.
    //! @see DgnModel::AddAppData
    //=======================================================================================
    struct AppData : RefCountedBase
        {
        //! A unique Key to identify each subclass of AppData.
        struct Key : NonCopyableClass {};

        enum class DropMe {No=0, Yes=1};

        //! Override this method to be notified after host DgnModel has been filled.
        virtual DropMe _OnFilled(DgnModelCR) {return DropMe::No;}

        //! Override this method to be notified when host DgnModel is about to be emptied.
        //! @return true to be dropped from model
        virtual void _OnEmpty(DgnModelCR) {}

        //! Override this method to be notified after host DgnModel has been emptied.
        virtual DropMe _OnEmptied(DgnModelCR) {return DropMe::No;}

        //! Override this method to be notified before the DgnModel is deleted.
        virtual void _OnDelete(DgnModelCR) {}

        virtual DropMe _OnDeleted(DgnModelCR) {return DropMe::Yes;}
    };

    //=======================================================================================
    //! The properties for a DgnModel. These are stored as a JSON string in the "Props" column of the DgnModel table.
    //! These properties are saved by calling DgnModel::SaveProperties
    //! @ingroup DgnModelGroup
    //=======================================================================================
    struct Properties
    {
        friend struct DgnModel;

    private:
        struct FormatterFlags
        {
            uint32_t m_linearUnitMode:2;
            uint32_t m_linearPrecType:4;
            uint32_t m_linearPrecision:8;
            uint32_t m_angularMode:3;
            uint32_t m_angularPrecision:8;
            uint32_t m_directionMode:2;
            uint32_t m_directionClockwise:1;
            void FromJson(Json::Value const& inValue);
            void ToJson(Json::Value& outValue) const;
        };

        FormatterFlags m_formatterFlags;               //!< format flags
        UnitDefinition m_masterUnit;                   //!< Master Unit information
        UnitDefinition m_subUnit;                      //!< Sub Unit information
        double         m_roundoffUnit;                 //!< unit lock roundoff val in uors
        double         m_roundoffRatio;                //!< Unit roundoff ratio y to x (if 0 use Grid Ratio)
        double         m_formatterBaseDir;             //!< Base Direction used for Direction To/From String

    public:
        Properties()
            {
            m_formatterFlags.m_linearUnitMode = 0;
            m_formatterFlags.m_linearPrecType = 0;
            m_formatterFlags.m_linearPrecision= 0;
            m_formatterFlags.m_angularMode = 0;
            m_formatterFlags.m_angularPrecision = 0;
            m_formatterFlags.m_directionMode = 0;
            m_formatterFlags.m_directionClockwise = 0;
            m_roundoffRatio= 0;
            m_formatterBaseDir = 0;
            m_roundoffUnit = 0;
            m_subUnit.Init(UnitBase::Meter, UnitSystem::Metric, 1.0, 1.0, L"m");
            m_masterUnit = m_subUnit;
            }

        void FromJson(Json::Value const& inValue);
        void ToJson(Json::Value& outValue) const;

        //! Set working-units and sub-units. Units must be valid and comparable.
        DGNPLATFORM_EXPORT BentleyStatus SetWorkingUnits(UnitDefinitionCR newMasterUnit, UnitDefinitionCR newSubUnit);
        void SetLinearUnitMode(DgnUnitFormat value) { m_formatterFlags.m_linearUnitMode = (uint32_t)value; }
        void SetLinearPrecision(PrecisionFormat value) {
                m_formatterFlags.m_linearPrecType  = static_cast<uint32_t>(DoubleFormatter::GetTypeFromPrecision(value));
                m_formatterFlags.m_linearPrecision = DoubleFormatter::GetByteFromPrecision(value);}
        void SetAngularMode(AngleMode value) { m_formatterFlags.m_angularMode = (uint32_t)value; }
        void SetAngularPrecision(AnglePrecision value) { m_formatterFlags.m_angularPrecision = (uint32_t)value; }
        void SetDirectionMode(DirectionMode value) {m_formatterFlags.m_directionMode = (uint32_t)value; }
        void SetDirectionClockwise(bool value) { m_formatterFlags.m_directionClockwise = value; }
        void SetDirectionBaseDir(double value) { m_formatterBaseDir = value; }
        DgnUnitFormat GetLinearUnitMode() const {return (DgnUnitFormat) m_formatterFlags.m_linearUnitMode; }
        PrecisionFormat GetLinearPrecision() const {return DoubleFormatter::ToPrecisionEnum((PrecisionType) m_formatterFlags.m_linearPrecType, m_formatterFlags.m_linearPrecision); }
        AngleMode GetAngularMode() const {return (AngleMode) m_formatterFlags.m_angularMode; }
        AnglePrecision GetAngularPrecision() const {return (AnglePrecision) m_formatterFlags.m_angularPrecision; }
        DirectionMode GetDirectionMode() const {return (DirectionMode) m_formatterFlags.m_directionMode; }
        bool GetDirectionClockwise() const {return m_formatterFlags.m_directionClockwise; }
        double GetDirectionBaseDir() const {return m_formatterBaseDir; }
        void SetRoundoffUnit(double roundoffUnit, double roundoffRatio) {m_roundoffUnit  = roundoffUnit;m_roundoffRatio = roundoffRatio;}
        double GetRoundoffUnit() const {return m_roundoffUnit;}
        double GetRoundoffRatio() const {return m_roundoffRatio;}
        FormatterFlags GetFormatterFlags() const    {return m_formatterFlags;}
        UnitDefinitionCR GetMasterUnit() const {return m_masterUnit;}
        UnitDefinitionCR GetSubUnit() const {return m_subUnit;}
    };

    //=======================================================================================
    //! Parameters to create new instances of DgnModel.
    //! @ingroup DgnModelGroup
    //=======================================================================================
    struct CreateParams
    {
        DgnDbR      m_dgndb;
        DgnModelId  m_id;
        DgnClassId  m_classId;
        Utf8String  m_name;
        Properties  m_props;
        CreateParams(DgnDbR dgndb, DgnClassId classId, Utf8CP name, Properties props=Properties(), DgnModelId id=DgnModelId()) :
            m_dgndb(dgndb), m_id(id), m_classId(classId), m_name(name), m_props(props) {}
    };

private:
    template<class T> void CallAppData(T const& caller) const;
    void RegisterElement(DgnElementCR);
    void SetFilled() {m_filled=true; AllocateRangeIndex();}
    void AllocateRangeIndex() const;
    void ClearRangeIndex();
    void ReleaseAllElements();
    void AddToRangeIndex(DgnElementCR);
    void RemoveFromRangeIndex(DgnElementCR);
    void UpdateRangeIndex(DgnElementCR modified, DgnElementCR original);

protected:
    DgnDbR          m_dgndb;
    DgnModelId      m_modelId;
    DgnClassId      m_classId;
    Utf8String      m_name;
    Properties      m_properties;
    DgnElementMap   m_elements;
    mutable bmap<AppData::Key const*, RefCountedPtr<AppData>, std::less<AppData::Key const*>, 8> m_appData;
    mutable DgnRangeTreeP m_rangeIndex;
    mutable bool    m_persistent;   // true of this DgnModel is in the DgnModels "loaded models" list.
    bool            m_filled;       // true if the FillModel was called on this DgnModel.
    bool            m_readonly;     // true if this model is from a read-only file.

    explicit DGNPLATFORM_EXPORT DgnModel(CreateParams const&);
    DGNPLATFORM_EXPORT virtual ~DgnModel();

    DGNPLATFORM_EXPORT virtual void _InitFrom(DgnModelCR other);            //!< @private
    virtual DgnModelType _GetModelType() const = 0;
    virtual DgnModels::Model::CoordinateSpace _GetCoordinateSpace() const = 0;
    DGNPLATFORM_EXPORT virtual AxisAlignedBox3d _QueryModelRange() const;
    virtual bool _Is3d() const = 0;
    DGNPLATFORM_EXPORT virtual void _ToPropertiesJson(Json::Value&) const;
    DGNPLATFORM_EXPORT virtual void _FromPropertiesJson(Json::Value const&);
    DGNPLATFORM_EXPORT virtual DPoint3d _GetGlobalOrigin() const;

    /** @name Events for the DgnElements of a DgnModel */
    /** @{ */
    //! Called when a DgnElement in this DgnModel is about to be inserted.
    //! @note If you override this method, you @em must call the T_Super implementation, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element);

    //! Called when a DgnElement in this DgnModel is about to be updated.
    //! @note If you override this method, you @em must call the T_Super implementation, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnUpdateElement(DgnElementCR modified, DgnElementCR original);

    //! Called when a DgnElement in this DgnModel is about to be deleted.
    //! @note If you override this method, you @em must call the T_Super implementation, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnDeleteElement(DgnElementCR element);

    //! Called after a DgnElement in this DgnModel has been loaded into memory.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    DGNPLATFORM_EXPORT virtual void _OnLoadedElement(DgnElementCR el);

    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    DGNPLATFORM_EXPORT virtual void _OnInsertedElement(DgnElementCR el);

    //! Called after a DgnElement in this DgnModel has been updated.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    DGNPLATFORM_EXPORT virtual void _OnUpdatedElement(DgnElementCR modified, DgnElementCR original);

    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    DGNPLATFORM_EXPORT virtual void _OnDeletedElement(DgnElementCR element);
    /** @} */

    /** @name Events for a DgnModel */
    /** @{ */
    DGNPLATFORM_EXPORT virtual void _FillModel();
    DGNPLATFORM_EXPORT virtual void _OnLoaded();
    DGNPLATFORM_EXPORT virtual void _OnInserted();
    DGNPLATFORM_EXPORT virtual void _OnDeleted();
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert();
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnDelete();
    /** @} */

    /** @name Dynamic cast shortcuts for a DgnModel */
    /** @{ */
    virtual DgnModel2dCP _ToDgnModel2d() const {return nullptr;}
    virtual DgnModel3dCP _ToDgnModel3d() const {return nullptr;}
    virtual PhysicalModelCP _ToPhysicalModel() const {return nullptr;}
    virtual PlanarPhysicalModelCP _ToPlanarPhysicalModel() const {return nullptr;}
    virtual SheetModelCP _ToSheetModel() const {return nullptr;}
    /** @} */

    //! Add non-element graphics for this DgnModel to the scene.
    //! Normally, the scene is generated by QueryViewController from the elements in a model.
    //! A subclass can override this method to add non-element-based graphics to the scene. Or, a subclass
    //! can override this method to do add graphics that QueryViewController would normally exclude.
    //! Currently, only QueryViewController calls this method.
    //! <h2>Coordinate Systems</h2>
    //! A DgnDb defines a single physical coordinate system. The original is at (0,0,0) and distances are always in meters.
    //! A DgnDb is associated with a single Geographic Coordinate System (GCS). See DgnUnits::GetDgnGCS.
    //! Graphics in the scene must be defined in the DgnDb's coordinate system.
    //! The implementation must transform external data into the coordinate system of the DgnDb as necessary before adding graphics to the scene.
    //! <h2>Displaying external data using progressive display</h2>
    //! An implementation of _AddGraphicsToScene is required to be very fast. If some external data is not immediately available, then the implementation should
    //! a) make arrangements to obtain the data in the background and b) schedule itself for callbacks during progressive display in order to display the data when it becomes available.
    //! See DgnViewport::ScheduleProgressiveDisplay for how to register for progressive display.
    virtual void _AddGraphicsToScene(ViewContextR) {}

public:
    DGNPLATFORM_EXPORT ModelHandlerR GetModelHandler() const;

    DGNPLATFORM_EXPORT DgnRangeTree* GetRangeIndexP(bool create) const;
    void ReadProperties();
    DGNPLATFORM_EXPORT BeSQLite::DbResult SaveProperties();
    void AddGraphicsToScene(ViewContextR context) {_AddGraphicsToScene(context);}

    void ClearAllQvElems(); //!< @private

    DGNPLATFORM_EXPORT double GetMillimetersPerMaster() const;
    DGNPLATFORM_EXPORT double GetSubPerMaster() const;
    DGNPLATFORM_EXPORT DPoint3d GetGlobalOrigin() const;

    //! Empty the contents of this DgnModel. This will release any references to DgnElements held by this DgnModel.
    DGNPLATFORM_EXPORT void Empty();

    //! Load all elements of this DgnModel.
    //! After this call, all of the DgnElements of this model are loaded and are held in memory by this DgnModel.
    //! @note if this DgnModel is already filled, this method does nothing and returns DgnDbStatus::Success.
    void FillModel() {_FillModel();}

    //! Determine whether this DgnModel has been "filled" from disk or not.
    //! @return true if the DgnModel is filled.
    bool IsFilled() const {return m_filled;}

    //! Determine whether this DgnModel is a persistent model.
    bool IsPersistent() const {return m_persistent;}

    //! Get the number of elements in this DgnModel.
    //! @return the number of elements in this DgnModel.
    //! @note The DgnModel must be filled before calling this method.
    //! @see FillSections
    uint32_t CountElements() const {return (uint32_t) m_elements.size();}

    //! Find a DgnElementP in this DgnModel by DgnElementId.
    //! @return DgnElementP of element with \a id, or NULL.
    DGNPLATFORM_EXPORT DgnElementCP FindElementById(DgnElementId id);

    //! Determine whether this is a 3D DgnModel
    bool Is3d() const {return _Is3d();}

    //! Get the range of all visible elements in the DgnModel.
    AxisAlignedBox3d QueryModelRange() const {return _QueryModelRange();}

    //! Get the Properties for this DgnModel.
    Properties& GetPropertiesR() {return m_properties;}

    //! Get the Properties for this DgnModel.
    Properties const& GetProperties() const {return m_properties;}

    //! Get the name of this DgnModel
    Utf8CP GetModelName() const {return m_name.c_str();}

    //! Get the type of this DgnModel
    DgnModelType GetModelType() const {return _GetModelType();}

    //! Get the DgnClassId of this DgnModel
    DgnClassId GetClassId() const {return m_classId;}

    //! Get the DgnModelId of this DgnModel
    DgnModelId GetModelId() const {return m_modelId;}

    DgnModel2dCP ToDgnModel2d() const {return _ToDgnModel2d();}
    DgnModel3dCP ToDgnModel3d() const {return _ToDgnModel3d();}
    PhysicalModelCP ToPhysicalModel() const {return _ToPhysicalModel();}
    PlanarPhysicalModelCP ToPlanarPhysicalModel() const {return _ToPlanarPhysicalModel();}
    SheetModelCP ToSheetModel() const {return _ToSheetModel();}
    DgnModel2dP ToDgnModel2dP() {return const_cast<DgnModel2dP>(_ToDgnModel2d());}
    DgnModel3dP ToDgnModel3dP() {return const_cast<DgnModel3dP>(_ToDgnModel3d());}
    PhysicalModelP ToPhysicalModelP() {return const_cast<PhysicalModelP>(_ToPhysicalModel());}
    PlanarPhysicalModelP ToPlanarPhysicalModelP() {return const_cast<PlanarPhysicalModelP>(_ToPlanarPhysicalModel());}
    SheetModelP ToSheetModelP() {return const_cast<SheetModelP>(_ToSheetModel());}

    //! Determine whether this is a readonly DgnModel or not.
    bool IsReadOnly() const {return m_readonly;}

    void SetReadOnly(bool val) {m_readonly = val;}

    //! Get the DgnDb that contains this model.
    //! @return the DgnDb that contains this model.
    DgnDbR GetDgnDb() const {return m_dgndb;}

    //! Insert this model into the DgnDb.
    //! @return DgnDbStatus::Success if this model was successfully inserted, error otherwise.
    DGNPLATFORM_EXPORT DgnDbStatus Insert(Utf8CP description=nullptr, bool inGuiList=true);

    //! Delete this model.
    //! @note All elements from this model are deleted as well.
    DGNPLATFORM_EXPORT DgnDbStatus Delete();

    /** @name DgnModel AppData */
    /** @{ */
    //! Add (or replace) AppData on this DgnModel.
    //! @note It is illegal to add or remove AppData from within
    //! any of the AppData "_On" methods. If an entry with \a key already exists, it will be dropped and replaced with \a appData.
    DGNPLATFORM_EXPORT void AddAppData(AppData::Key const& key, AppData* appData);

    //! Remove AppData from thsis DgnModel
    //! @return SUCCESS if appData with key is found and was dropped.
    //! @remarks Calls the app data object's _OnCleanup method.
    DGNPLATFORM_EXPORT StatusInt DropAppData(AppData::Key const& key);

    //! Search for AppData on this model by AppData::Key.
    //! @return the AppData with \a key, or nullptr.
    DGNPLATFORM_EXPORT AppData* FindAppData(AppData::Key const& key) const;
    /** @} */

    DGNPLATFORM_EXPORT DgnModelPtr Clone(Utf8CP newName) const;

    typedef DgnElementMap::const_iterator const_iterator;
    DgnElementMap const& GetElements() const {return m_elements;}
    bool IsEmpty() const {return (begin() != end());}
    const_iterator begin() const {return m_elements.begin();}
    const_iterator end() const {return m_elements.end();}
};

//=======================================================================================
//! @ingroup DgnModelGroup
// @bsiclass                                                    Keith.Bentley   03/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnModel3d : DgnModel
{
    DEFINE_T_SUPER(DgnModel)

protected:
    virtual bool _Is3d() const override {return true;}
    virtual DgnModel3dCP _ToDgnModel3d() const override {return this;}

public:
    explicit DgnModel3d(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A DgnModel2d is a infinite planar model. Coordinates values are X,Y.
//! @ingroup DgnModelGroup
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnModel2d : DgnModel
    {
    DEFINE_T_SUPER(DgnModel)

protected:
    DPoint2d m_globalOrigin;    //!< Global Origin - all coordinates are offset by this value.

    bool _Is3d() const override {return false;}
    DGNPLATFORM_EXPORT void _ToPropertiesJson(Json::Value&) const override;
    DGNPLATFORM_EXPORT void _FromPropertiesJson(Json::Value const&) override;
    DPoint3d _GetGlobalOrigin() const override {return DPoint3d::From(m_globalOrigin);}
    DgnModel2dCP _ToDgnModel2d() const override {return this;}

    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element);

public:
    void SetGlobalOrigin(DPoint2dCR org) {m_globalOrigin = org;}

    explicit DgnModel2d(CreateParams const& params, DPoint2dCR origin=DPoint2d::FromZero()) : T_Super(params), m_globalOrigin(origin) {}
    };

//=======================================================================================
//! @ingroup DgnModelGroup
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalModel : DgnModel3d
{
    DEFINE_T_SUPER(DgnModel3d)
protected:
    DgnModelType _GetModelType() const override {return DgnModelType::Physical;}
    PhysicalModelCP _ToPhysicalModel() const override {return this;}
    DgnModels::Model::CoordinateSpace _GetCoordinateSpace() const override {return DgnModels::Model::CoordinateSpace::World;}

public:
    explicit PhysicalModel(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! @ingroup DgnModelGroup
//! @private
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ComponentModel : DgnModel3d
{
    DEFINE_T_SUPER(DgnModel3d)

protected:
    DgnModelType _GetModelType() const override {return DgnModelType::Component;}
    DPoint3d _GetGlobalOrigin() const override {return DPoint3d::FromZero();}

public:
    explicit ComponentModel(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A GraphicsModel2d is a DgnModel2d that does not have any relationship to physical space.
//! @ingroup DgnModelGroup
// @bsiclass                                                    Sam.Wilson  05/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GraphicsModel2d : DgnModel2d
    {
    DEFINE_T_SUPER(DgnModel2d)

protected:
    bool _Is3d() const override {return false;}
    DgnModelType _GetModelType() const override {return DgnModelType::Drawing;}
    DgnModels::Model::CoordinateSpace _GetCoordinateSpace() const override {return DgnModels::Model::CoordinateSpace::Local;}

public:
    explicit GraphicsModel2d(CreateParams const& params, DPoint2dCR origin=DPoint2d::FromZero()) : T_Super(params, origin) {}
    };

//=======================================================================================
//! A PlanarPhysicalModel is an infinite planar model that subdivides physical space into two halves. The plane of a
//! PhysicalPlanar model may be mapped into physical space in non-linear way, but every finite point in physical space is
//! either "in front" or "in back" of the plane.
//! @note a PlanarPhysicalModel @b is @b a DgnModel2d, and all of its elements are 2-dimensional.
//! Also note that any (2d) point on a PlanarPhysicalModel corresponds to a single point in physical space.
//! @ingroup DgnModelGroup
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PlanarPhysicalModel : DgnModel2d
{
    DEFINE_T_SUPER(DgnModel2d)

protected:
    DgnModels::Model::CoordinateSpace _GetCoordinateSpace() const override {return DgnModels::Model::CoordinateSpace::World;}
    DgnModelType _GetModelType() const override {return DgnModelType::Drawing;}
    PlanarPhysicalModelCP _ToPlanarPhysicalModel() const override {return this;}
public:
    explicit PlanarPhysicalModel(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A SectionDrawingModel is a PlanarPhysicalModel that is mapped into physical space such that the vertical direction (Y
//! vector) of the SectionDrawingModel is constant in physical space. That is, physical space is divided in half (cut) by a
//! series of line segments, continuous and monotonically increasing along the X axis, in the XZ plane of the drawing. This
//! is called the "section plane", and the line segments are called the "section lines". In AEC section drawings, a further
//! restriction is that the section lines always parallel but may be disjoint (some mechanical section drawings allow
//! continuous but non-parallel section lines). Physical space in the positive Z direction of the section plane is called
//! "in front" of the section plane, and space in negative Z is called "behind" the section plane.
//! <p> All of the graphics in a SectionDrawingModel are 2d elements. Some elements are
//! computed by intersecting the section plane with elements in some physical models according to some rules. These
//! elements are called "section graphics". Some elements in the SectionDrawingModel are computed by projecting element in
//! front the section plane onto the section plane according to some rules. These elements are called "forward graphics".
//! Other elements in the SectionDrawingModel are computed by projecting element behind the section plane onto the section
//! plane according to some rules. These elements are called "reverse graphics". Lastly, the SectionDrawingModel may contain
//! elements that are pure annotation placed by the user or created by other rules.
//! @ingroup DgnModelGroup
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SectionDrawingModel : PlanarPhysicalModel
    {
    DEFINE_T_SUPER(PlanarPhysicalModel)

public:
    SectionDrawingModel(CreateParams const& params) : T_Super(params) {}
    };

//=======================================================================================
//! A sheet model is a GraphicsModel2d that has the following characteristics:
//!     - Has fixed extents (is not infinite), specified in meters.
//!     - Can contain @b views of other models, like pictures pasted on a photo album.
//! @ingroup DgnModelGroup
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SheetModel : GraphicsModel2d
    {
    DEFINE_T_SUPER(GraphicsModel2d)

    struct CreateParams : GraphicsModel2d::CreateParams
    {
        DEFINE_T_SUPER(GraphicsModel2d::CreateParams);
        DPoint2d m_size;

        //! Parameters for creating a new SheetModel.
        //! @param[in] dgndb the DgnDb into which the SheetModel will be created
        //! @param[in] classId the DgnClassId of thew new SheetModel (must be or derive from SheetModel)
        //! @param[in] name the name of the new SheetModel
        //! @param[in] size the size of the SheetModel, in meters.
        //! @param[in] props the Properties of the new SheetModel
        //! @param[in] id the DgnModelId of thew new SheetModel. This should be DgnModelId() when creating a new model.
        CreateParams(DgnDbR dgndb, DgnClassId classId, Utf8CP name, DPoint2d size, Properties props=Properties(), DgnModelId id=DgnModelId()) :
            T_Super(dgndb, classId, name, props, id), m_size(size) {}

        explicit CreateParams(DgnModel::CreateParams const& params, DPoint2d size=DPoint2d::FromZero()) : T_Super(params), m_size(size) {}
    };

protected:
    DPoint2d        m_size;

    DgnModelType _GetModelType() const override {return DgnModelType::Sheet;}
    SheetModelCP _ToSheetModel() const override {return this;}
    DgnModels::Model::CoordinateSpace _GetCoordinateSpace() const override {return DgnModels::Model::CoordinateSpace::Local;}

    DGNPLATFORM_EXPORT virtual void _ToPropertiesJson(Json::Value&) const override;
    DGNPLATFORM_EXPORT virtual void _FromPropertiesJson(Json::Value const&) override;

public:
    //! construct a new SheetModel
    explicit DGNPLATFORM_EXPORT SheetModel(CreateParams const& params);

    //! Construct a SheetModel
    //! @param[in] params The CreateParams for the new SheetModel
    DGNPLATFORM_EXPORT static SheetModelPtr Create(CreateParams const& params) {return new SheetModel(params);}

    //! Get the sheet size, in meters
    DPoint2d GetSize() const {return m_size;}
    };

#define MODELHANDLER_DECLARE_MEMBERS(__ECClassName__,__classname__,_handlerclass__,_handlersuperclass__,__exporter__) \
        private: virtual DgnModel* _CreateInstance(DgnModel::CreateParams const& params) override {return new __classname__(__classname__::CreateParams(params));}\
        DOMAINHANDLER_DECLARE_MEMBERS(__ECClassName__,_handlerclass__,_handlersuperclass__,__exporter__)

//=======================================================================================
//! @namespace BentleyApi::Dgn::dgn_ModelHandler DgnModel Handlers in the base "Dgn" domain. 
//! @note Only handlers from the base "Dgn" domain belong in this namespace.
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
namespace dgn_ModelHandler
{
    //! The ModelHandler for DgnModel
    struct EXPORT_VTABLE_ATTRIBUTE Model : DgnDomain::Handler
    {
        DOMAINHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_Model, Model, DgnDomain::Handler, DGNPLATFORM_EXPORT)

    protected:
        ModelHandlerP _ToModelHandler() override {return this;}
        virtual DgnModelP _CreateInstance(DgnModel::CreateParams const& params) {return nullptr;}

    public:
        //! Find an ModelHandler for a subclass of dgn.Model. This is just a shortcut for FindHandler with the base class
        //! of "dgn.Model".
        DGNPLATFORM_EXPORT static ModelHandlerP FindHandler(DgnDb const&, DgnClassId handlerId);

        //! Create an instance of a (subclass of) DgnModel from CreateParams.
        //! @param[in] params the parameters for the model
        DgnModelPtr Create(DgnModel::CreateParams const& params) {return _CreateInstance(params);}
    };

    //! The ModelHandler for PhysicalModel
    struct EXPORT_VTABLE_ATTRIBUTE Physical : Model
    {
        MODELHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_PhysicalModel, PhysicalModel, Physical, Model, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for GraphicsModel2d
    struct EXPORT_VTABLE_ATTRIBUTE Graphics2d : Model
    {
        MODELHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_GraphicsModel2d, GraphicsModel2d, Graphics2d, Model, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for PlanarPhysicalModel
    struct EXPORT_VTABLE_ATTRIBUTE PlanarPhysical : Model
    {
        MODELHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_PlanarPhysicalModel, PlanarPhysicalModel, PlanarPhysical, Model, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for SectionDrawingModel
    struct EXPORT_VTABLE_ATTRIBUTE SectionDrawing : PlanarPhysical
    {
        MODELHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_SectionDrawingModel, SectionDrawingModel, SectionDrawing, PlanarPhysical, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for SheetModel
    struct EXPORT_VTABLE_ATTRIBUTE Sheet : Model
    {
        MODELHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_SheetModel, SheetModel, Sheet, Model, DGNPLATFORM_EXPORT)
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
