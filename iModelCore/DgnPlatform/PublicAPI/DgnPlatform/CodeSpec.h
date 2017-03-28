/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/CodeSpec.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDomain.h"

//-----------------------------------------------------------------------------------------
// Names of built-in CodeSpecs. The best practice is to include the domain name as part of the CodeSpec name to ensure global uniqueness
//-----------------------------------------------------------------------------------------
#define BIS_CODESPEC(name)                          "bis:" name
#define BIS_CODESPEC_NullCodeSpec                   BIS_CODESPEC("NullCodeSpec")
#define BIS_CODESPEC_AnnotationFrameStyle           BIS_CODESPEC(BIS_CLASS_AnnotationFrameStyle)
#define BIS_CODESPEC_AnnotationLeaderStyle          BIS_CODESPEC(BIS_CLASS_AnnotationLeaderStyle)
#define BIS_CODESPEC_AnnotationTextStyle            BIS_CODESPEC(BIS_CLASS_AnnotationTextStyle)
#define BIS_CODESPEC_AuxCoordSystem2d               BIS_CODESPEC(BIS_CLASS_AuxCoordSystem2d)
#define BIS_CODESPEC_AuxCoordSystem3d               BIS_CODESPEC(BIS_CLASS_AuxCoordSystem3d)
#define BIS_CODESPEC_CategorySelector               BIS_CODESPEC(BIS_CLASS_CategorySelector)
#define BIS_CODESPEC_DisplayStyle                   BIS_CODESPEC(BIS_CLASS_DisplayStyle)
#define BIS_CODESPEC_Drawing                        BIS_CODESPEC(BIS_CLASS_Drawing)
#define BIS_CODESPEC_DrawingCategory                BIS_CODESPEC(BIS_CLASS_DrawingCategory)
#define BIS_CODESPEC_GeometryPart                   BIS_CODESPEC(BIS_CLASS_GeometryPart)
#define BIS_CODESPEC_GraphicalType2d                BIS_CODESPEC(BIS_CLASS_GraphicalType2d)
#define BIS_CODESPEC_TemplateRecipe2d               BIS_CODESPEC(BIS_CLASS_TemplateRecipe2d)
#define BIS_CODESPEC_LineStyle                      BIS_CODESPEC(BIS_CLASS_LineStyle)
#define BIS_CODESPEC_LinkElement                    BIS_CODESPEC(BIS_CLASS_LinkElement)
#define BIS_CODESPEC_MaterialElement                BIS_CODESPEC(BIS_CLASS_MaterialElement)
#define BIS_CODESPEC_ModelSelector                  BIS_CODESPEC(BIS_CLASS_ModelSelector)
#define BIS_CODESPEC_PhysicalType                   BIS_CODESPEC(BIS_CLASS_PhysicalType)
#define BIS_CODESPEC_InformationPartitionElement    BIS_CODESPEC(BIS_CLASS_InformationPartitionElement)
#define BIS_CODESPEC_Sheet                          BIS_CODESPEC(BIS_CLASS_Sheet)
#define BIS_CODESPEC_SpatialCategory                BIS_CODESPEC(BIS_CLASS_SpatialCategory)
#define BIS_CODESPEC_SpatialLocationType            BIS_CODESPEC(BIS_CLASS_SpatialLocationType)
#define BIS_CODESPEC_SubCategory                    BIS_CODESPEC(BIS_CLASS_SubCategory)
#define BIS_CODESPEC_Subject                        BIS_CODESPEC(BIS_CLASS_Subject)
#define BIS_CODESPEC_TemplateRecipe3d               BIS_CODESPEC(BIS_CLASS_TemplateRecipe3d)
#define BIS_CODESPEC_TextAnnotationSeed             BIS_CODESPEC(BIS_CLASS_TextAnnotationSeed)
#define BIS_CODESPEC_Texture                        BIS_CODESPEC(BIS_CLASS_Texture)
#define BIS_CODESPEC_TrueColor                      BIS_CODESPEC(BIS_CLASS_TrueColor)
#define BIS_CODESPEC_ViewDefinition                 BIS_CODESPEC(BIS_CLASS_ViewDefinition)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! A CodeFragmentSpec defines the rules for generating and validating a single part of the overall code.
//! @see CodeSpec
// @bsistruct                                                    Shaun.Sewall    01/17
//=======================================================================================
struct CodeFragmentSpec : Json::Value
{
private:
    static constexpr Utf8CP str_fixedString() {return "fixedString";}
    static constexpr Utf8CP str_inSequenceMask() {return "inSequenceMask";}
    static constexpr Utf8CP str_maxChars() {return "maxChars";}
    static constexpr Utf8CP str_minChars() {return "minChars";}
    static constexpr Utf8CP str_numberGap() {return "numberGap";}
    static constexpr Utf8CP str_prompt() {return "prompt";}
    static constexpr Utf8CP str_propertyName() {return "propertyName";}
    static constexpr Utf8CP str_startNumber() {return "startNumber";}
    static constexpr Utf8CP str_type() {return "type";}

    JsonValueCR GetValue(Utf8CP key) const {return (*this)[key];}
    void SetOrRemoveString(Utf8CP key, Utf8CP value) {if (value && *value) (*this)[key] = value; else removeMember(key);}

public:
    static constexpr int MAX_MaxChars = 256; //!< Maximum value for MaxChars

    enum class Type
    {
        Invalid = 0,
        FixedString = 1,
        ElementTypeCode = 2,
        Sequence = 3,
        PropertyValue = 4,
        // WIP: add RelationshipPath
    };

public:
    CodeFragmentSpec(Type type=Type::Invalid) {SetType(type);}

    static CodeFragmentSpec FromFixedString(Utf8CP fixedString, Utf8CP prompt=nullptr, bool inSequenceMask=true)
        {
        if (!fixedString || !*fixedString)
            return CodeFragmentSpec();

        CodeFragmentSpec fragmentSpec(Type::FixedString);
        fragmentSpec.SetFixedString(fixedString);
        fragmentSpec.SetMinChars(static_cast<uint32_t>(strlen(fixedString)));
        fragmentSpec.SetMaxChars(static_cast<uint32_t>(strlen(fixedString)));
        fragmentSpec.SetPrompt(prompt);
        fragmentSpec.SetInSequenceMask(inSequenceMask);
        return fragmentSpec;
        }

    static CodeFragmentSpec FromElementTypeCode(Utf8CP prompt=nullptr, bool inSequenceMask=true)
        {
        CodeFragmentSpec fragmentSpec(Type::ElementTypeCode);
        fragmentSpec.SetPrompt(prompt);
        fragmentSpec.SetInSequenceMask(inSequenceMask);
        return fragmentSpec;
        }

    static CodeFragmentSpec FromSequence(Utf8CP prompt=nullptr)
        {
        CodeFragmentSpec fragmentSpec(Type::Sequence);
        fragmentSpec.SetPrompt(prompt);
        fragmentSpec.SetInSequenceMask(false); // by definition, the sequence is not part of the sequence mask
        return fragmentSpec;
        }

    static CodeFragmentSpec FromPropertyValue(Utf8CP propertyName, Utf8CP prompt=nullptr, bool inSequenceMask=true)
        {
        if (!propertyName || !*propertyName)
            return CodeFragmentSpec();

        CodeFragmentSpec fragmentSpec(Type::PropertyValue);
        fragmentSpec.SetPropertyName(propertyName);
        fragmentSpec.SetPrompt(prompt);
        fragmentSpec.SetInSequenceMask(inSequenceMask);
        return fragmentSpec;
        }

    Type GetType() const {return (Type) GetValue(str_type()).asInt((int) Type::Invalid);}
    void SetType(Type type) {SetOrRemoveInt(str_type(), (int) type, (int) Type::Invalid);}
    bool IsValid() const {return Type::Invalid != GetType();}
    bool IsFixedString() const {return Type::FixedString == GetType();}
    bool IsElementTypeCode() const {return Type::ElementTypeCode == GetType();}
    bool IsSequence() const {return Type::Sequence == GetType();}
    bool IsPropertyValue() const {return Type::PropertyValue == GetType();}

    Utf8String GetPrompt() const {return GetValue(str_prompt()).asString();}
    void SetPrompt(Utf8CP prompt) {SetOrRemoveString(str_prompt(), prompt);}

    bool IsInSequenceMask() const {return GetValue(str_inSequenceMask()).asBool(!IsSequence());}
    void SetInSequenceMask(bool inSequenceMask) {if (!IsSequence()) SetOrRemoveBool(str_inSequenceMask(), inSequenceMask, true);}

    uint32_t GetMinChars() const {return GetValue(str_minChars()).asUInt(0);} 
    void SetMinChars(uint32_t minChars) {if (minChars <= GetMaxChars()) SetOrRemoveUInt(str_minChars(), minChars, 0);}

    uint32_t GetMaxChars() const {return GetValue(str_maxChars()).asUInt(MAX_MaxChars);} 
    void SetMaxChars(uint32_t maxChars) {if ((maxChars <= MAX_MaxChars) && (maxChars >= GetMinChars())) SetOrRemoveUInt(str_maxChars(), maxChars, MAX_MaxChars);}

    Utf8String GetFixedString() const {return IsFixedString() ? GetValue(str_fixedString()).asString() : "";}
    void SetFixedString(Utf8CP fixedString) {if (IsFixedString()) SetOrRemoveString(str_fixedString(), fixedString);}

    Utf8String GetPropertyName() const {return IsPropertyValue() ? GetValue(str_propertyName()).asString() : "";}
    void SetPropertyName(Utf8CP propertyName) {if (IsPropertyValue()) SetOrRemoveString(str_propertyName(), propertyName);}

    uint32_t GetStartNumber() const {return GetValue(str_startNumber()).asInt(1);}
    void SetStartNumber(uint32_t startNumber) {if (IsSequence()) SetOrRemoveUInt(str_startNumber(), startNumber, 1);}

    uint32_t GetNumberGap() const {return GetValue(str_numberGap()).asInt(1);}
    void SetNumberGap(uint32_t numberGap) {if (IsSequence()) SetOrRemoveUInt(str_numberGap(), numberGap, 1);}
};

//=======================================================================================
// @bsistruct                                                    Shaun.Sewall    01/17
//=======================================================================================
struct CodeScopeSpec : Json::Value
{
    friend struct CodeSpec;

public:
    enum class Type
    {
        Repository = 1,
        Model = 2,
        ParentElement = 3,
        // WIP: ArbitraryElement = 4 (identified by relationship)
    };

private:
    static constexpr Utf8CP str_type() {return "type";}
    CodeScopeSpec(Type type=Type::Repository) {SetType(type);}
    void SetType(Type type) {(*this)[str_type()] = (int) type;}
    JsonValueCR GetValue(Utf8CP key) const {return (*this)[key];}

public:
    Type GetType() const {return (Type) GetValue(str_type()).asInt((int) Type::Repository);}
    static CodeScopeSpec CreateRepositoryScope() {return CodeScopeSpec(Type::Repository);}
    static CodeScopeSpec CreateModelScope() {return CodeScopeSpec(Type::Model);}
    static CodeScopeSpec CreateParentElementScope() {return CodeScopeSpec(Type::ParentElement);}
};

typedef bvector<CodeFragmentSpec> CodeFragmentSpecList;
typedef CodeFragmentSpecList& CodeFragmentSpecListR;
typedef CodeFragmentSpecList const& CodeFragmentSpecListCR;
typedef bvector<Utf8String> CodeFragmentStringList;
typedef CodeFragmentStringList const& CodeFragmentStringListCR;

//=======================================================================================
//! A CodeSpec captures the rules for encoding and decoding significant business information into and from a Code (string). 
//! A CodeSpec determines how DgnCodes for elements are generated and validated.
//! There are 2 general types of codes:
//!   - User/Application-supplied: The user/application supplies a DgnCode and the CodeSpec
//!     simply enforces any constraints on e.g. allowable characters
//!   - Generated: The CodeSpec defines a CodeFragmentSpec array to indicate how to build up the overall CodeValue.
//! @note A CodeSpec is DgnDb-specific. That is, CodeSpec names are constant, but their CodeSpecIds may vary per DgnDb.
// @bsistruct                                                    Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CodeSpec : RefCountedBase
{
    friend struct DgnCodeSpecs;
    friend struct dgn_CodeSpecHandler::CodeSpec;

public:
    struct CreateParams
    {
        DgnDbR          m_dgndb;
        CodeSpecId      m_id;
        Utf8String      m_name;
        CodeScopeSpec   m_scopeSpec;

        CreateParams(DgnDbR dgndb, Utf8CP name, CodeSpecId id=CodeSpecId(), CodeScopeSpecCR scopeSpec=CodeScopeSpec()) :
            m_dgndb(dgndb), m_id(id), m_name(name), m_scopeSpec(scopeSpec) {}
    };

private:
    static constexpr Utf8CP str_fragmentSpecs() {return "fragmentSpecs";}
    static constexpr Utf8CP str_registrySuffix() {return "registrySuffix";}
    static constexpr Utf8CP str_scopeSpec() {return "scopeSpec";}
    static constexpr Utf8CP str_spec() {return "spec";}
    static constexpr Utf8CP str_version() {return "version";}

private:
    DgnDbR          m_dgndb;
    CodeSpecId      m_codeSpecId;
    Utf8String      m_name;

    Json::Value m_specProperties;
    CodeScopeSpec m_scopeSpec;
    CodeFragmentSpecList m_fragmentSpecs;
    
    DGNPLATFORM_EXPORT explicit CodeSpec(CreateParams const&);

    void ReadProperties(Utf8StringCR jsonStr);
    Utf8String SerializeProperties() const;

    void ToPropertiesJson(JsonValueR) const;
    void FromPropertiesJson(JsonValueCR);
    CodeSpecPtr CloneForImport(DgnDbStatus* status, DgnImportContext& importer) const;

    static DgnDbStatus Insert(DgnDbR db, Utf8CP name, CodeScopeSpecCR scope, CodeSpecId codeSpecId=CodeSpecId());

public:
    DgnDbR GetDgnDb() const { return m_dgndb; }
    CodeSpecId GetCodeSpecId() const { return m_codeSpecId; }
    Utf8StringCR GetName() const { return m_name; }

    Utf8String GetRegistrySuffix() const {return m_specProperties[str_registrySuffix()].asString();}
    void SetRegistrySuffix(Utf8CP registrySuffix) {if (registrySuffix && *registrySuffix) m_specProperties[str_registrySuffix()] = registrySuffix;}

    //! Return the CodeSpecId of the NullCodeSpec
    static CodeSpecId GetNullCodeSpecId() {return CodeSpecId((uint64_t)1LL);}
    //! Return true if the specified CodeSpec is the NullCodeSpec
    bool IsNullCodeSpec() const {return GetCodeSpecId() == GetNullCodeSpecId();}

    DGNPLATFORM_EXPORT DgnDbStatus Insert();

    DGNPLATFORM_EXPORT static CodeSpecPtr Create(DgnDbR db, Utf8CP name, CodeScopeSpecCR scopeSpec=CodeScopeSpec::CreateRepositoryScope());

    CodeScopeSpecCR GetScope() const {return m_scopeSpec;}
    void SetScope(CodeScopeSpecCR scopeSpec) {m_scopeSpec = scopeSpec;}
    bool IsRepositoryScope() const {return CodeScopeSpec::Type::Repository == GetScope().GetType();}
    bool IsModelScope() const {return CodeScopeSpec::Type::Model == GetScope().GetType();}
    bool IsParentElementScope() const {return CodeScopeSpec::Type::ParentElement == GetScope().GetType();}
    //! Return the DgnElementId of the scope element for the specified element.
    DGNPLATFORM_EXPORT DgnElementId GetScopeElementId(DgnElementCR element) const;

    CodeFragmentSpecListCR GetFragmentSpecs() const {return m_fragmentSpecs;}
    CodeFragmentSpecListR GetFragmentSpecsR() {return m_fragmentSpecs;}
    bool CanGenerateCode() const {return m_fragmentSpecs.size() > 0;}

    DGNPLATFORM_EXPORT static DgnCode CreateCode(Utf8CP codeSpecName, DgnElementCR scopeElement, Utf8StringCR value);
    DGNPLATFORM_EXPORT DgnCode CreateCode(DgnElementCR scopeElement, Utf8StringCR value) const;
    DGNPLATFORM_EXPORT static DgnCode CreateCode(DgnDbR db, Utf8CP codeSpecName, Utf8StringCR value);
    DGNPLATFORM_EXPORT DgnCode CreateCode(Utf8StringCR value) const;
    DGNPLATFORM_EXPORT static DgnCode CreateCode(DgnDbR db, Utf8CP codeSpecName, Utf8StringCR value, Utf8StringCR nameSpace); // WIP: Deprecate?
    DgnCode CreateCode(Utf8StringCR value, Utf8StringCR nameSpace) const { return DgnCode(m_codeSpecId, value, nameSpace); } // WIP: Deprecate?

    DGNPLATFORM_EXPORT DgnDbStatus ValidateCode(DgnElementCR) const;
    DGNPLATFORM_EXPORT DgnDbStatus CloneCodeForImport(DgnCodeR newCode, DgnElementCR srcElem, DgnModelR destModel, DgnImportContext& importer) const;

    DGNPLATFORM_EXPORT static CodeSpecPtr Import(DgnDbStatus* status, CodeSpecCR sourceCodeSpec, DgnImportContext& importer);
};

//=======================================================================================
// Macro used for declaring the members of a CodeSpec handler
//=======================================================================================
#define AUTHORITYHANDLER_DECLARE_MEMBERS(__ECClassName__,__classname__,_handlerclass__,_handlersuperclass__,__exporter__) \
    private: Dgn::CodeSpecP _CreateInstance(Dgn::CodeSpec::CreateParams const& params) override {return new __classname__(__classname__::CreateParams(params));}\
        DOMAINHANDLER_DECLARE_MEMBERS(__ECClassName__,_handlerclass__,_handlersuperclass__,__exporter__)

//=======================================================================================
//! CodeSpec handlers
//=======================================================================================
namespace dgn_CodeSpecHandler
{
    struct EXPORT_VTABLE_ATTRIBUTE CodeSpec : DgnDomain::Handler
    {
        DOMAINHANDLER_DECLARE_MEMBERS (BIS_CLASS_CodeSpec, CodeSpec, DgnDomain::Handler, DGNPLATFORM_EXPORT)

    protected:
        CodeSpecHandlerP _ToCodeSpecHandler() override { return this; }
        virtual CodeSpecP _CreateInstance(Dgn::CodeSpec::CreateParams const& params) { return new Dgn::CodeSpec(params); }

    public:
        CodeSpecPtr Create(Dgn::CodeSpec::CreateParams const& params) { return _CreateInstance(params); }
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
