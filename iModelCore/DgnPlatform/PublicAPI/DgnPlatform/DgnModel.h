/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnModel.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDomain.h"
#include "DgnElement.h"
#include "DgnAuthority.h"
#include "ModelSolverDef.h"
#include <Bentley/ValueFormat.h>
#include <DgnPlatform/DgnProperties.h>

DGNPLATFORM_TYPEDEFS(GeometricModel)
DGNPLATFORM_TYPEDEFS(DefinitionModel)
DGNPLATFORM_TYPEDEFS(DgnModel2d)
DGNPLATFORM_TYPEDEFS(DgnModel3d)
DGNPLATFORM_TYPEDEFS(DgnRangeTree)
DGNPLATFORM_TYPEDEFS(ICheckStop)
DGNPLATFORM_TYPEDEFS(PlanarPhysicalModel)
DGNPLATFORM_TYPEDEFS(SheetModel)
DGNPLATFORM_TYPEDEFS(DictionaryModel)
DGNPLATFORM_TYPEDEFS(SystemModel)
DGNPLATFORM_REF_COUNTED_PTR(SheetModel)
DGNPLATFORM_REF_COUNTED_PTR(DefinitionModel)
DGNPLATFORM_REF_COUNTED_PTR(DictionaryModel)
DGNPLATFORM_REF_COUNTED_PTR(SystemModel)

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

    typedef DgnAuthority::Code Code;

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
        double         m_azimuthAngle;                 //!< Azimuth angle.  CCW from y axis.

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
        void SetAzimuthAngle (double azimuthAngle) { m_azimuthAngle = azimuthAngle; }
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
        double GetAzimuthAngle() const { return m_azimuthAngle; }

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
        Code        m_code;
        Properties  m_props;
        //! Parameters to create a new instance of a DgnModel.
        //! @param[in] dgndb The DgnDb for the new DgnModel
        //! @param[in] classId The DgnClassId for the new DgnModel.
        //! @param[in] code The code for the DgnModel
        //! @param[in] props The properties for the new DgnModel.
        //! @param[in] id Internal only, must be DgnModelId() to create a new DgnModel.
        CreateParams(DgnDbR dgndb, DgnClassId classId, Code code, Properties props=Properties(), DgnModelId id=DgnModelId()) :
            m_dgndb(dgndb), m_id(id), m_classId(classId), m_code(code), m_props(props) {}

        DGNPLATFORM_EXPORT void RelocateToDestinationDb(DgnImportContext&);
    };

    //! Actions which may be restricted for models when the handler for their ECClass is not loaded.
    struct RestrictedAction : DgnDomain::Handler::RestrictedAction
    {
        DEFINE_T_SUPER(DgnDomain::Handler::RestrictedAction);

        static const uint64_t InsertElement = T_Super::NextAvailable; //!< Insert an element into this model. "InsertElement"
        static const uint64_t UpdateElement = InsertElement << 1; //!< Modify an element in this model. "UpdateElement"
        static const uint64_t DeleteElement = UpdateElement << 1; //!< Delete an element in this model. "DeleteElement"
        static const uint64_t Clone = DeleteElement << 1; //!< Create a copy of this model. "Clone"
        static const uint64_t SetCode = Clone << 1; //!< Change this model's Code. "SetCode"

        static const uint64_t Reserved_2 = SetCode << 1; //!< Reserved for future use 
        static const uint64_t Reserved_3 = Reserved_2 << 1; //!< Reserved for future use 
        static const uint64_t Reserved_4 = Reserved_3 << 1; //!< Reserved for future use 
        static const uint64_t Reserved_5 = Reserved_4 << 1; //!< Reserved for future use 
        static const uint64_t Reserved_6 = Reserved_5 << 1; //!< Reserved for future use 

        static const uint64_t NextAvailable = Reserved_6 << 1; //!< Subclasses can add new actions beginning with this value

        DGNPLATFORM_EXPORT static uint64_t Parse(Utf8CP name);
    };

private:
    template<class T> void CallAppData(T const& caller) const;
    void RegisterElement(DgnElementCR el) {_RegisterElement(el);}
    void ReleaseAllElements();

protected:
    DgnDbR          m_dgndb;
    DgnModelId      m_modelId;
    DgnClassId      m_classId;
    Code            m_code;
    Properties      m_properties;
    DgnElementMap   m_elements;
    mutable bmap<AppData::Key const*, RefCountedPtr<AppData>, std::less<AppData::Key const*>, 8> m_appData;
    mutable bool    m_persistent;   // true if this DgnModel is in the DgnModels "loaded models" list.
    bool            m_filled;       // true if the FillModel was called on this DgnModel.

    explicit DGNPLATFORM_EXPORT DgnModel(CreateParams const&);
    DGNPLATFORM_EXPORT virtual ~DgnModel();

    virtual void _SetFilled() {m_filled=true;}
    virtual void DGNPLATFORM_EXPORT _RegisterElement(DgnElementCR element);

    DGNPLATFORM_EXPORT virtual void _InitFrom(DgnModelCR other);            //!< @private
    DGNPLATFORM_EXPORT virtual void _ToPropertiesJson(Json::Value&) const;//!< @private
    DGNPLATFORM_EXPORT virtual void _FromPropertiesJson(Json::Value const&);//!< @private

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

    //! Called after a DgnElement that was previously deleted from this DgnModel has been reinstated by undo
    //! @param[in] element The element that was just reinstatted.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    DGNPLATFORM_EXPORT virtual void _OnReversedDeleteElement(DgnElementCR element);

    //! Called after a DgnElement in this DgnModel has been updated in the DgnDb
    //! @param[in] modified The element in its changed state. This state was saved to the DgnDb
    //! @param[in] original The element in its pre-changed state.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    virtual void _OnUpdatedElement(DgnElementCR modified, DgnElementCR original) { }

    //! Called after an DgnElement that was previously updated has been reversed by undo.
    //! @param[in] original The element in its original state. This is the state before the original change (the current state)
    //! @param[in] modified The element in its post-changed (now reversed) state.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    virtual void _OnReversedUpdateElement(DgnElementCR original, DgnElementCR modified) { }

    //! Called after a DgnElement in this DgnModel has been deleted from the DgnDb
    //! @param[in] element The element that was just deleted.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    DGNPLATFORM_EXPORT virtual void _OnDeletedElement(DgnElementCR element);

    //! Called after a DgnElement in this DgnModel has been removed by undo
    //! @param[in] element The element that was just deleted by undo.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    DGNPLATFORM_EXPORT virtual void _OnReversedAddElement(DgnElementCR element);

    /** @} */

    //! Load all of the DgnElements of this DgnModel into memory.
    DGNPLATFORM_EXPORT virtual void _FillModel();

    //! Load this model's data from the database. If overridden, @em must first call the T_Super implementation, forwarding its status.
    DGNPLATFORM_EXPORT virtual void _ReadProperties();
    //! Update this model's data in the database. If overridden, @em must first call the T_Super implementation, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _Update();

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
    virtual GeometricModelCP _ToGeometricModel() const {return nullptr;}
    virtual DefinitionModelCP _ToDefinitionModel() const {return nullptr;}
    virtual DgnModel2dCP _ToDgnModel2d() const {return nullptr;}
    virtual DgnModel3dCP _ToDgnModel3d() const {return nullptr;}
    virtual PhysicalModelCP _ToPhysicalModel() const {return nullptr;}
    virtual PlanarPhysicalModelCP _ToPlanarPhysicalModel() const {return nullptr;}
    virtual SheetModelCP _ToSheetModel() const {return nullptr;}
    virtual SystemModelCP _ToSystemModel() const {return nullptr;}
    /** @} */

    void ReadProperties();

    //! The sublcass should import elements from the source model into this model. 
    //! Import is done in phases. The import framework will call _ImportElementAspectsFrom and then _ImportECRelationshipsFrom after calling this method.
    //! @note It should be rare for a subclass to override _ImportElementsFrom. The base class implementation copies all elements in the model,
    //! and it fixes up all parent-child pointers. A subclass can override _ShouldImportElementFrom in order to exclude individual elements.
    //! @see _ShouldImportElementFrom
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ImportElementsFrom(DgnModelCR sourceModel, DgnImportContext& importer);
    
    virtual bool _ShouldImportElement(DgnElementCR sourceElement) {return true;}

    //! The sublcass should import ECRelationships from the source model into this model. 
    //! Import is done in phases. This method will be called by the import framework after all elements have been imported and before ECRelationships are imported.
    //! A subclass implementation of _ImportElementAspectsFrom should copy only the ElementAspect subclasses that are defined by the 
    //! the ECSchema/DgnDomain of the subclass. For example, the base DgnModel implementation will handle the ElementAspects defined in the base Dgn schema, including
    //! ElementItem.
    //! @note The implementation should start by calling the superclass implementation.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ImportElementAspectsFrom(DgnModelCR sourceModel, DgnImportContext& importer);

    //! The sublcass should import ECRelationships from the source model into this model. 
    //! Import is done in phases. This method will be called by the import framework after all elements and aspects have been imported.
    //! This method will be called after all elements (and aspects) have been imported.
    //! <p>
    //! A subclass implementation of _ImportECRelationshipsFrom should copy only the relationship subclasses that are defined by the 
    //! the ECSchema/DgnDomain of the subclass. For example, the base DgnModel implementation will handle the relationships defined in the 
    //! base Dgn schema, including ElementDrivesElement, ElementGeomUsesParts, ElementGroupsMembers, and ElementUsesStyles.
    //! <p>
    //! Both endpoints of an ECRelationship must be in the same DgnDb. Since the import operation can copy elements between DgnDbs, a subclass implementation
    //! must be careful about which ECRelationships to import. Normally, only ECRelationships between elements in the model should be copied. 
    //! ECRelationships that start/end outside the model can only be copied if the foreign endpoint is also copied. 
    //! If endpoint elements must be deep-copyed, however, that must be done in _ImportElementsFrom, not in this function. That is because
    //! deep-copying an element in the general case requires all of the support for copying and remapping of parents and aspects that is implemented by the framework,
    //! prior to the phase where ECRelationships are copied.
    //! @note The implementation should start by calling the superclass implementation.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ImportECRelationshipsFrom(DgnModelCR sourceModel, DgnImportContext& importer);

    //! Generate the CreateParams to use for _CloneForImport
    //! @param importer Specifies source and destination DgnDbs and knows how to remap IDs
    //! @return CreateParams initialized with the model's current data, remapped to the destination DgnDb.
    DGNPLATFORM_EXPORT CreateParams GetCreateParamsForImport(DgnImportContext& importer) const;

    DGNPLATFORM_EXPORT virtual void _EmptyModel();
    virtual DgnRangeTree* _GetRangeIndexP(bool create) const {return nullptr;}
    virtual void _OnValidate() { }

    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetCode(Code const& code);
public:
    DGNPLATFORM_EXPORT ModelHandlerR GetModelHandler() const;
    DgnRangeTree* GetRangeIndexP(bool create) const {return _GetRangeIndexP(create);}

    //! Returns true if this is a 3d model.
    bool Is3d() const {return nullptr != ToDgnModel3d();}

    DGNPLATFORM_EXPORT DgnElementCP FindElementById(DgnElementId id); //!< @private

    //! Empty the contents of this DgnModel. This will release any references to DgnElements held by this DgnModel, decrementing
    //! their reference count and potentially freeing them.
    DGNPLATFORM_EXPORT void EmptyModel() {_EmptyModel();}

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

    //! Get a writable reference to the Properties for this DgnModel.
    Properties& GetPropertiesR() {return m_properties;}

    //! Get the Properties for this DgnModel.
    Properties const& GetProperties() const {return m_properties;}

    //! Get the Code of this DgnModel
    Code const& GetCode() const {return m_code;}

    //! Change the Code of this DgnModel
    DgnDbStatus SetCode(Code const& code) { return _SetCode(code); }

    //! Get the DgnClassId of this DgnModel
    DgnClassId GetClassId() const {return m_classId;}

    //! Get the DgnModelId of this DgnModel
    DgnModelId GetModelId() const {return m_modelId;}

    //! @name Dynamic casting to DgnModel subclasses
    //@{
    GeometricModelCP ToGeometricModel() const {return _ToGeometricModel();} //!< more efficient substitute for dynamic_cast<GeometricModelCP>(model)
    DefinitionModelCP ToDefinitionModel() const {return _ToDefinitionModel();} //!< more efficient substitute for dynamic_cast<DefinitionModelCP>(model)
    DgnModel2dCP ToDgnModel2d() const {return _ToDgnModel2d();} //!< more efficient substitute for dynamic_cast<DgnModel2dCP>(model)
    DgnModel3dCP ToDgnModel3d() const {return _ToDgnModel3d();} //!< more efficient substitute for dynamic_cast<DgnModel3dCP>(model)
    PhysicalModelCP ToPhysicalModel() const {return _ToPhysicalModel();} //!< more efficient substitute for dynamic_cast<PhysicalModelCP>(model)
    PlanarPhysicalModelCP ToPlanarPhysicalModel() const {return _ToPlanarPhysicalModel();} //!< more efficient substitute for dynamic_cast<PlanarPhysicalModelCP>(model)
    SheetModelCP ToSheetModel() const {return _ToSheetModel();} //!< more efficient substitute for dynamic_cast<SheetModelCP>(model)
    SystemModelCP ToSystemModel() const {return _ToSystemModel();} //!< more efficient substitute for dynamic_cast<SystemModelCP>(model)
    GeometricModelP ToGeometricModelP() {return const_cast<GeometricModelP>(_ToGeometricModel());} //!< more efficient substitute for dynamic_cast<GeometricModelP>(model)
    DefinitionModelP ToDefinitionModelP() {return const_cast<DefinitionModelP>(_ToDefinitionModel());} //!< more efficient substitute for dynamic_cast<DefinitionModelP>(model)
    DgnModel2dP ToDgnModel2dP() {return const_cast<DgnModel2dP>(_ToDgnModel2d());} //!< more efficient substitute for dynamic_cast<DgnModel2dP>(model)
    DgnModel3dP ToDgnModel3dP() {return const_cast<DgnModel3dP>(_ToDgnModel3d());} //!< more efficient substitute for dynamic_cast<DgnModel3dP>(model)
    PhysicalModelP ToPhysicalModelP() {return const_cast<PhysicalModelP>(_ToPhysicalModel());} //!< more efficient substitute for dynamic_cast<PhysicalModelP>(model)
    PlanarPhysicalModelP ToPlanarPhysicalModelP() {return const_cast<PlanarPhysicalModelP>(_ToPlanarPhysicalModel());} //!< more efficient substitute for dynamic_cast<PlanarPhysicalModelP>(model)
    SheetModelP ToSheetModelP() {return const_cast<SheetModelP>(_ToSheetModel());}//!< more efficient substitute for dynamic_cast<SheetModelP>(model)
    SystemModelP ToSystemModelP() {return const_cast<SystemModelP>(_ToSystemModel());}//!< more efficient substitute for dynamic_cast<SystemModelP>(model)

    bool IsGeometricModel() const { return nullptr != ToGeometricModel(); }
    bool IsPhysicalModel() const { return nullptr != ToPhysicalModel(); }
    bool Is2dModel() const { return nullptr != ToDgnModel2d(); }
    bool Is3dModel() const { return nullptr != ToDgnModel3d(); }
    bool IsDefinitionModel() const { return nullptr != ToDefinitionModel(); }
    bool IsSheetModel() const { return nullptr != ToSheetModel(); }
    bool IsDictionaryModel() const { return DictionaryId() == GetModelId(); }
    bool IsSystemModel() const { return nullptr != ToSystemModel(); }
    //@}

    //! Get the DgnDb of this DgnModel.
    DgnDbR GetDgnDb() const {return m_dgndb;}

    //! Insert this model into the DgnDb.
    //! @return DgnDbStatus::Success if this model was successfully inserted, error otherwise.
    DGNPLATFORM_EXPORT DgnDbStatus Insert(Utf8CP description=nullptr, bool inGuiList=true);

    //! Delete this model from the DgnDb
    //! @note All elements from this model are deleted as well. This method will fail on the first element that cannot be successfully deleted.
    //! @note All views which use this model as their base model are deleted as well. The method will fail on the first such view that cannot be successfully deleted.
    //! @return DgnDbStatus::Success if this model was successfully deleted, error otherwise. Note that if this method returns an error, it is possible
    //! that some elements may have been deleted. Therefore, you should always call DgnDb::AbandonChanges after a failure to avoid partial deletions.
    DGNPLATFORM_EXPORT DgnDbStatus Delete();

    //! Deletes all ViewDefinitions which use this model as their base model
    //! @return Success if all views were successfully deleted, or an error code.
    DGNPLATFORM_EXPORT DgnDbStatus DeleteAllViews();

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

    //! Make a copy of this DgnModel with the same DgnClassId and Properties.
    //! @param[in] newCode The code for the new DgnModel.
    //! @note This makes a new empty, non-persistent, DgnModel with the same properties as this Model, it does NOT clone the elements of this DgnModel.
    //! @see CopyModel, Import
    DGNPLATFORM_EXPORT DgnModelPtr Clone(Code newCode) const;

    //! Make a persitent copy of the specified DgnModel and its contents.
    //! @param[in] model The model to copy
    //! @param[in] newCode The Code for the new DgnModel.
    //! @see Import
    DGNPLATFORM_EXPORT static DgnModelPtr CopyModel(DgnModelCR model, Code newCode);

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

    //! Make a duplicate of this DgnModel object in memory. Do not copy its elements. @see ImportModel
    //! It's not normally necessary for a DgnModel subclass to override _Clone. The base class implementation will 
    //! invoke the subclass handler to create an instance of the subclass. The base class implementation will also
    //! cause the new model object to read its properties from this (source) model's properties. That will 
    //! take of populating most if not all subclass members.
    //! @return the copy of the model
    //! @param[out] stat        Optional. If not null, then an error code is stored here in case the clone fails.
    //! @param importer     Used by elements when copying between DgnDbs.
    //! @see GetCreateParamsForImport
    DGNPLATFORM_EXPORT DgnModelPtr virtual _CloneForImport(DgnDbStatus* stat, DgnImportContext& importer) const;

    //! Copy the contents of \a sourceModel into this model. Note that this model might be in a different DgnDb from \a sourceModel.
    //! This base class implemenation calls the following methods, in order:
    //!     -# _ImportElementsFrom
    //!     -# _ImportElementAspectsFrom
    //!     -# _ImportECRelationshipsFrom
    //! @param sourceModel The model to copy
    //! @param importer     Used by elements when copying between DgnDbs.
    //! @return non-zero if the copy failed
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ImportContentsFrom(DgnModelCR sourceModel, DgnImportContext& importer);

    //! Make a copy of the specified model, including all of the contents of the model, where the destination may be a different DgnDb.
    //! This is just a convenience method that calls the follow methods, in order:
    //!     -# _CloneForImport
    //!     -# Insert
    //!     -# _CopyContentsFrom
    //! @param[out] stat        Optional status to describe failures, a valid DgnModelPtr will only be returned if successful.
    //! @param importer     Enables the model to copy the definitions that it needs (if copying between DgnDbs)
    //! @param sourceModel The model to copy
    //! @return the copied model, already inserted into the destination Db.
    DGNPLATFORM_EXPORT static DgnModelPtr ImportModel(DgnDbStatus* stat, DgnModelCR sourceModel, DgnImportContext& importer);

    //! Make a copy of the specified model, including all of the contents of the model, where the destination may be a different DgnDb.
    //! @param[out] stat        Optional status to describe failures, a valid DgnModelPtr will only be returned if successful.
    //! @param sourceModel  The model to copy
    //! @param importer     Enables the model to copy the definitions that it needs (if copying between DgnDbs)
    //! @return the copied model
    //! @see ImportModel
    template<typename T>
    static RefCountedPtr<T> Import(DgnDbStatus* stat, T const& sourceModel, DgnImportContext& importer) {return dynamic_cast<T*>(ImportModel(stat, sourceModel, importer).get());}

    //! Returns the ID used by the unique dictionary model associated with each DgnDb
    static DgnModelId DictionaryId() { return DgnModelId((uint64_t)1LL); }

    //! This method is called when it is time to validate changes that have been made to the model's content during the transaction.
    //! This method is called by the transaction manager after all element-level changes have been validated and all root models have been solved.
    //! This method is called only if elements in this model were added, deleted, or modified or if this model object itself was added or modified.
    //! This method allows a subclass to apply validation logic that requires a view of the entire model and possibly of root models.
    //! This method may add, delete, or modify elements in this model.
    //! To indication a validation error, call TxnManager::ReportError. If the error is marked as fatal, then the transaction will be rolled back.
    //! @note This method must make changes of any kind to any other model. Dependent models will be validated later.
    void OnValidate() { _OnValidate(); }

    //! Creates a Code for a model with the given name, associated with the default DgnAuthority for models.
    DGNPLATFORM_EXPORT static Code CreateModelCode(Utf8StringCR modelName);
};

//=======================================================================================
//! A DgnModel that holds geometric DgnElements.
//! @ingroup DgnModelGroup
// @bsiclass                                                    Keith.Bentley   03/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometricModel : DgnModel
{
    DEFINE_T_SUPER(DgnModel);

    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(GeometricModel::T_Super::CreateParams);

        ModelSolverDef m_solver;

        //! Parameters to create a new instance of a DgnModel.
        //! @param[in] dgndb The DgnDb for the new DgnModel
        //! @param[in] classId The DgnClassId for the new DgnModel.
        //! @param[in] code The Code for the DgnModel
        //! @param[in] props The properties for the new DgnModel.
        //! @param[in] solver The definition of the solver to be used by this model when validating changes to its content.
        //! @param[in] id Internal only, must be DgnModelId() to create a new DgnModel.
        CreateParams(DgnDbR dgndb, DgnClassId classId, Code code, Properties props=Properties(), ModelSolverDef solver=ModelSolverDef(), DgnModelId id=DgnModelId())
            : T_Super(dgndb, classId, code, props, id), m_solver(solver) { }

        //! @private
        //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
        CreateParams(DgnModel::CreateParams const& params) : T_Super(params) { }

        //! Get the model solver
        ModelSolverDef const& GetSolver() const {return m_solver;}
        //! Set the model solver
        void SetSolver(ModelSolverDef const& s) {m_solver=s;}
    };

private:
    mutable DgnRangeTreeP m_rangeIndex;
    ModelSolverDef m_solver;

    DGNPLATFORM_EXPORT void AllocateRangeIndex() const;
    void AddToRangeIndex(DgnElementCR);
    void RemoveFromRangeIndex(DgnElementCR);
    void UpdateRangeIndex(DgnElementCR modified, DgnElementCR original);

protected:
    void SetSolver(ModelSolverDef const& solver) { m_solver = solver; }

    void ClearRangeIndex();

    virtual void _SetFilled() override {T_Super::_SetFilled(); AllocateRangeIndex();}

    //! Get the Global Origin for this DgnMode.
    //! The global origin is on offset that is added to all coordinate values stored in this model.
    DGNPLATFORM_EXPORT virtual DPoint3d _GetGlobalOrigin() const;//!< @private

    //! Get the coordinate space in which the model's geometry is defined.
    virtual CoordinateSpace _GetCoordinateSpace() const = 0;

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

    DGNPLATFORM_EXPORT virtual DgnRangeTree* _GetRangeIndexP(bool create) const override;
    DGNPLATFORM_EXPORT virtual AxisAlignedBox3d _QueryModelRange() const;//!< @private
    DGNPLATFORM_EXPORT virtual void _EmptyModel() override;
    DGNPLATFORM_EXPORT virtual void _RegisterElement(DgnElementCR element) override;
    DGNPLATFORM_EXPORT virtual void _OnDeletedElement(DgnElementCR element) override;
    DGNPLATFORM_EXPORT virtual void _OnReversedAddElement(DgnElementCR element) override;
    DGNPLATFORM_EXPORT virtual void _OnUpdatedElement(DgnElementCR modified, DgnElementCR original) override;
    DGNPLATFORM_EXPORT virtual void _OnReversedUpdateElement(DgnElementCR modified, DgnElementCR original) override;

    DGNPLATFORM_EXPORT virtual void _InitFrom(DgnModelCR other);            //!< @private
    DGNPLATFORM_EXPORT DgnModelPtr virtual _CloneForImport(DgnDbStatus* stat, DgnImportContext& importer) const override;
    DGNPLATFORM_EXPORT virtual void _ReadProperties();
    DGNPLATFORM_EXPORT virtual DgnDbStatus _Update();
    virtual void _GetSolverOptions(Json::Value&) {;}
    DGNPLATFORM_EXPORT virtual void _OnValidate() override;

    virtual GeometricModelCP _ToGeometricModel() const override {return this;}
    
    explicit GeometricModel(CreateParams const& params) : T_Super(params), m_rangeIndex(nullptr), m_solver(params.m_solver) { }
public:
    void AddGraphicsToScene(ViewContextR context) {_AddGraphicsToScene(context);}

    //! Get the AxisAlignedBox3d of the contents of this DgnModel.
    AxisAlignedBox3d QueryModelRange() const {return _QueryModelRange();}

    //! Get the Global Origin for this DgnModel.
    //! The global origin is an offset that is added to all coordinate values of this DgnModel when reporting them to the user.
    //! @note all PhysicalModels have the same coordinate system and the same global origin.
    DPoint3d GetGlobalOrigin() const {return _GetGlobalOrigin();}

    //! Get the coordinate space in which the model's geometry is defined.
    CoordinateSpace GetCoordinateSpace() const {return _GetCoordinateSpace();}

    //! Get the solver that is used to validate this model.
    ModelSolverDef const& GetSolver() const {return m_solver;}
    ModelSolverDef& GetSolver() {return m_solver;}

    void GetSolverOptions(Json::Value& options) { _GetSolverOptions(options); }
};

//=======================================================================================
//! A DgnModel that holds only 3-dimensional DgnElements.
//! @ingroup DgnModelGroup
// @bsiclass                                                    Keith.Bentley   03/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnModel3d : GeometricModel
{
    DEFINE_T_SUPER(GeometricModel)

protected:
    virtual DgnModel3dCP _ToDgnModel3d() const override {return this;}
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element) override;

public:
    explicit DgnModel3d(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A DgnModel2d is a infinite planar model that holds only 2-dimensional DgnElements. Coordinates values are X,Y.
//! @ingroup DgnModelGroup
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnModel2d : GeometricModel
    {
    DEFINE_T_SUPER(GeometricModel)

protected:
    DPoint2d m_globalOrigin;    //!< Global Origin - all coordinates are offset by this value.

    DGNPLATFORM_EXPORT void _ToPropertiesJson(Json::Value&) const override;
    DGNPLATFORM_EXPORT void _FromPropertiesJson(Json::Value const&) override;
    DPoint3d _GetGlobalOrigin() const override {return DPoint3d::From(m_globalOrigin);}
    DgnModel2dCP _ToDgnModel2d() const override {return this;}

    CoordinateSpace _GetCoordinateSpace() const override {return CoordinateSpace::Local;}
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element);

public:
    void SetGlobalOrigin(DPoint2dCR org) {m_globalOrigin = org;}

    explicit DgnModel2d(CreateParams const& params, DPoint2dCR origin=DPoint2d::FromZero()) : T_Super(params), m_globalOrigin(origin) {}
    };

//=======================================================================================
//! A DgnModel3d that occupies physical space in the DgnDb. All PhysicalModels in a DgnDb have the same coordinate
//! space (CoordinateSpace::World), aka "Physical Space".
//! DgnElements from PhysicalModels are indexed in the persistent range tree of the DgnDb (the DGN_VTABLE_RTree3d).
//! @ingroup DgnModelGroup
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalModel : DgnModel3d
{
    DEFINE_T_SUPER(DgnModel3d)
protected:
    PhysicalModelCP _ToPhysicalModel() const override {return this;}
    CoordinateSpace _GetCoordinateSpace() const override {return CoordinateSpace::World;}

public:
    explicit PhysicalModel(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A model which holds only definitions.
//! @ingroup DgnModelGroup
// @bsiclass                                                    Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DefinitionModel : DgnModel
{
    DEFINE_T_SUPER(DgnModel);
protected:
    DefinitionModelCP _ToDefinitionModel() const override {return this;}
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element) override;
public:
    explicit DefinitionModel(CreateParams const& params) : T_Super(params) { }

    static DefinitionModelPtr Create(CreateParams const& params) { return new DefinitionModel(params); }
};

//=======================================================================================
//! A definition model which holds definitions like materials and styles which are used
//! throughout a DgnDb. Each DgnDb has exactly one DictionaryModel.
//! A DictionaryModel can contain @em only DictionaryElements; and likewise, a
//! DictionaryElement can @em only reside in a DictionaryModel.
//! The dictionary model cannot be copied or deleted. In general, dictionary elements
//! are copied from one dictionary model to another, often indirectly as the result of
//! copying another element which depends upon them.
//! @ingroup DgnModelGroup
// @bsiclass                                                    Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DictionaryModel : DefinitionModel
{
    DEFINE_T_SUPER(DefinitionModel);
protected:
    virtual DgnDbStatus _OnDelete() override { return DgnDbStatus::WrongModel; }
    virtual void _OnDeleted() override { BeAssert(false && "The dictionary model cannot be deleted"); }
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element) override;
    DGNPLATFORM_EXPORT DgnModelPtr virtual _CloneForImport(DgnDbStatus* stat, DgnImportContext& importer) const override;
public:
    explicit DictionaryModel(CreateParams const& params) : T_Super(params) { }
};

//=======================================================================================
//! A SystemModel holds SystemElements which are used to model functional systems.
//! @see SystemElement
//! @ingroup DgnModelGroup
// @bsiclass                                                    Shaun.Sewall    12/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SystemModel : DgnModel
{
    DEFINE_T_SUPER(DgnModel);

protected:
    SystemModelCP _ToSystemModel() const override {return this;}
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element) override;

public:
    explicit SystemModel(CreateParams const& params) : T_Super(params) {}
    static SystemModelPtr Create(CreateParams const& params) {return new SystemModel(params);}
    DGNPLATFORM_EXPORT static SystemModelPtr Create(DgnDbR, Code const&);
};

/*=======================================================================================*//**
* A ComponentModel represents a set of possible configurations of a single type of thing. 
*
* <p>The type of the component is defined by the ECClass that is specified by the component's ItemECClassName property.
* Note that a ComponentModel does not define a type; it generates possible configurations of an existing type.
*
* <p>A ComponentModel uses an algorithm called a "solver" that generates a configuration, based on the values of a pre-defined set of parameters.
* The component's solver and its input parameters are specified by the model's ModelSolverDef.
*
* <p>The Items in a ComponentModel that are to be harvested are in the Category that is identified by the model's ItemCategoryName property.
* See #GetItemCategoryName for the name of this category. Items in the ComponentModel that are not assigned to the Element Category are considered to be 
* construction elements and are not harvested.
* @bsiclass                                                    Keith.Bentley   10/11
**//*=======================================================================================*/
struct EXPORT_VTABLE_ATTRIBUTE ComponentModel : DgnModel3d
{
private:
    DEFINE_T_SUPER(DgnModel3d)

    struct CompProps
        {
        Utf8String m_itemECClassName;
        Utf8String m_itemCategoryName;
        Utf8String m_itemCodeAuthority;

        CompProps(Utf8StringCR iclass, Utf8StringCR icat, Utf8String iauthority) : m_itemECClassName(iclass), m_itemCategoryName(icat), m_itemCodeAuthority(iauthority) {;}
        bool IsValid(DgnDbR) const;
        void FromJson(Json::Value const& inValue);
        void ToJson(Json::Value& outValue) const;

        DgnClassId GetItemECClassId(DgnDbR) const;
        DgnCategoryId QueryItemCategoryId(DgnDbR) const;
        };

public:
    //=======================================================================================
    //! Parameters to create a new instances of a ComponentModel.
    //! @ingroup DgnModelGroup
    //=======================================================================================
    struct CreateParams : DgnModel3d::CreateParams
    {
    private:
        DEFINE_T_SUPER(DgnModel3d::CreateParams)
        friend struct ComponentModel;
        CompProps m_compProps;
    public:
        //! Parameters to create a new instance of a ComponentModel.
        //! @param[in] dgndb The DgnDb for the new ComponentModel
        //! @param[in] name The name for the ComponentModel
        //! @param[in] iclass The full (namespace.class) name of the ECClass that should be used when creating an Item from this component.
        //! @param[in] icat The name of the category that will be used by Items that should be harvested and that should be used when creating an instance Item from this component.
        //! @param[in] iauthority The name of the CodeAuthority that should be used when creating an instance Item from this component.
        //! @param[in] solver The definition of the solver to be used by this model when validating changes to its content.
        DGNPLATFORM_EXPORT CreateParams(DgnDbR dgndb, Utf8StringCR name, Utf8StringCR iclass, Utf8StringCR icat, Utf8String iauthority, ModelSolverDef const& solver);

        //! @private
        //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
        explicit CreateParams(DgnModel::CreateParams const& params) : T_Super(params), m_compProps("","","") {}
    };

    friend struct CreateParams;

private:
    // Base for helper classes that assist in making harvested solutions persistent
    struct HarvestedSolutionWriter
        {
      protected:
        ComponentModel& m_cm;
        DgnModelR m_destModel;
      public:
        virtual DgnDbR _GetOutputDgnDb() {return m_destModel.GetDgnDb();}
        virtual DgnModelR _GetOutputModel() {return m_destModel;}
        virtual DgnElementPtr _CreateCapturedSolutionElement(DgnDbStatus& status, DgnClassId iclass, DgnElement::Code const& icode); // (rarely need to override this)
        virtual DgnElementCPtr _WriteSolution(DgnDbStatus&, DgnElementR) = 0;
        HarvestedSolutionWriter(DgnModelR m, ComponentModel& c) : m_destModel(m), m_cm(c) {;}
        };

    // Makes a "catalog item" persistent
    struct HarvestedSolutionInserter : HarvestedSolutionWriter
        {
        DEFINE_T_SUPER(HarvestedSolutionWriter)
      protected:
        DgnElementCPtr _WriteSolution(DgnDbStatus&, DgnElementR) override;
      public:
        HarvestedSolutionInserter(DgnModelR m, ComponentModel& c) : HarvestedSolutionWriter(m,c) {;}
        };

    // Makes a unique/single solution persistent
    struct HarvestedSingletonInserter : HarvestedSolutionInserter
        {
        DEFINE_T_SUPER(HarvestedSolutionInserter)
      protected:
        DgnElementCPtr _WriteSolution(DgnDbStatus&, DgnElementR) override;
      public:
        HarvestedSingletonInserter(DgnModelR m, ComponentModel& c) : HarvestedSolutionInserter(m,c) {;}
        };

    // Updates any kind of persistent solution element
    struct HarvestedSolutionUpdater : HarvestedSolutionWriter
        {
        DEFINE_T_SUPER(HarvestedSolutionWriter)
      protected:
        DgnElementCPtr m_existing;
        DgnElementCPtr _WriteSolution(DgnDbStatus&, DgnElementR) override;
      public:
        HarvestedSolutionUpdater(DgnModelR m, ComponentModel& c, DgnElementCR e) : HarvestedSolutionWriter(m, c), m_existing(&e) {;}
        };

private:
    CompProps m_compProps;

    DPoint3d _GetGlobalOrigin() const override final {return DPoint3d::FromZero();}
    CoordinateSpace _GetCoordinateSpace() const override final {return CoordinateSpace::Local;}
    void _GetSolverOptions(Json::Value&) override;
    DgnDbStatus _OnDelete() override;
    //DgnDbStatus _OnUpdate() override;

    void _ToPropertiesJson(Json::Value&) const override;//!< @private
    void _FromPropertiesJson(Json::Value const&) override;//!< @private

    DgnElement::Code CreateCapturedSolutionCode(Utf8StringCR slnId);

    DgnDbStatus HarvestSolution(bvector<bpair<DgnSubCategoryId, DgnGeomPartId>>& geomBySubcategory, bvector<DgnElementCPtr>& nestedInstances);
    DgnElementPtr CreateCapturedSolutionElement(DgnDbStatus& status, DgnElement::Code const& icode, bvector<bpair<DgnSubCategoryId, DgnGeomPartId>> const&, HarvestedSolutionWriter& writer);
    DgnElementCPtr MakeCapturedSolutionElement(DgnDbStatus& status, DgnElement::Code const& icode, HarvestedSolutionWriter& WriterHandler);

public:
    //! @private - used in testing only 
    DGNPLATFORM_EXPORT DgnDbStatus Solve(ModelSolverDef::ParameterSet const& parameters);

    //! @private - used only by ElementCopier
    static void OnElementCopied(DgnElementCR outputElement, DgnElementCR sourceElement, DgnCloneContext&);
    //! @private - used only by importer
    static void OnElementImported(DgnElementCR outputElement, DgnElementCR sourceElement, DgnImportContext&);

    DGNPLATFORM_EXPORT explicit ComponentModel(CreateParams const& params);

    /** @name Component Model Properties */
    /** @{ */

    //! Query if the ComponentModel is correctly defined.
    DGNPLATFORM_EXPORT bool IsValid() const;

    //! Get the full (namespace.class) name of the ECClass that should be used when creating an Item from this component.
    DGNPLATFORM_EXPORT Utf8String GetItemECClassName() const;

    //! Get the name of the category that will be used by Items that should be harvested and that should be used when creating an instance Item from this component.
    //! Items in this model that are in other categories should be treated as construction geometry.
    DGNPLATFORM_EXPORT Utf8String GetItemCategoryName() const;

    //! Get the name of the CodeAuthority that should be used when creating an instance Item from this component.
    DGNPLATFORM_EXPORT Utf8String GetItemCodeAuthority() const;

    //! Get the name of the component.
    Utf8CP GetModelName() const { return GetCode().GetValueCP(); }

    /** @} */

    /** @name Querying Component Models */
    /** @{ */

    //! Look up a component model by name
    //! @param db The DgnDb that contains the component model
    //! @param name The name of the component model
    //! @return A pointer to the component model or an invalid pointer if not found
    DGNPLATFORM_EXPORT static ComponentModelPtr FindModelByName(DgnDbR db, Utf8StringCR name);

    /** @} */

    /** @name Capturing Solutions */
    /** @{ */

    //! Captures the result of solving for the specified parameters. This may be called to create an Item in a solution, or it may be called to capture a unique, singleton solution.
    //! @param[out] stat        Optional. If not null and if the solution cannot be captured, then an error code is stored here to explain what happened, as explained below.
    //! @param[in] destModel    The output model, where the captured solution item(s) is(are) stored.
    //! @param[in] parameters   The parameters that specify the solution
    //! @param[in] solutionItemName The name of the Item to be created in the destModel. This cannot be blank, and it must be unique among all captured solutions for this component.
    //! @return A handle to the Item that was created and persisted in \a destModel. If more than one element was created, the returned element is the parent.
    //! @note This function is used only for solutions that result in DgnElements. That is, the ECClass identified by #GetItemECClassName must be a subclass of DgnElement.
    //! @note When a solution cannot be captured, the error code will be:
    //!     * DgnDbStatus::ValidationFailed - The model could not be solved, possibly because the values in \a parameters are invalid.
    //!     * DgnDbStatus::DuplicateCode - An Item already exists with the same name for this component.
    //!     * DgnDbStatus::NotFound - no element in the solution is in the category identified by #GetItemCategoryName.
    //!     * DgnDbStatus::SQLiteError or DgnDbStatus::WriteError - The solution could not be written to the Db. 
    //!     * DgnDbStatus::LockNotHeld - Failure to lock this component model or the output model.
    //!     * DgnDbStatus::InvalidCategory - The category identified by #GetItemCategoryName does not exist in this Db.
    //!     * DgnDbStatus::MissingDomain - The ECClass identified by #GetItemECClassName does not exist in this Db.
    //!     * DgnDbStatus::MissingHandler - The handler for the ECClass identified by #GetItemECClassName has not been registered.
    //!     * DgnDbStatus::WrongClass - The ECClass identified by #GetItemECClassName is not a subclass of DgnElement.
    //! @see MakeInstanceOfSolution, QuerySolutionByName
    DGNPLATFORM_EXPORT DgnElementCPtr CaptureSolution(DgnDbStatus* stat, PhysicalModelR destModel, ModelSolverDef::ParameterSet const& parameters, Utf8StringCR solutionItemName);

    //! Test if the specified code is that of a captured solution element.
    DGNPLATFORM_EXPORT static bool IsCapturedSolutionCode(DgnElement::Code const& icode);

    //! Delete the specified solution Item. This function also deletes the ECRelationship that relates the Item to this component model.
    //! @return DgnDbStatus::BadRequest if \a solutionItem is not an element that capatures a solution to this component model.
    //! @note This function does not delete all existing instances of the specified solution Item
    //! @see MakeInstanceOfSolution
    DGNPLATFORM_EXPORT DgnDbStatus DeleteSolution(DgnElementCR solutionItem);

    //! Search for all captured solutions for this component model
    //! @param solutions    Where to return the IDs of the captured solutions
    //! @see CaptureSolution, QuerySolutionById, QuerySolutionByName, MakeInstanceOfSolution
    DGNPLATFORM_EXPORT void QuerySolutions(bvector<DgnElementId>& solutions);

    //! Get the specified captured solution element. 
    //! This function looks up an element by ElementId and then calls QuerySolutionInfo.
    //! This method is better than DgnElements::Get, in that it \em also checks that the specified DgnElementId identifies an element that captures a solution to this component model
    //! and returns information about the solution.
    //! @param[out] capturedSolutionElement The element that captures the solution
    //! @param[out] params      The parameters that were used to generate the specified captured solution element
    //! @param[in] capturedSolutionElementId The ID of the captured solution element
    //! @return non-zero error status if \a capturedSolutionElementId does not identify an element that captures a solution of this component model
    //! @see CaptureSolution, QuerySolutionByName, QuerySolutionInfo
    DGNPLATFORM_EXPORT DgnDbStatus QuerySolutionById(DgnElementCPtr& capturedSolutionElement, ModelSolverDef::ParameterSet& params, DgnElementId capturedSolutionElementId);
    
    //! Get the element that captures the result of solving for the captured solution element Name.
    //! This function looks up an element by code and then calls QuerySolutionInfo.
    //! This method is better than DgnElements::Get, in that it \em also checks that the specified DgnElementId identifies an element that captures a solution to this component model
    //! and returns information about the solution.
    //! @param[out] capturedSolutionElement The element that captures the solution
    //! @param[out] params      The parameters that were used to generate the specified captured solution element
    //! @param[in] capturedSolutionName The name of the captured solution element
    //! @return non-zero error status if \a capturedSolutionName does not identify an element that captures a solution of this component model
    //! @see CaptureSolution, QuerySolutionById, QuerySolutionInfo
    DGNPLATFORM_EXPORT DgnDbStatus QuerySolutionByName(DgnElementCPtr& capturedSolutionElement, ModelSolverDef::ParameterSet& params, Utf8StringCR capturedSolutionName);

    //! Try to find a captured solution for this component model with the specified parameters.
    //! @param[out] capturedSolutionElement The element that captures the solution
    //! @param[in] params      A set of parameters 
    //! @return non-zero error status if no solution based on the specified parameters has been captured.
    DGNPLATFORM_EXPORT DgnDbStatus QuerySolutionByParameters(DgnElementCPtr& capturedSolutionElement, ModelSolverDef::ParameterSet const& params);

    //! Get the ComponentModel and parameters that were used to generate the specified captured solution element
    //! @param[out] params      The parameters that were used to generate the specified captured solution element
    //! @param[in] capturedSolutionElement  The captured solution element that is to be queried
    //! @return non-zero error status if \a capturedSolution is not an element that captures a solution of this component model
    //! @see MakeInstanceOfSolution
    DGNPLATFORM_EXPORT DgnDbStatus QuerySolutionInfo(ModelSolverDef::ParameterSet& params, DgnElementCR capturedSolutionElement);

    //! Get the ComponentModel and parameters that were used to generate the specified captured solution element
    //! @param[out] cmid        The ID of the ComponentModel that was used to generate the specified captured solution element
    //! @param[out] params      The parameters that were used to generate the specified captured solution element
    //! @param[in] capturedSolutionElement The captured solution element
    //! @return non-zero error status if \a capturedSolution is not the solution of any component model
    //! @see MakeInstanceOfSolution
    DGNPLATFORM_EXPORT static DgnDbStatus QuerySolutionInfo(DgnModelId& cmid, ModelSolverDef::ParameterSet& params, DgnElementCR capturedSolutionElement);

    //! Helper class for importing component models and their captured solutions
    struct Importer : DgnImportContext
        {
        ComponentModelR m_sourceComponent;
        ComponentModelPtr m_destComponent;

        //! Construct a new importer
        DGNPLATFORM_EXPORT Importer(DgnDbR destDb, ComponentModel& sourceComponent);

        //! If necessary, set the destination component model. 
        //! @note This is rarely needed. ImportComponentModel will set the destination model automatically.
        void SetDestComponent(ComponentModelR m) {m_destComponent=&m;}

        //! Import the component model.
        //! @param[out] status  Optional. If not null, then an error code is stored here in case the clone fails.
        //! @return the newly created component model in the destinataion Db
        DGNPLATFORM_EXPORT ComponentModelPtr ImportComponentModel(DgnDbStatus* status = nullptr);

        //! Import all or selected solutions of this component model.
        //! @param destModel    The model to which solutions are to be copied. 
        //! @param solutionFilter Optional. If not empty, this is the list of solutions to import. If empty, then all solutions are imported.
        //! @return non-zero error status if the import failed, including \a DgnDbStatus::WrongDgnDb if \a destModel is not in the
        //!         same DgnDb as the destination component model, \a DgnDbStatus::WrongModel if \a destModel is the same as the destination component model,
        //!         or \a DgnDbStatus::BadModel if the destination component model is not set, or some other
        //!         error code if the import of any individual solution Item fails.
        //! @note \a destModel must be different from but in the same DgnDb as the destination component model.
        DGNPLATFORM_EXPORT DgnDbStatus ImportSolutions(DgnModelR destModel, bvector<AuthorityIssuedCode> const& solutionFilter = bvector<AuthorityIssuedCode>());
        };

    //! Call this function if you update the solver definition itself.
    DGNPLATFORM_EXPORT DgnDbStatus UpdateSolutionsAndInstances();

    /** @} */

    /** @name Placing Instances of Captured Slutions */
    /** @{ */

    //! Make a persistent copy of a specified solution Item, along with all of its children.
    //! Call this function if you are sure that you just need a copy of an already captured solution.
    //! @param[out] stat        Optional. If not null, then an error code is stored here in case the copy fails.
    //! @param[in] targetModel  The model where the instance is to be inserted
    //! @param[in] capturedSolution  The captured solution element that is to be copied
    //! @param[in] code         Optional. The code to assign to the new item. If invalid, then a code will be generated by the CodeAuthority associated with this component model
    //! @return the instance item if successful
    //! @see QuerySolutionByName
    DGNPLATFORM_EXPORT static DgnElementCPtr MakeInstanceOfSolution(DgnDbStatus* stat, DgnModelR targetModel, DgnElementCR capturedSolution, DgnElement::Code const& code = DgnElement::Code());

    //! Make a persistent copy of a specified solution Item or create a unique/singleton Item.
    //! Call this function if you might need a unique/singleton solution.
    //! If \a capturedSolutionName is valid and identifies an existing solution \em and if \a parameters matches the captured solution's parameters, then this function calls MakeInstanceOfSolution to make a copy of the captured solution.
    //! Otherwise, this function calls CaptureSolution to create a unique/singleton solution.
    //! @param[out] stat        Optional. If not null, then an error code is stored here in case the creation of the instance fails.
    //! @param[in] targetModel  The model where the instance is to be inserted
    //! @param[in] capturedSolutionName  The name of the captured solution element that is to be copied, if any. Pass the empty string to create a unique/singleton solution.
    //! @param[in] parameters   The parameters that specify the solution
    //! @param[in] code         Optional. The code to assign to the new item. If invalid, then a code will be generated by the CodeAuthority associated with this component model
    //! @return the instance item if successful
    //! @see QuerySolutionByName
    DGNPLATFORM_EXPORT DgnElementCPtr MakeInstanceOfSolution(DgnDbStatus* stat, DgnModelR targetModel, Utf8StringCR capturedSolutionName,
                                                    ModelSolverDef::ParameterSet const& parameters, DgnElement::Code const& code = DgnElement::Code());

    //! Look up the captured solution element that was used to generate the specified instance
    //! @param instance An element that is an instance of a solution to some component model
    //! @return the captured solution element that was used to generate the specified instance or nullptr if \a instance is not in fact an instance of any known component model
    DGNPLATFORM_EXPORT static DgnElementCPtr QuerySolutionFromInstance(DgnElementCR instance);

    //! Search for all instances of the specified captured solution for this component model
    //! @param instances    Where to return the IDs of the instances
    //! @param solutionId   The captured solution element
    DGNPLATFORM_EXPORT void QueryInstances(bvector<DgnElementId>& instances, DgnElementId solutionId);

    /** @} */
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
//! A sheet model is a DgnModel2d that has the following characteristics:
//!     - Has fixed extents (is not infinite), specified in meters.
//!     - Can contain @b views of other models, like pictures pasted on a photo album.
//! @ingroup DgnModelGroup
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SheetModel : DgnModel2d
{
    DEFINE_T_SUPER(DgnModel2d)

    struct CreateParams : DgnModel2d::CreateParams
    {
        DEFINE_T_SUPER(DgnModel2d::CreateParams);
        DPoint2d m_size;

        //! Parameters for creating a new SheetModel.
        //! @param[in] dgndb the DgnDb into which the SheetModel will be created
        //! @param[in] classId the DgnClassId of thew new SheetModel (must be or derive from SheetModel)
        //! @param[in] code the code of the new SheetModel
        //! @param[in] size the size of the SheetModel, in meters.
        //! @param[in] props the Properties of the new SheetModel
        //! @param[in] id the DgnModelId of thew new SheetModel. This should be DgnModelId() when creating a new model.
        CreateParams(DgnDbR dgndb, DgnClassId classId, Code code, DPoint2d size, Properties props=Properties(), DgnModelId id=DgnModelId()) :
            T_Super(dgndb, classId, code, props, ModelSolverDef(), id), m_size(size) {}

        explicit CreateParams(DgnModel::CreateParams const& params, DPoint2d size=DPoint2d::FromZero()) : T_Super(params), m_size(size) {}
    };

protected:
    DPoint2d m_size;

    SheetModelCP _ToSheetModel() const override {return this;}

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
        protected: virtual uint64_t _ParseRestrictedAction(Utf8CP name) const override { return __classname__::RestrictedAction::Parse(name); }\
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
        virtual uint64_t _ParseRestrictedAction(Utf8CP name) const override { return DgnModel::RestrictedAction::Parse(name); }

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

    //! The ModelHandler for DefinitionModel
    struct EXPORT_VTABLE_ATTRIBUTE Definition : Model
    {
        MODELHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_DefinitionModel, DefinitionModel, Definition, Model, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for DictionaryModel
    struct EXPORT_VTABLE_ATTRIBUTE Dictionary : Definition
    {
        MODELHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_DictionaryModel, DictionaryModel, Dictionary, Definition, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for Model2d
    struct EXPORT_VTABLE_ATTRIBUTE Model2d : Model
    {
        MODELHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_Model2d, DgnModel2d, Model2d, Model, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for SystemModel
    struct EXPORT_VTABLE_ATTRIBUTE System : Model
    {
        MODELHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_SystemModel, SystemModel, System, Model, DGNPLATFORM_EXPORT)
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

