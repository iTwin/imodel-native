/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ComponentModel.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnDbTables.h>
#include <DgnPlatform/DgnScript.h>

#define CDEF_CA_MODEL "Model"
#define CDEF_CA_ELEMENT_GENERATOR "ElementGenerator"
#define CDEF_CA_CATEGORY "Category"
#define CDEF_CA_CODE_AUTHORITY "CodeAuthority"
#define CDEF_CA_INPUTS "Inputs"
#define CDEF_CA_ScriptOnlyParameters "ScriptOnlyParameters"

#define ECSUCCESS(STMT) if (ECN::ECObjectsStatus::Success != (ecstatus = STMT)) {BeAssert(false); return nullptr;}

void ComponentDef::DumpScriptOnlyParameters(ECN::IECInstanceCR props, Utf8CP title)
    {
    NativeLogging::LoggingManager::GetLogger("ComponentModelTest")->messagev(NativeLogging::SEVERITY::LOG_FATAL, title);
    ECN::AdhocPropertyQuery pquery(props, "ScriptOnlyParameters");
    auto pcount = pquery.GetCount();
    for (uint32_t i = 0; i < pcount; ++i)
        {
        ECN::ECValue value;
        pquery.GetValue(value, i);
        Utf8String name;
        pquery.GetName(name, i);
        Utf8PrintfString propdesc("%s = %s", name.c_str(), value.ToString().c_str());
        NativeLogging::LoggingManager::GetLogger("ComponentModelTest")->messagev(NativeLogging::SEVERITY::LOG_FATAL, propdesc.c_str());
        }
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
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
static ECN::ECClassCP getECClassByFullName(DgnDbR db, Utf8StringCR fullname)
    {
    Utf8String ns, cls;
    std::tie(ns, cls) = parseFullECClassName(fullname.c_str());
    return db.Schemas().GetECClass(ns.c_str(), cls.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
static ECN::IECInstancePtr getComponentSpecCA(DgnDbR db, ECN::ECClassCR cls)
    {
     ECN::ECClassCP caClass = db.Schemas().GetECClass(DGN_ECSCHEMA_NAME, "ComponentSpecification");
    if (nullptr == caClass)
        return nullptr;

    return cls.GetCustomAttribute(*caClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isInstanceOfComponent(DgnElementCR el)
    {
    return (nullptr != el.GetElementClass()) && getComponentSpecCA(el.GetDgnDb(), *el.GetElementClass()).IsValid();
    }

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
    virtual DgnElementPtr _CreateInstance(DgnDbStatus& status, DgnClassId iclass, DgnCodeCR icode); // (rarely need to override this)
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

    DgnDbStatus HarvestModel(bvector<bpair<DgnSubCategoryId, DgnGeometryPartId>>& geomBySubcategory, bvector<DgnElementCPtr>& nestedInstances);
    DgnElementPtr CreateInstance(DgnDbStatus& status, DgnCode const& icode, bvector<bpair<DgnSubCategoryId, DgnGeometryPartId>> const&, HarvestedSolutionWriter& writer);

    public:
    ComponentGeometryHarvester(ComponentDef& c) : m_cdef(c) {;}

    DgnElementCPtr MakeInstance(DgnDbStatus& status, DgnCode const& icode, HarvestedSolutionWriter& WriterHandler);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr HarvestedSolutionWriter::_CreateInstance(DgnDbStatus& status, DgnClassId iclass, DgnCode const& icode)
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
    if (!dgnElem->Is3d())
        {
        BeAssert(false && "HarvestModel creates only PhysicalElements");
        status = DgnDbStatus::WrongClass;
        return nullptr;
        }
    return dgnElem;
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

    if (!storedDgnElem->Is3d())
        return nullptr;

    return storedDgnElem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr HarvestedSingletonInserter::_WriteInstance(DgnDbStatus& status, DgnElementR el)
    {
    auto elOut = T_Super::_WriteInstance(status, el);
    if (!elOut.IsValid())
        return nullptr;

    return elOut;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentGeometryHarvester::HarvestModel(bvector<bpair<DgnSubCategoryId, DgnGeometryPartId>>& geomBySubcategory, bvector<DgnElementCPtr>& nestedInstances)
    {
    // ***
    // *** WIP_COMPONENT_MODEL *** the logic below is not complete. Must must add another dimension -- we must break out builders by same ElemDisplayParams, not just subcategory
    // ***

    DgnDbR db = m_cdef.GetDgnDb();

    DgnCategoryId harvestableGeometryCategoryId = m_cdef.QueryCategoryId();

    //  Gather geometry by SubCategory
    bmap<DgnSubCategoryId, GeometryBuilderPtr> builders;     
    auto cm = m_cdef.GetModel();
    cm.FillModel();
    for (auto const& mapEntry : cm)
        {
        GeometrySourceCP componentElement = mapEntry.second->ToGeometrySource();
        if (nullptr == componentElement)
            continue;

        //  Nested instances will become child elements or will be folded into the solution. That will be handled below.
        if (isInstanceOfComponent(*mapEntry.second))
            {
            nestedInstances.push_back(mapEntry.second);
            continue;
            }

        //  Only solution elements in the component's Category are collected. The rest are construction/annotation geometry.
        if (componentElement->GetCategoryId() != harvestableGeometryCategoryId)
            continue;

        GeometryCollection gcollection(*componentElement);
        for (auto iter : gcollection)
            {
            GeometricPrimitivePtr xgeom = iter.GetGeometryPtr();
            if (!xgeom.IsValid()) // what to do about GeometryParts?? - BB
                continue;

            //  Look up the subcategory ... IN THE CLIENT DB
            GeometryParamsCR dparams = iter.GetGeometryParams();
            DgnSubCategoryId clientsubcatid = dparams.GetSubCategoryId();

            GeometryBuilderPtr& builder = builders [clientsubcatid];
            if (!builder.IsValid())
                builder = GeometryBuilder::CreateGeometryPart(db, true);

            // Since each little piece of geometry can have its own transform, we must
            // build the transforms back into them in order to assemble them into a single geomstream.
            // It's all relative to 0,0,0 in the component model, so it's fine to do this.
            Transform trans = iter.GetGeometryToWorld(); // A component model is in its own local coordinate system, so "World" just means relative to local 0,0,0
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

    //  Generate and persist the geomeetry in one or more GeometryParts. Note that we must create a unique GeometryPart for each SubCategory.
    for (auto const& entry : builders)
        {
        DgnSubCategoryId clientsubcatid = entry.first;
        GeometryBuilderPtr builder = entry.second;

        // *** WIP_COMPONENT_MODEL How can we look up and re-use GeometryParts that are based on the same component and parameters?
        // Note: Don't assign a Code. If we did that, then we would have trouble with change-merging.
        DgnGeometryPartPtr geomPart = DgnGeometryPart::Create(db);
        builder->CreateGeometryPart(db, true);
        builder->Finish(*geomPart);
        if (!db.Elements().Insert(*geomPart).IsValid())
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
DgnElementPtr ComponentGeometryHarvester::CreateInstance(DgnDbStatus& status, DgnCode const& icode,
    bvector<bpair<DgnSubCategoryId, DgnGeometryPartId>> const& geomBySubcategory, HarvestedSolutionWriter& writer)
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

    GeometryBuilderPtr builder = GeometryBuilder::Create(*geom);
    for (bpair<DgnSubCategoryId, DgnGeometryPartId> const& subcatAndGeom : geomBySubcategory)
        {
        Transform noTransform = Transform::FromIdentity();
        builder->Append(subcatAndGeom.first);
        builder->Append(subcatAndGeom.second, noTransform);
        }

    builder->Finish(*geom);

    // *** TBD: Other Aspects??

    return capturedSolutionElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentGeometryHarvester::MakeInstance(DgnDbStatus& status, DgnCode const& icode, HarvestedSolutionWriter& writer)
    {
    DgnDbR db = m_cdef.GetDgnDb();

    if (&db != &writer._GetOutputDgnDb())
        {
        BeAssert(false && "you must import a component model before you can capture a solution");
        status = DgnDbStatus::WrongDgnDb;
        return nullptr;
        }

    //  Gather up the current solution results. Note: a side-effect of harvesting is to create and insert GeometryParts to hold the harvested solution geometry
    bvector<bpair<DgnSubCategoryId, DgnGeometryPartId>> geomBySubcategory;
    bvector<DgnElementCPtr> nestedInstances;
    if (DgnDbStatus::Success != (status = HarvestModel(geomBySubcategory, nestedInstances)))
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
        copier.MakeCopy(&status, writer._GetOutputModel(), *nestedInstance, DgnCode(), storedSolutionElement->GetElementId());
        // *** TBD: 
        }

    BeAssert(!storedSolutionElement->ToGeometrySource3d() || storedSolutionElement->ToGeometrySource3d()->GetCategoryId() == m_cdef.QueryCategoryId());
    BeAssert(!storedSolutionElement->ToGeometrySource2d() || storedSolutionElement->ToGeometrySource2d()->GetCategoryId() == m_cdef.QueryCategoryId());
    BeAssert(storedSolutionElement->GetElementClass() == &m_cdef.GetECClass());
    return storedSolutionElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentDef::ComponentDef(DgnDbR db, ECN::ECClassCR componentDefClass)
    : m_db(db), m_model(nullptr), m_class(componentDefClass)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_ECOBJECTS_PROBLEM // *** DerivedClassses collection is not updated when we insert a new ECClass
void ComponentDef::QueryComponentDefs(bvector<DgnClassId>& componentDefs, DgnDbR db, ECN::ECClassCR baseClassIn)
    {
    auto componentSpecificationCA = db.Schemas().GetECClass(DGN_ECSCHEMA_NAME, "ComponentSpecification");
    if (nullptr == componentSpecificationCA)
        return;

    bvector<ECN::ECClassCP> baseClasses;
    baseClasses.push_back(&baseClassIn);

    while (!baseClasses.empty())
        {
        ECN::ECClassCP baseClass = baseClasses.back();
        baseClasses.pop_back();
        for (auto cls : baseClass->GetDerivedClasses())
            {
            if (cls->GetCustomAttribute(*componentSpecificationCA).IsValid())
                componentDefs.push_back(DgnClassId(cls->GetId()));
            else
                baseClasses.push_back(cls);
            }
        }
    }
#else
void ComponentDef::QueryComponentDefs(bvector<DgnClassId>& componentDefs, DgnDbR db, ECN::ECClassCR baseClassIn)
    {
    auto componentSpecificationCA = db.Schemas().GetECClass(DGN_ECSCHEMA_NAME, "ComponentSpecification");
    if (nullptr == componentSpecificationCA)
        return;

    Statement stmt(db, "SELECT ClassId FROM ec_ClassHasBaseClasses WHERE BaseClassId=?");

    bvector<DgnClassId> baseClassIds;
    baseClassIds.push_back(DgnClassId(baseClassIn.GetId()));

    while (!baseClassIds.empty())
        {
        DgnClassId baseClassId = baseClassIds.back();
        baseClassIds.pop_back();

        stmt.Reset();
        stmt.ClearBindings();
        stmt.BindId(1, baseClassId);

        while (BE_SQLITE_ROW == stmt.Step())
            {
            DgnClassId derivedClassId = stmt.GetValueId<DgnClassId>(0);
            ECN::ECClassCP derivedClass = db.Schemas().GetECClass(derivedClassId);
            if (nullptr == derivedClass)
                continue;
            if (derivedClass->GetCustomAttribute(*componentSpecificationCA).IsValid())
                componentDefs.push_back(derivedClassId);
            else
                baseClassIds.push_back(derivedClassId);
            }
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentDefPtr ComponentDef::FromECClassId(DgnDbStatus* statusOut, DgnDbR db, DgnClassId componentDefClassId)
    {
    ECN::ECClassCP cls = db.Schemas().GetECClass(componentDefClassId);
    if (nullptr == cls)
        return nullptr;
    return FromECClass(statusOut, db, *cls);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentDefPtr ComponentDef::FromInstance(DgnDbStatus* statusOut, DgnElementCR instance)
    {
    if (nullptr == instance.GetElementClass())
        {
        if (nullptr != statusOut)
            *statusOut = DgnDbStatus::BadArg;
        return nullptr;
        }
    return FromECClass(statusOut, instance.GetDgnDb(), *instance.GetElementClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentDefPtr ComponentDef::FromComponentModel(DgnDbStatus* statusOut, ComponentModelCR cm)
    {
    ECN::ECClassCP ecclass = getECClassByFullName(cm.GetDgnDb(), cm.GetComponentECClassFullName());
    if (nullptr == ecclass)
        return nullptr;

    return FromECClass(nullptr, cm.GetDgnDb(), *ecclass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentDef::DeleteComponentDef(DgnDbR db, Utf8StringCR fullEcSqlClassName)
    {
    DgnDbStatus status;

    ECN::ECClassP ecclass;
    if (true)
        {
        ComponentDefPtr cdef = FromECSqlName(&status, db, fullEcSqlClassName);
        if (!cdef.IsValid())
            return status;

        if (!cdef->UsesTemporaryModel() && db.Models().Get<ComponentModel>(db.Models().QueryModelId(DgnModel::CreateModelCode(cdef->GetComponentName()))).IsValid())
            cdef->GetModel().Delete();

        ECSqlStatement stmt;
        stmt.Prepare(db, Utf8PrintfString("DELETE FROM %s", cdef->GetClassECSqlName().c_str()).c_str());
        stmt.Step();

        ecclass = const_cast<ECN::ECClassP>(&cdef->m_class);
        }

    // TRICKY: Must destruct cdef before we delete its ECClass, as ~ComponentDef uses its ECClass.

    ECN::ECSchemaR ecschema = const_cast<ECN::ECSchemaR>(ecclass->GetSchema());
    ecschema.DeleteClass(*ecclass);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentDefPtr ComponentDef::FromECClass(DgnDbStatus* statusOut, DgnDbR db, ECN::ECClassCR componentDefClass)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(status, statusOut);

    auto ca = getComponentSpecCA(db, componentDefClass);
    if (!ca.IsValid())
        {
        status = DgnDbStatus::WrongClass;
        return nullptr;
        }

    ComponentDef* cdef = new ComponentDef(db, componentDefClass);
    cdef->m_ca = ca;

    if (!cdef->QueryCategoryId().IsValid())
        {
        delete cdef;
        status = DgnDbStatus::InvalidCategory;
        return nullptr;
        }

    Utf8String modelName = cdef->GetComponentName();
    if (!modelName.empty() && !db.Models().QueryModelId(DgnModel::CreateModelCode(modelName)).IsValid())
        {
        delete cdef;
        status = DgnDbStatus::BadModel;
        return nullptr;
        }

    return cdef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentDefPtr ComponentDef::FromECSqlName(DgnDbStatus* statusOut, DgnDbR db, Utf8StringCR ecsqlClassName)
    {
    auto cdefclass = getECClassByFullName(db, ecsqlClassName);
    if (nullptr == cdefclass)
        {
        if (nullptr != statusOut)
            *statusOut = DgnDbStatus::NotFound;
        return nullptr;
        }
    return FromECClass(statusOut, db, *cdefclass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentDef::~ComponentDef()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentDef::GetCaValueString(ECN::IECInstanceCR ca, Utf8CP propName)
    {
    ECN::ECValue v;
    if (ECN::ECObjectsStatus::Success != ca.GetValue(v, propName) || v.IsNull())
        return "";
    return v.ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentDef::GetElementGeneratorName() const {return GetCaValueString(*m_ca, CDEF_CA_ELEMENT_GENERATOR);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentDef::GetCategoryName() const {return GetCaValueString(*m_ca, CDEF_CA_CATEGORY);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentDef::GetCodeAuthorityName() const {return GetCaValueString(*m_ca, CDEF_CA_CODE_AUTHORITY);}

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
bool ComponentDef::UsesTemporaryModel() const
    {
    return GetComponentName().empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr ComponentDef::MakeVariationSpec()
    {
    ECN::IECInstancePtr instance = m_class.GetDefaultStandaloneEnabler()->CreateInstance();

    //  Get the specification of the component's script-only parameters
    auto ca = getComponentSpecCA(m_db, GetECClass());
    ECN::ECValue adhocsJsonStr;
    ca->GetValue(adhocsJsonStr, "ScriptOnlyParameters");
    Json::Value adhocsJson;
    Json::Reader::Parse(adhocsJsonStr.ToString(), adhocsJson);
    TsComponentParameterSet adhocParams(adhocsJson);
    //  Add the script-only parameters to the ad hoc properties of the instance
    adhocParams.ToECProperties(*instance);

#ifdef DEBUG_COMPONENT_MODEL_TEST
    DumpScriptOnlyParameters(*instance, "MakeVariationSpec");
#endif

    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentDef::GetComponentName() const {return GetCaValueString(*m_ca, CDEF_CA_MODEL);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComponentDef::GetInputsForSelect() const
    {
    Utf8String inputs;

    for (auto const& input : GetInputs())
        {
        if (!inputs.empty())
            inputs.append(",");

        inputs.append("[");
        inputs.append(input);
        inputs.append("]");
        }

    return inputs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> ComponentDef::GetInputs() const
    {
    Utf8String inputList = GetCaValueString(*m_ca, CDEF_CA_INPUTS);

    bvector<Utf8String> inputs;

    size_t offset = 0;
    Utf8String m;
    while ((offset = inputList.GetNextToken (m, ",", offset)) != Utf8String::npos)
        inputs.push_back(m);

    if (inputs.empty())
        {
        // There are no declared inputs. Assume that all properties are inputs ...
        //  ... excluding the properties of dgn.Element, SpatialElement, and PhysicalElement, as they are very unlikely to be
        //      parametric model inputs, and excluding all non-primitive properties.
        ECN::ECClassCP physEle = GetDgnDb().Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement);
        bset<Utf8String> physEleProps;
        for (auto prop : physEle->GetProperties())
            physEleProps.insert(prop->GetName());

        for (auto ecprop : m_class.GetProperties())
            {
            if (!ecprop->GetIsPrimitive())
                continue;
            if (physEleProps.find(ecprop->GetName()) != physEleProps.end())
                continue;

            inputs.push_back(ecprop->GetName());
            }
        }

    return inputs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentModelR ComponentDef::GetModel() 
    {
    if (m_model.IsValid())
        return *m_model;

    DgnModelId modelId;
        
    Utf8String modelName = GetComponentName();
    if (!modelName.empty())
        {
        DgnCode modelCode = DgnModel::CreateModelCode(modelName);
        m_model = m_db.Models().Get<ComponentModel>(m_db.Models().QueryModelId(modelCode));
        if (m_model.IsValid())
            return *m_model;
        }

    //  model name not specified, or model does not exist. This component will use a sandbox model.

    Utf8String sandboxModelName = GetClassECSqlName();
    DgnDbTable::ReplaceInvalidCharacters(sandboxModelName, DgnModels::GetIllegalCharacters(), '_');

    m_model = m_db.Models().Get<ComponentModel>(m_db.Models().QueryModelId(DgnModel::CreateModelCode(sandboxModelName)));
    if (m_model.IsValid())
        {
        //  Sandbox model already exists. Clean it out and re-use it.
        CachedECSqlStatementPtr delStmt = m_db.GetPreparedECSqlStatement("DELETE FROM " DGN_SCHEMA(DGN_CLASSNAME_Element) " WHERE ModelId=?");
        delStmt->BindId(1, m_model->GetModelId());
        delStmt->Step();
        return *m_model;
        }

    //  Create a sandbox model.
    m_model = ComponentModel::Create(m_db, GetClassECSqlName().c_str());
    m_model->Insert();

    return *m_model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentModelPtr ComponentModel::Create(DgnDbR db, Utf8StringCR componentDefClassFullName)
    {
    if (componentDefClassFullName.find('.') == Utf8String::npos)
        {
        BeAssert(false && "requires full schema.class ECSql class name");
        return nullptr;
        }
    Utf8String modelName(componentDefClassFullName);
    DgnDbTable::ReplaceInvalidCharacters(modelName, DgnModels::GetIllegalCharacters(), '_');
    modelName = db.Models().GetUniqueModelName(modelName.c_str());
    DgnCode modelCode = DgnModel::CreateModelCode(modelName);
    return new ComponentModel(db, modelCode, componentDefClassFullName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentDef::GenerateElements(DgnModelR destModel, ECN::IECInstanceCR variationSpecIn)
    {
    GetModel();

    ECN::IECInstancePtr instanceTemplate = MakeVariationSpec();
    if (ECN::ECObjectsStatus::Success != instanceTemplate->CopyValues(variationSpecIn))
        {
        BeAssert(false);
        return DgnDbStatus::BadRequest;
        }

#ifdef DEBUG_COMPONENT_MODEL_TEST
    DumpScriptOnlyParameters(*instanceTemplate, "GenerateElements");
#endif

    Utf8String scriptName = GetElementGeneratorName();
    if (scriptName.empty())
        {
        auto* gg = dynamic_cast<ElementGenerator*>(m_model.get());
        if (nullptr == gg)
            return DgnDbStatus::Success;    // assume the model contains static geometry

        return gg->_GenerateElements(destModel, *m_model, *instanceTemplate, *this);
        }
        
    StatusInt retval;
    if (DgnDbStatus::Success != DgnScript::ExecuteComponentGenerateElements(retval, *m_model, destModel, *instanceTemplate, *this, scriptName.c_str()) || (0 != retval))
        return DgnDbStatus::BadRequest;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentDef::MakeInstance0(DgnDbStatus* statusOut, DgnModelR destModel, ECN::IECInstanceCR parameters, DgnCode const& code)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(status, statusOut);
    if (DgnDbStatus::Success != (status = GenerateElements(destModel, parameters)))
        return nullptr;

    HarvestedSolutionInserter inserter(destModel, GetModel());
    ComponentGeometryHarvester harvester(*this);
    return harvester.MakeInstance(status, code, inserter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentDef::QueryVariationByName(Utf8StringCR variationName)
    {
    return m_db.Elements().GetElement(m_db.Elements().QueryElementIdByCode(CreateVariationCode(variationName)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ComponentDef::DeleteVariation(DgnElementCR variation)
    {
    // *** WIP_COMPONENT: detect if variation is in use
    return variation.Delete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentDef::MakeVariation(DgnDbStatus* statusOut, DgnModelR destModel, ECN::IECInstanceCR variationParms, Utf8StringCR variationName)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(status, statusOut);
    
    ComponentDefPtr componentCDef = FromECClass(nullptr, destModel.GetDgnDb(), variationParms.GetClass());
    if (!componentCDef.IsValid())
        {
        status = DgnDbStatus::WrongClass;
        return nullptr;
        }

    if (variationName.empty())
        {
        BeAssert(false && "a variation must have a name");
        status = DgnDbStatus::InvalidName;
        return nullptr;
        }

    DgnCode vcode = componentCDef->CreateVariationCode(variationName);

    return componentCDef->MakeInstance0(statusOut, destModel, variationParms, vcode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentDef::MakeUniqueInstance(DgnDbStatus* statusOut, DgnModelR destModel, ECN::IECInstanceCR variationParms, DgnCode const& code)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(status, statusOut);

    ComponentDefPtr componentCDef = FromECClass(nullptr, destModel.GetDgnDb(), variationParms.GetClass());
    if (!componentCDef.IsValid())
        {
        status = DgnDbStatus::WrongClass;
        return nullptr;
        }
    return componentCDef->MakeInstance0(statusOut, destModel, variationParms, code);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ComponentDef::MakeInstanceOfVariation(DgnDbStatus* statusOut, DgnModelR destModel, DgnElementCR variation, ECN::IECInstanceCP instanceParms, DgnCode const& code)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(status, statusOut);
    
    DgnDbR db = variation.GetDgnDb();
    if (&db != &destModel.GetDgnDb())
        {
        status = DgnDbStatus::WrongDgnDb;
        BeAssert(false && "Catalog and target models must be in the same Db");
        return nullptr;
        }

    ComponentDefPtr cdef = FromECClass(nullptr, db, *variation.GetElementClass());
    if (!cdef.IsValid())
        {
        status = DgnDbStatus::WrongClass;
        return nullptr;
        }

    if (nullptr != instanceParms)
        {
        if (&instanceParms->GetClass() != variation.GetElementClass())
            {
            status = DgnDbStatus::WrongClass;
            return nullptr;
            }
        ECN::IECInstancePtr allParms = GetParameters(variation);
        if (!cdef->HaveEqualParameters(*instanceParms, *allParms, true))
            {
            //  If the caller has specified parameters and they don't match the variation's parameters, then we must make a unique instance.
            cdef->CopyInstanceParameters(*allParms, *instanceParms);
            return cdef->MakeUniqueInstance(statusOut, destModel, *allParms, code);
            }
        }

    DgnCode icode = code;
    if (!code.IsValid())
        {
        //  Generate the item code. This will be a null code, unless there's a specified authority for the componentmodel.
        DgnAuthorityCPtr authority = cdef->GetCodeAuthority();
        if (authority.IsValid())
            icode = DgnCode::CreateEmpty();  // *** WIP_COMPONENT_MODEL -- how do I ask an Authority to issue a code?
        }

    //  Creating the item is just a matter of copying the catalog item (and its children)
    DgnCloneContext ctx;
    ElementCopier copier(ctx);
    copier.SetCopyChildren(true);
    DgnElementCPtr inst = copier.MakeCopy(&status, destModel, variation, icode);
    if (!inst.IsValid())
        return nullptr;

    return inst;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr ComponentDef::GetParameters(DgnElementCR el)
    {
    ComponentDefPtr cdef = FromECClass(nullptr, el.GetDgnDb(), *el.GetElementClass());
    if (!cdef.IsValid())
        return nullptr;

    Utf8PrintfString sql("SELECT %s ScriptOnlyParameters FROM %s WHERE ECInstanceId=?", cdef->GetInputsForSelect().c_str(), GetClassECSqlName(*el.GetElementClass()).c_str());
    auto ecsql = el.GetDgnDb().GetPreparedECSqlStatement(sql.c_str());
    ecsql->BindId(1, el.GetElementId());
    ECInstanceECSqlSelectAdapter selector(*ecsql);
    return selector.GetInstance();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentDef::ParameterVariesPer ComponentDef::GetVariationScope(ECN::ECPropertyCR prop)
    {
    auto ca = GetPropSpecCA(prop);
    if (!ca.IsValid())
        return ParameterVariesPer::Instance;

    return ComponentDef::GetCaValueString(*ca, "ParameterVariesPer").EqualsI("Variation")? ParameterVariesPer::Variation: ParameterVariesPer::Instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ComponentDef::HaveEqualParameters(ECN::IECInstanceCR lhs, ECN::IECInstanceCR rhs, bool compareOnlyInstanceParameters)
    {
    if (&lhs.GetClass() != &rhs.GetClass())
        return false;
    for (auto const& paramName : GetInputs())
        {
        if (compareOnlyInstanceParameters && (ParameterVariesPer::Instance != GetVariationScope(*lhs.GetClass().GetPropertyP(paramName.c_str()))))
            continue;

        ECValue l;
        lhs.GetValue(l, paramName.c_str());
        if (l.IsNull())
            continue;
            
        ECValue r;
        rhs.GetValue(r, paramName.c_str());

        if (!l.Equals(r))
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentDef::CopyInstanceParameters(ECN::IECInstanceR target, ECN::IECInstanceCR source)
    {
    if (&source.GetClass() != &target.GetClass())
        {
        BeAssert(false);
        return;
        }
    for (auto const& paramName : GetInputs())
        {
        if (ParameterVariesPer::Instance != GetVariationScope(*source.GetClass().GetPropertyP(paramName.c_str())))
            continue;

        ECValue v;
        if (ECN::ECObjectsStatus::Success != source.GetValue(v, paramName.c_str()) || v.IsNull())
            continue;

        if (ECN::ECObjectsStatus::Success != target.SetValue(paramName.c_str(), v))
            {
            BeAssert(false);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
#ifdef COMMENT_OUT_UNUSED
static ECN::ECSchemaPtr copyECSchema(ECN::ECSchemaCR schemaIn)
    {
    ECN::ECSchemaPtr cc;
    if (ECN::ECObjectsStatus::Success != schemaIn.CopySchema(cc))
        return nullptr;
    if (ECN::ECObjectsStatus::Success != cc->SetName(schemaIn.GetName()))
        return nullptr;
    if (ECN::ECObjectsStatus::Success != cc->SetNamespacePrefix(schemaIn.GetNamespacePrefix()))
        return nullptr;
    return cc;
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static ECN::ECSchemaCP importECSchema(ECN::ECObjectsStatus& ecstatus, DgnDbR db, ECN::ECSchemaCR schemaIn, bool updateExistingSchemas)
    {
    ECN::ECSchemaCP existing = db.Schemas().GetECSchema(schemaIn.GetName().c_str());
    if (nullptr != existing)
        {
        if (!updateExistingSchemas)
            return existing;
        }
    else
        {
        updateExistingSchemas = false;
        }

    ECN::ECSchemaReadContextPtr contextPtr = ECN::ECSchemaReadContext::CreateContext();

#ifdef NEEDS_WORK_ECDB // *** If schemaIn is a real schema (from another file), then I will get an assertion failure in ECDbMapStorage::InsertOrReplace
                        // *** That happens, because the base class of new class points to the new class twice.
    ECN::ECSchemaPtr imported = copyECSchema(schemaIn);

    ECSUCCESS(contextPtr->AddSchema(*imported));

#else

    Utf8String ecschemaXml;
    schemaIn.WriteToXmlString(ecschemaXml);

    contextPtr->AddSchemaLocater(db.GetSchemaLocater());

    ECSchemaPtr imported;
    SchemaReadStatus readSchemaStatus = ECSchema::ReadFromXmlString(imported, ecschemaXml.c_str(), *contextPtr);
    if (SchemaReadStatus::Success != readSchemaStatus)
        return nullptr;

#endif

    if (BentleyStatus::SUCCESS != db.Schemas().ImportECSchemas(contextPtr->GetCache()))
        {
        ecstatus = ECObjectsStatus::Error;
        return nullptr;
        }

    if (nullptr == existing)    // Don't call CreateECClassViewsInDb twice!
        db.Schemas().CreateECClassViewsInDb();

    return imported.get();  // DgnDb holds a reference to the schema now.
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECN::ECSchemaCP ComponentDefCreator::ImportSchema(DgnDbR db, ECN::ECSchemaCR schemaIn, bool doUpdate)
    {
    ECN::ECObjectsStatus ecstatus;
    return importECSchema(ecstatus, db, schemaIn, doUpdate);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static ECN::ECClassCP getSameClassIn(DgnDbR db, ECN::ECClassCR cls)
    {
    return db.Schemas().GetECClass(cls.GetSchema().GetName().c_str(), cls.GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus ComponentDef::Export(DgnImportContext& context, ExportOptions const& options)
    {
    if (&GetDgnDb() != &context.GetSourceDb()) 
        {
        BeAssert(false && "You must call this method on the ComponentDef that is in the *source* db.");
        return DgnDbStatus::WrongDgnDb;
        }

    if (options.m_exportSchema)
        {
        ECN::ECObjectsStatus ecstatus = ECN::ECObjectsStatus::Success;
        importECSchema(ecstatus, context.GetDestinationDb(), GetECClass().GetSchema(), true);
        if (ECN::ECObjectsStatus::Success != ecstatus && ECN::ECObjectsStatus::DuplicateSchema != ecstatus)
            return DgnDbStatus::BadSchema;
        }

    if (options.m_embedScript && !GetElementGeneratorName().empty())
        {
        Utf8String jsProgramName (GetElementGeneratorName());
        size_t idot = jsProgramName.find('.');
        if (idot != Utf8String::npos)
            jsProgramName = jsProgramName.substr(0, idot);

        Utf8String sText;
        DgnScriptType sType;
        DateTime lmt;
        if (DgnDbStatus::Success == T_HOST.GetScriptAdmin()._FetchScript(sText, sType, lmt, context.GetSourceDb(), jsProgramName.c_str(), DgnScriptType::JavaScript))
            {
            DgnScriptLibrary slib(context.GetDestinationDb());
            slib.RegisterScript(jsProgramName.c_str(), sText.c_str(), sType, lmt, true);
            }
        }

    if (options.m_exportCategory && !DgnCategory::QueryCategoryId(GetCategoryName(), context.GetDestinationDb()).IsValid()) // *** WIP_COMPONENT - update existing category definition??
        {
        auto sourceCat = DgnCategory::QueryCategory(GetCategoryName(), GetDgnDb());
        if (sourceCat.IsValid())
            {
            ElementImporter importer(context);
            importer.ImportElement(nullptr, context.GetDestinationDb().GetDictionaryModel(), *sourceCat);
            }
        }

    if (!UsesTemporaryModel()) 
        {
        DgnModelPtr existingModel = context.GetDestinationDb().Models().GetModel(context.GetDestinationDb().Models().QueryModelId(DgnModel::CreateModelCode(GetComponentName())));
        if (existingModel.IsValid())
            existingModel->Delete();

        DgnDbStatus status;
        if (!DgnModel::Import(&status, GetModel(), context).IsValid())
            return status;
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus ComponentDef::ExportVariations(DgnModelR destVariationsModel, DgnModelId sourceVariationsModelId, DgnImportContext& context, bvector<DgnElementId> const& variationFilter)
    {
    //  Check that destination model is compatible with the import context
    if (&destVariationsModel.GetDgnDb() != &context.GetDestinationDb())
        return DgnDbStatus::WrongDgnDb;

    // Check that this component definition has already been imported into the destination Db
        {
        auto destClass = getSameClassIn(destVariationsModel.GetDgnDb(), m_class);
        if (nullptr == destClass)
            return DgnDbStatus::BadSchema;
        if (!FromECClass(nullptr, destVariationsModel.GetDgnDb(), *destClass).IsValid())
            return DgnDbStatus::WrongDgnDb;
        }

    ElementImporter importer(context);

    EC::ECSqlStatement selectInstancesOfComponent;
    selectInstancesOfComponent.Prepare(GetDgnDb(), Utf8PrintfString("SELECT ECInstanceId FROM %s WHERE(ModelId=?)", GetClassECSqlName().c_str()).c_str());
    selectInstancesOfComponent.BindId(1, sourceVariationsModelId);
    while (BE_SQLITE_ROW == selectInstancesOfComponent.Step())
        {
        DgnElementId sourceElementId = selectInstancesOfComponent.GetValueId<DgnElementId>(0);

        if (!variationFilter.empty() && variationFilter.end() != std::find(variationFilter.begin(), variationFilter.end(), sourceElementId))
            continue;

        DgnElementCPtr sourceVariation = context.GetSourceDb().Elements().GetElement(sourceElementId);
        if (!sourceVariation.IsValid())
            continue;

        if (!IsComponentVariationCode(sourceVariation->GetCode()))
            continue;

        DgnDbStatus status;
        DgnElementCPtr destCatalogItem = importer.ImportElement(&status, destVariationsModel, *sourceVariation);
        if (!destCatalogItem.IsValid())
            return status;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentDef::QueryVariations(bvector<DgnElementId>& variations, DgnModelId variationsModelId)
    {
    EC::ECSqlStatement selectInstancesOfComponent;
    selectInstancesOfComponent.Prepare(GetDgnDb(), Utf8PrintfString("SELECT ECInstanceId FROM %s WHERE(ModelId=?)", GetClassECSqlName().c_str()).c_str());
    selectInstancesOfComponent.BindId(1, variationsModelId);
    while (BE_SQLITE_ROW == selectInstancesOfComponent.Step())
        {
        DgnElementId instanceId = selectInstancesOfComponent.GetValueId<DgnElementId>(0);

        DgnElementCPtr variation = GetDgnDb().Elements().GetElement(instanceId);
        if (!variation.IsValid())
            continue;

        if (!IsComponentVariationCode(variation->GetCode()))
            continue;

        variations.push_back(instanceId);
        }
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
        DgnElementPtr newCatalogItem = HarvestModel(status, *sourceCatalogItem->GetModel()->ToPhysicalModelP(), sourceCatalogItem->GetPlacement(), sourceCatalogItem->GetCode());
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
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnClassId getComponentModelClassId(DgnDbR db)
    {
    return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "ComponentModel"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ComponentModel::ComponentModel(DgnDbR db, DgnCode code, Utf8StringCR defName) : GeometricModel3d(CreateParams(db, getComponentModelClassId(db), code))
    {
    m_componentECClass = defName;
    BeAssert(!m_componentECClass.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Sam.Wilson   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::_WriteJsonProperties(Json::Value& val) const 
    {
    T_Super::_WriteJsonProperties(val);
    if (val.isNull())
        val = Json::objectValue;

    Json::Value componentModelJson (Json::objectValue);
    componentModelJson["ComponentDefECClass"] = m_componentECClass;
    // add more ComponentModel-specific properties to componentModelJson ...
    
    val["ComponentModel"] = componentModelJson; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Sam.Wilson   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::_ReadJsonProperties(Json::Value const& val) 
    {
    T_Super::_ReadJsonProperties(val);

    BeAssert(val.isMember("ComponentModel"));
    Json::Value componentModelJson = val["ComponentModel"];

    m_componentECClass = componentModelJson["ComponentDefECClass"].asCString();
    // read more ComponentModel-specific properties from componentModelJson ...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Sam.Wilson   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentModel::_InitFrom(DgnModelCR other)
    {
    T_Super::_InitFrom(other);
    ComponentModelCP otherCM = dynamic_cast<ComponentModelCP> (&other);
    if (nullptr != otherCM)
        m_componentECClass = otherCM->m_componentECClass;
    }

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
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr ComponentDefCreator::CreatePropSpecCA()
    {
    ECN::ECClassCP caClass = m_db.Schemas().GetECClass(DGN_ECSCHEMA_NAME, "ComponentParameterSpecification");
    if (nullptr == caClass)
        return nullptr;
    return caClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr ComponentDef::GetPropSpecCA(ECN::ECPropertyCR prop)
    {
    ECN::ECClassCP caClass = m_db.Schemas().GetECClass(DGN_ECSCHEMA_NAME, "ComponentParameterSpecification");
    if (nullptr == caClass)
        return nullptr;
    return prop.GetCustomAttribute(*caClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static ECN::ECSchemaCP getStdCaSchema(DgnDbR db)
    {
    return db.Schemas().GetECSchema("Bentley_Standard_CustomAttributes");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static ECN::IECInstancePtr createAdHocPropSpecCA(DgnDbR db)
    {
    ECN::ECSchemaCP cashema = getStdCaSchema(db);
    if (nullptr == cashema)
        return nullptr;

    ECN::ECClassCP caClass = cashema->GetClassCP("AdhocPropertySpecification");
    if (nullptr == caClass)
        return nullptr;
    return caClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr ComponentDefCreator::CreateSpecCA()
    {
    ECN::ECClassCP caClass = m_db.Schemas().GetECClass(DGN_ECSCHEMA_NAME, "ComponentSpecification");
    if (nullptr == caClass)
        return nullptr;
    return caClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr ComponentDefCreator::AddSpecCA(ECN::ECClassR ecclass)
    {
    ECN::ECObjectsStatus ecstatus;

    ECN::IECInstancePtr componentSpec = CreateSpecCA();

    ECSUCCESS(componentSpec->SetValue(CDEF_CA_CATEGORY, ECN::ECValue(m_categoryName.c_str())));

    if (!m_scriptName.empty())
        {
        ECSUCCESS(componentSpec->SetValue(CDEF_CA_ELEMENT_GENERATOR, ECN::ECValue(m_scriptName.c_str())));
        }
    if (!m_codeAuthorityName.empty())
        {
        ECSUCCESS(componentSpec->SetValue(CDEF_CA_CODE_AUTHORITY, ECN::ECValue(m_codeAuthorityName.c_str())));
        }
    if (!m_modelName.empty())
        {
        ECSUCCESS(componentSpec->SetValue(CDEF_CA_MODEL, ECN::ECValue(m_modelName.c_str())));
        }
    if (!m_inputs.empty())
        {
        ECSUCCESS(componentSpec->SetValue(CDEF_CA_INPUTS, ECN::ECValue(m_inputs.c_str())));
        }
    if (!m_adhocParams.empty())
        {
        Json::Value adhocsJson = m_adhocParams.ToJson();
        Utf8String adhocsJsonStr = Json::FastWriter::ToString(adhocsJson);
        ECSUCCESS(componentSpec->SetValue(CDEF_CA_ScriptOnlyParameters, ECN::ECValue(adhocsJsonStr.c_str())));
        }

    ECSUCCESS(ecclass.SetCustomAttribute(*componentSpec));

    #ifndef NDEBUG
        {
        ECN::ECClassCP caClass = m_db.Schemas().GetECClass(DGN_ECSCHEMA_NAME, "ComponentSpecification");
        auto ca = ecclass.GetCustomAttribute(*caClass);
        ECN::ECValue v;
        BeAssert(ECN::ECObjectsStatus::Success == ca->GetValue(v, CDEF_CA_CATEGORY) && v.ToString() == m_categoryName);
        if (!m_scriptName.empty())
            {
            BeAssert(ECN::ECObjectsStatus::Success == ca->GetValue(v, CDEF_CA_ELEMENT_GENERATOR) && v.ToString() == m_scriptName);
            }
        if (!m_codeAuthorityName.empty())
            {
            BeAssert(ECN::ECObjectsStatus::Success == ca->GetValue(v, CDEF_CA_CODE_AUTHORITY) && v.ToString() == m_codeAuthorityName);
            }
        if (!m_modelName.empty())
            {
            BeAssert(ECN::ECObjectsStatus::Success == ca->GetValue(v, CDEF_CA_MODEL) && v.ToString() == m_modelName);
            }
        }
    #endif

    return componentSpec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ComponentDefCreator::AddInput(Utf8StringCR inp)
    {
    if (!m_inputs.empty())
        m_inputs.append(",");
    m_inputs.append(inp);
    }

/*
    ECN::IECInstancePtr adHocPropsHolder = adHocPropsHolderClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECN::AdhocPropertyEdit adHocPropsEditor(*adHocPropsHolder, "ScriptOnlyParameters");

    for (auto const& entry: m_params)
        {
        Utf8StringCR paramName = entry.first;
        TsComponentParameter const& param = entry.second;
        if (param.m_isForScriptOnly)
            {
            adHocPropsEditor.Add(paramName.c_str(), param.m_value, "");
            // *** TBD: Set some other 
            uint32_t idx;
            adHocPropsEditor.GetPropertyIndex(idx, paramName.c_str());
            ECN::ECValue v;
            adHocPropsEditor.GetValue(v, idx);
            BeAssert(v.IsString());
            }

*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP ComponentDefCreator::GenerateECClass()
    {
    ECN::ECObjectsStatus ecstatus;
    
    m_schema.AddReferencedSchema(const_cast<ECN::ECSchemaR>(m_baseClass.GetSchema()));
    m_schema.AddReferencedSchema(const_cast<ECN::ECSchemaR>(*getStdCaSchema(m_db)));

    if (true)
        {
        ECN::ECClassCP existing_ecclass = m_schema.GetClassCP(m_name.c_str());
        if (nullptr != existing_ecclass)
#ifdef WIP_COMPONENT // *** Problems with deleting ECClass
            ComponentDef::DeleteComponentDef(m_db, ComponentDef::GetClassECSqlName(*existing_ecclass));
#else
            return existing_ecclass;
#endif
        }
          
    ECN::ECEntityClassP ecclass;      
    ECSUCCESS(m_schema.CreateEntityClass(ecclass, m_name));

    ECSUCCESS(ecclass->AddBaseClass(m_baseClass));

    auto adhocspecCa = createAdHocPropSpecCA(m_db);
    adhocspecCa->SetValue("AdhocPropertyContainer", ECN::ECValue("ComponentSpecificationAdhocHolder"));
    ECSUCCESS(ecclass->SetCustomAttribute(*adhocspecCa));

    ECN::ECClassCP adHocPropsHolderClass0 = m_db.Schemas().GetECClass(DGN_ECSCHEMA_NAME, "ComponentSpecificationAdhocHolder");
    ECN::ECStructClassCP adHocPropsHolderClass = dynamic_cast<ECN::ECStructClassCP>(adHocPropsHolderClass0);

    ECN::StructArrayECPropertyP adhocsArrayProp;
    ECSUCCESS(ecclass->CreateStructArrayProperty(adhocsArrayProp, "ScriptOnlyParameters", adHocPropsHolderClass));

    for (auto const& entry: m_params)
        {
        if (entry.second.m_isForScriptOnly)
            m_adhocParams[entry.first] = entry.second;
        else
            m_firstClassParams[entry.first] = entry.second;
        }

    for (auto const& entry: m_firstClassParams)
        {
        Utf8StringCR paramName = entry.first;
        TsComponentParameter const& param = entry.second;
        ECN::PrimitiveECPropertyP ecprop;
        ECSUCCESS(ecclass->CreatePrimitiveProperty(ecprop, paramName));
        ecprop->SetType(param.m_value.GetPrimitiveType());

        if (param.m_variesPer != ComponentDef::ParameterVariesPer::Instance)
            {
            auto ca = CreatePropSpecCA();
            ca->SetValue("ParameterVariesPer", ECN::ECValue("Variation"));
            ecprop->SetCustomAttribute(*ca);
            }
        }

    AddSpecCA(*ecclass);

    return ecclass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECN::ECSchemaPtr ComponentDefCreator::GenerateSchema(DgnDbR db, Utf8StringCR schemaNameIn)
    {
    Utf8String schemaName(schemaNameIn);

    if (nullptr != db.Schemas().GetECSchema(schemaName.c_str()))
        return nullptr;

    // Ask componentDb to generate a schema
    ECN::ECSchemaPtr schema;
    if (ECN::ECObjectsStatus::Success != ECN::ECSchema::CreateSchema(schema, schemaName, 0, 0))
        return nullptr;

    schema->SetNamespacePrefix(schemaName);
    
    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TsComponentParameterSet::TsComponentParameterSet(Json::Value const& json)
    {
    for (auto const& pname: json.getMemberNames())
        (*this)[pname] = TsComponentParameter(json[pname]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value TsComponentParameterSet::ToJson() const
    {
    Json::Value parametersJson (Json::objectValue);
    for (auto const& entry : *this)
        parametersJson[entry.first] = entry.second.ToJson();
    return parametersJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TsComponentParameterSet::TsComponentParameterSet(ComponentDef& cdef, ECN::IECInstanceCR inst)
    {
    AdhocPropertyQuery adhocs(inst, "ScriptOnlyParameters");
    for (uint32_t i = 0; i < adhocs.GetCount(); ++i)
        {
        Utf8String name;
        ECN::ECValue value;
        adhocs.GetName(name, i);
        adhocs.GetValue(value, i);
        
        // *** WIP_COMPONENT - store VariesPer as part of adhoc property value
        TsComponentParameter tsparam (ComponentDef::ParameterVariesPer::Instance, value);
        (*this)[name] = tsparam;
        }

    for (auto const& paramName : cdef.GetInputs())
        {
        ECN::ECPropertyP prop = cdef.GetECClass().GetPropertyP(paramName.c_str());

        ECValue v;
        inst.GetValue(v, paramName.c_str());
        TsComponentParameter tsparam (cdef.GetVariationScope(*prop), v);

        (*this)[paramName] = tsparam;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void TsComponentParameterSet::ToECProperties(ECN::IECInstanceR props) const
    {
    ECN::AdhocPropertyEdit adHocPropsEditor(props, "ScriptOnlyParameters");
    for (auto const& entry : *this)
        {
        Utf8StringCR paramName = entry.first;
        TsComponentParameter const& param = entry.second;
        if (!param.m_isForScriptOnly)
            props.SetValue(paramName.c_str(), param.m_value);
        else
            {
            if (ECN::ECObjectsStatus::Success != adHocPropsEditor.Add(paramName.c_str(), param.m_value, ""))
				{
				BeAssert(false);
				}

            #ifndef NDEBUG
            uint32_t idx;
            adHocPropsEditor.GetPropertyIndex(idx, paramName.c_str());
            ECN::ECValue v;
            adHocPropsEditor.GetValue(v, idx);
            BeAssert(v.Equals(param.m_value));
            #endif
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TsComponentParameter::SetValue(ECN::ECValueCR valueIn)
    {
    if (!m_value.IsPrimitive())
        return DgnDbStatus::BadArg;
        
    ECN::ECValue value = valueIn;
    if (!value.ConvertToPrimitiveType(m_value.GetPrimitiveType()))
        return DgnDbStatus::BadArg;

    m_value = value;
    return DgnDbStatus::Success;
    }

#define PARARMETER_VARIES_PER_INSTANCE 0
#define PARARMETER_VARIES_PER_VARIATION 1

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value TsComponentParameter::ToJson() const
    {
    // *** Keep this consistent with ComponentParametersPane.ts! ***
    Json::Value v;
    v["VariesPer"] = (ComponentDef::ParameterVariesPer::Variation == m_variesPer)? PARARMETER_VARIES_PER_VARIATION: PARARMETER_VARIES_PER_INSTANCE;
    v["IsForScriptOnly"] = m_isForScriptOnly;
    ECUtils::StoreECValueAsJson(v["Value"], m_value);
    return v;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TsComponentParameter::TsComponentParameter(Json::Value const& json)
    {
    // *** Keep this consistent with ComponentParametersPane.ts! ***
    auto s = json["VariesPer"].asInt();
    m_variesPer = (PARARMETER_VARIES_PER_VARIATION == s)? ComponentDef::ParameterVariesPer::Variation: ComponentDef::ParameterVariesPer::Instance;
    m_isForScriptOnly = json["IsForScriptOnly"].asBool();
    ECUtils::LoadECValueFromJson(m_value, json["Value"]);
    }
