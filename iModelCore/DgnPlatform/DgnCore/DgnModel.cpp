/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnModel.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/CesiumTileTree.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::CreateParams::CreateParams(DgnDbR db, JsonValueCR val) : m_dgndb(db)
    {
    m_classId = ECJsonUtilities::GetClassIdFromClassNameJson(val[DgnModel::json_classFullName()], db.GetClassLocater());

    DgnElement::RelatedElement modeledElement;
    modeledElement.FromJson(db, val[json_modeledElement()]);
    if (!modeledElement.IsValid())
        {
        BeAssert(false);
        LOG.errorv("DgnModel::CreateParams: modeledElement is not a valid RelatedElement. It is \"%s\"", val[DgnModel::json_classFullName()].asString().c_str());
        return;
        }

    m_modeledElementId = modeledElement.m_id;
    m_modeledElementRelClassId = modeledElement.m_relClassId;
    
    if (val.isMember(DgnModel::json_isPrivate()))
        m_isPrivate = val[DgnModel::json_isPrivate()].asBool();
    if (val.isMember(DgnModel::json_isTemplate()))
        m_isTemplate = val[DgnModel::json_isTemplate()].asBool();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DgnModels::QuerySubModelId(DgnCodeCR modeledElementCode) const
    {
    DgnDbR db = GetDgnDb();
    DgnElementId modeledElementId = db.Elements().QueryElementIdByCode(modeledElementCode);
    if (!modeledElementId.IsValid())
        return DgnModelId();

    DgnElementCPtr modeledElement = db.Elements().GetElement(modeledElementId);
    if (!modeledElement.IsValid())
        return DgnModelId();

    return modeledElement->GetSubModelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModels::AddLoadedModel(DgnModelR model)
    {
    model.m_persistent = true;
    BeMutexHolder _v_v(m_mutex);
    m_models.Insert(model.GetModelId(), &model);

    auto geomModel = model.ToGeometricModelP();
    if (nullptr != geomModel)
        geomModel->InitLastElementModifiedTime();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModels::DropLoadedModel(DgnModelR model)
    {
    model.m_persistent = false;
    BeMutexHolder _v_v(m_mutex);
    m_models.erase(model.GetModelId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_PreDestroy()
    {
    NotifyAppData([](AppData& handler, DgnModelR model) { handler._OnUnload(model); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_Destroy()
    {
    NotifyAppData([](AppData& handler, DgnModelR model) { handler._OnUnloaded(model); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModels::Empty()
    {
    // Tile-loader threads may be executing, so must release roots (and wait for loaders to cancel) first
    // Must be done before destroying Elements because tile loaders may have ref-counted ptrs to elements
    // First, cancel all loads asynchronously.
    for (auto& kvp : m_models)
        kvp.second->_PreDestroy();

    // Now, synchronously wait for all loads to terminate - should be fast since we've marked them all canceled.
    for (auto& kvp : m_models)
        kvp.second->_Destroy();

    // Has to be called before models are released.
    m_dgndb.Elements().Destroy();
    m_models.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ModelIterator DgnModels::MakeIterator(Utf8CP className, Utf8CP whereClause, Utf8CP orderByClause) const
    {
    Utf8String sql("SELECT ECInstanceId,ECClassId,ModeledElement.Id,IsTemplate,IsPrivate,ParentModel.Id FROM ");
    sql.append(className);

    if (whereClause)
        {
        sql.append(" ");
        sql.append(whereClause);
        }

    if (orderByClause)
        {
        sql.append(" ");
        sql.append(orderByClause);
        }

    ModelIterator iterator;
    iterator.Prepare(m_dgndb, sql.c_str(), 0 /* Index of ECInstanceId */);
    return iterator;
    }

DgnModelId ModelIteratorEntry::GetModelId() const {return m_statement->GetValueId<DgnModelId>(0);}
DgnClassId ModelIteratorEntry::GetClassId() const {return m_statement->GetValueId<DgnClassId>(1);}
DgnElementId ModelIteratorEntry::GetModeledElementId() const {return m_statement->GetValueId<DgnElementId>(2);}
bool ModelIteratorEntry::IsTemplate() const {return m_statement->GetValueBoolean(3);}
bool ModelIteratorEntry::IsPrivate() const {return m_statement->GetValueBoolean(4);}
DgnModelId ModelIteratorEntry::GetParentModelId() const {return m_statement->GetValueId<DgnModelId>(5);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlClassInfo const& DgnModels::FindClassInfo(DgnModelR model)
    {
    DgnClassId classId = model.GetClassId();

    BeMutexHolder lock(m_mutex);

    auto found = m_classInfos.find(classId);
    if (found != m_classInfos.end())
        return found->second;

    ECSqlClassInfo& classInfo = m_classInfos[classId];
    bool populated = model.GetModelHandler().GetECSqlClassParams().BuildClassInfo(classInfo, m_dgndb, classId);
    BeAssert(populated);
    UNUSED_VARIABLE(populated);

    return classInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr DgnModels::GetSelectStmt(DgnModelR model) {return FindClassInfo(model).GetSelectStmt(m_dgndb, ECInstanceId(model.GetModelId().GetValue()));}
CachedECSqlStatementPtr DgnModels::GetInsertStmt(DgnModelR model) {return FindClassInfo(model).GetInsertStmt(m_dgndb);}
CachedECSqlStatementPtr DgnModels::GetUpdateStmt(DgnModelR model) {return FindClassInfo(model).GetUpdateStmt(m_dgndb, ECInstanceId(model.GetModelId().GetValue()));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::DgnModel(CreateParams const& params) : m_dgndb(params.m_dgndb), m_classId(params.m_classId), m_modeledElementId(params.m_modeledElementId), m_isPrivate(params.m_isPrivate),
    m_isTemplate(params.m_isTemplate), m_persistent(false)
    {
    m_parentModelId = GetDgnDb().Elements().QueryModelId(GetModeledElementId());
    m_modeledElementRelClassId = params.m_modeledElementRelClassId.IsValid() ? params.m_modeledElementRelClassId : GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ModelModelsElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnModel::GetName() const
    {
    // WIP: keep this method around to avoid having to change too much source code. Use the CodeValue of the modeled element as this model's name.
    DgnElementCPtr modeledElement = GetModeledElement();
    return modeledElement.IsValid() ? modeledElement->GetCode().GetValue().GetUtf8() : Utf8String();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnModel::GetModeledElement() const
    {
    DgnElementCPtr modeledElement = GetDgnDb().Elements().GetElement(GetModeledElementId());
    BeAssert(modeledElement.IsValid());
    return modeledElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::AddAppDataInternal(AppData::Key const& key, AppData* obj)
    {
    auto entry = m_appData.Insert(&key, obj);
    if (entry.second)
        return;

    // we already had appdata for this key. Clean up old and save new.
    entry.first->second = obj;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnModel::DropAppData(AppData::Key const& key)
    {
    BeMutexHolder lock(m_mutex);
    return 0 == m_appData.erase(&key) ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::AppData* DgnModel::FindAppDataInternal(AppData::Key const& key) const
    {
    auto entry = m_appData.find(&key);
    return entry == m_appData.end() ? nullptr : entry->second.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void DgnModel::CallAppData(T const& caller) const
    {
    BeMutexHolder lock(m_mutex);
    for (auto entry=m_appData.begin(); entry!=m_appData.end(); )
        {
        if (DgnModel::AppData::DropMe::Yes == caller(*entry->second, *this))
            entry = m_appData.erase(entry);
        else
            ++entry;
        }
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void DgnModel::NotifyAppData(T const& notifier)
    {
    BeMutexHolder lock(m_mutex);
    for (auto& entry : m_appData)
        notifier(*entry.second, *this);
    }

/*---------------------------------------------------------------------------------**//**
* Destructor for DgnModel. Free all memory allocated to this DgnModel.
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::~DgnModel()
    {
    BeMutexHolder lock(m_mutex);
    m_appData.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricModel2d::_OnInsertElement(DgnElementR element)
    {
    auto geom = element.ToGeometrySource();

    // if it is a geometric element, it must be a 2d element.
    return (geom && !geom->Is2d()) ? DgnDbStatus::Mismatch2d3d : T_Super::_OnInsertElement(element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SectionDrawingModel::_OnInsertElement(DgnElementR el)
    {
    return T_Super::_OnInsertElement(el);;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricModel3d::_OnInsertElement(DgnElementR element)
    {
    auto geom = element.ToGeometrySource();

    // if it is a geometric element, it must be a 3d element.
    return (geom && !geom->Is3d()) ? DgnDbStatus::Mismatch2d3d : T_Super::_OnInsertElement(element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GroupInformationModel::_OnInsertElement(DgnElementR element)
    {
    if (nullptr == dynamic_cast<GroupInformationElementCP>(&element))
        {
        BeAssert(false);
        return DgnDbStatus::WrongModel;
        }

    return T_Super::_OnInsertElement(element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus InformationRecordModel::_OnInsertElement(DgnElementR element)
    {
    if (nullptr == dynamic_cast<InformationRecordElementCP>(&element))
        {
        BeAssert(false);
        return DgnDbStatus::WrongModel;
        }

    return T_Super::_OnInsertElement(element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
InformationRecordModelPtr InformationRecordModel::Create(InformationRecordPartitionCR modeledElement)
    {
    DgnDbR db = modeledElement.GetDgnDb();
    ModelHandlerR handler = dgn_ModelHandler::InformationRecord::GetHandler();
    DgnClassId classId = db.Domains().GetClassId(handler);
    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, modeledElement.GetElementId()));
    if (!classId.IsValid() || !model.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return model->ToInformationRecordModelP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
InformationRecordModelPtr InformationRecordModel::CreateAndInsert(InformationRecordPartitionCR modeledElement)
    {
    InformationRecordModelPtr model = Create(modeledElement);
    return (model.IsValid() && (DgnDbStatus::Success == model->Insert())) ? model : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr PhysicalModel::Create(DgnDbR db, DgnElementId modeledElementId)
    {
    ModelHandlerR handler = dgn_ModelHandler::Physical::GetHandler();
    DgnClassId classId = db.Domains().GetClassId(handler);
    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, modeledElementId));
    if (!classId.IsValid() || !model.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return model->ToPhysicalModelP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialModel::OnProjectExtentsChanged(AxisAlignedBox3dCR newExtents)
    {
    // ###TODO_IMODELCORE: notify app data
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr PhysicalModel::Create(PhysicalPartitionCR modeledElement)
    {
    return Create(modeledElement.GetDgnDb(), modeledElement.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr PhysicalModel::Create(PhysicalElementCR modeledElement)
    {
    return Create(modeledElement.GetDgnDb(), modeledElement.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr PhysicalModel::Create(TemplateRecipe3dCR modeledElement)
    {
    PhysicalModelPtr model = Create(modeledElement.GetDgnDb(), modeledElement.GetElementId());
    if (model.IsValid())
        model->m_isTemplate = true;

    return model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr PhysicalModel::CreateAndInsert(PhysicalPartitionCR modeledElement)
    {
    PhysicalModelPtr model = Create(modeledElement);
    return (model.IsValid() && (DgnDbStatus::Success == model->Insert())) ? model : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr PhysicalModel::CreateAndInsert(PhysicalElementCR modeledElement)
    {
    PhysicalModelPtr model = Create(modeledElement);
    return (model.IsValid() && (DgnDbStatus::Success == model->Insert())) ? model : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr PhysicalModel::CreateAndInsert(TemplateRecipe3dCR modeledElement)
    {
    PhysicalModelPtr model = Create(modeledElement);
    return (model.IsValid() && (DgnDbStatus::Success == model->Insert())) ? model : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonas.Valiunas  03/17
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialLocationModelPtr SpatialLocationModel::Create(DgnDbR db, DgnElementId modeledElementId)
    {
    ModelHandlerR handler = dgn_ModelHandler::SpatialLocation::GetHandler();
    DgnClassId classId = db.Domains().GetClassId(handler);
    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, modeledElementId));
    if (!classId.IsValid() || !model.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return model->ToSpatialLocationModelP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonas.Valiunas  03/17
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialLocationModelPtr SpatialLocationModel::Create(SpatialLocationPortionCR modeledElement)
    {
    return SpatialLocationModel::Create(modeledElement.GetDgnDb(), modeledElement.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonas.Valiunas  03/17
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialLocationModelPtr SpatialLocationModel::Create(SpatialLocationElementCR modeledElement)
    {
    return SpatialLocationModel::Create(modeledElement.GetDgnDb(), modeledElement.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonas.Valiunas  03/17
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialLocationModelPtr SpatialLocationModel::CreateAndInsert(SpatialLocationPortionCR modeledElement)
    {
    SpatialLocationModelPtr model = Create(modeledElement);
    return (model.IsValid() && (DgnDbStatus::Success == model->Insert())) ? model : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonas.Valiunas  03/17
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialLocationModelPtr SpatialLocationModel::CreateAndInsert(SpatialLocationElementCR modeledElement)
    {
    SpatialLocationModelPtr model = Create(modeledElement);
    return (model.IsValid() && (DgnDbStatus::Success == model->Insert())) ? model : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialLocationModelPtr SpatialLocationModel::Create(SpatialLocationPartitionCR modeledElement)
    {
    return SpatialLocationModel::Create(modeledElement.GetDgnDb(), modeledElement.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialLocationModelPtr SpatialLocationModel::CreateAndInsert(SpatialLocationPartitionCR modeledElement)
    {
    SpatialLocationModelPtr model = Create(modeledElement);
    return (model.IsValid() && (DgnDbStatus::Success == model->Insert())) ? model : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SpatialLocationModel::_OnInsertElement(DgnElementR element)
    {
    return dynamic_cast<PhysicalElement*>(&element) ? DgnDbStatus::WrongModel : T_Super::_OnInsertElement(element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus InformationModel::_OnInsertElement(DgnElementR element)
    {
    return element.IsInformationContentElement() ? T_Super::_OnInsertElement(element) : DgnDbStatus::WrongModel;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonas.Valiunas  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionModelPtr DefinitionModel::Create(DgnDbR db, DgnElementId modeledElementId)
    {
    ModelHandlerR handler = dgn_ModelHandler::Definition::GetHandler();
    DgnClassId classId = db.Domains().GetClassId(handler);
    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, modeledElementId));
    if (!classId.IsValid() || !model.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return dynamic_cast<DefinitionModelP>(model.get());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionModelPtr DefinitionModel::Create(DefinitionPartitionCR modeledElement)
    {
    return DefinitionModel::Create(modeledElement.GetDgnDb(), modeledElement.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionModelPtr DefinitionModel::CreateAndInsert(DefinitionPartitionCR modeledElement)
    {
    DefinitionModelPtr model = Create(modeledElement);
    return (model.IsValid() && (DgnDbStatus::Success == model->Insert())) ? model : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Jonas.Valiunas     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionModelPtr DefinitionModel::Create(DefinitionElementCR modeledElement)
    {
    return DefinitionModel::Create(modeledElement.GetDgnDb(), modeledElement.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Jonas.Valiunas     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionModelPtr DefinitionModel::CreateAndInsert(DefinitionElementCR modeledElement)
    {
    DefinitionModelPtr model = Create(modeledElement);
    return (model.IsValid() && (DgnDbStatus::Success == model->Insert())) ? model : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DefinitionModel::_OnInsertElement(DgnElementR el)
    {
    auto status = T_Super::_OnInsertElement(el);
    if (DgnDbStatus::Success == status && el.IsGeometricElement())
        status = DgnDbStatus::WrongModel;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DictionaryModel::_OnInsertElement(DgnElementR el)
    {
    // dictionary model can contain *only* DefinitionElements
    DgnDbStatus status = el.IsDefinitionElement() ? T_Super::_OnInsertElement(el) : DgnDbStatus::WrongModel;
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DocumentListModel::_OnInsertElement(DgnElementR element)
    {
    // only Document elements go into a DocumentListModel
    return element.IsDocumentElement() ? T_Super::_OnInsertElement(element) : DgnDbStatus::WrongModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DocumentListModelPtr DocumentListModel::Create(DocumentPartitionCR modeledElement)
    {
    DgnDbR db = modeledElement.GetDgnDb();
    ModelHandlerR handler = dgn_ModelHandler::DocumentList::GetHandler();
    DgnClassId classId = db.Domains().GetClassId(handler);
    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, modeledElement.GetElementId()));
    if (!classId.IsValid() || !model.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return dynamic_cast<DocumentListModelP>(model.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DocumentListModelPtr DocumentListModel::CreateAndInsert(DocumentPartitionCR modeledElement)
    {
    DocumentListModelPtr model = Create(modeledElement);
    return (model.IsValid() && (DgnDbStatus::Success == model->Insert())) ? model : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DrawingModel::_OnInsert()
    {
    if (!GetDgnDb().Elements().Get<Drawing>(GetModeledElementId()).IsValid() && !GetDgnDb().Elements().Get<TemplateRecipe2d>(GetModeledElementId()).IsValid())
        {
        BeAssert(false && "A DrawingModel should be modeling a Drawing or TemplateRecipe2d element");
        return DgnDbStatus::BadElement;
        }

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DrawingModelPtr DrawingModel::Create(DrawingCR drawing)
    {
    DgnDbR db = drawing.GetDgnDb();
    ModelHandlerR handler = dgn_ModelHandler::Drawing::GetHandler();
    DgnClassId classId = db.Domains().GetClassId(handler);
    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, drawing.GetElementId()));
    if (!classId.IsValid() || !model.IsValid())
        return nullptr;

    return dynamic_cast<DrawingModelP>(model.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    02/17
+---------------+---------------+---------------+---------------+---------------+------*/
DrawingModelPtr DrawingModel::Create(TemplateRecipe2dCR recipe)
    {
    DgnDbR db = recipe.GetDgnDb();
    ModelHandlerR handler = dgn_ModelHandler::Drawing::GetHandler();
    DgnClassId classId = db.Domains().GetClassId(handler);
    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, recipe.GetElementId()));
    if (!classId.IsValid() || !model.IsValid())
        return nullptr;

    model->SetIsTemplate(true);
    return dynamic_cast<DrawingModelP>(model.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RepositoryModel::_OnInsertElement(DgnElementR element)
    {
    return element.IsInformationContentElement() && !element.IsDefinitionElement() ? T_Super::_OnInsertElement(element) : DgnDbStatus::WrongModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RoleModel::_OnInsertElement(DgnElementR element)
    {
    return element.IsGeometricElement() ? DgnDbStatus::WrongModel : T_Super::_OnInsertElement(element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::Read(DgnModelId modelId)
    {
    m_modelId = modelId;
    auto const& params = GetModelHandler().GetECSqlClassParams();

    CachedECSqlStatementPtr stmt = GetDgnDb().Models().GetSelectStmt(*this);
    if (stmt.IsNull())
        {
        BeAssert(false);
        return DgnDbStatus::ReadError;
        }
        
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::ReadError;

    return _ReadSelectParams(*stmt, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_ReadSelectParams(ECSqlStatement& statement, ECSqlClassParamsCR params)
    {
    int propsIndex = params.GetSelectIndex(prop_JsonProperties());
    if (!statement.IsValueNull(propsIndex))
        {
        if (!Json::Reader::Parse(statement.GetValueText(propsIndex), m_jsonProperties))
            {
            BeAssert(false);
            return DgnDbStatus::ReadError;
            }

        _OnLoadedJsonProperties();
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_BindWriteParams(BeSQLite::EC::ECSqlStatement& statement, ForInsert forInsert)
    {
    if (ForInsert::Yes == forInsert)
        statement.BindId(statement.GetParameterIndex(prop_ECInstanceId()), m_modelId);

    if (!m_parentModelId.IsValid() || !m_modeledElementId.IsValid() || !m_modeledElementRelClassId.IsValid())
        {
        BeAssert(false);
        return;
        }

    if (ForInsert::Yes == forInsert)
        {
        statement.BindNavigationValue(statement.GetParameterIndex(prop_ParentModel()), m_parentModelId);
        statement.BindNavigationValue(statement.GetParameterIndex(prop_ModeledElement()), m_modeledElementId, m_modeledElementRelClassId);
        }

    statement.BindBoolean(statement.GetParameterIndex(prop_IsPrivate()), m_isPrivate);
    statement.BindBoolean(statement.GetParameterIndex(prop_IsTemplate()), m_isTemplate);

    _OnSaveJsonProperties();
    if (!m_jsonProperties.isNull())
        statement.BindText(statement.GetParameterIndex(prop_JsonProperties()), m_jsonProperties.ToString().c_str(), IECSqlBinder::MakeCopy::Yes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_ToJson(JsonValueR val, JsonValueCR opts) const
    {
    val[json_id()] = m_modelId.ToHexStr();

    auto ecClass = GetDgnDb().Schemas().GetClass(m_classId);

    val[json_classFullName()] = ecClass->GetFullName();
    DgnElement::AddBisClassName(val, ecClass);
    
    if (m_parentModelId.IsValid())
        val[json_parentModel()] = m_parentModelId.ToHexStr();

    if (m_modeledElementId.IsValid())
        {
        DgnElement::RelatedElement modeledElement(m_modeledElementId, m_modeledElementRelClassId);
        val[json_modeledElement()] = modeledElement.ToJson(GetDgnDb());
        }

    if (!m_jsonProperties.empty())
        val[json_jsonProperties()] = m_jsonProperties;
    
    val[json_name()] = GetName();

    if (m_isPrivate)
        val[json_isPrivate()] = true;

    if (m_isTemplate)
        val[json_isTemplate()] = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_FromJson(JsonValueR val)
    {
    if (val.isMember(json_jsonProperties()))
        m_jsonProperties.From(std::move(val[json_jsonProperties()]));
    if (val.isMember(json_isPrivate()))
        m_isPrivate = val[json_isPrivate()].asBool();
    if (val.isMember(json_isTemplate()))
        m_isTemplate = val[json_isTemplate()].asBool();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::Update()
    {
    DgnDbStatus status = _OnUpdate();
    if (status != DgnDbStatus::Success)
        return status;

    CachedECSqlStatementPtr stmt = GetDgnDb().Models().GetUpdateStmt(*this);
    if (stmt.IsNull())
        return DgnDbStatus::WriteError;

    _BindWriteParams(*stmt, ForInsert::No);

    DbResult result = stmt->Step();
    if (BE_SQLITE_DONE != result)
        return DgnDbStatus::WriteError;

    if (DgnDbStatus::Success == status)
        _OnUpdated();

    return DgnDbStatus::Success;
    }

struct UpdatedCaller {DgnModel::AppData::DropMe operator()(DgnModel::AppData& handler, DgnModelCR model) const {return handler._OnUpdated(model);}};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnUpdated()
    {
    CallAppData(UpdatedCaller());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_OnUpdate()
    {
    ModelHandlerR modelHandler = GetModelHandler();
    if (modelHandler._IsRestrictedAction(RestrictedAction::Update))
        return DgnDbStatus::MissingHandler;

    if (modelHandler.GetDomain().IsReadonly())
        return DgnDbStatus::ReadOnlyDomain;

    {
    BeMutexHolder lock(m_mutex);
    for (auto entry=m_appData.begin(); entry!=m_appData.end(); ++entry)
        {
        DgnDbStatus stat = entry->second->_OnUpdate(*this);
        if (DgnDbStatus::Success != stat)
            return stat;
        }
    }

    // Ensure code is reserved and lock acquired
    return GetDgnDb().BriefcaseManager().OnModelUpdate(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricModel3d::_FillRangeIndex()
    {
    BeMutexHolder lock(m_mutex);
    if (nullptr != m_rangeIndex)
        return DgnDbStatus::Success;

    m_rangeIndex.reset(new RangeIndex::Tree(true, 20));
    auto stmt = m_dgndb.GetPreparedECSqlStatement("SELECT ECInstanceId,Category.Id,Origin,Yaw,Pitch,Roll,BBoxLow,BBoxHigh FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " WHERE Model.Id=?");
    stmt->BindId(1, GetModelId());
    while (BE_SQLITE_ROW == stmt->Step())
        {
        if (stmt->IsValueNull(2)) // has no placement
            continue;

        double yaw   = stmt->GetValueDouble(3);
        double pitch = stmt->GetValueDouble(4);
        double roll  = stmt->GetValueDouble(5);

        DPoint3d low = stmt->GetValuePoint3d(6);
        DPoint3d high = stmt->GetValuePoint3d(7);

        Placement3d placement(stmt->GetValuePoint3d(2),
                              YawPitchRollAngles(Angle::FromDegrees(yaw), Angle::FromDegrees(pitch), Angle::FromDegrees(roll)),
                              ElementAlignedBox3d(low.x, low.y, low.z, high.x, high.y, high.z));

        m_rangeIndex->AddEntry(RangeIndex::Entry(placement.CalculateRange(), stmt->GetValueId<DgnElementId>(0), stmt->GetValueId<DgnCategoryId>(1)));
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricModel2d::_FillRangeIndex()
    {
    BeMutexHolder lock(m_mutex);
    if (nullptr != m_rangeIndex)
        return DgnDbStatus::Success;

    m_rangeIndex.reset(new RangeIndex::Tree(false, 20));

    auto stmt = m_dgndb.GetPreparedECSqlStatement("SELECT ECInstanceId,Category.Id,Origin,Rotation,BBoxLow,BBoxHigh FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement2d) " WHERE Model.Id=?");
    stmt->BindId(1, GetModelId());

    while (BE_SQLITE_ROW == stmt->Step())
        {
        if (stmt->IsValueNull(2)) // has no placement
            continue;

        DPoint2d low  = stmt->GetValuePoint2d(4);
        DPoint2d high = stmt->GetValuePoint2d(5);

        Placement2d placement(stmt->GetValuePoint2d(2),
                              AngleInDegrees::FromDegrees(stmt->GetValueDouble(3)),
                              ElementAlignedBox2d(low.x, low.y, high.x, high.y));

        m_rangeIndex->AddEntry(RangeIndex::Entry(placement.CalculateRange(), stmt->GetValueId<DgnElementId>(0), stmt->GetValueId<DgnCategoryId>(1)));
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::AddToRangeIndex(DgnElementCR element)
    {
    // NB: We are relying on our knowledge that in order for the tile tree to exist, the range index must be non-null...
    if (nullptr == m_rangeIndex)
        return;

    GeometrySourceCP geom = element.ToGeometrySource();
    if (nullptr != geom)
        {
        m_rangeIndex->AddElement(*geom);
        /* ###TODO_IMODELCORE notify app data
        if (geom->HasGeometry() && m_root.IsValid())
            m_root->OnAddToRangeIndex(geom->CalculateRange3d(), element.GetElementId());
        */
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::RemoveFromRangeIndex(DgnElementCR element)
    {
    // NB: We are relying on our knowledge that in order for the tile tree to exist, the range index must be non-null...
    if (nullptr == m_rangeIndex)
        return;

    GeometrySourceCP geom = element.ToGeometrySource();
    if (nullptr != geom && geom->HasGeometry())
        {
        m_rangeIndex->RemoveElement(element.GetElementId());
        /* ###TODO_IMODELCORE notify app data
        if (m_root.IsValid())
            m_root->OnRemoveFromRangeIndex(geom->CalculateRange3d(), element.GetElementId());
        */
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::UpdateRangeIndex(DgnElementCR modified, DgnElementCR original)
    {
    // NB: We are relying on our knowledge that in order for the tile tree to exist, the range index must be non-null...
    if (nullptr == m_rangeIndex)
        return;

    GeometrySourceCP origGeom = original.ToGeometrySource();
    if (nullptr == origGeom)
        return;

    GeometrySourceCP newGeom = modified.ToGeometrySource();
    if (nullptr == newGeom)
        return;

    bool origHasGeom = origGeom->HasGeometry(),
         newHasGeom  = newGeom->HasGeometry();
    AxisAlignedBox3d origBox = origHasGeom ? origGeom->CalculateRange3d() : AxisAlignedBox3d(),
                     newBox  = newHasGeom ? newGeom->CalculateRange3d() : AxisAlignedBox3d();

    auto id = original.GetElementId();
    if (!origBox.IsEqual(newBox)) // many changes don't affect range
        {
        m_rangeIndex->RemoveElement(id);
        m_rangeIndex->AddEntry(RangeIndex::Entry(newBox, id, origGeom->GetCategoryId()));
        }

    /* ###TODO_IMODELCORE notify app data
    if (m_root.IsValid())
        {
        if (origHasGeom)
            {
            if (newHasGeom)
                m_root->OnUpdateRangeIndex(origBox, newBox, id);
            else
                m_root->OnRemoveFromRangeIndex(origBox, id);
            }
        else if (newHasGeom)
            {
            m_root->OnAddToRangeIndex(newBox, id);
            }
        }
    */
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_OnInsertElement(DgnElementR element)
    {
    if (m_dgndb.IsReadonly())
        return DgnDbStatus::ReadOnly;

    if (GetModelHandler()._IsRestrictedAction(RestrictedAction::InsertElement))
        return DgnDbStatus::MissingHandler;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_OnDeleteElement(DgnElementCR element)
    {
    if (m_dgndb.IsReadonly())
        return DgnDbStatus::ReadOnly;

    return GetModelHandler()._IsRestrictedAction(RestrictedAction::DeleteElement) ? DgnDbStatus::MissingHandler : DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_OnUpdateElement(DgnElementCR modified, DgnElementCR original)
    {
    if (m_dgndb.IsReadonly())
        return DgnDbStatus::ReadOnly;

    return GetModelHandler()._IsRestrictedAction(RestrictedAction::UpdateElement) ? DgnDbStatus::MissingHandler : DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_OnDelete()
    {
    ModelHandlerR modelHandler = GetModelHandler();
    if (modelHandler._IsRestrictedAction(RestrictedAction::Delete))
        return DgnDbStatus::MissingHandler;

    if (modelHandler.GetDomain().IsReadonly())
        return DgnDbStatus::ReadOnlyDomain;

    DgnDbStatus stat = GetDgnDb().BriefcaseManager().OnModelDelete(*this);
    if (DgnDbStatus::Success != stat)
        return stat;

    NotifyAppData([](AppData& handler, DgnModelR model) { handler._OnDelete(model); });

    // before we can delete a model, we must delete all of its elements. If that fails, we cannot continue.
    Statement stmt(m_dgndb, "SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE ModelId=?");
    stmt.BindId(1, m_modelId);

    auto& elements = m_dgndb.Elements();
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnElementCPtr el = elements.GetElement(stmt.GetValueId<DgnElementId>(0));
        if (!el.IsValid())
            {
            BeAssert(false);
            return DgnDbStatus::BadElement;
            }

        // Note: this may look dangerous (deleting an element in the model we're iterating), but is is actually safe in SQLite.
        DgnDbStatus stat = el->Delete();
        if (DgnDbStatus::Success != stat)
            return stat;
        }

    BeAssert(GetRefCount() > 1);
    m_dgndb.Models().DropLoadedModel(*this);
    return DgnDbStatus::Success;
    }

struct DeletedCaller {DgnModel::AppData::DropMe operator()(DgnModel::AppData& handler, DgnModelCR model) const {return handler._OnDeleted(model);}};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnDeleted()
    {
    CallAppData(DeletedCaller());
    GetDgnDb().DeleteLinkTableRelationships(BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels), DgnElementId() /* all ModelSelectors */, GetModeledElementId()); // replicate former foreign key behavior
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus DgnModel::_PopulateRequest(IBriefcaseManager::Request& req, BeSQLite::DbOpcode op) const
    {
    switch (op)
        {
        case BeSQLite::DbOpcode::Insert:
            req.Locks().Insert(GetDgnDb(), LockLevel::Shared);
            break;

        case BeSQLite::DbOpcode::Delete:
            {
            req.Locks().Insert(*this, LockLevel::Exclusive);

            // before we can delete a model, we must delete all of its elements. If that fails, we cannot continue.
            Statement stmt(m_dgndb, "SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE ModelId=?");
            stmt.BindId(1, m_modelId);
            auto& elements = m_dgndb.Elements();
            while (BE_SQLITE_ROW == stmt.Step())
                {
                DgnElementCPtr el = elements.GetElement(stmt.GetValueId<DgnElementId>(0));
                BeAssert(el.IsValid());
                if (el.IsValid())
                    {
                    auto stat = el->PopulateRequest(req, BeSQLite::DbOpcode::Delete);
                    if (RepositoryStatus::Success != stat)
                        return stat;
                    }
                }
            break;
            }
        case BeSQLite::DbOpcode::Update:
            {
            req.Locks().Insert(*this, LockLevel::Exclusive);
            break;
            }
        }

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_OnInsert()
    {
    if (GetDgnDb().IsReadonly())
        return DgnDbStatus::ReadOnly;

    if (m_modelId.IsValid())
        return DgnDbStatus::IdExists;

    ModelHandlerR modelHandler = GetModelHandler();
    if (modelHandler._IsRestrictedAction(RestrictedAction::Insert))
        return DgnDbStatus::MissingHandler;

    if (modelHandler.GetDomain().IsReadonly())
        return DgnDbStatus::ReadOnlyDomain;

    // Ensure db is not exclusively locked and code reserved
    return GetDgnDb().BriefcaseManager().OnModelInsert(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnInserted() 
    {
    GetDgnDb().Models().AddLoadedModel(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnLoaded() 
    {
    GetDgnDb().Models().AddLoadedModel(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::Delete()
    {
    if (!m_persistent)
        return DgnDbStatus::WrongModel;

    // give the element being modeled a chance to reject the delete
    DgnDbStatus status;
    DgnElementCPtr modeledElement = GetDgnDb().Elements().GetElement(GetModeledElementId());
    BeAssert(modeledElement.IsValid());
    if (modeledElement.IsValid() && (DgnDbStatus::Success != (status=modeledElement->_OnSubModelDelete(*this))))
        return status;

    DgnDbStatus stat = _OnDelete();
    if (DgnDbStatus::Success != stat)
        return stat;

    Statement stmt(m_dgndb, "DELETE FROM " BIS_TABLE(BIS_CLASS_Model) " WHERE Id=?");
    stmt.BindId(1, m_modelId);
    if (BE_SQLITE_DONE != stmt.Step())
        return DgnDbStatus::WriteError;

    _OnDeleted();

    // notify the element being modeled that the DgnModel has been deleted
    if (modeledElement.IsValid())
        modeledElement->_OnSubModelDeleted(*this);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::Insert()
    {
    if (!m_modeledElementId.IsValid())
        {
        BeAssert(false && "A DgnModel must be modeling a DgnElement (that is above it in the hierarchy)");
        return DgnDbStatus::BadElement;
        }

    DgnDbStatus status = _OnInsert();
    if (DgnDbStatus::Success != status)
        return status;

    // A DgnModel's ID has the same value as the DgnElement that it is modeling
    m_modelId = DgnModelId(m_modeledElementId.GetValue());

    // give the element being modeled a chance to reject the insert
    DgnElementCPtr modeledElement = GetDgnDb().Elements().GetElement(GetModeledElementId());
    BeAssert(modeledElement.IsValid());

    BeAssert(m_parentModelId == modeledElement->GetModelId()); // alert caller that we're going to change this value
    m_parentModelId = modeledElement->GetModelId(); // this is redundant data, make sure it's right

    if (modeledElement.IsValid() && (DgnDbStatus::Success != (status=modeledElement->_OnSubModelInsert(*this))))
        return status;

    CachedECSqlStatementPtr stmt = GetDgnDb().Models().GetInsertStmt(*this);
    if (stmt.IsNull())
        {
        m_modelId = DgnModelId();
        return DgnDbStatus::WriteError;
        }
    
    _BindWriteParams(*stmt, ForInsert::Yes);
        
    DbResult stmtResult = stmt->Step();
    if (BE_SQLITE_DONE != stmtResult)
        {
        m_modelId = DgnModelId();
        return DgnDbStatus::WriteError;
        }
        
    // NB: We do this here rather than in _OnInserted() because Update() is going to request a lock too, and the server doesn't need to be involved in locks for models created locally.
    GetDgnDb().BriefcaseManager().OnModelInserted(GetModelId());
    status = Update();
    BeAssert(status == DgnDbStatus::Success);

    _OnInserted();

    // notify the element being modeled about the new DgnModel
    if (modeledElement.IsValid())
        modeledElement->_OnSubModelInserted(*this);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_InitFrom(DgnModelCR other)
    {
    m_isPrivate = other.m_isPrivate;
    m_isTemplate = other.m_isTemplate;

    auto& ncOther = const_cast<DgnModelR>(other);

    ncOther._OnSaveJsonProperties();
    m_jsonProperties = other.m_jsonProperties;
    _OnLoadedJsonProperties();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::_OnSaveJsonProperties() 
    {
    T_Super::_OnSaveJsonProperties();
    SetJsonProperties(json_formatter(), m_displayInfo.ToJson());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::_OnLoadedJsonProperties() 
    {
    T_Super::_OnLoadedJsonProperties();
    m_displayInfo.FromJson(GetJsonProperties(json_formatter()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
ModelHandlerR DgnModel::GetModelHandler() const
    {
    return *dgn_ModelHandler::Model::FindHandler(m_dgndb, m_classId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnModels::LoadDgnModel(DgnModelId modelId)
    {
    enum Column {ClassId=0, ModeledElementId=1, ModeledElementRelECClassId=2, IsPrivate=3, IsTemplate=4};
    Statement stmt(m_dgndb, "SELECT ECClassId,ModeledElementId,ModeledElementRelECClassId,IsPrivate,IsTemplate FROM " BIS_TABLE(BIS_CLASS_Model) " WHERE Id=?");
    stmt.BindId(1, modelId);

    if (BE_SQLITE_ROW != stmt.Step())
        return nullptr;

    DgnClassId modelClassId = stmt.GetValueId<DgnClassId>(Column::ClassId);
    DgnElementId modeledElementId = stmt.GetValueId<DgnElementId>(Column::ModeledElementId);
    DgnClassId modeledElementRelECClassId = stmt.GetValueId<DgnClassId>(Column::ModeledElementRelECClassId);
    bool isPrivate = stmt.GetValueBoolean(Column::IsPrivate);
    bool isTemplate = stmt.GetValueBoolean(Column::IsTemplate);

    // make sure the class derives from Model (has a handler)
    ModelHandlerP handler = dgn_ModelHandler::Model::FindHandler(m_dgndb, modelClassId);
    if (nullptr == handler)
        return nullptr;

    DgnModel::CreateParams params(m_dgndb, modelClassId, modeledElementId, isPrivate, isTemplate);
    params.SetModeledElementRelClassId(modeledElementRelECClassId);
    DgnModelPtr dgnModel = handler->Create(params);
    if (!dgnModel.IsValid())
        return nullptr;

    dgnModel->Read(modelId);
    dgnModel->_OnLoaded();   // this adds the model to the loaded models list and increments the ref count, so returning by value is safe.
    return dgnModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnModels::FindModel(DgnModelId modelId)
    {
    BeMutexHolder _v_v(m_mutex);
    auto it=m_models.find(modelId);
    return it!=m_models.end() ? it->second : NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnModelPtr DgnModels::CreateModel(DgnDbStatus* inStat, ECN::IECInstanceCR properties)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);

    DgnClassId classId(properties.GetClass().GetId().GetValue());
    auto handler = dgn_ModelHandler::Model::FindHandler(GetDgnDb(), classId);
    if (nullptr == handler)
        {
        BeAssert(false);
        stat = DgnDbStatus::MissingHandler;
        return nullptr;
        }

    return handler->_CreateNewModel(inStat, GetDgnDb(), properties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnModels::GetModel(DgnModelId modelId)
    {
    if (!modelId.IsValid())
        return nullptr;

    // since we can load models on more than one thread, we need to check that the model doesn't already exist
    // *with the lock held* before we load it. This avoids a race condition where an model is loaded on more than one thread.
    BeMutexHolder _v_v(m_mutex);

    DgnModelPtr dgnModel = FindModel(modelId);
    return dgnModel.IsValid() ? dgnModel : LoadDgnModel(modelId);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GeometricModel::Formatter::SetUnits(UnitDefinitionCR newMasterUnit, UnitDefinitionCR newSubUnit)
    {
    if (!newMasterUnit.IsValid() || !newSubUnit.IsValid() || !newMasterUnit.AreComparable(newSubUnit))
        return ERROR;

    // subunits must be smaller than master.passed in, validate that they are smaller than master
    int subunitRelationship  = newMasterUnit.CompareByScale(newSubUnit);
    if (0 < subunitRelationship)
        return ERROR;

    m_masterUnit = newMasterUnit;
    m_subUnit    = newSubUnit;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
ModelHandlerP dgn_ModelHandler::Model::FindHandler(DgnDb const& db, DgnClassId handlerId)
    {
    // quick check for a handler already known
    DgnDomain::Handler* handler = db.Domains().LookupHandler(handlerId);
    if (nullptr != handler)
        return handler->_ToModelHandler();

    // not there, check via base classes
    handler = db.Domains().FindHandler(handlerId, db.Domains().GetClassId(GetHandler()));
    return handler ? handler->_ToModelHandler() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Ramanujam.Raman   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlClassParams const& dgn_ModelHandler::Model::GetECSqlClassParams()
    {
    BeMutexHolder _v(m_mutex);

    if (!m_classParams.IsInitialized())
        m_classParams.Initialize(*this);

    return m_classParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ModelHandler::Model::_GetClassParams(ECSqlClassParamsR params)
    {  
    params.Add(DgnModel::prop_ECInstanceId(), ECSqlClassParams::StatementType::Insert);
    params.Add(DgnModel::prop_ParentModel(), ECSqlClassParams::StatementType::Insert);
    params.Add(DgnModel::prop_ModeledElement(), ECSqlClassParams::StatementType::Insert);
    params.Add(DgnModel::prop_IsPrivate(), ECSqlClassParams::StatementType::All);
    params.Add(DgnModel::prop_JsonProperties(), ECSqlClassParams::StatementType::All);
    params.Add(DgnModel::prop_IsTemplate(), ECSqlClassParams::StatementType::All);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnModel::CreateParams DgnModel::InitCreateParamsFromECInstance(DgnDbStatus* inStat, DgnDbR db, ECN::IECInstanceCR properties)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);

    DgnClassId classId(properties.GetClass().GetId().GetValue());
    ECN::ECValue v;
    bool isPrivate = false;
    if (ECN::ECObjectsStatus::Success == properties.GetValue(v, prop_IsPrivate()) && !v.IsNull())
        isPrivate = v.GetBoolean();

    DgnElementId modeledElementId;
    if (ECN::ECObjectsStatus::Success != properties.GetValue(v, prop_ModeledElement()) || v.IsNull())
        stat = DgnDbStatus::BadArg;
    else
        modeledElementId = DgnElementId((uint64_t) v.GetNavigationInfo().GetId<BeInt64Id>().GetValue());
    DgnModel::CreateParams params(db, classId, modeledElementId, isPrivate);
    return params;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus DgnModel::_SetProperty(Utf8CP name, ECN::ECValueCR value)
    {
    //// Common case: auto-handled properties
    //ECN::ECPropertyCP ecprop = GetElementClass()->GetPropertyP(name);
    //if ((nullptr != ecprop) && !IsCustomHandledProperty(*ecprop))
    //    {
    //    if (!isValidValue(*ecprop, value))
    //        return DgnDbStatus::BadArg;

    //    auto autoHandledProps = GetAutoHandledProperties();
    //    if (nullptr != autoHandledProps && ECN::ECObjectsStatus::Success == autoHandledProps->SetValue(name, value))
    //        {
    //        m_flags.m_autoHandledPropsDirty = true;
    //        return DgnDbStatus::Success;
    //        }
    //    return DgnDbStatus::BadArg;
    //    }

    if (0 == strcmp("Id", name) || 0 == strcmp(prop_ECInstanceId(), name))
        return DgnDbStatus::ReadOnly;

    if (0 == strcmp(prop_JsonProperties(), name))
        {
        if (!Json::Reader::Parse(value.GetUtf8CP(), m_jsonProperties))
            return DgnDbStatus::BadArg;

        _OnLoadedJsonProperties();
        return DgnDbStatus::Success;
        }

    if (0 == strcmp(prop_IsTemplate(), name))
        {
        if (!value.IsBoolean())
            return DgnDbStatus::BadArg;
        m_isTemplate = value.GetBoolean();
        return DgnDbStatus::Success;
        }
    return DgnDbStatus::NotFound;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus DgnModel::_SetProperties(ECN::IECInstanceCR properties)
    {
    for (auto prop : properties.GetClass().GetProperties(true))
        {
        Utf8StringCR propName = prop->GetName();

        // Skip special properties that were passed in CreateParams. Generally, these are set once and then read-only properties.
        if (propName.Equals(prop_ECInstanceId()) || propName.Equals(prop_ParentModel()) || propName.Equals(prop_ModeledElement()) || propName.Equals(prop_IsPrivate()))
            continue;

        ECN::ECValue value;
        if (ECN::ECObjectsStatus::Success != properties.GetValue(value, propName.c_str()))
            continue;

        if (!value.IsNull())
            {
            DgnDbStatus stat;
            if (DgnDbStatus::Success != (stat = _SetProperty(propName.c_str(), value)))
                {
                if (DgnDbStatus::ReadOnly == stat) // Not sure what to do when caller wants to 
                    {
                    BeAssert(false && "Attempt to set read-only property value.");
                    }
                else
                    {
                    BeAssert(false && "Failed to set property value. _SetProperties is probably missing a case.");
                    }
                return stat;
                }
            }
        }
    
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnModelPtr dgn_ModelHandler::Model::_CreateNewModel(DgnDbStatus* inStat, DgnDbR db, ECN::IECInstanceCR properties)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);
    auto params = DgnModel::InitCreateParamsFromECInstance(inStat, db, properties);
    if (!params.m_classId.IsValid())
        return nullptr;
    auto model = _CreateInstance(params);
    if (nullptr == model)
        {
        BeAssert(false && "when would a handler fail to construct an element?");
        return nullptr;
        }
    stat = model->_SetProperties(properties);
    return (DgnDbStatus::Success == stat)? model : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* Note: Only returns a valid range if the range index is loaded.
* @bsimethod                                    Keith.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d GeometricModel::_QueryElementsRange() const
    {
    auto rangeIndex = GetRangeIndex();
    if (nullptr == rangeIndex)
        return AxisAlignedBox3d();

    auto fbox = rangeIndex->GetExtents();

    // NB: FBox::ToRange3d() creates from two points, which will turn a null range into the largest range possible...
    // Even if it didn't, AxisAlignedBox3d(DRange3dCR) ctor does the same thing given a null range...
    if (fbox.IsNull())
        return AxisAlignedBox3d();

    return AxisAlignedBox3d(fbox.ToRange3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/11
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d GeometricModel3d::_QueryElementsRange() const
    {
    auto range = T_Super::_QueryElementsRange();   // if we have a range index already loaded, use it rather than querying
    if (range.IsValid()) // was it valid?
        return range;   // yes, we're done

    // NOTE: we can't use the persistent range index, because it has elements from all models, not just this one.
    Statement stmt(m_dgndb,
        "SELECT DGN_bbox_union("
            "DGN_placement_aabb("
                "DGN_placement("
                    "DGN_point(g.Origin_X,g.Origin_Y,g.Origin_Z),"
                    "DGN_angles(g.Yaw,g.Pitch,g.Roll),"
                    "DGN_bbox("
                        "g.BBoxLow_X,g.BBoxLow_Y,g.BBoxLow_Z,"
                        "g.BBoxHigh_X,g.BBoxHigh_Y,g.BBoxHigh_Z))))"
        " FROM " BIS_TABLE(BIS_CLASS_Element) " AS e," BIS_TABLE(BIS_CLASS_GeometricElement3d) " As g"
        " WHERE e.ModelId=? AND e.Id=g.ElementId AND g.Origin_X IS NOT NULL");

    stmt.BindId(1, GetModelId());
    auto rc = stmt.Step();
    if (rc!=BE_SQLITE_ROW)
        {
        BeAssert(false);
        return AxisAlignedBox3d();
        }

    int resultSize = stmt.GetColumnBytes(0); // can be 0 if no elements in model
    return (sizeof(AxisAlignedBox3d) == resultSize) ? *(AxisAlignedBox3d*) const_cast<void*>(stmt.GetValueBlob(0)) : AxisAlignedBox3d(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/11
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d GeometricModel2d::_QueryElementsRange() const
    {
    auto range = T_Super::_QueryElementsRange();   // if we have a range index already loaded, use it rather than querying
    if (range.IsValid()) // was it valid?
        return range;   // yes, we're done

    // NOTE: there is no persistent range index for 2d models (they're each in a separate coordinate space)
    Statement stmt(m_dgndb,
        "SELECT DGN_bbox_union("
            "DGN_placement_aabb("
                "DGN_placement("
                    "DGN_point(g.Origin_X,g.Origin_Y,0),"
                    "DGN_angles(g.Rotation,0,0),"
                    "DGN_bbox("
                        "g.BBoxLow_X,g.BBoxLow_Y,-1,"
                        "g.BBoxHigh_X,g.BBoxHigh_Y,1))))"
        " FROM " BIS_TABLE(BIS_CLASS_Element) " AS e," BIS_TABLE(BIS_CLASS_GeometricElement2d) " As g"
        " WHERE e.ModelId=? AND e.Id=g.ElementId AND g.Origin_X IS NOT NULL");

    stmt.BindId(1, GetModelId());
    auto rc = stmt.Step();
    if (rc!=BE_SQLITE_ROW)
        {
        BeAssert(false);
        return AxisAlignedBox3d();
        }

    int resultSize = stmt.GetColumnBytes(0); // can be 0 if no elements in model
    return (sizeof(AxisAlignedBox3d) == resultSize) ? *(AxisAlignedBox3d*) const_cast<void*>(stmt.GetValueBlob(0)) : AxisAlignedBox3d(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::CreateParams::RelocateToDestinationDb(DgnImportContext& importer)
    {
    m_classId = importer.RemapClassId(m_classId);
    m_modeledElementId.Invalidate(); // WIP: Need to remap!
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
static void logPerformance(StopWatch& stopWatch, Utf8CP description, ...)
    {
    stopWatch.Stop();
    const NativeLogging::SEVERITY severity = NativeLogging::LOG_INFO;
    NativeLogging::ILogger& logger = *NativeLogging::LoggingManager::GetLogger(L"DgnCore.Performance");

    if (logger.isSeverityEnabled(severity))
        {
        va_list args;
        va_start(args, description);
        Utf8String formattedDescription;
        formattedDescription.VSprintf(description, args);
        va_end(args);

        logger.messagev(severity, "%s|%.0f millisecs", formattedDescription.c_str(), stopWatch.GetElapsedSeconds() * 1000.0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::CreateParams DgnModel::GetCreateParamsForImport(DgnImportContext& importer, DgnElementCR destinationElementToModel) const
    {
    CreateParams params(importer.GetDestinationDb(), GetClassId(), DgnElementId());
    if (importer.IsBetweenDbs())
        params.RelocateToDestinationDb(importer);

    params.SetModeledElementId(destinationElementToModel.GetElementId());
    return params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnModel::Clone(DgnElementId newModeledElementId) const
    {
    if (!newModeledElementId.IsValid())
        return nullptr;

    if (GetModelHandler()._IsRestrictedAction(RestrictedAction::Clone))
        return nullptr;

    DgnModelPtr newModel = GetModelHandler().Create(DgnModel::CreateParams(m_dgndb, m_classId, newModeledElementId));
    newModel->_InitFrom(*this);
    return newModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnModel::_CloneForImport(DgnDbStatus* stat, DgnImportContext& importer, DgnElementCR destinationElementToModel) const
    {
    if (nullptr != stat)
        *stat = DgnDbStatus::Success;

    DgnModel::CreateParams params = GetCreateParamsForImport(importer, destinationElementToModel); // remaps classid
    params.SetModeledElementId(destinationElementToModel.GetElementId());

    DgnModelPtr model = GetModelHandler().Create(params);
    if (!model.IsValid())
        return nullptr;

    model->_InitFrom(*this);
    return model;
    }

/*---------------------------------------------------------------------------------**//**
* NB: We must import parents first, then children, so that parentids can be remapped without multiple passes.
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_ImportElementsFrom(DgnModelCR sourceModel, DgnImportContext& importer)
    {
    BeAssert(&GetDgnDb() == &importer.GetDestinationDb());
    BeAssert(&sourceModel.GetDgnDb() == &importer.GetSourceDb());

    bmap<DgnElementId, DgnElementId> needsParentFixup;

    Statement stmt(sourceModel.GetDgnDb(), "SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE ModelId=?");
    stmt.BindId(1, sourceModel.GetModelId());
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnElementCPtr el = sourceModel.GetDgnDb().Elements().GetElement(stmt.GetValueId<DgnElementId>(0));

        if (!_ShouldImportElement(*el))
            continue;

        DgnElementId waspid = el->GetParentId();

        DgnDbStatus status;
        DgnElementCPtr cc = el->Import(&status, *this, importer);
        if (!cc.IsValid() || (DgnDbStatus::Success != status))
            {
            // *** TBD: Record failure somehow
            BeDataAssert(false && "element import failure -- probably a duplicate code");
            continue;
            }

        if (waspid.IsValid() && !cc->GetParentId().IsValid())
            needsParentFixup[cc->GetElementId()] = waspid;
        }

    for (auto entry : needsParentFixup)
        {
        DgnElementPtr cc = GetDgnDb().Elements().GetForEdit<DgnElement>(entry.first);
        cc->SetParentId(importer.FindElementId(entry.second), cc->GetParentRelClassId());
        cc->Update();
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_ImportElementAspectsFrom(DgnModelCR sourceModel, DgnImportContext& importer)
    {
    // This base class implementation of _ImportElementAspectsFrom knows only the ElementAspect subclasses that are defined by the
    //  base Dgn schema. 
    
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getECRelColIds(int& sourceInstanceIdCol, int& targetInstanceIdCol, ECSqlStatementCR sstmt)
    {
    sourceInstanceIdCol = 0;
    targetInstanceIdCol = 0;
    bool haveSource=false;
    bool haveTarget=false;
    for (int i=0, count=sstmt.GetColumnCount(); i<count; ++i)
        {
        ECN::ECPropertyCP ecprop = sstmt.GetColumnInfo(i).GetProperty();
        if (ecprop->GetName().Equals("SourceECInstanceId"))
            {
            sourceInstanceIdCol = i;
            haveSource = true;
            }
        else if (ecprop->GetName().Equals("TargetECInstanceId"))
            {
            targetInstanceIdCol = i;
            haveTarget = true;
            }

        if (haveSource && haveTarget)
            return BSISUCCESS;
        }

    BeAssert(false && "This does not seem to be a statement that queries the SourceECInstanceId and TargetECInstanceId columns of an ECRelationship");
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::ImportLinkTableECRelationshipsFrom(DgnDbR destDb, DgnModelCR sourceModel, DgnImportContext& importer, Utf8CP relschema, Utf8CP relname)
    {
    auto sstmt = sourceModel.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString(
        "SELECT rel.*, rel.SourceECClassId,rel.TargetECClassId FROM %s.%s rel, " BIS_SCHEMA(BIS_CLASS_Element) " source, " BIS_SCHEMA(BIS_CLASS_Element) " target"
        " WHERE rel.SourceECInstanceId=source.ECInstanceId AND rel.TargetECInstanceId=target.ECInstanceId AND source.Model.Id=? AND target.Model.Id=?",
        relschema, relname).c_str());

    if (!sstmt.IsValid())   // the statement will fail to prepare if the ecclass is not found, and that can only be because the necessary domain/schema was not imported
        return DgnDbStatus::MissingDomain;

    sstmt->BindId(1, sourceModel.GetModelId());
    sstmt->BindId(2, sourceModel.GetModelId());

    int sourceInstanceIdCol, targetInstanceIdCol;
    getECRelColIds(sourceInstanceIdCol, targetInstanceIdCol, *sstmt);

    ECInstanceECSqlSelectAdapter sourceReader(*sstmt);
    while (BE_SQLITE_ROW == sstmt->Step())
        {
        DgnElementId remappedSrcId = importer.FindElementId(sstmt->GetValueId<DgnElementId>(sourceInstanceIdCol));
        DgnElementId remappedDstId = importer.FindElementId(sstmt->GetValueId<DgnElementId>(targetInstanceIdCol));
        if (remappedSrcId.IsValid() && remappedDstId.IsValid()) // import rel ONLY if both source or target element were previously imported by the caller.
            {
            ECN::IECRelationshipInstancePtr relinst(dynamic_cast<ECN::IECRelationshipInstanceP>(sourceReader.GetInstance().get()));

            ECN::ECClassCR srcClass = relinst->GetClass();
            ECClassCP actualDstClass = destDb.Schemas().GetClass(srcClass.GetSchema().GetName().c_str(), srcClass.GetName().c_str());
            if (nullptr == actualDstClass)
                {
                // the lookup will fail to only if the ecclass is not found, and that can only be because the necessary domain/schema was not imported
                return DgnDbStatus::MissingDomain;
                }
            ECN::ECRelationshipClassCP actualDstRelClass = actualDstClass->GetRelationshipClassCP();
            if (nullptr == actualDstRelClass)
                {
                BeAssert(false);
                return DgnDbStatus::MissingDomain;
                }

            EC::ECInstanceKey ekey;
            destDb.InsertLinkTableRelationship(ekey, *actualDstRelClass, remappedSrcId, remappedDstId, relinst.get());
            }
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_ImportLinkTableECRelationshipsFrom(DgnModelCR sourceModel, DgnImportContext& importer)
    {
    // Copy ECRelationships where source and target are both in this model, and where the relationship is implemented as a link table.
    // Note: this requires domain-specific knowledge of what ECRelationships exist.

    // ElementGeomUsesParts are created automatically as a side effect of inserting GeometricElements 

    StopWatch timer(true);
    ImportLinkTableECRelationshipsFrom(GetDgnDb(), sourceModel, importer, BIS_ECSCHEMA_NAME, BIS_REL_ElementGroupsMembers);
    logPerformance(timer, "Import ECRelationships %s", BIS_REL_ElementGroupsMembers);
    timer.Start();
    ImportLinkTableECRelationshipsFrom(GetDgnDb(), sourceModel, importer, BIS_ECSCHEMA_NAME, BIS_REL_ElementDrivesElement);
    logPerformance(timer, "Import ECRelationships %s", BIS_REL_ElementDrivesElement);

#ifdef WIP_VIEW_DEFINITION
    BIS_TABLE(BIS_REL_CategorySelectorsReferToCategories)
    BIS_TABLE(BIS_REL_ModelSelectorsReferToModels)
#endif

    // *** WIP_IMPORT *** ElementHasLinks -- should we deep-copy links?

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_ImportContentsFrom(DgnModelCR sourceModel, DgnImportContext& importer)
    {
    StopWatch totalTimer(true);
    BeAssert(&GetDgnDb() == &importer.GetDestinationDb());
    BeAssert(&sourceModel.GetDgnDb() == &importer.GetSourceDb());

    DgnDbStatus status;
    StopWatch timer(true);
    if (DgnDbStatus::Success != (status = _ImportElementsFrom(sourceModel, importer)))
        return status;
    logPerformance(timer, "Import elements time");

    timer.Start();
    if (DgnDbStatus::Success != (status = _ImportElementAspectsFrom(sourceModel, importer)))
        return status;
    logPerformance(timer, "Import element aspects time");

    timer.Start();
    if (DgnDbStatus::Success != (status = _ImportLinkTableECRelationshipsFrom(sourceModel, importer)))
        return status;
    logPerformance(timer, "Import ECRelationships time");

    logPerformance(totalTimer, "Total contents import time");

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnModel::ImportModel(DgnDbStatus* statIn, DgnModelCR sourceModel, DgnImportContext& importer, DgnElementCR destinationElementToModel)
    {
    StopWatch totalTimer(true);

    DgnDbStatus _stat;
    DgnDbStatus& stat = (nullptr != statIn)? *statIn: _stat;

    BeAssert(&sourceModel.GetDgnDb() == &importer.GetSourceDb());

    DgnModelPtr newModel = sourceModel._CloneForImport(&stat, importer, destinationElementToModel);
    if (!newModel.IsValid())
        return nullptr;

    if ((stat = newModel->Insert()) != DgnDbStatus::Success)
        return nullptr;

    importer.AddModelId(sourceModel.GetModelId(), newModel->GetModelId());

    if ((stat = newModel->_ImportContentsFrom(sourceModel, importer)) != DgnDbStatus::Success)
        return nullptr;

    stat = DgnDbStatus::Success;
    logPerformance(totalTimer, "Total import time");
    return newModel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
DgnModelPtr DgnModel::CopyModel(DgnModelCR model, DgnElementId newModeledElementId)
    {
    DgnDbR db = model.GetDgnDb();

    DgnModelPtr model2 = model.Clone(newModeledElementId);
    if (DgnDbStatus::Success != model2->Insert())   
        return nullptr;

    DgnImportContext nopimport(db, db);
    if (DgnDbStatus::Success != model2->_ImportContentsFrom(model, nopimport))
        return nullptr;

    return model2;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DictionaryModel::_CloneForImport(DgnDbStatus* stat, DgnImportContext& importer, DgnElementCR destinationElementToModel) const
    {
    if (nullptr != stat)
        *stat = DgnDbStatus::WrongModel;

    BeAssert(false && "The dictionary model cannot be cloned");
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t DgnModel::RestrictedAction::Parse(Utf8CP name)
    {
    struct Pair {Utf8CP name; uint64_t value;};
    static const Pair s_pairs[] =
        {
            {"insertelement", InsertElement},
            {"updateelement", UpdateElement},
            {"deleteelement", DeleteElement},
            {"clone", Clone },
        };

    for (auto const& pair : s_pairs)
        {
        if (0 == BeStringUtilities::Stricmp(pair.name, name))
            return pair.value;
        }

    return T_Super::Parse(name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementIterator DgnModel::MakeIterator(Utf8CP whereClause, Utf8CP orderByClause) const
    {
    Utf8String where("WHERE Model.Id=?");

    if (whereClause)
        {
        Utf8String userWhere(whereClause);
        userWhere.Trim();
        if (0 == strncmp(userWhere.c_str(), "WHERE ", 6))
            userWhere.erase(0,6);

        where.append(" AND ");
        where.append(userWhere);
        }

    ElementIterator iterator = m_dgndb.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_Element), where.c_str(), orderByClause);
    iterator.GetStatement()->BindId(1, GetModelId());

    return iterator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::InitLastElementModifiedTime()
    {
    if (0 != m_lastModifiedTime.load())
        return;

    // NB: We're using ECSql because ECDb persists datetime as floating-point julian day value...
    constexpr Utf8CP ecsql = "SELECT MAX(LastMod) FROM " BIS_SCHEMA(BIS_CLASS_Element) " WHERE Model.Id=?";
    auto stmt = GetDgnDb().GetPreparedECSqlStatement(ecsql);
    stmt->BindId(1, GetModelId());
    if (BE_SQLITE_ROW == stmt->Step() && !stmt->IsValueNull(0))
        {
        DateTime dt = stmt->GetValueDateTime(0);
        int64_t unixMillis;
        if (SUCCESS == dt.ToUnixMilliseconds(unixMillis))
            {
            m_lastModifiedTime.store(static_cast<uint64_t>(unixMillis));
            }
        else
            {
            BeAssert(false);
            UpdateLastElementModifiedTime();
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::UpdateLastElementModifiedTime()
    {
    m_lastModifiedTime.store(BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t GeometricModel::GetLastElementModifiedTime() const
    {
    return m_lastModifiedTime.load();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
Cesium::RootPtr GeometricModel::_CreateCesiumTileTree(Cesium::OutputR)
    {
    return nullptr;
    }

