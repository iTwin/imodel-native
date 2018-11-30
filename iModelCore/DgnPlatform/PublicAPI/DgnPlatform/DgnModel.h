/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnModel.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDomain.h"
#include "DgnElement.h"
#include "CodeSpec.h"
#include "ECSqlClassParams.h"
#include <Bentley/ValueFormat.h>
#include <DgnPlatform/DgnProperties.h>

DGNPLATFORM_TYPEDEFS(SectionDrawingModel)
DGNPLATFORM_TYPEDEFS(DictionaryModel)
DGNPLATFORM_REF_COUNTED_PTR(DictionaryModel)

BEGIN_BENTLEY_DGN_NAMESPACE

namespace RangeIndex {struct Tree;}
namespace dgn_ModelHandler {struct Definition; struct DocumentList; struct Drawing; struct Geometric2d; struct GroupInformation; struct Information; struct InformationRecord; struct Physical; struct Repository; struct Role; struct Spatial; struct SpatialLocation;}

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
    protected:  Utf8CP _GetHandlerECClassName() const override {return MyHandlerECClassName();}\
                Utf8CP _GetSuperHandlerECClassName() const override {return T_Super::_GetHandlerECClassName();}

/**
* @addtogroup GROUP_DgnModel DgnModel Module
* Types related to working with %DgnModels
* @see @ref PAGE_ModelOverview
* @see @ref PAGE_CustomModel
*/

//=======================================================================================
//! The "current entry" of an ModelIterator
// @bsiclass                                                     Shaun.Sewall      12/16
//=======================================================================================
struct ModelIteratorEntry : ECSqlStatementEntry
{
    friend struct ECSqlStatementIterator<ModelIteratorEntry>;
private:
    ModelIteratorEntry(BeSQLite::EC::ECSqlStatement* statement = nullptr) : ECSqlStatementEntry(statement) {}

public:
    DGNPLATFORM_EXPORT DgnModelId GetModelId() const;
    DGNPLATFORM_EXPORT DgnClassId GetClassId() const;
    DGNPLATFORM_EXPORT DgnModelId GetParentModelId() const;
    DGNPLATFORM_EXPORT DgnElementId GetModeledElementId() const;
    DGNPLATFORM_EXPORT bool IsTemplate() const;
    DGNPLATFORM_EXPORT bool IsPrivate() const;
};

//=======================================================================================
//! An iterator over a set of DgnModels, defined by a query.
// @bsiclass                                                     Shaun.Sewall      12/16
//=======================================================================================
struct ModelIterator : ECSqlStatementIterator<ModelIteratorEntry>
{
    //! Iterates all entries to build an unordered IdSet of DgnModelIds
    BeSQLite::IdSet<DgnModelId> BuildIdSet()
        {
        BeSQLite::IdSet<DgnModelId> idSet;
        for (ModelIteratorEntry entry : *this)
            idSet.insert(entry.GetModelId());

        return idSet;
        }

    //! Iterates all entries to return an ordered bvector of DgnModelIds
    bvector<DgnModelId> BuildIdList()
        {
        bvector<DgnModelId> idList;
        for (ModelIteratorEntry entry : *this)
            idList.push_back(entry.GetModelId());

        return idList;
        }

    //! Iterates all entries to populate an ordered bvector of DgnModelIds
    void BuildIdList(bvector<DgnModelId>& idList)
        {
        for (ModelIteratorEntry entry : *this)
            idList.push_back(entry.GetModelId());
        }
};

//=======================================================================================
//! A DgnModel represents a model in memory and may hold references to elements that belong to it.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                     KeithBentley    10/00
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnModel : RefCountedBase
{
    friend struct DgnModels;
    friend struct DgnElement;
    friend struct DgnElements;
    friend struct dgn_TxnTable::Model;
    friend struct dgn_ModelHandler::Model;

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

        //! Called when a DgnModel is about to be updated in the DgnDb.
        //! @param[in] model The model to which this AppData is attached
        virtual DgnDbStatus _OnUpdate(DgnModelCR model) {return DgnDbStatus::Success;}

        //! Called after a DgnModel was updated in the DgnDb.
        //! @param[in] model The model to which this AppData is attached
        //! @return DropMe::Yes to be removed from DgnModel
        virtual DropMe _OnUpdated(DgnModelCR model) {return DropMe::No;}

        //! Called before the DgnModel is deleted from the DgnDb.
        //! @param[in] model The model to which this AppData is attached
        virtual void _OnDelete(DgnModelR model) {}

        //! Called after the DgnModel was deleted from the DgnDb.
        //! @param[in] model The model to which this AppData is attached
        //! @return DropMe::Yes to be removed from DgnModel
        virtual DropMe _OnDeleted(DgnModelCR model) {return DropMe::Yes;}

        //! Called before the DgnModel is unloaded from memory.
        //! @param[in] model The model to which this AppData is attached
        virtual void _OnUnload(DgnModelR model) {}

        //! Called after the DgnModel is unloaded from memory.
        //! @param[in] model The model to which this AppData is attached
        virtual void _OnUnloaded(DgnModelR model) {}

        // ###TODO_IMODELCORE: range index callbacks:
        // virtual void _OnAddToRangeIndex(DgnModelCR model, DRange3dCR range, DgnElementId elementId) {}
        // virtual Dropme _OnProjectExtentsChanged(SpatialModelCR model, AxisAlignedBox3dCR newExtents) {}
    };

    using AppDataPtr = RefCountedPtr<AppData>;

    //=======================================================================================
    //! Parameters to create a new instances of a DgnModel.
    //! @ingroup GROUP_DgnModel
    //=======================================================================================
    struct CreateParams
    {
        DgnDbR m_dgndb;
        DgnClassId m_classId;
        DgnElementId  m_modeledElementId;
        DgnClassId m_modeledElementRelClassId;
        bool m_isPrivate;
        bool m_isTemplate = false;

        //! Parameters to create a new instance of a DgnModel.
        //! @param[in] dgndb The DgnDb for the new DgnModel
        //! @param[in] classId The DgnClassId for the new DgnModel.
        //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
        //! @param[in] isPrivate Optional parameter specifying that this model should @em not appear in lists shown to the user
        //! @param[in] isTemplate Optional parameter that indicates whether the new DgnModel is a template or not
        CreateParams(DgnDbR dgndb, DgnClassId classId, DgnElementId modeledElementId, bool isPrivate=false, bool isTemplate=false) :
            m_dgndb(dgndb), m_classId(classId), m_modeledElementId(modeledElementId), m_isPrivate(isPrivate), m_isTemplate(isTemplate)
            {
            }

        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, JsonValueCR val);

        void SetModeledElementId(DgnElementId modeledElementId) {m_modeledElementId = modeledElementId;} //!< Set the DgnElementId of the element that this DgnModel is describing/modeling.
        void SetModeledElementRelClassId(DgnClassId classId) {m_modeledElementRelClassId = classId;} //!< Set the DgnClassId of the relationship of the DgnModel to the modeled element
        void SetIsPrivate(bool isPrivate) {m_isPrivate = isPrivate;} //!< Specify that this model should @em not appear in lists shown to the user
        void SetIsTemplate(bool isTemplate) {m_isTemplate = isTemplate;} //!< Set whether the DgnModel is a template used to create instances

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
        static const uint64_t Reserved_1 = Clone << 1;      //!< Reserved for future use 
        static const uint64_t Reserved_2 = Reserved_1 << 1; //!< Reserved for future use 
        static const uint64_t Reserved_3 = Reserved_2 << 1; //!< Reserved for future use 
        static const uint64_t Reserved_4 = Reserved_3 << 1; //!< Reserved for future use 
        static const uint64_t Reserved_5 = Reserved_4 << 1; //!< Reserved for future use 
        static const uint64_t Reserved_6 = Reserved_5 << 1; //!< Reserved for future use 

        static const uint64_t NextAvailable = Reserved_6 << 1; //!< Subclasses can add new actions beginning with this value

        DGNPLATFORM_EXPORT static uint64_t Parse(Utf8CP name);
    };

    BE_JSON_NAME(id)
    BE_JSON_NAME(classFullName)
    BE_JSON_NAME(parentModel)
    BE_JSON_NAME(modeledElement)
    BE_JSON_NAME(jsonProperties)
    BE_JSON_NAME(UserProps)
    BE_JSON_NAME(isPrivate)
    BE_JSON_NAME(isTemplate)
    BE_JSON_NAME(name)

    BE_PROP_NAME(ECInstanceId)
    BE_PROP_NAME(ParentModel)
    BE_PROP_NAME(ModeledElement)
    BE_PROP_NAME(IsPrivate)
    BE_PROP_NAME(JsonProperties)
    BE_PROP_NAME(IsTemplate)

private:
    mutable bmap<AppData::Key const*, RefCountedPtr<AppData>, std::less<AppData::Key const*>, 8> m_appData;

    template<typename T> void CallAppData(T const& caller) const;
    template<typename T> void NotifyAppData(T const& notifier);
    DGNPLATFORM_EXPORT void AddAppDataInternal(AppData::Key const& key, AppData* data);
    DGNPLATFORM_EXPORT AppData* FindAppDataInternal(AppData::Key const& key) const;

    void UnloadRangeIndex();
    DgnDbStatus BindInsertAndUpdateParams(BeSQLite::EC::ECSqlStatement& statement);
    DgnDbStatus Read(DgnModelId modelId);
    ECN::AdHocJsonValueR GetUserPropsR() {return (ECN::AdHocJsonValueR) m_jsonProperties[json_UserProps()];}

protected:
    DgnDbR m_dgndb;
    DgnModelId m_modelId;
    DgnClassId m_classId;
    DgnModelId m_parentModelId;
    DgnElementId m_modeledElementId;
    DgnClassId m_modeledElementRelClassId;
    mutable BeMutex m_mutex;
    bool m_isPrivate;
    bool m_isTemplate;
    mutable bool m_persistent;   // true if this DgnModel is in the DgnModels "loaded models" list.
    ECN::AdHocJsonValue m_jsonProperties;

    explicit DGNPLATFORM_EXPORT DgnModel(CreateParams const&);
    DGNPLATFORM_EXPORT virtual ~DgnModel();
    DGNPLATFORM_EXPORT virtual void _Destroy();
    DGNPLATFORM_EXPORT virtual void _PreDestroy();

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

     //! argument for _BindWriteParams
    enum class ForInsert : bool {No=false, Yes=true};

    //! Called to bind the model's property values to the ECSqlStatement when inserting or updating
    //! a model.  The parameters to bind were the ones specified by this model's Handler.
    //! @note If you override this method, you should bind your subclass properties
    //! to the supplied ECSqlStatement, using statement.GetParameterIndex with your property's name.
    //! Then you @em must call T_Super::_BindWriteParams
    DGNPLATFORM_EXPORT virtual void _BindWriteParams(BeSQLite::EC::ECSqlStatement& statement, ForInsert forInsert);

    //! Convert this DgnModel to a Json::Value.
    //! @note If you override this method, you @em must call T_Super::_ToJson
    DGNPLATFORM_EXPORT virtual void _ToJson(JsonValueR out, JsonValueCR opts) const;

    //! Initialize this DgnModel from a Json::Value.
    //! @param props The properties. @note The input object's properties will be moved into and consumed by this model.
    //! @note If you override this method, you @em must call T_Super::_FromJson
    DGNPLATFORM_EXPORT virtual void _FromJson(JsonValueR props);

    //! Invoked on saving the JsonProperties field into the Db as part of an Insert or Update operation.
    //! @note If you override this method, you @em must call T_Super::_WriteJsonProperties.
    //! Put your properties into a sub node of m_jsonProperties to avoid collisions with the 
    //! super class properties.
    virtual void _OnSaveJsonProperties() {}

    //! Invoked after the DgnModel is loaded so that subclasses can read from the m_jsonProperties field, if necessary. 
    //! The properties will already have been loaded into the m_jsonProperties member.
    //! @note If you override this method, you @em must call T_Super::_OnLoadedJsonProperties.
    virtual void _OnLoadedJsonProperties() {}

    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetProperty(Utf8CP name, ECN::ECValueCR value);

    //! Set the properties of this model from the specified instance. Calls _SetProperty for each non-NULL property in the input instance.
    //! @return non-zero error status if any property could not be set. Note that some properties might be set while others are not in case of error.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetProperties(ECN::IECInstanceCR);

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
    virtual void _OnLoadedElement(DgnElementCR element) {}

    //! Called after a DgnElement in this DgnModel has been inserted into the DgnDb
    //! @param[in] element The element that was just inserted.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    virtual void _OnInsertedElement(DgnElementCR element) {}

    //! Called after a change representing addition of a DgnElement (belonging to this DgnModel) was applied to the DgnDb.
    //! @param[in] element The element that was just added.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    virtual void _OnAppliedAddElement(DgnElementCR element) {}

    //! Called after a change representing update of a DgnElement (belonging to this DgnModel) was applied to the DgnDb.
    //! @param[in] modified The element in its changed state. This state was saved to the DgnDb
    //! @param[in] original The element in its pre-changed state.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    virtual void _OnUpdatedElement(DgnElementCR modified, DgnElementCR original) {}

    //! Called after a change representing DgnElement in this model was updated by applying a change set.
    //! @param[in] original The element in its original state. This is the state before the original change (the current state)
    //! @param[in] modified The element in its post-changed state.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    virtual void _OnAppliedUpdateElement(DgnElementCR original, DgnElementCR modified) {}

    //! Called after a DgnElement in this DgnModel has been deleted from the DgnDb
    //! @param[in] element The element that was just deleted.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    virtual void _OnDeletedElement(DgnElementCR element) {}

    //! Called after a change representing deletion of a DgnElement (belonging to this DgnModel) was applied to the DgnDb.
    //! @param[in] element The element that was just deleted by applying a change set.
    //! @note If you override this method, you @em must call the T_Super implementation.
    //! DgnModels maintain an id->element lookup table, and possibly a DgnRangeTree. The DgnModel implementation of this method maintains them.
    virtual void _OnAppliedDeleteElement(DgnElementCR element) {}

    /** @} */

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
    //! @param viewController   The view for which the copyright message is requested.
    //! @return a copyright message or nullptr
    virtual Utf8String _GetCopyrightMessage(ViewController& viewController) const {return "";}

    //! Return the copyright image to display if this model is drawn to a viewport,
    //! @param viewController   The view for which the copyright message is requested.
    //! @return a copyright message or nullptr
    virtual Render::RgbaSpriteP _GetCopyrightSprite (ViewController& viewController) const { return nullptr;}

    /** @name Dynamic cast shortcuts for a DgnModel */
    /** @{ */
    virtual GeometricModelCP _ToGeometricModel() const {return nullptr;}
    virtual RoleModelCP _ToRoleModel() const {return nullptr;}
    virtual InformationModelCP _ToInformationModel() const {return nullptr;}
    virtual InformationRecordModelCP _ToInformationRecordModel() const {return nullptr;}
    virtual DefinitionModelCP _ToDefinitionModel() const {return nullptr;}
    virtual GeometricModel2dCP _ToGeometricModel2d() const {return nullptr;}
    virtual GeometricModel3dCP _ToGeometricModel3d() const {return nullptr;}
    virtual SpatialModelCP _ToSpatialModel() const {return nullptr;}
    virtual SpatialLocationModelCP _ToSpatialLocationModel() const {return nullptr;}
    virtual PhysicalModelCP _ToPhysicalModel() const {return nullptr;}
    virtual DrawingModelCP _ToDrawingModel() const {return nullptr;}
    virtual SectionDrawingModelCP _ToSectionDrawingModel() const {return nullptr;}
    virtual Sheet::ModelCP _ToSheetModel() const {return nullptr;}
    /** @} */

    //! The sublcass should import elements from the source model into this model. 
    //! Import is done in phases. The import framework will call _ImportElementAspectsFrom and then _ImportLinkTableECRelationshipsFrom after calling this method.
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
    //! A subclass implementation of _ImportLinkTableECRelationshipsFrom should copy only the non-navigation relationship subclasses that are defined by the 
    //! the ECSchema/DgnDomain of the subclass. For example, the base DgnModel implementation will handle the relationships defined in the 
    //! base bis schema, including ElementDrivesElement and ElementGroupsMembers.
    //! <p>
    //! Both endpoints of an ECRelationship must be in the same DgnDb. Since the import operation can copy elements between DgnDbs, a subclass implementation
    //! must be careful about which ECRelationships to import. Normally, only ECRelationships between elements in the model should be copied. 
    //! ECRelationships that start/end outside the model can only be copied if the foreign endpoint is also copied. 
    //! If endpoint elements must be deep-copied, however, that must be done in _ImportElementsFrom, not in this function. That is because
    //! deep-copying an element in the general case requires all of the support for copying and remapping of parents and aspects that is implemented by the framework,
    //! prior to the phase where ECRelationships are copied.
    //! @note The implementation should start by calling the superclass implementation.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ImportLinkTableECRelationshipsFrom(DgnModelCR sourceModel, DgnImportContext& importer);

    //! Utility function to import non-Navigation ECRelationships from one DgnDb to another, selecting only the relationship instances whose source and target elements are
    //! in the specified source model and only if both source and target have already been imported and are registered in the importContext's remap tables. The source and 
    //! target DgnElementIds of each imported relationship instance are remapped to the destination DgnDb using the importContext's remap tables.
    //! @param destDb   The destination DgnDb
    //! @param sourceModel  The model that is being imported. Only ECRelationship instances with both source and target elements from this model are imported. This parameter
    //!                     also identifies the source DgnDb
    //! @param importContext The context that contains remapping tables. 
    //! @param relschema    The schema of the relationship class
    //! @param relname      The name of the relationship class
    //! @return non-zero error status if the relationship class does not exist in the source or target DgnDb. Note that this function will return success even if no 
    //! relationship instances are imported.
    DGNPLATFORM_EXPORT static DgnDbStatus ImportLinkTableECRelationshipsFrom(DgnDbR destDb, DgnModelCR sourceModel, DgnImportContext& importContext, Utf8CP relschema, Utf8CP relname);

    //! Disclose any locks which must be acquired and/or codes which must be reserved in order to perform the specified operation on this model.
    //! @param[in]      request  Request to populate
    //! @param[in]      opcode   The operation to be performed
    //! @return RepositoryStatus::Success, or an error code if for example a required lock or code is known to be unavailable without querying the repository manager.
    //! @note If you override this function you @b must call T_Super::_PopulateRequest(), forwarding its status.
    DGNPLATFORM_EXPORT virtual RepositoryStatus _PopulateRequest(IBriefcaseManager::Request& request, BeSQLite::DbOpcode opcode) const;

    //! Generate the CreateParams to use for _CloneForImport
    //! @param[in] importer Specifies source and destination DgnDbs and knows how to remap IDs
    //! @param[in] destinationElementToModel After import, the copy of the source model will model this element
    //! @return CreateParams initialized with the model's current data, remapped to the destination DgnDb.
    DGNPLATFORM_EXPORT CreateParams GetCreateParamsForImport(DgnImportContext& importer, DgnElementCR destinationElementToModel) const;

    static CreateParams InitCreateParamsFromECInstance(DgnDbStatus*, DgnDbR db, ECN::IECInstanceCR);

    virtual void _OnValidate() {}

public:
    Utf8String GetCopyrightMessage(ViewController& viewController) const {return _GetCopyrightMessage(viewController);}

    Render::RgbaSpriteP GetCopyrightSprite (ViewController& viewController) const { return _GetCopyrightSprite(viewController);}

    virtual Utf8CP _GetHandlerECClassName() const {return BIS_CLASS_Model;} //!< @private
    virtual Utf8CP _GetSuperHandlerECClassName() const {return nullptr;}    //!< @private

    DGNPLATFORM_EXPORT ModelHandlerR GetModelHandler() const;

    //! Returns true if this is a 3d model.
    bool Is3d() const {return nullptr != ToGeometricModel3d();}

    //! Determine whether this DgnModel is persistent.
    //! A model is "persistent" if it was loaded via DgnModels::GetModel, or after it is inserted into the DgnDb via Insert.
    //! A newly created model before it is inserted, or a model after calling Delete, is not persistent.
    bool IsPersistent() const {return m_persistent;}

    //! Get the name of this model
    DGNPLATFORM_EXPORT Utf8String GetName() const;
    
    //! Get the DgnClassId of this DgnModel
    DgnClassId GetClassId() const {return m_classId;}

    //! Get the DgnModelId of this DgnModel
    DgnModelId GetModelId() const {return m_modelId;}

    //! Get the DgnModelId of the DgnModel above this one in the information hierarchy
    DgnModelId GetParentModelId() const {return m_parentModelId;}

    //! Get the DgnElement that this DgnModel is describing/modeling
    DGNPLATFORM_EXPORT DgnElementCPtr GetModeledElement() const;

    //! Get the DgnElementId of the element that this DgnModel is describing/modeling.
    DgnElementId GetModeledElementId() const {return m_modeledElementId;}

    //! Get the ECClassId of the ECRelationship that specifies how this DgnModel relates to the modeled element.
    DgnClassId GetModeledElementRelClassId() const {return m_modeledElementRelClassId;}

    //! Test if this model should @em not appear in lists shown to the user
    bool IsPrivate() const {return m_isPrivate;}

    //!< Specify that this model should @em not appear in lists shown to the user
    void SetIsPrivate(bool val) {m_isPrivate = val;}

    //! Test whether this DgnModel is a template used to create instances
    bool IsTemplate() const {return m_isTemplate;}

    //! @private
    void SetIsTemplate(bool b) {m_isTemplate = b;}

    //! @name Dynamic casting to DgnModel subclasses
    //@{
    GeometricModelCP ToGeometricModel() const {return _ToGeometricModel();} //!< more efficient substitute for dynamic_cast<GeometricModelCP>(model)
    RoleModelCP ToRoleModel() const {return _ToRoleModel();} //!< more efficient substitute for dynamic_cast<RoleModelCP>(model)
    InformationModelCP ToInformationModel() const {return _ToInformationModel();} //!< more efficient substitute for dynamic_cast<InformationModelCP>(model)
    InformationRecordModelCP ToInformationRecordModel() const {return _ToInformationRecordModel();} //!< more efficient substitute for dynamic_cast<InformationRecordModelCP>(model)
    DefinitionModelCP ToDefinitionModel() const {return _ToDefinitionModel();} //!< more efficient substitute for dynamic_cast<DefinitionModelCP>(model)
    GeometricModel2dCP ToGeometricModel2d() const {return _ToGeometricModel2d();} //!< more efficient substitute for dynamic_cast<GeometricModel2dCP>(model)
    GeometricModel3dCP ToGeometricModel3d() const {return _ToGeometricModel3d();} //!< more efficient substitute for dynamic_cast<GeometricModel3dCP>(model)
    SpatialModelCP ToSpatialModel() const {return _ToSpatialModel();} //!< more efficient substitute for dynamic_cast<SpatialModelCP>(model)
    SpatialLocationModelCP ToSpatialLocationModel() const {return _ToSpatialLocationModel();} //!< more efficient substitute for dynamic_cast<SpatialLocationModelCP>(model)
    PhysicalModelCP ToPhysicalModel() const {return _ToPhysicalModel();} //!< more efficient substitute for dynamic_cast<PhysicalModelCP>(model)
    DrawingModelCP ToDrawingModel() const {return _ToDrawingModel();} //!< more efficient substitute for dynamic_cast<DrawingModelCP>(model)
    SectionDrawingModelCP ToSectionDrawingModel() const {return _ToSectionDrawingModel();} //!< more efficient substitute for dynamic_cast<SectionDrawingModelCP>(model)
    Sheet::ModelCP ToSheetModel() const {return _ToSheetModel();} //!< more efficient substitute for dynamic_cast<SheetModelCP>(model)
    GeometricModelP ToGeometricModelP() {return const_cast<GeometricModelP>(_ToGeometricModel());} //!< more efficient substitute for dynamic_cast<GeometricModelP>(model)
    RoleModelP ToRoleModelP() {return const_cast<RoleModelP>(_ToRoleModel());} //!< more efficient substitute for dynamic_cast<RoleModelP>(model)
    InformationModelP ToInformationModelP() {return const_cast<InformationModelP>(_ToInformationModel());} //!< more efficient substitute for dynamic_cast<InformationModelP>(model)
    InformationRecordModelP ToInformationRecordModelP() {return const_cast<InformationRecordModelP>(_ToInformationRecordModel());} //!< more efficient substitute for dynamic_cast<InformationRecordModelP>(model)
    DefinitionModelP ToDefinitionModelP() {return const_cast<DefinitionModelP>(_ToDefinitionModel());} //!< more efficient substitute for dynamic_cast<DefinitionModelP>(model)
    GeometricModel2dP ToGeometricModel2dP() {return const_cast<GeometricModel2dP>(_ToGeometricModel2d());} //!< more efficient substitute for dynamic_cast<GeometricModel2dP>(model)
    GeometricModel3dP ToGeometricModel3dP() {return const_cast<GeometricModel3dP>(_ToGeometricModel3d());} //!< more efficient substitute for dynamic_cast<GeometricModel3dP>(model)
    SpatialModelP ToSpatialModelP() {return const_cast<SpatialModelP>(_ToSpatialModel());} //!< more efficient substitute for dynamic_cast<SpatialModelP>(model)
    SpatialLocationModelP ToSpatialLocationModelP() {return const_cast<SpatialLocationModelP>(_ToSpatialLocationModel());} //!< more efficient substitute for dynamic_cast<SpatialLocationModelP>(model)
    PhysicalModelP ToPhysicalModelP() {return const_cast<PhysicalModelP>(_ToPhysicalModel());} //!< more efficient substitute for dynamic_cast<PhysicalModelP>(model)
    DrawingModelP ToDrawingModelP() {return const_cast<DrawingModelP>(_ToDrawingModel());} //!< more efficient substitute for dynamic_cast<DrawingModelP>(model)
    SectionDrawingModelP ToSectionDrawingModelP() {return const_cast<SectionDrawingModelP>(_ToSectionDrawingModel());} //!< more efficient substitute for dynamic_cast<SectionDrawingModelP>(model)
    Sheet::ModelP ToSheetModelP() {return const_cast<Sheet::ModelP>(_ToSheetModel());}//!< more efficient substitute for dynamic_cast<SheetModelP>(model)

    bool IsGeometricModel() const {return nullptr != ToGeometricModel();}
    bool IsSpatialModel() const {return nullptr != ToSpatialModel();}
    bool IsSpatialLocationModel() const {return nullptr != ToSpatialLocationModel();}
    bool IsPhysicalModel() const {return nullptr != ToPhysicalModel();}
    bool Is2dModel() const {return nullptr != ToGeometricModel2d();}
    bool Is3dModel() const {return nullptr != ToGeometricModel3d();}
    bool IsRoleModel() const {return nullptr != ToRoleModel();}
    bool IsInformationModel() const {return nullptr != ToInformationModel();}
    bool IsInformationRecordModel() const {return nullptr != ToInformationRecordModel();}
    bool IsDefinitionModel() const {return nullptr != ToDefinitionModel();}
    bool IsSheetModel() const {return nullptr != ToSheetModel();}
    bool IsDrawingModel() const {return nullptr != ToDrawingModel();}
    bool IsDictionaryModel() const {return DictionaryId() == GetModelId();}
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

    //! Update the Properties of this model in the DgnDb
    //! @return DgnDbStatus::Success if the properties of this model were successfully updated, error otherwise.
    DGNPLATFORM_EXPORT DgnDbStatus Update();

    /** @name DgnModel AppData */
    /** @{ */
    //! Add (or replace) AppData on this DgnModel.
    //! @note It is illegal to add or remove AppData from within
    //! any of the AppData "_OnXxx" methods. If an entry with \a key already exists, it will be dropped and replaced with \a appData.
    void AddAppData(AppData::Key const& key, AppData* appData) { BeMutexHolder lock(m_mutex); return AddAppDataInternal(key, appData); }

    //! Find or add ApPData on this DgnModel.
    //! If AppData with the specified key already exists, it is returned.
    //! Otherwise the supplied function is invoked to create new AppData which will be added and returned.
    //! @param[in] key The key identifying the type of AppData.
    //! @params[in] createAppData a callable taking no arguments and returning an AppData*, invoked if no AppData corresponding to the supplied key currently exists on the model.
    //! @return the AppData corresponding to the supplied key.
    template<typename T> AppDataPtr FindOrAddAppData(AppData::Key const& key, T createAppData)
        {
        BeMutexHolder lock(m_mutex);
        AppData* data = FindAppDataInternal(key);
        if (nullptr == data)
            AddAppDataInternal(key, data = createAppData());

        return data;
        }

    //! Find or add ApPData on this DgnModel.
    //! If AppData with the specified key already exists, it is returned.
    //! Otherwise the supplied function is invoked to create new AppData which will be added and returned.
    //! @param[in] key The key identifying the type of AppData.
    //! @params[in] createAppData a callable taking no arguments and returning an AppData*, invoked if no AppData corresponding to the supplied key currently exists on the model.
    //! @return the AppData corresponding to the supplied key as a ref-counted pointer to the derived type.
    template<typename T> auto ObtainAppData(AppData::Key const& key, T createAppData) -> RefCountedPtr<typename std::remove_pointer<decltype(createAppData())>::type>
        {
        using U = decltype(createAppData());
        AppDataPtr data = FindOrAddAppData(key, createAppData);
        return static_cast<U>(data.get());
        }

    //! Remove AppData from this DgnModel
    //! @return SUCCESS if appData with key is found and was dropped.
    //! @remarks Calls the object's _OnCleanup method.
    DGNPLATFORM_EXPORT StatusInt DropAppData(AppData::Key const& key);

    //! Search for AppData on this model by AppData::Key.
    //! @return the AppData with \a key, or nullptr.
    AppDataPtr FindAppData(AppData::Key const& key) const { BeMutexHolder lock(m_mutex); return FindAppDataInternal(key); }
    /** @} */

    //! Make a copy of this DgnModel with the same DgnClassId and Properties.
    //! @param[in] newModeledElementId The DgnElementId of the element for the new DgnModel to model.
    //! @note This makes a new empty, non-persistent, DgnModel with the same properties as this DgnModel, it does NOT clone the elements of this DgnModel.
    //! @see CopyModel, Import
    DGNPLATFORM_EXPORT DgnModelPtr Clone(DgnElementId newModeledElementId) const;

    //! Make a persitent copy of the specified DgnModel and its contents.
    //! @param[in] model The model to copy
    //! @param[in] newModeledElementId The DgnElementId of the element for the new DgnModel to model.
    //! @see Import
    DGNPLATFORM_EXPORT static DgnModelPtr CopyModel(DgnModelCR model, DgnElementId newModeledElementId);

    //! Make a duplicate of this DgnModel object in memory. Do not copy its elements. @see ImportModel
    //! It's not normally necessary for a DgnModel subclass to override _Clone. The base class implementation will 
    //! invoke the subclass handler to create an instance of the subclass. The base class implementation will also
    //! cause the new model object to read its properties from this (source) model's properties. That will 
    //! take of populating most if not all subclass members.
    //! @return the copy of the model
    //! @param[out] stat Optional. If not null, then an error code is stored here in case the clone fails.
    //! @param[in] importer Used by elements when copying between DgnDbs.
    //! @param[in] destinationElementToModel after import, the copy of the source model will model this element
    //! @see GetCreateParamsForImport
    DGNPLATFORM_EXPORT DgnModelPtr virtual _CloneForImport(DgnDbStatus* stat, DgnImportContext& importer, DgnElementCR destinationElementToModel) const;

    //! Copy the contents of \a sourceModel into this model. Note that this model might be in a different DgnDb from \a sourceModel.
    //! This base class implemenation calls the following methods, in order:
    //!     -# _ImportElementsFrom
    //!     -# _ImportElementAspectsFrom
    //!     -# _ImportLinkTableECRelationshipsFrom
    //! @param[in] sourceModel The model to copy
    //! @param[in] importer Used by elements when copying between DgnDbs.
    //! @return non-zero if the copy failed
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ImportContentsFrom(DgnModelCR sourceModel, DgnImportContext& importer);

    //! Make a copy of the specified model, including all of the contents of the model, where the destination may be a different DgnDb.
    //! This is just a convenience method that calls the follow methods, in order:
    //!     -# _CloneForImport
    //!     -# Insert
    //!     -# _ImportContentsFrom
    //! @param[out] stat  Optional status to describe failures, a valid DgnModelPtr will only be returned if successful.
    //! @param[in] sourceModel The model to copy
    //! @param[in] importer Enables the model to copy the definitions that it needs (if copying between DgnDbs)
    //! @param[in] destinationElementToModel after import, the copy of the sourceModel will model this element
    //! @return the copied model, already inserted into the destination Db.
    DGNPLATFORM_EXPORT static DgnModelPtr ImportModel(DgnDbStatus* stat, DgnModelCR sourceModel, DgnImportContext& importer, DgnElementCR destinationElementToModel);

    //! Make a copy of the specified model, including all of the contents of the model, where the destination may be a different DgnDb.
    //! @param[out] stat Optional status to describe failures, a valid DgnModelPtr will only be returned if successful.
    //! @param[in] sourceModel The model to copy
    //! @param[in] importer Enables the model to copy the definitions that it needs (if copying between DgnDbs)
    //! @param[in] destinationElementToModel after import, the copy of the sourceModel will model this element
    //! @return the copied model
    //! @see ImportModel
    template<typename T> static RefCountedPtr<T> Import(DgnDbStatus* stat, T const& sourceModel, DgnImportContext& importer, DgnElementCR destinationElementToModel) {return dynamic_cast<T*>(ImportModel(stat, sourceModel, importer, destinationElementToModel).get());}

    //! Returns the DgnModelId used by the RepositoryModel associated with each DgnDb
    static DgnModelId RepositoryModelId() {return DgnModelId((uint64_t)1LL);}

//__PUBLISH_SECTION_END__
    //-------------------------------------------------------------------------------------
    // NOTE: Setting DictionaryId to 16 effectively reserves the IDs below it. 
    // NOTE: New auto-created BisCore models must be below 16 to prevent ID remapping issues during file format upgrades.
    // NOTE: Applications/domains should look up models by code and not hard-code IDs
    //-------------------------------------------------------------------------------------
//__PUBLISH_SECTION_START__
    //! Returns the ID used by the unique dictionary model associated with each DgnDb
    static DgnModelId DictionaryId() {return DgnModelId((uint64_t)16LL);}

    //! This method is called when it is time to validate changes that have been made to the model's content during the transaction.
    //! This method is called by the transaction manager after all element-level changes have been validated and all root models have been solved.
    //! This method is called only if elements in this model were added, deleted, or modified or if this model object itself was added or modified.
    //! This method allows a subclass to apply validation logic that requires a view of the entire model and possibly of root models.
    //! This method may add, delete, or modify elements in this model.
    //! To indication a validation error, call TxnManager::ReportError. If the error is marked as fatal, then the transaction will be rolled back.
    //! @note This method must make changes of any kind to any other model. Dependent models will be validated later.
    void OnValidate() {_OnValidate();}

    //! Disclose any locks which must be acquired and/or codes which must be reserved in order to perform the specified operation on this model.
    //! @param[in] request Request to populate
    //! @param[in] opcode The operation to be performed
    //! @return RepositoryStatus::Success, or an error code if for example a required lock or code is known to be unavailable without querying the repository manager.
    //! @note If you override this function you @b must call T_Super::_PopulateRequest(), forwarding its status.
    RepositoryStatus PopulateRequest(IBriefcaseManager::Request& request, BeSQLite::DbOpcode opcode) const {return _PopulateRequest(request, opcode);}

    //! Make an iterator over the elements in this DgnModel
    //! @param[in] whereClause The optional where clause starting with WHERE (note, ModelId is already specified.)
    //! @param[in] orderByClause The optional order by clause starting with ORDER BY
    DGNPLATFORM_EXPORT ElementIterator MakeIterator(Utf8CP whereClause=nullptr, Utf8CP orderByClause=nullptr) const;

    //! @name JsonProperties 
    //! @{
    //! Get the current value of a set of Json Properties on this element
    ECN::AdHocJsonValueCR GetJsonProperties(Utf8CP nameSpace) const {return m_jsonProperties.GetMember(nameSpace);}

    //! Change the value of a set of Json Properties on this element
    void SetJsonProperties(Json::StaticString nameSpace, JsonValueCR value) {m_jsonProperties.GetMemberR(nameSpace) = (ECN::AdHocJsonValueCR) value;}

    //! Remove a set of Json Properties on this element
    void RemoveJsonProperties(Utf8CP nameSpace) {m_jsonProperties.RemoveMember(nameSpace);}

    ECN::AdHocJsonValueCR GetUserProperties(Utf8CP nameSpace) const {return GetJsonProperties(json_UserProps()).GetMember(nameSpace);}

    void SetUserProperties(Utf8CP nameSpace, JsonValueCR value) {GetUserPropsR().GetMemberR(nameSpace) = (ECN::AdHocJsonValueCR) value;}

    void RemoveUserProperties(Utf8CP nameSpace) {GetUserPropsR().RemoveMember(nameSpace);}
    /** @} */

    //! Set this model's properties from JavaScript.
    //! @param opts The properties. @note The input object's properties will be moved into and consumed by this model.
    void FromJson(JsonValueR opts) {return _FromJson(opts);}

    Json::Value ToJson(JsonValueCR opts) const {Json::Value val; _ToJson(val, opts); return val;}
}; // DgnModel

//=======================================================================================
//! A DgnModel that contains geometric DgnElements.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Keith.Bentley   03/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometricModel : DgnModel
{
    DEFINE_T_SUPER(DgnModel);
    friend struct SpatialViewController;

public:
    //=======================================================================================
    //! The DisplayInfo for a DgnModel. These are stored within a "DisplayInfo"
    //! node of the JSON value that's serialized as a string in "Properties" column of the DgnModel table.
    //! @ingroup GROUP_DgnModel
    //=======================================================================================
    struct Formatter
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

            BE_JSON_NAME(linMode)
            BE_JSON_NAME(linType)
            BE_JSON_NAME(linPrec)
            BE_JSON_NAME(angMode)
            BE_JSON_NAME(angPrec)
            BE_JSON_NAME(dirMode)
            BE_JSON_NAME(clockwise)

            void FromJson(JsonValueCR);
            Json::Value ToJson() const;
        };

        FormatterFlags m_formatterFlags; //!< format flags
        UnitDefinition m_masterUnit;  //!< Master Unit information
        UnitDefinition m_subUnit; //!< Sub Unit information
        double m_roundoffUnit; //!< unit lock roundoff val
        double m_roundoffRatio; //!< Unit roundoff ratio y to x (if 0 use Grid Ratio)
        double m_formatterBaseDir; //!< Base Direction used for Direction To/From String

    public:
        Formatter()
            {
            m_formatterFlags.m_linearUnitMode = (uint32_t)DgnUnitFormat::MU;
            m_formatterFlags.m_linearPrecType = (uint32_t)PrecisionType::Decimal;
            m_formatterFlags.m_linearPrecision = (uint32_t)PrecisionFormat::Decimal8Places;
            m_formatterFlags.m_angularMode = (uint32_t)AngleMode::Degrees;
            m_formatterFlags.m_angularPrecision = (uint32_t)AnglePrecision::Use8Places;
            m_formatterFlags.m_directionMode = (uint32_t)DirectionMode::Azimuth;
            m_formatterFlags.m_directionClockwise = 0;
            m_roundoffRatio = 0;
            m_formatterBaseDir = 0;
            m_roundoffUnit = 0;
            m_masterUnit = UnitDefinition::GetStandardUnit(StandardUnit::MetricMeters);
            m_subUnit = UnitDefinition::GetStandardUnit(StandardUnit::MetricMillimeters);
            }

        BE_JSON_NAME(fmtFlags)
        BE_JSON_NAME(mastUnit)
        BE_JSON_NAME(subUnit)
        BE_JSON_NAME(rndUnit)
        BE_JSON_NAME(rndRatio)
        BE_JSON_NAME(fmtDir)

        void FromJson(JsonValueCR);
        Json::Value ToJson() const;

        //! Set master units and sub-units. Units must be valid and comparable.
        DGNPLATFORM_EXPORT BentleyStatus SetUnits(UnitDefinitionCR newMasterUnit, UnitDefinitionCR newSubUnit);
        void SetLinearUnitMode(DgnUnitFormat value) {m_formatterFlags.m_linearUnitMode = (uint32_t) value;}
        void SetLinearPrecision(PrecisionFormat value)
            {
            m_formatterFlags.m_linearPrecType = static_cast<uint32_t>(DoubleFormatter::GetTypeFromPrecision(value));
            m_formatterFlags.m_linearPrecision = DoubleFormatter::GetByteFromPrecision(value);
            }
        void SetAngularMode(AngleMode value) {m_formatterFlags.m_angularMode = (uint32_t) value;}
        void SetAngularPrecision(AnglePrecision value) {m_formatterFlags.m_angularPrecision = (uint32_t) value;}
        void SetDirectionMode(DirectionMode value) {m_formatterFlags.m_directionMode = (uint32_t) value;}
        void SetDirectionClockwise(bool value) {m_formatterFlags.m_directionClockwise = value;}
        void SetDirectionBaseDir(double value) {m_formatterBaseDir = value;}
        DgnUnitFormat GetLinearUnitMode() const {return (DgnUnitFormat) m_formatterFlags.m_linearUnitMode;}
        PrecisionFormat GetLinearPrecision() const {return DoubleFormatter::ToPrecisionEnum((PrecisionType) m_formatterFlags.m_linearPrecType, m_formatterFlags.m_linearPrecision);}
        AngleMode GetAngularMode() const {return (AngleMode) m_formatterFlags.m_angularMode;}
        AnglePrecision GetAngularPrecision() const {return (AnglePrecision) m_formatterFlags.m_angularPrecision;}
        DirectionMode GetDirectionMode() const {return (DirectionMode) m_formatterFlags.m_directionMode;}
        bool GetDirectionClockwise() const {return m_formatterFlags.m_directionClockwise;}
        double GetDirectionBaseDir() const {return m_formatterBaseDir;}
        void SetRoundoffUnit(double roundoffUnit, double roundoffRatio) {m_roundoffUnit = roundoffUnit; m_roundoffRatio = roundoffRatio;}
        double GetRoundoffUnit() const {return m_roundoffUnit;}
        double GetRoundoffRatio() const {return m_roundoffRatio;}
        FormatterFlags GetFormatterFlags() const {return m_formatterFlags;}

        //! Get the Master units for this DgnModel.
        //! Master units are the major display units for coordinates in this DgnModel (e.g. "Meters", or "Feet").
        //! @see SetUnits, GetSubUnits
        //! @note Coordinate values are always stored as distance from the BIM global origin in Meters. Master Units may be used to display values in some other unit system.
        UnitDefinitionCR GetMasterUnits() const {return m_masterUnit;}

        //! Get the Sub-units for this DgnModel.
        //! Sub units are the minor readout units for coordinates in this DgnModel (e.g. "Centimeters, or "Inches").
        //! @see SetUnits, GetMasterUnits
        UnitDefinitionCR GetSubUnits() const {return m_subUnit;}

        //! Get the number of Meters per Master unit.
        //! @see GetMasterUnits
        double GetMetersPerMaster() const {return m_masterUnit.ToMeters();}

        //! Get the number of Sub units per Master unit.
        //! @see GetSubUnits
        double GetSubPerMaster() const {return m_subUnit.GetConversionFactorFrom(m_masterUnit);}
    };

    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(GeometricModel::T_Super::CreateParams);
        Formatter m_displayInfo;

        //! Parameters to create a new instance of a GeometricModel.
        //! @param[in] dgndb The DgnDb for the new DgnModel
        //! @param[in] classId The DgnClassId for the new DgnModel.
        //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
        //! @param[in] displayInfo The DisplayInfo for the new DgnModel.
        //! @param[in] isPrivate Optional parameter specifying that this model should @em not appear in lists shown to the user
        CreateParams(DgnDbR dgndb, DgnClassId classId, DgnElementId modeledElementId, Formatter displayInfo = Formatter(), bool isPrivate = false)
            : T_Super(dgndb, classId, modeledElementId, isPrivate), m_displayInfo(displayInfo)
            {}

        //! @private
        //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
        CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}

        Formatter const& GetFormatter() const {return m_displayInfo;} //!< Get the Formatter
        void SetFormatter(Formatter const& displayInfo) {m_displayInfo = displayInfo;} //!< Set the Formatter
    };

protected:
    mutable std::unique_ptr<RangeIndex::Tree> m_rangeIndex;
    Formatter m_displayInfo;
    BeAtomic<uint64_t> m_lastModifiedTime;

    DGNPLATFORM_EXPORT void AddToRangeIndex(DgnElementCR);
    DGNPLATFORM_EXPORT void RemoveFromRangeIndex(DgnElementCR);
    DGNPLATFORM_EXPORT void UpdateRangeIndex(DgnElementCR modified, DgnElementCR original);

    DGNPLATFORM_EXPORT void UpdateLastElementModifiedTime();

    virtual DgnDbStatus _FillRangeIndex() = 0;//!< @private
    DGNPLATFORM_EXPORT virtual AxisAlignedBox3d _QueryElementsRange() const;//!< @private
    virtual AxisAlignedBox3d _QueryNonElementModelRange() const { return AxisAlignedBox3d(DRange3d::NullRange()); }

    void _OnInsertedElement(DgnElementCR element) override {T_Super::_OnInsertedElement(element); AddToRangeIndex(element); UpdateLastElementModifiedTime();}
    void _OnAppliedAddElement(DgnElementCR element) override {T_Super::_OnAppliedAddElement(element); AddToRangeIndex(element); UpdateLastElementModifiedTime();}
    void _OnDeletedElement(DgnElementCR element) override {RemoveFromRangeIndex(element); T_Super::_OnDeletedElement(element); UpdateLastElementModifiedTime();}
    void _OnAppliedDeleteElement(DgnElementCR element) override {RemoveFromRangeIndex(element); T_Super::_OnAppliedDeleteElement(element); UpdateLastElementModifiedTime();}
    void _OnUpdatedElement(DgnElementCR modified, DgnElementCR original) override {UpdateRangeIndex(modified, original); T_Super::_OnUpdatedElement(modified, original); UpdateLastElementModifiedTime();}
    void _OnAppliedUpdateElement(DgnElementCR modified, DgnElementCR original) override {UpdateRangeIndex(modified, original); T_Super::_OnAppliedUpdateElement(modified, original); UpdateLastElementModifiedTime();}
    DGNPLATFORM_EXPORT void _OnSaveJsonProperties() override;
    DGNPLATFORM_EXPORT void _OnLoadedJsonProperties() override;
    GeometricModelCP _ToGeometricModel() const override final {return this;}
    
    explicit GeometricModel(CreateParams const& params) : T_Super(params), m_rangeIndex(nullptr), m_displayInfo(params.m_displayInfo) {}

    
public:
    BE_JSON_NAME(formatter)

    DgnDbStatus FillRangeIndex() {return _FillRangeIndex();}
    void RemoveRangeIndex() {BeMutexHolder lock(m_mutex); m_rangeIndex.reset();}

    RangeIndex::Tree* GetRangeIndex() const {return m_rangeIndex.get();}

    //! Get the AxisAlignedBox3d of the geometric elements contained within this model.
    AxisAlignedBox3d QueryElementsRange() const {return _QueryElementsRange();}

    //! Get the AxisAlignedBox3d of the non element contents of this model.
    AxisAlignedBox3d QueryNonElementModelRange() const {return _QueryNonElementModelRange();}

    //! Get a writable reference to the Formatter for this model.
    Formatter& GetFormatterR() {return m_displayInfo;}

    //! Get the Formatter for this model.
    Formatter const& GetFormatter() const {return m_displayInfo;}

    //! Returns the time of the most recent modification to any element in this model, in unix milliseconds.
    DGNPLATFORM_EXPORT uint64_t GetLastElementModifiedTime() const;
    void InitLastElementModifiedTime(); //!< @private

    //! If this model supports producing Cesium 3D tiles, return a root of such a tile tree. The default implementation returns nullptr.
    //! The rootTileOutput is supplied for loading the root tile. It should not be stored and reused.
    DGNPLATFORM_EXPORT virtual Cesium::RootPtr _CreateCesiumTileTree(Cesium::OutputR rootTileOutput);
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
    DGNPLATFORM_EXPORT DgnDbStatus _FillRangeIndex() override;
    DGNPLATFORM_EXPORT AxisAlignedBox3d _QueryElementsRange() const override;
    GeometricModel3dCP _ToGeometricModel3d() const override final {return this;}
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsertElement(DgnElementR element) override;
    explicit GeometricModel3d(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A GeometricModel2d is a infinite planar model that contains only 2-dimensional DgnElements. Coordinates values are X,Y.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometricModel2d : GeometricModel
{
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_GeometricModel2d, GeometricModel);
    friend struct dgn_ModelHandler::Geometric2d;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _FillRangeIndex() override;
    GeometricModel2dCP _ToGeometricModel2d() const override final {return this;}
    DGNPLATFORM_EXPORT AxisAlignedBox3d _QueryElementsRange() const override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsertElement(DgnElementR element) override;
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
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_SpatialModel, GeometricModel3d);
    friend struct dgn_ModelHandler::Spatial;

protected:
    SpatialModelCP _ToSpatialModel() const override final {return this;}
    explicit SpatialModel(CreateParams const& params) : T_Super(params) {}
    virtual BentleyStatus _GetSpatialClassifiers(Dgn::ModelSpatialClassifiersR classifiers) const { return ERROR; }

public:
    BentleyStatus GetSpatialClassifiers(Dgn::ModelSpatialClassifiersR classifiers) const { return _GetSpatialClassifiers(classifiers); }
    void OnProjectExtentsChanged(AxisAlignedBox3dCR newExtents);
};

//=======================================================================================
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Shaun.Sewall    08/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalModel : SpatialModel
{
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_PhysicalModel, SpatialModel);
    friend struct dgn_ModelHandler::Physical;

private:
    static PhysicalModelPtr Create(DgnDbR db, DgnElementId modeledElementId);

protected:
    PhysicalModelCP _ToPhysicalModel() const override final {return this;}
    explicit PhysicalModel(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static PhysicalModelPtr Create(PhysicalPartitionCR modeledElement);
    DGNPLATFORM_EXPORT static PhysicalModelPtr CreateAndInsert(PhysicalPartitionCR modeledElement);

    DGNPLATFORM_EXPORT static PhysicalModelPtr Create(PhysicalElementCR modeledElement);
    DGNPLATFORM_EXPORT static PhysicalModelPtr CreateAndInsert(PhysicalElementCR modeledElement);

    DGNPLATFORM_EXPORT static PhysicalModelPtr Create(TemplateRecipe3dCR modeledElement);
    DGNPLATFORM_EXPORT static PhysicalModelPtr CreateAndInsert(TemplateRecipe3dCR modeledElement);
};

//=======================================================================================
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Shaun.Sewall    11/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SpatialLocationModel : SpatialModel
{
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_SpatialLocationModel, SpatialModel);
    friend struct dgn_ModelHandler::SpatialLocation;

private:
    static SpatialLocationModelPtr Create(DgnDbR db, DgnElementId modeledElementId);

protected:
    SpatialLocationModelCP _ToSpatialLocationModel() const override final {return this;}
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsertElement(DgnElementR) override;
    explicit SpatialLocationModel(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static SpatialLocationModelPtr Create(SpatialLocationPartitionCR modeledElement);
    DGNPLATFORM_EXPORT static SpatialLocationModelPtr CreateAndInsert(SpatialLocationPartitionCR modeledElement);

    DGNPLATFORM_EXPORT static SpatialLocationModelPtr Create(SpatialLocationPortionCR modeledElement);
    DGNPLATFORM_EXPORT static SpatialLocationModelPtr CreateAndInsert(SpatialLocationPortionCR modeledElement);

    DGNPLATFORM_EXPORT static SpatialLocationModelPtr Create (SpatialLocationElementCR modeledElement);
    DGNPLATFORM_EXPORT static SpatialLocationModelPtr CreateAndInsert (SpatialLocationElementCR modeledElement);
};

//=======================================================================================
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Shaun.Sewall    09/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoleModel : DgnModel
{
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_RoleModel, DgnModel);
    friend struct dgn_ModelHandler::Role;

protected:
    RoleModelCP _ToRoleModel() const override final {return this;}
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsertElement(DgnElementR element) override;
    explicit RoleModel(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Shaun.Sewall    04/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE InformationModel : DgnModel
{
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_InformationModel, DgnModel);
    friend struct dgn_ModelHandler::Information;

protected:
    InformationModelCP _ToInformationModel() const override final {return this;}
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsertElement(DgnElementR element) override;
    explicit InformationModel(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A model which contains only definitions.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DefinitionModel : InformationModel
{
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_DefinitionModel, InformationModel);
    friend struct dgn_ModelHandler::Definition;

private:
    static DefinitionModelPtr Create(DgnDbR db, DgnElementId modeledElementId);

protected:
    DefinitionModelCP _ToDefinitionModel() const override final {return this;}
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsertElement(DgnElementR element) override;

    explicit DefinitionModel(CreateParams const& params) : T_Super(params) {}
    static DefinitionModelPtr Create(CreateParams const& params) {return new DefinitionModel(params);}
public:
    DGNPLATFORM_EXPORT static DefinitionModelPtr Create(DefinitionPartitionCR modeledElement);
    DGNPLATFORM_EXPORT static DefinitionModelPtr CreateAndInsert(DefinitionPartitionCR modeledElement);

    DGNPLATFORM_EXPORT static DefinitionModelPtr Create(DefinitionElementCR modeledElement);
    DGNPLATFORM_EXPORT static DefinitionModelPtr CreateAndInsert(DefinitionElementCR modeledElement);
};

//=======================================================================================
//! A model which contains information about this repository.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Shaun.Sewall    08/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DocumentListModel : InformationModel
{
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_DocumentListModel, InformationModel);
    friend struct dgn_ModelHandler::DocumentList;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsertElement(DgnElementR element) override;
    explicit DocumentListModel(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static DocumentListModelPtr Create(DocumentPartitionCR modeledElement);
    DGNPLATFORM_EXPORT static DocumentListModelPtr CreateAndInsert(DocumentPartitionCR modeledElement);
};

//=======================================================================================
//! A model which contains GroupInformationElements
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Shaun.Sewall    10/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GroupInformationModel : InformationModel
{
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_GroupInformationModel, InformationModel);
    friend struct dgn_ModelHandler::GroupInformation;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsertElement(DgnElementR element) override;
    explicit GroupInformationModel(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A model which contains only information records.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Shaun.Sewall    03/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE InformationRecordModel : InformationModel
{
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_InformationRecordModel, InformationModel);
    friend struct dgn_ModelHandler::InformationRecord;

protected:
    InformationRecordModelCP _ToInformationRecordModel() const override final {return this;}
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsertElement(DgnElementR element) override;

    explicit InformationRecordModel(CreateParams const& params) : T_Super(params) {}
    static InformationRecordModelPtr Create(CreateParams const& params) {return new InformationRecordModel(params);}
public:
    DGNPLATFORM_EXPORT static InformationRecordModelPtr Create(InformationRecordPartitionCR modeledElement);
    DGNPLATFORM_EXPORT static InformationRecordModelPtr CreateAndInsert(InformationRecordPartitionCR modeledElement);
};

//=======================================================================================
//! A model which contains information about this repository.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Shaun.Sewall    08/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RepositoryModel : InformationModel
{
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_RepositoryModel, InformationModel);
    friend struct dgn_ModelHandler::Repository;

protected:
    DgnDbStatus _OnDelete() override {BeAssert(false && "The RepositoryModel cannot be deleted"); return DgnDbStatus::WrongModel;}
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsertElement(DgnElementR element) override;
    explicit RepositoryModel(CreateParams const& params) : T_Super(params) {}
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
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_DictionaryModel, DefinitionModel);
protected:
    DgnDbStatus _OnDelete() override {BeAssert(false && "The DictionaryModel cannot be deleted"); return DgnDbStatus::WrongModel;}
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsertElement(DgnElementR element) override;
    DGNPLATFORM_EXPORT DgnModelPtr _CloneForImport(DgnDbStatus* stat, DgnImportContext& importer, DgnElementCR destinationElementToModel) const override;
public:
    explicit DictionaryModel(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Shaun.Sewall    02/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DrawingModel : GraphicalModel2d
{
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_DrawingModel, GraphicalModel2d);
    friend struct dgn_ModelHandler::Drawing;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    explicit DrawingModel(CreateParams const& params) : T_Super(params) {}

    DrawingModelCP _ToDrawingModel() const override {return this;}

public:
    //! Create a DrawingModel that breaks down the specified Drawing element
    DGNPLATFORM_EXPORT static DrawingModelPtr Create(DrawingCR drawing);

    //! Create a DrawingModel that breaks down the specified TemplateRecipe2d element
    DGNPLATFORM_EXPORT static DrawingModelPtr Create(TemplateRecipe2dCR drawing);
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
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_SectionDrawingModel, DrawingModel);
protected:
    SectionDrawingModelCP _ToSectionDrawingModel() const override final {return this;}
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsertElement(DgnElementR element) override;
public:
    SectionDrawingModel(CreateParams const& params) : T_Super(params) {}
};


#define MODELHANDLER_DECLARE_MEMBERS(__ECClassName__,__classname__,_handlerclass__,_handlersuperclass__,__exporter__) \
        private: Dgn::DgnModel* _CreateInstance(Dgn::DgnModel::CreateParams const& params) override {return new __classname__(__classname__::CreateParams(params));}\
        protected: uint64_t _ParseRestrictedAction(Utf8CP name) const override {return __classname__::RestrictedAction::Parse(name);}\
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
        DOMAINHANDLER_DECLARE_MEMBERS (BIS_CLASS_Model, Model, DgnDomain::Handler, DGNPLATFORM_EXPORT)

    private:
        mutable BeMutex m_mutex;
        ECSqlClassParams m_classParams;

        ECSqlClassParams const& GetECSqlClassParams();
    protected:
        ModelHandlerP _ToModelHandler() override final {return this;}
        virtual DgnModelP _CreateInstance(DgnModel::CreateParams const& params) {return nullptr;}
        DGNPLATFORM_EXPORT virtual DgnModelPtr _CreateNewModel(DgnDbStatus* stat, DgnDbR db, ECN::IECInstanceCR);
        uint64_t _ParseRestrictedAction(Utf8CP name) const override {return DgnModel::RestrictedAction::Parse(name);}
        DGNPLATFORM_EXPORT DgnDbStatus _VerifySchema(DgnDomains&) override;

        //! Add the names of any subclass properties used by ECSql INSERT, UPDATE, and/or SELECT statements to the ECSqlClassParams list.
        //! If you override this method, you @em must invoke T_Super::_GetClassParams().
        DGNPLATFORM_EXPORT void _GetClassParams(ECSqlClassParamsR params) override;

    public:
        //! Find an ModelHandler for a subclass of dgn.Model. This is just a shortcut for FindHandler with the base class
        //! of "dgn.Model".
        DGNPLATFORM_EXPORT static ModelHandlerP FindHandler(DgnDb const&, DgnClassId handlerId);

        //! Create an instance of a (subclass of) DgnModel from CreateParams.
        //! @param[in] params the parameters for the model
        DgnModelPtr Create(DgnModel::CreateParams const& params) {return _CreateInstance(params);}
    };

    //! The ModelHandler for GeometricModel2d
    struct EXPORT_VTABLE_ATTRIBUTE Geometric2d : Model
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_GeometricModel2d, GeometricModel2d, Geometric2d, Model, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for DrawingModel
    struct EXPORT_VTABLE_ATTRIBUTE Drawing : Geometric2d
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_DrawingModel, DrawingModel, Drawing, Geometric2d, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for SpatialModel
    struct EXPORT_VTABLE_ATTRIBUTE Spatial : Model
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_SpatialModel, SpatialModel, Spatial, Model, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for SpatialLocationModel
    struct EXPORT_VTABLE_ATTRIBUTE SpatialLocation : Spatial
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_SpatialLocationModel, SpatialLocationModel, SpatialLocation, Spatial, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for PhysicalModel
    struct EXPORT_VTABLE_ATTRIBUTE Physical : Spatial
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_PhysicalModel, PhysicalModel, Physical, Spatial, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for SectionDrawingModel
    struct EXPORT_VTABLE_ATTRIBUTE SectionDrawing : Drawing
    {
        MODELHANDLER_DECLARE_MEMBERS (BIS_CLASS_SectionDrawingModel, SectionDrawingModel, SectionDrawing, Drawing, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for RoleModel
    struct EXPORT_VTABLE_ATTRIBUTE Role : Model
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_RoleModel, RoleModel, Role, Model, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for InformationModel
    struct EXPORT_VTABLE_ATTRIBUTE Information : Model
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_InformationModel, InformationModel, Information, Model, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for DefinitionModel
    struct EXPORT_VTABLE_ATTRIBUTE Definition : Information
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_DefinitionModel, DefinitionModel, Definition, Information, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for DocumentListModel
    struct EXPORT_VTABLE_ATTRIBUTE DocumentList : Information
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_DocumentListModel, DocumentListModel, DocumentList, Information, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for GroupInformationModel
    struct EXPORT_VTABLE_ATTRIBUTE GroupInformation : Information
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_GroupInformationModel, GroupInformationModel, GroupInformation, Information, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for InformationRecordModel
    struct EXPORT_VTABLE_ATTRIBUTE InformationRecord : Information
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_InformationRecordModel, InformationRecordModel, InformationRecord, Information, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for DictionaryModel
    struct EXPORT_VTABLE_ATTRIBUTE Dictionary : Definition
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_DictionaryModel, DictionaryModel, Dictionary, Definition, DGNPLATFORM_EXPORT)
    };

    //! The ModelHandler for RepositoryModel
    struct EXPORT_VTABLE_ATTRIBUTE Repository : Information
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_RepositoryModel, RepositoryModel, Repository, Information, DGNPLATFORM_EXPORT)
    };
};

END_BENTLEY_DGN_NAMESPACE
