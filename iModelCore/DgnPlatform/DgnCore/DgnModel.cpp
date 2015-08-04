/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnModel.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnScriptContext.h>
#include <DgnPlatform/DgnCore/DgnDbTables.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DgnModels::QueryModelId(Utf8CP name) const
    {
    CachedStatementPtr stmt;
    GetDgnDb().GetCachedStatement(stmt, "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE Name=?");
    stmt->BindText(1, name, Statement::MakeCopy::No);
    return (BE_SQLITE_ROW != stmt->Step()) ? DgnModelId() : stmt->GetValueId<DgnModelId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModels::QueryModelById(Model* out, DgnModelId id) const
    {
    Statement stmt(m_dgndb, "SELECT Name,Descr,Type,ECClassId,Space,Visibility FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE Id=?");
    stmt.BindId(1, id);

    if (BE_SQLITE_ROW != stmt.Step())
        return ERROR;

    if (out) // this can be null to just test for the existence of a model by id
        {
        out->m_id = id;
        out->m_name.AssignOrClear(stmt.GetValueText(0));
        out->m_description.AssignOrClear(stmt.GetValueText(1));
        out->m_modelType =(DgnModelType) stmt.GetValueInt(2);
        out->m_classId = DgnClassId(stmt.GetValueInt64(3));
        out->m_space =(Model::CoordinateSpace) stmt.GetValueInt(4);
        out->m_inGuiList = TO_BOOL(stmt.GetValueInt(5));
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModels::GetModelName(Utf8StringR name, DgnModelId id) const
    {
    Statement stmt(m_dgndb, "SELECT Name FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE Id=?");
    stmt.BindId(1, id);

    if (BE_SQLITE_ROW != stmt.Step())
        return  ERROR;

    name.AssignOrClear(stmt.GetValueText(0));
    return  SUCCESS;
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
BentleyStatus DgnModels::QueryModelDependencyIndexAndType(uint64_t& didx, DgnModelType& mtype, DgnModelId mid)
    {
    auto i = m_modelDependencyIndexAndType.find(mid);
    if (i != m_modelDependencyIndexAndType.end())
        {
        didx = i->second.first;
        mtype = i->second.second;
        return BSISUCCESS;
        }

    CachedStatementPtr selectDependencyIndex;
    GetDgnDb().GetCachedStatement(selectDependencyIndex, "SELECT DependencyIndex,Type FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE Id=?");
    selectDependencyIndex->BindId(1, mid);
    if (selectDependencyIndex->Step() != BE_SQLITE_ROW)
        return BSIERROR;

    didx = selectDependencyIndex->GetValueInt64(0);
    mtype =(DgnModelType)selectDependencyIndex->GetValueInt(1);
    m_modelDependencyIndexAndType[mid] = make_bpair(didx, mtype);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/11
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnModels::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString("SELECT count(*) FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE (?1 = (Visibility & ?1))", true);

    Statement sql(*m_db, sqlString.c_str());
    sql.BindInt(1,(int) m_itType);

    return (BE_SQLITE_ROW != sql.Step()) ? 0 : sql.GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModels::Iterator::const_iterator DgnModels::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString("SELECT Id,Name,Descr,Type,Space,Visibility,ECClassId FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE (?1 = (Visibility & ?1))", true);

        m_db->GetCachedStatement(m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
        m_stmt->BindInt(1,(int) m_itType);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

DgnModelId   DgnModels::Iterator::Entry::GetModelId() const {Verify(); return m_sql->GetValueId<DgnModelId>(0);}
Utf8CP       DgnModels::Iterator::Entry::GetName() const {Verify(); return m_sql->GetValueText(1);}
Utf8CP       DgnModels::Iterator::Entry::GetDescription() const {Verify(); return m_sql->GetValueText(2);}
DgnModelType DgnModels::Iterator::Entry::GetModelType() const {Verify(); return (DgnModelType) m_sql->GetValueInt(3);}
DgnModels::Model::CoordinateSpace DgnModels::Iterator::Entry::GetCoordinateSpace() const {Verify(); return (Model::CoordinateSpace) m_sql->GetValueInt(4);}
uint32_t     DgnModels::Iterator::Entry::GetVisibility() const {Verify(); return m_sql->GetValueInt(5);}
DgnClassId   DgnModels::Iterator::Entry::GetClassId() const {Verify(); return DgnClassId(m_sql->GetValueInt64(6));}

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
DgnModel::DgnModel(CreateParams const& params) : m_dgndb(params.m_dgndb), m_modelId(params.m_id), m_classId(params.m_classId), m_name(params.m_name), m_properties(params.m_props), m_solver(params.m_solver)
    {
    m_rangeIndex = nullptr;
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
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::EmptyModel()
    {
    ClearRangeIndex();

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
void DgnModel::Solver::Solve(DgnModelR model)
    {
    if (Type::Script == m_type)
        {
        DgnScriptContextR som = T_HOST.GetScriptingAdmin().GetDgnScriptContext();
        int retval;
        DgnDbStatus xstatus = som.ExecuteModelSolver(retval, model, m_name.c_str(), m_parameters);
        if (xstatus != DgnDbStatus::Success || 0 != retval)
            {
            TxnManager::ValidationError err(TxnManager::ValidationError::Severity::Fatal, "Model solver failed");   // *** NEEDS WORK: Get failure description from ModelSolver
            model.GetDgnDb().Txns().ReportError(err);
            }
        }
    else
        {
        // *** TBD: Add support for built-in constraint solvers 

        BeAssert((m_type == Solver::Type::None) && "Only Script model solvers supported");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnValidate()
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
    return element.Is3d() ? DgnDbStatus::Mismatch2d3d : DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::ReadProperties()
    {
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
    for (auto entry=m_appData.begin(); entry!=m_appData.end(); ++entry)
        {
        DgnDbStatus stat = entry->second->_OnUpdate(*this);
        if (DgnDbStatus::Success != stat)
            return stat;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::Update()
    {
    DgnDbStatus stat = _OnUpdate();
    if (stat != DgnDbStatus::Success)
        return stat;

    Json::Value propJson(Json::objectValue);
    _ToPropertiesJson(propJson);
    Utf8String val = Json::FastWriter::ToString(propJson);

    Statement stmt(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Model) " SET Props=?, Solver=? WHERE Id=?");
    stmt.BindText(1, val, Statement::MakeCopy::No);
    if (m_solver.IsValid())
        stmt.BindText(2, m_solver.ToJson(), Statement::MakeCopy::Yes);
    else
        stmt.BindNull(2);
    stmt.BindId(3, m_modelId);

    auto rc=stmt.Step();
    if (rc!= BE_SQLITE_DONE)
        return DgnDbStatus::WriteError;

    _OnUpdated();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Allocate and initialize a range tree for this model.
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::AllocateRangeIndex() const
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
void DgnModel::ClearRangeIndex()
    {
    DELETE_AND_CLEAR(m_rangeIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::AddToRangeIndex(DgnElementCR element)
    {
    if (nullptr == m_rangeIndex)
        return;

    GeometricElementCP geom = element._ToGeometricElement();
    if (nullptr != m_rangeIndex && nullptr != geom && geom->HasGeometry())
        m_rangeIndex->AddGeomElement(*geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::RemoveFromRangeIndex(DgnElementCR element)
    {
    if (nullptr==m_rangeIndex)
        return;

    GeometricElementCP geom = element._ToGeometricElement();
    if (nullptr != geom && geom->HasGeometry())
        m_rangeIndex->RemoveElement(DgnRangeTree::Entry(geom->CalculateRange3d(), *geom));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::UpdateRangeIndex(DgnElementCR modified, DgnElementCR original)
    {
    if (nullptr==m_rangeIndex)
        return;

    GeometricElementCP origGeom = original._ToGeometricElement();
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
void DgnModel::RegisterElement(DgnElementCR element)
    {
    if (m_filled)
        m_elements.Add(element);

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
    return m_dgndb.IsReadonly() ? DgnDbStatus::ReadOnly : DgnDbStatus::Success;
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
DgnDbStatus DgnModel::_OnDeleteElement(DgnElementCR element)
    {
    return m_dgndb.IsReadonly() ? DgnDbStatus::ReadOnly : DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnDeletedElement(DgnElementCR element)
    {
    RemoveFromRangeIndex(element);
    if (m_filled)
        m_elements.erase(element.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_OnUpdateElement(DgnElementCR modified, DgnElementCR original)
    {
    return m_dgndb.IsReadonly() ? DgnDbStatus::ReadOnly : DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnUpdatedElement(DgnElementCR modified, DgnElementCR original)
    {
    UpdateRangeIndex(modified, original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRangeTreeP DgnModel::GetRangeIndexP(bool create) const
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

    if (!DgnModels::IsValidName(m_name))
        {
        BeAssert(false);
        return DgnDbStatus::InvalidName;
        }

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

    m_name.Trim();
    if (models.QueryModelId(m_name.c_str()).IsValid()) // can't allow two models with the same name
        return DgnDbStatus::DuplicateName;

    DbResult rc = m_dgndb.GetNextRepositoryBasedId(m_modelId, DGN_TABLE(DGN_CLASSNAME_Model), "Id");
    BeAssert(rc == BE_SQLITE_OK);

    Statement stmt(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Model) "(Id,Name,Descr,Type,ECClassId,Space,Visibility) VALUES(?,?,?,?,?,?,?)");
    stmt.BindId(1, m_modelId);
    stmt.BindText(2, m_name.c_str(), Statement::MakeCopy::No);
    stmt.BindText(3, description, Statement::MakeCopy::No);
    stmt.BindInt(4, (int)_GetModelType());
    stmt.BindId(5, GetClassId());
    stmt.BindInt(6,(int) _GetCoordinateSpace());
    stmt.BindInt(7, inGuiList);

    rc = stmt.Step();
    if (BE_SQLITE_DONE != rc)
        {
        BeAssert(false);
        m_modelId = DgnModelId();
        return DgnDbStatus::WriteError;
        }

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

    m_solver = other.m_solver;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnModel::Clone(Utf8CP newName) const
    {
    DgnModelPtr newModel = GetModelHandler().Create(DgnModel::CreateParams(m_dgndb, m_classId, newName));
    newModel->_InitFrom(*this);
    return newModel;
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

    DgnModel::CreateParams params(m_dgndb, model.GetClassId(), model.GetName().c_str(), DgnModel::Properties(), DgnModel::Solver(), modelId);
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

    if (!tmpStr.empty() && !m_dgndb.Models().QueryModelId(tmpStr.c_str()).IsValid())
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
        } while (m_dgndb.Models().QueryModelId(uniqueModelName.c_str()).IsValid());

    return uniqueModelName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DgnModels::QueryFirstModelId() const
    {
    return MakeIterator().begin().GetModelId();
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
DPoint3d DgnModel::_GetGlobalOrigin() const
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

    enum Column : int {Id=0,ClassId=1,CategoryId=2,Label=3,Code=4,ParentId=5};
    Statement stmt(m_dgndb, "SELECT Id,ECClassId,CategoryId,Label,Code,ParentId FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE ModelId=?");
    stmt.BindId(1, m_modelId);

    SetFilled();

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
            stmt.GetValueId<DgnCategoryId>(Column::CategoryId), 
            stmt.GetValueText(Column::Label), 
            stmt.GetValueText(Column::Code), 
            id,
            stmt.GetValueId<DgnElementId>(Column::ParentId)), true);
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
AxisAlignedBox3d DgnModel::_QueryModelRange() const
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

// *** Persistent values *** Do not change ***
#define SOLVER_TYPE_NONE    0
#define SOLVER_TYPE_SCRIPT  1

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnModel::Solver::ToJson() const
    {
    int tval;
    switch (m_type)
        { 
        case Type::None:   tval = SOLVER_TYPE_NONE;   break;
        case Type::Script: tval = SOLVER_TYPE_SCRIPT; break;
        default:
            tval = 0;
            BeAssert(false);
        }

    Json::Value json (Json::objectValue);
    json["Type"] = tval;
    json["Name"] = m_name.c_str();
    json["Parameters"] = m_parameters;

    return Json::FastWriter::ToString(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::Solver::FromJson(Utf8CP str)
    {
    //  Parse
    Json::Value json(Json::objectValue);
    if (!Json::Reader::Parse(str, json))
        {
        BeAssert(false);
        return;
        }

    //  Validate content
    if (!json.isMember("Type") || !json.isMember("Name") || !json.isMember("Parameters"))
        {
        BeAssert(false);
        return;
        }

    switch (json["Type"].asInt())
        { 
        case SOLVER_TYPE_NONE:   m_type = Type::None;   break;
        case SOLVER_TYPE_SCRIPT: m_type = Type::Script; break;
        default:
            m_type = Type::None;
            BeAssert(false);
        }
    
    //  Extract simple properties
    m_name = json["Name"].asCString();
    m_parameters = json["Parameters"];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
EC::ECInstanceId DgnComponentSolutions::QuerySolutionId(DgnModelId cmid, Utf8StringCR solutionName)
    {
    CachedStatementPtr stmt = GetDgnDb().GetCachedStatement("SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_ComponentModelSolution) " WHERE(ComponentModelId=? AND Parameters=?)");
    stmt->BindId(1, cmid);
    stmt->BindText(2, solutionName, Statement::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt->Step())
        return EC::ECInstanceId();
    return stmt->GetValueId<EC::ECInstanceId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnComponentSolutions::Query(Solution& sln, EC::ECInstanceId sid)
    {
    CachedStatementPtr stmt = GetDgnDb().GetCachedStatement("SELECT ComponentModelId,Parameters FROM " DGN_TABLE(DGN_CLASSNAME_ComponentModelSolution) " WHERE(Id=?)");
    stmt->BindId(1, sid);
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::NotFound;
    sln.m_id = sid;
    sln.m_componentModelId = stmt->GetValueId<DgnModelId>(0);
    sln.m_parameters = stmt->GetValueText(1);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnComponentSolutions::Solution::QueryGeomStream(GeomStreamR geom, DgnDbR db) const
    {
    return geom.ReadGeomStream(db, DGN_TABLE(DGN_CLASSNAME_ComponentModelSolution), "Geom", m_id.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnComponentSolutions::ComputeSolutionName(Json::Value const& parms)
    {
    return Json::FastWriter::ToString(parms).c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
EC::ECInstanceId DgnComponentSolutions::CaptureSolution(ComponentModelR componentModel)
    {
    if (&componentModel.GetDgnDb() != &GetDgnDb())
        {
        BeAssert(false && "you must import the component model before you can capture a solution");
        return EC::ECInstanceId();
        }

    Utf8String solutionCode = ComputeSolutionName(componentModel.GetSolver().GetParameters());

    EC::ECInstanceId existingsolution = QuerySolutionId(componentModel.GetModelId(), solutionCode);
    if (existingsolution.IsValid())
        return existingsolution;

    DgnCategoryId componentCategoryId = GetDgnDb().Categories().QueryCategoryId(componentModel.GetElementCategoryName().c_str());
    if (!componentCategoryId.IsValid())
        {
        BeAssert(false && "component category not found -- you must import the component model before you can capture a solution");
        return EC::ECInstanceId();
        }

    //  Accumulate geometry by SubCategory
    bmap<DgnSubCategoryId, ElementGeometryBuilderPtr> builders;     // *** TBD: add another dimension: break out builders by same ElemDisplayParams
    componentModel.FillModel();
    for (auto const& mapEntry : componentModel)
        {
        GeometricElementCP componentElement = mapEntry.second->ToGeometricElement();
        if (nullptr == componentElement)
            continue;

        //  Only solution elements in the component's Category are collected. The rest are construction/annotation geometry.
        if (componentElement->GetCategoryId() != componentCategoryId)
            continue;

        // *** NEEDS WORK: Detect, schedule, and skip instances of other CM's
        ElementGeometryCollection gcollection(*componentElement);
        for (ElementGeometryPtr const& geom : gcollection)
            {
            //  Look up the subcategory ... IN THE CLIENT DB
            ElemDisplayParamsCR dparams = gcollection.GetElemDisplayParams();
            DgnSubCategoryId clientsubcatid = dparams.GetSubCategoryId();

            ElementGeometryBuilderPtr& builder = builders[clientsubcatid];
            if (!builder.IsValid())
                builder = ElementGeometryBuilder::CreateGeomPart(GetDgnDb(), true);

            // Since each little piece of geometry can have its own transform, we must
            // build the transforms back into them in order to assemble them into a single geomstream.
            // It's all relative to 0,0,0 in the component model, so it's fine to do this.
            ElementGeometryPtr xgeom = geom->Clone();
            Transform trans = gcollection.GetGeometryToWorld(); // A component model is in its own local coordinate system, so "World" just means relative to local 0,0,0
            xgeom->TransformInPlace(trans);

            builder->Append(*xgeom);
            }
        }
    
    //  Create a GeomPart for each SubCategory
    bvector<bpair<DgnSubCategoryId,DgnGeomPartId>> subcatAndGeoms;
    for (auto const& entry : builders)
        {
        DgnSubCategoryId clientsubcatid = entry.first;
        ElementGeometryBuilderPtr builder = entry.second;

        Utf8String geomPartCode (solutionCode);
        geomPartCode.append(Utf8PrintfString("_%lld", clientsubcatid.GetValue()));

        DgnGeomPartPtr geomPart = DgnGeomPart::Create(geomPartCode.c_str());
        builder->CreateGeomPart(GetDgnDb(), true);
        builder->SetGeomStream(*geomPart);
        if (BSISUCCESS != GetDgnDb().GeomParts().InsertGeomPart(*geomPart))
            return EC::ECInstanceId();

        subcatAndGeoms.push_back(make_bpair(clientsubcatid, geomPart->GetId()));
        }

    //  The solution is a geomstream that refers to the geomparts
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(componentModel, componentCategoryId, DPoint3d::FromZero(), YawPitchRollAngles());
    for (bpair<DgnSubCategoryId,DgnGeomPartId> const& subcatAndGeom : subcatAndGeoms)
        {
        Transform noTransform = Transform::FromIdentity();
        builder->Append(subcatAndGeom.first);
        builder->Append(subcatAndGeom.second, noTransform);
        }


    GeomStream  geom;
    if (builder->SetGeomStream(geom) != BSISUCCESS)
        return EC::ECInstanceId();
        
    CachedStatementPtr istmt = GetDgnDb().GetCachedStatement("INSERT INTO " DGN_TABLE(DGN_CLASSNAME_ComponentModelSolution) " (ComponentModelId,Parameters) VALUES(?,?)");
    istmt->BindId(1, componentModel.GetModelId());
    istmt->BindText(2, solutionCode, Statement::MakeCopy::No);
    if (BE_SQLITE_DONE != istmt->Step())
        return EC::ECInstanceId();
    istmt = nullptr;

    EC::ECInstanceId sid = EC::ECInstanceId(GetDgnDb().GetLastInsertRowId());

    CachedStatementPtr gstmt = GetDgnDb().GetCachedStatement("UPDATE " DGN_TABLE(DGN_CLASSNAME_ComponentModelSolution) " SET Geom=? WHERE Id=?");
    gstmt->BindId(2, sid);
    geom.WriteGeomStreamAndStep(GetDgnDb(), DGN_TABLE(DGN_CLASSNAME_ComponentModelSolution), "Geom", sid.GetValue(), *gstmt, 1);
    return sid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalElementPtr DgnComponentSolutions::CreateSolutionInstance(DgnModelR destinationModel, Utf8CP componentSchemaName, BeSQLite::EC::ECInstanceId solutionId, DPoint3dCR origin, YawPitchRollAnglesCR angles)
    {
    Solution sln;
    if (DgnDbStatus::Success != Query(sln, solutionId))
        return nullptr;

    RefCountedPtr<ComponentModel> cm = GetDgnDb().Models().Get<ComponentModel>(sln.m_componentModelId);
    if (!cm.IsValid())
        return nullptr;

    DgnClassId cmClassId = DgnClassId(GetDgnDb().Schemas().GetECClassId(componentSchemaName, cm->ComputeElementECClassName().c_str()));
    DgnCategoryId cmCategoryId = GetDgnDb().Categories().QueryCategoryId(cm->GetElementCategoryName().c_str());

    GeomStream geom;
    sln.QueryGeomStream(geom, GetDgnDb());

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(destinationModel, cmCategoryId, origin, angles);
    builder->SetGeomStream(geom);

    PhysicalElementPtr instance = PhysicalElement::Create(PhysicalElement::CreateParams(GetDgnDb(), destinationModel.GetModelId(), cmClassId, cmCategoryId));
    builder->SetGeomStreamAndPlacement(*instance);

    return instance;
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
    CreateParams parms(importer.GetDestinationDb(), GetClassId(), GetModelName(), GetProperties(), GetSolver());
    if (importer.IsBetweenDbs())
        {
        // Caller probably wants to preserve these when copying between Dbs. We *never* preserve them when copying within a Db.
        parms.m_name = GetModelName();

        parms.RelocateToDestinationDb(importer);
        }
    return parms;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnModel::_Clone(DgnDbStatus* stat, DgnModel::CreateParams const& params) const
    {
    if (nullptr != stat)
        *stat = DgnDbStatus::Success;
    DgnModelPtr model = GetModelHandler().Create(params);
    if (!model.IsValid())
        return nullptr;

    Json::Value propJson(Json::objectValue);
    _ToPropertiesJson(propJson);
    model->_FromPropertiesJson(propJson);
    return model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::_ImportContentsFrom(DgnModelCR sourceModel, DgnImportContext& importer)
    {
    BeAssert(&GetDgnDb() == &importer.GetDestinationDb());
    BeAssert(&sourceModel.GetDgnDb() == &importer.GetSourceDb());

    Statement stmt(sourceModel.GetDgnDb(), "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE ModelId=?");
    stmt.BindId(1, m_modelId);
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnElementCPtr el = sourceModel.GetDgnDb().Elements().GetElement(stmt.GetValueId<DgnElementId>(0));
        DgnElement::CreateParams ccparms = el->GetCreateParamsForImport(importer);
        DgnDbStatus status;
        DgnElementCPtr cc = el->Import(&status, ccparms, importer);
        if (!cc.IsValid() || (DgnDbStatus::Success != status))
            {
            // *** TBD: Record failure somehow
            continue;
            }
        if (DgnDbStatus::Success != status)
            {
            // *** TBD: Record failure somehow
            continue;
            }
        }

    // *** TBD: Copy ECRelationships where source and target are both in this model, and where the relationship is implemented as a link table.
    //              Note: this requires domain-specific knowledge of what ECRelationships exist.
    //              Examples from Dgn domain include ElementDrivesElement
    
    // *** TBD: Copy ECRelationships where element in this model uses a shared resource that is deep-copied.
    //              Examples from Dgn domain include ElementGeomUsesParts, ElementUsesStyles

    // *** TBD: ElementHasLinks -- should we deep-copy links?

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnModel::ImportModel(DgnDbStatus* statIn, CreateParams const& paramsIn, DgnModelCR sourceModel, DgnImportContext& importer)
    {
    DgnDbStatus _stat;
    DgnDbStatus& stat = (nullptr != statIn)? *statIn: _stat;

    BeAssert(&sourceModel.GetDgnDb() == &importer.GetSourceDb());
    BeAssert(&paramsIn.m_dgndb == &importer.GetDestinationDb());

    CreateParams params = paramsIn;

    if (params.m_name.empty())
        params.m_name = sourceModel.GetModelName();

    if (params.m_dgndb.Models().QueryModelId(params.m_name.c_str()).IsValid()) // Is the name already used in destination?
        {
        stat = DgnDbStatus::DuplicateName;
        return nullptr;
        }

    DgnModelPtr newModel = sourceModel._Clone(&stat, params);
    if (!newModel.IsValid())
        {
        stat = DgnDbStatus::BadRequest;
        return nullptr;
        }

    if ((stat = newModel->Insert()) != DgnDbStatus::Success)
        return nullptr;

    if ((stat = newModel->_ImportContentsFrom(sourceModel, importer)) != DgnDbStatus::Success)
        return nullptr;

    stat = DgnDbStatus::Success;
    return newModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::ImportSchema(DgnDbR db, BeFileNameCR schemaFile)
    {
    ECSchemaReadContextPtr contextPtr = ECSchemaReadContext::CreateContext();
    contextPtr->AddSchemaLocater(db.GetSchemaLocater());
    contextPtr->AddSchemaPath(schemaFile.GetDirectoryName().GetName());

    ECSchemaPtr schemaPtr;
    SchemaReadStatus readSchemaStatus = ECSchema::ReadFromXmlFile(schemaPtr, schemaFile.GetName(), *contextPtr);
    if (SCHEMA_READ_STATUS_Success != readSchemaStatus)
        return DgnDbStatus::ReadError;

    if (BentleyStatus::SUCCESS != db.Schemas().ImportECSchemas(contextPtr->GetCache()))
        return DgnDbStatus::BadSchema;

    db.Domains().SyncWithSchemas();

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::GenerateECClass(ECN::ECSchemaR schema)
    {
    ECN::ECClassP ecclass;
    auto stat = schema.CreateClass(ecclass, ComputeElementECClassName());
    if (ECN::ECOBJECTS_STATUS_Success != stat)
        return DgnDbStatus::BadRequest;

    schema.AddReferencedSchema(*const_cast<ECN::ECSchemaP>(GetDgnDb().Schemas().GetECSchema(DGN_ECSCHEMA_NAME)), "dgn");
    ecclass->AddBaseClass(*GetDgnDb().Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement));

    Json::Value const& parms = m_solver.GetParameters();
    for (Utf8String const& parmname : parms.getMemberNames())
        {
        ECN::PrimitiveECPropertyP ecprop;
        stat = ecclass->CreatePrimitiveProperty(ecprop, parmname.c_str());
        if (ECN::ECOBJECTS_STATUS_Success != stat)
            return DgnDbStatus::BadRequest;
        Json::Value const& parmvalue = parms[parmname.c_str()];
        if (parmvalue.isBool())
            ecprop->SetType(ECN::PRIMITIVETYPE_Boolean);
        else if (parmvalue.isIntegral())
            ecprop->SetType(ECN::PRIMITIVETYPE_Long);
        else if (parmvalue.isDouble())
            ecprop->SetType(ECN::PRIMITIVETYPE_Double);
        else if (parmvalue.isString())
            ecprop->SetType(ECN::PRIMITIVETYPE_String);
        else
            {
            BeAssert(false);
            return DgnDbStatus::BadSchema;
            }
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::AddAllToECSchema(ECN::ECSchemaR schema, DgnDbR db)
    {
    for (auto const& cm : db.Models().MakeIterator())
        {
        if (cm.GetModelType() != DgnModelType::Component)
            continue;
        ComponentModelPtr componentModel = db.Models().Get<ComponentModel>(cm.GetModelId());
        if (DgnDbStatus::Success != componentModel->GenerateECClass(schema))
            return DgnDbStatus::WriteError;
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentModel::GetElementCategoryName() const
    {
    return m_elementCategoryName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentModel::ComputeElementECClassName() const
    {
    return GetModelName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::_ToPropertiesJson(Json::Value& json) const
    {
    T_Super::_ToPropertiesJson(json);
    json["ComponentElementCategoryName"] = m_elementCategoryName.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::_FromPropertiesJson(Json::Value const& json)
    {
    T_Super::_FromPropertiesJson(json);
    m_elementCategoryName = json["ComponentElementCategoryName"].asCString();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::Solve(Json::Value const& parms)
    {
    GetSolverParametersR() = parms;
    auto status = Update();
    if (DgnDbStatus::Success != status)
        return status;
    // SaveChanges triggers validation, which invokes the model's solver
    auto sstatus = GetDgnDb().SaveChanges();
    if (BE_SQLITE_OK == sstatus)
        return DgnDbStatus::Success;
    if (BE_SQLITE_ERROR_ChangeTrackError == sstatus)
        return DgnDbStatus::ValidationFailed;
    return DgnDbStatus::SQLiteError;
    }