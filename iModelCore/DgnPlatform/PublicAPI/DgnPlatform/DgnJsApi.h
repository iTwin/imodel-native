/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnJsApi.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#ifndef _DGN_JS_API_H_
#define _DGN_JS_API_H_

#include <BeJavaScript/BeJavaScript.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/DgnModel.h>
#include <DgnPlatform/GeomJsApi.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// Needed by generated callbacks to construct instances of wrapper classes.
// @bsiclass                                                    Sam/Steve.Wilson    7/15
//=======================================================================================
struct RefCountedBaseWithCreate : public RefCounted <IRefCounted>
{
    template <typename T, typename... Arguments>
    static RefCountedPtr<T> Create (Arguments&&... arguments)
        {
        return new T (std::forward<Arguments> (arguments)...);
        };

    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsUtils : RefCountedBaseWithCreate // ***  NEEDS WORK: It should not be necessary to derived from RefCountedBase, since I suppress my constructor. This is a bug in BeJavaScript that should be fixed.
{
    static void ImportLibrary (Utf8StringCR libName);
    static void ReportError(Utf8StringCR description);
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnElement : RefCountedBaseWithCreate
{
    DgnElementPtr m_el;

    JsDgnElement(DgnElementR el) : m_el(&el) {;}

    Utf8String GetElementId() {return Utf8PrintfString("%lld", m_el.IsValid()? (long long)m_el->GetElementId().GetValueUnchecked(): -1);}
    int32_t Insert() {return m_el.IsValid()? m_el->Insert().IsValid()? 0: -1: -2;}
    int32_t Update() {return m_el.IsValid()? m_el->Update().IsValid()? 0: -1: -2;}
    void SetParent(JsDgnElement* parent) {if (m_el.IsValid() && (nullptr != parent)) m_el->SetParentId(parent->m_el->GetElementId());}
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
    void DeleteAllElements();
};

typedef JsDgnModel* JsDgnModelP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsElementGeometryBuilder : RefCountedBaseWithCreate
{
    ElementGeometryBuilderPtr m_builder;

    JsElementGeometryBuilder(DgnElement3dR el, DPoint3dCR o, YawPitchRollAnglesCR angles) : m_builder (ElementGeometryBuilder::Create(el, o, angles)) {}
    JsElementGeometryBuilder(DgnElement2dR el, DPoint2dCR o, AngleInDegrees angle) : m_builder (ElementGeometryBuilder::Create(el, o, angle)) {}
    JsElementGeometryBuilder(JsDgnElementP el, JsDPoint3dP o, JsYawPitchRollAnglesP angles);
    ~JsElementGeometryBuilder() {}

    void AppendBox(double x, double y, double z);
    void AppendSphere(double radius);
    BentleyStatus SetGeomStreamAndPlacement (JsDgnElementP el) {return m_builder->SetGeomStreamAndPlacement(*el->m_el->ToGeometricElementP());}
};
typedef JsElementGeometryBuilder* JsElementGeometryBuilderP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct DgnJsApi : DgnPlatformLib::Host::ScriptAdmin::ScriptLibraryImporter
{
    BeJsContextR m_context;

    DgnJsApi(BeJsContextR);
    ~DgnJsApi();

    void _ImportScriptLibrary(BeJsContextR, Utf8CP) override;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _DGN_JS_API_H_

