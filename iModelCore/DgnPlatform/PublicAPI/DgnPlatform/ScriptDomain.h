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
#include "DgnModel.h"
#include "ElementHandler.h"
#include "DgnDomain.h"
#include "DgnPlatformLib.h"

#define SCRIPT_DOMAIN_ECSCHEMA_PATH             L"ECSchemas/Domain/Script.01.00.ecschema.xml"
#define SCRIPT_DOMAIN_NAME                      "SCRIPT"
#define SCRIPT_DOMAIN_SCHEMA(className)         SCRIPT_DOMAIN_NAME "." className

#define SCRIPT_DOMAIN_CLASSNAME_ScriptLibraryModel      "ScriptLibraryModel"

#define SCRIPT_DOMAIN_CLASSNAME_Script                  "Script"
#define SCRIPT_DOMAIN_CLASSNAME_PopulateModelList       "PopulateModelList"
#define SCRIPT_DOMAIN_CLASSNAME_FilterModel             "FilterModel"
#define SCRIPT_DOMAIN_CLASSNAME_PopulateElementList     "PopulateElementList"
#define SCRIPT_DOMAIN_CLASSNAME_FilterElement           "FilterElement"

#define SCRIPT_DOMAIN_PROPERTY_Script_Text              "Text"
#define SCRIPT_DOMAIN_PROPERTY_Script_EntryPoint        "EntryPoint"
#define SCRIPT_DOMAIN_PROPERTY_Script_EcmaScriptVersionRequired "EcmaScriptVersionRequired"
#define SCRIPT_DOMAIN_PROPERTY_Script_SourceUrl         "SourceUrl"
#define SCRIPT_DOMAIN_PROPERTY_Script_Description       "Description"

DGNPLATFORM_REF_COUNTED_PTR(ScriptLibraryModel)
DGNPLATFORM_REF_COUNTED_PTR(ScriptDefinitionElement)

BEGIN_BENTLEY_DGN_NAMESPACE

struct ScriptDomain;
struct ScriptDefinitionElementHandler;
struct ScriptLibraryModelHandler;

//=======================================================================================
//! A model which contains only scripts.
//! @ingroup GROUP_DgnScript
// @bsiclass                                                    Paul.Connelly   09/15
//=======================================================================================
struct ScriptLibraryModel : DefinitionModel
    {
    DGNMODEL_DECLARE_MEMBERS(SCRIPT_DOMAIN_CLASSNAME_ScriptLibraryModel, DefinitionModel);
    friend struct ScriptLibraryModelHandler;
    explicit ScriptLibraryModel(CreateParams const& params) : T_Super(params) {}

  public:
    DGNPLATFORM_EXPORT static ScriptLibraryModelPtr Create(DgnDbR, DgnCode, Utf8CP sourceUrl="");
    };

//! @private
struct ScriptLibraryModelHandler : dgn_ModelHandler::Definition
    {
    MODELHANDLER_DECLARE_MEMBERS(SCRIPT_DOMAIN_CLASSNAME_ScriptLibraryModel, ScriptLibraryModel, ScriptLibraryModelHandler, dgn_ModelHandler::Definition, )
    public:
    static void Register(DgnDomain& domain) { domain.RegisterHandler(GetHandler()); }
    };

//=======================================================================================
//! Stores an ECMAScript script in a bim. Scripts can play many roles in a bim. Roles are clarified by subclasses in the ECSchema.
//! Scripts may be stored only in a ScriptLibraryModel.
//! @note Call ScriptDomain::ImportSchema when setting up a bim to hold script elements.
//! @ingroup GROUP_DgnScript
// @bsiclass                                                    Sam.Wilson      07/2016
//=======================================================================================
struct ScriptDefinitionElement : DefinitionElement
    {
    DEFINE_T_SUPER(DefinitionElement);
    DGNELEMENT_DECLARE_MEMBERS(SCRIPT_DOMAIN_CLASSNAME_Script, DefinitionElement)

public:
    friend struct ScriptDefinitionElementHandler;

private:
    explicit ScriptDefinitionElement(CreateParams const&);
    DGNPLATFORM_EXPORT DgnDbStatus _SetProperty(Utf8CP name, ECN::ECValueCR value) override;
public:
    //! Create a script element. The returned element is non-persisent. The caller must call its Insert method in order to store it to the bim.
    //! @param[out] stat    Optional. An error status if the element could not be created. DgnDbStatus::MissingDomain if the Script domain has not been imported.
    //! @param[in] model    The ScriptLibraryModel model that will eventually hold the new script element.
    //! @param[in] className The name of the script element ECClass
    //! @param[in] text     The script
    //! @param[in] entryPoint Optional. The function in the script that should be called. Defaults to the last function defined in \a text.
    //! @param[in] desc     Optional. A description of the script.
    //! @param[in] url      Optional. The source URL.
    //! @param[in] esv      Optional. The ECMAScript version required to run this script. Defaults to "ES5".
    //! @return a new non-persistent script element or an invalid ptr if the input is invalid.
    DGNPLATFORM_EXPORT static ScriptDefinitionElementPtr Create(DgnDbStatus* stat, ScriptLibraryModel& model, 
        Utf8CP className, Utf8CP text, Utf8CP entryPoint = nullptr, Utf8CP desc = nullptr, Utf8CP url = nullptr, Utf8CP esv = "ES5");

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
    
    //! Get the description of the script
    DGNPLATFORM_EXPORT Utf8String GetDescription() const;

    //! Get the Script source URL
    DGNPLATFORM_EXPORT Utf8String GetSourceUrl() const;

    DGNPLATFORM_EXPORT DgnDbStatus LoadScript(bool forceReload = false) const;

    //! Holds the types of data that can be arguments to a script
    struct ArgValueUnion
        {
        enum class Type { Bool, ObjectId, Int32, Double, Utf8CP, Pointer };
        Type m_type;
        union 
            {
            void* m_ptr;
            BeInt64Id m_objectId;
            int32_t m_int32;
            double m_double;
            Utf8CP m_utf8cp;
            bool m_bool;
            };
        ArgValueUnion(std::nullptr_t ) : m_type(Type::Pointer), m_ptr(nullptr) { ; }
        ArgValueUnion(void const*p) : m_type(Type::Pointer), m_ptr((void*)p) { ; }
        ArgValueUnion(int32_t i) : m_type(Type::Int32), m_int32(i) { ; }
        ArgValueUnion(BeInt64Id const& id) : m_type(Type::ObjectId), m_objectId(id) { ; }
        ArgValueUnion(double v) : m_type(Type::Double), m_double(v) { ; }
        ArgValueUnion(Utf8CP s) : m_type(Type::Utf8CP), m_utf8cp(s) { ; }
        };
    
    //! Execute this script, passing it the supplied arguments.
    //! @param[out] result   A JSON-encoded object that captures the return value from the script. Will be empty if the script could not be executed
    //! @param[in] args     The arguments to the script's entry point function
    //! @return non-zero error status if any argument is bad or if the script is invalid or triggered and exception.
    DGNPLATFORM_EXPORT DgnDbStatus Execute(Utf8StringR result, std::initializer_list<ArgValueUnion> const& args) const;
    };


//! @nodoc
struct ScriptDefinitionElementHandler : dgn_ElementHandler::Definition
    {
    private:
    ELEMENTHANDLER_DECLARE_MEMBERS(SCRIPT_DOMAIN_CLASSNAME_Script, ScriptDefinitionElement, ScriptDefinitionElementHandler, dgn_ElementHandler::Definition, )
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
