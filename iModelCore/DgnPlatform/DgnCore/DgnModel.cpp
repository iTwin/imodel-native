/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnModel.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnScript.h>
#include <DgnPlatform/DgnCore/DgnDbTables.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
static std::pair<ECN::PrimitiveType, BentleyStatus> jsonTypeToEcPrimitiveType(Json::Value const& jsonValue)
    {
    if (jsonValue.isBool())
        return std::make_pair(ECN::PRIMITIVETYPE_Boolean, BSISUCCESS);
    if (jsonValue.isIntegral())
        return std::make_pair(ECN::PRIMITIVETYPE_Long, BSISUCCESS);
    if (jsonValue.isDouble())
        return std::make_pair(ECN::PRIMITIVETYPE_Double, BSISUCCESS);
    if (jsonValue.isString())
        return std::make_pair(ECN::PRIMITIVETYPE_String, BSISUCCESS);
    
    return std::make_pair(PRIMITIVETYPE_Binary, BSIERROR);
    }e
+---------------+---------------+---------------+---------------+---------------+------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus ecPrimitiveValueToJson(Json::Value& json, ECN::ECValueCR ecv)
    {
    if (ECN::VALUEKIND_Primitive != ecv.GetKind())
        return DgnDbStatus::BadArg;
    json["Type"] = (int)ecv.GetPrimitiveType();
    json["Value"] = ecv.ToString().c_str();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus ecPrimitiveValueFromJson(ECN::ECValueR ecv, Json::Value const& json)
    {
    ecv = ECValue(json["Value"].asCString());
    return ecv.ConvertToPrimitiveType((ECN::PrimitiveType)(json["Type"].asInt()))? DgnDbStatus::Success: DgnDbStatus::BadArg;
    }

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
static BentleyStatus toJsonPropertiesFromECValues(Json::Value& json, bvector<DgnModel::Solver::Parameter> const& parms)
    {
    for (auto const& parm : parms)
        {
        if (BSISUCCESS != DgnScriptLibrary::ToJsonFromEC(json[parm.GetName().c_str()], parm.GetValue()))
            return BSIERROR;
        }
    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::Solver::Solve(DgnModelR model)
    {
    if (Type::Script == m_type)
        {
        int retval;
        Json::Value parmsJson(Json::objectValue);
        toJsonPropertiesFromECValues(parmsJson, GetParameters());
        DgnDbStatus xstatus = DgnScript::ExecuteModelSolver(retval, model, m_name.c_str(), parmsJson);
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
DgnModel::Solver::Parameter DgnModel::Solver::GetParameter(Utf8StringCR pname) const
    {
    for (auto const& parameter : m_parameters)
        {
        if (parameter.GetName().EqualsI(pname))
            return parameter;
        }
    return DgnModel::Solver::Parameter("", Parameter::Scope::Class, ECValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::Solver::Parameter::SetValue(ECN::ECValueCR valueIn)
    {
    if (!m_value.IsPrimitive())
        {
        if (m_value.GetKind() != valueIn.GetKind())
            return DgnDbStatus::BadArg;
        m_value = valueIn;
        }
    else
        {
        ECN::ECValue value = valueIn;
        if (!value.ConvertToPrimitiveType(m_value.GetPrimitiveType()))
            return DgnDbStatus::BadArg;
        m_value = value;
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::Solver::SetParameterValue(Utf8StringCR pname, ECN::ECValueCR valueIn)
    {
    for (auto& parameter : m_parameters)
        {
        if (parameter.GetName().EqualsI(pname))
            {
            return parameter.SetValue(valueIn);
            }
        }
    return DgnDbStatus::NotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnModel::Solver::SetParameterValues(bvector<Parameter> const& parms)
    {
    for (auto const& parm : parms)
        {
        DgnDbStatus status = SetParameterValue(parm.GetName(), parm.GetValue());
        if (DgnDbStatus::Success != status)
            return status;
        }
    return DgnDbStatus::Success;
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
void DgnModel::_OnDeletedElement(DgnElementCR element)
    {
    RemoveFromRangeIndex(element);
    if (m_filled)
        m_elements.erase(element.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnReversedAddElement(DgnElementCR element)
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
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnReversedUpdateElement(DgnElementCR modified, DgnElementCR original)
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
#define PARARMETER_SCOPE_CLASS 0
#define PARARMETER_SCOPE_TYPE 1
#define PARARMETER_SCOPE_INSTANCE 2

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DgnModel::Solver::Parameter::ToJson() const
    {
    Json::Value v;
    v["Scope"] = (Scope::Class == m_scope)? PARARMETER_SCOPE_CLASS: (Scope::Type == m_scope)? PARARMETER_SCOPE_TYPE: PARARMETER_SCOPE_INSTANCE;
    v["Name"] = m_name.c_str();
    ecPrimitiveValueToJson(v["Value"], m_value);
    return v;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::Solver::Parameter::Parameter(Json::Value const& json)
    {
    auto s = json["Scope"].asInt();
    m_scope = (PARARMETER_SCOPE_CLASS == s)? Scope::Class: (PARARMETER_SCOPE_TYPE == s)? Scope::Type: Scope::Instance;
    m_name = json["Name"].asCString();
    ecPrimitiveValueFromJson(m_value, json["Value"]);
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

    Json::Value parametersJson (Json::arrayValue);
    for (auto const& parameter : m_parameters)
        parametersJson.append(parameter.ToJson());

    json["Parameters"] = parametersJson;

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
    auto& parameters = json["Parameters"];
    for (Json::ArrayIndex i=0; i < parameters.size(); ++i)
        m_parameters.push_back(Parameter(parameters[i]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
EC::ECInstanceId ComponentModelSolution::QuerySolutionId(DgnModelId cmid, Utf8StringCR solutionName)
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
DgnDbStatus ComponentModelSolution::Query(Solution& sln, EC::ECInstanceId sid)
    {
    CachedStatementPtr stmt = GetDgnDb().GetCachedStatement("SELECT ComponentModelId,Parameters,Range FROM " DGN_TABLE(DGN_CLASSNAME_ComponentModelSolution) " WHERE(Id=?)");
    stmt->BindId(1, sid);
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::NotFound;
    sln.m_id = sid;
    sln.m_componentModelId = stmt->GetValueId<DgnModelId>(0);
    sln.m_parameters = stmt->GetValueText(1);
    sln.m_range = *(ElementAlignedBox3d*)stmt->GetValueBlob(2);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModelSolution::Solution::QueryGeomStream(GeomStreamR geom, DgnDbR db) const
    {
    return geom.ReadGeomStream(db, DGN_TABLE(DGN_CLASSNAME_ComponentModelSolution), "Geom", m_id.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentModel::ComputeSolutionName()
    {
    return ComputeSolutionName(m_solver.GetParameters());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::GetSolutionParametersFromECProperties(bvector<Solver::Parameter>& parameters, ECN::IECInstanceCR instance)
    {
    for (auto& parameter : parameters)
        {
        ECN::ECValue ecv;
        if (ECN::ECOBJECTS_STATUS_Success != instance.GetValue(ecv, parameter.GetName().c_str()))
            {
            BeDataAssert(false);
            return DgnDbStatus::BadArg;
            }
        parameter.SetValue(ecv);
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentModel::ComputeSolutionName(bvector<Solver::Parameter> const& parameters)
    {
    Json::Value json(Json::objectValue);
    for (auto& parameter : parameters)
        {
        ecPrimitiveValueToJson(json[parameter.GetName().c_str()], parameter.GetValue());
        }
    return Json::FastWriter::ToString(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
EC::ECInstanceId ComponentModelSolution::CaptureSolution(ComponentModelR componentModel)
    {
    if (&componentModel.GetDgnDb() != &GetDgnDb())
        {
        BeAssert(false && "you must import the component model before you can capture a solution");
        return EC::ECInstanceId();
        }

    Utf8String solutionCode = componentModel.ComputeSolutionName();

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
        BeDataAssert(false && "A component model contains no elements in the component's category.");
        return EC::ECInstanceId();
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
    if (builder->GetGeomStream(geom) != BSISUCCESS)
        return EC::ECInstanceId();

    Placement3d placement;
    if (builder->GetPlacement(placement) != BSISUCCESS)
        return EC::ECInstanceId();

    ElementAlignedBox3d box = placement.GetElementBox();
        
    CachedStatementPtr istmt = GetDgnDb().GetCachedStatement("INSERT INTO " DGN_TABLE(DGN_CLASSNAME_ComponentModelSolution) " (ComponentModelId,Parameters,Range) VALUES(?,?,?)");
    istmt->BindId(1, componentModel.GetModelId());
    istmt->BindText(2, solutionCode, Statement::MakeCopy::No);
    istmt->BindBlob(3, &box, sizeof(box), Statement::MakeCopy::No);
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
DgnElementPtr ComponentModelSolution::CreateSolutionInstanceElement(DgnModelR destinationModel, BeSQLite::EC::ECInstanceId solutionId, DPoint3dCR origin, YawPitchRollAnglesCR angles)
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
    
    if (destinationModel.Is3d())
        return PhysicalElement::Create(PhysicalElement::CreateParams(GetDgnDb(), destinationModel.GetModelId(), cmClassId, cmCategoryId, 
                                            Placement3d(origin, angles, ElementAlignedBox3d())));

    return DrawingElement::Create(DrawingElement::CreateParams(GetDgnDb(), destinationModel.GetModelId(), cmClassId, cmCategoryId, 
                                            Placement2d(DPoint2d::From(origin.x,origin.y), angles.GetYaw(), ElementAlignedBox2d())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModelSolution::CreateSolutionInstanceItem(DgnElementR instanceElement, ECN::IECInstancePtr& itemProperties, Utf8CP componentSchemaName, BeSQLite::EC::ECInstanceId solutionId)
    {
    DgnDbR db = instanceElement.GetDgnDb();

    //  The the details of the solution and the class of the Item that should be created
    ComponentModelSolution solutions(db);
    Solution sln;
    DgnDbStatus status = solutions.Query(sln, solutionId);
    if (DgnDbStatus::Success != status)
        return status;

    RefCountedPtr<ComponentModel> cm = db.Models().Get<ComponentModel>(sln.m_componentModelId);
    if (!cm.IsValid())
        return DgnDbStatus::NotFound;

    DgnClassId cmItemClassId = DgnClassId(db.Schemas().GetECClassId(componentSchemaName, cm->GetItemECClassName().c_str()));
    if (!cmItemClassId.IsValid())
        return DgnDbStatus::WrongClass;

    // Make and return a standalone instance of the itemClass that holds the parameters as properties. It is up to the caller to transfer these properties to the DgnElement::Item object.
    auto itemClass = db.Schemas().GetECClass(cmItemClassId.GetValue());
    itemProperties = itemClass->GetDefaultStandaloneEnabler()->CreateInstance();
    bvector<DgnModel::Solver::Parameter> const& params = cm->GetSolver().GetParameters();
    for (auto param : params)
        {
        ECN::ECValue ecv;
        itemProperties->GetValue(ecv, param.GetName().c_str()); // call GetValue in order to get the TYPE

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
DgnDbStatus ComponentModelSolution::ExecuteEGA(DgnElementR el, DPoint3dCR origin, YawPitchRollAnglesCR angles, ECN::IECInstanceCR itemInstance, Utf8StringCR cmName, Utf8StringCR paramNames, DgnElement::Item& item)
    {
    DgnDbR db = el.GetDgnDb();

    ComponentModelPtr cm = db.Models().Get<ComponentModel>(db.Models().QueryModelId(cmName.c_str()));
    if (!cm.IsValid())
        {
        BeDataAssert(false);
        DGNCORELOG->warningv("generated component model item [class %s] names a component model [%s] that is not in the target DgnDb", itemInstance.GetClass().GetName().c_str(), cmName.c_str());
        return DgnDbStatus::NotFound;
        }

    bvector<DgnModel::Solver::Parameter> parms = cm->GetSolver().GetParameters();

    ComponentModel::GetSolutionParametersFromECProperties(parms, itemInstance); // Set the parameter values to what I have saved

    ComponentModelSolution solutions(db);
    BeSQLite::EC::ECInstanceId sid = solutions.QuerySolutionId(cm->GetModelId(), ComponentModel::ComputeSolutionName(parms));
    Solution sln;
    if (DgnDbStatus::Success != solutions.Query(sln, BeSQLite::EC::ECInstanceId(sid)))
        return DgnDbStatus::NotFound;

    GeomStream geom;
    sln.QueryGeomStream(geom, db);

    DgnElement3dP e3d = el.ToElement3dP();
    if (nullptr != e3d)
        {
        Placement3d placement(origin, angles, sln.m_range);
        e3d->SetPlacement(placement);
        e3d->GetGeomStreamR().SaveData (geom.GetData(), geom.GetSize());
        return DgnDbStatus::Success;
        }
        
    DgnElement2dP e2d = el.ToElement2dP();
    if (nullptr == e2d)
        {
        ElementAlignedBox3dCR range3d = sln.m_range;
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
            continue;
            }
        if (DgnDbStatus::Success != status)
            {
            // *** TBD: Record failure somehow
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
    m_itemECClassName     = json["ComponentItemECClassName"].asCString();
    m_itemECBaseClassName = json["ComponentItemECBaseClassName"].asCString();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::Solve()
    {
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