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
#define BIS_CODESPEC_CategorySelector               BIS_CODESPEC(BIS_CLASS_CategorySelector)
#define BIS_CODESPEC_DisplayStyle                   BIS_CODESPEC(BIS_CLASS_DisplayStyle)
#define BIS_CODESPEC_Drawing                        BIS_CODESPEC(BIS_CLASS_Drawing)
#define BIS_CODESPEC_DrawingCategory                BIS_CODESPEC(BIS_CLASS_DrawingCategory)
#define BIS_CODESPEC_GeometryPart                   BIS_CODESPEC(BIS_CLASS_GeometryPart)
#define BIS_CODESPEC_LightDefinition                BIS_CODESPEC(BIS_CLASS_LightDefinition)
#define BIS_CODESPEC_LineStyle                      BIS_CODESPEC(BIS_CLASS_LineStyle)
#define BIS_CODESPEC_LinkElement                    BIS_CODESPEC(BIS_CLASS_LinkElement)
#define BIS_CODESPEC_MaterialElement                BIS_CODESPEC(BIS_CLASS_MaterialElement)
#define BIS_CODESPEC_ModelSelector                  BIS_CODESPEC(BIS_CLASS_ModelSelector)
#define BIS_CODESPEC_InformationPartitionElement    BIS_CODESPEC(BIS_CLASS_InformationPartitionElement)
#define BIS_CODESPEC_Session                        BIS_CODESPEC(BIS_CLASS_Session)
#define BIS_CODESPEC_Sheet                          BIS_CODESPEC(BIS_CLASS_Sheet)
#define BIS_CODESPEC_SpatialCategory                BIS_CODESPEC(BIS_CLASS_SpatialCategory)
#define BIS_CODESPEC_SubCategory                    BIS_CODESPEC(BIS_CLASS_SubCategory)
#define BIS_CODESPEC_Subject                        BIS_CODESPEC(BIS_CLASS_Subject)
#define BIS_CODESPEC_TextAnnotationSeed             BIS_CODESPEC(BIS_CLASS_TextAnnotationSeed)
#define BIS_CODESPEC_Texture                        BIS_CODESPEC(BIS_CLASS_Texture)
#define BIS_CODESPEC_TrueColor                      BIS_CODESPEC(BIS_CLASS_TrueColor)
#define BIS_CODESPEC_ViewDefinition                 BIS_CODESPEC(BIS_CLASS_ViewDefinition)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsistruct                                                    Shaun.Sewall    01/17
//=======================================================================================
struct CodeFragmentSpec
{
public:
    enum class Type : uint8_t
    {
        Invalid = 0,
        FixedString = 1,
        ElementTypeCode = 2,
        SequenceNumber = 3,
        PropertyValue = 4,
        // WIP: add RelationshipPath
    };

    static const int MIN_MinChars = 1;      //!< Minimum value for MinChars
    static const int MAX_MaxChars = 256;    //!< Maximum value for MaxChars

private:
    Type m_type = Type::Invalid;    //!< the fragment type
    Utf8String m_prompt;            //!< The prompt for when the fragment is directly input by user
    bool m_inSequenceMask = true;   //!< If true, include this CodeFragmentSpec when generating the sequence mask
    int m_minChars = MIN_MinChars;  //!< The minimum number of characters in the resulting fragment string
    int m_maxChars = MAX_MaxChars;  //!< The maximum number of characters in the resulting fragment string
    Utf8String m_param1;            //!< The first variable parameter associated with the fragment (dependent on type)
    Utf8String m_param2;            //!< The second variable parameter associated with the fragment (dependent on type)
    // WIP: format specification?

    CodeFragmentSpec() {}
    void SetType(Type type) {m_type = type;}
    void SetParam1(Utf8CP param) {m_param1 = param;}
    void SetParam2(Utf8CP param) {m_param2 = param;}

public:
    Type GetType() const {return m_type;}
    bool IsValid() const {return Type::Invalid != m_type;}
    Utf8String GetFixedString() const {return (Type::FixedString == GetType()) ? m_param1 : "";}
    Utf8String GetPropertyName() const {return (Type::PropertyValue == GetType()) ? m_param1 : "";}

    Utf8String GetPrompt() const {return m_prompt;}
    void SetPrompt(Utf8CP prompt) {m_prompt.AssignOrClear(prompt);}

    bool IsInSequenceMask() const {return m_inSequenceMask;}
    void SetInSequenceMask(bool inSequenceMask) {m_inSequenceMask = inSequenceMask;}

    int GetMinChars() const {return m_minChars;}
    int GetMaxChars() const {return m_maxChars;}
    void SetMinChars(int minChars) {if ((minChars >= MIN_MinChars) && (minChars <= m_maxChars)) m_minChars = minChars;}
    void SetMaxChars(int maxChars) {if ((maxChars <= MAX_MaxChars) && (maxChars >= m_minChars)) m_maxChars = maxChars;}

    DGNPLATFORM_EXPORT Json::Value ToJson() const;
    DGNPLATFORM_EXPORT static CodeFragmentSpec FromJson(JsonValueCR);
    DGNPLATFORM_EXPORT static CodeFragmentSpec FromFixedString(Utf8CP fixedString, Utf8CP prompt=nullptr, bool inSequenceMask=true);
    DGNPLATFORM_EXPORT static CodeFragmentSpec FromElementTypeCode(Utf8CP prompt=nullptr, bool inSequenceMask=true);
    DGNPLATFORM_EXPORT static CodeFragmentSpec FromSequenceNumber(Utf8CP prompt=nullptr);
    DGNPLATFORM_EXPORT static CodeFragmentSpec FromPropertyValue(Utf8CP propertyName, Utf8CP prompt=nullptr, bool inSequenceMask=true);
};

//=======================================================================================
// @bsistruct                                                    Shaun.Sewall    01/17
//=======================================================================================
struct CodeScopeSpec
{
    friend struct CodeSpec;

public:
    enum class Type : uint8_t
    {
        Repository = 1,
        Model = 2,
        ParentElement = 3,
        // WIP: ArbitraryElement = 4 (identified by relationship)
    };

private:
    Type m_type = Type::Repository;
    CodeScopeSpec() {}
    explicit CodeScopeSpec(Type type) : m_type(type) {}

public:
    Type GetType() const {return m_type;}
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
public:
    struct CreateParams
    {
        DgnDbR          m_dgndb;
        CodeSpecId      m_id;
        DgnClassId      m_classId;
        Utf8String      m_name;
        CodeScopeSpec   m_scopeSpec;

        CreateParams(DgnDbR dgndb, DgnClassId classId, Utf8CP name, CodeSpecId id=CodeSpecId(), CodeScopeSpecCR scopeSpec=CodeScopeSpec()) :
            m_dgndb(dgndb), m_id(id), m_classId(classId), m_name(name), m_scopeSpec(scopeSpec) {}
    };

private:
    friend struct DgnCodeSpecs;
    friend struct dgn_CodeSpecHandler::CodeSpec;

    DgnDbR          m_dgndb;
    CodeSpecId      m_codeSpecId;
    DgnClassId      m_classId;
    Utf8String      m_name;

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
    DgnClassId GetClassId() const { return m_classId; }
    Utf8StringCR GetName() const { return m_name; }

    //! Return the CodeSpecId of the NullCodeSpec
    static CodeSpecId GetNullCodeSpecId() {return CodeSpecId((uint64_t)1LL);}
    //! Return true if the specified CodeSpec is the NullCodeSpec
    bool IsNullCodeSpec() const {return GetCodeSpecId() == GetNullCodeSpecId();}

    DGNPLATFORM_EXPORT CodeSpecHandlerR GetCodeSpecHandler() const;

    DGNPLATFORM_EXPORT DgnDbStatus Insert();

    DGNPLATFORM_EXPORT static CodeSpecPtr Create(DgnDbR db, Utf8CP name, CodeScopeSpecCR scopeSpec=CodeScopeSpec::CreateRepositoryScope());

    CodeScopeSpecCR GetScope() const {return m_scopeSpec;}
    void SetScope(CodeScopeSpecCR scopeSpec) {m_scopeSpec = scopeSpec;}
    bool IsRepositoryScope() const {return CodeScopeSpec::Type::Repository == GetScope().GetType();}
    bool IsModelScope() const {return CodeScopeSpec::Type::Model == GetScope().GetType();}
    bool IsParentElementScope() const {return CodeScopeSpec::Type::ParentElement == GetScope().GetType();}

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
        DGNPLATFORM_EXPORT static CodeSpecHandlerP FindHandler(DgnDb const& dgndb, DgnClassId classId);

        CodeSpecPtr Create(Dgn::CodeSpec::CreateParams const& params) { return _CreateInstance(params); }
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
