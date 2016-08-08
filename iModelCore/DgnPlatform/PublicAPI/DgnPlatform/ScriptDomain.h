/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ScriptDomain.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "DgnElement.h"
#include "ElementHandler.h"
#include "DgnDomain.h"

#define SCRIPT_DOMAIN_ECSCHEMA_PATH             L"ECSchemas/Domain/Script.01.00.ecschema.xml"
#define SCRIPT_DOMAIN_NAME                      "SCRIPT"
#define SCRIPT_DOMAIN_SCHEMA(className)         SCRIPT_DOMAIN_NAME "." className

#define SCRIPT_DOMAIN_CLASSNAME_Script                  "Script"
#define SCRIPT_DOMAIN_CLASSNAME_Function                "Function"
#define SCRIPT_DOMAIN_CLASSNAME_ModelListFunction       "ModelListFunction"
#define SCRIPT_DOMAIN_CLASSNAME_ModelFilterFunction     "ModelFilterFunction"
#define SCRIPT_DOMAIN_CLASSNAME_ElementListFunction     "ElementListFunction"
#define SCRIPT_DOMAIN_CLASSNAME_ElementFilterFunction   "ElementFilterFunction"

#define SCRIPT_DOMAIN_PROPERTY_Script_Text              "Text"
#define SCRIPT_DOMAIN_PROPERTY_Script_EntryPoint        "EntryPoint"
#define SCRIPT_DOMAIN_PROPERTY_Script_EcmaScriptVersionRequired "EcmaScriptVersionRequired"

DGNPLATFORM_REF_COUNTED_PTR(ScriptDefinitionElement)

BEGIN_BENTLEY_DGN_NAMESPACE

struct ScriptDomain;
struct ScriptDefinitionElementHandler;

//=======================================================================================
//! Stores an ECMAScript script in a bim. Scripts can play many roles in a bim. Roles are clarified by subclasses in the ECSchema.
//! @note Call ScriptDomain::ImportSchema when setting up a bim to hold script elements.
//! @ingroup GROUP_DgnScript
// @bsiclass                                                    Sam.Wilson      07/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ScriptDefinitionElement : DefinitionElement
    {
    DEFINE_T_SUPER(DefinitionElement);
    DGNELEMENT_DECLARE_MEMBERS(SCRIPT_DOMAIN_CLASSNAME_Script, DefinitionElement)

public:
    friend struct ScriptDefinitionElementHandler;

private:
    explicit ScriptDefinitionElement(CreateParams const&);
    DGNPLATFORM_EXPORT static RefCountedPtr<ScriptDefinitionElement> CreateElement(DgnDbStatus*, DgnDbR, DgnModelId, Utf8CP className, Utf8CP text, Utf8CP entryPoint, Utf8CP esv);
    DGNPLATFORM_EXPORT DgnDbStatus _SetProperty(Utf8CP name, ECN::ECValueCR value) override;
public:
    //! Create an ElementFilterFunction. The returned element is non-persisent. The caller must call its Insert method in order to store it to the bim.
    //! @param[out] stat    Optional. An error status if the element could not be created. DgnDbStatus::MissingDomain if the Script domain has not been imported.
    //! @param[in] db       The DgnDb where the caller will eventually insert the new element.
    //! @param[in] modelId  The definition model that will eventually hold the new element.
    //! @param[in] text     The script
    //! @param[in] entryPoint The function in the script that should be called 
    //! @param[in] esv      The ECMAScript version required to run this script.
    //! @return a new non-persistent script element or an invalid ptr if the input is invalid
    static ScriptDefinitionElementPtr CreateElementFilterFunction(DgnDbStatus* stat, DgnDbR db, DgnModelId modelId, Utf8CP text, Utf8CP entryPoint, Utf8CP esv = "ES5")
        {return CreateElement(stat, db, modelId, SCRIPT_DOMAIN_CLASSNAME_ElementFilterFunction, text, entryPoint, esv);}

    //! Create an ElementListFunction. The returned element is non-persisent. The caller must call its Insert method in order to store it to the bim.
    //! @param[out] stat    Optional. An error status if the element could not be created. DgnDbStatus::MissingDomain if the Script domain has not been imported.
    //! @param[in] db       The DgnDb where the caller will eventually insert the new element.
    //! @param[in] modelId  The definition model that will eventually hold the new element.
    //! @param[in] text     The script
    //! @param[in] entryPoint The function in the script that should be called 
    //! @param[in] esv      The ECMAScript version required to run this script.
    //! @return a new non-persistent script element or an invalid ptr if the input is invalid
    static ScriptDefinitionElementPtr CreateElementListFunction(DgnDbStatus* stat, DgnDbR db, DgnModelId modelId, Utf8CP text, Utf8CP entryPoint, Utf8CP esv = "ES5")
        {return CreateElement(stat, db, modelId, SCRIPT_DOMAIN_CLASSNAME_ElementListFunction, text, entryPoint, esv);}

    //! Get the Script text
    DGNPLATFORM_EXPORT Utf8String GetText() const;

    //! Get the Script entry point
    DGNPLATFORM_EXPORT Utf8String GetEntryPoint() const;

    //! Get the EcmaScript version required to run this script
    DGNPLATFORM_EXPORT Utf8String GetEcmaScriptVersionRequired() const;

    //! Get the entry point signature
    //! @param[out] returnType  The return type of the script's entry point function
    //! @param[out] arguments   The TypeScript types of the arguments to the script's entry point function
    DGNPLATFORM_EXPORT void GetSignature(Utf8StringR returnType, Utf8StringR arguments) const;
    
    DGNPLATFORM_EXPORT DgnDbStatus LoadScript(bool forceReload = false) const;

    DGNPLATFORM_EXPORT DgnDbStatus Execute(Utf8String argTypeCdl, ...) const;
    };


//! @nodoc
struct ScriptDefinitionElementHandler : dgn_ElementHandler::Element
    {
    private:
    ELEMENTHANDLER_DECLARE_MEMBERS(SCRIPT_DOMAIN_CLASSNAME_Script, ScriptDefinitionElement, ScriptDefinitionElementHandler, dgn_ElementHandler::Element, )
    friend struct ScriptDomain;
    static void Register(DgnDomain& domain) { domain.RegisterHandler(GetHandler()); }
    };

//=======================================================================================
//! Domain for the SCRIPT_DOMAIN_CLASSNAME_Script ECSchema
//! @ingroup GROUP_DgnScript
// @bsiclass                                                    Sam.Wilson      07/2016
//=======================================================================================
struct ScriptDomain : DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(ScriptDomain, DGNPLATFORM_EXPORT)

        ScriptDomain() : DgnDomain(SCRIPT_DOMAIN_NAME, "Script Domain", 1)
            {
            ScriptDefinitionElementHandler::Register(*this);
            }

    public:
        static void Register()
            {
            DgnDomains::RegisterDomain(GetDomain());
            }

        //! Import the ECSchema for the ScriptDomain into the specified DgnDb
        DGNPLATFORM_EXPORT static DgnDbStatus ImportSchema(DgnDbR);
    };

END_BENTLEY_DGN_NAMESPACE
