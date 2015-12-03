/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ComponentModel.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnDbTables.h>

static DgnDbStatus deleteAllSolutionsOfComponentRelationships(DgnDbR db, DgnModelId cmid);

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
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentModelPtr ComponentModel::FindModelByName(DgnDbR db, Utf8StringCR name)
    {
    return db.Models().Get<ComponentModel>(db.Models().QueryModelId(DgnModel::CreateModelCode(name)));
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      090/2015
//---------------------------------------------------------------------------------------
static StatusInt deleteComponentViews(DgnDbR db, DgnModelId mid)
    {
    // construct an iterator using the criteria from the request message
    auto viewIterator = ViewDefinition::MakeIterator(db, ViewDefinition::Iterator::Options(mid));

    bvector<DgnViewId> tbd;

    for (auto const& viewEntry : viewIterator)
        tbd.push_back(viewEntry.GetId());

    for (auto vid : tbd)
        {
        auto viewElem = db.Elements().GetElement(vid);
        viewElem->Delete();
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::_OnDelete()
    {
    // If any instance exists, then block the deletion. Tricky: unique/singleton elements are both types and instances -- they are instances of themselves.
    bvector<DgnElementId> types;
    QuerySolutions(types);
    for (auto teid : types)
        {
        bvector<DgnElementId> instances;
        QueryInstances(instances, teid);
        for (auto ieid : instances)
            {
            if (GetDgnDb().Elements().GetElement(ieid).IsValid())
                return DgnDbStatus::IdExists; // *** WIP_COMPONENT_MODEL need more appropriate error code
            }
        }

    deleteComponentViews(GetDgnDb(), GetModelId());
    deleteAllSolutionsOfComponentRelationships(GetDgnDb(), GetModelId());

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
static DgnDbStatus createSolutionOfComponentRelationship(DgnElementCR inst, ComponentModelR componentModel, ModelSolverDef::ParameterSet const& params)
    {
    CachedECSqlStatementPtr statement = inst.GetDgnDb().GetPreparedECSqlStatement(
        "INSERT INTO " DGN_SCHEMA(DGN_RELNAME_SolutionOfComponent)
        " (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId,Parameters) VALUES(?,?,?,?,?)");

    if (!statement.IsValid())
        return DgnDbStatus::BadRequest;

    statement->BindId(1, inst.GetElementClassId());
    statement->BindId(2, inst.GetElementId());
    statement->BindId(3, componentModel.GetClassId());
    statement->BindId(4, componentModel.GetModelId());
    statement->BindText(5, Json::FastWriter::ToString(params.ToJson()).c_str(), EC::IECSqlBinder::MakeCopy::Yes);
    return (BE_SQLITE_DONE == statement->Step()) ? DgnDbStatus::Success : DgnDbStatus::BadRequest;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus deleteSolutionOfComponentRelationship(DgnElementCR inst)
    {
    CachedECSqlStatementPtr statement = inst.GetDgnDb().GetPreparedECSqlStatement("DELETE FROM " DGN_SCHEMA(DGN_RELNAME_SolutionOfComponent) " WHERE (SourceECInstanceId=?)");

    if (!statement.IsValid())
        return DgnDbStatus::BadRequest;

    statement->BindId(1, inst.GetElementId());
    return (BE_SQLITE_DONE == statement->Step()) ? DgnDbStatus::Success : DgnDbStatus::NotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus deleteAllSolutionsOfComponentRelationships(DgnDbR db, DgnModelId cmid)
    {
    CachedECSqlStatementPtr statement = db.GetPreparedECSqlStatement("DELETE FROM " DGN_SCHEMA(DGN_RELNAME_SolutionOfComponent) " WHERE (TargetECInstanceId=?)");

    if (!statement.IsValid())
        return DgnDbStatus::BadRequest;

    statement->BindId(1, cmid);
    return (BE_SQLITE_DONE == statement->Step()) ? DgnDbStatus::Success : DgnDbStatus::NotFound;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    10/2015
//---------------------------------------------------------------------------------------
static BeSQLite::DbResult queryComponentModelFromSolution(DgnModelId& mid, ModelSolverDef::ParameterSet& params, DgnDbR db, DgnElementId itemId)
    {
    CachedECSqlStatementPtr statement = db.GetPreparedECSqlStatement(
        "SELECT TargetECInstanceId, Parameters FROM " DGN_SCHEMA(DGN_RELNAME_SolutionOfComponent) " WHERE SourceECInstanceId=? LIMIT 1");

    if (!statement.IsValid())
        return BE_SQLITE_ERROR_BadDbSchema;

    statement->BindId(1, itemId);
    DbResult dbr = statement->Step();
    if (BE_SQLITE_ROW != dbr)
        return dbr;

    mid = statement->GetValueId<DgnModelId>(0);
    Json::Value paramsObj(Json::objectValue);
    Json::Reader::Parse(statement->GetValueText(1), paramsObj);
    params = ModelSolverDef::ParameterSet(paramsObj);
    return BE_SQLITE_OK;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentModel::Importer::Importer(DgnDbR destDb, ComponentModel& sourceComponent)
    : DgnImportContext(sourceComponent.GetDgnDb(), destDb), m_sourceComponent(sourceComponent)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentModelPtr ComponentModel::Importer::ImportComponentModel(DgnDbStatus* status)
    {
    return m_destComponent = DgnModel::Import(status, m_sourceComponent, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::Importer::ImportSolutions(DgnModelR destCatalogModel, bvector<AuthorityIssuedCode> const& selected)
    {
    if (!m_destComponent.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::BadModel;
        }

    if (&m_destComponent->GetDgnDb() != &destCatalogModel.GetDgnDb())
        {
        BeAssert(false);
        return DgnDbStatus::WrongDgnDb;
        }

    if (m_destComponent.get() == &destCatalogModel)
        {
        BeAssert(false);
        return DgnDbStatus::WrongModel;
        }

    ElementImporter importer(*this);

    EC::ECSqlStatement selectCatalogItems;
    selectCatalogItems.Prepare(GetSourceDb(), "SELECT SourceECInstanceId,Parameters FROM " DGN_SCHEMA(DGN_RELNAME_SolutionOfComponent) " WHERE(TargetECInstanceId=?)");
    selectCatalogItems.BindId(1, m_sourceComponent.GetModelId());
    while (BE_SQLITE_ROW == selectCatalogItems.Step())
        {
        DgnElementCPtr sourceCatalogItem = GetSourceDb().Elements().GetElement(selectCatalogItems.GetValueId<DgnElementId>(0));
        if (!sourceCatalogItem.IsValid())
            continue;

        if (!selected.empty() && selected.end() != std::find(selected.begin(), selected.end(), sourceCatalogItem->GetCode()))  // apply optional caller-supplied filter
            continue;

        Json::Value paramsObj(Json::objectValue);
        Json::Reader::Parse(selectCatalogItems.GetValueText(1), paramsObj);
        ModelSolverDef::ParameterSet params = ModelSolverDef::ParameterSet(paramsObj);

        DgnDbStatus status;
        DgnElementCPtr destCatalogItem = importer.ImportElement(&status, destCatalogModel, *sourceCatalogItem);
        if (!destCatalogItem.IsValid())
            {
            return status;
            }

        //createSolutionOfComponentRelationship(*destCatalogItem, *m_destComponent, params);
        // OnElementImported should have taken care of linking copied solutions to the component model
        #ifndef NDEBUG
        DgnModelId qmid;
        ModelSolverDef::ParameterSet qparams;
        BeAssert(BE_SQLITE_OK == queryComponentModelFromSolution(qmid, qparams, GetDestinationDb(), destCatalogItem->GetElementId()) 
                    && qmid == m_destComponent->GetModelId() 
                    && qparams == params);
        #endif
        }

    return DgnDbStatus::Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::QuerySolutions(bvector<DgnElementId>& solutions)
    {
    EC::ECSqlStatement selectCatalogItems;
    selectCatalogItems.Prepare(GetDgnDb(), "SELECT SourceECInstanceId FROM " DGN_SCHEMA(DGN_RELNAME_SolutionOfComponent) " WHERE(TargetECInstanceId=?)");
    selectCatalogItems.BindId(1, GetModelId());
    while (BE_SQLITE_ROW == selectCatalogItems.Step())
        {
        solutions.push_back(selectCatalogItems.GetValueId<DgnElementId>(0));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::QueryInstances(bvector<DgnElementId>& instances, DgnElementId solutionId)
    {
    EC::ECSqlStatement selectCatalogItems;
    selectCatalogItems.Prepare(GetDgnDb(), "SELECT SourceECInstanceId FROM " DGN_SCHEMA(DGN_RELNAME_InstantiationOfTemplate) " WHERE(TargetECInstanceId=?)");
    selectCatalogItems.BindId(1, solutionId);
    while (BE_SQLITE_ROW == selectCatalogItems.Step())
        {
        instances.push_back(selectCatalogItems.GetValueId<DgnElementId>(0));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::UpdateSolutionsAndInstances()
    {
    EC::ECSqlStatement selectCatalogItems;
    selectCatalogItems.Prepare(GetDgnDb(), "SELECT SourceECInstanceId,Parameters FROM " DGN_SCHEMA(DGN_RELNAME_SolutionOfComponent) " WHERE(TargetECInstanceId=?)");
    selectCatalogItems.BindId(1, GetModelId());
    while (BE_SQLITE_ROW == selectCatalogItems.Step())
        {
        DgnElementCPtr sourceCatalogItem = GetDgnDb().Elements().GetElement(selectCatalogItems.GetValueId<DgnElementId>(0));
        if (!sourceCatalogItem.IsValid())
            continue;

        Json::Value paramsObj(Json::objectValue);
        Json::Reader::Parse(selectCatalogItems.GetValueText(1), paramsObj);
        ModelSolverDef::ParameterSet params = ModelSolverDef::ParameterSet(paramsObj);

        if (DgnDbStatus::Success != Solve(params))
            {
            deleteSolutionOfComponentRelationship(*sourceCatalogItem); // This solution cannot be renewed. It and its instances become orphans.
            continue;
            }

            #ifdef WIP_WIP_WIP
        DgnDbStatus status;
        DgnElementPtr newCatalogItem = HarvestSolution(status, *sourceCatalogItem->GetModel()->ToPhysicalModelP(), sourceCatalogItem->GetPlacement(), sourceCatalogItem->GetCode());
        if (!newCatalogItem.IsValid())
            {
            deleteSolutionOfComponentRelationship(*sourceCatalogItem); // This solution cannot be renewed. It and its instances become orphans.
            continue;
            }

        // *** TBD: Update

        bvector<DgnElementId> instances;
        QueryInstances(instances, teid);
        for (auto ieid : instances)
            {
            //if (GetDgnDb().Elements().GetElement(ieid).IsValid())
            }
            #endif
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentModel::CaptureSolution(DgnDbStatus* statusOut, PhysicalModelR catalogModel, ModelSolverDef::ParameterSet const& parameters, Utf8StringCR catalogItemName)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(status, statusOut);
    if (catalogItemName.empty())
        {
        BeAssert(false && "a catalog item must have a name");
        status = DgnDbStatus::InvalidName;
        return nullptr;
        }

    DgnElement::Code icode = CreateCapturedSolutionCode(catalogItemName);
    if (!icode.IsValid())
        {
        BeAssert(false && "could not generate a code for a catalog item??");
        return nullptr;
        }

    DgnElementId existingCatalogItemId = GetDgnDb().Elements().QueryElementIdByCode(icode);
    if (existingCatalogItemId.IsValid())
        {
        BeAssert(false && "catalog item names must be unique");
        status = DgnDbStatus::DuplicateName;
        return nullptr;
        }

    if (DgnDbStatus::Success != (status = Solve(parameters)))
        return nullptr;

    HarvestedSolutionInserter inserter(catalogModel, *this);
    return MakeCapturedSolutionElement(status, icode, inserter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::DeleteSolution(DgnElementCR catalogItem)
    {
    // *** WIP_COMPONENT_MODELS - Deleting a captured solution makes orphans of all instances of that solution. 
    // ***                          Maybe we should refuse to delete the captured solution in this case?

    DgnDbStatus status = deleteSolutionOfComponentRelationship(catalogItem);
    if (DgnDbStatus::Success != status)
        {
        if (DgnDbStatus::NotFound == status)
            return DgnDbStatus::BadRequest;
        return status;
        }
    return catalogItem.Delete();
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

    GetSolver().Solve(*this);
    if (GetDgnDb().Txns().HasFatalErrors())
        status = DgnDbStatus::ValidationFailed;
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isInstanceOfComponent(DgnElementCR el)
    {
    return queryTemplateItemFromInstance(el.GetDgnDb(), el.GetElementId()).IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentModel::QuerySolutionFromInstance(DgnElementCR instance)
    {
    DgnElementId capturedSolutionElementId = queryTemplateItemFromInstance(instance.GetDgnDb(), instance.GetElementId());
    return instance.GetDgnDb().Elements().Get<DgnElement>(capturedSolutionElementId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr ComponentModel::HarvestedSolutionWriter::_CreateCapturedSolutionElement(DgnDbStatus& status, DgnClassId iclass, DgnElement::Code const& icode)
    {
    DgnElement::CreateParams cparams(m_destModel.GetDgnDb(), m_destModel.GetModelId(), iclass, icode);
    dgn_ElementHandler::Element* handler = dgn_ElementHandler::Element::FindHandler(m_destModel.GetDgnDb(), iclass);
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
    DgnElementPtr capturedSolutionElement = dgnElem->ToPhysicalElementP();
    if (!capturedSolutionElement.IsValid())
        {
        BeAssert(false && "ComponentModel::HarvestSolution creates only PhysicalElements");
        status = DgnDbStatus::WrongClass;
        return nullptr;
        }
    return capturedSolutionElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::HarvestSolution(bvector<bpair<DgnSubCategoryId, DgnGeomPartId>>& geomBySubcategory, bvector<DgnElementCPtr>& nestedInstances)
    {
    DgnDbR db = GetDgnDb();

    // ***
    // *** WIP_COMPONENT_MODEL *** the logic below is not complete. Must must add another dimension -- we must break out builders by same ElemDisplayParams, not just subcategory
    // ***

    DgnCategoryId harvestableGeometryCategoryId = m_compProps.QueryItemCategoryId(db);

    //  Gather geometry by SubCategory
    bmap<DgnSubCategoryId, ElementGeometryBuilderPtr> builders;     
    FillModel();
    for (auto const& mapEntry : *this)
        {
        GeometrySourceCP componentElement = mapEntry.second->ToGeometrySource();
        if (nullptr == componentElement)
            continue;

        //  Nested instances will become child elements or will be folded into the solution. That will be handled below.
        if (isInstanceOfComponent(*mapEntry.second))
            {
            PhysicalElementCP pnested = mapEntry.second->ToPhysicalElement();
            if (nullptr != pnested)
                nestedInstances.push_back(pnested);
            else
                {
                BeDataAssert(false && "HarvestSolution supports only PhysicalElements.");
                }
            continue;
            }

        //  Only solution elements in the component's Category are collected. The rest are construction/annotation geometry.
        if (componentElement->GetCategoryId() != harvestableGeometryCategoryId)
            continue;

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

    #ifdef WIP_COMPONENT_MODEL // *** allow empty instances?
    if (builders.empty())
        {
        BeDataAssert(false && "Component model contains no elements in the component's category. Therefore, instances will have no geometry.");
        return DgnDbStatus::NotFound;
        }
    #endif

    //  Generate and persist the geomeetry in one or more GeomParts. Note that we must create a unique GeomPart for each SubCategory.
    for (auto const& entry : builders)
        {
        DgnSubCategoryId clientsubcatid = entry.first;
        ElementGeometryBuilderPtr builder = entry.second;

        // *** WIP_COMPONENT_MODEL How can we look up and re-use GeomParts that are based on the same component and parameters?
        // Note: Don't assign a Code. If we did that, then we would have trouble with change-merging.
        DgnGeomPartPtr geomPart = DgnGeomPart::Create();
        builder->CreateGeomPart(db, true);
        builder->SetGeomStream(*geomPart);
        if (BSISUCCESS != db.GeomParts().InsertGeomPart(*geomPart))
            {
            BeAssert(false && "cannot create geompart for solution geometry -- what could have gone wrong?");
            return DgnDbStatus::WriteError;
            }
        geomBySubcategory.push_back(make_bpair(clientsubcatid, geomPart->GetId()));
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr ComponentModel::CreateCapturedSolutionElement(DgnDbStatus& status, DgnElement::Code const& icode,
    bvector<bpair<DgnSubCategoryId, DgnGeomPartId>> const& geomBySubcategory, HarvestedSolutionWriter& writer)
    {
    DgnDbR db = GetDgnDb();

    DgnElementPtr capturedSolutionElement = writer._CreateCapturedSolutionElement(status, m_compProps.GetItemECClassId(db), icode);
    if (!capturedSolutionElement.IsValid())
        return nullptr;

    GeometrySource* geom = capturedSolutionElement->ToGeometrySourceP();
    if (nullptr == geom)
        {
        BeAssert(false && "*** TBD: support for non-geometric components");
        return nullptr;
        }

    geom->SetCategoryId(m_compProps.QueryItemCategoryId(db));

    //  The element's geometry is just a list of references to the GeomParts
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*geom);
    for (bpair<DgnSubCategoryId, DgnGeomPartId> const& subcatAndGeom : geomBySubcategory)
        {
        Transform noTransform = Transform::FromIdentity();
        builder->Append(subcatAndGeom.first);
        builder->Append(subcatAndGeom.second, noTransform);
        }

    builder->SetGeomStreamAndPlacement(*geom);

    return capturedSolutionElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentModel::MakeCapturedSolutionElement(DgnDbStatus& status, DgnElement::Code const& icode, HarvestedSolutionWriter& writer)
    {
    DgnDbR db = GetDgnDb();

    if (&db != &writer._GetOutputDgnDb())
        {
        BeAssert(false && "you must import a component model before you can capture a solution");
        status = DgnDbStatus::WrongDgnDb;
        return nullptr;
        }

    if (!m_compProps.GetItemECClassId(db).IsValid())
        {
        BeAssert(false && "component ECClass not found. The target ECSchema must be imported and must contain solution ECClass before you can capture a solution or create instances.");
        status = DgnDbStatus::MissingDomain;
        return nullptr;
        }

    if (!m_compProps.QueryItemCategoryId(db).IsValid())
        {
        BeAssert(false && "component category not found. This is normally created by the target domain but may also be created a side-effect of importing the component model.");
        status = DgnDbStatus::InvalidCategory;
        return nullptr;
        }

    //  Gather up the current solution results. Note: a side-effect of harvesting is to create and insert GeomParts to hold the harvested solution geometry
    bvector<bpair<DgnSubCategoryId, DgnGeomPartId>> geomBySubcategory;
    bvector<DgnElementCPtr> nestedInstances;
    if (DgnDbStatus::Success != (status = HarvestSolution(geomBySubcategory, nestedInstances)))
        return nullptr;

    //  Create an element that holds this geometry. 
    DgnElementPtr capturedSolutionElement = CreateCapturedSolutionElement(status, icode, geomBySubcategory, writer);
    if (!capturedSolutionElement.IsValid())
        return nullptr;

    //  Write the captured solution element. Do this before dealing with nested instances, as they will need to refer to it as their parent.
    DgnElementCPtr storedSolutionElement = writer._WriteSolution(status, *capturedSolutionElement);
    if (!storedSolutionElement.IsValid())
        return nullptr;

    //  Nested instances become child elements.
    DgnCloneContext ctx;
    ElementCopier copier(ctx);
    for (auto nestedInstance : nestedInstances)
        {
        copier.MakeCopy(&status, writer._GetOutputModel(), *nestedInstance, DgnElement::Code(), storedSolutionElement->GetElementId());
        // *** TBD: 
        }

    return storedSolutionElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::OnElementCopied(DgnElementCR outputElement, DgnElementCR sourceElement, DgnCloneContext&)
    {
    DgnDbR db = sourceElement.GetDgnDb();
    BeAssert(&db == &outputElement.GetDgnDb());

    DgnModelId cmid;
    ModelSolverDef::ParameterSet prms;
    if (BE_SQLITE_OK == queryComponentModelFromSolution(cmid, prms, db, sourceElement.GetElementId()))
        {
        ComponentModelPtr cm = db.Models().Get<ComponentModel>(cmid);
        if (!cm.IsValid())
            return;
        if (IsCapturedSolutionCode(sourceElement.GetCode()))
            {
            //  Copying a non-unique solution means that we are creating an instance
            createInstanceOfTemplateRelationship(outputElement, sourceElement);
            return;
            }
        // Copying a unique/singleton solution - preserve it as such
        createSolutionOfComponentRelationship(outputElement, *cm, prms);
        createInstanceOfTemplateRelationship(outputElement, outputElement);
        return;
        }

    DgnElementCPtr solutionElement = sourceElement.GetDgnDb().Elements().GetElement(queryTemplateItemFromInstance(db, sourceElement.GetElementId()));
    if (solutionElement.IsValid())
        {
        //  When we copy an instance, we must connect to the solution from which the original was created. The copy is just another instance.
        createInstanceOfTemplateRelationship(outputElement, *solutionElement);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::OnElementImported(DgnElementCR outputElement, DgnElementCR sourceElement, DgnImportContext& importer)
    {
    DgnDbR sourceDb = importer.GetSourceDb();
    DgnDbR destDb = importer.GetDestinationDb();

    //  Solutions must remap to their component models
    DgnModelId sourceComponentModelId;
    ModelSolverDef::ParameterSet sourceParameters;
    queryComponentModelFromSolution(sourceComponentModelId, sourceParameters, sourceDb, sourceElement.GetElementId());
    ComponentModelPtr sourceComponentModel = sourceDb.Models().Get<ComponentModel>(sourceComponentModelId);
    bool isSolution = false;
    if (sourceComponentModel.IsValid())
        {
        //  Look up the component in the destination by its name
        ComponentModelPtr destComponentModel = ComponentModel::FindModelByName(destDb, sourceComponentModel->GetModelName());
        if (!destComponentModel.IsValid())
            return; // if the component model hasn't been imported, then this solution element must become an orphan
        createSolutionOfComponentRelationship(outputElement, *destComponentModel, sourceParameters);     // set up the copy as a solution of the target's copy of the component
        isSolution = true;
        }

    //  Instances must remap to their solutions
    DgnElementCPtr sourceSolutionElement = sourceDb.Elements().GetElement(queryTemplateItemFromInstance(sourceDb, sourceElement.GetElementId()));
    if (sourceSolutionElement.IsValid())
        {
        //  Look up the solution in the destination by its code
        DgnElementCPtr destSolutionElement;
        if (isSolution)
            destSolutionElement = &outputElement;       // this is a unique/singleton instance: it is both an instance and a solution
        else
            destSolutionElement = destDb.Elements().GetElement(destDb.Elements().QueryElementIdByCode(sourceSolutionElement->GetCode()));

        if (!destSolutionElement.IsValid())
            return; // if solution has not been imported, then this instance must become an orphan

        createInstanceOfTemplateRelationship(outputElement, *destSolutionElement);  // set up the copy as an instance of the same solution in the destination db
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentModel::HarvestedSolutionInserter::_WriteSolution(DgnDbStatus& status, DgnElementR el)
    {
    if (&m_cm.GetDgnDb() != &m_destModel.GetDgnDb())
        {
        BeAssert(false && "you must import a component model before you can capture a solution");
        status = DgnDbStatus::WrongDgnDb;
        return nullptr;
        }

    DgnElementCPtr storedDgnElem = el.Insert(&status);
    if (!storedDgnElem.IsValid())
        return nullptr;

    DgnElementCPtr storedPhysicalElem = storedDgnElem->ToPhysicalElement();

    createSolutionOfComponentRelationship(*storedPhysicalElem, m_cm, m_cm.GetSolver().GetParameters());

    #ifndef NDEBUG
    DgnModelId qmid;
    ModelSolverDef::ParameterSet qparams;
    BeAssert(BE_SQLITE_OK == queryComponentModelFromSolution(qmid, qparams, m_destModel.GetDgnDb(), storedPhysicalElem->GetElementId()) 
                && qmid == m_cm.GetModelId() 
                && qparams == m_cm.GetSolver().GetParameters());
    #endif
    
    return storedPhysicalElem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentModel::HarvestedSingletonInserter::_WriteSolution(DgnDbStatus& status, DgnElementR el)
    {
    auto elOut = T_Super::_WriteSolution(status, el);
    if (!elOut.IsValid())
        return nullptr;

    createInstanceOfTemplateRelationship(*elOut, *elOut); // set up a singleton as an instance of itself.  That makes it much simpler to implement the code that cleans up after a component model is deleted.
    return elOut;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentModel::MakeInstanceOfSolution(DgnDbStatus* statusOut, DgnModelR targetModel, DgnElementCR catalogItem, DgnElement::Code const& code)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(status, statusOut);
    
    DgnDbR db = catalogItem.GetDgnDb();
    if (&db != &targetModel.GetDgnDb())
        {
        status = DgnDbStatus::WrongDgnDb;
        BeAssert(false && "Catalog and target models must be in the same Db");
        return nullptr;
        }

    // Look up the component model that generated the catalog item
    DgnModelId mid;
    ModelSolverDef::ParameterSet qparams;
    if (BE_SQLITE_OK != queryComponentModelFromSolution(mid, qparams, db, catalogItem.GetElementId()))
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

    DgnElement::Code icode = code;
    if (!code.IsValid())
        {
        //  Generate the item code. This will be a null code, unless there's a specified authority for the componentmodel.
        if (!cmm->m_compProps.m_itemCodeAuthority.empty())  // WARNING: Don't call GetAuthority with an invalid authority name. It will always prepare a statement and will not cache the (negative) answer.
            {
            DgnAuthorityCPtr authority = db.Authorities().GetAuthority(cmm->m_compProps.m_itemCodeAuthority.c_str());
            if (authority.IsValid())
                icode = authority->CreateDefaultCode();  // *** WIP_COMPONENT_MODEL -- how do I ask an Authority to issue a code?
            }    
        }

    //  Creating the item is just a matter of copying the catalog item (and its children)
    DgnCloneContext ctx;
    ElementCopier copier(ctx);
    copier.SetCopyChildren(true);
    DgnElementCPtr inst = copier.MakeCopy(&status, targetModel, catalogItem, icode); // >> OnElementCopied, which creates the instance->solution relationship
    if (!inst.IsValid())
        return nullptr;

    BeAssert(queryTemplateItemFromInstance(db, inst->GetElementId()) == catalogItem.GetElementId());

    return inst;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentModel::MakeInstanceOfSolution(DgnDbStatus* statusOut, DgnModelR targetModel, Utf8StringCR capturedSolutionName, ModelSolverDef::ParameterSet const& params, DgnElement::Code const& code)
    {
    //  If this is an instance of an existing type, just make a copy of the type
    DgnElementCPtr typeElem;
    ModelSolverDef::ParameterSet typeParams;
    if ((DgnDbStatus::Success == QuerySolutionByName(typeElem, typeParams, capturedSolutionName)) && (params == typeParams))
        return MakeInstanceOfSolution(statusOut, targetModel, *typeElem, code);
    
    // Otherwise, capture a unique/singleton solution
    DgnDbStatus ALLOW_NULL_OUTPUT(status, statusOut);
    if (DgnDbStatus::Success != (status = Solve(params)))
        return nullptr;

    HarvestedSingletonInserter inserter(targetModel, *this);
    return MakeCapturedSolutionElement(status, code, inserter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::QuerySolutionInfo(DgnModelId& cmid, ModelSolverDef::ParameterSet& params, DgnElementCR catalogItem)
    {
    return (BE_SQLITE_OK == queryComponentModelFromSolution(cmid, params, catalogItem.GetDgnDb(), catalogItem.GetElementId()))? DgnDbStatus::Success: DgnDbStatus::NotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::QuerySolutionInfo(ModelSolverDef::ParameterSet& params, DgnElementCR catalogItem)
    {
    DgnModelId cmid;
    DgnDbStatus status = QuerySolutionInfo(cmid, params, catalogItem);
    if (DgnDbStatus::Success != status)
        return status;

    if (GetModelId() != cmid)
        return DgnDbStatus::WrongModel;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::QuerySolutionById(DgnElementCPtr& ele, ModelSolverDef::ParameterSet& params, DgnElementId solutionId)
    {
    ele = GetDgnDb().Elements().GetElement(solutionId);
    if (!ele.IsValid())
        return DgnDbStatus::NotFound;

    return QuerySolutionInfo(params, *ele);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::QuerySolutionByName(DgnElementCPtr& ele, ModelSolverDef::ParameterSet& params, Utf8StringCR capturedSolutionName)
    {
    DgnElement::Code icode = CreateCapturedSolutionCode(capturedSolutionName);
    if (!icode.IsValid())
        return DgnDbStatus::NotFound;

    ele = GetDgnDb().Elements().GetElement(GetDgnDb().Elements().QueryElementIdByCode(icode));
    if (!ele.IsValid())
        return DgnDbStatus::NotFound;

    return QuerySolutionInfo(params, *ele);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::QuerySolutionByParameters(DgnElementCPtr& ele, ModelSolverDef::ParameterSet const& params)
    {
    bvector<DgnElementId> types;
    QuerySolutions(types);
    for (auto teid : types)
        {
        DgnElementCPtr thisType;
        ModelSolverDef::ParameterSet thisParams;
        QuerySolutionById(thisType, thisParams, teid);
        if (thisParams == params)
            return DgnDbStatus::Success;
        }
    return DgnDbStatus::NotFound;
    }
