/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnAuthority.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDomain.h"

//-----------------------------------------------------------------------------------------
// Names of built-in Authorities. The best practice is to include the domain name as part of the authority name to ensure global uniqueness
//-----------------------------------------------------------------------------------------
#define BIS_AUTHORITY(name)                         "bis:" name
#define BIS_AUTHORITY_NullAuthority                 BIS_AUTHORITY(BIS_CLASS_NullAuthority)
#define BIS_AUTHORITY_AnnotationFrameStyle          BIS_AUTHORITY(BIS_CLASS_AnnotationFrameStyle)
#define BIS_AUTHORITY_AnnotationLeaderStyle         BIS_AUTHORITY(BIS_CLASS_AnnotationLeaderStyle)
#define BIS_AUTHORITY_AnnotationTextStyle           BIS_AUTHORITY(BIS_CLASS_AnnotationTextStyle)
#define BIS_AUTHORITY_CategorySelector              BIS_AUTHORITY(BIS_CLASS_CategorySelector)
#define BIS_AUTHORITY_DisplayStyle                  BIS_AUTHORITY(BIS_CLASS_DisplayStyle)
#define BIS_AUTHORITY_Drawing                       BIS_AUTHORITY(BIS_CLASS_Drawing)
#define BIS_AUTHORITY_DrawingCategory               BIS_AUTHORITY(BIS_CLASS_DrawingCategory)
#define BIS_AUTHORITY_GeometryPart                  BIS_AUTHORITY(BIS_CLASS_GeometryPart)
#define BIS_AUTHORITY_LightDefinition               BIS_AUTHORITY(BIS_CLASS_LightDefinition)
#define BIS_AUTHORITY_LineStyle                     BIS_AUTHORITY(BIS_CLASS_LineStyle)
#define BIS_AUTHORITY_LinkElement                   BIS_AUTHORITY(BIS_CLASS_LinkElement)
#define BIS_AUTHORITY_MaterialElement               BIS_AUTHORITY(BIS_CLASS_MaterialElement)
#define BIS_AUTHORITY_ModelSelector                 BIS_AUTHORITY(BIS_CLASS_ModelSelector)
#define BIS_AUTHORITY_InformationPartitionElement   BIS_AUTHORITY(BIS_CLASS_InformationPartitionElement)
#define BIS_AUTHORITY_Session                       BIS_AUTHORITY(BIS_CLASS_Session)
#define BIS_AUTHORITY_Sheet                         BIS_AUTHORITY(BIS_CLASS_Sheet)
#define BIS_AUTHORITY_SpatialCategory               BIS_AUTHORITY(BIS_CLASS_SpatialCategory)
#define BIS_AUTHORITY_SubCategory                   BIS_AUTHORITY(BIS_CLASS_SubCategory)
#define BIS_AUTHORITY_Subject                       BIS_AUTHORITY(BIS_CLASS_Subject)
#define BIS_AUTHORITY_TextAnnotationSeed            BIS_AUTHORITY(BIS_CLASS_TextAnnotationSeed)
#define BIS_AUTHORITY_Texture                       BIS_AUTHORITY(BIS_CLASS_Texture)
#define BIS_AUTHORITY_TrueColor                     BIS_AUTHORITY(BIS_CLASS_TrueColor)
#define BIS_AUTHORITY_ViewDefinition                BIS_AUTHORITY(BIS_CLASS_ViewDefinition)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_AuthorityHandler {struct DatabaseScope; struct ElementScope; struct ModelScope; struct Null;};

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
    friend struct DgnAuthority;

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

typedef DgnAuthority CodeSpec;
typedef DgnAuthority const& CodeSpecCR;
typedef DgnAuthorityPtr CodeSpecPtr;
typedef DgnAuthorityCPtr CodeSpecCPtr;
typedef DgnAuthorityId CodeSpecId;
typedef bvector<CodeFragmentSpec> CodeFragmentSpecList;
typedef CodeFragmentSpecList& CodeFragmentSpecListR;
typedef CodeFragmentSpecList const& CodeFragmentSpecListCR;
typedef bvector<Utf8String> CodeFragmentStringList;
typedef CodeFragmentStringList const& CodeFragmentStringListCR;

DEFINE_POINTER_SUFFIX_TYPEDEFS(CodeFragmentSpec)
DEFINE_POINTER_SUFFIX_TYPEDEFS(CodeScopeSpec)

//=======================================================================================
//! A DgnAuthority issues and validates DgnCodes for elements.
//! There are 2 general types of codes issued by authorities:
//!   - User/Application-supplied: The user/application supplies a DgnCode and the authority
//!     simply enforces uniqueness and any constraints on e.g. allowable characters
//!   - Generated: The authority generates a DgnCode for an object based on the object's
//!     properties, and/or some external logic. e.g., sequence numbers.
//! Some authorities may combine both approaches. e.g., sub-category names are supplied by
//! the user or application, but their DgnCode namespaces are generated from the category
//! to which they belong.
//! @note Authorities are DgnDb-specific. That is, authority names are constant, but their DgnAuthorityIds may vary per DgnDb.
// @bsistruct                                                    Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnAuthority : RefCountedBase
{
public:
    struct CreateParams
    {
        DgnDbR          m_dgndb;
        DgnAuthorityId  m_id;
        DgnClassId      m_classId;
        Utf8String      m_name;
        CodeScopeSpec   m_scopeSpec;

        CreateParams(DgnDbR dgndb, DgnClassId classId, Utf8CP name, DgnAuthorityId id=DgnAuthorityId(), CodeScopeSpecCR scopeSpec=CodeScopeSpec()) :
            m_dgndb(dgndb), m_id(id), m_classId(classId), m_name(name), m_scopeSpec(scopeSpec) {}
    };

protected:
    friend struct DgnAuthorities;
    friend struct dgn_AuthorityHandler::Authority;

    DgnDbR          m_dgndb;
    DgnAuthorityId  m_authorityId;
    DgnClassId      m_classId;
    Utf8String      m_name;

    CodeScopeSpec m_scopeSpec;
    CodeFragmentSpecList m_fragmentSpecs;
    
    DGNPLATFORM_EXPORT explicit DgnAuthority(CreateParams const&);

    void ReadProperties(Utf8StringCR jsonStr);
    Utf8String SerializeProperties() const;

    DGNPLATFORM_EXPORT virtual void _ToPropertiesJson(JsonValueR) const;
    DGNPLATFORM_EXPORT virtual void _FromPropertiesJson(JsonValueCR);
    DGNPLATFORM_EXPORT virtual DgnAuthorityPtr _CloneForImport(DgnDbStatus* status, DgnImportContext& importer) const;

    DGNPLATFORM_EXPORT virtual DgnDbStatus _CloneCodeForImport(DgnCodeR clonedCode, DgnElementCR srcElem, DgnModelR destModel, DgnImportContext& importer) const;

    DGNPLATFORM_EXPORT virtual DgnDbStatus _ValidateCode(DgnElementCR element) const;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _RegenerateCode(DgnCodeR regeneratedCode, DgnElementCR element) const;

private:
    static DgnDbStatus Insert(DgnDbR db, Utf8CP name, CodeScopeSpecCR scope, CodeSpecId codeSpecId=CodeSpecId());

public:
    DgnDbR GetDgnDb() const { return m_dgndb; }
    DgnAuthorityId GetAuthorityId() const { return m_authorityId; }
    DgnClassId GetClassId() const { return m_classId; }
    Utf8StringCR GetName() const { return m_name; }

    //! Return the DgnAuthorityId of the NullAuthority
    static DgnAuthorityId GetNullAuthorityId() {return DgnAuthorityId((uint64_t)1LL);}
    //! Return true if the specified DgnAuthority is the NullAuthority
    bool IsNullAuthority() const {return GetAuthorityId() == GetNullAuthorityId();}

    DGNPLATFORM_EXPORT AuthorityHandlerR GetAuthorityHandler() const;

    DGNPLATFORM_EXPORT DgnDbStatus Insert();

    DGNPLATFORM_EXPORT static DgnAuthorityPtr Create(DgnDbR db, Utf8CP name, CodeScopeSpecCR scopeSpec=CodeScopeSpec::CreateRepositoryScope());

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
    DgnCode CreateCode(Utf8StringCR value, Utf8StringCR nameSpace) const { return DgnCode(m_authorityId, value, nameSpace); } // WIP: Deprecate?

    DGNPLATFORM_EXPORT DgnDbStatus ValidateCode(DgnElementCR) const;
    DGNPLATFORM_EXPORT DgnDbStatus RegenerateCode(DgnCodeR newCode, DgnElementCR) const;
    DGNPLATFORM_EXPORT DgnDbStatus CloneCodeForImport(DgnCodeR newCode, DgnElementCR srcElem, DgnModelR destModel, DgnImportContext& importer) const;

    DGNPLATFORM_EXPORT static DgnAuthorityPtr Import(DgnDbStatus* status, DgnAuthorityCR sourceAuthority, DgnImportContext& importer);

    //! Identifies actions which may be restricted for authorities created by a handler for a missing subclass of DgnAuthority
    struct RestrictedAction : DgnDomain::Handler::RestrictedAction
    {
        DEFINE_T_SUPER(DgnDomain::Handler::RestrictedAction);

        static const uint64_t ValidateCode = T_Super::NextAvailable; //!< Validate code issued by this authority
        static const uint64_t RegenerateCode = ValidateCode << 1; //!< Regenerate a code issued by this authority
        static const uint64_t CloneCode = RegenerateCode << 1; //!< Clone a code issued by this authority

        DGNPLATFORM_EXPORT static uint64_t Parse(Utf8CP name); //!< Parse action name from ClassHasHandler custom attribute
    };
};

//=======================================================================================
// Macro used for declaring the members of a DgnAuthority handler
//=======================================================================================
#define AUTHORITYHANDLER_DECLARE_MEMBERS(__ECClassName__,__classname__,_handlerclass__,_handlersuperclass__,__exporter__) \
    private: virtual Dgn::DgnAuthorityP _CreateInstance(Dgn::DgnAuthority::CreateParams const& params) override {return new __classname__(__classname__::CreateParams(params));}\
        DOMAINHANDLER_DECLARE_MEMBERS(__ECClassName__,_handlerclass__,_handlersuperclass__,__exporter__)

//=======================================================================================
//! DgnAuthority handlers
//=======================================================================================
namespace dgn_AuthorityHandler
{
    struct EXPORT_VTABLE_ATTRIBUTE Authority : DgnDomain::Handler
    {
        DOMAINHANDLER_DECLARE_MEMBERS (BIS_CLASS_Authority, Authority, DgnDomain::Handler, DGNPLATFORM_EXPORT)

    protected:
        virtual AuthorityHandlerP _ToAuthorityHandler() override { return this; }
        virtual DgnAuthorityP _CreateInstance(DgnAuthority::CreateParams const& params) { return new DgnAuthority(params); }
    public:
        virtual uint64_t _ParseRestrictedAction(Utf8CP name) const override { return DgnAuthority::RestrictedAction::Parse(name); }
        DGNPLATFORM_EXPORT static AuthorityHandlerP FindHandler(DgnDb const& dgndb, DgnClassId classId);

        DgnAuthorityPtr Create(DgnAuthority::CreateParams const& params) { return _CreateInstance(params); }
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
