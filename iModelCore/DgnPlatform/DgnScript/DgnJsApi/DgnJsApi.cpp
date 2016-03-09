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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static RefCountedPtr<GeometricElement3d> createGeometricElement3d(DgnModelR model, Utf8CP ecSqlClassName, DgnCategoryId catid)//, RefCountedPtr<T> geom)
    {
    if (!ecSqlClassName || !*ecSqlClassName)
        ecSqlClassName = GENERIC_SCHEMA(GENERIC_CLASSNAME_PhysicalObject);
    Utf8CP dot = strchr(ecSqlClassName, '.');
    if (nullptr == dot)
        return nullptr;
    Utf8String ecschema(ecSqlClassName, dot);
    Utf8String ecclass(dot+1);
    DgnDbR db = model.GetDgnDb();
    DgnClassId pclassId = DgnClassId(db.Schemas().GetECClassId(ecschema.c_str(), ecclass.c_str()));
    if (!pclassId.IsValid())
        return nullptr;
    DgnElementPtr el = dgn_ElementHandler::Geometric3d::GetHandler().Create(GeometricElement3d::CreateParams(db, model.GetModelId(), pclassId, catid));
    GeometricElement3d* geom = JsPhysicalElement::ToGeometricElement3d(*el);
    geom->SetCategoryId(catid); // *** TRICKY: Generic ElementHandler::Create does not set Category
    return geom;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsGeometryBuilder::JsGeometryBuilder(JsDgnElementP e, JsDPoint3dP o, JsYawPitchRollAnglesP a)
    {
    DGNJSAPI_VALIDATE_ARGS_VOID(DGNJSAPI_IS_VALID_JSOBJ(e) && e->m_el->ToGeometrySource() && o && a);

    GeometrySource3dCP source3d = e->m_el->ToGeometrySource3d();
    if (nullptr != source3d)
        m_builder = GeometryBuilder::Create(*source3d, o->Get (), a->GetYawPitchRollAngles ());
    else
        {
        GeometrySource2dCP source2d = e->m_el->ToGeometrySource2d();
        if (nullptr != source2d)
            m_builder = GeometryBuilder::Create(*source2d, DPoint2d::From(o->GetX(), o->GetY()), AngleInDegrees::FromDegrees(a->GetYawDegrees ()));
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
JsGeometryCollectionP JsPhysicalElement::GetGeometry() const 
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
    auto cptr = m_el->Insert();
    if (!cptr.IsValid())
        return -2;
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
JsECValueP JsDgnElement::GetUnhandledProperty(Utf8StringCR name) 
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    ECN::ECValue v;
    if (m_el->GetUnhandledPropertyValue(v, name.c_str()) != DgnDbStatus::Success || v.IsNull())
        return nullptr;
    return new JsECValue(v);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
int32_t JsDgnElement::SetUnhandledProperty(Utf8StringCR name, JsECValueP v)
    {
    DGNJSAPI_VALIDATE_ARGS_ERROR(IsValid());
    return (int32_t) m_el->SetUnhandledPropertyValue(name.c_str(), v->m_value);
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
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsPlacement3dP JsPhysicalElement::GetPlacement() const 
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
    return new JsPlacement3d(m_el->ToGeometrySource3d()->GetPlacement());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
int32_t JsPhysicalElement::Transform(JsTransformP jstransform)
    {
    DGNJSAPI_VALIDATE_ARGS_ERROR(IsValid());

    if (m_el->IsPersistent())
        m_el = m_el->CopyForEdit();

    return (DgnDbStatus::Success == DgnElementTransformer::ApplyTransformTo(*m_el, jstransform->Get()))? 0: -2;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsPhysicalElement* JsPhysicalElement::Create(JsDgnModelP model, JsDgnObjectIdP categoryId, Utf8StringCR ecSqlClassName)
    {
    DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSOBJ(model) && DGNJSAPI_IS_VALID_JSOBJ(categoryId))
    DgnCategoryId catid(categoryId->m_id);
    auto geom = createGeometricElement3d(*model->m_model, ecSqlClassName.c_str(), catid);
    if (!geom.IsValid())
        {
        DGNJSAPI_DGNSCRIPT_THROW("Create", ecSqlClassName.c_str());
        return nullptr;
        }
    return new JsPhysicalElement(*geom);
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
JsECInstanceP JsECClass::GetCustomAttribute(Utf8StringCR className) {return getCustomAttribute(m_ecClass, className);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
JsECInstanceP JsECProperty::GetCustomAttribute(Utf8StringCR className) {return getCustomAttribute(m_property, className);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
JsECPropertyP JsECClass::GetProperty(Utf8StringCR name)
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
