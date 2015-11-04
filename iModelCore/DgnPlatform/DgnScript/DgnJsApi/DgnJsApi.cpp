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
// *** TEMPORARY METHOD *** 
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
void JsElementGeometryBuilder::AppendSphere(double radius)
    {
    DgnSphereDetail sphere(DPoint3d::FromZero(), radius);
    ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnSphere(sphere);
    m_builder->Append(*solid);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsElementGeometryBuilder::JsElementGeometryBuilder(JsDgnElementP e, JsDPoint3dP o, JsYawPitchRollAnglesP a)
    {
    DgnElement3dP e3d = dynamic_cast<DgnElement3dP>(e->m_el.get());
    if (nullptr != e3d)
        m_builder = ElementGeometryBuilder::Create(*e3d, o->Get (), a->GetYawPitchRollAngles ());
    else
        {
        DgnElement2dP e2d = dynamic_cast<DgnElement2dP>(e->m_el.get());
        if (nullptr != e2d)
            m_builder = ElementGeometryBuilder::Create(*e2d, DPoint2d::From(o->GetX(), o->GetY()), AngleInDegrees::FromDegrees(a->GetYawDegrees ()));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsDgnElement* JsDgnModel::CreateElement(Utf8StringCR ecSqlClassName, Utf8StringCR categoryName)
    {
    DgnCategoryId catid = DgnCategory::QueryCategoryId(categoryName.c_str(), m_model->GetDgnDb());
    return new JsDgnElement(*createPhysicalElement(*m_model, ecSqlClassName.c_str(), catid));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void JsUtils::ImportLibrary (Utf8StringCR libName)
    {
    T_HOST.GetScriptAdmin().ImportScriptLibrary(libName.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void JsUtils::ReportError (Utf8StringCR description)
    {
    T_HOST.GetScriptAdmin().HandleScriptError(DgnPlatformLib::Host::ScriptAdmin::ScriptErrorHandler::Category::ReportedByScript, description.c_str(), "");
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
