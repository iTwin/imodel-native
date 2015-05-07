/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnModel.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnModels::InsertModel(Model& row)
    {
    DbResult status = m_dgndb.GetNextRepositoryBasedId(row.m_id, DGN_TABLE(DGN_CLASSNAME_Model), "Id");
    BeAssert(status == BE_SQLITE_OK);

    Statement stmt;
    stmt.Prepare(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Model) " (Id,Name,Descr,Type,ECClassId,Space,Visibility) VALUES(?,?,?,?,?,?,?)");

    stmt.BindId(1, row.GetId());
    stmt.BindText(2, row.GetNameCP(), Statement::MakeCopy::No);
    stmt.BindText(3, row.GetDescription(), Statement::MakeCopy::No);
    stmt.BindInt(4,(int)row.GetModelType());
    stmt.BindInt64(5, row.GetClassId().GetValue());
    stmt.BindInt(6,(int) row.GetCoordinateSpace());
    stmt.BindInt(7, row.InGuiList());

    status = stmt.Step();
    BeAssert(BE_SQLITE_DONE==status);
    return(BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModels::DeleteModel(DgnModelId modelId)
    {
    Statement stmt;
    stmt.Prepare(m_dgndb, "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE Id=?");
    stmt.BindId(1, modelId);
    return BE_SQLITE_DONE == stmt.Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DgnModels::QueryModelId(Utf8CP name) const
    {
    CachedStatementPtr statementPtr;
    GetDgnDb().GetCachedStatement(statementPtr, "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE Name=?");
    statementPtr->BindText(1, name, Statement::MakeCopy::No);
    return (BE_SQLITE_ROW != statementPtr->Step()) ? DgnModelId() : statementPtr->GetValueId<DgnModelId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    02/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DgnModels::QueryModelId(DgnElementId elementId)
    {
    CachedStatementPtr statementPtr;
    GetDgnDb().GetCachedStatement(statementPtr, "SELECT ModelId FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?");
    statementPtr->BindId(1, elementId);
    return (BE_SQLITE_ROW != statementPtr->Step()) ? DgnModelId() : statementPtr->GetValueId<DgnModelId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModels::QueryModelById(Model* out, DgnModelId id) const
    {
    Statement stmt;
    stmt.Prepare(m_dgndb, "SELECT Name,Descr,Type,ECClassId,Space,Visibility FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE Id=?");
    stmt.BindId(1, id);

    if (BE_SQLITE_ROW != stmt.Step())
        return  ERROR;

    if (out) // this can be null to just test for the existence of a model by id
        {
        out->m_id = id;
        out->m_name.AssignOrClear(stmt.GetValueText(0));
        out->m_description.AssignOrClear(stmt.GetValueText(1));
        out->m_modelType = (DgnModelType) stmt.GetValueInt(2);
        out->m_classId = DgnClassId(stmt.GetValueInt64(3));
        out->m_space = (Model::CoordinateSpace) stmt.GetValueInt(4);
        out->m_inGuiList = TO_BOOL(stmt.GetValueInt(5));
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModels::GetModelName(Utf8StringR name, DgnModelId id) const
    {
    Statement stmt;
    stmt.Prepare(m_dgndb, "SELECT Name FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE Id=?");
    stmt.BindId(1, id);

    if (BE_SQLITE_ROW != stmt.Step())
        return  ERROR;

    name.AssignOrClear(stmt.GetValueText(0));
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModels::ClearLoaded()
    {
    for (auto iter : m_models)
        iter.second->Empty();

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
    mtype = (DgnModelType)selectDependencyIndex->GetValueInt(1);
    m_modelDependencyIndexAndType[mid] = make_bpair(didx, mtype);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/11
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnModels::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString("SELECT count(*) FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE (?1 = (Visibility & ?1))", true);

    Statement sql;
    sql.Prepare(*m_db, sqlString.c_str());
    sql.BindInt(1, (int) m_itType);

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
        m_stmt->BindInt(1, (int) m_itType);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

DgnModelId   DgnModels::Iterator::Entry::GetModelId() const {Verify(); return m_sql->GetValueId<DgnModelId>(0);}
Utf8CP       DgnModels::Iterator::Entry::GetName() const {Verify();return m_sql->GetValueText(1);}
Utf8CP       DgnModels::Iterator::Entry::GetDescription() const {Verify();return m_sql->GetValueText(2);}
DgnModelType DgnModels::Iterator::Entry::GetModelType() const {Verify();return (DgnModelType) m_sql->GetValueInt(3);}
DgnModels::Model::CoordinateSpace DgnModels::Iterator::Entry::GetCoordinateSpace() const {Verify();return (Model::CoordinateSpace) m_sql->GetValueInt(4);}
uint32_t     DgnModels::Iterator::Entry::GetVisibility() const {Verify();return m_sql->GetValueInt(5);}
DgnClassId   DgnModels::Iterator::Entry::GetClassId() const {Verify();return DgnClassId(m_sql->GetValueInt64(6));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/00
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::ReleaseAllElements()
    {
    m_wasFilled  = false;   // this must be before we release all elementrefs
    m_elements.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::DgnModel(CreateParams const& params) : m_dgndb(params.m_dgndb), m_modelId(params.m_id), m_classId(params.m_classId), m_name(params.m_name)
    {
    m_rangeIndex = nullptr;
    m_wasFilled = false;
    m_readonly  = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnModel::AddAppData(DgnModelAppData::Key const& key, DgnModelAppData* obj)
    {
    return  m_appData.AddAppData(key, obj, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnModel::DropAppData(DgnModelAppData::Key const& key)
    {
    return  m_appData.DropAppData(key, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelAppData* DgnModel::FindAppData(DgnModelAppData::Key const& key) const
    {
    return  m_appData.FindAppData(key);
    }

struct EmptyCaller
    {
    DgnModelR m_model;
    EmptyCaller(DgnModelR model) : m_model(model) {}
    bool CallHandler(DgnModelAppData& handler) const {return handler._OnEmpty(m_model);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnModel::NotifyOnEmpty()
    {
    if (!IsFilled())
        return false;

    m_appData.CallAllDroppable(EmptyCaller(*this), *this);
    return  true;
    }

struct EmptiedCaller
    {
    DgnModelR m_model;
    EmptiedCaller(DgnModelR model) : m_model(model) {}
    bool CallHandler(DgnModelAppData& handler) const {return handler._OnEmptied(m_model);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::Empty()
    {
    bool wasFilled = m_wasFilled;

    ReleaseAllElements();
    ClearRangeIndex();

    if (wasFilled)
        m_appData.CallAllDroppable(EmptiedCaller(*this), *this);
    }

struct CleanupCaller
    {
    DgnModelR m_model;
    CleanupCaller(DgnModelR model) : m_model(model) {}
    void CallHandler(DgnModelAppData& handler) const {handler._OnCleanup(m_model);}
    };

/*---------------------------------------------------------------------------------**//**
* Destructor for DgnModel. Free all memory allocated to this DgnModel.
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::~DgnModel()
    {
    m_appData.CallAll(CleanupCaller(*this));
    m_appData.m_list.clear();  // (some handlers may have deleted themselves. Don't allow Empty to call handlers.)

    Empty();
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
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void PlanarPhysicalModel::_ToPropertiesJson(Json::Value& val) const
    {
    T_Super::_ToPropertiesJson(val);
    JsonUtils::TransformToJson(val["worldTrans"], m_worldTrans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void PlanarPhysicalModel::_FromPropertiesJson(Json::Value const& val) 
    {
    T_Super::_FromPropertiesJson(val);
    if (val.isMember("worldTrans"))
        JsonUtils::TransformFromJson(m_worldTrans, val["worldTrans"]);
    else
        m_worldTrans.InitIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SectionDrawingModel::_ToPropertiesJson(Json::Value& val) const
    {
    T_Super::_ToPropertiesJson(val);

    val["zlow"] = m_zrange.low;
    val["zhigh"] = m_zrange.high;
    val["annotation_scale"] = m_annotationScale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SectionDrawingModel::_FromPropertiesJson(Json::Value const& val) 
    {
    T_Super::_FromPropertiesJson(val);
    if (val.isMember("zlow"))
        {
        m_zrange.low = JsonUtils::GetDouble(val["zlow"], 0);
        m_zrange.high = JsonUtils::GetDouble(val["zhigh"], 0);
        }
    else
        {
        m_zrange.InitNull();
        }

    if (val.isMember ("annotation_scale"))
        m_annotationScale = val["annotation_scale"].asDouble();
    else
        m_annotationScale = 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Transform SectionDrawingModel::GetFlatteningMatrix(double zdelta) const
    {
    if (m_zrange.IsNull())
        return Transform::FromIdentity();

    double nearlyZero = 1.0e-12;
    
    RotMatrix   flattenRMatrix;
    flattenRMatrix.initFromScaleFactors(1.0, 1.0, nearlyZero);

    RotMatrix   mustBeInvertible;
    while (!mustBeInvertible.InverseOf(flattenRMatrix))
        {
        printf("nearlyZero=%lg too small\n", nearlyZero);
        nearlyZero *= 10;
        flattenRMatrix.initFromScaleFactors(1.0, 1.0, nearlyZero);
        }

    auto trans = Transform::From(flattenRMatrix);

    if (zdelta != 0.0)
        {
        auto xlat = Transform::FromIdentity();
        xlat.SetTranslation(DPoint3d::FromXYZ(0,0,zdelta));
        trans = Transform::FromProduct(xlat, trans);  // flatten and then translate (in z)
        }

    return trans;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::ReadProperties()
    {
    Statement stmt;
    stmt.Prepare(m_dgndb, "SELECT Props FROM " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE Id=?");
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DgnModel::SaveProperties()
    {
    Json::Value propJson(Json::objectValue);
    _ToPropertiesJson(propJson);
    Utf8String val = Json::FastWriter::ToString(propJson);

    Statement stmt;
    stmt.Prepare(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Model) " SET Props=? WHERE Id=?");
    stmt.BindText(1, val, Statement::MakeCopy::No);
    stmt.BindId(2, m_modelId);

    auto rc=stmt.Step();
    return rc== BE_SQLITE_DONE ? BE_SQLITE_OK : rc;
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

struct FilledCaller
    {
    DgnModelR m_model;
    FilledCaller(DgnModelR model) : m_model(model) {}
    void CallHandler(DgnModelAppData& handler) const {handler._OnFilled(m_model);}
    };

/*---------------------------------------------------------------------------------**//**
* Called after the process of filling a model is complete. Provides an opportunity for application to
* do things that have to happen after a model is completely filled.
* @bsimethod                                                    KeithBentley    02/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::ModelFillComplete()
    {
    m_appData.CallAll(FilledCaller(*this));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DgnModel::CountElements() const
    {
    uint32_t count = 0;

    for (const_iterator it=begin(); it!=end(); ++it)
        ++count;

    return  count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::ClearRangeIndex()
    {
    DELETE_AND_CLEAR(m_rangeIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::RegisterElement(DgnElementCR element)
    {
    if (m_wasFilled)
        m_elements.Add(element);

    GeometricElementCP geom = element._ToGeometricElement();
    if (nullptr != m_rangeIndex && nullptr != geom)
        m_rangeIndex->AddGeomElement(*geom);
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
DgnModelStatus DgnModel::_OnInsertElement(DgnElementR element)
    {
    return m_readonly ? DGNMODEL_STATUS_ReadOnly : DGNMODEL_STATUS_Success;
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
DgnModelStatus DgnModel::_OnDeleteElement(DgnElementR element)
    {
    return m_readonly ? DGNMODEL_STATUS_ReadOnly : DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* "canceled" means that we're abandoning a rolled-back transaction. In that case, remove it from the list even if the list is filled.
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnDeletedElement(DgnElementR element, bool canceled)
    {
    // if this list is marked as "filled", then we keep the deleted elements in the list so they 
    // will remain owned by the list if the delete is subsequently undone.
    if (!canceled && m_wasFilled)
        return;

    DgnElementId  id = element.GetElementId();
    if (!id.IsValid())
        {
        BeAssert(false);
        return;
        }

    m_elements.erase(id);

    // if this is a cancel, then the element was dropped from the range index when it was deleted.
    if (canceled || nullptr==m_rangeIndex)
        return;

    GeometricElementCP geom = element._ToGeometricElement();
    if (nullptr != geom)
        m_rangeIndex->RemoveElement(DgnRangeTree::Entry(geom->_GetRange3d(), *geom));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_OnUpdatedElement(DgnElementR element, DgnElementR replacement)
    {
    GeometricElementCP geom = element._ToGeometricElement();
    if (nullptr != m_rangeIndex && nullptr != geom)
        {
        GeometricElementCP replaceGeom = replacement._ToGeometricElement();
        m_rangeIndex->RemoveElement(DgnRangeTree::Entry(replaceGeom->_GetRange3d(), *geom));
        m_rangeIndex->AddGeomElement(*geom);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnModel::_OnUpdateElement(DgnElementCR element, DgnElementR replacement)
    {
    if (this != (&replacement.GetDgnModel()) || (this != &element.GetDgnModel()))
        {
        BeAssert(false);
        return DGNMODEL_STATUS_WrongModel;
        }

    if (element.IsDeleted())
        return DGNMODEL_STATUS_ReplacingDeleted;

    if (IsReadOnly())
        return DGNMODEL_STATUS_ReadOnly;

    if (element.m_classId != replacement.m_classId)
        return DGNMODEL_STATUS_WrongClass;

    return DGNMODEL_STATUS_Success;
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
* Called whenever the geom for an element (could possibly have) changed.
* If a cached presentation of the geom is stored on the DgnElementP, delete it. Also
* remove its range from the range tree
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::ElementChanged(DgnElement& elRef, DgnElementChangeReason reason)
    {
#if defined (NEEDS_WORK_ELEMDSCR_REWORK)
    // if there are qvElems on this DgnElementP, delete them
    elRef.ForceElemChanged(false, reason);

    if (ELEMREF_CHANGE_REASON_Modify != reason)
        return;

    OnElementModify((DgnElementR) elRef);
#endif
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
void DgnModel::ClearAllQvElems()
    {
#if defined (NEEDS_WORK_DGNITEM)
    DgnModel::DgnElementIterator iter(this);
    for (ElementRefP elRef = iter.GetFirstElementRef(); NULL != elRef && !iter.HitEOF(); elRef = iter.GetNextElementRef(true))
        elRef->ForceElemChanged(true, ELEMREF_CHANGE_REASON_ClearQVData);
#endif
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
* @bsimethod                                    Keith.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnModels::InsertNewModel(DgnModelR model, Utf8CP description, bool inGuiList)
    {
    if (model.m_modelId.IsValid())
        return DGNMODEL_STATUS_IdExists;

    if (&model.m_dgndb != &m_dgndb)
        return DGNMODEL_STATUS_WrongDgnDb;

    DgnModels::Model row(model.m_name.c_str(), model._GetModelType(), model._GetCoordinateSpace(), model.GetClassId());
    row.SetDescription(description);
    row.SetInGuiList(inGuiList);
    row.GetNameR().Trim();
    row.GetDescriptionR().Trim();

    if (!IsValidName(row.GetName()))
        {
        BeAssert(false);
        return  DGNMODEL_STATUS_InvalidModelName;
        }

    if (QueryModelId(row.GetNameCP()).IsValid()) // can't allow two models with the same name
        return DGNMODEL_STATUS_DuplicateModelName;

    if (BE_SQLITE_OK != InsertModel(row))
        return DGNMODEL_STATUS_ModelTableWriteError;

    model.m_modelId = row.GetId(); // retrieve new modelId
    m_models[model.m_modelId] = &model;  // this holds the reference count
    return (BE_SQLITE_OK == model.SaveProperties()) ? DGNMODEL_STATUS_Success : DGNMODEL_STATUS_BadRequest;
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
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnModel::Duplicate(Utf8CP newName) const
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
    return *ModelHandler::FindHandler(m_dgndb, m_classId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP DgnModels::CreateNewModelFromSeed(DgnModelStatus* result, Utf8CP name, DgnModelId seedModelId)
    {
    DgnModelStatus t, &status(result ? *result : t);
    if (!seedModelId.IsValid())
        {
        status = DGNMODEL_STATUS_BadSeedModel;
        return nullptr;
        }

    DgnModelP seedModel = GetModelById(seedModelId);
    if (nullptr == seedModel)
        {
        status = DGNMODEL_STATUS_BadSeedModel;
        return nullptr;
        }

    DgnModelPtr newModel = seedModel->Duplicate(name);
    status = InsertNewModel(*newModel);
    if (DGNMODEL_STATUS_Success != status)
        return nullptr;

    return newModel.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP PhysicalModelHandler::_CreateInstance(DgnModel::CreateParams const& params)
    {
    return new PhysicalModel(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP DgnModels::CreateDgnModel(DgnModelId modelId)
    {
    DgnModels::Model model;
    if (SUCCESS != QueryModelById(&model, modelId))
        return nullptr;

    // make sure the class derives from Model (has a handler)
    ModelHandlerP handler = ModelHandler::FindHandler(m_dgndb, model.GetClassId());
    if (nullptr == handler)
        return nullptr;

    DgnModel::CreateParams params(m_dgndb, model.GetClassId(), model.GetName().c_str(), modelId);
    DgnModelPtr dgnModel = handler->Create(params);
    if (!dgnModel.IsValid())
        return nullptr;

    dgnModel->ReadProperties();

    m_models[modelId] = dgnModel; // this holds the reference count
    return dgnModel.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP DgnModels::FindModelById(DgnModelId modelId)
    {
    auto const& it=m_models.find(modelId);
    return  it!=m_models.end() ? it->second.get() : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP DgnModels::GetModelById(DgnModelId modelId)
    {
    if (!modelId.IsValid())
        return  NULL;

    DgnModelP dgnModel = FindModelById(modelId);
    return (NULL != dgnModel) ? dgnModel : CreateDgnModel(modelId);
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
    return MakeIterator(ModelIterate::All).begin().GetModelId();
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
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCP DgnElementIterator::GetFirstDgnElement(DgnModelCR model, bool wantDeleted)
    {
    m_map = &model.m_elements;
    m_it  = m_map->begin();

    DgnElementCP currElement = GetCurrentDgnElement();

    if (currElement && (!wantDeleted && currElement->IsDeleted()))
        currElement = GetNextDgnElement(wantDeleted);

    return  currElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCP DgnElementIterator::GetNextDgnElement(bool wantDeleted)
    {
    if (!IsValid())
        return  nullptr;
    
    while (true)
        {
        ++m_it;

        DgnElementCP currElement = GetCurrentDgnElement();

        if (!wantDeleted && currElement && currElement->IsDeleted())
            continue;

        return  currElement;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCP DgnElementIterator::SetCurrentDgnElement(DgnElementCP toElm)
    {
    if (!IsValid())
        return  nullptr;

    m_it = m_map->find(toElm->GetElementId());
    return GetCurrentDgnElement();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIterator& DgnElementIterator::operator++ ()
    {
    GetNextDgnElement();
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElementIterator::operator== (DgnElementIterator const& rhs) const
    {
    // two invalid iterator are equal
    return (!IsValid() && !rhs.IsValid()) || (m_map == rhs.m_map && m_it == rhs.m_it);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIterator DgnModel::begin() const
    {
    DgnElementIterator it;
    it.GetFirstDgnElement(*this);
    return it;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIterator DgnModel::end() const
    {
    return DgnElementIterator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d DgnModel::_GetGlobalOrigin() const
    {
    return GetDgnDb().Units().GetGlobalOrigin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
SectioningViewControllerPtr SectionDrawingModel::GetSourceView()
    {
#if defined (NEEDS_WORK_DRAWINGS)
    auto sectioningViewId = GetDgnDb().GeneratedDrawings().QuerySourceView(GetModelId());
    return dynamic_cast<SectioningViewController*>(GetDgnDb().Views().LoadViewController(sectioningViewId, DgnViews::FillModels::No).get());
#else
    return nullptr;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
double DgnModel::GetMillimetersPerMaster() const
    {
    return m_properties.GetMasterUnit().IsLinear() ? m_properties.GetMasterUnit().ToMillimeters() : 1000.;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/01
+---------------+---------------+---------------+---------------+---------------+------*/
double DgnModel::GetSubPerMaster() const
    {
    double  subPerMast;
    m_properties.GetSubUnit().ConvertDistanceFrom(subPerMast, 1.0, m_properties.GetMasterUnit());
    return subPerMast;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFileStatus DgnModel::FillModel()
    {
    if (IsFilled())
        return  DGNFILE_STATUS_Success;

    Statement stmt;
    enum Column : int            {Id=0,ClassId=1,CategoryId=2,Code=3,ParentId=4};
    stmt.Prepare(m_dgndb, "SELECT Id,ECClassId,CategoryId,Code,ParentId FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE ModelId=?");
    stmt.BindId(1, m_modelId);

    SetFilled();

    auto& elements = m_dgndb.Elements();
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnElementId id(stmt.GetValueId<DgnElementId>(Column::Id));
        if (nullptr != elements.FindElement(id))  // already loaded?
            continue;

        elements.LoadElement(DgnElement::CreateParams(*this,
            stmt.GetValueId<DgnClassId>(Column::ClassId), 
            stmt.GetValueId<DgnCategoryId>(Column::CategoryId), 
            stmt.GetValueText(Column::Code), 
            id,
            stmt.GetValueId<DgnElementId>(Column::ParentId)));
        }

    SetReadOnly(m_dgndb.IsReadonly());
    ModelFillComplete();

    return  DGNFILE_STATUS_Success;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModel::Properties::SetWorkingUnits(UnitDefinitionCR newMasterUnit, UnitDefinitionCR newSubUnit)
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
ModelHandlerP ModelHandler::FindHandler(DgnDb const& db, DgnClassId handlerId)
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
    Statement stmt;
    stmt.Prepare(m_dgndb, "SELECT DGN_bbox_union(DGN_placement_aabb(g.Placement)) FROM " 
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

