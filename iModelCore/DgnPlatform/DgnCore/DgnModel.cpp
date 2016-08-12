/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnModel.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define MODEL_PROP_ECInstanceId "ECInstanceId"
#define MODEL_PROP_CodeAuthorityId "CodeAuthorityId"
#define MODEL_PROP_CodeNamespace "CodeNamespace"
#define MODEL_PROP_CodeValue "CodeValue"
#define MODEL_PROP_Label "Label"
#define MODEL_PROP_Visibility "Visibility"
#define MODEL_PROP_Properties "Properties"
#define MODEL_PROP_DependencyIndex "DependencyIndex"
#define SHEET_MODEL_PROP_SheetSize "SheetSize"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DgnModels::QueryModelId(DgnCode code) const
    {
    CachedStatementPtr stmt;
    GetDgnDb().GetCachedStatement(stmt, "SELECT Id FROM " BIS_TABLE(BIS_CLASS_Model) " WHERE CodeAuthorityId=? AND CodeNamespace=? AND CodeValue=? LIMIT 1");
    stmt->BindId(1, code.GetAuthority());
    stmt->BindText(2, code.GetNamespace(), Statement::MakeCopy::No);
    stmt->BindText(3, code.GetValue(), Statement::MakeCopy::No);
    return (BE_SQLITE_ROW != stmt->Step()) ? DgnModelId() : stmt->GetValueId<DgnModelId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModels::QueryModelById(Model* out, DgnModelId id) const
    {
    Statement stmt(m_dgndb, "SELECT CodeValue,Label,ECClassId,Visibility,CodeNamespace,CodeAuthorityId FROM " BIS_TABLE(BIS_CLASS_Model) " WHERE Id=?");
    stmt.BindId(1, id);

    if (BE_SQLITE_ROW != stmt.Step())
        return ERROR;

    if (out) // this can be null to just test for the existence of a model by id
        {
        out->m_id = id;
        out->m_label.AssignOrClear(stmt.GetValueText(1));
        out->m_classId = stmt.GetValueId<DgnClassId>(2);
        out->m_inGuiList = TO_BOOL(stmt.GetValueInt(3));
        out->m_code.From(stmt.GetValueId<DgnAuthorityId>(5), stmt.GetValueText(0), stmt.GetValueText(4));
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModels::GetModelCode(DgnCode& code, DgnModelId id) const
    {
    Statement stmt(m_dgndb, "SELECT CodeAuthorityId,CodeNamespace,CodeValue FROM " BIS_TABLE(BIS_CLASS_Model) " WHERE Id=?");
    stmt.BindId(1, id);

    if (BE_SQLITE_ROW != stmt.Step())
        return  ERROR;

    code.From(stmt.GetValueId<DgnAuthorityId>(0), stmt.GetValueText(2), stmt.GetValueText(1));
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DgnModels::GetModelCode(Iterator::Entry const& entry)
    {
    DgnCode code;
    code.From(entry.GetCodeAuthorityId(), entry.GetCodeValue(), entry.GetCodeNamespace());
    return code;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModels::AddLoadedModel(DgnModelR model)
    {
    model.m_persistent = true;
    m_models.Insert(model.GetModelId(), &model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModels::DropLoadedModel(DgnModelR model)
    {
    model.m_persistent = false;
    m_models.erase(model.GetModelId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModels::Empty()
    {
    for (auto iter : m_models)
        {
        iter.second->EmptyModel();
        iter.second->m_persistent = false;
        }

    m_dgndb.Elements().Destroy(); // Has to be called before models are released.
    m_models.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.Wilson                      03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModels::QueryModelDependencyIndex(uint64_t& didx, DgnModelId mid)
    {
    auto i = m_modelDependencyIndices.find(mid);
    if (i != m_modelDependencyIndices.end())
        {
        didx = i->second;
        return BSISUCCESS;
        }

    CachedStatementPtr selectDependencyIndex;
    GetDgnDb().GetCachedStatement(selectDependencyIndex, "SELECT DependencyIndex FROM " BIS_TABLE(BIS_CLASS_Model) " WHERE Id=?");
    selectDependencyIndex->BindId(1, mid);
    if (selectDependencyIndex->Step() != BE_SQLITE_ROW)
        return BSIERROR;

    didx = selectDependencyIndex->GetValueInt64(0);
    m_modelDependencyIndices[mid] = didx;
    return BSISUCCESS;
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
        Utf8String sqlString = "SELECT Id,CodeValue,Label,Visibility,ECClassId,CodeAuthorityId,CodeNamespace FROM " BIS_TABLE(BIS_CLASS_Model);
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

DgnModelId      DgnModels::Iterator::Entry::GetModelId() const {Verify(); return m_sql->GetValueId<DgnModelId>(0);}
Utf8CP          DgnModels::Iterator::Entry::GetCodeValue() const {Verify(); return m_sql->GetValueText(1);}
Utf8CP          DgnModels::Iterator::Entry::GetLabel() const {Verify(); return m_sql->GetValueText(2);}
bool            DgnModels::Iterator::Entry::GetInGuiList() const { Verify(); return (0 != m_sql->GetValueInt(3)); }
DgnClassId      DgnModels::Iterator::Entry::GetClassId() const {Verify(); return m_sql->GetValueId<DgnClassId>(4);}
Utf8CP          DgnModels::Iterator::Entry::GetCodeNamespace() const {Verify(); return m_sql->GetValueText(5);}
DgnAuthorityId  DgnModels::Iterator::Entry::GetCodeAuthorityId() const {Verify(); return m_sql->GetValueId<DgnAuthorityId>(6);}

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
CachedECSqlStatementPtr DgnModels::GetSelectStmt(DgnModelR model) { return FindClassInfo(model).GetSelectStmt(m_dgndb, ECInstanceId(model.GetModelId().GetValue())); }
CachedECSqlStatementPtr DgnModels::GetInsertStmt(DgnModelR model) { return FindClassInfo(model).GetInsertStmt(m_dgndb); }
CachedECSqlStatementPtr DgnModels::GetUpdateStmt(DgnModelR model) { return FindClassInfo(model).GetUpdateStmt(m_dgndb, ECInstanceId(model.GetModelId().GetValue())); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/00
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::ReleaseAllElements()
    {
    m_filled  = false;   // this must be before we release all elementrefs
    m_elements.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::DgnModel(CreateParams const& params) : m_dgndb(params.m_dgndb), m_classId(params.m_classId), m_code(params.m_code), m_inGuiList(params.m_inGuiList), m_label(params.m_label),
    m_dependencyIndex(-1), m_persistent(false), m_filled(false)
    {
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

struct EmptiedCaller {DgnModel::AppData::DropMe operator()(DgnModel::AppData& handler, DgnModelCR model) const {return handler._OnEmptied(model);}};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::_EmptyModel()
    {
    ClearRangeIndex();
    T_Super::_EmptyModel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_EmptyModel()
    {
    if (!m_filled)
        return;

    for (auto appdata : m_appData)
        appdata.second->_OnEmpty(*this);

    ReleaseAllElements();
    CallAppData(EmptiedCaller());
    }

/*---------------------------------------------------------------------------------**//**
* Destructor for DgnModel. Free all memory allocated to this DgnModel.
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::~DgnModel()
    {
    m_appData.clear();
    EmptyModel();
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
DgnDbStatus GroupInformationModel::_OnInsertElement(DgnElementR element)
    {
    return element.IsGroupInformationElement() ? T_Super::_OnInsertElement(element) : DgnDbStatus::WrongModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
GroupInformationModelPtr GroupInformationModel::Create(DgnDbR db, DgnCode const& modelCode)
    {
    ModelHandlerR handler = dgn_ModelHandler::GroupInformation::GetHandler();
    DgnClassId classId = db.Domains().GetClassId(handler);
    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, modelCode));

    return dynamic_cast<GroupInformationModelP>(model.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RepositoryModel::_OnInsertElement(DgnElementR element)
    {
    return element.IsInformationElement() ? T_Super::_OnInsertElement(element) : DgnDbStatus::WrongModel;
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
    m_dependencyIndex = statement.GetValueInt(params.GetSelectIndex(MODEL_PROP_DependencyIndex));
    
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
DgnDbStatus DgnModel::BindInsertAndUpdateParams(ECSqlStatement& statement)
    {
    if (!m_code.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::InvalidName;
        }

    if (m_code.IsEmpty() && (ECSqlStatus::Success != statement.BindNull(statement.GetParameterIndex(MODEL_PROP_CodeValue))))
        return DgnDbStatus::BadArg;

    if (!m_code.IsEmpty() && (ECSqlStatus::Success != statement.BindText(statement.GetParameterIndex(MODEL_PROP_CodeValue), m_code.GetValue().c_str(), IECSqlBinder::MakeCopy::No)))
        return DgnDbStatus::BadArg;

    if ((ECSqlStatus::Success != statement.BindId(statement.GetParameterIndex(MODEL_PROP_CodeAuthorityId), m_code.GetAuthority())) ||
        (ECSqlStatus::Success != statement.BindText(statement.GetParameterIndex(MODEL_PROP_CodeNamespace), m_code.GetNamespace().c_str(), IECSqlBinder::MakeCopy::No)))
        return DgnDbStatus::BadArg;

    if (!m_label.empty())
        statement.BindText(statement.GetParameterIndex(MODEL_PROP_Label), m_label.c_str(), IECSqlBinder::MakeCopy::No);
    else
        statement.BindNull(statement.GetParameterIndex(MODEL_PROP_Label));

    statement.BindBoolean(statement.GetParameterIndex(MODEL_PROP_Visibility), m_inGuiList);

    statement.BindInt(statement.GetParameterIndex(MODEL_PROP_DependencyIndex), m_dependencyIndex);

    Json::Value propJson(Json::objectValue);
    _WriteJsonProperties(propJson);
    if (!propJson.isNull())
        {
        Utf8String val = Json::FastWriter::ToString(propJson);
        statement.BindText(statement.GetParameterIndex(MODEL_PROP_Properties), val.c_str(), IECSqlBinder::MakeCopy::Yes);
        }
    else
        statement.BindNull(statement.GetParameterIndex(MODEL_PROP_Properties));

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_BindInsertParams(ECSqlStatement& statement)
    {
    statement.BindId(statement.GetParameterIndex(MODEL_PROP_ECInstanceId), m_modelId);
    return BindInsertAndUpdateParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_BindUpdateParams(ECSqlStatement& statement)
    {
    return BindInsertAndUpdateParams(statement);
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

    status = _BindUpdateParams(*stmt);
    if (DgnDbStatus::Success != status)
        return status;

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

    auto existingModelIdWithCode = GetDgnDb().Models().QueryModelId(m_code);
    if (existingModelIdWithCode.IsValid() && existingModelIdWithCode != GetModelId())
        return DgnDbStatus::DuplicateCode;
    else if (GetDgnDb().Elements().QueryElementIdByCode(m_code).IsValid())
        return DgnDbStatus::DuplicateCode;

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
* Allocate and initialize a range tree for this model.
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::AllocateRangeIndex() const
    {
    if (nullptr == m_rangeIndex)
        {
        m_rangeIndex = new DgnRangeTree(Is3d(), 20);
        m_rangeIndex->LoadTree(*this);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::ClearRangeIndex()
    {
    DELETE_AND_CLEAR(m_rangeIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::AddToRangeIndex(DgnElementCR element)
    {
    if (nullptr == m_rangeIndex)
        return;

    GeometrySourceCP geom = element.ToGeometrySource();
    if (nullptr != m_rangeIndex && nullptr != geom && geom->HasGeometry())
        m_rangeIndex->AddGeomElement(*geom);
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
        m_rangeIndex->RemoveElement(DgnRangeTree::Entry(geom->CalculateRange3d(), *geom));
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

    if (origBox.IsValid())
        m_rangeIndex->RemoveElement(DgnRangeTree::Entry(origBox, *origGeom));

    if (newBox.IsValid())
        m_rangeIndex->AddElement(DgnRangeTree::Entry(newBox, *origGeom));  // origGeom has the address that will be used after update completes
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_RegisterElement(DgnElementCR element)
    {
    if (m_filled)
        m_elements.Add(element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::_RegisterElement(DgnElementCR element)
    {
    T_Super::_RegisterElement(element);
    AddToRangeIndex(element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnLoadedElement(DgnElementCR el) 
    {
    RegisterElement(el);
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
void DgnModel::_OnInsertedElement(DgnElementCR el) 
    {
    RegisterElement(el);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnReversedDeleteElement(DgnElementCR el) 
    {
    RegisterElement(el);
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
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::_OnDeletedElement(DgnElementCR element)
    {
    RemoveFromRangeIndex(element);
    T_Super::_OnDeletedElement(element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnDeletedElement(DgnElementCR element)
    {
    if (m_filled)
        m_elements.erase(element.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::_OnReversedAddElement(DgnElementCR element)
    {
    RemoveFromRangeIndex(element);
    T_Super::_OnReversedAddElement(element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnReversedAddElement(DgnElementCR element)
    {
    if (m_filled)
        m_elements.erase(element.GetElementId());
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
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::_OnUpdatedElement(DgnElementCR modified, DgnElementCR original)
    {
    UpdateRangeIndex(modified, original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::_OnReversedUpdateElement(DgnElementCR modified, DgnElementCR original)
    {
    UpdateRangeIndex(modified, original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRangeTreeP GeometricModel::_GetRangeIndexP(bool create) const
    {
    if (nullptr == m_rangeIndex && create)
        AllocateRangeIndex();

    return m_rangeIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCP DgnModel::FindElementById(DgnElementId id)
    {
    auto it = m_elements.find(id);
    return it == m_elements.end() ? nullptr : it->second.get();
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
    if (Is2dModel())
        {
        for (auto const& entry : ViewDefinition::MakeIterator(GetDgnDb(), ViewDefinition::Iterator::Options()))
            {
            auto view = ViewDefinition::QueryView(entry.GetId(), GetDgnDb());
            if (!view.IsValid())
                continue;
            auto view2d = dynamic_cast<ViewDefinition2d const*>(view.get());
            if (nullptr == view2d)
                continue;
            if (view2d->GetBaseModelId() != GetModelId())
                continue;
            view->Delete();
            }
        }
    else
        {
#ifdef WIP_VIEW_DEFINITION // *** cascade model delete to modelselectors
        // *** 3d models: models are in modelselectors, which are used by views and by other things
        // *** Select all modelselectors where this ID is in the selector
        // *** WIP_VIEW_DEFINITION If this is the only model in the selector, delete the selector ... which should cascade to deleting all views that use it.
        // *** WIP_VIEW_DEFINITION If this is only one model in the selector, remove it from the selector. *** NEEDS WORK: How to notify an open view that its selector has changed?
#endif
        BeAssert(false && "*** WIP_VIEW_DEFINITION");
        }

    return DgnDbStatus::Success;
    }

struct DeletedCaller {DgnModel::AppData::DropMe operator()(DgnModel::AppData& handler, DgnModelCR model) const {return handler._OnDeleted(model);}};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnDeleted()
    {
    CallAppData(DeletedCaller());
    GetDgnDb().Models().DropLoadedModel(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus DgnModel::_PopulateRequest(IBriefcaseManager::Request& req, BeSQLite::DbOpcode op) const
    {
    BeAssert(m_code.IsValid() && !m_code.IsEmpty());
    switch (op)
        {
        case BeSQLite::DbOpcode::Insert:
            req.Codes().insert(m_code);
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

#ifdef WIP_VIEW_DEFINITION // *** Must handle 2D and 3D models differently
            // and we must delete all of its views
            for (auto const& entry : ViewDefinition::MakeIterator(GetDgnDb(), ViewDefinition::Iterator::Options()))
                {
                auto view = ViewDefinition::QueryView(entry.GetId(), GetDgnDb());
                if (view.IsValid())
                    {
                    auto stat = view->PopulateRequest(req, BeSQLite::DbOpcode::Delete);
                    if (RepositoryStatus::Success != stat)
                        return stat;
                    }
                }
#endif

            break;
            }
        case BeSQLite::DbOpcode::Update:
            {
            req.Locks().Insert(*this, LockLevel::Exclusive);
            DgnCode originalCode;
            if (SUCCESS == GetDgnDb().Models().GetModelCode(originalCode, GetModelId()) && originalCode != m_code)
                req.Codes().insert(m_code);
            
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

    if (!DgnModels::IsValidName(m_code.GetValue()))
        {
        BeAssert(false);
        return DgnDbStatus::InvalidName;
        }

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

    DgnDbStatus stat = _OnDelete();
    if (DgnDbStatus::Success != stat)
        return stat;

    Statement stmt(m_dgndb, "DELETE FROM " BIS_TABLE(BIS_CLASS_Model) " WHERE Id=?");
    stmt.BindId(1, m_modelId);
    return BE_SQLITE_DONE == stmt.Step() ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::Insert()
    {
    DgnDbStatus status = _OnInsert();
    if (DgnDbStatus::Success != status)
        return status;

    if (GetDgnDb().Models().QueryModelId(m_code).IsValid() || GetDgnDb().Elements().QueryElementIdByCode(m_code).IsValid()) // can't allow two models with the same code
        return DgnDbStatus::DuplicateCode;

    m_modelId = DgnModelId(m_dgndb, BIS_TABLE(BIS_CLASS_Model), "Id");

    CachedECSqlStatementPtr stmt = GetDgnDb().Models().GetInsertStmt(*this);
    if (stmt.IsNull())
        {
        m_modelId = DgnModelId();
        return DgnDbStatus::WriteError;
        }
    
    status = _BindInsertParams(*stmt);
    if (DgnDbStatus::Success != status)
        {
        m_modelId = DgnModelId();
        return status;
        }
        
    DbResult stmtResult = stmt->Step();
    if (BE_SQLITE_DONE != stmtResult)
        {
        m_modelId = DgnModelId();
        return DgnDbStatus::WriteError;
        }
        
    // NB: We do this here rather than in _OnInserted() because Update() is going to request a lock too, and the server doesn't need to be
    // involved in locks for models created locally.
    GetDgnDb().BriefcaseManager().OnModelInserted(GetModelId());
    status = Update();
    BeAssert(status == DgnDbStatus::Success);

    _OnInserted();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_InitFrom(DgnModelCR other)
    {
    m_label = other.m_label;
    m_inGuiList = other.m_inGuiList;
    m_dependencyIndex = other.m_dependencyIndex;

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

    DgnModel::CreateParams params(m_dgndb, model.GetClassId(), model.GetCode(), model.GetLabel(), model.GetInGuiList());
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
    auto it=m_models.find(modelId);
    return  it!=m_models.end() ? it->second : NULL;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnModels::GetUniqueModelName(Utf8CP baseName)
    {
    Utf8String tmpStr(baseName);

    if (!tmpStr.empty() && !m_dgndb.Models().QueryModelId(DgnModel::CreateModelCode(tmpStr)).IsValid())
        return tmpStr;

    bool addDash = !tmpStr.empty();
    int index = 0;
    size_t lastDash = tmpStr.find_last_of('-');
    if (lastDash != Utf8String::npos)
        {
        if (BE_STRING_UTILITIES_UTF8_SSCANF(&tmpStr[lastDash], "-%d", &index) == 1)
            addDash = false;
        else
            index = 0;
        }

    Utf8String uniqueModelName;
    do  {
        uniqueModelName.assign(tmpStr);
        if (addDash)
            uniqueModelName.append("-");
        uniqueModelName.append(Utf8PrintfString("%d", ++index));
        } while (m_dgndb.Models().QueryModelId(DgnModel::CreateModelCode(uniqueModelName)).IsValid());

    return uniqueModelName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DgnModels::QueryFirstModelId() const
    {
    for (auto const& model : MakeIterator())
        if ((model.GetModelId() != DgnModel::RepositoryModelId()) && (model.GetModelId() != DgnModel::DictionaryId()) && (model.GetModelId() != DgnModel::GroupInformationId()))
            return model.GetModelId();

    return DgnModelId();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  2/2016
//----------------------------------------------------------------------------------------
void DgnModels::DropGraphicsForViewport(DgnViewportCR viewport)
    {
    for (auto iter : m_models)
        {
        if(iter.second.IsValid())
            iter.second->_DropGraphicsForViewport(viewport);
        }        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
double GeometricModel::DisplayInfo::GetMillimetersPerMaster() const
    {
    return GetMasterUnits().IsLinear() ? GetMasterUnits().ToMillimeters() : 1000.;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/01
+---------------+---------------+---------------+---------------+---------------+------*/
double GeometricModel::DisplayInfo::GetSubPerMaster() const
    {
    return GetSubUnits().ConvertDistanceFrom(1.0, GetMasterUnits());
    }

struct FilledCaller {DgnModel::AppData::DropMe operator()(DgnModel::AppData& handler, DgnModelCR model) const {return handler._OnFilled(model);}};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_FillModel()
    {
    if (IsFilled())
        return;

    enum Column : int {Id=0,ClassId=1,CodeAuthorityId=2,CodeNamespace=3,CodeValue=4,Label=5,ParentId=6};
    Statement stmt(m_dgndb, "SELECT Id,ECClassId,CodeAuthorityId,CodeNamespace,CodeValue,Label,ParentId FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE ModelId=?");
    stmt.BindId(1, m_modelId);

    _SetFilled();

    auto& elements = m_dgndb.Elements();
    BeDbMutexHolder _v(elements.m_mutex);

    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnElementId id(stmt.GetValueId<DgnElementId>(Column::Id));
        DgnElementCP el = elements.FindElement(id);
        if (nullptr != el)  // already loaded?
            {
            RegisterElement(*el);
            continue;
            }

        DgnCode code;
        code.From(stmt.GetValueId<DgnAuthorityId>(Column::CodeAuthorityId), stmt.GetValueText(Column::CodeValue), stmt.GetValueText(Column::CodeNamespace));
        DgnElement::CreateParams createParams(m_dgndb, m_modelId,
            stmt.GetValueId<DgnClassId>(Column::ClassId), 
            code,
            stmt.GetValueText(Column::Label),
            stmt.GetValueId<DgnElementId>(Column::ParentId));

        createParams.SetElementId(id);

        elements.LoadElement(createParams, true);
        }

    CallAppData(FilledCaller());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      01/10
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
    params.Add(MODEL_PROP_CodeAuthorityId, ECSqlClassParams::StatementType::InsertUpdate);
    params.Add(MODEL_PROP_CodeNamespace, ECSqlClassParams::StatementType::InsertUpdate);
    params.Add(MODEL_PROP_CodeValue, ECSqlClassParams::StatementType::InsertUpdate);
    params.Add(MODEL_PROP_Label, ECSqlClassParams::StatementType::InsertUpdate);
    params.Add(MODEL_PROP_Visibility, ECSqlClassParams::StatementType::InsertUpdate);
    params.Add(MODEL_PROP_Properties, ECSqlClassParams::StatementType::All);
    params.Add(MODEL_PROP_DependencyIndex, ECSqlClassParams::StatementType::All);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ModelHandler::Sheet::_GetClassParams(ECSqlClassParamsR params)
    {
    T_Super::_GetClassParams(params);
    params.Add(SHEET_MODEL_PROP_SheetSize, ECSqlClassParams::StatementType::All);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/11
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d GeometricModel::_QueryModelRange() const
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
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
SheetModel::SheetModel(CreateParams const& params) : T_Super(params), m_size(params.m_size) {}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SheetModel::BindInsertAndUpdateParams(ECSqlStatement& statement)
    {
    statement.BindPoint2D(statement.GetParameterIndex(SHEET_MODEL_PROP_SheetSize), m_size);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SheetModel::_BindInsertParams(ECSqlStatement& statement)
    {
    T_Super::_BindInsertParams(statement);
    return BindInsertAndUpdateParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SheetModel::_BindUpdateParams(ECSqlStatement& statement)
    {
    T_Super::_BindUpdateParams(statement);
    return BindInsertAndUpdateParams(statement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SheetModel::_ReadSelectParams(ECSqlStatement& statement, ECSqlClassParamsCR params)
    {
    DgnDbStatus status = T_Super::_ReadSelectParams(statement, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_size = statement.GetValuePoint2D(params.GetSelectIndex(SHEET_MODEL_PROP_SheetSize));
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SheetModel::_InitFrom(DgnModelCR other)
    {
    T_Super::_InitFrom(other);
    SheetModelCP otherModel = other.ToSheetModel();
    if (nullptr != otherModel)
        m_size = otherModel->m_size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::CreateParams::RelocateToDestinationDb(DgnImportContext& importer)
    {
    m_classId = importer.RemapClassId(m_classId);
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
DgnModel::CreateParams DgnModel::GetCreateParamsForImport(DgnImportContext& importer) const
    {
    CreateParams parms(importer.GetDestinationDb(), GetClassId(), GetCode());
    if (importer.IsBetweenDbs())
        {
        // Caller probably wants to preserve these when copying between Dbs. We *never* preserve them when copying within a Db.
        parms.m_code = GetCode();

        parms.RelocateToDestinationDb(importer);
        }
    return parms;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnModel::Clone(DgnCode newCode) const
    {
    if (GetModelHandler()._IsRestrictedAction(RestrictedAction::Clone))
        return nullptr;

    DgnModelPtr newModel = GetModelHandler().Create(DgnModel::CreateParams(m_dgndb, m_classId, newCode));
    newModel->_InitFrom(*this);
    return newModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnModel::_CloneForImport(DgnDbStatus* stat, DgnImportContext& importer) const
    {
    if (nullptr != stat)
        *stat = DgnDbStatus::Success;

    DgnModel::CreateParams params = GetCreateParamsForImport(importer); // remaps classid

    if (!params.m_code.IsValid())
        params.m_code = GetCode();

    if (importer.GetDestinationDb().Models().QueryModelId(params.m_code).IsValid()) // Is the name already used in destination?
        {
        if (nullptr != stat)
            *stat = DgnDbStatus::DuplicateCode;
        return nullptr;
        }

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
    importECRelationshipsFrom(GetDgnDb(), sourceModel, importer, BIS_TABLE(BIS_REL_CategorySelectorsReferToCategories), "SourceECInstanceId", "TargetECInstanceId");
    LogPerformance(timer, "Import ECRelationships %s", BIS_REL_CategorySelectorsReferToCategories);

    // *** WIP_IMPORT *** ElementsHaveLinks -- should we deep-copy links?

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
DgnModelPtr DgnModel::ImportModel(DgnDbStatus* statIn, DgnModelCR sourceModel, DgnImportContext& importer)
    {
    StopWatch totalTimer(true);

    DgnDbStatus _stat;
    DgnDbStatus& stat = (nullptr != statIn)? *statIn: _stat;

    BeAssert(&sourceModel.GetDgnDb() == &importer.GetSourceDb());

    DgnModelPtr newModel = sourceModel._CloneForImport(&stat, importer);
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
DgnModelPtr DgnModel::CopyModel(DgnModelCR model, DgnCode newCode)
    {
    DgnDbR db = model.GetDgnDb();

    DgnModelPtr model2 = model.Clone(newCode);
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
DgnModelPtr DictionaryModel::_CloneForImport(DgnDbStatus* stat, DgnImportContext& importer) const
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
    struct Pair { Utf8CP name; uint64_t value; };
    static const Pair s_pairs[] =
        {
            { "insertelement", InsertElement },
            { "updateelement", UpdateElement },
            { "deleteelement", DeleteElement },
            { "clone", Clone },
            { "setcode", SetCode },
        };

    for (auto const& pair : s_pairs)
        {
        if (0 == BeStringUtilities::Stricmp(pair.name, name))
            return pair.value;
        }

    return T_Super::Parse(name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_SetCode(DgnCode const& code)
    {
    if (GetModelHandler()._IsRestrictedAction(RestrictedAction::SetCode))
        return DgnDbStatus::MissingHandler;

    m_code = code;
    return DgnDbStatus::Success;
    }
