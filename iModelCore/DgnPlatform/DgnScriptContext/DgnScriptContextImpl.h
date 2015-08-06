/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScriptContext/DgnScriptContextImpl.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef _DGN_SCRIPT_CONTEXT_IMPL_H_
#define _DGN_SCRIPT_CONTEXT_IMPL_H_

#include <BeJavaScript/BeJavaScript.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnCore/DgnElement.h>
#include <DgnPlatform/DgnCore/DgnModel.h>
#include <Geom/DPoint3d.h>
#include <Geom/Angle.h>

extern Utf8CP dgnScriptContext_GetBootstrappingSource();

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Steve.Wilson    7/15
//=======================================================================================
struct RefCountedBaseWithCreate : public RefCountedBase
{
    template <typename T, typename... Arguments>
    static RefCountedPtr<T> Create (Arguments&&... arguments)
        {
        return new T (std::forward<Arguments> (arguments)...);
        };
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnElement : RefCountedBaseWithCreate
{
    DgnElementPtr m_el;

    JsDgnElement(DgnElementR el) : m_el(&el) {;}

    Utf8String GetElementId() {return Utf8PrintfString("%lld", (long long)m_el->GetElementId().GetValueUnchecked());}
};

typedef JsDgnElement* JsDgnElementP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnModel : RefCountedBaseWithCreate
{
    DgnModelPtr m_model;

    JsDgnModel(DgnModelR m) : m_model(&m) {;}

    Utf8String GetModelId() {return Utf8PrintfString("%lld", m_model->GetModelId().GetValueUnchecked());}
    JsDgnElement* CreateElement(Utf8StringCR elType, Utf8StringCR categoryName);
    void InsertElement(JsDgnElementP element);
    void DeleteAllElements();
};

typedef JsDgnModel* JsDgnModelP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDPoint3d : RefCountedBaseWithCreate
{
    DPoint3d m_pt;

    JsDPoint3d() {m_pt.Init(0,0,0);}
    JsDPoint3d(DPoint3dCR pt) : m_pt(pt) {;}
    JsDPoint3d(double x, double y, double z) {m_pt.x=x; m_pt.y=y; m_pt.z=z;}
    static JsDPoint3d* Create(double x, double y, double z) {return new JsDPoint3d(x,y,z);}

    double GetX() {return m_pt.x;}
    double GetY() {return m_pt.y;}
    double GetZ() {return m_pt.z;}
    void SetX(double v) {m_pt.x = v;}
    void SetY(double v) {m_pt.y = v;}
    void SetZ(double v) {m_pt.z = v;}
};

typedef JsDPoint3d* JsDPoint3dP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsYawPitchRollAngles : RefCountedBaseWithCreate
{
    YawPitchRollAngles m_angles;

    JsYawPitchRollAngles() {m_angles.FromDegrees(0,0,0);}
    JsYawPitchRollAngles(YawPitchRollAnglesCR angles) : m_angles(angles) {;}
    static JsYawPitchRollAngles* Create(double yaw, double pitch, double roll) {return new JsYawPitchRollAngles(YawPitchRollAngles::FromDegrees(yaw,pitch,roll));}

    double GetYaw  () {return m_angles.GetYaw().Degrees();}
    double GetPitch() {return m_angles.GetPitch().Degrees();}
    double GetRoll () {return m_angles.GetRoll().Degrees();}
    void SetYaw  (double v) {m_angles.FromDegrees(v, GetPitch(), GetRoll());}
    void SetPitch(double v) {m_angles.FromDegrees(GetYaw(), v, GetRoll());}
    void SetRoll (double v) {m_angles.FromDegrees(GetYaw(), GetPitch(), v);}
};

typedef JsYawPitchRollAngles* JsYawPitchRollAnglesP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsElementGeometryBuilder : RefCountedBaseWithCreate
{
    ElementGeometryBuilderPtr m_builder;

    JsElementGeometryBuilder(DgnElement3dR el, DPoint3dCR o, YawPitchRollAnglesCR angles) : m_builder (ElementGeometryBuilder::Create(el, o, angles)) {}
    JsElementGeometryBuilder(DgnElement2dR el, DPoint2dCR o, AngleInDegrees angle) : m_builder (ElementGeometryBuilder::Create(el, o, angle)) {}
    ~JsElementGeometryBuilder() {}
    static JsElementGeometryBuilder* Create(JsDgnElementP el, JsDPoint3dP o, JsYawPitchRollAnglesP angles);

    void AppendBox(double x, double y, double z);
    BentleyStatus SetGeomStreamAndPlacement (JsDgnElementP el) {return m_builder->SetGeomStreamAndPlacement(*el->m_el->ToGeometricElementP());}
};
typedef JsElementGeometryBuilder* JsElementGeometryBuilderP;

//=======================================================================================
// This class holds the one and only BeJsEnvironment and BeJsContext for the session.
// It defines all of the DgnScript Object Model proxies and registers them in Script.
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct DgnScriptContextImpl : BeJsContext
{
    // -------------------------------
    // Member variables
    // -------------------------------
    BeJsObject m_egaRegistry;
    BeJsObject m_modelSolverRegistry;
    bset<Utf8String> m_jsScriptsExecuted;

    // -------------------------------
    // Member functions
    // -------------------------------
    DgnScriptContextImpl(BeJsEnvironmentR);
    ~DgnScriptContextImpl();
    DgnDbStatus LoadProgram(Dgn::DgnDbR db, Utf8CP tsFunctionSpec);
    DgnDbStatus ExecuteEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms);
    DgnDbStatus ExecuteModelSolver(int& functionReturnStatus, Dgn::DgnModelR model, Utf8CP jsFunctionName, Json::Value const& parms);

    static BeJsContext::Projection* InitializeJsProjections (BeJsContext& context);

    static void DestroyJsProjections (BeJsContext& context, BeJsContext::Projection*);
};

END_BENTLEY_DGN_NAMESPACE

#endif//ndef _DGN_SCRIPT_CONTEXT_IMPL_H_

