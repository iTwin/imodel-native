/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnModel.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnDbTables.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static std::pair<Utf8String,Utf8String> parseFullECClassName(Utf8CP fullname)
    {
    Utf8CP dot = strchr(fullname, '.');
    if (nullptr == dot)
        return std::make_pair("","");
    return std::make_pair(Utf8String(fullname,dot), Utf8String(dot+1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DgnModels::QueryModelId(DgnModel::Code code) const
    {
    CachedStatementPtr stmt;
    GetDgnDb().GetCachedStatement(stmt, "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE CodeAuthorityId=? AND CodeNameSpace=? AND Code=? LIMIT 1");
    stmt->BindId(1, code.GetAuthority());
    stmt->BindText(2, code.GetNameSpace(), Statement::MakeCopy::No);
    stmt->BindText(3, code.GetValue(), Statement::MakeCopy::No);
    return (BE_SQLITE_ROW != stmt->Step()) ? DgnModelId() : stmt->GetValueId<DgnModelId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModels::QueryModelById(Model* out, DgnModelId id) const
    {
    Statement stmt(m_dgndb, "SELECT Code,Descr,ECClassId,Visibility,CodeNameSpace,CodeAuthorityId FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE Id=?");
    stmt.BindId(1, id);

    if (BE_SQLITE_ROW != stmt.Step())
        return ERROR;

    if (out) // this can be null to just test for the existence of a model by id
        {
        out->m_id = id;
        out->m_description.AssignOrClear(stmt.GetValueText(1));
        out->m_classId = DgnClassId(stmt.GetValueInt64(2));
        out->m_inGuiList = TO_BOOL(stmt.GetValueInt(3));
        out->m_code = DgnModel::Code(stmt.GetValueId<DgnAuthorityId>(5), stmt.GetValueText(0), stmt.GetValueText(4));
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModels::GetModelCode(DgnModel::Code& code, DgnModelId id) const
    {
    Statement stmt(m_dgndb, "SELECT CodeAuthorityId,CodeNameSpace,Code FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE Id=?");
    stmt.BindId(1, id);

    if (BE_SQLITE_ROW != stmt.Step())
        return  ERROR;

    code = DgnModel::Code(stmt.GetValueId<DgnAuthorityId>(0), stmt.GetValueText(2), stmt.GetValueText(1));
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::Code DgnModels::GetModelCode(Iterator::Entry const& entry)
    {
    return DgnModel::Code(entry.GetCodeAuthorityId(), entry.GetCodeValue(), entry.GetCodeNameSpace());
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
void DgnModels::ClearLoaded()
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
    GetDgnDb().GetCachedStatement(selectDependencyIndex, "SELECT DependencyIndex FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE Id=?");
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
    Utf8String sqlString = "SELECT count(*) FROM " DGN_TABLE(DGN_CLASSNAME_Model);
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
        Utf8String sqlString = "SELECT Id,Code,Descr,Visibility,ECClassId,CodeAuthorityId,CodeNameSpace FROM " DGN_TABLE(DGN_CLASSNAME_Model);
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
Utf8CP          DgnModels::Iterator::Entry::GetDescription() const {Verify(); return m_sql->GetValueText(2);}
bool            DgnModels::Iterator::Entry::InGuiList() const {Verify(); return (0 != m_sql->GetValueInt(3));}
DgnClassId      DgnModels::Iterator::Entry::GetClassId() const {Verify(); return DgnClassId(m_sql->GetValueInt64(4));}
Utf8CP          DgnModels::Iterator::Entry::GetCodeNameSpace() const {Verify(); return m_sql->GetValueText(5);}
DgnAuthorityId  DgnModels::Iterator::Entry::GetCodeAuthorityId() const {Verify(); return m_sql->GetValueId<DgnAuthorityId>(6);}

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
DgnModel::DgnModel(CreateParams const& params) : m_dgndb(params.m_dgndb), m_modelId(params.m_id), m_classId(params.m_classId), m_code(params.m_code), m_properties(params.m_props)
    {
    m_persistent = false;
    m_filled     = false;
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
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::_OnValidate()
    {
    m_solver.Solve(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_ToPropertiesJson(Json::Value& val) const {m_properties.ToJson(val);}
void DgnModel::_FromPropertiesJson(Json::Value const& val) {m_properties.FromJson(val);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel2d::_ToPropertiesJson(Json::Value& val) const 
    {
    T_Super::_ToPropertiesJson(val);
    JsonUtils::DPoint2dToJson(val["globalOrigin"], m_globalOrigin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel2d::_FromPropertiesJson(Json::Value const& val) 
    {
    T_Super::_FromPropertiesJson(val);
    JsonUtils::DPoint2dFromJson(m_globalOrigin, val["globalOrigin"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel2d::_OnInsertElement(DgnElementR element)
    {
    DgnDbStatus status = T_Super::_OnInsertElement(element);
    if (DgnDbStatus::Success != status)
        return status;

    // if it is a geometric element, it must be a 2d element.
    return element.IsGeometricElement() && element.Is3d() ? DgnDbStatus::Mismatch2d3d : DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel3d::_OnInsertElement(DgnElementR element)
    {
    auto status = T_Super::_OnInsertElement(element);
    if (DgnDbStatus::Success == status && element.IsGeometricElement() && !element.Is3d())
        status = DgnDbStatus::Mismatch2d3d;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ResourceModel::_OnInsertElement(DgnElementR el)
    {
    auto status = T_Super::_OnInsertElement(el);
    if (DgnDbStatus::Success == status && nullptr != el.ToGeometricElement())
        status = DgnDbStatus::WrongModel;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DictionaryModel::_OnInsertElement(DgnElementR el)
    {
    // dictionary model can contain *only* dictionary elements
    auto status = el.IsDictionaryElement() ? T_Super::_OnInsertElement(el) : DgnDbStatus::WrongModel;
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::ReadProperties()
    {
    _ReadProperties();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::_ReadProperties()
    {
    // NB: Intentionally not invoking superclass implementation to avoid redundant sql
    Statement stmt(m_dgndb, "SELECT Props,Solver FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE Id=?");
    stmt.BindId(1, m_modelId);

    DbResult  result = stmt.Step();
    if (BE_SQLITE_ROW != result)
        {
        BeAssert(false);
        return;
        }

    Json::Value  propsJson(Json::objectValue);
    if (!Json::Reader::Parse(stmt.GetValueText(0), propsJson))
        {
        BeAssert(false);
        return;
        }

    _FromPropertiesJson(propsJson);

    if (!stmt.IsColumnNull(1))
        m_solver.FromJson(stmt.GetValueText(1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_ReadProperties()
    {
    Statement stmt(m_dgndb, "SELECT Props FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE Id=?");
    stmt.BindId(1, m_modelId);

    DbResult  result = stmt.Step();
    if (BE_SQLITE_ROW != result)
        {
        BeAssert(false);
        return;
        }

    Json::Value  propsJson(Json::objectValue);
    if (!Json::Reader::Parse(stmt.GetValueText(0), propsJson))
        {
        BeAssert(false);
        return;
        }

    _FromPropertiesJson(propsJson);
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

    if (LockStatus::Success != GetDgnDb().Locks().LockModel(*this, LockLevel::Exclusive))
        return DgnDbStatus::LockNotHeld;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::Update()
    {
    DgnDbStatus stat = _OnUpdate();
    if (stat != DgnDbStatus::Success)
        return stat;

    stat = _Update();

    if (DgnDbStatus::Success == stat)
        _OnUpdated();

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricModel::_Update()
    {
    // NB: Intentionally not invoking superclass implementation to avoid redundant sql
    Json::Value propJson(Json::objectValue);
    _ToPropertiesJson(propJson);
    Utf8String val = Json::FastWriter::ToString(propJson);

    Statement stmt(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Model) " SET Props=?,Solver=? WHERE Id=?");
    stmt.BindText(1, val, Statement::MakeCopy::No);
    if (m_solver.IsValid())
        stmt.BindText(2, m_solver.ToJson(), Statement::MakeCopy::Yes);
    else
        stmt.BindNull(2);
    stmt.BindId(3, m_modelId);

    return stmt.Step() == BE_SQLITE_DONE ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_Update()
    {
    Json::Value propJson(Json::objectValue);
    _ToPropertiesJson(propJson);
    Utf8String val = Json::FastWriter::ToString(propJson);

    Statement stmt(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Model) " SET Props=? WHERE Id=?");
    stmt.BindText(1, val, Statement::MakeCopy::No);
    stmt.BindId(2, m_modelId);

    return stmt.Step() == BE_SQLITE_DONE ? DgnDbStatus::Success : DgnDbStatus::WriteError;
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

    GeometricElementCP geom = element.ToGeometricElement();
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

    GeometricElementCP geom = element.ToGeometricElement();
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

    GeometricElementCP origGeom = original.ToGeometricElement();
    if (nullptr == origGeom)
        return;

    GeometricElementCP newGeom = (GeometricElementCP) &modified;
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
    else if (GetModelHandler()._IsRestrictedAction(RestrictedAction::InsertElement))
        return DgnDbStatus::MissingHandler;
    else
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
    else if (GetModelHandler()._IsRestrictedAction(RestrictedAction::DeleteElement))
        return DgnDbStatus::MissingHandler;
    else
        return DgnDbStatus::Success;
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
    else if (GetModelHandler()._IsRestrictedAction(RestrictedAction::UpdateElement))
        return DgnDbStatus::MissingHandler;
    else
        return DgnDbStatus::Success;
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
* @bsimethod                                                    Keith.Bentley   05/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnModels::FreeQvCache()
    {
    if (nullptr == m_qvCache)
        return  false;

    // if there is a QvCache associated with this DgnFile, delete it too.
    T_HOST.GetGraphicsAdmin()._DeleteQvCache(m_qvCache);
    m_qvCache = nullptr;
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_OnDelete()
    {
    if (GetModelHandler()._IsRestrictedAction(RestrictedAction::Delete))
        return DgnDbStatus::MissingHandler;

    if (LockStatus::Success != GetDgnDb().Locks().LockModel(*this, LockLevel::Exclusive))
        return DgnDbStatus::LockNotHeld;

    for (auto appdata : m_appData)
        appdata.second->_OnDelete(*this);

    // before we can delete a model, we must delete all of its elements. If that fails, we cannot continue.
    Statement stmt(m_dgndb, "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE ModelId=?");
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
    GetDgnDb().Models().DropLoadedModel(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_OnInsert()
    {
    if (m_modelId.IsValid())
        return DgnDbStatus::IdExists;

    if (&m_dgndb != &m_dgndb)
        return DgnDbStatus::WrongDgnDb;

    if (!DgnModels::IsValidName(m_code.GetValue()))
        {
        BeAssert(false);
        return DgnDbStatus::InvalidName;
        }

    if (GetModelHandler()._IsRestrictedAction(RestrictedAction::Insert))
        return DgnDbStatus::MissingHandler;

    // If db is exclusively locked, cannot create models in it
    if (LockStatus::Success != GetDgnDb().Locks().LockDb(LockLevel::Shared))
        return DgnDbStatus::LockNotHeld;

    return DgnDbStatus::Success;
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

    Statement stmt(m_dgndb, "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE Id=?");
    stmt.BindId(1, m_modelId);
    return BE_SQLITE_DONE == stmt.Step() ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::Insert(Utf8CP description, bool inGuiList)
    {
    DgnDbStatus stat = _OnInsert();
    if (DgnDbStatus::Success != stat)
        return stat;

    DgnModels& models = GetDgnDb().Models();

    m_code.m_value.Trim();
    if (models.QueryModelId(m_code).IsValid()) // can't allow two models with the same code
        return DgnDbStatus::DuplicateName;

    m_modelId = DgnModelId(m_dgndb, DGN_TABLE(DGN_CLASSNAME_Model), "Id");

    Statement stmt(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Model) "(Id,Code,Descr,ECClassId,Visibility,CodeAuthorityId,CodeNameSpace) VALUES(?,?,?,?,?,?,?)");
    stmt.BindId(1, m_modelId);
    stmt.BindText(2, m_code.GetValue().c_str(), Statement::MakeCopy::No);
    stmt.BindText(3, description, Statement::MakeCopy::No);
    stmt.BindId(4, GetClassId());
    stmt.BindInt(5, inGuiList ? 1 : 0);
    stmt.BindId(6, m_code.GetAuthority());
    stmt.BindText(7, m_code.GetNameSpace().c_str(), Statement::MakeCopy::No);

    auto rc = stmt.Step();
    if (BE_SQLITE_DONE != rc)
        {
        BeAssert(false);
        m_modelId = DgnModelId();
        return DgnDbStatus::WriteError;
        }

    // NB: We do this here rather than in _OnInserted() because Update() is going to request a lock too, and the server doesn't need to be
    // involved in locks for models created locally.
    GetDgnDb().Locks().OnModelInserted(GetModelId());
    stat = Update();
    BeAssert(stat==DgnDbStatus::Success);

    _OnInserted();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_InitFrom(DgnModelCR other)
    {
    Json::Value props;
    other._ToPropertiesJson(props);
    _FromPropertiesJson(props);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricModel::_InitFrom(DgnModelCR other)
    {
    T_Super::_InitFrom(other);
    auto geom = other.ToGeometricModel();
    if (nullptr != geom)
        m_solver = geom->m_solver;
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

    DgnModel::CreateParams params(m_dgndb, model.GetClassId(), model.GetCode(), DgnModel::Properties(), modelId);
    DgnModelPtr dgnModel = handler->Create(params);
    if (!dgnModel.IsValid())
        return nullptr;

    dgnModel->ReadProperties();
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
        if (model.GetModelId() != DgnModel::DictionaryId())
            return model.GetModelId();

    return DgnModelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/15
+---------------+---------------+---------------+---------------+---------------+------*/
QvCache* DgnModels::GetQvCache(bool createIfNecessary)
    {
    if (nullptr != m_qvCache || !createIfNecessary)
        return m_qvCache;

    return (m_qvCache = T_HOST.GetGraphicsAdmin()._CreateQvCache());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d GeometricModel::_GetGlobalOrigin() const
    {
    return GetDgnDb().Units().GetGlobalOrigin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
double DgnModel::Properties::GetMillimetersPerMaster() const
    {
    return GetMasterUnits().IsLinear() ? GetMasterUnits().ToMillimeters() : 1000.;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/01
+---------------+---------------+---------------+---------------+---------------+------*/
double DgnModel::Properties::GetSubPerMaster() const
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

    enum Column : int {Id=0,ClassId=1,Code=2,ParentId=3,CodeAuthorityId=4,CodeNameSpace=5,CategoryId=6};
    Statement stmt(m_dgndb, "SELECT Id,ECClassId,Code,ParentId,CodeAuthorityId,CodeNameSpace,CategoryId FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE ModelId=?");
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

        elements.LoadElement(DgnElement::CreateParams(m_dgndb, m_modelId,
            stmt.GetValueId<DgnClassId>(Column::ClassId), 
            DgnElement::Code(stmt.GetValueId<DgnAuthorityId>(Column::CodeAuthorityId), stmt.GetValueText(Column::Code), stmt.GetValueText(Column::CodeNameSpace)), 
            id,
            stmt.GetValueId<DgnElementId>(Column::ParentId)),
            stmt.GetValueId<DgnCategoryId>(Column::CategoryId),
            true);
        }

    CallAppData(FilledCaller());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModel::Properties::SetUnits(UnitDefinitionCR newMasterUnit, UnitDefinitionCR newSubUnit)
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
* @bsimethod                                    Keith.Bentley                   11/11
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d GeometricModel::_QueryModelRange() const
    {
    Statement stmt(m_dgndb, "SELECT DGN_bbox_union(DGN_placement_aabb(g.Placement)) FROM " 
                           DGN_TABLE(DGN_CLASSNAME_Element)     " AS e," 
                           DGN_TABLE(DGN_CLASSNAME_ElementGeom) " AS g"
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
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SheetModel::_ToPropertiesJson(Json::Value& val) const 
    {
    T_Super::_ToPropertiesJson(val);
    JsonUtils::DPoint2dToJson(val["sheet_size"], m_size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SheetModel::_FromPropertiesJson(Json::Value const& val) 
    {
    T_Super::_FromPropertiesJson(val);
    JsonUtils::DPoint2dFromJson(m_size, val["sheet_size"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::CreateParams::RelocateToDestinationDb(DgnImportContext& importer)
    {
    m_classId = importer.RemapClassId(m_classId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::CreateParams DgnModel::GetCreateParamsForImport(DgnImportContext& importer) const
    {
    CreateParams parms(importer.GetDestinationDb(), GetClassId(), GetCode(), GetProperties());
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
DgnModelPtr DgnModel::Clone(Code newCode) const
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
            *stat = DgnDbStatus::DuplicateName;
        return nullptr;
        }

    DgnModelPtr model = GetModelHandler().Create(params);
    if (!model.IsValid())
        return nullptr;

    model->_InitFrom(*this);

    return model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr GeometricModel::_CloneForImport(DgnDbStatus* stat, DgnImportContext& importer) const
    {
    auto clone = T_Super::_CloneForImport(stat, importer);
    auto geom = clone.IsValid() ? clone->ToGeometricModelP() : nullptr;
    if (nullptr != geom)
        geom->m_solver.RelocateToDestinationDb(importer);

    return clone;
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

    Statement stmt(sourceModel.GetDgnDb(), "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE ModelId=?");
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
    
#ifdef WIP_ELEMENT_ITEM // *** pending redesign
    
    // That is, only DgnItem.


    // Step through all items in the source model
    Statement stmt(sourceModel.GetDgnDb(), "SELECT ele.Id FROM " DGN_TABLE(DGN_CLASSNAME_ElementItem) " item JOIN " DGN_TABLE(DGN_CLASSNAME_Element) " ele ON (item.ElementId=ele.Id) WHERE ele.ModelId=?");
    stmt.BindId(1, sourceModel.GetModelId());
    while (BE_SQLITE_ROW == stmt.Step())
        {
        //  Get a source element and its Item
        DgnElementCPtr sourceEl = sourceModel.GetDgnDb().Elements().GetElement(stmt.GetValueId<DgnElementId>(0));
        DgnElement::Item const* sourceitem = DgnElement::Item::GetItem(*sourceEl);
        if (nullptr == sourceitem)
            {
            BeDataAssert(false && "Element has item, but item can't be loaded");
            continue;
            }

        //  Get the corresponding element in the destination model
        DgnElementId ccelid = importer.FindElementId(sourceEl->GetElementId());
        DgnElementPtr ccel = GetDgnDb().Elements().GetForEdit<DgnElement>(ccelid);
        if (!ccel.IsValid())
            continue;       // I guess the source element wasn't copied.

        //  Make a copy of the source item 
        RefCountedPtr<DgnElement::Item> ccitem = dynamic_cast<DgnElement::Item*>(sourceitem->_CloneForImport(*sourceEl, importer).get());
        if (!ccitem.IsValid())
            {
            // *** TBD: Record failure somehow
            BeDataAssert(false && "item import failure");
            continue;
            }

        // Write the copy of the Item to the destination element
        DgnElement::Item::SetItem(*ccel, *ccitem);
        ccel->Update();
        }
#endif
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

    Statement sstmt(sourceModel.GetDgnDb(), Utf8PrintfString(
        "WITH elems(id) AS (SELECT Id from " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE (ModelId=?))"
        " SELECT %s FROM %s rel WHERE (rel.%s IN elems AND rel.%s IN elems)", colsList.c_str(), relname, sourcecol, targetcol));
    sstmt.BindId(1, sourceModel.GetModelId());

    Statement istmt(destDb, Utf8PrintfString(
        "INSERT INTO %s (%s) VALUES(%s)", relname, colsList.c_str(), placeholderList.c_str()));

    while (BE_SQLITE_ROW == sstmt.Step())
        {
        istmt.Reset();
        istmt.ClearBindings();
        int icol = 0;
        istmt.BindId(icol+1, importer.FindElementId(sstmt.GetValueId<DgnElementId>(icol))); // [0] sourcecol
        ++icol;
        istmt.BindId(icol+1, importer.FindElementId(sstmt.GetValueId<DgnElementId>(icol))); // [1] targetcol
        ++icol;
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

    importECRelationshipsFrom(GetDgnDb(), sourceModel, importer, DGN_TABLE(DGN_RELNAME_ElementGroupHasMembers), "GroupId", "MemberId");
    importECRelationshipsFrom(GetDgnDb(), sourceModel, importer, DGN_TABLE(DGN_RELNAME_ElementDrivesElement), "RootElementId", "DependentElementId", "ECClassId", {"Status", "Priority"});
    importECRelationshipsFrom(GetDgnDb(), sourceModel, importer, DGN_TABLE(DGN_RELNAME_ElementUsesStyles), "ElementId", "StyleId");

    // *** WIP_IMPORT *** ElementHasLinks -- should we deep-copy links?

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_ImportContentsFrom(DgnModelCR sourceModel, DgnImportContext& importer)
    {
    BeAssert(&GetDgnDb() == &importer.GetDestinationDb());
    BeAssert(&sourceModel.GetDgnDb() == &importer.GetSourceDb());

    DgnDbStatus status;
     
    if (DgnDbStatus::Success != (status = _ImportElementsFrom(sourceModel, importer)))
        return status;

    if (DgnDbStatus::Success != (status = _ImportElementAspectsFrom(sourceModel, importer)))
        return status;

    if (DgnDbStatus::Success != (status = _ImportECRelationshipsFrom(sourceModel, importer)))
        return status;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnModel::ImportModel(DgnDbStatus* statIn, DgnModelCR sourceModel, DgnImportContext& importer)
    {
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
    return newModel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
DgnModelPtr DgnModel::CopyModel(DgnModelCR model, Code newCode)
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
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentModel::CreateParams::CreateParams(DgnDbR dgndb, Utf8StringCR name, Utf8StringCR iclass, Utf8StringCR icat, Utf8String iauthority, ModelSolverDef const& solver)
    :
    T_Super(dgndb, DgnClassId(dgndb.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ComponentModel)), CreateModelCode(name), Properties(), solver),
    m_compProps(iclass, icat, iauthority)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentModel::ComponentModel(CreateParams const& params) : T_Super(params), m_compProps(params.m_compProps)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::_GetSolverOptions(Json::Value& json)
    {
    json["Category"] = m_compProps.m_itemCategoryName.c_str();          // *** NB: Do not change this name. It is part of the DgnScriptAPI
    json["ECClass"] = m_compProps.m_itemECClassName.c_str();            //              "
    json["CodeAuthority"] = m_compProps.m_itemCodeAuthority.c_str();    //              "
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ComponentModel::CompProps::IsValid(DgnDbR db) const
    {
    if (!QueryItemCategoryId(db).IsValid())
        return false;
    if (!GetItemECClassId(db).IsValid())
        return false;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId ComponentModel::CompProps::QueryItemCategoryId(DgnDbR db) const
    {
    return DgnCategory::QueryCategoryId(m_itemCategoryName.c_str(), db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId ComponentModel::CompProps::GetItemECClassId(DgnDbR db) const
    {
    Utf8String ns, cls;
    std::tie(ns, cls) = parseFullECClassName(m_itemECClassName.c_str());
    return DgnClassId(db.Schemas().GetECClassId(ns.c_str(), cls.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ComponentModel::IsValid() const
    {
    if (!GetSolver().IsValid())
        return false;
    if (!m_compProps.IsValid(GetDgnDb()))
        return false;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::_OnDelete()
    {
#ifdef WIP_COMPONENT_MODEL // *** Pending redesign
        //  *** TRICKY: We don't/can't have a trigger do this, because the solutions table references this model by name
    Statement delSolutions;

    delSolutions.Prepare(GetDgnDb(), "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_ComponentSolution) " WHERE ComponentModelName=?");
    delSolutions.BindText(1, GetModelName(), Statement::MakeCopy::No);
    delSolutions.Step();

    // *** NEEDS WORK: I should kill off all of the GeomParts referenced by the geomstreams in the rows of ComponentSolution 

#endif
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentModel::GetItemCategoryName() const {return m_compProps.m_itemCategoryName;}
Utf8String ComponentModel::GetItemECClassName() const {return m_compProps.m_itemECClassName;}
Utf8String ComponentModel::GetItemCodeAuthority() const {return m_compProps.m_itemCodeAuthority;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::_ToPropertiesJson(Json::Value& val) const {m_compProps.ToJson(val);}
void ComponentModel::_FromPropertiesJson(Json::Value const& val) {m_compProps.FromJson(val);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::CompProps::FromJson(Json::Value const& inValue)
    {
    m_itemCategoryName = inValue["ComponentModel_itemCategoryName"].asCString();
    m_itemECClassName = inValue["ComponentModel_itemECClassName"].asCString();
    m_itemCodeAuthority = inValue["ComponentModel_itemCodeAuthority"].asCString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::CompProps::ToJson(Json::Value& outValue) const
    {
    outValue["ComponentModel_itemCategoryName"] = m_itemCategoryName;
    outValue["ComponentModel_itemECClassName"] = m_itemECClassName;
    outValue["ComponentModel_itemCodeAuthority"] = m_itemCodeAuthority;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus createSolutionOfComponentRelationship(DgnElementCR inst, ComponentModelR componentModel)
    {
    CachedECSqlStatementPtr statement = inst.GetDgnDb().GetPreparedECSqlStatement(
        "INSERT INTO " DGN_SCHEMA(DGN_RELNAME_SolutionOfComponent)
        " (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");

    if (!statement.IsValid())
        return DgnDbStatus::BadRequest;

    statement->BindId(1, inst.GetElementClassId());
    statement->BindId(2, inst.GetElementId());
    statement->BindId(3, componentModel.GetClassId());
    statement->BindId(4, componentModel.GetModelId());
    return (BE_SQLITE_DONE == statement->Step()) ? DgnDbStatus::Success : DgnDbStatus::BadRequest;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    10/2015
//---------------------------------------------------------------------------------------
static DgnModelId queryComponentModelFromSolution(DgnDbR db, DgnElementId itemId)
    {
    CachedECSqlStatementPtr statement = db.GetPreparedECSqlStatement(
        "SELECT TargetECInstanceId FROM " DGN_SCHEMA(DGN_RELNAME_SolutionOfComponent) " WHERE SourceECInstanceId=? LIMIT 1");

    if (!statement.IsValid())
        return DgnModelId();

    statement->BindId(1, itemId);
    if (BE_SQLITE_ROW != statement->Step())
        return DgnModelId();

    return statement->GetValueId<DgnModelId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus createInstanceOfTemplateRelationship(DgnElementCR inst, DgnElementCR templateItem)
    {
    CachedECSqlStatementPtr statement = inst.GetDgnDb().GetPreparedECSqlStatement(
        "INSERT INTO " DGN_SCHEMA(DGN_RELNAME_InstantiationOfTemplate)
        " (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");

    if (!statement.IsValid())
        return DgnDbStatus::BadRequest;

    statement->BindId(1, inst.GetElementClassId());
    statement->BindId(2, inst.GetElementId());
    statement->BindId(3, templateItem.GetElementClassId());
    statement->BindId(4, templateItem.GetElementId());
    return (BE_SQLITE_DONE == statement->Step()) ? DgnDbStatus::Success : DgnDbStatus::BadRequest;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    10/2015
//---------------------------------------------------------------------------------------
#ifndef NDEBUG
static DgnElementId queryTemplateItemFromInstance(DgnDbR db, DgnElementId instanceId)
    {
    CachedECSqlStatementPtr statement = db.GetPreparedECSqlStatement(
        "SELECT TargetECInstanceId FROM " DGN_SCHEMA(DGN_RELNAME_InstantiationOfTemplate) " WHERE SourceECInstanceId=? LIMIT 1");

    if (!statement.IsValid())
        return DgnElementId();

    statement->BindId(1, instanceId);
    if (BE_SQLITE_ROW != statement->Step())
        return DgnElementId();

    return statement->GetValueId<DgnElementId>(0);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalElementCPtr ComponentModel::GetCapturedSolution(DgnDbStatus* statusOut, PhysicalModelR catalogModel, ModelSolverDef::ParameterSet const& parameters, bool generateSolution)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(status, statusOut);

    DgnDbR db = catalogModel.GetDgnDb();
    if (&db != &GetDgnDb())
        {
        BeAssert(false && "you must import the component model before you can capture a solution");
        status = DgnDbStatus::WrongDgnDb;
        return nullptr;
        }

    Utf8String solutionName = parameters.ComputeSolutionName();

    DgnElement::Code icode = CreateCatalogItemCode(solutionName);
    if (!icode.IsValid())
        {
        BeAssert(false);
        status = DgnDbStatus::BadRequest;
        return nullptr;
        }

    //  Check to see if this solution has already been captured. If so, return the existing catalog item.
    DgnElementId existingTemplateItemId = db.Elements().QueryElementIdByCode(icode);
    if (existingTemplateItemId.IsValid())
        {
        PhysicalElementCPtr existingTemplateItem = db.Elements().Get<PhysicalElement>(existingTemplateItemId);
        if (!existingTemplateItem.IsValid())
            {
            BeAssert(false && "template item cannot be loaded");
            status = DgnDbStatus::BadElement;
            return nullptr;
            }
        if (existingTemplateItem->GetModel().get() != &catalogModel)
            {
            BeDataAssert(false && "solution has already been captured in a different model");
            }
        return existingTemplateItem;
        }
    
    if (!generateSolution)
        {
        status = DgnDbStatus::NotFound;
        return nullptr;
        }

    if (DgnDbStatus::Success != (status = Solve(parameters)))
        return nullptr;

    return HarvestSolution(status, catalogModel, solutionName, icode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::Solve(ModelSolverDef::ParameterSet const& parameters)
    {
    //  Generate the requested solution
    GetSolver().GetParametersR().SetValues(parameters);

    DgnDbStatus status = Update();
    if (DgnDbStatus::Success != status)
        return status;

    auto sstatus = GetDgnDb().SaveChanges(); // => txn validation invokes the model's solver
    if (BE_SQLITE_OK != sstatus)
        status = (BE_SQLITE_ERROR_ChangeTrackError == sstatus)? DgnDbStatus::ValidationFailed: DgnDbStatus::SQLiteError;
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalElementCPtr ComponentModel::HarvestSolution(DgnDbStatus& status, PhysicalModelR catalogModel, Utf8StringCR solutionName, DgnElement::Code const& icode)
    {
    DgnDbR db = GetDgnDb();

    DgnClassId iclass = m_compProps.GetItemECClassId(db);
    if (!iclass.IsValid())
        {
        BeAssert(false);
        status = DgnDbStatus::BadSchema;
        return nullptr;
        }

    //  We'll use the same Category for the catalog item as for the (future) instance items.
    DgnCategoryId itemCategoryId = m_compProps.QueryItemCategoryId(db);
    if (!itemCategoryId.IsValid())
        {
        BeAssert(false && "component category not found -- you must import the component model before you can capture a solution");
        status = DgnDbStatus::InvalidCategory;
        return nullptr;
        }

    //  Gather geometry by SubCategory
    bmap<DgnSubCategoryId, ElementGeometryBuilderPtr> builders;     // *** WIP_IMPORT: add another dimension: break out builders by same ElemDisplayParams
    FillModel();
    for (auto const& mapEntry : *this)
        {
        GeometricElementCP componentElement = mapEntry.second->ToGeometricElement();
        if (nullptr == componentElement)
            continue;

        //  Only solution elements in the component's Category are collected. The rest are construction/annotation geometry.
        if (componentElement->GetCategoryId() != itemCategoryId)
            continue;

        // *** NEEDS WORK: Detect, schedule, and skip instances of other CM's
        ElementGeometryCollection gcollection(*componentElement);
        for (ElementGeometryPtr const& geom : gcollection)
            {
            //  Look up the subcategory ... IN THE CLIENT DB
            ElemDisplayParamsCR dparams = gcollection.GetElemDisplayParams();
            DgnSubCategoryId clientsubcatid = dparams.GetSubCategoryId();

            ElementGeometryBuilderPtr& builder = builders [clientsubcatid];
            if (!builder.IsValid())
                builder = ElementGeometryBuilder::CreateGeomPart(db, true);

            // Since each little piece of geometry can have its own transform, we must
            // build the transforms back into them in order to assemble them into a single geomstream.
            // It's all relative to 0,0,0 in the component model, so it's fine to do this.
            ElementGeometryPtr xgeom = geom->Clone();
            Transform trans = gcollection.GetGeometryToWorld(); // A component model is in its own local coordinate system, so "World" just means relative to local 0,0,0
            xgeom->TransformInPlace(trans);

            builder->Append(*xgeom);
            }
        }

    if (builders.empty())
        {
        BeDataAssert(false && "Component model contains no elements in the component's category.");
        status = DgnDbStatus::NotFound;
        return nullptr;
        }

    //  **** GeomParts ****
    //  Create a GeomPart for each SubCategory
    bvector<bpair<DgnSubCategoryId, DgnGeomPartId>> subcatAndGeoms;
    for (auto const& entry : builders)
        {
        DgnSubCategoryId clientsubcatid = entry.first;
        ElementGeometryBuilderPtr builder = entry.second;

        Utf8String geomPartCode(solutionName);
        geomPartCode.append("|");
        geomPartCode.append(GetModelName());
        geomPartCode.append(Utf8PrintfString("|%lld", clientsubcatid.GetValue()));

        DgnGeomPartPtr geomPart = DgnGeomPart::Create(geomPartCode.c_str());
        builder->CreateGeomPart(db, true);
        builder->SetGeomStream(*geomPart);
        if (BSISUCCESS != db.GeomParts().InsertGeomPart(*geomPart))
            {
            BeAssert(false && "cannot create geompart for solution geometry -- what could have gone wrong?");
            status = DgnDbStatus::WriteError;
            return nullptr;
            }
        subcatAndGeoms.push_back(make_bpair(clientsubcatid, geomPart->GetId()));
        }

    //  **** Catalog Item ****
    PhysicalElementPtr catalogItem;
        {
        DgnElement::CreateParams cparams(db, catalogModel.GetModelId(), iclass, icode);
        dgn_ElementHandler::Element* handler = dgn_ElementHandler::Element::FindHandler(db, iclass);
        if (nullptr == handler)
            {
            BeAssert(false);
            status = DgnDbStatus::MissingHandler;
            return nullptr;
            }
        DgnElementPtr dgnElem = handler->Create(cparams);    
        if (!dgnElem.IsValid())
            {
            BeAssert(false && "Handler::Create failed");
            status = DgnDbStatus::WrongHandler;
            return nullptr;
            }
        catalogItem = dgnElem->ToPhysicalElementP();
        if (!catalogItem.IsValid())
            {
            BeAssert(false && "ComponentModel::HarvestSolution creates only PhysicalItems");
            status = DgnDbStatus::WrongClass;
            return nullptr;
            }
        }

    catalogItem->SetCategoryId(itemCategoryId); // Note that I have to set this afterward, as handler Create works in terms of DgnElement::CreateParams, not GeometricElement::CreateParams

    //  Build a single geomstream that refers to all of the geomparts -- that's the solution
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*catalogItem);
    for (bpair<DgnSubCategoryId, DgnGeomPartId> const& subcatAndGeom : subcatAndGeoms)
        {
        Transform noTransform = Transform::FromIdentity();
        builder->Append(subcatAndGeom.first);
        builder->Append(subcatAndGeom.second, noTransform);
        }

    builder->SetGeomStreamAndPlacement(*catalogItem);

    DgnElementCPtr persistentDgnElem = catalogItem->Insert(&status);
    if (!persistentDgnElem.IsValid())
        return nullptr;

    PhysicalElementCPtr persistentCatalogItem = persistentDgnElem->ToPhysicalElement();

    //  Link the catalog item to this ComponentModel
    createSolutionOfComponentRelationship(*persistentCatalogItem, *this);
    BeAssert(queryComponentModelFromSolution(db, persistentCatalogItem->GetElementId()) == GetModelId());
    return persistentCatalogItem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalElementCPtr ComponentModel::MakeInstanceOfSolution(DgnDbStatus* statusOut, PhysicalModelR targetModel, PhysicalElementCR catalogItem,
                                              DPoint3dCR origin, YawPitchRollAnglesCR angles, DgnElement::Code const& code)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(status, statusOut);
    
    DgnDbR db = catalogItem.GetDgnDb();
    if (&db != &targetModel.GetDgnDb())
        {
        status = DgnDbStatus::WrongDgnDb;
        BeAssert(false && "Catalog and target models must be in the same Db");
        return nullptr;
        }

    //  Generate the item code. This will be a null code, unless there's a specified authority for the componentmodel.
    DgnModelId mid = queryComponentModelFromSolution(db, catalogItem.GetElementId());
    if (!mid.IsValid())
        {
        status = DgnDbStatus::BadArg;
        BeAssert(false && "input catalog item must be the solution of a componentmodel");
        return nullptr;
        }
    ComponentModelPtr cmm = db.Models().Get<ComponentModel>(mid);
    if (!cmm.IsValid())
        {
        status = DgnDbStatus::BadArg;
        BeAssert(false && "input catalog item must be the solution of a componentmodel");
        return nullptr;
        }

    DgnElement::Code icode;
    if (!cmm->m_compProps.m_itemCodeAuthority.empty())  // WARNING: Don't call GetAuthority with an invalid authority name. It will always prepare a statement and will not cache the (negative) answer.
        {
        DgnAuthorityCPtr authority = db.Authorities().GetAuthority(cmm->m_compProps.m_itemCodeAuthority.c_str());
        if (authority.IsValid())
            icode = authority->CreateDefaultCode();  // *** WIP_COMPONENT_MODEL -- how do I ask an Authority to issue a code?
        }    

    //  Creating the item is just a matter of copying the catalog item
    ElementCopier copier;
    PhysicalElementCPtr inst = copier.MakeCopy(&status, targetModel, catalogItem, origin, angles, icode);
    if (!inst.IsValid())
        return nullptr;

    //  Insert InstanceOfTemplate relationship
    createInstanceOfTemplateRelationship(*inst, catalogItem);
    BeAssert(queryTemplateItemFromInstance(db, inst->GetElementId()) == catalogItem.GetElementId());

    return inst;
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
        };

    for (auto const& pair : s_pairs)
        if (0 == BeStringUtilities::Stricmp(pair.name, name))
            return pair.value;

    return T_Super::Parse(name);
    }

