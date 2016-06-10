/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScript/DgnJsApi/DgnJsApi.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeJavaScript/BeJavaScript.h>
#include <DgnPlatform/DgnScript.h>
#include <Bentley/BeFileListIterator.h>
#include <ECObjects/ECObjectsAPI.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnJsApi.h>
#include <DgnPlatform/DgnJsApiProjection.h>
#include <Geom/SolidPrimitive.h>
#include <DgnPlatform/GeomJsTypes/JsCurvePrimitive.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

size_t JsDgnElement::s_count;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnElementPtr createElementByClass(DgnModelR model, Utf8CP ecSqlClassName)
    {
    if (!ecSqlClassName || !*ecSqlClassName)
        return nullptr;

    Utf8CP dot = strchr(ecSqlClassName, '.');
    if (nullptr == dot)
        {
        T_HOST.GetScriptAdmin().HandleScriptError(DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::Other, "malformed ECSql ecclass name", ecSqlClassName);
        return nullptr;
        }
    Utf8String ecschema(ecSqlClassName, dot);
    Utf8String ecclass(dot+1);
    DgnDbR db = model.GetDgnDb();
    DgnClassId pclassId = DgnClassId(db.Schemas().GetECClassId(ecschema.c_str(), ecclass.c_str()));
    if (!pclassId.IsValid())
        {
        T_HOST.GetScriptAdmin().HandleScriptError(DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::Other, "ECClass not found", ecSqlClassName);
        return nullptr;
        }
    dgn_ElementHandler::Element* handler = dgn_ElementHandler::Element::FindHandler(model.GetDgnDb(), pclassId);
    if (nullptr == handler)
        {
        T_HOST.GetScriptAdmin().HandleScriptError(DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::Other, "handler not found", ecSqlClassName);
        return nullptr;
        }
    DgnElementPtr el = handler->Create(DgnElement::CreateParams(db, model.GetModelId(), pclassId));
    if (!el.IsValid())
        {
        Utf8PrintfString details ("class: %s", ecSqlClassName);
        T_HOST.GetScriptAdmin().HandleScriptError(DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::Other, "dgn_ElementHandler::Geometric3d::GetHandler().Create failed", details.c_str());
        return nullptr;
        }
    return el;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsGeometryBuilder::JsGeometryBuilder(JsGeometrySourceP jsgs, JsDPoint3dP o, JsYawPitchRollAnglesP a)
    {
    DGNJSAPI_VALIDATE_ARGS_VOID(DGNJSAPI_IS_VALID_JSELEMENT_PLACEHOLDER(jsgs) && jsgs->m_el->ToGeometrySource() && o && a);
    auto el = jsgs->m_el;

    GeometrySource3dCP source3d = el->ToGeometrySource3d();
    if (nullptr != source3d)
        m_builder = GeometryBuilder::Create(*source3d, o->Get (), a->GetYawPitchRollAngles ());
    else
        {
        GeometrySource2dCP source2d = el->ToGeometrySource2d();
        if (nullptr != source2d)
            m_builder = GeometryBuilder::Create(*source2d, DPoint2d::From(o->GetX(), o->GetY()), AngleInDegrees::FromDegrees(a->GetYawDegrees ()));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Earlin.Lutz                      03/16
//---------------------------------------------------------------------------------------
JsGeometryBuilder::JsGeometryBuilder(JsGeometrySourceP jsgs, DPoint3dCR o, YawPitchRollAnglesCR a)
    {
    DGNJSAPI_VALIDATE_ARGS_VOID(DGNJSAPI_IS_VALID_JSELEMENT_PLACEHOLDER(jsgs) && jsgs->m_el->ToGeometrySource());
    auto el = jsgs->m_el;

    GeometrySource3dCP source3d = el->ToGeometrySource3d();
    if (nullptr != source3d)
        m_builder = GeometryBuilder::Create(*source3d, o, a);
    else
        {
        GeometrySource2dCP source2d = el->ToGeometrySource2d();
        if (nullptr != source2d)
            m_builder = GeometryBuilder::Create(*source2d, DPoint2d::From(o.x, o.y), AngleInDegrees::FromDegrees(a.GetYaw ().Degrees ()));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
void JsGeometryBuilder::AppendCopyOfGeometry(JsGeometryBuilderP jsbuilder, JsPlacement3dP jsrelativePlacement)
    {
    DGNJSAPI_VALIDATE_ARGS_VOID(IsValid());
    
    GeometryBuilderR otherbuilder = *jsbuilder->m_builder;
    GeometryStream otherStream;
    otherbuilder.GetGeometryStream(otherStream);
    GeometryCollection otherGeomCollection(otherStream, m_builder->GetDgnDb());
    
    Transform t;
    if (nullptr != jsrelativePlacement)
        t = jsrelativePlacement->m_placement.GetTransform();
    else
        t.InitIdentity();

    for (auto otherItem: otherGeomCollection)
        {
        GeometryParams sourceParams (otherItem.GetGeometryParams());
        sourceParams.SetCategoryId(m_builder->GetGeometryParams().GetCategoryId());
        m_builder->Append(sourceParams);

        auto geomprim = otherItem.GetGeometryPtr();
        if (geomprim.IsValid())
            {
            GeometricPrimitivePtr cc = geomprim->Clone();
            cc->TransformInPlace(t);
            m_builder->Append(*cc);
            }
        else
            {
                /* *** TBD: embedded geompart instances
            DgnGeometryPartCPtr gp = otherItem.GetGeometryPartPtr();
            if (gp.IsValid())
                {
                Transform t = otherItem.GetGeometryToSource();
                }
                */
            BeAssert(false && "AppendCopyOfBuilder - geompart instances not supported");
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
void JsGeometryBuilder::AppendGeometryPart(JsDgnGeometryPartP part, JsPlacement3dP jsrelativePlacement)
    {
    DGNJSAPI_VALIDATE_ARGS_VOID(IsValid() && DGNJSAPI_IS_VALID_JSOBJ(part));
    Transform t;
    if (nullptr != jsrelativePlacement)
        t = jsrelativePlacement->m_placement.GetTransform();
    else
        t.InitIdentity();
    m_builder->Append(part->m_value->GetId(), t);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      02/16
//---------------------------------------------------------------------------------------
JsGeometryCollectionP JsGeometricElementBase::GetGeometry() const
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    return new JsGeometryCollection(*m_el->ToGeometrySource());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      02/16
//---------------------------------------------------------------------------------------
JsGeometryP JsGeometricPrimitive::GetGeometry() const 
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    switch (m_value->GetGeometryType())
        {
        case GeometricPrimitive::GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr curve = m_value->GetAsICurvePrimitive();
            return JsCurvePrimitive::StronglyTypedJsCurvePrimitive (curve, true);
            }
        
        case GeometricPrimitive::GeometryType::CurveVector:
            {
            CurveVectorPtr cv = m_value->GetAsCurveVector();
            return JsCurveVector::StronglyTypedJsCurveVector(cv);
            }
        
        case GeometricPrimitive::GeometryType::Polyface:
            return new JsPolyfaceMesh(m_value->GetAsPolyfaceHeader());

        case GeometricPrimitive::GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr solid = m_value->GetAsISolidPrimitive();
            return JsSolidPrimitive::StronglyTypedJsSolidPrimitive (solid);
            }
        }

    #ifdef WIP_DGNJSAPI // *** How to wrap BsplineSurface?
        case GeometricPrimitive::GeometryType::BsplineSurface:
            return new JsBsplineSurface(m_value->GetAsMSBsplineSurface());
    #endif

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      02/16
//---------------------------------------------------------------------------------------
JsTextStringP JsGeometricPrimitive::GetTextString()  const
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    auto ts = m_value->GetAsTextString();
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
int32_t JsDgnElement::Insert() 
    {
    DGNJSAPI_VALIDATE_ARGS_ERROR(IsValid());
    DgnDbStatus status;
    auto cptr = m_el->Insert(&status);
    if (!cptr.IsValid())
        return (int32_t)status;
    m_el = cptr->CopyForEdit();
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
int32_t JsDgnElement::Update() 
    {
    DGNJSAPI_VALIDATE_ARGS_ERROR(IsValid());
    auto cptr = m_el->Update();
    if (!cptr.IsValid())
        return -2;
    m_el = cptr->CopyForEdit();
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsECValueP JsDgnElement::GetProperty(Utf8StringCR name) 
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    ECN::ECValue v;
    if (m_el->_GetProperty(v, name.c_str()) != DgnDbStatus::Success || v.IsNull())
        return nullptr;
    return new JsECValue(v);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
int32_t JsDgnElement::SetProperty(Utf8StringCR name, JsECValueP v)
    {
    DGNJSAPI_VALIDATE_ARGS_ERROR(IsValid());
    return (int32_t) m_el->_SetProperty(name.c_str(), v->m_value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsAdHocJsonPropertyValueP JsDgnElement::GetUserProperty(Utf8StringCR name) const
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    return new JsAdHocJsonPropertyValue(const_cast<JsDgnElement*>(this), m_el->GetUserProperty(name.c_str()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
bool JsDgnElement::ContainsUserProperty(Utf8StringCR name) const
    {
    DGNJSAPI_VALIDATE_ARGS(IsValid(), false);
    return m_el->ContainsUserProperty(name.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
void JsDgnElement::RemoveUserProperty(Utf8StringCR name) const
    {
    DGNJSAPI_VALIDATE_ARGS_VOID(IsValid());
    m_el->RemoveUserProperty(name.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/16
//---------------------------------------------------------------------------------------
JsGeometrySourceP JsDgnElement::ToGeometrySource()
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    GeometrySourceP gs = m_el->ToGeometrySourceP();
    if (nullptr == gs)
        return nullptr;
    return new JsGeometrySource(*gs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/16
//---------------------------------------------------------------------------------------
JsGeometrySource3dP JsDgnElement::ToGeometrySource3d()
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    GeometrySource3dP gs = m_el->ToGeometrySource3dP();
    if (nullptr == gs)
        return nullptr;
    return new JsGeometrySource3d(*gs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/16
//---------------------------------------------------------------------------------------
JsGeometrySource2dP JsDgnElement::ToGeometrySource2d()
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    GeometrySource2dP gs = m_el->ToGeometrySource2dP();
    if (nullptr == gs)
        return nullptr;
    return new JsGeometrySource2d(*gs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsPlacement3dP JsGeometricElementBase::GetPlacement() const
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    return new JsPlacement3d(m_el->ToGeometrySource3d()->GetPlacement());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
int32_t JsGeometricElementBase::Transform(JsTransformP jstransform)
    {
    DGNJSAPI_VALIDATE_ARGS_ERROR(IsValid());

    if (m_el->IsPersistent())
        m_el = m_el->CopyForEdit();

    return (DgnDbStatus::Success == DgnElementTransformer::ApplyTransformTo(*m_el, jstransform->Get()))? 0: -2;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsDgnElement* JsDgnElement::Create(JsDgnModelP model, Utf8StringCR ecSqlClassName)
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSOBJ(model))
    auto el = createElementByClass(*model->m_model, ecSqlClassName.c_str());
    if (!el.IsValid())
        {
        DGNJSAPI_DGNSCRIPT_THROW("DgnElement.Create", ecSqlClassName.c_str());
        return nullptr;
        }
    return new JsDgnElement(*el);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsGeometricElement3d* JsGeometricElement3d::CreateGeometricElement3d(JsDgnModelP model, JsDgnObjectIdP catid, Utf8StringCR ecSqlClassName)
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSOBJ(model) && DGNJSAPI_IS_VALID_JSOBJ(catid))
    auto el = createElementByClass(*model->m_model, ecSqlClassName.c_str());
    if (!el.IsValid())
        {
        DGNJSAPI_DGNSCRIPT_THROW("JsGeometricElement3d.Create", ecSqlClassName.c_str());
        return nullptr;
        }
    auto gel = dynamic_cast<GeometricElement3d*>(el.get());     // *** WIP_GeometricElement3d - DgnElement should have a _ToGeometricElement3d method
    if (nullptr == gel)
        {
        Utf8PrintfString msg("[%s] is not a subclass of GeometricElement3d", ecSqlClassName.c_str());
        DGNJSAPI_DGNSCRIPT_THROW("JsGeometricElement3d.Create", msg.c_str());
        return nullptr;
        }
    gel->SetCategoryId(DgnCategoryId(catid->m_id));
    return new JsGeometricElement3d(*gel);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsDgnModelP JsDgnElement::GetModel() 
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    return new JsDgnModel(*m_el->GetModel());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsECClassP JsDgnElement::GetElementClass() 
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    return new JsECClass(*m_el->GetElementClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus JsDgnElement::PopulateRequest(JsRepositoryRequestP req, BeSQLiteDbOpcode opcode)
    {
    DGNJSAPI_VALIDATE_ARGS(IsValid() && nullptr != req, RepositoryStatus::InvalidRequest);
    return m_el->PopulateRequest(req->m_req, static_cast<BeSQLite::DbOpcode>(opcode));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus JsDgnModel::PopulateRequest(JsRepositoryRequestP req, BeSQLiteDbOpcode opcode)
    {
    DGNJSAPI_VALIDATE_ARGS(IsValid() && nullptr != req, RepositoryStatus::InvalidRequest);
    return m_model->PopulateRequest(req->m_req, static_cast<BeSQLite::DbOpcode>(opcode));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
struct ECDbIssueListener : BeSQLite::EC::ECDb::IIssueListener
    {
    mutable BeSQLite::EC::ECDbIssueSeverity m_severity;
    mutable Utf8String m_issue;

    void _OnIssueReported(BeSQLite::EC::ECDbIssueSeverity severity, Utf8CP message) const override
        {
        m_severity = severity;
        m_issue = message;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/16
//---------------------------------------------------------------------------------------
JsPreparedECSqlStatementP JsDgnDb::GetPreparedECSqlSelectStatement(Utf8StringCR ecsqlFragment)
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    Utf8String ecsql;
    if (!ecsqlFragment.StartsWithI("SELECT"))
        ecsql.append("SELECT "); // We want to prevent callers from doing INSERT, UPDATE, or DELETE. Pre-pending SELECT will guarantee a prepare error if ecsqlFragment also contains one of those keywords.
    ecsql.append(ecsqlFragment);
    ECDbIssueListener issues;
    m_db->AddIssueListener(issues);
    auto stmt = m_db->GetPreparedECSqlStatement(ecsql.c_str());
    m_db->RemoveIssueListener();
    if (!stmt.IsValid())
        {
        Utf8String msg (ecsql);
        msg.append(" - ").append(issues.m_issue.c_str());
        DGNJSAPI_DGNSCRIPT_THROW("ECSql", msg.c_str());
        return nullptr;
        }
    return new JsPreparedECSqlStatement(*stmt);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/16
//---------------------------------------------------------------------------------------
int32_t JsDgnDb::SaveChanges()
    {
    DGNJSAPI_VALIDATE_ARGS(IsValid(), -1);
    auto res = (int32_t)m_db->SaveChanges();

    m_db->Memory().Purge();

    if (JsDgnElement::s_count > 1000)
        {
        LOG.warningv("DgnJsApi - element count is %d. Call Dispose to free objects.", JsDgnElement::s_count);
        }
    //printf("saved and purged:");
    //getchar();
    return res;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/16
//---------------------------------------------------------------------------------------
JsDgnElementP JsDgnElements::FindElement(JsDgnObjectIdP id) const
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSOBJ(id));
    auto el = m_elements.FindElement(DgnElementId(id->m_id));
    return (nullptr != el) ? new JsDgnElement(*el->CopyForEdit()): nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/16
//---------------------------------------------------------------------------------------
JsDgnElementP JsDgnElements::GetElement(JsDgnObjectIdP id) const
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSOBJ(id));
    auto el = m_elements.GetElement(DgnElementId(id->m_id));
    return el.IsValid() ? new JsDgnElement(*el->CopyForEdit()) : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/16
//---------------------------------------------------------------------------------------
JsDgnObjectIdP JsDgnElements::QueryElementIdByCode(Utf8StringCR codeAuthorityName, Utf8StringCR codeValue, Utf8StringCR nameSpace) const
    {
    DgnElementId id = m_elements.QueryElementIdByCode(codeAuthorityName.c_str(), codeValue, nameSpace);
    return id.IsValid()? new JsDgnObjectId(id.GetValue()): nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsDgnModelsP JsDgnDb::GetModels()
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    return new JsDgnModels(m_db->Models());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsComponentDefP JsComponentDef::FindByName(JsDgnDbP db, Utf8StringCR name) 
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSOBJ(db));
    ComponentDefPtr cdef = ComponentDef::FromECSqlName(nullptr, *db->m_db, name);
    if (!cdef.IsValid())
        return nullptr;
    return new JsComponentDef(*cdef);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsDgnCategoryP JsComponentDef::GetCategory() const 
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    DgnCategoryCPtr cat = DgnCategory::QueryCategory(m_cdef->GetCategoryName(), m_cdef->GetDgnDb());
    return cat.IsValid()? new JsDgnCategory(*cat): nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsECClassP JsComponentDef::GetComponentECClass() const
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    return new JsECClass(m_cdef->GetECClass());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsDgnElementP JsComponentDef::QueryVariationByName(Utf8StringCR variationName)
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    auto velem = m_cdef->QueryVariationByName(variationName);
    return velem.IsValid()? new JsDgnElement(*velem->CopyForEdit()): nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsECInstanceP JsComponentDef::GetParameters(JsDgnElementP instance)
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSOBJ(instance));
    auto params = ComponentDef::GetParameters(*instance->m_el);
    if (!params.IsValid())
        return nullptr;
    return new JsECInstance(*params);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsDgnElementP JsComponentDef::MakeInstanceOfVariation(JsDgnModelP targetModel, JsDgnElementP variation, JsECInstanceP instanceParameters, JsAuthorityIssuedCodeP code)
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid() && DGNJSAPI_IS_VALID_JSOBJ(targetModel) && DGNJSAPI_IS_VALID_JSOBJ(variation));

    auto inst = m_cdef->MakeInstanceOfVariation(nullptr, *targetModel->m_model, *variation->m_el, 
                                                instanceParameters? instanceParameters->m_instance.get(): nullptr, 
                                                code? code->m_code: DgnCode()); 
    if (!inst.IsValid())
        {
        DGNJSAPI_DGNSCRIPT_THROW("MakeInstanceOfVariation", m_cdef->GetName().c_str());
        return nullptr;
        }

    return new JsDgnElement(*inst->CopyForEdit());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsDgnElementP JsComponentDef::MakeUniqueInstance(JsDgnModelP targetModel, JsECInstanceP instanceParameters, JsAuthorityIssuedCodeP code)
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid() && DGNJSAPI_IS_VALID_JSOBJ(targetModel) && DGNJSAPI_IS_VALID_JSOBJ(instanceParameters));
    auto inst = m_cdef->MakeUniqueInstance(nullptr, *targetModel->m_model, *instanceParameters->m_instance, code? code->m_code: DgnCode()); 
    if (!inst.IsValid())
        {
        DGNJSAPI_DGNSCRIPT_THROW("MakeInstanceOfVariation", m_cdef->GetName().c_str());
        return nullptr;
        }
    return new JsDgnElement(*inst->CopyForEdit());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsECInstanceP JsComponentDef::MakeParameters()
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    return new JsECInstance(*m_cdef->MakeVariationSpec());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
JsECInstanceP getCustomAttribute(T* ccontainer, Utf8StringCR className)
    {
    if (nullptr == ccontainer)
        return nullptr;
    ECN::IECInstancePtr ca = ccontainer->GetCustomAttribute(className);
    return ca.IsValid()? new JsECInstance(*ca): nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
JsECInstanceP JsECClass::GetCustomAttribute(Utf8StringCR className) const {return getCustomAttribute(m_ecClass, className);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
JsECInstanceP JsECProperty::GetCustomAttribute(Utf8StringCR className) const {return getCustomAttribute(m_property, className);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
JsECSchemaP JsECClass::GetSchema() const
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    return new JsECSchema(m_ecClass->GetSchema());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
JsECPropertyP JsECClass::GetProperty(Utf8StringCR name) const
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    auto prop = m_ecClass->GetPropertyP(name.c_str());
    return prop? new JsECProperty(*prop): nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
JsPrimitiveECPropertyP JsECProperty::GetAsPrimitiveProperty() const 
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    auto prim = m_property->GetAsPrimitiveProperty();
    return prim? new JsPrimitiveECProperty(*prim): nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
JsECClassP JsECClassCollection::GetECClass(JsECClassCollectionIteratorP iter) 
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid(iter) && (nullptr != *iter->m_iter));
    return new JsECClass(**iter->m_iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
JsECPropertyP JsECPropertyCollection::GetECProperty(JsECPropertyCollectionIteratorP iter) 
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid(iter) && (nullptr != *iter->m_iter));
    return new JsECProperty(**iter->m_iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
JsECDbSchemaManagerP JsDgnDb::GetSchemas() 
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    return new JsECDbSchemaManager(m_db->Schemas());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
JsDgnElementsP JsDgnDb::GetElements()
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    return new JsDgnElements(m_db->Elements());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
JsECInstanceP JsECClass::MakeInstance() 
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    ECN::IECInstancePtr inst = m_ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    if (!inst.IsValid())
        {
        DGNJSAPI_DGNSCRIPT_THROW("MakeInstance", m_ecClass->GetName().c_str());
        return nullptr;
        } 
    return new JsECInstance(*inst);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
int32_t Script::LoadScript(JsDgnDbP db, Utf8StringCR scriptName, bool forceReload)
    {
    DGNJSAPI_VALIDATE_ARGS_ERROR(DGNJSAPI_IS_VALID_JSOBJ(db));
    return (int32_t) DgnScript::LoadScript(*db->m_db, scriptName.c_str(), forceReload);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void Script::ImportLibrary (Utf8StringCR libName)
    {
    T_HOST.GetScriptAdmin().ImportScriptLibrary(libName.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void Script::ReportError (Utf8StringCR description)
    {
    T_HOST.GetScriptAdmin().HandleScriptError(DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::ReportedByScript, description.c_str(), "");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void Script::BeginDisposeContext()
    {
    T_HOST.GetScriptAdmin().GetDgnScriptContext().BeginCallContext();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void Script::EndDisposeContext()
    {
    T_HOST.GetScriptAdmin().GetDgnScriptContext().EndCallContext();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void Logging::Message(Utf8StringCR category, LoggingSeverity severity, Utf8StringCR message)
    {
    T_HOST.GetScriptAdmin().HandleLogMessage(category.c_str(), (DgnPlatformLib::Host::ScriptAdmin::LoggingSeverity)severity, message.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
static NativeLogging::SEVERITY toNativeLoggingSeverity(LoggingSeverity severity)
    {
    // *** NB: ScriptAdmin::LoggingSeverity must be the same as NativeLogging::SEVERITY, except that they are positive instead of negative
    return (NativeLogging::SEVERITY)(-(int32_t)severity); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void Logging::SetSeverity(Utf8StringCR category, LoggingSeverity severity)
    {
    NativeLogging::LoggingConfig::SetSeverity(category.c_str(), toNativeLoggingSeverity(severity));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
bool Logging::IsSeverityEnabled(Utf8StringCR category, LoggingSeverity severity)
    {
    return NativeLogging::LoggingManager::GetLogger(category.c_str())->isSeverityEnabled(toNativeLoggingSeverity(severity));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                      04/16
//---------------------------------------------------------------------------------------
JsECSqlArrayValueP JsECSqlValue::GetArray()
    {
    return new JsECSqlArrayValue(m_value->GetArray());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
BentleyStatus JsPreparedECSqlStatement::CheckValueIndexInRange(int idx)
    {
    if (0 <= idx && idx < m_stmt->GetColumnCount())
        return BSISUCCESS;
    DGNJSAPI_DGNSCRIPT_THROW("ECSql", "IndexOutOfRange");
    return BSIERROR;
    }

#define CHECK_BIND_RESULT(BINDEXP) if (ECSqlStatus::Success != (BINDEXP)) {DGNJSAPI_DGNSCRIPT_THROW("ECSql", "BindError");}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/16
//---------------------------------------------------------------------------------------
void JsPreparedECSqlStatement::BindId(int parameterIndex, JsDgnObjectIdP value)
    { 
    DGNJSAPI_VALIDATE_ARGS_VOID(IsValid() && DGNJSAPI_IS_VALID_JSOBJ(value)); 
    CHECK_BIND_RESULT(m_stmt->BindInt64(parameterIndex, value->m_id));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/16
//---------------------------------------------------------------------------------------
void JsPreparedECSqlStatement::BindText(int parameterIndex, Utf8StringCR value)
    { 
    DGNJSAPI_VALIDATE_ARGS_VOID(IsValid()); 
    CHECK_BIND_RESULT(m_stmt->BindText(parameterIndex, value.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/16
//---------------------------------------------------------------------------------------
void JsPreparedECSqlStatement::BindInt(int parameterIndex, int32_t value)
    { 
    DGNJSAPI_VALIDATE_ARGS_VOID(IsValid()); 
    CHECK_BIND_RESULT(m_stmt->BindInt(parameterIndex, value));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/16
//---------------------------------------------------------------------------------------
void JsPreparedECSqlStatement::BindDouble(int parameterIndex, double value)
    { 
    DGNJSAPI_VALIDATE_ARGS_VOID(IsValid()); 
    CHECK_BIND_RESULT(m_stmt->BindDouble(parameterIndex, value));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/16
//---------------------------------------------------------------------------------------
void JsPreparedECSqlStatement::BindDRange3d(int parameterIndex, JsDRange3dP value)
    {
    DGNJSAPI_VALIDATE_ARGS_VOID(IsValid() && value);
    CHECK_BIND_RESULT(m_stmt->BindBinary(parameterIndex, &value->GetCR(), sizeof(DRange3d), IECSqlBinder::MakeCopy::Yes));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void JsPreparedECSqlStatement::BindDPoint3d(int parameterIndex, JsDPoint3dP value) 
    {
    DGNJSAPI_VALIDATE_ARGS_VOID(nullptr != value);
    CHECK_BIND_RESULT(m_stmt->BindPoint3D(parameterIndex, value->GetCR()));
    }

#define CHECK_GET_VALUE_ARGS(COL,ERRVAL) DGNJSAPI_VALIDATE_ARGS(IsValid(), ERRVAL); if (BSISUCCESS != CheckValueIndexInRange(COL)) return ERRVAL;

Utf8String JsPreparedECSqlStatement::GetValueText(int32_t col) { CHECK_GET_VALUE_ARGS(col,""); return m_stmt->GetValueText(col); }
Utf8String JsPreparedECSqlStatement::GetValueDateTime(int32_t col) { CHECK_GET_VALUE_ARGS(col, ""); return m_stmt->GetValueDateTime(col).ToUtf8String(); }
double JsPreparedECSqlStatement::GetValueDouble(int32_t col) { CHECK_GET_VALUE_ARGS(col, 0.0); return m_stmt->GetValueDouble(col); }
JsDPoint3dP JsPreparedECSqlStatement::GetValueDPoint3d(int32_t col) { CHECK_GET_VALUE_ARGS(col, nullptr); return new JsDPoint3d(m_stmt->GetValuePoint3D(col)); }
int32_t JsPreparedECSqlStatement::GetValueInt(int32_t col) { CHECK_GET_VALUE_ARGS(col, 0); return m_stmt->GetValueInt(col); }
JsDgnObjectIdP JsPreparedECSqlStatement::GetValueId(int32_t col) { CHECK_GET_VALUE_ARGS(col, nullptr); return new JsDgnObjectId(m_stmt->GetValueUInt64(col)); }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/16
//---------------------------------------------------------------------------------------
JsDRange3dP JsPreparedECSqlStatement::GetValueDRange3d(int32_t col)
    {
    CHECK_GET_VALUE_ARGS(col,nullptr);
    int sz;
    void const* p = m_stmt->GetValueBinary(col, &sz);
    if (nullptr == p || sz != sizeof(DRange3d))
        {
        DGNJSAPI_DGNSCRIPT_THROW("ECSql", "ColumnType");
        return nullptr;
        }

    return new JsDRange3d(*(DRange3d*)p);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                      04/16
//---------------------------------------------------------------------------------------
JsECSqlArrayValueP JsPreparedECSqlStatement::GetValueArray(int32_t col)
    {
    CHECK_GET_VALUE_ARGS(col, nullptr);
    IECSqlArrayValue const& arrayValue = m_stmt->GetValueArray(col);
    return new JsECSqlArrayValue(arrayValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/16
//---------------------------------------------------------------------------------------
BeSQLiteDbResult JsPreparedECSqlStatement::Step() 
    {
    DGNJSAPI_VALIDATE_ARGS(IsValid(), BeSQLiteDbResult::BE_SQLITE_ERROR);
    switch (m_stmt->Step())
        {
        case BE_SQLITE_ROW: return BeSQLiteDbResult::BE_SQLITE_ROW;
        case BE_SQLITE_DONE: return BeSQLiteDbResult::BE_SQLITE_DONE;
        case BE_SQLITE_OK: return BeSQLiteDbResult::BE_SQLITE_OK;
        }
    return BeSQLiteDbResult::BE_SQLITE_ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/16
//---------------------------------------------------------------------------------------
int32_t JsPreparedECSqlStatement::GetParameterIndex(Utf8StringCR colName)
    {
    DGNJSAPI_VALIDATE_ARGS(IsValid(), 0); 
    auto i = m_stmt->GetParameterIndex(colName.c_str()); 
    if (-1 == i)
        DGNJSAPI_DGNSCRIPT_THROW("ECSql", "NamedParameterNotFound");
    return (int32_t)i;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsFile* JsFile::Fopen(Utf8StringCR name, Utf8StringCR mode)
    {
    BeFileName fn(name.c_str());
    auto fp = fopen(fn.GetNameUtf8().c_str(), mode.c_str());
    if (nullptr == fp)
        {
        perror("Fopen failed");
        return nullptr;
        }
    return new JsFile(fp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
bool JsFile::Feof()
    {
    DGNJSAPI_VALIDATE_ARGS(IsValid(), true); 
    return 0 != feof(m_fp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
Utf8String JsFile::ReadLine()
    {
    DGNJSAPI_VALIDATE_ARGS(IsValid(), ""); 

    if (ferror(m_fp))
        {
        DGNJSAPI_DGNSCRIPT_THROW("FERROR", "");
        return "";
        }

    char buf[4096];
    buf[0] = '\0';
    auto res = fgets(buf, sizeof(buf), m_fp);
    if (nullptr == res)
        {
        DGNJSAPI_DGNSCRIPT_THROW("EOF", "");
        return "";
        }
    return buf;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
int32_t JsFile::WriteLine(Utf8StringCR line)
    {
    DGNJSAPI_VALIDATE_ARGS(IsValid(), -1); 
    if (ferror(m_fp))
        {
        DGNJSAPI_DGNSCRIPT_THROW("FERROR", "");
        return -1;
        }
    auto res = fputs(line.c_str(), m_fp);
    if (EOF == res)
        {
        DGNJSAPI_DGNSCRIPT_THROW("FERROR", "");
        return -1;
        }
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
DgnJsApi::DgnJsApi(BeJsContext& jsContext) : m_context(jsContext)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
DgnJsApi::~DgnJsApi()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
void DgnJsApi::_ImportScriptLibrary(BeJsContextR jsContext, Utf8CP)
    {
    // TRICKY Unlike most JSApis, DgnJsApi does not have its own bootstrapping source. That is because it is combined with DgnScriptContext's boostrapping 
    // source, and so it must be evaluated later. See DgnPlatformLib::Host::ScriptAdmin::GetDgnScriptContext. 
    // *** NEEDS WORK: See if we can untangle this by building DgnScriptContext and DgnJsApi in different makefiles and thereby generating two different bootstrapping .cpp files.
    jsContext.RegisterProjection<DgnJsApiProjection>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
JsRepositoryRequest::JsRepositoryRequest(JsDgnDb& db, Utf8StringCR operation) : m_req(), m_db(&db), m_operation(operation)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus JsRepositoryRequest::Acquire()
    {
    DGNJSAPI_VALIDATE_ARGS(m_db && m_db->m_db.IsValid(), RepositoryStatus::InvalidRequest);
    m_req.SetOptions(T_HOST.GetRepositoryAdmin()._GetResponseOptions(false));
    auto response = m_db->m_db->BriefcaseManager().Acquire(m_req);
    T_HOST.GetRepositoryAdmin()._OnResponse(response, m_operation.c_str());
    return response.Result();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus JsRepositoryRequest::Query(bool fast)
    {
    DGNJSAPI_VALIDATE_ARGS(m_db && m_db->m_db.IsValid(), RepositoryStatus::InvalidRequest);
    m_req.SetOptions(T_HOST.GetRepositoryAdmin()._GetResponseOptions(true));
    IBriefcaseManager::Response response(fast ? IBriefcaseManager::RequestPurpose::FastQuery : IBriefcaseManager::RequestPurpose::Query, m_req.Options());
    m_db->m_db->BriefcaseManager().AreResourcesAvailable(m_req, &response, fast ? IBriefcaseManager::FastQuery::Yes : IBriefcaseManager::FastQuery::No);
    T_HOST.GetRepositoryAdmin()._OnResponse(response, m_operation.c_str());
    return response.Result();
    }

