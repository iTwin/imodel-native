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

BEGIN_BENTLEY_DGN_NAMESPACE

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

END_BENTLEY_DGN_NAMESPACE

#endif//ndef _DGN_JS_API_H_

