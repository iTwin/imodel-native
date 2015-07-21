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
DGNPLATFORM_TYPEDEFS (SheetModel)
DGNPLATFORM_REF_COUNTED_PTR(SheetModel)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

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
    friend struct dgn_TxnTable::Model;

    struct CreateParams;

    //========================================================================================
    //! Specifies the solver to invoke when changes to a model or its contents are validated.
    //=======================================================================================
    struct Solver
        {
        friend struct DgnModel;
        friend struct CreateParams;

        //! Identifies the type of solver used by a model
        enum class Type 
            {
            None=0,     //!< This model has no solver
            Script,     //!< Execute a named JavaScript function
            // *** TBD: Add built-in constraint solvers 
            };

      private:
        Type        m_type;
        Utf8String  m_name;
        Json::Value m_parameters;

      protected:
        void FromJson(Utf8CP);
        Utf8String ToJson() const;

        void Solve(DgnModelR);

      public:
        //! @private
        Solver() {m_type = Type::None;}

        //! Construct a Solver specification, in preparation for creating a new DgnModel.
        Solver(Type t, Utf8CP n, Json::Value const& p) : m_type(t), m_name(n), m_parameters(p) {;}

        Type GetType() const {return m_type;}
        Utf8StringCR GetName() const {return m_name;}
        Json::Value const& GetParameters() const {return m_parameters;}
        Json::Value& GetParametersR() {return m_parameters;}
        };

    //========================================================================================
    //! Application data attached to a DgnModel. Create a subclass of this to store non-persistent information on a DgnModel and
    //! to react to significant events on a DgnModel.
    //! @see DgnModel::AddAppData
    //=======================================================================================
    struct AppData : RefCountedBase
        {
        //! A unique Key to identify each subclass of AppData.
        struct Key : NonCopyableClass {};

        enum class DropMe {No=0, Yes=1};

        //! Called after DgnModel has been filled.
        //! @param[in] model The model to which this AppData is attached
        //! @return DropMe::Yes to be removed from DgnModel
        virtual DropMe _OnFilled(DgnModelCR model) {return DropMe::No;}

        //! Called when a DgnModel is about to be emptied.
        //! @param[in] model The model to which this AppData is attached
        //! @return true to be dropped from model
        virtual void _OnEmpty(DgnModelCR model) {}

        //! Called after a DgnModel has been emptied.
        //! @param[in] model The model to which this AppData is attached
        //! @return DropMe::Yes to be removed from DgnModel
        virtual DropMe _OnEmptied(DgnModelCR model) {return DropMe::No;}

        //! Called when a DgnModel is about to be updated in the DgnDb.
        //! @param[in] model The model to which this AppData is attached
        virtual DgnDbStatus _OnUpdate(DgnModelCR model) {return DgnDbStatus::Success;}

        //! Called after a DgnModel was updated in the DgnDb.
        //! @param[in] model The model to which this AppData is attached
        //! @return DropMe::Yes to be removed from DgnModel
        virtual DropMe _OnUpdated(DgnModelCR model) {return DropMe::No;}

        //! Called before the DgnModel is deleted.
        //! @param[in] model The model to which this AppData is attached
        virtual void _OnDelete(DgnModelR model) {}

        //! Called after the DgnModel was deleted.
        //! @param[in] model The model to which this AppData is attached
        //! @return DropMe::Yes to be removed from DgnModel
        virtual DropMe _OnDeleted(DgnModelCR model) {return DropMe::Yes;}
    };

    //=======================================================================================
    //! The properties for a DgnModel. These are stored as a JSON string in the "Props" column of the DgnModel table.
    //! These properties are saved by calling DgnModel::Update
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
        double         m_roundoffUnit;                 //!< unit lock roundoff val
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
            m_roundoffRatio = 0;
            m_formatterBaseDir = 0;
            m_roundoffUnit = 0;
            m_subUnit.Init(UnitBase::Meter, UnitSystem::Metric, 1.0, 1.0, L"m");
            m_masterUnit = m_subUnit;
            }

        void FromJson(Json::Value const& inValue);
        void ToJson(Json::Value& outValue) const;

        //! Set master units and sub-units. Units must be valid and comparable.
        DGNPLATFORM_EXPORT BentleyStatus SetUnits(UnitDefinitionCR newMasterUnit, UnitDefinitionCR newSubUnit);
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

        //! Get the master units for this DgnModel.
        //! Master units are the major display units for coordinates in a DgnModel (e.g. "Meters", or "Feet").
        //! @see SetUnits, GetSubUnits
        UnitDefinitionCR GetMasterUnits() const {return m_masterUnit;}

        //! Get the sub-units for this DgnModel.
        //! Sub units are the minor readout units for coordinates in a DgnModel (e.g. "Centimeters, or "Inches").
        //! @see SetUnits, GetMasterUnits
        UnitDefinitionCR GetSubUnits() const {return m_subUnit;}

        //! Get the number of millimeters per master unit.
        //! @see GetMasterUnits
        DGNPLATFORM_EXPORT double GetMillimetersPerMaster() const;

        //! Get the number of sub units per master unit.
        //! @see GetSubUnits
        DGNPLATFORM_EXPORT double GetSubPerMaster() const;
    };

    //=======================================================================================
    //! Parameters to create a new instances of a DgnModel.
    //! @ingroup DgnModelGroup
    //=======================================================================================
    struct CreateParams
    {
        DgnDbR      m_dgndb;
        DgnModelId  m_id;
        DgnClassId  m_classId;
        Utf8String  m_name;
        Properties  m_props;
        Solver      m_solver;
        //! Parameters to create a new instance of a DgnModel.
        //! @param[in] dgndb The DgnDb for the new DgnModel
        //! @param[in] classId The DgnClassId for the new DgnModel.
        //! @param[in] name The name for the DgnModel
        //! @param[in] props The properties for the new DgnModel.
        //! @param[in] solver The definition of the solver to be used by this model when validating changes to its content.
        //! @param[in] id Internal only, must be DgnModelId() to create a new DgnModel.
        CreateParams(DgnDbR dgndb, DgnClassId classId, Utf8CP name, Properties props=Properties(), Solver solver=Solver(), DgnModelId id=DgnModelId()) :
            m_dgndb(dgndb), m_id(id), m_classId(classId), m_name(name), m_props(props), m_solver(solver) {}
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
    Solver          m_solver;
    DgnElementMap   m_elements;
    mutable bmap<AppData::Key const*, RefCountedPtr<AppData>, std::less<AppData::Key const*>, 8> m_appData;
    mutable DgnRangeTreeP m_rangeIndex;
    mutable bool    m_persistent;   // true if this DgnModel is in the DgnModels "loaded models" list.
    bool            m_filled;       // true if the FillModel was called on this DgnModel.

    explicit DGNPLATFORM_EXPORT DgnModel(CreateParams const&);
    DGNPLATFORM_EXPORT virtual ~DgnModel();

    DGNPLATFORM_EXPORT virtual void _InitFrom(DgnModelCR other);            //!< @private
    virtual DgnModelType _GetModelType() const = 0; //!< @private
    virtual DgnModels::Model::CoordinateSpace _GetCoordinateSpace() const = 0; //!< @private
    DGNPLATFORM_EXPORT virtual AxisAlignedBox3d _QueryModelRange() const;//!< @private
    virtual bool _Is3d() const = 0;//!< @private
    DGNPLATFORM_EXPORT virtual void _ToPropertiesJson(Json::Value&) const;//!< @private
    DGNPLATFORM_EXPORT virtual void _FromPropertiesJson(Json::Value const&);//!< @private

    //! Get the Global Origin for this DgnMode.
    //! The global origin is on offset that is added to all coordinate values stored in this model.
    DGNPLATFORM_EXPORT virtual DPoint3d _GetGlobalOrigin() const;//!< @private

    /** @name Events associated with DgnElements of a DgnModel */
    /** @{ */
    //! Called when a DgnElement in this DgnModel is about to be inserted.
    //! @param[in] element The element about to be inserted into the DgnDb
    //! @return DgnDbStatus::Success to allow the element to be added. Any other status will block the insert and will be
    //! returned to the caller attempting to insert the element.
    //! @note If you override this method, you @em must call the T_Super implementation, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element);

    //! Called when a DgnElement in this DgnModel is about to be updated.
    //! @param[in] modified The element in its changed state. This state will be saved to the DgnDb
    //! @param[in] original The element in its pre-changed state.
    //! @return DgnDbStatus::Success to allow the element to be updated. Any other status will block the update and will be
    //! returned to the caller attempting to update the element.
    //! @note If you override this method, you @em must call the T_Super implementation, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnUpdateElement(DgnElementCR modified, DgnElementCR original);

    //! Called when a DgnElement in this DgnModel is about to be deleted.
    //! @param[in] element The element about to be deleted from the DgnDb
    //! @return DgnDbStatus::Success to allow the element to be deleted. Any other status will block the delete and will be
    //! returned to the caller attempting to delete the element.
    //! @note If you override this method, you @em must call the T_Super implementation, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnDeleteElement(DgnElementCR element);

    //! Called after a DgnElement in this DgnModel has been loaded into memory.
    //! @param[in] element The element that was just loaded.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    DGNPLATFORM_EXPORT virtual void _OnLoadedElement(DgnElementCR element);

    //! Called after a DgnElement in this DgnModel has been inserted into the DgnDb
    //! @param[in] element The element that was just inserted.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    DGNPLATFORM_EXPORT virtual void _OnInsertedElement(DgnElementCR element);

    //! Called after a DgnElement in this DgnModel has been updated in the DgnDb
    //! @param[in] modified The element in its changed state. This state was saved to the DgnDb
    //! @param[in] original The element in its pre-changed state.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    DGNPLATFORM_EXPORT virtual void _OnUpdatedElement(DgnElementCR modified, DgnElementCR original);

    //! Called after a DgnElement in this DgnModel has been deleted from the DgnDb
    //! @param[in] element The element that was just deleted.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    DGNPLATFORM_EXPORT virtual void _OnDeletedElement(DgnElementCR element);
    /** @} */

    //! Load all of the DgnElements of this DgnModel into memory.
    DGNPLATFORM_EXPORT virtual void _FillModel();

    /** @name Events for a DgnModel */
    /** @{ */
    //! Called when this DgnModel is about to be inserted into the DgnDb.
    //! @note If you override this method, you @em must call the T_Super implementation, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert();
    //! Called when this DgnModel is about to be updated in the DgnDb.
    //! @note If you override this method, you @em must call the T_Super implementation, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnUpdate();
    //! Called when this DgnModel is about to be deleted from the DgnDb.
    //! @note If you override this method, you @em must call the T_Super implementation, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnDelete();
    //! Called after this DgnModel was loaded from the DgnDb.
    //! @note If you override this method, you @em must call the T_Super implementation.
    DGNPLATFORM_EXPORT virtual void _OnLoaded();
    //! Called after this DgnModel was inserted into the DgnDb.
    //! @note If you override this method, you @em must call the T_Super implementation.
    DGNPLATFORM_EXPORT virtual void _OnInserted();
    //! Called after this DgnModel was updated in the DgnDb.
    //! @note If you override this method, you @em must call the T_Super implementation.
    DGNPLATFORM_EXPORT virtual void _OnUpdated();
    //! Called after this DgnModel was deleted from the DgnDb.
    //! @note If you override this method, you @em must call the T_Super implementation.
    DGNPLATFORM_EXPORT virtual void _OnDeleted();
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
    //! <h2>Coordinate Systems</h2>
    //! A DgnDb defines a single physical coordinate system. 
    //! A DgnDb is associated with a single Geographic Coordinate System (GCS). See DgnUnits::GetDgnGCS.
    //! Graphics in the scene must be defined in the DgnDb's coordinate system.
    //! The implementation must transform external data into the coordinate system of the DgnDb as necessary before adding graphics to the scene.
    //! <h2>Displaying external data using progressive display</h2>
    //! An implementation of _AddGraphicsToScene is required to be very fast. If some external data is not immediately available, then the implementation should
    //! a) make arrangements to obtain the data in the background and b) schedule itself for callbacks during progressive display in order to display the data when it becomes available.
    virtual void _AddGraphicsToScene(ViewContextR) {}
    void ReadProperties();

public:
    void AddGraphicsToScene(ViewContextR context) {_AddGraphicsToScene(context);}
    DGNPLATFORM_EXPORT ModelHandlerR GetModelHandler() const;

    DGNPLATFORM_EXPORT DgnRangeTree* GetRangeIndexP(bool create) const; //!< @private
    DGNPLATFORM_EXPORT DgnElementCP FindElementById(DgnElementId id); //!< @private

    //! Get the Global Origin for this DgnModel.
    //! The global origin is an offset that is added to all coordinate values of this DgnModel when reporting them to the user.
    //! @note all PhysicalModels have the same coordinate system and the same global origin.
    DPoint3d GetGlobalOrigin() const {return _GetGlobalOrigin();}

    //! Empty the contents of this DgnModel. This will release any references to DgnElements held by this DgnModel, decrementing
    //! their reference count and potentially freeing them.
    DGNPLATFORM_EXPORT void EmptyModel();

    //! Load all elements of this DgnModel.
    //! After this call, all of the DgnElements of this model are loaded and are held in memory by this DgnModel.
    //! @note if this DgnModel is already filled, this method does nothing and returns DgnDbStatus::Success.
    void FillModel() {_FillModel();}

    //! Determine whether this DgnModel's elements have been "filled" from the DgnDb or not.
    //! @return true if the DgnModel was filled.
    //! @see FillModel
    bool IsFilled() const {return m_filled;}

    //! Determine whether this DgnModel is persistent.
    //! A model is "persistent" if it was loaded via DgnModels::GetModel, or after it is inserted into the DgnDb via Insert.
    //! A newly created model before it is inserted, or a model after calling Delete, is not persistent.
    bool IsPersistent() const {return m_persistent;}

    //! Determine whether this is a 3d DgnModel
    bool Is3d() const {return _Is3d();}

    //! Get the AxisAlignedBox3d of the contents of this DgnModel.
    AxisAlignedBox3d QueryModelRange() const {return _QueryModelRange();}

    //! Get a writable reference to the Properties for this DgnModel.
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

    //! @name Dynamic casting to DgnModel subclasses
    //! @{
    DgnModel2dCP ToDgnModel2d() const {return _ToDgnModel2d();} //!< more efficient substitute for dynamic_cast<DgnModel2dCP>(model)
    DgnModel3dCP ToDgnModel3d() const {return _ToDgnModel3d();} //!< more efficient substitute for dynamic_cast<DgnModel3dCP>(model)
    PhysicalModelCP ToPhysicalModel() const {return _ToPhysicalModel();} //!< more efficient substitute for dynamic_cast<PhysicalModelCP>(model)
    PlanarPhysicalModelCP ToPlanarPhysicalModel() const {return _ToPlanarPhysicalModel();} //!< more efficient substitute for dynamic_cast<PlanarPhysicalModelCP>(model)
    SheetModelCP ToSheetModel() const {return _ToSheetModel();} //!< more efficient substitute for dynamic_cast<SheetModelCP>(model)
    DgnModel2dP ToDgnModel2dP() {return const_cast<DgnModel2dP>(_ToDgnModel2d());} //!< more efficient substitute for dynamic_cast<DgnModel2dP>(model)
    DgnModel3dP ToDgnModel3dP() {return const_cast<DgnModel3dP>(_ToDgnModel3d());} //!< more efficient substitute for dynamic_cast<DgnModel3dP>(model)
    PhysicalModelP ToPhysicalModelP() {return const_cast<PhysicalModelP>(_ToPhysicalModel());} //!< more efficient substitute for dynamic_cast<PhysicalModelP>(model)
    PlanarPhysicalModelP ToPlanarPhysicalModelP() {return const_cast<PlanarPhysicalModelP>(_ToPlanarPhysicalModel());} //!< more efficient substitute for dynamic_cast<PlanarPhysicalModelP>(model)
    SheetModelP ToSheetModelP() {return const_cast<SheetModelP>(_ToSheetModel());}//!< more efficient substitute for dynamic_cast<SheetModelP>(model)
    //! @}

    //! Get the DgnDb of this DgnModel.
    DgnDbR GetDgnDb() const {return m_dgndb;}

    //! Insert this model into the DgnDb.
    //! @return DgnDbStatus::Success if this model was successfully inserted, error otherwise.
    DGNPLATFORM_EXPORT DgnDbStatus Insert(Utf8CP description=nullptr, bool inGuiList=true);

    //! Delete this model from the DgnDb
    //! @note All elements from this model are deleted as well. This method will fail on the first element that cannot be successfully deleted.
    //! @return DgnDbStatus::Success if this model was successfully deleted, error otherwise. Note that if this method returns an error, it is possible
    //! that some elements may have been deleted. Therefore, you should always call DgnDb::AbandonChanges after a failure to avoid partial deletions.
    DGNPLATFORM_EXPORT DgnDbStatus Delete();

    //! Update the Properties of this model in the DgnDb
    //! @return DgnDbStatus::Success if the properties of this model were successfully updated, error otherwise.
    DGNPLATFORM_EXPORT DgnDbStatus Update();

    /** @name DgnModel AppData */
    /** @{ */
    //! Add (or replace) AppData on this DgnModel.
    //! @note It is illegal to add or remove AppData from within
    //! any of the AppData "_OnXxx" methods. If an entry with \a key already exists, it will be dropped and replaced with \a appData.
    DGNPLATFORM_EXPORT void AddAppData(AppData::Key const& key, AppData* appData);

    //! Remove AppData from this DgnModel
    //! @return SUCCESS if appData with key is found and was dropped.
    //! @remarks Calls the object's _OnCleanup method.
    DGNPLATFORM_EXPORT StatusInt DropAppData(AppData::Key const& key);

    //! Search for AppData on this model by AppData::Key.
    //! @return the AppData with \a key, or nullptr.
    DGNPLATFORM_EXPORT AppData* FindAppData(AppData::Key const& key) const;
    /** @} */

    /** @name Solver The Model Solver */
    /** @{ */
    //! Get the solver that is used to validate this model.
    Solver const& GetSolver() const {return m_solver;}

    //! Get read-write access to the parameters that will be used by the solver to validate this model when the transaction is validated.
    //! After modifying a parameter value, you must call #Update in order to save the change to the Db.
    //! That will have the effect of scheduling the model for validation by TxnManager later on. The model's #_OnValidate method will be called at validation time.
    Json::Value& GetSolverParametersR() {return m_solver.m_parameters;}

    //! This method is called when it is time to validate changes that have been made to the model's content during the transaction.
    //! This method is called by the transaction manager after all element-level changes have been validated and all root models have been solved.
    //! This method is called only if elements in this model were added, deleted, or modified or if this model object itself was added or modified.
    //! This method allows a subclass to apply validation logic that requires a view of the entire model and possibly of root models.
    //! This method may add, delete, or modify elements in this model.
    //! To indication a validation error, call TxnManager::ReportError. If the error is marked as fatal, then the transaction will be rolled back.
    //! @note This method must make changes of any kind to any other model. Dependent models will be validated later.
    DGNPLATFORM_EXPORT virtual void _OnValidate();
    /** @} */

    //! Make a copy of this DgnModel with the same DgnClassId and Properties.
    //! @param[in] newName The name for the new DgnModel.
    //! @note This makes a new empty, non-persistent, DgnModel with the same properties as this Model, it does NOT clone the elements of this DgnModel.
    DGNPLATFORM_EXPORT DgnModelPtr Clone(Utf8CP newName) const;

    //! Get the collection of elements for this DgnModel that were loaded by a previous call to FillModel.
    DgnElementMap const& GetElements() const {return m_elements;}

    //! Determine whether this DgnModel has any elements loaded. This will always be true if FillModel was never called,
    //! or after EmptyModel is called.
    bool IsEmpty() const {return (begin() == end());}

    typedef DgnElementMap::const_iterator const_iterator;

    //! a const iterator to the start of the loaded elements for this DgnModel.
    const_iterator begin() const {return m_elements.begin();}

    //! a const iterator to the end of the loaded elements for this DgnModel.
    const_iterator end() const {return m_elements.end();}
};

//=======================================================================================
//! A DgnModel that holds only 3-dimensional DgnElements.
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
//! A DgnModel2d is a infinite planar model that holds only 2-dimensional DgnElements. Coordinates values are X,Y.
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
//! A DgnModel3d that occupies physical space in the DgnDb. All PhysicalModels in a DgnDb have the same coordinate
//! space (DgnModels::Model::CoordinateSpace::World), aka "Physical Space".
//! DgnElements from PhysicalModels are indexed in the persistent range tree of the DgnDb (the DGN_VTABLE_RTree3d).
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
//! A DgnModel3d that exists in its own independent coordinate space. This is used to store the definitions of components.
//! DgnElements of a ComponentModel are not in the persistent range tree.
//! @see ComponentProxyModel
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ComponentModel : DgnModel3d
{
    DEFINE_T_SUPER(DgnModel3d)

public:
    //=======================================================================================
    //! Parameters to create a new instances of a ComponentModel.
    //! @ingroup DgnModelGroup
    //=======================================================================================
    struct CreateParams : DgnModel3d::CreateParams
    {
        DEFINE_T_SUPER(DgnModel3d::CreateParams)
        Utf8String m_elementCategoryName;
    public:
        //! Parameters to create a new instance of a DgnModel.
        //! @param[in] dgndb The DgnDb for the new DgnModel
        //! @param[in] classId The DgnClassId for the new DgnModel.
        //! @param[in] name The name for the DgnModel
        //! @param[in] elementCategory the element category
        //! @param[in] solver The definition of the solver to be used by this model when validating changes to its content.
        //! @param[in] props The properties for the new DgnModel.
        //! @param[in] id Internal only, must be DgnModelId() to create a new DgnModel.
        CreateParams(DgnDbR dgndb, DgnClassId classId, Utf8CP name, Utf8String elementCategory, Solver const& solver, Properties props=Properties(), DgnModelId id=DgnModelId()) :
            T_Super(dgndb, classId, name, props, solver, id), m_elementCategoryName(elementCategory) {}

        //! @private
        //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
        explicit CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}

        Utf8StringCR GetElementCategoryName() const {return m_elementCategoryName;}
    };

protected:
    Utf8String m_elementCategoryName;

    DgnModelType _GetModelType() const override {return DgnModelType::Component;}
    DPoint3d _GetGlobalOrigin() const override {return DPoint3d::FromZero();}
    DgnModels::Model::CoordinateSpace _GetCoordinateSpace() const override {return DgnModels::Model::CoordinateSpace::Local;}
    DGNPLATFORM_EXPORT void _ToPropertiesJson(Json::Value&) const override;//!< @private
    DGNPLATFORM_EXPORT void _FromPropertiesJson(Json::Value const&) override;//!< @private

public:
    explicit ComponentModel(CreateParams const& params) : T_Super(params) {m_elementCategoryName = params.GetElementCategoryName();}

    DGNPLATFORM_EXPORT DgnDbStatus GenerateECClass(ECN::ECSchemaR);
    DGNPLATFORM_EXPORT DgnDbStatus SetParametersFromEC(ECN::IECInstanceCR);

    DGNPLATFORM_EXPORT Utf8String ComputeElementECClassName() const;
    DGNPLATFORM_EXPORT Utf8String GetElementCategoryName() const;

};

//=======================================================================================
//! A DgnModel3d that exists soley to contain information about another
//! ComponentModel and instances of solutions to that other model.
//! @see ComponentModel
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ComponentProxyModel : DgnModel3d
{
    DEFINE_T_SUPER(DgnModel3d)
    
    Utf8String m_componentName;
    DgnCategoryId m_categoryId;
    DgnClassId m_elementClassId;
    bmap<DgnSubCategoryId, DgnSubCategoryId> m_subcatxlat;

protected:
    DgnModelType _GetModelType() const override {return DgnModelType::Component;}
    DPoint3d _GetGlobalOrigin() const override {return DPoint3d::FromZero();}
    DgnModels::Model::CoordinateSpace _GetCoordinateSpace() const override {return DgnModels::Model::CoordinateSpace::Local;}
    DGNPLATFORM_EXPORT void _ToPropertiesJson(Json::Value&) const override;//!< @private
    DGNPLATFORM_EXPORT void _FromPropertiesJson(Json::Value const&) override;//!< @private

    void ImportElementCategory(ComponentModel const&);

public:
    explicit ComponentProxyModel(CreateParams const& params) : T_Super(params) {}

    //! Compute the name of the proxy model that would represent \a componentModel. 
    DGNPLATFORM_EXPORT static Utf8String ComputeName(ComponentModel const& componentModel);

    //! Compute the name of the current solution in \a componentModel. 
    DGNPLATFORM_EXPORT static Utf8String ComputeSolutionName(ComponentModel const& componentModel);

    //! Look up the element in the this proxy model that represents the current solution to the component model.
    //! @param[in] componentModel The ComponentModel that is to be queried
    //! @return The element in this model that represents the specified solution, if it is stored.
    //! @see ComputeSolutionName
    DGNPLATFORM_EXPORT PhysicalElementCPtr QuerySolution(ComponentModel const& componentModel);

    //! Look up the element in the this proxy model that represents the specified solution
    //! @param[in] solutionName     The solution name to look for
    //! @return The element in this model that represents the specified solution, if it is stored.
    //! @see ComputeSolutionName
    DGNPLATFORM_EXPORT PhysicalElementCPtr QuerySolution(Utf8StringCR solutionName);

    //! Harvest geometry from the component model and store in this proxy model in the form of an element. 
    //! @param[in] componentModel The ComponentModel that is to be harvested
    //! @return An element that captures the solution
    DGNPLATFORM_EXPORT PhysicalElementCPtr CaptureSolution(ComponentModel& componentModel);

    //! Create a new ComponentProxyModel object to represent the specified ComponentModel (which might be in a different DgnDb).
    //! The proxy's name will be computed from the name of \a componentModel.
    //! @note The caller must call #Insert. 
    //! @param[in] targetDb     The DgnDb that is to hold the new ComponentProxyModel
    //! @param[in] componentModel The ComponentModel that is to be represented
    //! @param[in] componentSchema The ComponentModel's ECSchema, which should have already been imported into this DgnDb.
    //! @return The newly created ComponentProxyModel
    DGNPLATFORM_EXPORT static RefCountedPtr<ComponentProxyModel> Create(DgnDbR targetDb, ComponentModel const& componentModel, ECN::ECSchemaCR componentSchema);

    //! Look up an existing ComponentProxyModel in the target DgnDb that represents the specified ComponentModel (which might be in a different DgnDb)
    //! The caller does not specify the proxy's name, because a proxy's name is always computed from the name of the componentModel that it represents.
    //! @param[out] existingModel The ComponentProxyModel in the target Db that represents the component model.
    //! @param[in] targetDb     The DgnDb that holds ComponentProxyModel
    //! @param[in] componentModel The ComponentModel that is to be represented
    //! @return DgnDbStatus::NotFound if the proxy could not be found or another non-zero status if the model could not be loaded; Success otherwise.
    DGNPLATFORM_EXPORT static DgnDbStatus Query(RefCountedPtr<ComponentProxyModel>& existingModel, DgnDbR targetDb, ComponentModel const& componentModel);

    //! Import the specified ECSchema into the target DgnDb.
    //! This is a utility function that is used to prepare to work with a ComponentModel that generated the schema.
    //! @param[in] targetDb     The DgnDb that is to hold the new schema
    //! @param[in] schemaFile   The full filename of the ECSchema.xml file to import
    //! @return non-zero error status if the ECSchema could not be imported; DgnDbStatus::DuplicateName if an ECSchema by the same name already exists.
    DGNPLATFORM_EXPORT static DgnDbStatus ImportSchema(DgnDbR targetDb, BeFileNameCR schemaFile);

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
            T_Super(dgndb, classId, name, props, Solver(), id), m_size(size) {}

        explicit CreateParams(DgnModel::CreateParams const& params, DPoint2d size=DPoint2d::FromZero()) : T_Super(params), m_size(size) {}
    };

protected:
    DPoint2d m_size;

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

    //! The ModelHandler for ComponentModel
    struct EXPORT_VTABLE_ATTRIBUTE Component : Model
    {
        MODELHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_ComponentModel, ComponentModel, Component, Model, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for ComponentProxyModel
    struct EXPORT_VTABLE_ATTRIBUTE ComponentProxy : Model
    {
        MODELHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_ComponentProxyModel, ComponentProxyModel, ComponentProxy, Model, DGNPLATFORM_EXPORT)
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
