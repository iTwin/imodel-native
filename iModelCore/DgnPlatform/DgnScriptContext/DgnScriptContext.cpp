/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScriptContext/DgnScriptContext.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeJavaScript/BeJavaScript.h>
#include <DgnPlatform/DgnCore/DgnScriptContext.h>
#include <Bentley/BeFileListIterator.h>
#include <ECObjects/ECObjectsAPI.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include "DgnScriptContextImpl.h"
#include <DgnPlatform/DgnObjectModelJsProjections.h>

extern Utf8CP dgnScriptContext_GetBootstrappingSource();

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
template<typename T>
static void doDelete(void* b) {delete (T*)b;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static RefCountedPtr<PhysicalElement> createPhysicalElement(DgnModelR model, Utf8CP ecSqlClassName, DgnCategoryId catid, Utf8CP code)//, RefCountedPtr<T> geom)
    {
    Utf8CP dot = strchr(ecSqlClassName, '.');
    if (nullptr == dot)
        return nullptr;
    Utf8String ecschema(ecSqlClassName, dot);
    Utf8String ecclass(dot+1);
    DgnDbR db = model.GetDgnDb();
    DgnClassId pclassId = DgnClassId(db.Schemas().GetECClassId(ecschema.c_str(), ecclass.c_str()));
    PhysicalElementPtr el = PhysicalElement::Create(PhysicalElement::CreateParams(db, model.GetModelId(), pclassId, catid));

    if (nullptr != code)
        el->SetCode(code);

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
// *** TEMPORARY METHOD *** 
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
void JsElementGeometryBuilder::AppendBox(double x, double y, double z)
    {
    // *** TEMPORARY METHOD *** 
    DPoint3d localOrigin;
    localOrigin.x = 0.0;
    localOrigin.y = 0.0;
    localOrigin.z = 0.0;

    DPoint3d localTop (localOrigin);
    localTop.z = z;

    DVec3d localX = DVec3d::From(1,0,0);
    DVec3d localY = DVec3d::From(0,1,0);

    DgnBoxDetail boxd(localOrigin, localTop, localX, localY, x, y, x, y, true);
    ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnBox(boxd);

    m_builder->Append(*solid);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsElementGeometryBuilder* JsElementGeometryBuilder::Create(JsDgnElementP e, JsDPoint3dP o, JsYawPitchRollAnglesP a)
    {
    DgnElement3dP e3d = dynamic_cast<DgnElement3dP>(e->m_el.get());
    if (nullptr != e3d)
        return new JsElementGeometryBuilder(*e3d, o->m_pt, a->m_angles);

    DgnElement2dP e2d = dynamic_cast<DgnElement2dP>(e->m_el.get());
    if (nullptr != e2d)
        return new JsElementGeometryBuilder(*e2d, DPoint2d::From(o->GetX(), o->GetY()), AngleInDegrees::FromDegrees(a->GetYaw()));

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsDgnElement* JsDgnModel::CreateElement(Utf8StringCR ecSqlClassName, Utf8StringCR categoryName)
    {
    DgnCategoryId catid = m_model->GetDgnDb().Categories().QueryCategoryId(categoryName.c_str());
    return new JsDgnElement(*createPhysicalElement(*m_model, ecSqlClassName.c_str(), catid, nullptr));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
DgnScriptContextImpl::DgnScriptContextImpl(BeJsEnvironmentR jsenv)
    : 
    BeJsContext(jsenv, "DgnScriptContext")
    {
    RegisterProjectionCallbacks (&InitializeJsProjections, &DestroyJsProjections, dgnScriptContext_GetBootstrappingSource(), "file:///BePortableUi.js");

    m_egaRegistry = EvaluateScript("BentleyApi.Dgn.GetEgaRegistry()");
    m_modelSolverRegistry = EvaluateScript("BentleyApi.Dgn.GetModelSolverRegistry()");

    BeAssert(!m_egaRegistry.IsUndefined() && m_egaRegistry.IsObject());
    BeAssert(!m_modelSolverRegistry.IsUndefined() && m_modelSolverRegistry.IsObject());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
DgnScriptContextImpl::~DgnScriptContextImpl()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContextImpl::LoadProgram(Dgn::DgnDbR db, Utf8CP jsFunctionSpec)
    {
    Utf8String jsProgramName;
    Utf8CP dot = strrchr(jsFunctionSpec, '.');
    if (nullptr == dot)
        {
        NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->errorv ("[%s] is an illegal Script function spec. Must be of the form program.function", jsFunctionSpec);
        BeAssert(false && "illegal Script function spec");
        return DgnDbStatus::BadArg;
        }

    jsProgramName.assign(jsFunctionSpec, dot);

    if (m_jsScriptsExecuted.find(jsProgramName) != m_jsScriptsExecuted.end())
        return DgnDbStatus::Success;

    DgnScriptType sTypePreferred = DgnScriptType::JavaScript;
    DgnScriptType sTypeFound;
    Utf8String jsprog;
    DgnDbStatus status = T_HOST.GetScriptingAdmin()._FetchScript(jsprog, sTypeFound, db, jsProgramName.c_str(), sTypePreferred);
    if (DgnDbStatus::Success != status)
        {
        NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->infov ("Script program %s is not registered in the script library", jsProgramName.c_str());
        return status;
        }

    if (sTypeFound != DgnScriptType::JavaScript)
        {
        switch (sTypeFound)
            {
            case DgnScriptType::TypeScript:
                BeAssert(false && "*** TBD: compile script type on the fly");
                return DgnDbStatus::NotEnabled;
                break;
            default:
                BeAssert(false && "Unsupported script type");
                return DgnDbStatus::NotEnabled;
                break;
            }
        }

    m_jsScriptsExecuted.insert(jsProgramName);

    Utf8String fileUrl("file:///"); // This does not really identify a file. It is something tricky that is needed to get JS to accept the script that we pass in jsprog.
    fileUrl.append(jsProgramName);
    fileUrl.append(".js");

    NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->tracev ("Evaluating %s", jsProgramName.c_str());

    EvaluateScript(jsprog.c_str(), fileUrl.c_str());   // evaluate the whole script, allowing it to define objecjs and their properties. 
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContextImpl::ExecuteEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms)
    {
    functionReturnStatus = -1;

    DgnDbStatus status = LoadProgram(el.GetDgnDb(), jsEgaFunctionName);
    if (DgnDbStatus::Success != status)
        return status;

    BeJsFunction jsfunc = m_egaRegistry.GetFunctionProperty(jsEgaFunctionName);
    if (jsfunc.IsUndefined() || !jsfunc.IsFunction())
        {
        NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->errorv ("[%s] is not registered as an EGA", jsEgaFunctionName);
        BeAssert(false && "EGA not registered");
        return DgnDbStatus::NotEnabled;
        }

    BeJsObject parmsObj = EvaluateJson(parms);
    BeJsNativePointer jsel = ObtainProjectedClassInstancePointer(new JsDgnElement(el), false);
    BeJsNativePointer jsorigin = ObtainProjectedClassInstancePointer(new JsDPoint3d(origin), true);
    BeJsNativePointer jsangles = ObtainProjectedClassInstancePointer(new JsYawPitchRollAngles(angles), true);
    BeJsValue retval = jsfunc(jsel, jsorigin, jsangles, parmsObj);

    auto jselwrapper = jsel.GetValueTyped<JsDgnElement>(); // Don't wait for GC to release the element
    jsel.SetValue(nullptr);
    delete jselwrapper;

    if (!retval.IsNumber())
        {
        NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->errorv ("[%s] does not have the correct signature for an EGA - must return an int", jsEgaFunctionName);
        BeAssert(false && "EGA has incorrect return type");
        return DgnDbStatus::NotEnabled;
        }

    BeJsNumber num(retval);
    functionReturnStatus = num.GetIntegerValue();
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContextImpl::ExecuteModelSolver(int& functionReturnStatus, Dgn::DgnModelR model, Utf8CP jsFunctionName, Json::Value const& parms)
    {
    functionReturnStatus = -1;

    DgnDbStatus status = LoadProgram(model.GetDgnDb(), jsFunctionName);
    if (DgnDbStatus::Success != status)
        return status;

    BeJsFunction jsfunc = m_modelSolverRegistry.GetFunctionProperty(jsFunctionName);
    if (jsfunc.IsUndefined() || !jsfunc.IsFunction())
        {
        NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->errorv ("[%s] is not registered as a model solver", jsFunctionName);
        BeAssert(false && "model solver not registered");
        return DgnDbStatus::NotEnabled;
        }

    BeJsObject parmsObj = EvaluateJson(parms);
    BeJsNativePointer jsModel = ObtainProjectedClassInstancePointer(new JsDgnModel(model), true);
    BeJsValue retval = jsfunc(jsModel, parmsObj);

    if (!retval.IsNumber())
        {
        NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->errorv ("[%s] does not have the correct signature for a model solver - must return an int", jsFunctionName);
        BeAssert(false && "model solver has incorrect return type");
        return DgnDbStatus::NotEnabled;
        }

    BeJsNumber num(retval);
    functionReturnStatus = num.GetIntegerValue();
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
DgnScriptContext::DgnScriptContext(BeJsEnvironmentR jsenv)
    {
    m_pimpl = new DgnScriptContextImpl(jsenv);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
DgnScriptContext::~DgnScriptContext()
    {
    delete m_pimpl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContext::ExecuteEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms)
    {
    return m_pimpl->ExecuteEga(functionReturnStatus, el, jsEgaFunctionName, origin, angles, parms);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContext::ExecuteModelSolver(int& functionReturnStatus, Dgn::DgnModelR model, Utf8CP jsFunctionName, Json::Value const& parms)
    {
    return m_pimpl->ExecuteModelSolver(functionReturnStatus, model, jsFunctionName, parms);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptingAdmin::ScriptingAdmin()
    {
    m_jsenv = nullptr;
    m_dgnContext = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptingAdmin::~ScriptingAdmin()
    {
    if (nullptr != m_dgnContext)
        delete m_dgnContext;
    if (nullptr != m_jsenv)
        delete m_jsenv;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
BeJsEnvironmentR DgnPlatformLib::Host::ScriptingAdmin::GetBeJsEnvironment()
    {
    if (nullptr == m_jsenv)
        m_jsenv = new BeJsEnvironment;
    return *m_jsenv;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnScriptContextR DgnPlatformLib::Host::ScriptingAdmin::GetDgnScriptContext()
    {
    if (nullptr == m_dgnContext)
        m_dgnContext = new DgnScriptContext(GetBeJsEnvironment());
    return *m_dgnContext;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnPlatformLib::Host::ScriptingAdmin::_FetchScript(Utf8StringR sText, DgnScriptType& stypeFound, DgnDbR db, Utf8CP sName, DgnScriptType stypePreferred)
    {
    DgnScriptLibrary jslib(db);
    return jslib.QueryScript(sText, stypeFound, sName, stypePreferred);
    }
