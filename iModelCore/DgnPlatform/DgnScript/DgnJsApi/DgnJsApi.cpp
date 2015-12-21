/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScript/DgnJsApi/DgnJsApi.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void JsDgnModel::DeleteAllElements() 
    {
    m_model->FillModel();
    bvector<DgnElementCPtr> elementsInModel;
    for (auto const& emapEntry : *m_model)
        elementsInModel.push_back(emapEntry.second);
    for (auto const& el : elementsInModel)
        el->Delete();
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
JsComponentModelP JsDgnModel::ToComponentModel() 
    {
    ComponentModel* cm = ToDgnComponentModel();
    return (nullptr == cm)? nullptr: new JsComponentModel(*cm);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
static BentleyStatus loadParams(ModelSolverDef::ParameterSet& params, ComponentModel& cm, Utf8StringCR paramsJSON)
    {
    Json::Value paramsJsonValue(Json::objectValue);
    if (!Json::Reader::Parse(paramsJSON.c_str(), paramsJsonValue))
        return BSIERROR;

    ModelSolverDef::ParameterSet newParameterValues = cm.GetSolver().GetParameters();
    for (auto pname : paramsJsonValue.getMemberNames())
        {
        ModelSolverDef::Parameter* sparam = newParameterValues.GetParameterP(pname.c_str());
        if (nullptr == sparam)
            {
            // *** TBD: print warning
            continue;
            }
        ECN::ECValue ecv;
        ECUtils::ConvertJsonToECValue(ecv, paramsJsonValue[pname], sparam->GetValue().GetPrimitiveType());
        sparam->SetValue(ecv);
        }
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
JsDgnElement* JsComponentModel::MakeInstance(JsDgnModelP targetJsModel, Utf8StringCR capturedSolutionName, Utf8StringCR paramsJSON, JsAuthorityIssuedCodeP jscode)
    {
    ComponentModel* cm = ToDgnComponentModel();
    if (nullptr == cm || nullptr == targetJsModel || !targetJsModel->m_model.IsValid())
        return nullptr;
    DgnModelR targetModel = *targetJsModel->m_model;

    ModelSolverDef::ParameterSet params;
    if (BSISUCCESS != loadParams(params, *cm, paramsJSON))
        return nullptr;

    DgnElement::Code ecode;
    if (nullptr != jscode)
        ecode = jscode->m_code;

    DgnElementCPtr instance = cm->MakeInstance(nullptr, targetModel, capturedSolutionName, params, ecode);
    if (!instance.IsValid())
        return nullptr;

    return new JsDgnElement(*const_cast<DgnElementP>(instance.get()));
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
