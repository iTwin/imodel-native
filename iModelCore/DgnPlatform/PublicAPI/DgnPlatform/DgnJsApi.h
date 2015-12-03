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
#include <Logging/bentleylogging.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct JsDgnModel;
typedef JsDgnModel* JsDgnModelP;

struct JsDgnModels;
typedef JsDgnModels* JsDgnModelsP;

struct JsComponentModel;
typedef JsComponentModel* JsComponentModelP;

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
//! Logging Severity 
// *** TRICKY: Note that BEJAVASCRIPT_EXPORT_CLASS must be followed by the Typescript module name in parentheses.
//              That is, even though this enum is defined in the BentleyApi::Dgn, you mus tell the TS API generator 
//              explicitly that this enum should be defined in the 'BentleyApi.Dgn' module.
//
// @bsiclass                                                    Sam.Wilson      10/15
//=======================================================================================
BEJAVASCRIPT_EXPORT_CLASS (Bentley.Dgn)
enum class LoggingSeverity : uint32_t
    {
    // *** WARNING: Keep this consistent with DgnPlatformLib::ScriptAdmin::LoggingSeverity
    Fatal   = 0, 
    Error   = 1, 
    Warning = 2, 
    Info    = 3, 
    Debug   = 4, 
    Trace   = 5
    };

//=======================================================================================
//! Access to the message log
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct Logging : RefCountedBaseWithCreate // ***  NEEDS WORK: It should not be necessary to derive from RefCountedBase, since I suppress my constructor. This is a bug in BeJavaScript that should be fixed.
    {
    //! Set the severity level for the specified category
    //! @param category     The logging category
    //! @param severity     The minimum severity to display. Note that messages will not be logged if their severity is below this level.
    static void SetSeverity(Utf8StringCR category, LoggingSeverity severity);

    //! Test if the specified severity level is enabled for the specified category
    //! @param category     The logging category
    //! @param severity     The severity of the message. Note that the message will not be logged if 'severity' is below the current logging severity level
    static bool IsSeverityEnabled(Utf8StringCR category, LoggingSeverity severity);

    //! Send a message to the log
    //! @param category     The logging category
    //! @param severity     The severity of the message. Note that the message will not be logged if \a severity is below the severity level set by calling SetSeverity
    //! @param message      The message to log
    static void Message(Utf8StringCR category, LoggingSeverity severity, Utf8StringCR message);
    };

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct Script : RefCountedBaseWithCreate // ***  NEEDS WORK: It should not be necessary to derive from RefCountedBase, since I suppress my constructor. This is a bug in BeJavaScript that should be fixed.
{
    //! Make sure the that specified library is loaded
    //! @param libName  The name of the library that is to be loaded
    static void ImportLibrary (Utf8StringCR libName);

    //! Report an error. An error is more than a message. The platform is will treat it as an error. For example, the platform may terminate the current command.
    //! @param description  A description of the error
    static void ReportError(Utf8StringCR description);
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnObjectId : RefCountedBaseWithCreate
{
    uint64_t m_id;
    JsDgnObjectId() : m_id(0) {;}
    explicit JsDgnObjectId(uint64_t v) : m_id(v) {;}
};

typedef JsDgnObjectId* JsDgnObjectIdP;


//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsAuthorityIssuedCode : RefCountedBaseWithCreate
{
    AuthorityIssuedCode m_code;
    explicit JsAuthorityIssuedCode(AuthorityIssuedCode const& c) : m_code(c) {;}

    Utf8String GetValue() const {return m_code.GetValue();}
    void SetValue(Utf8StringCR) {BeAssert(false);} // *** WIP_SCRIPT - this should be a read-only property
    Utf8String GetNamespace() const {return m_code.GetNamespace();}
    void SetNamespace(Utf8StringCR) {BeAssert(false);} // *** WIP_SCRIPT - this should be a read-only property
    JsDgnObjectIdP GetAuthority() {return new JsDgnObjectId(m_code.GetAuthority().GetValueUnchecked());}
    void SetAuthority(JsDgnObjectIdP) {BeAssert(false);} // *** WIP_SCRIPT - this should be a read-only property
};

typedef JsAuthorityIssuedCode* JsAuthorityIssuedCodeP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnDb : RefCountedBaseWithCreate
{
    DgnDbPtr m_db;
    explicit JsDgnDb(DgnDbR db) : m_db(&db) {;}

    JsDgnModelsP GetModels();
    void SetModels(JsDgnModelsP) {BeAssert(false);} // *** WIP_SCRIPT - this should be a read-only property
};

typedef JsDgnDb* JsDgnDbP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnElement : RefCountedBaseWithCreate
{
    DgnElementPtr m_el;

    JsDgnElement(DgnElementR el) : m_el(&el) {;}

    JsDgnObjectIdP GetElementId() {return new JsDgnObjectId(m_el->GetElementId().GetValueUnchecked());}
    void SetElementId(JsDgnObjectIdP) {BeAssert(false);} // *** WIP_SCRIPT - this should be a read-only property
    JsAuthorityIssuedCodeP GetCode() const {return new JsAuthorityIssuedCode(m_el->GetCode());}
    void SetCode(JsAuthorityIssuedCodeP) {BeAssert(false);} // *** WIP_SCRIPT - this should be a read-only property
    JsDgnModelP GetModel();
    void SetModel(JsDgnModelP) {BeAssert(false);} // *** WIP_SCRIPT - this should be a read-only property
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

    ComponentModel* ToDgnComponentModel() {return dynamic_cast<ComponentModel*>(m_model.get());}

    JsDgnModel(DgnModelR m) : m_model(&m) {;}

    JsDgnObjectIdP GetModelId() {return new JsDgnObjectId(m_model->GetModelId().GetValueUnchecked());}
    void SetModelId(JsDgnObjectIdP) {BeAssert(false);} // *** WIP_SCRIPT - this should be a read-only property
    JsAuthorityIssuedCodeP GetCode() const {return new JsAuthorityIssuedCode(m_model->GetCode());}
    void SetCode(JsAuthorityIssuedCodeP) {BeAssert(false);} // *** WIP_SCRIPT - this should be a read-only property
    JsDgnDbP GetDgnDb() {return new JsDgnDb(m_model->GetDgnDb());}
    void SetDgnDb(JsDgnDbP) {BeAssert(false);} // *** WIP_SCRIPT - this should be a read-only property
    JsDgnElement* CreateElement(Utf8StringCR elType, Utf8StringCR categoryName);
    void DeleteAllElements();
    static JsAuthorityIssuedCodeP CreateModelCode(Utf8StringCR name) {return new JsAuthorityIssuedCode(DgnModel::CreateModelCode(name));}

    JsComponentModelP ToComponentModel();
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsPlacement3d : RefCountedBaseWithCreate
{
    Placement3d m_placement;
    JsPlacement3d() {;}
    JsPlacement3d(JsDPoint3dP origin, JsYawPitchRollAnglesP angles) : m_placement(origin? origin->Get(): DPoint3d::FromZero(), angles? angles->GetYawPitchRollAngles(): YawPitchRollAngles(), ElementAlignedBox3d()) {;}
    JsPlacement3d(Placement3d const& p) : m_placement(p) {;}

    JsDPoint3dP GetOrigin() const {return new JsDPoint3d(m_placement.GetOrigin());}
    void SetOrigin(JsDPoint3dP p) {m_placement.GetOriginR() = p->Get();}
    JsYawPitchRollAnglesP GetAngles() const {return new JsYawPitchRollAngles(m_placement.GetAngles());}
    void SetAngles(JsYawPitchRollAnglesP p) {m_placement.GetAnglesR() = p->GetYawPitchRollAngles();}
};

typedef JsPlacement3d* JsPlacement3dP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsComponentModel : JsDgnModel
{
    JsComponentModel(ComponentModel& m) : JsDgnModel(m) {;}

    Utf8String GetName() {return m_model.IsValid()? m_model->GetCode().GetValue(): "";}
    void SetName(Utf8StringCR) {BeAssert(false);} // *** WIP_SCRIPT - this should be a read-only property
    JsDgnElement* MakeInstanceOfSolution(JsDgnModelP targetModel, Utf8StringCR capturedSolutionName, Utf8StringCR paramsJSON, JsPlacement3dP placement, JsAuthorityIssuedCodeP code);
    void DeleteAllElements();
    static JsComponentModelP FindModelByName(JsDgnDbP db, Utf8StringCR name) {auto cm = ComponentModel::FindModelByName(*db->m_db, name); return cm.IsValid()? new JsComponentModel(*cm): nullptr; }
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnModels : RefCountedBaseWithCreate
{
    DgnModels& m_models;

    JsDgnModels(DgnModels& m) : m_models(m) {;}

    JsDgnObjectIdP QueryModelId(JsAuthorityIssuedCodeP code) {return new JsDgnObjectId(m_models.QueryModelId(code->m_code).GetValueUnchecked());}
    JsDgnModelP GetModel(JsDgnObjectIdP mid) {auto model = m_models.GetModel(DgnModelId(mid->m_id)); return model.IsValid()? new JsDgnModel(*model): nullptr;}
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsElementGeometryBuilder : RefCountedBaseWithCreate
{
    ElementGeometryBuilderPtr m_builder;

    //JsElementGeometryBuilder(DgnElement3dR el, DPoint3dCR o, YawPitchRollAnglesCR angles) : m_builder (ElementGeometryBuilder::Create(el, o, angles)) {} // Not exposed to JS
    //JsElementGeometryBuilder(DgnElement2dR el, DPoint2dCR o, AngleInDegrees angle) : m_builder (ElementGeometryBuilder::Create(el, o, angle)) {} // Not exposed to JS
    JsElementGeometryBuilder(JsDgnElementP el, JsDPoint3dP o, JsYawPitchRollAnglesP angles);
    ~JsElementGeometryBuilder() {}

    void AppendBox(double x, double y, double z);
    void AppendSphere(double radius);
    BentleyStatus SetGeomStreamAndPlacement (JsDgnElementP el) {return m_builder->SetGeomStreamAndPlacement(*el->m_el->ToGeometrySourceP());}
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

