/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnModel.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDomain.h"
#include "DgnElement.h"
#include "DgnAuthority.h"
#include "ECSqlClassParams.h"
#include <Bentley/ValueFormat.h>
#include <DgnPlatform/DgnProperties.h>

DGNPLATFORM_TYPEDEFS(GeometricModel)
DGNPLATFORM_TYPEDEFS(InformationModel)
DGNPLATFORM_TYPEDEFS(DefinitionModel)
DGNPLATFORM_TYPEDEFS(GeometricModel2d)
DGNPLATFORM_TYPEDEFS(GeometricModel3d)
DGNPLATFORM_TYPEDEFS(GraphicalModel2d)
DGNPLATFORM_TYPEDEFS(GroupInformationModel)
DGNPLATFORM_TYPEDEFS(DgnRangeTree)
DGNPLATFORM_TYPEDEFS(CheckStop)
DGNPLATFORM_TYPEDEFS(DrawingModel)
DGNPLATFORM_TYPEDEFS(SectionDrawingModel)
DGNPLATFORM_TYPEDEFS(SheetModel)
DGNPLATFORM_TYPEDEFS(DictionaryModel)
DGNPLATFORM_REF_COUNTED_PTR(SheetModel)
DGNPLATFORM_REF_COUNTED_PTR(DefinitionModel)
DGNPLATFORM_REF_COUNTED_PTR(DictionaryModel)
DGNPLATFORM_REF_COUNTED_PTR(GroupInformationModel)

BEGIN_BENTLEY_DGN_NAMESPACE

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
    bool Contains(DgnElementId id) const {return end() != find(id);}
    uint32_t GetCount() const {return (uint32_t) size();}
    };

#define DGNMODEL_DECLARE_MEMBERS(__ECClassName__,__superclass__)\
    private: typedef __superclass__ T_Super;\
    public: static Utf8CP MyHandlerECClassName() {return __ECClassName__;}\
    protected:  virtual Utf8CP _GetHandlerECClassName() const override {return MyHandlerECClassName();}\
                virtual Utf8CP _GetSuperHandlerECClassName() const override {return T_Super::_GetHandlerECClassName();}

/**
* @addtogroup GROUP_DgnModel DgnModel Module
* Types related to working with %DgnModels
* @see @ref PAGE_ModelOverview
* @see @ref PAGE_CustomModel
*/

//=======================================================================================
//! A DgnModel represents a model in memory and may hold references to elements that belong to it.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                     KeithBentley    10/00
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnModel : RefCountedBase, ICodedEntity
    {
    friend struct DgnModels;
    friend struct DgnElement;
    friend struct DgnElements;
    friend struct QueryModel;
    friend struct dgn_TxnTable::Model;

    struct CreateParams;

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
    //! Parameters to create a new instances of a DgnModel.
    //! @ingroup GROUP_DgnModel
    //=======================================================================================
    struct CreateParams
    {
        DgnDbR      m_dgndb;
        DgnClassId  m_classId;
        DgnCode     m_code;
        Utf8String  m_label;
        bool        m_inGuiList;

        //! Parameters to create a new instance of a DgnModel.
        //! @param[in] dgndb The DgnDb for the new DgnModel
        //! @param[in] classId The DgnClassId for the new DgnModel.
        //! @param[in] code The code for the DgnModel
        //! @param[in] label Label of the new DgnModel
        //! @param[in] inGuiList Controls the visibility of the new DgnModel in model lists shown to the user
        CreateParams(DgnDbR dgndb, DgnClassId classId, DgnCode code, Utf8CP label = nullptr, bool inGuiList = true) :
            m_dgndb(dgndb), m_classId(classId), m_code(code), m_inGuiList(inGuiList)
            {
            SetLabel(label);
            }

        void SetCode(DgnCode code) { m_code = code; } //!< Set the DgnCode for models created with this CreateParams
        void SetLabel(Utf8CP label) { m_label.AssignOrClear(label); } //!< Set the Label for models created with this CreateParams
        void SetInGuiList(bool inGuiList) { m_inGuiList = inGuiList; } //!< Set the visibility of models created with this CreateParams in model lists shown to the user

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
        static const uint64_t SetCode = Clone << 1; //!< Change this model's DgnCode. "SetCode"

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

    DgnDbStatus BindInsertAndUpdateParams(BeSQLite::EC::ECSqlStatement& statement);
    DgnDbStatus Read(DgnModelId modelId);
protected:
    DgnDbR          m_dgndb;
    DgnModelId      m_modelId;
    DgnClassId      m_classId;
    DgnCode         m_code;
    Utf8String      m_label;
    bool            m_inGuiList;
    int             m_dependencyIndex;

    DgnElementMap   m_elements;
    mutable bmap<AppData::Key const*, RefCountedPtr<AppData>, std::less<AppData::Key const*>, 8> m_appData;
    mutable bool    m_persistent;   // true if this DgnModel is in the DgnModels "loaded models" list.
    bool            m_filled;       // true if the FillModel was called on this DgnModel.

    explicit DGNPLATFORM_EXPORT DgnModel(CreateParams const&);
    DGNPLATFORM_EXPORT virtual ~DgnModel();

    virtual void _SetFilled() {m_filled=true;}
    virtual void DGNPLATFORM_EXPORT _RegisterElement(DgnElementCR element);

    DGNPLATFORM_EXPORT virtual void _InitFrom(DgnModelCR other);            //!< @private

    //! Invoked when loading a model from the table, to allow subclasses to extract their property values
    //! from the SELECT statement. The parameters are those which were specified by this elements Handler.
    //! @param[in] statement The SELECT statement which selected the class using ECSql
    //! @param[in] params The properties selected by the SELECT statement. Use this to obtain an index into the statement.
    //! @return DgnDbStatus::Success if the data was loaded successfully, or else an error status.
    //! @note If you override this method, you @em must first call T_Super::_ReadSelectParams, forwarding its status.
    //! You should then extract your subclass properties from the supplied ECSqlStatement, using
    //! selectParams.GetParameterIndex() to look up the index of each parameter within the statement.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParamsCR params);

    //! Called to bind the model's property values to the ECSqlStatement when inserting
    //! a new model.  The parameters to bind were the ones specified by this model's Handler.
    //! @note If you override this method, you should bind your subclass properties
    //! to the supplied ECSqlStatement, using statement.GetParameterIndex with your property's name.
    //! Then you @em must call T_Super::_BindInsertParams, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& statement);

    //! Called to bind the model's property values to the ECSqlStatement when updating
    //! an existing model.  The parameters to bind were the ones specified by this model's Handler
    //! @note If you override this method, you should bind your subclass properties
    //! to the supplied ECSqlStatement, using statement.GetParameterIndex with your property's name.
    //! Then you @em must call T_Super::_BindUpdateParams, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement);

    //! Invoked when writing the Properties field into the Db as part of an Insert or Update operation.
    //! @note If you override this method, you @em must call T_Super::_WriteJsonProperties. Consider
    //! writing your properties into a sub node of the value to avoid collisions with the 
    //! super class properties.
    virtual void _WriteJsonProperties(Json::Value& value) const {}

    //! Invoked when reading the Properties field from the Db. 
    //! @note If you override this method, you @em must call T_Super::_WriteJsonProperties. Consider
    //! writing your properties into a sub node of the value to avoid collisions with the 
    //! super class properties.
    virtual void _ReadJsonProperties(Json::Value const& value) {}

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

    //! Return the copyright message to display if this model is drawn in a viewport
    //! @return a copyright message or nullptr
    virtual Utf8CP _GetCopyrightMessage() const {return nullptr;}

    /** @name Dynamic cast shortcuts for a DgnModel */
    /** @{ */
    virtual GeometricModelCP _ToGeometricModel() const {return nullptr;}
    virtual InformationModelCP _ToInformationModel() const {return nullptr;}
    virtual DefinitionModelCP _ToDefinitionModel() const {return nullptr;}
    virtual GeometricModel2dCP _ToGeometricModel2d() const {return nullptr;}
    virtual GeometricModel3dCP _ToGeometricModel3d() const {return nullptr;}
    virtual SpatialModelCP _ToSpatialModel() const {return nullptr;}
    virtual SectionDrawingModelCP _ToSectionDrawingModel() const {return nullptr;}
    virtual SheetModelCP _ToSheetModel() const {return nullptr;}
    virtual GroupInformationModelCP _ToGroupInformationModel() const {return nullptr;}
    /** @} */

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
    //! base Dgn schema, including ElementDrivesElement, ElementUsesGeometryParts, ElementGroupsMembers, and ElementUsesStyles.
    //! <p>
    //! Both endpoints of an ECRelationship must be in the same DgnDb. Since the import operation can copy elements between DgnDbs, a subclass implementation
    //! must be careful about which ECRelationships to import. Normally, only ECRelationships between elements in the model should be copied. 
    //! ECRelationships that start/end outside the model can only be copied if the foreign endpoint is also copied. 
    //! If endpoint elements must be deep-copyed, however, that must be done in _ImportElementsFrom, not in this function. That is because
    //! deep-copying an element in the general case requires all of the support for copying and remapping of parents and aspects that is implemented by the framework,
    //! prior to the phase where ECRelationships are copied.
    //! @note The implementation should start by calling the superclass implementation.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ImportECRelationshipsFrom(DgnModelCR sourceModel, DgnImportContext& importer);

    //! Disclose any locks which must be acquired and/or codes which must be reserved in order to perform the specified operation on this model.
    //! @param[in]      request  Request to populate
    //! @param[in]      opcode   The operation to be performed
    //! @return RepositoryStatus::Success, or an error code if for example a required lock or code is known to be unavailable without querying the repository manager.
    //! @note If you override this function you @b must call T_Super::_PopulateRequest(), forwarding its status.
    DGNPLATFORM_EXPORT virtual RepositoryStatus _PopulateRequest(IBriefcaseManager::Request& request, BeSQLite::DbOpcode opcode) const;

    //! Generate the CreateParams to use for _CloneForImport
    //! @param importer Specifies source and destination DgnDbs and knows how to remap IDs
    //! @return CreateParams initialized with the model's current data, remapped to the destination DgnDb.
    DGNPLATFORM_EXPORT CreateParams GetCreateParamsForImport(DgnImportContext& importer) const;

    DGNPLATFORM_EXPORT virtual void _EmptyModel();
    virtual DgnRangeTree* _GetRangeIndexP(bool create) const {return nullptr;}
    virtual void _OnValidate() { }

    virtual void _DropGraphicsForViewport(DgnViewportCR viewport) {};

    virtual DgnCode const& _GetCode() const override final { return m_code; }
    virtual DgnDbR _GetDgnDb() const override final { return m_dgndb; }
    virtual DgnModelCP _ToDgnModel() const override final { return this; }
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetCode(DgnCode const& code) override final;
    DGNPLATFORM_EXPORT virtual bool _SupportsCodeAuthority(DgnAuthorityCR) const override;
    virtual DgnCode _GenerateDefaultCode() const override { return DgnCode(); }
public:
    Utf8CP GetCopyrightMessage() const {return _GetCopyrightMessage();}

    virtual Utf8CP _GetHandlerECClassName() const {return DGN_CLASSNAME_Model;} //!< @private
    virtual Utf8CP _GetSuperHandlerECClassName() const {return nullptr;}        //!< @private

    DGNPLATFORM_EXPORT ModelHandlerR GetModelHandler() const;
    DgnRangeTree* GetRangeIndexP(bool create) const {return _GetRangeIndexP(create);}

    //! Returns true if this is a 3d model.
    bool Is3d() const {return nullptr != ToGeometricModel3d();}

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

    //! Get the name of this model
    Utf8String GetName() const {return m_code.GetValue();}
    
    //! Get the DgnClassId of this DgnModel
    DgnClassId GetClassId() const {return m_classId;}

    //! Get the DgnModelId of this DgnModel
    DgnModelId GetModelId() const {return m_modelId;}

    //! Get the label of this DgnModel.
    //! @note may be nullptr
    Utf8CP GetLabel() const { return m_label.c_str(); }
    
    //! Set the label of this DgnModel.
    void SetLabel(Utf8CP label) { m_label.AssignOrClear(label); }

    //! Get the visibility in model lists shown to the user
    bool GetInGuiList() const { return m_inGuiList; }

    //! Set the visibility in model lists shown to the user
    void SetInGuiList(bool val) { m_inGuiList = val; }

    //! @name Dynamic casting to DgnModel subclasses
    //@{
    GeometricModelCP ToGeometricModel() const {return _ToGeometricModel();} //!< more efficient substitute for dynamic_cast<GeometricModelCP>(model)
    InformationModelCP ToInformationModel() const {return _ToInformationModel();} //!< more efficient substitute for dynamic_cast<InformationModelCP>(model)
    DefinitionModelCP ToDefinitionModel() const {return _ToDefinitionModel();} //!< more efficient substitute for dynamic_cast<DefinitionModelCP>(model)
    GeometricModel2dCP ToGeometricModel2d() const {return _ToGeometricModel2d();} //!< more efficient substitute for dynamic_cast<GeometricModel2dCP>(model)
    GeometricModel3dCP ToGeometricModel3d() const {return _ToGeometricModel3d();} //!< more efficient substitute for dynamic_cast<GeometricModel3dCP>(model)
    SpatialModelCP ToSpatialModel() const {return _ToSpatialModel();} //!< more efficient substitute for dynamic_cast<SpatialModelCP>(model)
    SectionDrawingModelCP ToSectionDrawingModel() const {return _ToSectionDrawingModel();} //!< more efficient substitute for dynamic_cast<SectionDrawingModelCP>(model)
    SheetModelCP ToSheetModel() const {return _ToSheetModel();} //!< more efficient substitute for dynamic_cast<SheetModelCP>(model)
    GroupInformationModelCP ToGroupInformationModel() const {return _ToGroupInformationModel();} //!< more efficient substitute for dynamic_cast<GroupInformationModelCP>(model)
    GeometricModelP ToGeometricModelP() {return const_cast<GeometricModelP>(_ToGeometricModel());} //!< more efficient substitute for dynamic_cast<GeometricModelP>(model)
    InformationModelP ToInformationModelP() {return const_cast<InformationModelP>(_ToInformationModel());} //!< more efficient substitute for dynamic_cast<InformationModelP>(model)
    DefinitionModelP ToDefinitionModelP() {return const_cast<DefinitionModelP>(_ToDefinitionModel());} //!< more efficient substitute for dynamic_cast<DefinitionModelP>(model)
    GeometricModel2dP ToGeometricModel2dP() {return const_cast<GeometricModel2dP>(_ToGeometricModel2d());} //!< more efficient substitute for dynamic_cast<GeometricModel2dP>(model)
    GeometricModel3dP ToGeometricModel3dP() {return const_cast<GeometricModel3dP>(_ToGeometricModel3d());} //!< more efficient substitute for dynamic_cast<GeometricModel3dP>(model)
    SpatialModelP ToSpatialModelP() {return const_cast<SpatialModelP>(_ToSpatialModel());} //!< more efficient substitute for dynamic_cast<SpatialModelP>(model)
    SectionDrawingModelP ToSectionDrawingModelP() {return const_cast<SectionDrawingModelP>(_ToSectionDrawingModel());} //!< more efficient substitute for dynamic_cast<SectionDrawingModelP>(model)
    SheetModelP ToSheetModelP() {return const_cast<SheetModelP>(_ToSheetModel());}//!< more efficient substitute for dynamic_cast<SheetModelP>(model)
    GroupInformationModelP ToGroupInformationModelP() {return const_cast<GroupInformationModelP>(_ToGroupInformationModel());}//!< more efficient substitute for dynamic_cast<GroupInformationModelP>(model)

    bool IsGeometricModel() const { return nullptr != ToGeometricModel(); }
    bool IsSpatialModel() const { return nullptr != ToSpatialModel(); }
    bool Is2dModel() const { return nullptr != ToGeometricModel2d(); }
    bool Is3dModel() const { return nullptr != ToGeometricModel3d(); }
    bool IsInformationModel() const { return nullptr != ToInformationModel(); }
    bool IsDefinitionModel() const { return nullptr != ToDefinitionModel(); }
    bool IsSheetModel() const { return nullptr != ToSheetModel(); }
    bool IsGroupInformationModel() const { return nullptr != ToGroupInformationModel(); }
    bool IsDictionaryModel() const { return DictionaryId() == GetModelId(); }
    //@}

    //! Get the DgnDb of this DgnModel.
    DgnDbR GetDgnDb() const {return m_dgndb;}

    //! Insert this model into the DgnDb.
    //! @return DgnDbStatus::Success if this model was successfully inserted, error otherwise.
    DGNPLATFORM_EXPORT DgnDbStatus Insert();

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
    DGNPLATFORM_EXPORT DgnModelPtr Clone(DgnCode newCode) const;

    //! Make a persitent copy of the specified DgnModel and its contents.
    //! @param[in] model The model to copy
    //! @param[in] newCode The DgnCode for the new DgnModel.
    //! @see Import
    DGNPLATFORM_EXPORT static DgnModelPtr CopyModel(DgnModelCR model, DgnCode newCode);

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
    //! Returns the ID used by the default GroupInformationModel associated with each DgnDb
    static DgnModelId GroupInformationId() { return DgnModelId((uint64_t)2LL); }

    //! This method is called when it is time to validate changes that have been made to the model's content during the transaction.
    //! This method is called by the transaction manager after all element-level changes have been validated and all root models have been solved.
    //! This method is called only if elements in this model were added, deleted, or modified or if this model object itself was added or modified.
    //! This method allows a subclass to apply validation logic that requires a view of the entire model and possibly of root models.
    //! This method may add, delete, or modify elements in this model.
    //! To indication a validation error, call TxnManager::ReportError. If the error is marked as fatal, then the transaction will be rolled back.
    //! @note This method must make changes of any kind to any other model. Dependent models will be validated later.
    void OnValidate() { _OnValidate(); }

    //! Creates a DgnCode for a model with the given name, associated with the default DgnAuthority for models.
    static DgnCode CreateModelCode(Utf8StringCR modelName, Utf8StringCR nameSpace="") { return ModelAuthority::CreateModelCode(modelName, nameSpace); }

    //! Disclose any locks which must be acquired and/or codes which must be reserved in order to perform the specified operation on this model.
    //! @param[in]      request  Request to populate
    //! @param[in]      opcode   The operation to be performed
    //! @return RepositoryStatus::Success, or an error code if for example a required lock or code is known to be unavailable without querying the repository manager.
    //! @note If you override this function you @b must call T_Super::_PopulateRequest(), forwarding its status.
    RepositoryStatus PopulateRequest(IBriefcaseManager::Request& request, BeSQLite::DbOpcode opcode) const { return _PopulateRequest(request, opcode); }
};

//=======================================================================================
//! A DgnModel that contains geometric DgnElements.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Keith.Bentley   03/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometricModel : DgnModel
{
    DEFINE_T_SUPER(DgnModel);
    friend struct DgnQueryView;

public:
    //=======================================================================================
    //! The DisplayInfo for a DgnModel. These are stored within a "DisplayInfo"
    //! node of the JSON value that's serialized as a string in "Properties" column of the DgnModel table.
    //! @ingroup GROUP_DgnModel
    //=======================================================================================
    struct DisplayInfo
    {
    friend struct GeometricModel;

    private:
        struct FormatterFlags
            {
            uint32_t m_linearUnitMode : 2;
            uint32_t m_linearPrecType : 4;
            uint32_t m_linearPrecision : 8;
            uint32_t m_angularMode : 3;
            uint32_t m_angularPrecision : 8;
            uint32_t m_directionMode : 2;
            uint32_t m_directionClockwise : 1;
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
        DisplayInfo()
            {
            m_formatterFlags.m_linearUnitMode = 0;
            m_formatterFlags.m_linearPrecType = 0;
            m_formatterFlags.m_linearPrecision = 0;
            m_formatterFlags.m_angularMode = 0;
            m_formatterFlags.m_angularPrecision = 0;
            m_formatterFlags.m_directionMode = 0;
            m_formatterFlags.m_directionClockwise = 0;
            m_roundoffRatio = 0;
            m_formatterBaseDir = 0;
            m_roundoffUnit = 0;
            m_subUnit.Init(UnitBase::Meter, UnitSystem::Metric, 1.0, 1.0, "m");
            m_masterUnit = m_subUnit;
            }

        void FromJson(Json::Value const& inValue);
        void ToJson(Json::Value& outValue) const;

        //! Set master units and sub-units. Units must be valid and comparable.
        DGNPLATFORM_EXPORT BentleyStatus SetUnits(UnitDefinitionCR newMasterUnit, UnitDefinitionCR newSubUnit);
        void SetLinearUnitMode(DgnUnitFormat value) { m_formatterFlags.m_linearUnitMode = (uint32_t) value; }
        void SetLinearPrecision(PrecisionFormat value)
            {
            m_formatterFlags.m_linearPrecType = static_cast<uint32_t>(DoubleFormatter::GetTypeFromPrecision(value));
            m_formatterFlags.m_linearPrecision = DoubleFormatter::GetByteFromPrecision(value);
            }
        void SetAngularMode(AngleMode value) { m_formatterFlags.m_angularMode = (uint32_t) value; }
        void SetAngularPrecision(AnglePrecision value) { m_formatterFlags.m_angularPrecision = (uint32_t) value; }
        void SetDirectionMode(DirectionMode value) { m_formatterFlags.m_directionMode = (uint32_t) value; }
        void SetDirectionClockwise(bool value) { m_formatterFlags.m_directionClockwise = value; }
        void SetDirectionBaseDir(double value) { m_formatterBaseDir = value; }
        DgnUnitFormat GetLinearUnitMode() const { return (DgnUnitFormat) m_formatterFlags.m_linearUnitMode; }
        PrecisionFormat GetLinearPrecision() const { return DoubleFormatter::ToPrecisionEnum((PrecisionType) m_formatterFlags.m_linearPrecType, m_formatterFlags.m_linearPrecision); }
        AngleMode GetAngularMode() const { return (AngleMode) m_formatterFlags.m_angularMode; }
        AnglePrecision GetAngularPrecision() const { return (AnglePrecision) m_formatterFlags.m_angularPrecision; }
        DirectionMode GetDirectionMode() const { return (DirectionMode) m_formatterFlags.m_directionMode; }
        bool GetDirectionClockwise() const { return m_formatterFlags.m_directionClockwise; }
        double GetDirectionBaseDir() const { return m_formatterBaseDir; }
        void SetRoundoffUnit(double roundoffUnit, double roundoffRatio) { m_roundoffUnit = roundoffUnit; m_roundoffRatio = roundoffRatio; }
        double GetRoundoffUnit() const { return m_roundoffUnit; }
        double GetRoundoffRatio() const { return m_roundoffRatio; }
        FormatterFlags GetFormatterFlags() const { return m_formatterFlags; }

        //! Get the master units for this DgnModel.
        //! Master units are the major display units for coordinates in a DgnModel (e.g. "Meters", or "Feet").
        //! @see SetUnits, GetSubUnits
        UnitDefinitionCR GetMasterUnits() const { return m_masterUnit; }

        //! Get the sub-units for this DgnModel.
        //! Sub units are the minor readout units for coordinates in a DgnModel (e.g. "Centimeters, or "Inches").
        //! @see SetUnits, GetMasterUnits
        UnitDefinitionCR GetSubUnits() const { return m_subUnit; }

        //! Get the number of millimeters per master unit.
        //! @see GetMasterUnits
        DGNPLATFORM_EXPORT double GetMillimetersPerMaster() const;

        //! Get the number of sub units per master unit.
        //! @see GetSubUnits
        DGNPLATFORM_EXPORT double GetSubPerMaster() const;
    };

    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(GeometricModel::T_Super::CreateParams);
        DisplayInfo  m_displayInfo;

        //! Parameters to create a new instance of a GeometricModel.
        //! @param[in] dgndb The DgnDb for the new DgnModel
        //! @param[in] classId The DgnClassId for the new DgnModel.
        //! @param[in] code The DgnCode for the DgnModel
        //! @param[in] label Label of the new DgnModel
        //! @param[in] displayInfo The DisplayInfo for the new DgnModel.
        //! @param[in] inGuiList Controls the visibility of the new DgnModel in model lists shown to the user
        CreateParams(DgnDbR dgndb, DgnClassId classId, DgnCode code, Utf8CP label = nullptr, DisplayInfo displayInfo = DisplayInfo(), bool inGuiList = true)
            : T_Super(dgndb, classId, code, label, inGuiList), m_displayInfo(displayInfo)
            {}

        //! @private
        //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
        CreateParams(DgnModel::CreateParams const& params) : T_Super(params) { }

        DisplayInfo const& GetDisplayInfo() const { return m_displayInfo; } //!< Get the DisplayInfo
        void SetDisplayInfo(DisplayInfo const& displayInfo) { m_displayInfo = displayInfo; } //!< Set the DisplayInfo
    };

private:
    mutable DgnRangeTreeP m_rangeIndex;
    DisplayInfo  m_displayInfo;

    DGNPLATFORM_EXPORT void AllocateRangeIndex() const;
    void AddToRangeIndex(DgnElementCR);
    void RemoveFromRangeIndex(DgnElementCR);
    void UpdateRangeIndex(DgnElementCR modified, DgnElementCR original);
    
protected:
    void ClearRangeIndex();

    virtual void _SetFilled() override {T_Super::_SetFilled(); AllocateRangeIndex();}

    //! Get the coordinate space in which the model's geometry is defined.
    virtual CoordinateSpace _GetCoordinateSpace() const = 0;

    //! Add non-element graphics for this DgnModel to the scene.
    //! The scene is first generated by QueryView from the elements in the view.
    //! A subclass can override this method to add non-element-based graphics to the scene. Or, a subclass
    //! can override this method to do add elements that QueryView would normally exclude.
    //! <h2>Coordinate Systems</h2>
    //! A DgnDb defines the physical coordinate system. 
    //! That physical coordinate system can be associated with a single Geographic Coordinate System (GCS). See DgnUnits::GetDgnGCS.
    //! The implementation must transform external data into the coordinate system of the DgnDb as necessary before adding graphics to the scene.
    //! <h2>Displaying external data using progressive display</h2>
    //! An implementation of _AddGraphicsToScene is required to be very fast to keep the client thread responsive. If data is not immediately available, you should
    //! a) make arrangements to obtain the data in the background and b) schedule a ProgressiveTask to display it when available.
    virtual void _AddSceneGraphics(SceneContextR) const {}

    //! Add "terrain" graphics for this DgnModel. Terrain graphics are drawn with the scene graphics every time the camera moves. The difference between
    //! them is that this method is called every frame whereas _AddGraphicsToSceen is only called when the query thread completes. Terrain graphics must 
    //! be re-added every time this method is called or they will disappear.
    //! An implementation of _AddTerrain is required to be very fast to keep the client thread responsive. If data is not immediately available, you should
    //! a) make arrangements to obtain the data in the background and b) schedule a ProgressiveTask to display it when available.
    virtual void _AddTerrainGraphics(TerrainContextR) const {}

    virtual void _OnFitView(FitContextR) {}
    virtual void _DrawModel(ViewContextR) {}

    DGNPLATFORM_EXPORT virtual DgnRangeTree* _GetRangeIndexP(bool create) const override;
    DGNPLATFORM_EXPORT virtual AxisAlignedBox3d _QueryModelRange() const;//!< @private
    DGNPLATFORM_EXPORT virtual void _EmptyModel() override;
    DGNPLATFORM_EXPORT virtual void _RegisterElement(DgnElementCR element) override;
    DGNPLATFORM_EXPORT virtual void _OnDeletedElement(DgnElementCR element) override;
    DGNPLATFORM_EXPORT virtual void _OnReversedAddElement(DgnElementCR element) override;
    DGNPLATFORM_EXPORT virtual void _OnUpdatedElement(DgnElementCR modified, DgnElementCR original) override;
    DGNPLATFORM_EXPORT virtual void _OnReversedUpdateElement(DgnElementCR modified, DgnElementCR original) override;
    DGNPLATFORM_EXPORT virtual void _WriteJsonProperties(Json::Value&) const override;
    DGNPLATFORM_EXPORT virtual void _ReadJsonProperties(Json::Value const&) override;

    virtual GeometricModelCP _ToGeometricModel() const override final {return this;}
    
    explicit GeometricModel(CreateParams const& params) : T_Super(params), m_rangeIndex(nullptr), m_displayInfo(params.m_displayInfo) {}

public:

    //! Get the AxisAlignedBox3d of the contents of this model.
    AxisAlignedBox3d QueryModelRange() const {return _QueryModelRange();}

    //! Get the coordinate space in which the model's geometry is defined.
    CoordinateSpace GetCoordinateSpace() const {return _GetCoordinateSpace();}

    //! Get a writable reference to the DisplayInfo for this model.
    DisplayInfo& GetDisplayInfoR() { return m_displayInfo; }

    //! Get the Properties for this model.
    DisplayInfo const& GetDisplayInfo() const { return m_displayInfo; }
};

//=======================================================================================
//! A DgnModel that contains only 3-dimensional DgnElements.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Keith.Bentley   03/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometricModel3d : GeometricModel
{
    DEFINE_T_SUPER(GeometricModel);

protected:
    virtual GeometricModel3dCP _ToGeometricModel3d() const override final {return this;}
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element) override;

public:
    explicit GeometricModel3d(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A GeometricModel2d is a infinite planar model that contains only 2-dimensional DgnElements. Coordinates values are X,Y.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometricModel2d : GeometricModel
{
    DGNMODEL_DECLARE_MEMBERS(DGN_CLASSNAME_GeometricModel2d, GeometricModel);

protected:
    GeometricModel2dCP _ToGeometricModel2d() const override final {return this;}
    CoordinateSpace _GetCoordinateSpace() const override final {return CoordinateSpace::Local;}
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element);

public:
    explicit GeometricModel2d(CreateParams const& params, DPoint2dCR origin=DPoint2d::FromZero()) : T_Super(params) {}
};

//=======================================================================================
//! A GraphicalModel2d contains 2-dimensional geometric elements for graphical presentation purposes (as opposed to analytical purposes).
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Shaun.Sewall    02/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GraphicalModel2d : GeometricModel2d
{
    DEFINE_T_SUPER(GeometricModel2d);

public:
    explicit GraphicalModel2d(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A GeometricModel3d that occupies physical space in the DgnDb. All SpatialModels in a DgnDb have the same coordinate
//! space (CoordinateSpace::World), aka "Physical Space".
//! DgnElements from SpatialModels are indexed in the persistent range tree of the DgnDb (the DGN_VTABLE_SpatialIndex).
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SpatialModel : GeometricModel3d
{
    DGNMODEL_DECLARE_MEMBERS(DGN_CLASSNAME_SpatialModel, GeometricModel3d);
protected:
    SpatialModelCP _ToSpatialModel() const override final {return this;}
    CoordinateSpace _GetCoordinateSpace() const override final {return CoordinateSpace::World;}

public:
    explicit SpatialModel(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Shaun.Sewall    04/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE InformationModel : DgnModel
{
    DEFINE_T_SUPER(DgnModel);
protected:
    InformationModelCP _ToInformationModel() const override final {return this;}
    explicit InformationModel(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A model which contains only definitions.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DefinitionModel : InformationModel
{
    DGNMODEL_DECLARE_MEMBERS(DGN_CLASSNAME_DefinitionModel, InformationModel);
protected:
    DefinitionModelCP _ToDefinitionModel() const override final {return this;}
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element) override;
public:
    explicit DefinitionModel(CreateParams const& params) : T_Super(params) { }

    static DefinitionModelPtr Create(CreateParams const& params) { return new DefinitionModel(params); }
};

//=======================================================================================
//! A definition model which contains definitions like materials and styles which are used
//! throughout a DgnDb. Each DgnDb has exactly one DictionaryModel.
//! A DictionaryModel can contain @em only DefinitionElements.
//! The dictionary model cannot be copied or deleted. In general, dictionary elements
//! are copied from one dictionary model to another, often indirectly as the result of
//! copying another element which depends upon them.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DictionaryModel : DefinitionModel
{
    DGNMODEL_DECLARE_MEMBERS(DGN_CLASSNAME_DictionaryModel, DefinitionModel);
protected:
    virtual DgnDbStatus _OnDelete() override { return DgnDbStatus::WrongModel; }
    virtual void _OnDeleted() override { BeAssert(false && "The dictionary model cannot be deleted"); }
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element) override;
    DGNPLATFORM_EXPORT DgnModelPtr virtual _CloneForImport(DgnDbStatus* stat, DgnImportContext& importer) const override;
public:
    explicit DictionaryModel(CreateParams const& params) : T_Super(params) { }
};

//=======================================================================================
//! A model which contains only GroupInformationElements.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Shaun.Sewall    05/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GroupInformationModel : InformationModel
{
    DGNMODEL_DECLARE_MEMBERS(DGN_CLASSNAME_GroupInformationModel, InformationModel);
protected:
    GroupInformationModelCP _ToGroupInformationModel() const override final {return this;}
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element) override;
public:
    explicit GroupInformationModel(CreateParams const& params) : T_Super(params) {}
    DGNPLATFORM_EXPORT static GroupInformationModelPtr Create(DgnDbR db, DgnCode const& code = DgnCode());
};

struct ComponentDef;

/*=======================================================================================*//**
* A ComponentModel is used by a ComponentDef 
* @bsiclass                                                    Keith.Bentley   10/11
**//*=======================================================================================*/
struct EXPORT_VTABLE_ATTRIBUTE ComponentModel : GeometricModel3d
{
    DGNMODEL_DECLARE_MEMBERS(DGN_CLASSNAME_ComponentModel, GeometricModel3d);
    
    friend struct ComponentDef;

protected:
    Utf8String m_componentECClass;

    CoordinateSpace _GetCoordinateSpace() const override final {return CoordinateSpace::Local;}
    DGNPLATFORM_EXPORT DgnDbStatus _OnDelete() override;
    //DgnDbStatus _OnUpdate() override;
    DGNPLATFORM_EXPORT void _InitFrom(DgnModelCR other) override;

    DGNPLATFORM_EXPORT virtual void _WriteJsonProperties(Json::Value&) const override;
    DGNPLATFORM_EXPORT virtual void _ReadJsonProperties(Json::Value const&) override;

    DGNPLATFORM_EXPORT ComponentModel(DgnDbR db, DgnCode, Utf8StringCR defName);

public:
    ComponentModel(CreateParams const& params) : GeometricModel3d(params) {;} //!< @private

    //! Create a ComponentModel that can be used by a component definition
    //! @param db The DgnDb that is intended to contain the new model
    //! @param componentDefClassECSqlName The full ECSQL name of the component definition ECClass
    //! @return a new, non-persistent component model
    DGNPLATFORM_EXPORT static ComponentModelPtr Create(DgnDbR db, Utf8StringCR componentDefClassECSqlName);

    Utf8StringCR GetComponentECClassFullName() const {return m_componentECClass;}
};

//=======================================================================================
//! A component definition.
// @bsiclass                                                    Sam.Wilson      12/15
//=======================================================================================
struct ComponentDef : RefCountedBase
{
 private:
    DgnDbR m_db;
    ECN::ECClassCR m_class;
    ECN::IECInstancePtr m_ca;
    ComponentModelPtr m_model;

    static Utf8String GetCaValueString(ECN::IECInstanceCR, Utf8CP propName);
    ECN::IECInstancePtr GetPropSpecCA(ECN::ECPropertyCR prop);

    DgnCode CreateVariationCode(Utf8StringCR slnId) { return ComponentAuthority::CreateVariationCode(slnId, GetName()); }

    //! Test if the specified code is that of a component variation instance element.
    static bool IsComponentVariationCode(DgnCode const& icode);

    //! This is the basic logic to create an instance of this component. It is used to create variations and singletons.
    DgnElementCPtr MakeInstance0(DgnDbStatus* stat, DgnModelR targetModel, ECN::IECInstanceCR parameters, DgnCode const& code);
    
    //! Compare two instances and return true if their parameter values are the same.
    //! @note This function infers that that parameters to be compared are the properties of \a lhs that are not ECInstanceId and are not NULL.
    bool HaveEqualParameters(ECN::IECInstanceCR lhs, ECN::IECInstanceCR rhs, bool compareOnlyInstanceParameters = true);

    //! Copy instance parameters from source to target
    void CopyInstanceParameters(ECN::IECInstanceR target, ECN::IECInstanceCR source);

    ComponentDef(DgnDbR db, ECN::ECClassCR componentDefClass);
    ~ComponentDef();

 public:
    static Utf8String GetClassECSqlName(ECN::ECClassCR cls) {Utf8String ns(cls.GetSchema().GetNamespacePrefix()); return ns.append(".").append(cls.GetName());}
    Utf8String GetClassECSqlName() const {return GetClassECSqlName(m_class);}

    //! @private - called only by componenteditor
    DGNPLATFORM_EXPORT DgnDbStatus GenerateElements(DgnModelR destModel, ECN::IECInstanceCR variationSpec);

    //! Get the list of properties that are the main inputs to the component's element generator
    DGNPLATFORM_EXPORT bvector<Utf8String> GetInputs() const;

    //! Get the list of input properties in a form that can be put into a SQL SELECT statement
    DGNPLATFORM_EXPORT Utf8String GetInputsForSelect() const;

    //! Get a list of all of the component definitions derived from the specified base class in the specified DgnDb
    //! @param[out] componentDefs    Where to return the results
    //! @param[in] db               The Db to search
    //! @param[in] baseClass        The base class to start from
    DGNPLATFORM_EXPORT static void QueryComponentDefs(bvector<DgnClassId>& componentDefs, DgnDbR db, ECN::ECClassCR baseClass);

    //! Make a ComponentDef object
    //! @param db           The DgnDb that contains the component def
    //! @param componentDefClass   The ECClass that defines the component
    //! @param status       If not null, an error code in case of failure
    //! @note possible status values include:
    //! DgnDbStatus::BadModel - the component definition specifies a model, but the model does not exist in \a db
    //! DgnDbStatus::InvalidCategory - the component definition's category cannot be found in \a db
    //! DgnDbStatus::WrongClass - \a componentDefClass is not a component definition ECClass
    DGNPLATFORM_EXPORT static ComponentDefPtr FromECClass(DgnDbStatus* status, DgnDbR db, ECN::ECClassCR componentDefClass);

    //! Make a ComponentDef object
    //! @param db           The DgnDb that contains the component def
    //! @param componentDefClassId Identfies the ECClass that defines the component
    //! @param status       If not null, an error code in case the component definition could not be returned
    //! @see FromECClass
    DGNPLATFORM_EXPORT static ComponentDefPtr FromECClassId(DgnDbStatus* status, DgnDbR db, DgnClassId componentDefClassId);

    //! Make a ComponentDef object
    //! @param db           The DgnDb that contains the component def
    //! @param ecsqlClassName   The full ECSQL name of the ECClass that defines the component
    //! @param status       If not null, an error code in case of failure
    //! @see FromECClass
    DGNPLATFORM_EXPORT static ComponentDefPtr FromECSqlName(DgnDbStatus* status, DgnDbR db, Utf8StringCR ecsqlClassName);

    //! Get the ComponentDef corresponding to the specified component instance
    //! @param instance     An element that might be an instance of a component
    //! @param status       If not null, an error code in case of failure
    //! @see FromECClass
    DGNPLATFORM_EXPORT static ComponentDefPtr FromInstance(DgnDbStatus* status, DgnElementCR instance);

    //! Get the ComponentDef corresponding to the specified ComponentModel
    //! @param model        A ComponentModel
    //! @param status       If not null, an error code in case of failure
    //! @see FromECClass
    DGNPLATFORM_EXPORT static ComponentDefPtr FromComponentModel(DgnDbStatus* status, ComponentModelCR model);

    //! Delete a component definition.
    //! @param db           The DgnDb that contains the component def
    //! @param ecsqlClassName   The full ECSQL name of the ECClass that defines the component
    //! @return non-zero error status if \a componentDefClass is not a component definition class
    //! @note All existing variations and instances are also deleted. The component def's work model is also deleted.
    DGNPLATFORM_EXPORT static DgnDbStatus DeleteComponentDef(DgnDbR db, Utf8StringCR ecsqlClassName);

    struct ElementGenerator
        {
        //! Generate geometry
        //! @param destModel The model where instances of this component will be placed
        //! @param componentWorkModel The model where intermediate results may be written. Everything written to the work model will be harvested when instances are placed.
        //! @param variationSpec The input parameters and their values
        //! @param cdef The definition of the component that is to be instanced
        //! @return non-zero error status if elements could not be generated
        virtual DgnDbStatus _GenerateElements(DgnModelR destModel, DgnModelR componentWorkModel, ECN::IECInstanceCR variationSpec, ComponentDef& cdef) = 0;
        };

    //! Component parameter variation scope
    enum class ParameterVariesPer
    {
        Instance = 0,   //!< Varies per instance
        Variation = 1   //!< Varies per Variation
    };

    DgnDbR GetDgnDb() const {return m_db;}

    ECN::ECClassCR GetECClass() const {return m_class;}

    DGNPLATFORM_EXPORT Utf8String GetComponentName() const;
    DGNPLATFORM_EXPORT ComponentModelR GetModel();

    DGNPLATFORM_EXPORT bool UsesTemporaryModel() const;

    //! Get the name of this component
    Utf8String GetName() const {return m_class.GetName();}

    DGNPLATFORM_EXPORT Utf8String GetElementGeneratorName() const;

    DGNPLATFORM_EXPORT Utf8String GetCategoryName() const;
    DGNPLATFORM_EXPORT DgnCategoryId QueryCategoryId() const;

    DGNPLATFORM_EXPORT Utf8String GetCodeAuthorityName() const;
    DGNPLATFORM_EXPORT DgnAuthorityCPtr GetCodeAuthority() const;

    DGNPLATFORM_EXPORT ECN::IECInstancePtr MakeVariationSpec();

    //! Return the properties of the specified instance of this component in the form of an IECInstance.
    //! @param instance The component instance element.
    //! @returns nullptr if \a instance is an instance of a component
    DGNPLATFORM_EXPORT static ECN::IECInstancePtr GetParameters(DgnElementCR instance);
    
    DGNPLATFORM_EXPORT static void DumpScriptOnlyParameters(ECN::IECInstanceCR props, Utf8CP title);

    //! Controls export/import of ComponentDefs
    struct ExportOptions
        {
        bool m_exportSchema, m_exportCategory, m_embedScript;
        //! Initialize ExporParams
        //! @param exportSchema     Export the ECSchema that includes this component definition's ECClass to the destination DgnDb? The export will fail if \a destDb does not contain this component definition's ECClass. Pass false if you expect the schema already to be defined. Pass true to update an existing schema.
        //! @param exportCategory   Export the Category used by this component definition to the destination DgnDb? The export will fail if \a destDb does not contain this component definition's Category. Pass false if you expect the category already to be defined. If you pass true and if the category already exists, it is \em not updated.
        //! @param embedScript      Copy and store the script used by this component definition in the destination DgnDb?  Pass false if the script is to be loaded from disk or the network. If true and if the script already exists in the destination DgnDb, it will be updated.
        ExportOptions(bool exportSchema = true, bool exportCategory = true, bool embedScript = true) : m_exportSchema(exportSchema), m_exportCategory(exportCategory), m_embedScript(embedScript) {;}
        };

    //! Export this component definition to the specified DgnDb. 
    //! @param context The id remapping context to use
    //! @param options Export options
    //! @return non-zero error if the export/import failed.
    //! @see ExportVariations
    DGNPLATFORM_EXPORT DgnDbStatus Export(DgnImportContext& context, ExportOptions const& options = ExportOptions());

    //! Export variations of this component definition to the the specified model. 
    //! @param destVariationsModel Write copies of variations to this model.
    //! @param sourceVariationsModel Query variations in this model.
    //! @param context The id remapping context to use
    //! @param variationFilter If specified, the variations to export. If not specified and if \a destVariationsModel is specified, then all variations are exported.
    //! @return non-zero error if the import failed.
    //! @see Export
    DGNPLATFORM_EXPORT DgnDbStatus ExportVariations(DgnModelR destVariationsModel, DgnModelId sourceVariationsModel, DgnImportContext& context, bvector<DgnElementId> const& variationFilter = bvector<DgnElementId>());

    //! Creates a variation of a component, based on the specified parameters.
    //! @param[out] stat        Optional. If not null and if the variation cannot be computed, then an error code is stored here to explain what happened, as explained below.
    //! @param[in] destModel    The output model, where the variation instance should be stored.
    //! @param[in] variationParameters  The parameters that specify the solution. Note that the ECClass of this instance also identifies the component.
    //! @param[in] variationName The name of the variation that is to be created in the destModel. This cannot be blank, and it must be unique among all variations for this component.
    //! @return A handle to the variation instance that was created and persisted in \a destModel. If more than one element was created, the returned element is the parent. If the instance
    //! cannot be created, then this function returns nullptr and sets \a stat to a non-error status. Some of the possible error values include:
    //!     * DgnDbStatus::WrongClass - \a variationParameters is not an instance of a component definition ECClass.
    //!     * DgnDbStatus::BadRequest - The component's geometry could not be generated, possibly because the values in \a variationParameters are invalid.
    //!     * DgnDbStatus::DuplicateCode - An element already exists with a name equal to \a variationName
    //! @see MakeInstanceOfVariation
    DGNPLATFORM_EXPORT static DgnElementCPtr MakeVariation(DgnDbStatus* stat, DgnModelR destModel, ECN::IECInstanceCR variationParameters, Utf8StringCR variationName);

    //! Delete a variation.
    //! @return non-zero error status if \a variation is not a variation of any componentdef, or if it is a variation but is currently used by existing instances.
    DGNPLATFORM_EXPORT static DgnDbStatus DeleteVariation(DgnElementCR variation);

    //! Look up a variation by name
    //! @param[in] variationName The name of the variation to look up.
    //! @return the variation or an invalid handle if not found
    DGNPLATFORM_EXPORT DgnElementCPtr QueryVariationByName(Utf8StringCR variationName);

    //! Search for all variations of this component definition
    //! @param variations    Where to return the IDs of the captured solutions
    //! @param variationModelId The model that contains the variations
    //! @see MakeVariation, QueryVariationByName
    DGNPLATFORM_EXPORT void QueryVariations(bvector<DgnElementId>& variations, DgnModelId variationModelId);

    //! Get the scope of the specified property
    DGNPLATFORM_EXPORT ParameterVariesPer GetVariationScope(ECN::ECPropertyCR prop);

    //! Make either a persistent copy of a specified variation or a unique instance of the component definition.
    //! If \a variation has instance parameters, then the \a instanceParameters argument may be passed into specific the instance parameter values to use.
    //! If the values in \a instanceParameters differs from the instance parameters of \a variation, then a unique instance is created.
    //! If \a instanceParameters is not specified or if it matches the instance parameters of \a variation, then a copy of \a variation is made.
    //! @note Per-type parameters are ignored when comparing \a parameters to \a variation. It is illegal for the caller to specify new values for per-type parameters.
    //! @param[out] stat        Optional. If not null, then an error code is stored here in case the copy fails. Set to DgnDbStatus::WrongClass if \a variation is not an instance of a component. Otherwise, see ElementCopier for possible error codes.
    //! @param[in] targetModel  The model where the instance is to be inserted
    //! @param[in] variation    The variation that is to be turned into an instance.
    //! @param[in] instanceParameters   The instance parameters to use. If null, then default instance parameter values are used. Pass null if the variation has no instance parameters.
    //! @param[in] code         Optional. The code to assign to the new instance. If invalid, then a code will be generated by the CodeAuthority associated with this component definition.
    //! @return A handle to the instance that was created and persisted in \a destModel. If more than one element was created, the returned element is the parent. If the instance
    //! cannot be created, then this function returns nullptr and sets \a stat to a non-error status. Some of the possible error values include:
    //!     * DgnDbStatus::WrongClass - \a variation is not an instance of a component definition ECClass or \a the ECClass of instanceParms and of \a variation do not match.
    //!     * DgnDbStatus::WrongDgnDb - \a variation and \a targetModel must both be in the same DgnDb.
    //!     * DgnDbStatus::NotFound - \a parameters does not match the parameters of \a variation. Call 
    //! @see MakeVariation
    DGNPLATFORM_EXPORT static DgnElementCPtr MakeInstanceOfVariation(DgnDbStatus* stat, DgnModelR targetModel, DgnElementCR variation, ECN::IECInstanceCP instanceParameters, DgnCode const& code = DgnCode());

    //! Make a unique instance that is not based on a pre-defined variation. This method must be used if \a parameters include per-instance parameters that do not match the default values
    //! of any pre-defined variation. This method may also be used for components that do not have pre-defined variations.
    //! @note This function should not be used when the compponent definition does define a set of variations. In that case, call MakeInstanceOfVariation instead.
    //! @param[out] stat        Optional. If not null, then an error code is stored here in case the creation of the instance fails.
    //! @param[in] targetModel  The model where the instance is to be inserted
    //! @param[in] parameters   The parameters that specify the desired variation
    //! @param[in] code         Optional. The code to assign to the new item. If invalid, then a code will be generated by the CodeAuthority associated with this component model
    //! @return A handle to the instance that was created and persisted in \a destModel. If more than one element was created, the returned element is the parent. If the instance
    //! cannot be created, then this function returns nullptr and sets \a stat to a non-error status. Some of the possible error values include:
    //!     * DgnDbStatus::WrongClass - \a parameters is not an instance of a component definition ECClass.
    //!     * DgnDbStatus::WrongDgnDb - \a parameters and \a targetModel must both be in the same DgnDb.
    //!     * DgnDbStatus::BadRequest - The component's geometry could not be generated, possibly because the values in \a parameters are invalid.
    //! @see MakeInstanceOfVariation
    DGNPLATFORM_EXPORT static DgnElementCPtr MakeUniqueInstance(DgnDbStatus* stat, DgnModelR targetModel, ECN::IECInstanceCR parameters, DgnCode const& code = DgnCode());
};

//========================================================================================
//! A component definition parameter 
//! Defines the standard JSON format for a parameter
//========================================================================================
struct TsComponentParameter
    {
    ComponentDef::ParameterVariesPer m_variesPer;
    ECN::ECValue m_value;
    bool m_isForScriptOnly;

    TsComponentParameter() : m_variesPer(ComponentDef::ParameterVariesPer::Instance), m_isForScriptOnly(true) {;}
    //! Construct a new Parameter
    TsComponentParameter(ComponentDef::ParameterVariesPer s, ECN::ECValueCR v) : m_variesPer(s), m_value(v), m_isForScriptOnly(true) {;}
    //! From JSON
    DGNPLATFORM_EXPORT explicit TsComponentParameter(Json::Value const&);
    //! To JSON
    DGNPLATFORM_EXPORT Json::Value ToJson() const;
    //! Get the scope of this parameter
    //ComponentDef::ParameterVariesPer GetScope() const {return m_variesPer;}
    //! Get the value of this parameter
    //ECN::ECValueCR GetValue() const {return m_value;}
    //! Set the value of this parameter
    DgnDbStatus SetValue(ECN::ECValueCR newValue);

    bool EqualValues(TsComponentParameter const& rhs) {return m_value.Equals(rhs.m_value);}
    bool operator==(TsComponentParameter const& rhs) const {return m_variesPer == rhs.m_variesPer && m_value.Equals(rhs.m_value);}
    };
    
//========================================================================================
//! A collection of named component definition parameters
//! Defines the standard JSON format for a parameter set
//========================================================================================
struct TsComponentParameterSet : bmap<Utf8String,TsComponentParameter>
    {
    TsComponentParameterSet() {}
    DGNPLATFORM_EXPORT TsComponentParameterSet(ComponentDefR, ECN::IECInstanceCR);
    DGNPLATFORM_EXPORT TsComponentParameterSet(Json::Value const& v);

    DGNPLATFORM_EXPORT Json::Value ToJson() const;
    DGNPLATFORM_EXPORT void ToECProperties(ECN::IECInstanceR) const;
    };

//=======================================================================================
//! A helper class that makes it easier to create a ComponentDefinition ECClass
//=======================================================================================
struct ComponentDefCreator
{
private:
    DgnDbR m_db;
    ECN::ECSchemaR m_schema;
    ECN::ECClassCR m_baseClass;
    Utf8String m_name;
    Utf8String m_scriptName;
    Utf8String m_categoryName;
    Utf8String m_codeAuthorityName;
    Utf8String m_modelName;
    Utf8String m_inputs;
    TsComponentParameterSet m_params;
    TsComponentParameterSet m_firstClassParams;
    TsComponentParameterSet m_adhocParams;
    
    ECN::IECInstancePtr CreatePropSpecCA();
    ECN::IECInstancePtr CreateSpecCA();
    ECN::IECInstancePtr AddSpecCA(ECN::ECClassR);

public:
    DGNPLATFORM_EXPORT static ECN::ECSchemaPtr GenerateSchema(DgnDbR, Utf8StringCR schemaNameIn);

    DGNPLATFORM_EXPORT static ECN::ECSchemaCP ImportSchema(DgnDbR db, ECN::ECSchemaCR schemaIn, bool updateExistingSchemas);
  
    ComponentDefCreator(DgnDbR db, ECN::ECSchemaR schema, Utf8StringCR name, ECN::ECClassCR baseClass, Utf8StringCR geomgen, Utf8StringCR cat, Utf8StringCR codeauth, TsComponentParameterSet const& params = TsComponentParameterSet())
        :m_db(db), m_schema(schema), m_baseClass(baseClass), m_name(name), m_scriptName(geomgen), m_categoryName(cat), m_codeAuthorityName(codeauth), m_params(params)
        {
        }

    //! Set the model name. The default is no model name, indicating that the component should use a temporary "sandbox" model.
    void SetModelName(Utf8StringCR n) {m_modelName=n;}

    TsComponentParameterSet& GetTsComponentParameterSetR() {return m_params;}

    DGNPLATFORM_EXPORT void AddInput(Utf8StringCR inp); //!< You can call this directly to mark existing (subclass) properties as inputs

    DGNPLATFORM_EXPORT ECN::ECClassCP GenerateECClass();
};

//=======================================================================================
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Shaun.Sewall    02/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DrawingModel : GraphicalModel2d
    {
    DEFINE_T_SUPER(GraphicalModel2d);

    public:
        explicit DrawingModel(CreateParams const& params) : T_Super(params) {}
    };

//=======================================================================================
//! A SectionDrawingModel is an infinite planar model that subdivides physical space into two halves.
//! The plane of a SectionDrawingModel is mapped into physical space such that the vertical direction (Y
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
//! @note Any (2d) point on a SectionDrawingModel corresponds to a single point in physical space.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SectionDrawingModel : DrawingModel
{
    DGNMODEL_DECLARE_MEMBERS(DGN_CLASSNAME_SectionDrawingModel, DrawingModel);
protected:
    SectionDrawingModelCP _ToSectionDrawingModel() const override final {return this;}
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsertElement(DgnElementR element) override;
public:
    SectionDrawingModel(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A sheet model is a GraphicalModel2d that has the following characteristics:
//!     - Has fixed extents (is not infinite), specified in meters.
//!     - Can contain @b views of other models, like pictures pasted on a photo album.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SheetModel : GraphicalModel2d
{
    DGNMODEL_DECLARE_MEMBERS(DGN_CLASSNAME_SheetModel, GraphicalModel2d);
public:
    struct CreateParams : GraphicalModel2d::CreateParams
    {
        DEFINE_T_SUPER(GraphicalModel2d::CreateParams);
        DPoint2d m_size;

        //! Parameters for creating a new SheetModel.
        //! @param[in] dgndb the DgnDb into which the SheetModel will be created
        //! @param[in] classId the DgnClassId of thew new SheetModel (must be or derive from SheetModel)
        //! @param[in] code the code of the new SheetModel
        //! @param[in] size the size of the SheetModel, in meters.
        //! @param[in] label Label of the new DgnModel
        //! @param[in] displayInfo the Properties of the new SheetModel
        //! @param[in] inGuiList Controls the visibility of the new DgnModel in model lists shown to the user
        CreateParams(DgnDbR dgndb, DgnClassId classId, DgnCode code, DPoint2d size, Utf8CP label = nullptr, DisplayInfo displayInfo = DisplayInfo(), bool inGuiList = true) :
            T_Super(dgndb, classId, code, label, displayInfo, inGuiList), m_size(size)
            {}

        explicit CreateParams(DgnModel::CreateParams const& params, DPoint2d size=DPoint2d::FromZero()) : T_Super(params), m_size(size) {}

        DPoint2dCR GetSize() const { return m_size; } //!< Get the size of the SheetModel to be created, in meters. 
        void SetSize(DPoint2dCR size) { m_size = size; } //!< Set the size of the SheetModel to be created, in meters. 
    };

private:
    DgnDbStatus BindInsertAndUpdateParams(BeSQLite::EC::ECSqlStatement& statement);

protected:
    DPoint2d m_size;

    SheetModelCP _ToSheetModel() const override final {return this;}

    DGNPLATFORM_EXPORT virtual void _InitFrom(DgnModelCR other) override;

    DGNPLATFORM_EXPORT DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParamsCR params) override;
    DGNPLATFORM_EXPORT DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& statement) override;
    DGNPLATFORM_EXPORT DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement) override;

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
        private: virtual Dgn::DgnModel* _CreateInstance(Dgn::DgnModel::CreateParams const& params) override {return new __classname__(__classname__::CreateParams(params));}\
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
    struct EXPORT_VTABLE_ATTRIBUTE Model : DgnDomain::Handler, IECSqlClassParamsProvider
    {
        friend struct Dgn::DgnModel;
        friend struct Dgn::DgnModels;
        DOMAINHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_Model, Model, DgnDomain::Handler, DGNPLATFORM_EXPORT)

    private:
        ECSqlClassParams m_classParams;

        ECSqlClassParams const& GetECSqlClassParams();
    protected:
        ModelHandlerP _ToModelHandler() override final {return this;}
        virtual DgnModelP _CreateInstance(DgnModel::CreateParams const& params) {return nullptr;}
        virtual uint64_t _ParseRestrictedAction(Utf8CP name) const override { return DgnModel::RestrictedAction::Parse(name); }
        DGNPLATFORM_EXPORT virtual DgnDbStatus _VerifySchema(DgnDomains&) override;

        //! Add the names of any subclass properties used by ECSql INSERT, UPDATE, and/or SELECT statements to the ECSqlClassParams list.
        //! If you override this method, you @em must invoke T_Super::_GetClassParams().
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParamsR params) override;

    public:
        //! Find an ModelHandler for a subclass of dgn.Model. This is just a shortcut for FindHandler with the base class
        //! of "dgn.Model".
        DGNPLATFORM_EXPORT static ModelHandlerP FindHandler(DgnDb const&, DgnClassId handlerId);

        //! Create an instance of a (subclass of) DgnModel from CreateParams.
        //! @param[in] params the parameters for the model
        DgnModelPtr Create(DgnModel::CreateParams const& params) {return _CreateInstance(params);}
    };

    //! The ModelHandler for Geometric2d
    struct EXPORT_VTABLE_ATTRIBUTE Geometric2d : Model
    {
        MODELHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_GeometricModel2d, GeometricModel2d, Geometric2d, Model, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for SpatialModel
    struct EXPORT_VTABLE_ATTRIBUTE Spatial : Model
    {
        MODELHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_SpatialModel, SpatialModel, Spatial, Model, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for ComponentModel
    struct EXPORT_VTABLE_ATTRIBUTE Component : Model
    {
        MODELHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_ComponentModel, ComponentModel, Component, Model, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for SectionDrawingModel
    struct EXPORT_VTABLE_ATTRIBUTE SectionDrawing : Geometric2d
    {
        MODELHANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_SectionDrawingModel, SectionDrawingModel, SectionDrawing, Geometric2d, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for SheetModel
    struct EXPORT_VTABLE_ATTRIBUTE Sheet : Geometric2d
    {
        MODELHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_SheetModel, SheetModel, Sheet, Geometric2d, DGNPLATFORM_EXPORT)
    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParamsR params) override;
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

    //! The ModelHandler for GroupInformationModel
    struct EXPORT_VTABLE_ATTRIBUTE GroupInformation : Model
    {
        MODELHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_GroupInformationModel, GroupInformationModel, GroupInformation, Model, DGNPLATFORM_EXPORT)
    };
};

END_BENTLEY_DGN_NAMESPACE
