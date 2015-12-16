/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ComponentModel.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnDbTables.h>
#include <DgnPlatform/DgnScript.h>

#define CDEF_CA_SANDBOX "Sandbox"
#define CDEF_CA_ELEMENT_GENERATOR "ElementGenerator"
#define CDEF_CA_CATEGORY "Category"
#define CDEF_CA_CODE_AUTHORITY "CodeAuthority"

//=======================================================================================
// Base for helper classes that assist in making harvested solutions persistent
//=======================================================================================
struct HarvestedSolutionWriter
    {
    protected:
    ComponentModel& m_cm;
    DgnModelR m_destModel;
    public:
    virtual DgnDbR _GetOutputDgnDb() {return m_destModel.GetDgnDb();}
    virtual DgnModelR _GetOutputModel() {return m_destModel;}
    virtual DgnElementPtr _CreateInstance(DgnDbStatus& status, DgnClassId iclass, DgnElement::Code const& icode); // (rarely need to override this)
    virtual DgnElementCPtr _WriteInstance(DgnDbStatus&, DgnElementR) = 0;
    HarvestedSolutionWriter(DgnModelR m, ComponentModel& c) : m_destModel(m), m_cm(c) {;}
    };

//=======================================================================================
// Makes a "catalog item" persistent
//=======================================================================================
struct HarvestedSolutionInserter : HarvestedSolutionWriter
    {
    DEFINE_T_SUPER(HarvestedSolutionWriter)
    protected:
    DgnElementCPtr _WriteInstance(DgnDbStatus&, DgnElementR) override;
    public:
    HarvestedSolutionInserter(DgnModelR m, ComponentModel& c) : HarvestedSolutionWriter(m,c) {;}
    };

//=======================================================================================
// Makes a unique/single solution persistent
//=======================================================================================
struct HarvestedSingletonInserter : HarvestedSolutionInserter
    {
    DEFINE_T_SUPER(HarvestedSolutionInserter)
    protected:
    DgnElementCPtr _WriteInstance(DgnDbStatus&, DgnElementR) override;
    public:
    HarvestedSingletonInserter(DgnModelR m, ComponentModel& c) : HarvestedSolutionInserter(m,c) {;}
    };

//=======================================================================================
// Updates any kind of persistent solution element
//=======================================================================================
struct HarvestedSolutionUpdater : HarvestedSolutionWriter
    {
    DEFINE_T_SUPER(HarvestedSolutionWriter)
    protected:
    DgnElementCPtr m_existing;
    DgnElementCPtr _WriteInstance(DgnDbStatus&, DgnElementR) override;
    public:
    HarvestedSolutionUpdater(DgnModelR m, ComponentModel& c, DgnElementCR e) : HarvestedSolutionWriter(m, c), m_existing(&e) {;}
    };

//=======================================================================================
// Harvests component solution geometry
//=======================================================================================
struct ComponentGeometryHarvester
    {
    private:
    ComponentDef& m_cdef;

    DgnDbStatus HarvestSandbox(bvector<bpair<DgnSubCategoryId, DgnGeomPartId>>& geomBySubcategory, bvector<DgnElementCPtr>& nestedInstances);
    DgnElementPtr CreateInstance(DgnDbStatus& status, DgnElement::Code const& icode, bvector<bpair<DgnSubCategoryId, DgnGeomPartId>> const&, HarvestedSolutionWriter& writer);

    public:
    ComponentGeometryHarvester(ComponentDef c) : m_cdef(c) {;}

    DgnElementCPtr MakeInstance(DgnDbStatus& status, DgnElement::Code const& icode, HarvestedSolutionWriter& WriterHandler);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr HarvestedSolutionWriter::_CreateInstance(DgnDbStatus& status, DgnClassId iclass, DgnElement::Code const& icode)
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
        BeAssert(false && "HarvestSandbox creates only PhysicalElements");
        status = DgnDbStatus::WrongClass;
        return nullptr;
        }
    return capturedSolutionElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr HarvestedSolutionInserter::_WriteInstance(DgnDbStatus& status, DgnElementR el)
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

    return storedPhysicalElem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr HarvestedSingletonInserter::_WriteInstance(DgnDbStatus& status, DgnElementR el)
    {
    auto elOut = T_Super::_WriteInstance(status, el);
    if (!elOut.IsValid())
        return nullptr;

    // *** WIP_COMPONENT
    // createInstanceOfTemplateRelationship(*elOut, *elOut); // set up a singleton as an instance of itself.  That makes it much simpler to implement the code that cleans up after a component model is deleted.
    return elOut;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentGeometryHarvester::HarvestSandbox(bvector<bpair<DgnSubCategoryId, DgnGeomPartId>>& geomBySubcategory, bvector<DgnElementCPtr>& nestedInstances)
    {
    // ***
    // *** WIP_COMPONENT_MODEL *** the logic below is not complete. Must must add another dimension -- we must break out builders by same ElemDisplayParams, not just subcategory
    // ***

    DgnDbR db = m_cdef.GetDgnDb();

    DgnCategoryId harvestableGeometryCategoryId = m_cdef.QueryCategoryId();

    //  Gather geometry by SubCategory
    bmap<DgnSubCategoryId, ElementGeometryBuilderPtr> builders;     
    auto cm = m_cdef.GetSandbox();
    cm.FillModel();
    for (auto const& mapEntry : cm)
        {
        GeometrySourceCP componentElement = mapEntry.second->ToGeometrySource();
        if (nullptr == componentElement)
            continue;

        //  Nested instances will become child elements or will be folded into the solution. That will be handled below.
#ifdef WIP_COMPONENT
        if (isInstanceOfComponent(*mapEntry.second))
            {
            PhysicalElementCP pnested = mapEntry.second->ToPhysicalElement();
            if (nullptr != pnested)
                nestedInstances.push_back(pnested);
            else
                {
                BeDataAssert(false && "HarvestSandbox supports only PhysicalElements.");
                }
            continue;
            }
#endif

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
DgnElementPtr ComponentGeometryHarvester::CreateInstance(DgnDbStatus& status, DgnElement::Code const& icode,
    bvector<bpair<DgnSubCategoryId, DgnGeomPartId>> const& geomBySubcategory, HarvestedSolutionWriter& writer)
    {
    DgnElementPtr capturedSolutionElement = writer._CreateInstance(status, DgnClassId(m_cdef.GetECClass().GetId()), icode);
    if (!capturedSolutionElement.IsValid())
        return nullptr;

    GeometrySource* geom = capturedSolutionElement->ToGeometrySourceP();
    if (nullptr == geom)
        {
        BeAssert(false && "*** TBD: support for non-geometric components. Harvest aspects?");
        return capturedSolutionElement;
        }

    geom->SetCategoryId(m_cdef.QueryCategoryId());

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*geom);
    for (bpair<DgnSubCategoryId, DgnGeomPartId> const& subcatAndGeom : geomBySubcategory)
        {
        Transform noTransform = Transform::FromIdentity();
        builder->Append(subcatAndGeom.first);
        builder->Append(subcatAndGeom.second, noTransform);
        }

    builder->SetGeomStreamAndPlacement(*geom);

    // *** TBD: Other Aspects??

    return capturedSolutionElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentGeometryHarvester::MakeInstance(DgnDbStatus& status, DgnElement::Code const& icode, HarvestedSolutionWriter& writer)
    {
    DgnDbR db = m_cdef.GetDgnDb();

    if (&db != &writer._GetOutputDgnDb())
        {
        BeAssert(false && "you must import a component model before you can capture a solution");
        status = DgnDbStatus::WrongDgnDb;
        return nullptr;
        }

    //  Gather up the current solution results. Note: a side-effect of harvesting is to create and insert GeomParts to hold the harvested solution geometry
    bvector<bpair<DgnSubCategoryId, DgnGeomPartId>> geomBySubcategory;
    bvector<DgnElementCPtr> nestedInstances;
    if (DgnDbStatus::Success != (status = HarvestSandbox(geomBySubcategory, nestedInstances)))
        return nullptr;

    //  Create an element that holds this geometry. 
    DgnElementPtr capturedSolutionElement = CreateInstance(status, icode, geomBySubcategory, writer);
    if (!capturedSolutionElement.IsValid())
        return nullptr;

    //  Write the captured solution element. Do this before dealing with nested instances, as they will need to refer to it as their parent.
    DgnElementCPtr storedSolutionElement = writer._WriteInstance(status, *capturedSolutionElement);
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
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentDef::ComponentDef(DgnDbR db, ECN::ECClassCR componentDefClass)
    : m_db(db), m_sandbox(nullptr), m_class(componentDefClass)
    {
    ECN::ECClassCP caClass = db.Schemas().GetECClass(DGN_ECSCHEMA_NAME, "ComponentSpecification");
    if (nullptr == caClass)
        {
        m_isValid = false;
        return;
        }
    m_ca = m_class.GetCustomAttribute(*caClass);
    if (!m_ca.IsValid())
        {
        m_isValid = false;
        return;
        }
    m_isValid = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentDef::~ComponentDef()
    {
    if (m_sandbox.IsValid() && UsesTemporarySandbox())
        m_sandbox->Delete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentDef::GetCaValueString(Utf8CP propName) const
    {
    ECN::ECValue v;
    m_ca->GetValue(v, propName);
    return v.ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentDef::GetCategoryName() const {return GetCaValueString(CDEF_CA_CATEGORY);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentDef::GetCodeAuthorityName() const {return GetCaValueString(CDEF_CA_CODE_AUTHORITY);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityCPtr ComponentDef::GetCodeAuthority() const
    {
    if (GetCodeAuthorityName().empty())  // WARNING: Don't call GetAuthority with an invalid authority name. It will always prepare a statement and will not cache the (negative) answer.
        return nullptr;
    
    return m_db.Authorities().GetAuthority(GetCodeAuthorityName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId ComponentDef::QueryCategoryId() const
    {
    return DgnCategory::QueryCategoryId(GetCategoryName().c_str(), m_db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ComponentDef::UsesTemporarySandbox() const
    {
    return GetCaValueString(CDEF_CA_SANDBOX).empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr ComponentDef::MakeVariationSpec()
    {
    return m_class.GetDefaultStandaloneEnabler()->CreateInstance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentDef::GetGeneratedName() const
    {
    Utf8String name(m_class.GetFullName());
    DgnDbTable::ReplaceInvalidCharacters(name, DgnModels::GetIllegalCharacters(), '_');
    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentModelR ComponentDef::GetSandbox() 
    {
    if (m_sandbox.IsValid())
        return *m_sandbox;
        
    Utf8String sandBoxName = GetCaValueString(CDEF_CA_SANDBOX);
    if (sandBoxName.empty())
        sandBoxName = m_db.Models().GetUniqueModelName(GetGeneratedName().c_str());

    DgnModel::Code sandBoxCode = DgnModel::CreateModelCode(sandBoxName);

    DgnModelId sbid;
    m_db.Models().QueryModelId(sandBoxCode);
    if (sbid.IsValid())
        m_sandbox = m_db.Models().Get<ComponentModel>(sbid);

    if (m_sandbox.IsValid())
        return *m_sandbox;

    ComponentModel::CreateParams cmcparms(m_db, DgnClassId(m_db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "ComponentModel")), sandBoxCode);
    m_sandbox = new ComponentModel(ComponentModel::CreateParams(cmcparms));
    m_sandbox->Insert("", false);

    return *m_sandbox;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentDef::GenerateGeometry(ECN::IECInstanceCR variationSpecIn)
    {
    GetSandbox();

    ECN::IECInstancePtr instanceTemplate = MakeVariationSpec();
    instanceTemplate->CopyValues(variationSpecIn);

    Utf8String scriptName = GetCaValueString(CDEF_CA_ELEMENT_GENERATOR);
    if (scriptName.empty())
        {
        GeometryGenerator* gg = dynamic_cast<GeometryGenerator*>(m_sandbox.get());
        if (nullptr == gg)
            return DgnDbStatus::Success;    // assume the sandbox contains static geometry

        return gg->_GenerateGeometry(*instanceTemplate);
        }
        
    StatusInt retval;
    if (DgnDbStatus::Success != DgnScript::ExecuteModelSolver(retval, *m_sandbox, scriptName.c_str(), *instanceTemplate) || (0 == retval))
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentDef::MakeInstance0(DgnDbStatus* statusOut, DgnModelR targetModel, ECN::IECInstanceCR parameters, DgnElement::Code const& code)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(status, statusOut);
    if (DgnDbStatus::Success != (status = GenerateGeometry(parameters)))
        return nullptr;

    HarvestedSolutionInserter inserter(targetModel, GetSandbox());
    ComponentGeometryHarvester harvester(*this);
    return harvester.MakeInstance(status, code, inserter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentDef::MakeVariation(DgnDbStatus* statusOut, DgnModelR targetModel, ECN::IECInstanceCR parameters, Utf8StringCR variationName)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(status, statusOut);
    if (variationName.empty())
        {
        BeAssert(false && "a variation must have a name");
        status = DgnDbStatus::InvalidName;
        return nullptr;
        }

    DgnElement::Code vcode = CreateCapturedSolutionCode(variationName);

    return MakeInstance0(statusOut, targetModel, parameters, vcode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentDef::MakeInstance(DgnDbStatus* statusOut, DgnModelR targetModel, Utf8StringCR variationName, ECN::IECInstanceCR parameters, DgnElement::Code const& code)
    {
    if (!variationName.empty())
        {
        //  If this is an instance of an existing type, just make a copy of the type
        DgnElement::Code vcode = CreateCapturedSolutionCode(variationName);
        DgnElementCPtr variation = GetDgnDb().Elements().GetElement(GetDgnDb().Elements().QueryElementIdByCode(vcode));
        if (variation.IsValid() && HaveEqualParameters(parameters, *GetParameters(*variation)))
            return MakeInstanceOfVariation(statusOut, targetModel, *variation, code);
        }
            
    // Otherwise, capture a unique/singleton solution
    return MakeInstance0(statusOut, targetModel, parameters, code);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentDef::MakeInstanceOfVariation(DgnDbStatus* statusOut, DgnModelR targetModel, DgnElementCR variation, DgnElement::Code const& code)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(status, statusOut);
    
    DgnDbR db = variation.GetDgnDb();
    if (&db != &targetModel.GetDgnDb())
        {
        status = DgnDbStatus::WrongDgnDb;
        BeAssert(false && "Catalog and target models must be in the same Db");
        return nullptr;
        }

    ComponentDef cdef(db, *variation.GetElementClass());
    if (!cdef.IsValid())
        {
        status = DgnDbStatus::WrongClass;
        return nullptr;
        }

    DgnElement::Code icode = code;
    if (!code.IsValid())
        {
        //  Generate the item code. This will be a null code, unless there's a specified authority for the componentmodel.
        DgnAuthorityCPtr authority = cdef.GetCodeAuthority();
        if (authority.IsValid())
            icode = authority->CreateDefaultCode();  // *** WIP_COMPONENT_MODEL -- how do I ask an Authority to issue a code?
        }

    //  Creating the item is just a matter of copying the catalog item (and its children)
    DgnCloneContext ctx;
    ElementCopier copier(ctx);
    copier.SetCopyChildren(true);
    DgnElementCPtr inst = copier.MakeCopy(&status, targetModel, variation, icode); // >> OnElementCopied, which creates the instance->solution relationship
    if (!inst.IsValid())
        return nullptr;

    // *** WIP_COMPONENT
    //BeAssert(queryInstantiationOfTemplateTarget(db, inst->GetElementId()) == catalogItem.GetElementId());

    return inst;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr ComponentDef::GetParameters(DgnElementCR el)
    {
    ComponentDef cdef(el.GetDgnDb(), *el.GetElementClass());
    if (!cdef.IsValid())
        return nullptr;
    Utf8PrintfString sql("SELECT * FROM %s.%s WHERE ECInstanceId=?", el.GetElementClass()->GetSchema().GetNamespacePrefix().c_str(), el.GetElementClass()->GetName().c_str());
    auto ecsql = el.GetDgnDb().GetPreparedECSqlStatement(sql);
    ecsql->BindId(1, el.GetElementId());
    ECInstanceECSqlSelectAdapter selector(*ecsql);
    return selector.GetInstance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ComponentDef::HaveEqualParameters(ECN::IECInstanceCR lhs, ECN::IECInstanceCR rhs)
    {
    if (&lhs.GetClass() != &rhs.GetClass())
        return false;
    for (auto const& prop : lhs.GetClass().GetProperties())
        {
        if (prop->GetName().Equals("ECInstanceId"))
            continue;

        ECValue l, r;
        lhs.GetValue(l, prop->GetName().c_str());
        rhs.GetValue(r, prop->GetName().c_str());
        if (l.IsNull())
            continue;
            
        if (!l.Equals(r))
            return false;
        }
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus ComponentDef::ImportSchema(DgnDbR db, ECN::ECSchemaR schema)
    {
    ECN::ECSchemaReadContextPtr contextPtr = ECN::ECSchemaReadContext::CreateContext();
    contextPtr->AddSchema(schema);
    if (BentleyStatus::SUCCESS != db.Schemas().ImportECSchemas(contextPtr->GetCache()))
        return DgnDbStatus::BadSchema;

    //db.Domains().SyncWithSchemas(); no need to do this. A component's class will not have a handler.

    return DgnDbStatus::Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_COMPONENT
DgnDbStatus ComponentDef::UpdateSolutionsAndInstances()
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
        DgnElementPtr newCatalogItem = HarvestSandbox(status, *sourceCatalogItem->GetModel()->ToPhysicalModelP(), sourceCatalogItem->GetPlacement(), sourceCatalogItem->GetCode());
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
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentModel::_OnDelete()
    {
#ifdef WIP_COMPONENT
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
#endif
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::OnElementCopied(DgnElementCR outputElement, DgnElementCR sourceElement, DgnCloneContext&)
    {
#ifdef WIP_COMPONENT
    DgnDbR db = sourceElement.GetDgnDb();
    BeAssert(&db == &outputElement.GetDgnDb());

    DgnModelId cmid;
    ModelSolverDef::ParameterSet prms;
    if (BE_SQLITE_OK == querySolutionOfComponentTargetAndParameters(cmid, prms, db, sourceElement.GetElementId()))
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

    DgnElementCPtr solutionElement = sourceElement.GetDgnDb().Elements().GetElement(queryInstantiationOfTemplateTarget(db, sourceElement.GetElementId()));
    if (solutionElement.IsValid())
        {
        //  When we copy an instance, we must connect to the solution from which the original was created. The copy is just another instance.
        createInstanceOfTemplateRelationship(outputElement, *solutionElement);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::OnElementImported(DgnElementCR outputElement, DgnElementCR sourceElement, DgnImportContext& importer)
    {
#ifdef WIP_COMPONENT
    DgnDbR sourceDb = importer.GetSourceDb();
    DgnDbR destDb = importer.GetDestinationDb();

    //  Solutions must remap to their component models
    DgnModelId sourceComponentModelId;
    ModelSolverDef::ParameterSet sourceParameters;
    querySolutionOfComponentTargetAndParameters(sourceComponentModelId, sourceParameters, sourceDb, sourceElement.GetElementId());
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
    DgnElementCPtr sourceSolutionElement = sourceDb.Elements().GetElement(queryInstantiationOfTemplateTarget(sourceDb, sourceElement.GetElementId()));
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
#endif
    }

