/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnModel.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
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
    Statement stmt(m_dgndb, "SELECT Name,Descr,Type,ECClassId,Visibility FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE Id=?");
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
        out->m_inGuiList = TO_BOOL(stmt.GetValueInt(4));
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
        Utf8String sqlString = "SELECT Id,Name,Descr,Type,Visibility,ECClassId FROM " DGN_TABLE(DGN_CLASSNAME_Model);
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
Utf8CP       DgnModels::Iterator::Entry::GetName() const {Verify(); return m_sql->GetValueText(1);}
Utf8CP       DgnModels::Iterator::Entry::GetDescription() const {Verify(); return m_sql->GetValueText(2);}
DgnModelType DgnModels::Iterator::Entry::GetModelType() const {Verify(); return (DgnModelType) m_sql->GetValueInt(3);}
bool         DgnModels::Iterator::Entry::InGuiList() const {Verify(); return (0 != m_sql->GetValueInt(4));}
DgnClassId   DgnModels::Iterator::Entry::GetClassId() const {Verify(); return DgnClassId(m_sql->GetValueInt64(5));}

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

    Statement stmt(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Model) " SET Props=?,Solver=? WHERE Id=?");
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
void DgnModel::_OnReversedDeleteElement(DgnElementCR el) 
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
    return m_dgndb.IsReadonly() ? DgnDbStatus::ReadOnly : DgnDbStatus::Success;
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

    m_modelId.Invalidate();
    m_modelId.ToNextAvailable(m_dgndb, DGN_TABLE(DGN_CLASSNAME_Model), "Id");

    Statement stmt(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Model) "(Id,Name,Descr,Type,ECClassId,Visibility,Space) VALUES(?,?,?,?,?,?,?)");
    stmt.BindId(1, m_modelId);
    stmt.BindText(2, m_name.c_str(), Statement::MakeCopy::No);
    stmt.BindText(3, description, Statement::MakeCopy::No);
    stmt.BindInt(4, (int)_GetModelType());
    stmt.BindId(5, GetClassId());
    stmt.BindInt(6, inGuiList ? 1 : 0);
    auto geomModel = ToGeometricModel();
    if (nullptr != geomModel)
        stmt.BindInt(7, (int)geomModel->GetCoordinateSpace());
    else
        stmt.BindNull(7);

    auto rc = stmt.Step();
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

    DgnModel::CreateParams params(m_dgndb, model.GetClassId(), model.GetName().c_str(), DgnModel::Properties(), ModelSolverDef(), modelId);
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

    enum Column : int {Id=0,ClassId=1,CategoryId=2,Label=3,Code=4,ParentId=5,CodeAuthorityId=6,CodeNameSpace=7};
    Statement stmt(m_dgndb, "SELECT Id,ECClassId,CategoryId,Label,Code,ParentId,CodeAuthorityId,CodeNameSpace FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE ModelId=?");
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
            stmt.GetValueId<DgnCategoryId>(Column::CategoryId), 
            stmt.GetValueText(Column::Label), 
            DgnElement::Code(stmt.GetValueId<DgnAuthorityId>(Column::CodeAuthorityId), stmt.GetValueText(Column::Code), stmt.GetValueText(Column::CodeNameSpace)), 
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
DgnDbStatus ComponentSolution::Query(Solution& sln, SolutionId sid)
    {
    //DgnModelId cmid = GetDgnDb().Models().QueryModelId(sid.m_modelName.c_str());

    CachedStatementPtr stmt = GetDgnDb().GetCachedStatement("SELECT Id,Range,Parameters FROM " DGN_TABLE(DGN_CLASSNAME_ComponentSolution) " WHERE ComponentModelName=? AND SolutionName=?");
    stmt->BindText(1, sid.m_modelName, Statement::MakeCopy::No);
    stmt->BindText(2, sid.m_solutionName, Statement::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::NotFound;
    sln.m_id = sid;
    sln.m_rowId = stmt->GetValueInt64(0);
    sln.m_range = *(ElementAlignedBox3d*)stmt->GetValueBlob(1);
    Utf8CP parametersJsonSerialized = stmt->GetValueText(2);
    Json::Value parametersJson(Json::objectValue);
    if (!Json::Reader::Parse(parametersJsonSerialized, parametersJson))
        return DgnDbStatus::ReadError;
    sln.m_parameters = ModelSolverDef::ParameterSet(parametersJson);
        
    sln.m_componentModelId = GetDgnDb().Models().QueryModelId(sid.m_modelName.c_str());
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentSolution::Solution::QueryGeomStream(GeomStreamR geom, DgnDbR db) const
    {
    return geom.ReadGeomStream(db, DGN_TABLE(DGN_CLASSNAME_ComponentSolution), "Geom", m_rowId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentSolution::SolutionId ComponentModel::ComputeSolutionId(ModelSolverDef::ParameterSet const& params)
    {
    return ComponentSolution::SolutionId(GetModelName(), params.ComputeSolutionName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentSolution::SolutionId ComponentSolution::CaptureSolution(ComponentModelR componentModel)
    {
    if (&componentModel.GetDgnDb() != &GetDgnDb())
        {
        BeAssert(false && "you must import the component model before you can capture a solution");
        return SolutionId();
        }

    ModelSolverDef::ParameterSet const& currentParameters = componentModel.GetSolver().GetParameters();
    Utf8String solutionName = currentParameters.ComputeSolutionName();

    SolutionId sid;
    sid.m_modelName = componentModel.GetModelName();
    sid.m_solutionName = solutionName;

    Solution sln;
    if (DgnDbStatus::Success == Query(sln, sid)) // see if this solution is already cached.
        return sid;

    DgnCategoryId componentCategoryId = GetDgnDb().Categories().QueryCategoryId(componentModel.GetElementCategoryName().c_str());
    if (!componentCategoryId.IsValid())
        {
        BeAssert(false && "component category not found -- you must import the component model before you can capture a solution");
        return SolutionId();
        }

    //  Gather geometry by SubCategory
    bmap<DgnSubCategoryId, ElementGeometryBuilderPtr> builders;     // *** WIP_IMPORT: add another dimension: break out builders by same ElemDisplayParams
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
    
    if(builders.empty())
        {
        BeDataAssert(false && "Component model contains no elements in the component's category.");
        return SolutionId();
        }

    //  Create a GeomPart for each SubCategory
    bvector<bpair<DgnSubCategoryId,DgnGeomPartId>> subcatAndGeoms;
    for (auto const& entry : builders)
        {
        DgnSubCategoryId clientsubcatid = entry.first;
        ElementGeometryBuilderPtr builder = entry.second;

        Utf8String geomPartCode (solutionName);
        geomPartCode.append(Utf8PrintfString("_%lld", clientsubcatid.GetValue()));

        DgnGeomPartPtr geomPart = DgnGeomPart::Create(geomPartCode.c_str());
        builder->CreateGeomPart(GetDgnDb(), true);
        builder->SetGeomStream(*geomPart);
        if (BSISUCCESS != GetDgnDb().GeomParts().InsertGeomPart(*geomPart))
            {
            BeAssert(false && "cannot create geompart for solution geometry -- what could have gone wrong?");
            return SolutionId();
            }
        subcatAndGeoms.push_back(make_bpair(clientsubcatid, geomPart->GetId()));
        }

    //  Build a single geomstream that refers to all of the geomparts -- that's the solution
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(componentModel, componentCategoryId, DPoint3d::FromZero(), YawPitchRollAngles());
    for (bpair<DgnSubCategoryId,DgnGeomPartId> const& subcatAndGeom : subcatAndGeoms)
        {
        Transform noTransform = Transform::FromIdentity();
        builder->Append(subcatAndGeom.first);
        builder->Append(subcatAndGeom.second, noTransform);
        }

    GeomStream  geom;
    Placement3d placement;
    if (builder->GetGeomStream(geom) != BSISUCCESS || builder->GetPlacement(placement) != BSISUCCESS)
        {
        BeAssert(false);
        return SolutionId();
        }

    ElementAlignedBox3d box = placement.GetElementBox();
        
    //  Insert a row into the ComponentSolution table that is keyed by the solutionId and that captures the solution geometry
    CachedStatementPtr istmt = GetDgnDb().GetCachedStatement("INSERT INTO " DGN_TABLE(DGN_CLASSNAME_ComponentSolution) " (ComponentModelName,SolutionName,Parameters,Range) VALUES(?,?,?,?)");
    istmt->BindText(1, componentModel.GetModelName(), Statement::MakeCopy::No);
    istmt->BindText(2, solutionName, Statement::MakeCopy::No);
    istmt->BindText(3, Json::FastWriter::ToString(currentParameters.ToJson()).c_str(), Statement::MakeCopy::Yes);
    istmt->BindBlob(4, &box, sizeof(box), Statement::MakeCopy::No);
    if (BE_SQLITE_DONE != istmt->Step())
        return SolutionId();
    istmt = nullptr;

    uint64_t solutionRowId = GetDgnDb().GetLastInsertRowId();
    
    sid.m_modelName = componentModel.GetModelName();
    sid.m_solutionName = solutionName;

    CachedStatementPtr gstmt = GetDgnDb().GetCachedStatement("UPDATE " DGN_TABLE(DGN_CLASSNAME_ComponentSolution) " SET Geom=? WHERE Id=?");
    gstmt->BindInt64(2, solutionRowId);
    geom.WriteGeomStreamAndStep(GetDgnDb(), DGN_TABLE(DGN_CLASSNAME_ComponentSolution), "Geom", solutionRowId, *gstmt, 1);
    return sid;
    }

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
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr ComponentSolution::CreateSolutionInstanceElement(DgnModelR destinationModel, SolutionId solutionId, DPoint3dCR origin, YawPitchRollAnglesCR angles)
    {
    Solution sln;
    if (DgnDbStatus::Success != Query(sln, solutionId))
        return nullptr;

    RefCountedPtr<ComponentModel> cm = GetDgnDb().Models().Get<ComponentModel>(sln.m_componentModelId);
    if (!cm.IsValid())
        return nullptr;

    Utf8String schemaname, classname;
    std::tie(schemaname, classname) = parseFullECClassName(cm->GetElementECClassName().c_str());
    DgnClassId cmClassId = DgnClassId(GetDgnDb().Schemas().GetECClassId(schemaname.c_str(), classname.c_str()));
    DgnCategoryId cmCategoryId = GetDgnDb().Categories().QueryCategoryId(cm->GetElementCategoryName().c_str());
    
    auto geomDestModel = destinationModel.ToGeometricModel();
    BeAssert (nullptr != geomDestModel);
    if (nullptr == geomDestModel)
        return nullptr;

    if (geomDestModel->Is3d())
        return PhysicalElement::Create(PhysicalElement::CreateParams(GetDgnDb(), destinationModel.GetModelId(), cmClassId, cmCategoryId, 
                                            Placement3d(origin, angles, ElementAlignedBox3d())));

    return DrawingElement::Create(DrawingElement::CreateParams(GetDgnDb(), destinationModel.GetModelId(), cmClassId, cmCategoryId, 
                                            Placement2d(DPoint2d::From(origin.x,origin.y), angles.GetYaw(), ElementAlignedBox2d())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentSolution::CreateSolutionInstanceItem(DgnElementR instanceElement, ECN::IECInstancePtr& itemProperties, SolutionId solutionId)
    {
    DgnDbR db = instanceElement.GetDgnDb();

    //  The the details of the solution and the class of the Item that should be created
    ComponentSolution solutions(db);
    Solution sln;
    DgnDbStatus status = solutions.Query(sln, solutionId);
    if (DgnDbStatus::Success != status)
        return status;

    RefCountedPtr<ComponentModel> cm = db.Models().Get<ComponentModel>(sln.m_componentModelId);
    if (!cm.IsValid())
        return DgnDbStatus::NotFound;

    Utf8String componentSchemaName = cm->GetItemECSchemaName();
    DgnClassId cmItemClassId = DgnClassId(db.Schemas().GetECClassId(componentSchemaName.c_str(), cm->GetItemECClassName().c_str()));
    if (!cmItemClassId.IsValid())
        {
        BeAssert(false && "you must store the name of the generated ECSchema in the ComponentModel");
        return DgnDbStatus::WrongClass;
        }

    // Make and return a standalone instance of the itemClass that holds the parameters as properties. It is up to the caller to transfer these properties to the DgnElement::Item object.
    auto itemClass = db.Schemas().GetECClass(cmItemClassId.GetValue());
    itemProperties = itemClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ModelSolverDef::ParameterSet const& params = sln.GetParameters();
    for (auto param : params)
        {
        if (ECN::ECOBJECTS_STATUS_Success != itemProperties->SetValue(param.GetName().c_str(), param.GetValue()))
            {
            BeDataAssert(false);
            return DgnDbStatus::WrongClass;
            }
        }

    //  Create and set on the element a DgnElement::Item object based on the itemClass. This will have the effect of scheduling an insert of the item when the element is inserted.
    //  Note that we cannot set up the "properties" of this DgnElement::Item object, because we don't know how it stores properties, except in one special case (below).
    dgn_AspectHandler::Aspect* handler = dgn_AspectHandler::Aspect::FindHandler(db, cmItemClassId);
    if (nullptr == handler)
        return DgnDbStatus::WrongClass;

    RefCountedPtr<DgnElement::Item> item = dynamic_cast<DgnElement::Item*>(handler->_CreateInstance().get());
    if (!item.IsValid())
        {
        auto ibi = new InstanceBackedItem;
        ibi->m_instance = itemProperties;       // In this special case, we can set the properties of the DgnElement::Item object.
        item = ibi;
        }

    DgnElement::Item::SetItem(instanceElement, *item);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus BentleyApi::Dgn::ExecuteComponentSolutionEGA(DgnElementR el, DPoint3dCR origin, YawPitchRollAnglesCR angles, ECN::IECInstanceCR itemInstance, Utf8StringCR cmName, Utf8StringCR paramNames, DgnElement::Item& item)
    {
    DgnDbR db = el.GetDgnDb();

    // Look up the ComponentModel that itemInstance is based on
    ComponentModelPtr cm = db.Models().Get<ComponentModel>(db.Models().QueryModelId(cmName.c_str()));
    if (!cm.IsValid())
        {
        BeDataAssert(false);
        DGNCORELOG->warningv("generated component model item [class %s] names a component model [%s] that is not in the target DgnDb", itemInstance.GetClass().GetName().c_str(), cmName.c_str());
        return DgnDbStatus::NotFound;
        }

    //  Look up the captured solution that itemInstance is based on
    ModelSolverDef::ParameterSet parms = cm->GetSolver().GetParameters();
    parms.SetValuesFromECProperties(itemInstance); // Set the parameter values to what I have saved
    ComponentSolution solutions(db);
    ComponentSolution::Solution sln;
    if (DgnDbStatus::Success != solutions.Query(sln, cm->ComputeSolutionId(parms)))
        return DgnDbStatus::NotFound;

    //  Copy the geometry from the captured solution to the element
    GeomStream geom;
    sln.QueryGeomStream(geom, db);

    DgnElement3dP e3d = el.ToElement3dP();
    if (nullptr != e3d)
        {
        Placement3d placement(origin, angles, sln.GetRange());
        e3d->SetPlacement(placement);
        e3d->GetGeomStreamR().SaveData (geom.GetData(), geom.GetSize());
        return DgnDbStatus::Success;
        }
        
    DgnElement2dP e2d = el.ToElement2dP();
    if (nullptr == e2d)
        {
        ElementAlignedBox3dCR range3d = sln.GetRange();
        ElementAlignedBox2d range2d(range3d.low.x, range3d.low.y, range3d.high.x, range3d.high.y);
        Placement2d placement(DPoint2d::From(origin.x,origin.y), angles.GetYaw(), range2d);
        e2d->SetPlacement(placement);
        e2d->GetGeomStreamR().SaveData (geom.GetData(), geom.GetSize());
        return DgnDbStatus::Success;
        }

    return DgnDbStatus::BadElement;
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
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnModel::Clone(Utf8CP newName) const
    {
    DgnModelPtr newModel = GetModelHandler().Create(DgnModel::CreateParams(m_dgndb, m_classId, newName));
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

    if (params.m_name.empty())
        params.m_name = GetModelName();

    if (importer.GetDestinationDb().Models().QueryModelId(params.m_name.c_str()).IsValid()) // Is the name already used in destination?
        {
        if (nullptr != stat)
            *stat = DgnDbStatus::DuplicateName;
        return nullptr;
        }

    DgnModelPtr model = GetModelHandler().Create(params);
    if (!model.IsValid())
        return nullptr;

    model->_InitFrom(*this);

    model->m_solver.RelocateToDestinationDb(importer);

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
    //  base Dgn schema. That is, only DgnItem.


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
DgnModelPtr DgnModel::CopyModel(DgnModelCR model, Utf8CP newName)
    {
    DgnDbR db = model.GetDgnDb();

    DgnModelPtr model2 = model.Clone(newName);
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
ComponentModel::ComponentModel(CreateParams const& params) : T_Super(params) 
    {
    m_elementCategoryName = params.GetElementCategoryName();
    m_elementECClassName = params.GetElementECClassName();
    m_itemECClassName = params.GetItemECClassName();
    m_itemECBaseClassName = params.GetItemECBaseClassName();

    if (m_itemECClassName.empty())
        m_itemECClassName = GetModelName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::_GetSolverOptions(Json::Value& json)
    {
    json["category"] = m_elementCategoryName.c_str();
    Json::Value& instance = json["instance"];
    instance["elementClass"] = m_elementECClassName.c_str();
    instance["itemClass"] = m_itemECClassName.c_str();
    instance["itemBaseClass"] = m_itemECBaseClassName.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ComponentModel::IsValid() const
    {
    if (m_elementCategoryName.empty() || !GetDgnDb().Categories().QueryCategoryId(m_elementCategoryName.c_str()).IsValid())
        return false;
    if (m_elementECClassName.empty())
        return false;
    if (m_itemECClassName.empty())
        return false;
    if (m_itemECBaseClassName.empty())
        return false;
    if (!m_solver.IsValid())
        return false;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::_OnDelete()
    {
    //  *** TRICKY: We don't/can't have a trigger do this, because the solutions table references this model by name
    Statement delSolutions;

    delSolutions.Prepare(GetDgnDb(), "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_ComponentSolution) " WHERE ComponentModelName=?");
    delSolutions.BindText(1, GetModelName(), Statement::MakeCopy::No);
    delSolutions.Step();

    // *** NEEDS WORK: I should kill off all of the GeomParts referenced by the geomstreams in the rows of ComponentSolution 
    return DgnDbStatus::Success;
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

    if (nullptr != db.Schemas().GetECSchema(schemaPtr->GetName().c_str()))
        return DgnDbStatus::AlreadyLoaded;

    if (BentleyStatus::SUCCESS != db.Schemas().ImportECSchemas(contextPtr->GetCache()))
        return DgnDbStatus::BadSchema;

    db.Domains().SyncWithSchemas();

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus ComponentModel::ExportSchemaTo(DgnDbR clientDb, DgnDbR componentDb, Utf8StringCR schemaNameIn)
    {
    Utf8String schemaName(schemaNameIn);
    if (schemaName.empty())
        schemaName = Utf8String(componentDb.GetFileName().GetFileNameWithoutExtension());

    if (nullptr != clientDb.Schemas().GetECSchema(schemaName.c_str()))
        return DgnDbStatus::AlreadyLoaded;

    // Ask componentDb to generate a schema
    ECN::ECSchemaPtr schema;
    if (ECN::ECOBJECTS_STATUS_Success != ECN::ECSchema::CreateSchema(schema, schemaName, 0, 0))
        return DgnDbStatus::BadSchema;
    
    schema->AddReferencedSchema(*const_cast<ECN::ECSchemaP>(componentDb.Schemas().GetECSchema(DGN_ECSCHEMA_NAME)), "dgn");

    if (DgnDbStatus::Success != ComponentModel::AddAllToECSchema(*schema, componentDb))
        return DgnDbStatus::BadSchema;

    componentDb.SaveChanges(); // *** TRICKY: AddAllToECSchema set the ItemECSchemaName property of the comonent models. Save it.
        
    //  Import the schema into clientDb
    ECSchemaReadContextPtr contextPtr = ECSchemaReadContext::CreateContext();
    contextPtr->AddSchema(*schema);
    if (BentleyStatus::SUCCESS != clientDb.Schemas().ImportECSchemas(contextPtr->GetCache()))
        return DgnDbStatus::BadSchema;

    clientDb.Domains().SyncWithSchemas();

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::GenerateECClass(ECN::ECSchemaR schema)
    {
    if (!IsValid())
        return DgnDbStatus::NotEnabled;

    ECN::ECClassP ecclass;
    auto stat = schema.CreateClass(ecclass, GetItemECClassName());
    if (ECN::ECOBJECTS_STATUS_Success != stat)
        return DgnDbStatus::BadRequest;

    Utf8String baseschemaname, baseclassname;
    std::tie(baseschemaname, baseclassname) = parseFullECClassName(GetItemECBaseClassName().c_str());

    ECN::ECClassCP baseClass = GetDgnDb().Schemas().GetECClass(baseschemaname.c_str(), baseclassname.c_str());

    ecclass->AddBaseClass(*baseClass);

    Utf8String paramsList;  
    Utf8CP paramsListSep = "";

    for (auto const& parm: m_solver.GetParameters())
        {
        Utf8String parmname = parm.GetName();
        ECN::ECValue parmvalue = parm.GetValue();

        ECN::ECPropertyP baseClassProperty = baseClass->GetPropertyP(parmname.c_str());
        if (nullptr != baseClassProperty)
            {
            // The base class has a property for this parameter, so I don't need to add one.
            if (!baseClassProperty->GetIsPrimitive() || baseClassProperty->GetAsPrimitiveProperty()->GetType() != parmvalue.GetPrimitiveType())
                {
                BeDataAssert(false && "component model parameter is incompatible with existing base class ECProperty");
                DGNCORELOG->warningv("component model parameter [%s] is incompatible with existing base class ECProperty", parmname.c_str());
                }
            continue;
            }

        //  Add a corresponding property to the generated Item class
        ECN::PrimitiveECPropertyP ecprop;
        stat = ecclass->CreatePrimitiveProperty(ecprop, parmname.c_str());
        if (ECN::ECOBJECTS_STATUS_Success != stat)
            return DgnDbStatus::BadRequest;
        
        ecprop->SetType(parmvalue.GetPrimitiveType());

        paramsList.append(paramsListSep).append(parmname);
        paramsListSep = ",";
        }

    //  Store an EGASpecifier custom attribute on the item class that identifies this component model and its parameters (in order!)
    auto egaSpecClass = GetDgnDb().Schemas().GetECClass(DGN_ECSCHEMA_NAME, "EGASpecifier");
    ECN::IECInstancePtr egaSpec = egaSpecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    egaSpec->SetValue("Type", ECValue("ComponentModel"));
    egaSpec->SetValue("Name", ECValue(GetModelName()));
    egaSpec->SetValue("Inputs", ECValue(paramsList.c_str()));   // NB: paramsList must be in the same order as m_solver.GetParameters, so that clients can use it to generate the solution name
    ecclass->SetCustomAttribute(*egaSpec);

    m_itemECSchemaName = schema.GetName();
    Update();

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
Utf8String ComponentModel::GetElementECClassName() const
    {
    return m_elementECClassName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentModel::GetItemECClassName() const
    {
    return m_itemECClassName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentModel::GetItemECBaseClassName() const
    {
    return m_itemECBaseClassName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::_ToPropertiesJson(Json::Value& json) const
    {
    T_Super::_ToPropertiesJson(json);
    json["ComponentElementCategoryName"] = m_elementCategoryName.c_str();
    json["ComponentElementECClassName"]  = m_elementECClassName.c_str();
    json["ComponentItemECSchemaName"]    = m_itemECSchemaName.c_str();
    json["ComponentItemECClassName"]     = m_itemECClassName.c_str();
    json["ComponentItemECBaseClassName"] = m_itemECBaseClassName.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::_FromPropertiesJson(Json::Value const& json)
    {
    T_Super::_FromPropertiesJson(json);
    m_elementCategoryName = json["ComponentElementCategoryName"].asCString();
    m_elementECClassName  = json["ComponentElementECClassName"].asCString();
    m_itemECSchemaName     = json["ComponentItemECSchemaName"].asCString();
    m_itemECClassName     = json["ComponentItemECClassName"].asCString();
    m_itemECBaseClassName = json["ComponentItemECBaseClassName"].asCString();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::Solve(ModelSolverDef::ParameterSet const& newParameters)
    {
    m_solver.GetParametersR().SetValues(newParameters);

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
