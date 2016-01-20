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

USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static RefCountedPtr<PhysicalElement> createPhysicalElement(DgnModelR model, Utf8CP ecSqlClassName, DgnCategoryId catid)//, RefCountedPtr<T> geom)
    {
    if (!ecSqlClassName || !*ecSqlClassName)
        ecSqlClassName = DGN_SCHEMA(DGN_CLASSNAME_PhysicalElement);
    Utf8CP dot = strchr(ecSqlClassName, '.');
    if (nullptr == dot)
        return nullptr;
    Utf8String ecschema(ecSqlClassName, dot);
    Utf8String ecclass(dot+1);
    DgnDbR db = model.GetDgnDb();
    DgnClassId pclassId = DgnClassId(db.Schemas().GetECClassId(ecschema.c_str(), ecclass.c_str()));
    PhysicalElementPtr el = PhysicalElement::Create(PhysicalElement::CreateParams(db, model.GetModelId(), pclassId, catid));

    return el;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsElementGeometryBuilder::JsElementGeometryBuilder(JsDgnElementP e, JsDPoint3dP o, JsYawPitchRollAnglesP a)
    {
    GeometrySource3dCP source3d = e->m_el->ToGeometrySource3d();
    if (nullptr != source3d)
        m_builder = ElementGeometryBuilder::Create(*source3d, o->Get (), a->GetYawPitchRollAngles ());
    else
        {
        GeometrySource2dCP source2d = e->m_el->ToGeometrySource2d();
        if (nullptr != source2d)
            m_builder = ElementGeometryBuilder::Create(*source2d, DPoint2d::From(o->GetX(), o->GetY()), AngleInDegrees::FromDegrees(a->GetYawDegrees ()));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsPhysicalElement* JsPhysicalElement::Create(JsDgnModelP model, JsDgnObjectIdP categoryId, Utf8StringCR ecSqlClassName)
    {
    if (!categoryId || !categoryId->IsValid() || !model || !model->m_model.IsValid())
        return nullptr;
    DgnCategoryId catid(categoryId->m_id);
    return new JsPhysicalElement(*createPhysicalElement(*model->m_model, ecSqlClassName.c_str(), catid));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsDgnModelP JsDgnElement::GetModel() {return new JsDgnModel(*m_el->GetModel());}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsDgnModelsP JsDgnDb::GetModels() {return new JsDgnModels(m_db->Models());}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsComponentDefP JsComponentDef::FindByName(JsDgnDbP db, Utf8StringCR name) 
    {
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
    if (!m_cdef.IsValid())
        return nullptr;
    DgnCategoryCPtr cat = DgnCategory::QueryCategory(m_cdef->GetCategoryName(), m_cdef->GetDgnDb());
    return cat.IsValid()? new JsDgnCategory(*cat): nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsECClassP JsComponentDef::GetComponentECClass() const
    {
    if (!m_cdef.IsValid())
        return nullptr;
    return new JsECClass(m_cdef->GetECClass());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsDgnElementP JsComponentDef::QueryVariationByName(Utf8StringCR variationName)
    {
    if (!m_cdef.IsValid())
        return nullptr;
    auto velem = m_cdef->QueryVariationByName(variationName);
    return velem.IsValid()? new JsDgnElement(*velem->CopyForEdit()): nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsECInstanceP JsComponentDef::GetParameters(JsDgnElementP instance)
    {
    if (nullptr == instance || !instance->m_el.IsValid())
        return nullptr;
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
    if (!m_cdef.IsValid() || nullptr == targetModel || !targetModel->m_model.IsValid() || nullptr == variation || !variation->m_el.IsValid())
        return nullptr;
    auto inst = m_cdef->MakeInstanceOfVariation(nullptr, *targetModel->m_model, *variation->m_el, 
                                                instanceParameters? instanceParameters->m_instance.get(): nullptr, 
                                                code? code->m_code: DgnCode()); 
    if (!inst.IsValid())
        return nullptr;
    return new JsDgnElement(*inst->CopyForEdit());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsDgnElementP JsComponentDef::MakeUniqueInstance(JsDgnModelP targetModel, JsECInstanceP instanceParameters, JsAuthorityIssuedCodeP code)
    {
    if (!m_cdef.IsValid() || nullptr == targetModel || !targetModel->m_model.IsValid() || nullptr == instanceParameters || !instanceParameters->m_instance.IsValid())
        return nullptr;
    auto inst = m_cdef->MakeUniqueInstance(nullptr, *targetModel->m_model, *instanceParameters->m_instance, code? code->m_code: DgnCode()); 
    if (!inst.IsValid())
        return nullptr;
    return new JsDgnElement(*inst->CopyForEdit());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsECInstanceP JsComponentDef::MakeParameters()
    {
    if (!m_cdef.IsValid())
        return nullptr;
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
JsECClassP JsECClassCollection::GetECClass(JsECClassCollectionIteratorP iter) {return (IsValid(iter) && (nullptr != *iter->m_iter))? new JsECClass(**iter->m_iter): nullptr;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
JsECPropertyP JsECPropertyCollection::GetECProperty(JsECPropertyCollectionIteratorP iter) {return (IsValid(iter) && (nullptr != *iter->m_iter))? new JsECProperty(**iter->m_iter): nullptr;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
JsECDbSchemaManagerP JsDgnDb::GetSchemas() {return m_db.IsValid()? new JsECDbSchemaManager(m_db->Schemas()): nullptr;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
JsECInstanceP JsECClass::MakeInstance() 
    {
    ECN::IECInstancePtr inst = m_ecClass? m_ecClass->GetDefaultStandaloneEnabler()->CreateInstance(): nullptr; 
    return inst.IsValid()? new JsECInstance(*inst): nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
int32_t Script::LoadScript(JsDgnDbP db, Utf8StringCR scriptName)
    {
    if (!db || !db->m_db.IsValid())
        return -1;
    return (int32_t) DgnScript::LoadScript(*db->m_db, scriptName.c_str());
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
