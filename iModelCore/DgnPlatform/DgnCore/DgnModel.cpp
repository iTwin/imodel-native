/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnModel.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define MODEL_PROP_ECInstanceId "ECInstanceId"
#define MODEL_PROP_ModeledElementId "ModeledElementId"
#define MODEL_PROP_Visibility "Visibility"
#define MODEL_PROP_Properties "Properties"
#define MODEL_PROP_IsTemplate "IsTemplate"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModels::QueryModelById(Model* out, DgnModelId id) const
    {
    Statement stmt(m_dgndb, "SELECT ECClassId,Visibility,ModeledElementId,IsTemplate FROM " BIS_TABLE(BIS_CLASS_Model) " WHERE Id=?");
    stmt.BindId(1, id);

    if (BE_SQLITE_ROW != stmt.Step())
        return ERROR;

    if (out) // this can be null to just test for the existence of a model by id
        {
        out->m_id = id;
        out->m_classId = stmt.GetValueId<DgnClassId>(0);
        out->m_inGuiList = TO_BOOL(stmt.GetValueInt(1));
        out->m_modeledElementId = stmt.GetValueId<DgnElementId>(2);
        out->m_isTemplate = TO_BOOL(stmt.GetValueInt(3));
        }

    return SUCCESS;
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
    BeDbMutexHolder _v_v(m_mutex);
    m_models.Insert(model.GetModelId(), &model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModels::DropLoadedModel(DgnModelR model)
    {
    model.m_persistent = false;
    BeDbMutexHolder _v_v(m_mutex);
    m_models.erase(model.GetModelId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModels::Empty()
    {
    m_dgndb.Elements().Destroy(); // Has to be called before models are released.
    m_models.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/11
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnModels::Iterator::QueryCount() const
    {
    Utf8String sqlString = "SELECT count(*) FROM " BIS_TABLE(BIS_CLASS_Model);
    bool hasWhere = false;
    if (ModelIterate::Gui == m_itType)
        {
        sqlString += " WHERE (0 <> Visibility)";
        hasWhere = true;
        }

    sqlString = MakeSqlString(sqlString.c_str(), hasWhere);

    Statement sql(*m_db, sqlString.c_str());

    return (BE_SQLITE_ROW != sql.Step()) ? 0 : sql.GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModels::Iterator::const_iterator DgnModels::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = "SELECT Id,Visibility,ECClassId,ModeledElementId,IsTemplate FROM " BIS_TABLE(BIS_CLASS_Model);
        bool hasWhere = false;
        if (ModelIterate::Gui == m_itType)
            {
            sqlString += " WHERE (0 <> Visibility)";
            hasWhere = true;
            }

        sqlString = MakeSqlString(sqlString.c_str(), hasWhere);

        m_db->GetCachedStatement(m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

DgnModelId   DgnModels::Iterator::Entry::GetModelId() const {Verify(); return m_sql->GetValueId<DgnModelId>(0);}
bool         DgnModels::Iterator::Entry::GetInGuiList() const {Verify(); return (0 != m_sql->GetValueInt(1));}
DgnClassId   DgnModels::Iterator::Entry::GetClassId() const {Verify(); return m_sql->GetValueId<DgnClassId>(2);}
DgnElementId DgnModels::Iterator::Entry::GetModeledElementId() const {Verify(); return m_sql->GetValueId<DgnElementId>(3);}
bool         DgnModels::Iterator::Entry::GetIsTemplate() const {Verify(); return (0 != m_sql->GetValueInt(4));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlClassInfo const& DgnModels::FindClassInfo(DgnModelR model)
    {
    DgnClassId classId = model.GetClassId();
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
DgnModel::DgnModel(CreateParams const& params) : m_dgndb(params.m_dgndb), m_classId(params.m_classId), m_modeledElementId(params.m_modeledElementId), m_inGuiList(params.m_inGuiList),
    m_isTemplate(params.m_isTemplate), m_persistent(false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnModel::GetName() const
    {
    // WIP: keep this method around to avoid having to change too much source code.  Use the CodeValue of the modeled element as this model's name.
    DgnElementCPtr modeledElement = GetDgnDb().Elements().GetElement(GetModeledElementId());
    BeAssert(modeledElement.IsValid());
    return modeledElement.IsValid() ? modeledElement->GetCode().GetValue() : Utf8String();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::AddAppData(AppData::Key const& key, AppData* obj)
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
    return 0==m_appData.erase(&key) ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::AppData* DgnModel::FindAppData(AppData::Key const& key) const
    {
    auto entry = m_appData.find(&key);
    return entry==m_appData.end() ? nullptr : entry->second.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T> void DgnModel::CallAppData(T const& caller) const
    {
    for (auto entry=m_appData.begin(); entry!=m_appData.end(); )
        {
        if (DgnModel::AppData::DropMe::Yes == caller(*entry->second, *this))
            entry = m_appData.erase(entry);
        else
            ++entry;
        }
    }    

/*---------------------------------------------------------------------------------**//**
* Destructor for DgnModel. Free all memory allocated to this DgnModel.
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::~DgnModel()
    {
    m_appData.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricModel2d::_OnInsertElement(DgnElementR element)
    {
    DgnDbStatus status = T_Super::_OnInsertElement(element);
    if (DgnDbStatus::Success != status)
        return status;

    // if it is a geometric element, it must be a 2d element.
    return element.IsGeometricElement() && element.Is3d() ? DgnDbStatus::Mismatch2d3d : DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SectionDrawingModel::_OnInsertElement(DgnElementR el)
    {
    auto stat = T_Super::_OnInsertElement(el);
    if (DgnDbStatus::Success == stat && el.IsGeometricElement() && !el.IsAnnotationElement2d() && !el.IsDrawingGraphic())
        stat = DgnDbStatus::WrongModel;

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricModel3d::_OnInsertElement(DgnElementR element)
    {
    auto status = T_Super::_OnInsertElement(element);
    if (DgnDbStatus::Success == status && element.IsGeometricElement() && !element.Is3d())
        status = DgnDbStatus::Mismatch2d3d;

    return status;
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
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr PhysicalModel::Create(DgnDbR db, DgnElementId modeledElementId)
    {
    ModelHandlerR handler = dgn_ModelHandler::Physical::GetHandler();
    DgnClassId classId = db.Domains().GetClassId(handler);
    if (!classId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, modeledElementId));
    if (!model.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return model->ToPhysicalModelP();
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
PhysicalModelPtr PhysicalModel::Create(PhysicalTemplateCR modeledElement)
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
    if (!model.IsValid())
        return nullptr;

    return (DgnDbStatus::Success != model->Insert()) ? nullptr : model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr PhysicalModel::CreateAndInsert(PhysicalElementCR modeledElement)
    {
    PhysicalModelPtr model = Create(modeledElement);
    if (!model.IsValid())
        return nullptr;

    return (DgnDbStatus::Success != model->Insert()) ? nullptr : model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr PhysicalModel::CreateAndInsert(PhysicalTemplateCR modeledElement)
    {
    PhysicalModelPtr model = Create(modeledElement);
    if (!model.IsValid())
        return nullptr;

    return (DgnDbStatus::Success != model->Insert()) ? nullptr : model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus InformationModel::_OnInsertElement(DgnElementR element)
    {
    return element.IsInformationContentElement() ? T_Super::_OnInsertElement(element) : DgnDbStatus::WrongModel;
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
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SessionModel::_OnInsertElement(DgnElementR element)
    {
    // SessionModel can contain *only* Session elements
    // WIP: waiting for Session element to be introduced!
    return T_Super::_OnInsertElement(element);
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

    if (!classId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, modeledElement.GetElementId()));
    if (!model.IsValid())
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
    if (!model.IsValid())
        return nullptr;

    return (DgnDbStatus::Success != model->Insert()) ? nullptr : model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DrawingModel::_OnInsert()
    {
    if (!GetModeledElementId().IsValid() || !GetDgnDb().Elements().Get<Drawing>(GetModeledElementId()).IsValid())
        {
        BeAssert(false && "A DrawingModel should be modeling a Drawing element");
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

    if (!classId.IsValid() || !drawing.GetElementId().IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, drawing.GetElementId()));
    if (!model.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return dynamic_cast<DrawingModelP>(model.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SheetModel::_OnInsert()
    {
    if (!GetDgnDb().Elements().Get<Sheet>(GetModeledElementId()).IsValid())
        {
        BeAssert(false && "A SheetModel should be modeling a Sheet element");
        return DgnDbStatus::BadElement;
        }

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
SheetModelPtr SheetModel::Create(SheetCR sheet)
    {
    DgnDbR db = sheet.GetDgnDb();
    ModelHandlerR handler = dgn_ModelHandler::Sheet::GetHandler();
    DgnClassId classId = db.Domains().GetClassId(handler);

    if (!classId.IsValid() || !sheet.GetElementId().IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, sheet.GetElementId()));
    if (!model.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return dynamic_cast<SheetModelP>(model.get());
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
    int propsIndex = params.GetSelectIndex(MODEL_PROP_Properties);
    if (!statement.IsValueNull(propsIndex))
        {
        Json::Value  propsJson(Json::objectValue);
        if (!Json::Reader::Parse(statement.GetValueText(propsIndex), propsJson))
            {
            BeAssert(false);
            return DgnDbStatus::ReadError;
            }

        _ReadJsonProperties(propsJson);
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_BindWriteParams(BeSQLite::EC::ECSqlStatement& statement, ForInsert forInsert)
    {
    if (forInsert == ForInsert::Yes)
        statement.BindId(statement.GetParameterIndex(MODEL_PROP_ECInstanceId), m_modelId);


    if (!m_modeledElementId.IsValid())
        {
        BeAssert(false);
        return ;
        }

    statement.BindId(statement.GetParameterIndex(MODEL_PROP_ModeledElementId), m_modeledElementId);
    statement.BindBoolean(statement.GetParameterIndex(MODEL_PROP_Visibility), m_inGuiList);
    statement.BindBoolean(statement.GetParameterIndex(MODEL_PROP_IsTemplate), m_isTemplate);

    Json::Value propJson(Json::objectValue);
    _WriteJsonProperties(propJson);
    if (!propJson.isNull())
        {
        Utf8String val = Json::FastWriter::ToString(propJson);
        statement.BindText(statement.GetParameterIndex(MODEL_PROP_Properties), val.c_str(), IECSqlBinder::MakeCopy::Yes);
        }
    else
        statement.BindNull(statement.GetParameterIndex(MODEL_PROP_Properties));
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
    if (GetModelHandler()._IsRestrictedAction(RestrictedAction::Update))
        return DgnDbStatus::MissingHandler;

    for (auto entry=m_appData.begin(); entry!=m_appData.end(); ++entry)
        {
        DgnDbStatus stat = entry->second->_OnUpdate(*this);
        if (DgnDbStatus::Success != stat)
            return stat;
        }

    // Ensure code is reserved and lock acquired
    return GetDgnDb().BriefcaseManager().OnModelUpdate(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricModel3d::_FillRangeIndex()
    {
    if (nullptr != m_rangeIndex)
        return DgnDbStatus::Success;

    m_rangeIndex.reset(new RangeIndex::Tree(true, 20));
    auto stmt = m_dgndb.GetPreparedECSqlStatement("SELECT ECInstanceId,CategoryId,Origin,Yaw,Pitch,Roll,BBoxLow,BBoxHigh FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " WHERE ModelId=?");
    stmt->BindId(1, GetModelId());
    while (BE_SQLITE_ROW == stmt->Step())
        {
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
    if (nullptr != m_rangeIndex)
        return DgnDbStatus::Success;

    m_rangeIndex.reset(new RangeIndex::Tree(false, 20));

    auto stmt = m_dgndb.GetPreparedECSqlStatement("SELECT ECInstanceId,CategoryId,Origin,Rotation,BBoxLow,BBoxHigh FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement2d) " WHERE ModelId=?");
    stmt->BindId(1, GetModelId());
    while (BE_SQLITE_ROW == stmt->Step())
        {
        DPoint2d low = stmt->GetValuePoint2d(4);
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
    if (nullptr == m_rangeIndex)
        return;

    GeometrySourceCP geom = element.ToGeometrySource();
    if (nullptr != geom)
        m_rangeIndex->AddElement(*geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::RemoveFromRangeIndex(DgnElementCR element)
    {
    if (nullptr==m_rangeIndex)
        return;

    GeometrySourceCP geom = element.ToGeometrySource();
    if (nullptr != geom && geom->HasGeometry())
        m_rangeIndex->RemoveElement(element.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::UpdateRangeIndex(DgnElementCR modified, DgnElementCR original)
    {
    if (nullptr==m_rangeIndex)
        return;

    GeometrySourceCP origGeom = original.ToGeometrySource();
    if (nullptr == origGeom)
        return;

    GeometrySourceCP newGeom = modified.ToGeometrySource();
    if (nullptr == newGeom)
        return;

    AxisAlignedBox3d origBox = origGeom->HasGeometry() ? origGeom->CalculateRange3d() : AxisAlignedBox3d();
    AxisAlignedBox3d newBox  = newGeom->HasGeometry() ? newGeom->CalculateRange3d() : AxisAlignedBox3d();

    if (origBox.IsEqual(newBox)) // many changes don't affect range
        return;

    auto id = original.GetElementId();
    m_rangeIndex->RemoveElement(id);
    m_rangeIndex->AddEntry(RangeIndex::Entry(newBox, id, origGeom->GetCategoryId()));
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
    if (GetModelHandler()._IsRestrictedAction(RestrictedAction::Delete))
        return DgnDbStatus::MissingHandler;

    DgnDbStatus stat = GetDgnDb().BriefcaseManager().OnModelDelete(*this);
    if (DgnDbStatus::Success != stat)
        return stat;

    for (auto appdata : m_appData)
        appdata.second->_OnDelete(*this);

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

    // delete all views which use this model as their base model
    stat = DeleteAllViews();
    if (DgnDbStatus::Success != stat)
        return stat;

    BeAssert(GetRefCount() > 1);
    m_dgndb.Models().DropLoadedModel(*this);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::DeleteAllViews()
    {
    if (Is3dModel())
        return ModelSelector::OnModelDelete(GetDgnDb(), GetModelId());
     
    ViewDefinition2d::OnModelDelete(GetDgnDb(), GetModelId());
    return DgnDbStatus::Success;
    }

struct DeletedCaller {DgnModel::AppData::DropMe operator()(DgnModel::AppData& handler, DgnModelCR model) const {return handler._OnDeleted(model);}};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnDeleted()
    {
    CallAppData(DeletedCaller());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void getElementsThatPointToModel(bset<DgnElementId>& dependents, DgnModelCR model)
    {
    BeSQLite::EC::CachedECSqlStatementPtr stmt;
    if (model.Is2dModel())
        stmt = model.GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_BaseModelForView2d) " WHERE SourceECInstanceId = ?");
    else
        stmt = model.GetDgnDb().GetPreparedECSqlStatement("SELECT SourceECInstanceId As ModelSelectorId, SourceECClassId ModelSelectorClassId FROM " BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels) " WHERE TargetECInstanceId = ?");

    stmt->BindId(1, model.GetModelId());
    while (BE_SQLITE_ROW == stmt->Step())
        dependents.insert(stmt->GetValueId<DgnElementId>(0));
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

            // and we must delete all of its views
            bset<DgnElementId> dependents;
            getElementsThatPointToModel(dependents, *this);
            for (auto id : dependents)
                {
                auto dependent = GetDgnDb().Elements().GetElement(id);
                auto stat = dependent->PopulateRequest(req, BeSQLite::DbOpcode::Delete);
                if (RepositoryStatus::Success != stat)
                    return stat;
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

    if (GetModelHandler()._IsRestrictedAction(RestrictedAction::Insert))
        return DgnDbStatus::MissingHandler;

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
        BeAssert(false && "A DgnModel must be modeling a DgnElement (that is above it in the hiearchy)");
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
    m_inGuiList = other.m_inGuiList;
    m_isTemplate = other.m_isTemplate;

    Json::Value otherProperties;
    other._WriteJsonProperties(otherProperties);
    _ReadJsonProperties(otherProperties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::_WriteJsonProperties(Json::Value& val) const 
    {
    T_Super::_WriteJsonProperties(val);
    if (val.isNull())
        val = Json::objectValue;
    m_displayInfo.ToJson(val["DisplayInfo"]); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::_ReadJsonProperties(Json::Value const& val) 
    {
    T_Super::_ReadJsonProperties(val);

    BeAssert(val.isMember("DisplayInfo"));
    m_displayInfo.FromJson(val["DisplayInfo"]);
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
    DgnModels::Model model;
    if (SUCCESS != QueryModelById(&model, modelId))
        return nullptr;

    // make sure the class derives from Model (has a handler)
    ModelHandlerP handler = dgn_ModelHandler::Model::FindHandler(m_dgndb, model.GetClassId());
    if (nullptr == handler)
        return nullptr;

    DgnModel::CreateParams params(m_dgndb, model.GetClassId(), model.GetModeledElementId(), model.GetInGuiList());
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
    BeDbMutexHolder _v_v(m_mutex);
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

    DgnModelPtr dgnModel = FindModel(modelId);
    return dgnModel.IsValid() ? dgnModel : LoadDgnModel(modelId);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  2/2016
//----------------------------------------------------------------------------------------
void DgnModels::DropGraphicsForViewport(DgnViewportCR viewport)
    {
    for (auto iter : m_models)
        {
        if (iter.second.IsValid())
            iter.second->_DropGraphicsForViewport(viewport);
        }        
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GeometricModel::DisplayInfo::SetUnits(UnitDefinitionCR newMasterUnit, UnitDefinitionCR newSubUnit)
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
    if (!m_classParams.IsInitialized())
        m_classParams.Initialize(*this);

    return m_classParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ModelHandler::Model::_GetClassParams(ECSqlClassParamsR params)
    {   
    params.Add(MODEL_PROP_ECInstanceId, ECSqlClassParams::StatementType::Insert);
    params.Add(MODEL_PROP_ModeledElementId, ECSqlClassParams::StatementType::InsertUpdate);
    params.Add(MODEL_PROP_Visibility, ECSqlClassParams::StatementType::InsertUpdate);
    params.Add(MODEL_PROP_Properties, ECSqlClassParams::StatementType::All);
    params.Add(MODEL_PROP_IsTemplate, ECSqlClassParams::StatementType::All);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnModel::CreateParams DgnModel::InitCreateParamsFromECInstance(DgnDbStatus* inStat, DgnDbR db, ECN::IECInstanceCR properties)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);

    DgnClassId classId(properties.GetClass().GetId().GetValue());
    ECN::ECValue v;
    bool inGuiList = true;
    if (ECN::ECObjectsStatus::Success == properties.GetValue(v, MODEL_PROP_Visibility) && !v.IsNull())
        inGuiList = v.GetInteger() == 1;

    DgnElementId modeledElementId;
    if (ECN::ECObjectsStatus::Success != properties.GetValue(v, MODEL_PROP_ModeledElementId) || v.IsNull())
        stat = DgnDbStatus::BadArg;
    else
        modeledElementId = DgnElementId((uint64_t) v.GetLong());
    DgnModel::CreateParams params(db, classId, modeledElementId, inGuiList);
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

    if (0 == strcmp("Id", name) || 0 == strcmp(MODEL_PROP_ECInstanceId, name))
        {
        return DgnDbStatus::ReadOnly;
        }
    if (0 == strcmp(MODEL_PROP_Properties, name))
        {
        Json::Value  propsJson(Json::objectValue);
        if (!Json::Reader::Parse(value.GetUtf8CP(), propsJson))
            return DgnDbStatus::BadArg;
        _ReadJsonProperties(propsJson);
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
        if (propName.Equals(MODEL_PROP_ECInstanceId) || propName.Equals(MODEL_PROP_ModeledElementId) || propName.Equals(MODEL_PROP_Visibility))
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
* @bsimethod                                    Keith.Bentley                   11/11
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d GeometricModel3d::_QueryModelRange() const
    {
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
        " WHERE e.ModelId=? AND e.Id=g.ElementId");

    stmt.BindId(1, GetModelId());
    auto rc = stmt.Step();
    if (rc!=BE_SQLITE_ROW)
        {
        BeAssert(false);
        return AxisAlignedBox3d();
        }

    int resultSize = stmt.GetColumnBytes(0); // can be 0 if no elements in model
    return (sizeof(AxisAlignedBox3d) == resultSize) ? *(AxisAlignedBox3d*) stmt.GetValueBlob(0) : AxisAlignedBox3d(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/11
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d GeometricModel2d::_QueryModelRange() const
    {
    Statement stmt(m_dgndb,
        "SELECT DGN_bbox_union("
            "DGN_placement_aabb("
                "DGN_placement("
                    "DGN_point(g.Origin_X,g.Origin_Y,0),"
                    "DGN_angles(g.Rotation,0,0),"
                    "DGN_bbox("
                        "g.BBoxLow_X,g.BBoxLow_Y,0,"
                        "g.BBoxHigh_X,g.BBoxHigh_Y,0))))"
        " FROM " BIS_TABLE(BIS_CLASS_Element) " AS e," BIS_TABLE(BIS_CLASS_GeometricElement2d) " As g"
        " WHERE e.ModelId=? AND e.Id=g.ElementId");

    stmt.BindId(1, GetModelId());
    auto rc = stmt.Step();
    if (rc!=BE_SQLITE_ROW)
        {
        BeAssert(false);
        return AxisAlignedBox3d();
        }

    int resultSize = stmt.GetColumnBytes(0); // can be 0 if no elements in model
    return (sizeof(AxisAlignedBox3d) == resultSize) ? *(AxisAlignedBox3d*) stmt.GetValueBlob(0) : AxisAlignedBox3d(); 
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
static void LogPerformance(StopWatch& stopWatch, Utf8CP description, ...)
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
        cc->SetParentId(importer.FindElementId(entry.second));
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
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void appendToColsLists(Utf8StringR colsList, Utf8StringR placeholderList, Utf8CP colname)
    {
    if (nullptr == colname)
        return;

    if (!colsList.empty())
        {
        colsList.append(",");
        placeholderList.append(",");
        }

    colsList.append("[").append(colname).append("]");
    placeholderList.append("?");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus importECRelationshipsFrom(DgnDbR destDb, DgnModelCR sourceModel, DgnImportContext& importer, Utf8CP relname, Utf8CP sourcecol, Utf8CP targetcol, Utf8CP classcol = nullptr, bvector<Utf8CP> const& othercols = bvector<Utf8CP>())
    {
    Utf8String colsList, placeholderList;
    appendToColsLists(colsList, placeholderList, sourcecol);        // [0] sourcecol
    appendToColsLists(colsList, placeholderList, targetcol);        // [1] targetcolo
    appendToColsLists(colsList, placeholderList, classcol);         // [2] classcol  (optional)
    for (auto othercolname : othercols)
        appendToColsLists(colsList, placeholderList, othercolname);

    Utf8String selectList;
    bvector<Utf8String> cols;
    BeStringUtilities::Split(colsList.c_str(), ",", cols);
    for (Utf8String col : cols)
        {
        if (!selectList.empty())
            selectList.append(", ");
        selectList.append("rel.").append(col);
        }
    Statement sstmt(sourceModel.GetDgnDb(), Utf8PrintfString(
        "SELECT %s FROM %s rel, " BIS_TABLE(BIS_CLASS_Element) " source, " BIS_TABLE(BIS_CLASS_Element) " target WHERE rel.%s=source.Id AND rel.%s=target.Id AND source.ModelId=? AND target.ModelId=?",
        selectList.c_str(), relname, sourcecol, targetcol).c_str());

    sstmt.BindId(1, sourceModel.GetModelId());
    sstmt.BindId(2, sourceModel.GetModelId());

    Statement istmt(destDb, Utf8PrintfString(
        "INSERT INTO %s (%s) VALUES(%s)", relname, colsList.c_str(), placeholderList.c_str()).c_str());

    StopWatch timer(true);
    DbResult stepResult = sstmt.Step();
    LogPerformance(timer, "Statement.Step for %s (ModelId=%d)", sstmt.GetSql(), sourceModel.GetModelId().GetValue());
    while (BE_SQLITE_ROW == stepResult)
        {
        istmt.Reset();
        istmt.ClearBindings();

        DgnElementId remappedSrcId;
        DgnElementId remappedDstId;

        int icol = 0;
        istmt.BindId(icol+1, (remappedSrcId = importer.FindElementId(sstmt.GetValueId<DgnElementId>(icol)))); // [0] sourcecol
        ++icol;
        istmt.BindId(icol+1, (remappedDstId = importer.FindElementId(sstmt.GetValueId<DgnElementId>(icol)))); // [1] targetcol
        ++icol;

        if (remappedSrcId.IsValid() && remappedDstId.IsValid())
            {
            if (nullptr != classcol)
                {
                istmt.BindId(icol+1, importer.RemapClassId(sstmt.GetValueId<DgnClassId>(icol))); // [2] classcol (optional)
                ++icol;
                }

            for (size_t iothercol=0; iothercol < (int)othercols.size(); ++iothercol)
                {
                istmt.BindText(icol+1, sstmt.GetValueText(icol), Statement::MakeCopy::No);
                ++icol;
                }

            if (BE_SQLITE_DONE != istmt.Step())
                {
                // *** TBD: Report error somehow
                }
            }

        timer.Start();
        stepResult = sstmt.Step();
        LogPerformance(timer, "Statement.Step for %s", sstmt.GetSql());

        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_ImportECRelationshipsFrom(DgnModelCR sourceModel, DgnImportContext& importer)
    {
    // Copy ECRelationships where source and target are both in this model, and where the relationship is implemented as a link table.
    // Note: this requires domain-specific knowledge of what ECRelationships exist.

    // ElementGeomUsesParts are created automatically as a side effect of inserting GeometricElements 

    StopWatch timer(true);
    importECRelationshipsFrom(GetDgnDb(), sourceModel, importer, BIS_TABLE(BIS_REL_ElementGroupsMembers), "GroupId", "MemberId", nullptr, {"MemberPriority"});
    LogPerformance(timer, "Import ECRelationships %s", BIS_REL_ElementGroupsMembers);
    timer.Start();
    importECRelationshipsFrom(GetDgnDb(), sourceModel, importer, BIS_TABLE(BIS_REL_ElementDrivesElement), "SourceECInstanceId", "TargetECInstanceId", "ECClassId", {"Status", "Priority"});
    LogPerformance(timer, "Import ECRelationships %s", BIS_REL_ElementDrivesElement);

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
    LogPerformance(timer, "Import elements time");

    timer.Start();
    if (DgnDbStatus::Success != (status = _ImportElementAspectsFrom(sourceModel, importer)))
        return status;
    LogPerformance(timer, "Import element aspects time");

    timer.Start();
    if (DgnDbStatus::Success != (status = _ImportECRelationshipsFrom(sourceModel, importer)))
        return status;
    LogPerformance(timer, "Import ECRelationships time");

    LogPerformance(totalTimer, "Total contents import time");

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
    LogPerformance(totalTimer, "Total import time");
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
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr SessionModel::_CloneForImport(DgnDbStatus* stat, DgnImportContext& importer, DgnElementCR destinationElementToModel) const
    {
    if (nullptr != stat)
        *stat = DgnDbStatus::WrongModel;

    BeAssert(false && "The SessionModel cannot be cloned");
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
DgnModel::ElementIterator DgnModel::MakeIterator(Utf8CP whereClause, Utf8CP orderByClause) const
    {
    Utf8PrintfString sql("SELECT ECInstanceId,CodeValue,UserLabel,ECClassId,ParentId,FederationGuid,LastMod FROM " BIS_SCHEMA(BIS_CLASS_Element) " WHERE ModelId=?");

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

    DgnModel::ElementIterator iterator;
    auto stmt = iterator.Prepare(m_dgndb, sql.c_str(), 0 /* Index of ECInstanceId */);
    stmt->BindId(1, GetModelId());

    return iterator;
    }

DgnElementId DgnModel::IterEntry::GetId() const {return m_statement->GetValueId<DgnElementId>(0);}
Utf8String DgnModel::IterEntry::GetCodeValue() const {return m_statement->GetValueText(1);}
Utf8String DgnModel::IterEntry::GetUserLabel() const {return m_statement->GetValueText(2);}
DgnClassId DgnModel::IterEntry::GetClassId() const {return m_statement->GetValueId<DgnClassId>(3);}
DgnElementId DgnModel::IterEntry::GetParentId() const {return m_statement->GetValueId<DgnElementId>(4);}
BeSQLite::BeGuid DgnModel::IterEntry::GetFederationGuid() const {return m_statement->GetValueGuid(5);}
DateTime DgnModel::IterEntry::GetTimeStamp() const {return m_statement->GetValueDateTime(6);}
