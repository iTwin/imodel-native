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

typedef bset<DgnElementP> T_StdDgnElementSet;

//=======================================================================================
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


//========================================================================================
//! Application-defined object that is stored with a DgnModel. This object is notified as significant events occur
//! for its "host" DgnModel. By creating a subclass of this class, applications can maintain relevant information about
//! a DgnModel with the DgnModel for efficient lookup and lifecycle management.
//! @see DgnModel::AddAppData
//! @bsiclass
//=======================================================================================
struct DgnModelAppData
    {
    virtual ~DgnModelAppData() {}

    //! A unique Key to identify each subclass of DgnModelAppData.
    struct Key : BeSQLite::AppDataKey {};

    //! Override this method to be notified when host DgnModel is about to be deleted from memory or when this appdata is being dropped from the host model.
    //! @note The persistent DgnModel is not being deleted, just this in-memory copy of it.
    virtual void _OnCleanup (DgnModelR host) = 0;

    //! Override this method to be notified after host DgnModel has been filled.
    virtual void _OnFilled (DgnModelR host) {}

    //! Override this method to be notified when host DgnModel is about to be emptied.
    //! @return true to be dropped from host (_OnCleanup will be called.)
    virtual bool _OnEmpty (DgnModelR host) {return true;}

    //! Override this method to be notified after host DgnModel has been emptied. Won't be called unless #_OnEmpty returns false.
    virtual bool _OnEmptied (DgnModelR host) {return false;}

    //! Override this method to be notified before the persistent MicroStationModel is marked for delete. The actual delete happens when the DgnFile
    //! holding the DgnModel is closed.
    virtual void _OnModelDelete (DgnModelR host) {}

    //! Override this method to be notified after the persistent MicroStationModel is undeleted.
    virtual void _OnModelUnDelete (DgnModelR host) {}
    };

//=======================================================================================
//! A DgnModel represents a model in memory and may hold references to elements that belong to it.
//! @bsiclass                                                     KeithBentley    10/00
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnModel : RefCountedBase
    {
    friend struct DgnModels;
    friend struct DgnElement;
    friend struct DgnElementIterator;
    friend struct DgnElements;

    //=======================================================================================
    //! The properties for a DgnModel.
    //! @bsiclass
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
            void FromJson (Json::Value const& inValue);
            void ToJson(Json::Value& outValue) const;
        };

        FormatterFlags   m_formatterFlags;               //!< Flags saved on "save settings"
        UnitDefinition   m_masterUnit;                   //!< Master Unit information
        UnitDefinition   m_subUnit;                      //!< Sub Unit information
        double           m_roundoffUnit;                 //!< unit lock roundoff val in uors
        double           m_roundoffRatio;                //!< Unit roundoff ratio y to x (if 0 use Grid Ratio)
        double           m_formatterBaseDir;             //!< Base Direction used for Direction To/From String

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
            m_subUnit.Init (UnitBase::Meter, UnitSystem::Metric, 1.0, 1.0, L"m");
            m_masterUnit = m_subUnit;
            }

        void FromJson (Json::Value const& inValue);
        void ToJson(Json::Value& outValue) const;

        //! Set working-units and sub-units. Units must be valid and comparable.
        DGNPLATFORM_EXPORT BentleyStatus SetWorkingUnits (UnitDefinitionCR newMasterUnit, UnitDefinitionCR newSubUnit);
        void SetLinearUnitMode (DgnUnitFormat value) { m_formatterFlags.m_linearUnitMode = (uint32_t)value; }
        void SetLinearPrecision (PrecisionFormat value) {
                m_formatterFlags.m_linearPrecType  = static_cast<uint32_t>(DoubleFormatter::GetTypeFromPrecision (value));
                m_formatterFlags.m_linearPrecision = DoubleFormatter::GetByteFromPrecision (value);}
        void SetAngularMode (AngleMode value) { m_formatterFlags.m_angularMode = (uint32_t)value; }
        void SetAngularPrecision (AnglePrecision value) { m_formatterFlags.m_angularPrecision = (uint32_t)value; }
        void SetDirectionMode (DirectionMode value) {m_formatterFlags.m_directionMode = (uint32_t)value; }
        void SetDirectionClockwise (bool value) { m_formatterFlags.m_directionClockwise = value; }
        void SetDirectionBaseDir (double value) { m_formatterBaseDir = value; }
        DgnUnitFormat GetLinearUnitMode() const {return (DgnUnitFormat) m_formatterFlags.m_linearUnitMode; }
        PrecisionFormat GetLinearPrecision() const {return DoubleFormatter::ToPrecisionEnum ((PrecisionType) m_formatterFlags.m_linearPrecType, m_formatterFlags.m_linearPrecision); }
        AngleMode GetAngularMode() const {return (AngleMode) m_formatterFlags.m_angularMode; }
        AnglePrecision GetAngularPrecision() const {return (AnglePrecision) m_formatterFlags.m_angularPrecision; }
        DirectionMode GetDirectionMode() const {return (DirectionMode) m_formatterFlags.m_directionMode; }
        bool GetDirectionClockwise() const {return m_formatterFlags.m_directionClockwise; }
        double GetDirectionBaseDir() const {return m_formatterBaseDir; }
        void SetRoundoffUnit (double roundoffUnit, double roundoffRatio) {m_roundoffUnit  = roundoffUnit;m_roundoffRatio = roundoffRatio;}
        double GetRoundoffUnit() const {return m_roundoffUnit;}
        double GetRoundoffRatio() const {return m_roundoffRatio;}
        FormatterFlags GetFormatterFlags() const    {return m_formatterFlags;}
        UnitDefinitionCR GetMasterUnit() const {return m_masterUnit;}
        UnitDefinitionCR GetSubUnit() const {return m_subUnit;}
    };

    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct CreateParams
    {
    DgnDbR      m_dgndb;
    DgnModelId  m_id;
    DgnClassId  m_classId;
    Utf8String  m_name;
    CreateParams(DgnDbR dgndb, DgnClassId classId, Utf8CP name, DgnModelId id=DgnModelId()) : m_dgndb(dgndb), m_id(id), m_classId(classId), m_name(name) {}
    };
    
protected:
    typedef BeSQLite::AppDataList<DgnModelAppData, DgnModelAppData::Key, DgnModelR> T_AppDataList;

    DgnDbR          m_dgndb;
    DgnModelId      m_modelId;
    DgnClassId      m_classId;
    T_AppDataList   m_appData;
    Utf8String      m_name;
    Properties      m_properties;
    DgnElementMap   m_elements;
    mutable DgnRangeTreeP m_rangeIndex;
    bool            m_wasFilled;    // true if the list was filled from db
    bool            m_readonly;     // true if this model is from a read-only file.

    explicit DGNPLATFORM_EXPORT DgnModel(CreateParams const&);
    DGNPLATFORM_EXPORT virtual ~DgnModel();

    DGNPLATFORM_EXPORT virtual DgnModelPtr Duplicate(Utf8CP newName) const;
    DGNPLATFORM_EXPORT virtual void _InitFrom(DgnModelCR other);
    virtual DgnModelType _GetModelType() const = 0;
    virtual DgnModels::Model::CoordinateSpace _GetCoordinateSpace() const = 0;
    DGNPLATFORM_EXPORT virtual AxisAlignedBox3d _QueryModelRange() const;
    virtual bool _Is3d() const = 0;
    DGNPLATFORM_EXPORT virtual void _ToPropertiesJson(Json::Value&) const;
    DGNPLATFORM_EXPORT virtual void _FromPropertiesJson(Json::Value const&);
    DGNPLATFORM_EXPORT virtual DPoint3d _GetGlobalOrigin() const;
    DGNPLATFORM_EXPORT virtual DgnModelStatus _OnUpdateElement(DgnElementCR original, DgnElementR replacement);
    DGNPLATFORM_EXPORT virtual void _OnUpdatedElement(DgnElementCR element);
    DGNPLATFORM_EXPORT virtual void _OnUpdateElementFailed(DgnElementCR original);
    DGNPLATFORM_EXPORT virtual DgnModelStatus _OnInsertElement(DgnElementR element);
    DGNPLATFORM_EXPORT virtual DgnModelStatus _OnDeleteElement(DgnElementCR element);
    DGNPLATFORM_EXPORT virtual void _OnLoadedElement(DgnElementCR el);
    DGNPLATFORM_EXPORT virtual void _OnInsertedElement(DgnElementCR el);
    DGNPLATFORM_EXPORT virtual void _OnDeletedElement(DgnElementCR element);
    DGNPLATFORM_EXPORT virtual void _OnModelFillComplete();
    virtual DgnModel2dCP _ToDgnModel2d() const {return nullptr;}
    virtual DgnModel3dCP _ToDgnModel3d() const {return nullptr;}
    virtual PhysicalModelCP _ToPhysicalModel() const {return nullptr;}
    virtual PlanarPhysicalModelCP _ToPlanarPhysicalModel() const {return nullptr;}
    virtual SheetModelCP _ToSheetModel() const {return nullptr;}

    void RegisterElement(DgnElementCR);
    void SetFilled() {m_wasFilled=true; AllocateRangeIndex();}
    void AllocateRangeIndex() const;
    void ClearRangeIndex();
    void ReleaseAllElements();
    void AddToRangeIndex(DgnElementCR);
    void RemoveFromRangeIndex(DgnElementCR);

public:
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

    bool NotifyOnEmpty();

    DGNPLATFORM_EXPORT ModelHandler& GetModelHandler() const;

    DGNPLATFORM_EXPORT DgnRangeTree* GetRangeIndexP(bool create) const;
    void ReadProperties();
    DGNPLATFORM_EXPORT BeSQLite::DbResult SaveProperties();

    DGNPLATFORM_EXPORT void ClearAllDirtyFlags();
    void ClearAllQvElems();

    DGNPLATFORM_EXPORT double GetMillimetersPerMaster() const;
    DGNPLATFORM_EXPORT double GetSubPerMaster() const;
    DGNPLATFORM_EXPORT DPoint3d GetGlobalOrigin() const;

    /** @name Filling */
    /** @{ */
    //! Empty the contents of this model.
    DGNPLATFORM_EXPORT void Empty();

    //! Make sure this model is filled.
    DGNPLATFORM_EXPORT DgnFileStatus FillModel();

    //! Determine whether this model has been "filled" from disk or not.
    //! @return true if the model is filled.
    bool IsFilled() const {return m_wasFilled;}

    //! Get the number of elements in this model.
    //! @return the number of elements in this model.
    //! @note The model must be filled before calling this method.
    //! @see FillSections
    uint32_t CountElements() const {return (uint32_t) m_elements.size();}
    /** @} */

    /** @name Finding Elements */
    /** @{ */
    //! Find a DgnElementP in this model by DgnElementId.
    //! @return DgnElementP of element with \a id, or NULL.
    DGNPLATFORM_EXPORT DgnElementCP FindElementById(DgnElementId id);
    /** @} */

    //! Query if this is a 3D model
    bool Is3d() const {return _Is3d();}

    //! Get the range of all visible elements in the model.
    AxisAlignedBox3d QueryModelRange() const {return _QueryModelRange();}

    //! Get the Properties for this model.
    Properties& GetPropertiesR() {return m_properties;}

    //! Get the Properties for this model.
    Properties const& GetProperties() const {return m_properties;}

    //! Get the name of this model
    Utf8CP GetModelName() const {return m_name.c_str();}
    DgnModelType GetModelType() const {return _GetModelType();}
    DgnClassId GetClassId() const {return m_classId;}
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

    /** @name DgnModelAppData */
    /** @{ */
    //! Add (or replace) appData to this model.
    //! @return SUCCESS if appData was successfully added. Note that it is illegal to add or remove AppData from within
    //! any of the AppData "_On" methods. If an entry with \a key already exists, it will be dropped and replaced with \a appData.
    DGNPLATFORM_EXPORT StatusInt AddAppData(DgnModelAppData::Key const& key, DgnModelAppData* appData);

    //! @return SUCCESS if appData with key is found and was dropped.
    //! @remarks Calls the app data object's _OnCleanup method.
    DGNPLATFORM_EXPORT StatusInt DropAppData(DgnModelAppData::Key const& key);

    //! Search for appData on this model that was added with the specified key.
    //! @return the DgnModelAppData with \a key, or nullptr.
    DGNPLATFORM_EXPORT DgnModelAppData* FindAppData(DgnModelAppData::Key const& key) const;
    /** @} */

    typedef DgnElementMap::const_iterator const_iterator;
    DgnElementMap const& GetElements() const {return m_elements;}
    bool IsEmpty() const {return (begin() != end());}
    const_iterator begin() const {return m_elements.begin();}
    const_iterator end() const {return m_elements.end();}
};

//=======================================================================================
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
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct DgnModel2d : DgnModel
    {
    DEFINE_T_SUPER(DgnModel)

protected:
    DPoint2d m_globalOrigin;    //!< Global Origin - all coordinates are offset by this value.

    bool _Is3d() const override {return false;}
    DGNPLATFORM_EXPORT void _ToPropertiesJson(Json::Value&) const override;
    DGNPLATFORM_EXPORT void _FromPropertiesJson(Json::Value const&) override;
    DPoint3d _GetGlobalOrigin() const override {return DPoint3d::From(m_globalOrigin);}
    DgnModel2dCP _ToDgnModel2d() const override {return this;}

    DGNPLATFORM_EXPORT virtual DgnModelStatus _OnInsertElement(DgnElementR element);

public:
    void SetGlobalOrigin(DPoint2dCR org) {m_globalOrigin = org;}

    explicit DgnModel2d(CreateParams const& params, DPoint2dCR origin=DPoint2d::FromZero()) : T_Super(params), m_globalOrigin(origin) {}
    };

//=======================================================================================
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
//! @private
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct ComponentModel : DgnModel3d
{
    DEFINE_T_SUPER(DgnModel3d)

protected:
    DgnModelType _GetModelType() const override {return DgnModelType::Component;}
    DPoint3d _GetGlobalOrigin() const override {return DPoint3d::FromZero();}

public:
    explicit ComponentModel(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A PlanarPhysicalModel is a infinite planar model that is positioned in physical space.
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct PlanarPhysicalModel : DgnModel2d
{
    DEFINE_T_SUPER(DgnModel2d)

    Transform m_worldTrans;  // positions XY model coordinates into XYZ physical coordinates

protected:
    void _FromPropertiesJson(Json::Value const&) override;
    void _ToPropertiesJson(Json::Value&) const override;
    DgnModelType _GetModelType() const override {return DgnModelType::Drawing;}
    PlanarPhysicalModelCP _ToPlanarPhysicalModel() const override {return this;}

public:
    PlanarPhysicalModel(CreateParams const& params) : T_Super(params) {m_worldTrans.InitIdentity();}

    Transform GetTransformToWorld() const {return m_worldTrans;} //!< Returns the transform FROM a local coordinate system TO world coordinates.
    void SetTransformToWorld(TransformCR trans) {m_worldTrans=trans;}
};

//=======================================================================================
//! A SectionDrawingModel represents a plane in physical space. The SectionDrawingModel contains
//! graphics that were computed in some way by intersecting physical models with the drawing model plane.
//! SectionDrawingModel can have one or more convex clip plane sets that can be used to clip a
//! physical view in order to display the drawing.
//! SectionDrawingModels can also contain 2-D annotation elements.
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct SectionDrawingModel : PlanarPhysicalModel
    {
    DEFINE_T_SUPER(PlanarPhysicalModel)

protected:
    DRange1d m_zrange; // range of Z values of non-planar proxy graphics such as "forward" visible edges in drawing's LCS.
    double m_annotationScale; // the intended viewing scale of annotations in this drawing

    void _FromPropertiesJson(Json::Value const&) override;
    void _ToPropertiesJson(Json::Value&) const override;
    DRange1d GetZRange() const {return m_zrange;}

public:
    SectionDrawingModel(CreateParams const& params) : T_Super(params)
        {
        m_zrange.InitNull();
        m_annotationScale = 1.0;
        }

    //! Query and load the sectioning view that was used to generate this drawing.
    //! @return an invalid pointer if this drawing was not generated or if the section view cannot be found or loaded.
    DGNPLATFORM_EXPORT SectioningViewControllerPtr GetSourceView();

public:

    Transform GetFlatteningMatrix (double zdelta = 0.0) const; //!< Get the matrix to use when viewing this model. It ensures that all elements lie in the LCS plane.
    void AddToZRange (double z) {m_zrange.Extend(z);} //!< Report the Z value of non-planar proxy graphics such as "forward" visible edges in drawing's LCS.

    //! Set the scale of annotations in this drawing
    void SetAnnotationScale (double s) {m_annotationScale = s;}

    //! Get the scale of annotations in this drawing
    double GetAnnotationScale() const {return m_annotationScale;}
    };

//=======================================================================================
//! A sheet model is a 2-D model that has the following characteristics:
//! -- Has fixed extents (is not infinite).
//! -- Does not correspond to any location in physical space.
//! -- Likewise, locations and distances in the SheetModel have no physical meaning, except measurements of the sheet itself.
//! -- Its units are generally "paper" units, e.g., inches or mm. Its units bear no relation to the units of the physical models that may be portrayed on the sheet.
//! -- Like any 2-D model, a sheet model can contain only planar geometry and annotations.
//! -- Can embed \em views of other models. Embedded views are pictures only.
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct SheetModel : DgnModel2d
    {
    DEFINE_T_SUPER(DgnModel2d)

    friend struct SheetModelHandler;

public:
    enum class Units {Millimeters, Inches, Custom};

protected:
    DPoint2d        m_size;

    DgnModelType _GetModelType() const override {return DgnModelType::Sheet;}
    SheetModelCP _ToSheetModel() const override {return this;}
    DgnModels::Model::CoordinateSpace _GetCoordinateSpace() const override {return DgnModels::Model::CoordinateSpace::Local;}

    DGNPLATFORM_EXPORT virtual void _ToPropertiesJson(Json::Value&) const override;
    DGNPLATFORM_EXPORT virtual void _FromPropertiesJson(Json::Value const&) override;

    //! construct a SheetModel object, prior to loading it from a Db
    DGNPLATFORM_EXPORT SheetModel(CreateParams const& params);

    //! construct a SheetModel object, as part of creating a new SheetModel in the Db.
    //! @param db       The DgnDb that will hold the new model
    //! @param name     The name of the new model
    //! @param size     The fixed extents of the sheet (expressed in the specified units)
    //! @param units    The sheet's units.
    //! @note if \a units is Units::Custom, then you must follow up with a call to set the "WorkingUnits" on the model's properties.
    DGNPLATFORM_EXPORT SheetModel(DgnDbR db, Utf8CP name, DPoint2dCR size, Units units);

public:
    //! construct a SheetModel object, as part of creating a new SheetModel in the Db.
    //! @param db       The DgnDb that will hold the new model
    //! @param name     The name of the new model
    //! @param size     The fixed extents of the sheet (expressed in the specified units)
    //! @param units    The sheet's units.
    //! @note if \a units is Units::Custom, then you must follow up with a call to set the "WorkingUnits" on the model's properties.
    DGNPLATFORM_EXPORT static SheetModelPtr Create(DgnDbR db, Utf8CP name, DPoint2dCR size, Units units) {return new SheetModel(db,name,size,units);}

    //! get the sheet size
    DPoint2d GetSize() const {return m_size;}
    };


#define MODELHANDLER_DECLARE_MEMBERS(__ECClassName__,__classname__,_handlerclass__,_handlersuperclass__,__exporter__) \
        private: virtual DgnModel* _CreateInstance(DgnModel::CreateParams const& params) override {return new __classname__(params);}\
        DOMAINHANDLER_DECLARE_MEMBERS(__ECClassName__,_handlerclass__,_handlersuperclass__,__exporter__)

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ModelHandler : DgnDomain::Handler
{
    DOMAINHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_Model, ModelHandler, DgnDomain::Handler, DGNPLATFORM_EXPORT)

protected:
    ModelHandlerP _ToModelHandler() override {return this;}
    virtual DgnModelP _CreateInstance(DgnModel::CreateParams const& params) {return nullptr;}

public:
    //! Find an ModelHandler for a subclass of dgn.Model. This is just a shortcut for FindHandler with the base class
    //! of "dgn.Model".
    DGNPLATFORM_EXPORT static ModelHandlerP FindHandler(DgnDb const&, DgnClassId handlerId);

    //! Create an instance of a DgnModel for a Model.
    //! @param[in] params the parameters for the model
    DgnModelPtr Create(DgnModel::CreateParams const& params) {return _CreateInstance(params);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalModelHandler : ModelHandler
{
    MODELHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_PhysicalModel, PhysicalModel, PhysicalModelHandler, ModelHandler, DGNPLATFORM_EXPORT)
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

