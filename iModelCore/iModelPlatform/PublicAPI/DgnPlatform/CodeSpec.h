/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

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
#define BIS_CODESPEC_AuxCoordSystemSpatial          BIS_CODESPEC(BIS_CLASS_AuxCoordSystemSpatial)
#define BIS_CODESPEC_CategorySelector               BIS_CODESPEC(BIS_CLASS_CategorySelector)
#define BIS_CODESPEC_ColorBook                      BIS_CODESPEC(BIS_CLASS_ColorBook)
#define BIS_CODESPEC_DisplayStyle                   BIS_CODESPEC(BIS_CLASS_DisplayStyle)
#define BIS_CODESPEC_Drawing                        BIS_CODESPEC(BIS_CLASS_Drawing)
#define BIS_CODESPEC_DrawingCategory                BIS_CODESPEC(BIS_CLASS_DrawingCategory)
#define BIS_CODESPEC_ExternalSource                 BIS_CODESPEC(BIS_CLASS_ExternalSource)
#define BIS_CODESPEC_ExternalSourceAttachment       BIS_CODESPEC(BIS_CLASS_ExternalSourceAttachment)
#define BIS_CODESPEC_GeometryPart                   BIS_CODESPEC(BIS_CLASS_GeometryPart)
#define BIS_CODESPEC_GraphicalType2d                BIS_CODESPEC(BIS_CLASS_GraphicalType2d)
#define BIS_CODESPEC_TemplateRecipe2d               BIS_CODESPEC(BIS_CLASS_TemplateRecipe2d)
#define BIS_CODESPEC_LineStyle                      BIS_CODESPEC(BIS_CLASS_LineStyle)
#define BIS_CODESPEC_LinkElement                    BIS_CODESPEC(BIS_CLASS_LinkElement)
#define BIS_CODESPEC_ModelSelector                  BIS_CODESPEC(BIS_CLASS_ModelSelector)
#define BIS_CODESPEC_PhysicalMaterial               BIS_CODESPEC(BIS_CLASS_PhysicalMaterial)
#define BIS_CODESPEC_PhysicalType                   BIS_CODESPEC(BIS_CLASS_PhysicalType)
#define BIS_CODESPEC_InformationPartitionElement    BIS_CODESPEC(BIS_CLASS_InformationPartitionElement)
#define BIS_CODESPEC_RenderMaterial                 BIS_CODESPEC(BIS_CLASS_RenderMaterial)
#define BIS_CODESPEC_Sheet                          BIS_CODESPEC(BIS_CLASS_Sheet)
#define BIS_CODESPEC_SpatialCategory                BIS_CODESPEC(BIS_CLASS_SpatialCategory)
#define BIS_CODESPEC_SpatialLocationType            BIS_CODESPEC(BIS_CLASS_SpatialLocationType)
#define BIS_CODESPEC_SubCategory                    BIS_CODESPEC(BIS_CLASS_SubCategory)
#define BIS_CODESPEC_Subject                        BIS_CODESPEC(BIS_CLASS_Subject)
#define BIS_CODESPEC_TemplateRecipe3d               BIS_CODESPEC(BIS_CLASS_TemplateRecipe3d)
#define BIS_CODESPEC_TextAnnotationSeed             BIS_CODESPEC(BIS_CLASS_TextAnnotationSeed)
#define BIS_CODESPEC_Texture                        BIS_CODESPEC(BIS_CLASS_Texture)
#define BIS_CODESPEC_ViewDefinition                 BIS_CODESPEC(BIS_CLASS_ViewDefinition)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsistruct
//=======================================================================================
struct CodeScopeSpec {
    friend struct CodeSpec;

public:
    enum class Type {
        Repository = 1,     //!< The Code value must be unique within (at least) the DgnDb repository
        Model = 2,          //!< The Code value must be unique within the scope of the DgnModel
        ParentElement = 3,  //!< The Code value must be unique among other children of the same parent element
        RelatedElement = 4, //!< The Code value must be unique among other elements also scoped by the same element
    };

private:
    BE_JSON_NAME(type);
    BE_JSON_NAME(relationship); // only valid for Type::RelatedElement
    BE_JSON_NAME(fGuidRequired);

    Type m_type;
    bool m_fedGuidRequired;
    Utf8String m_relationship;

    CodeScopeSpec(Type type = Type::Repository, bool fedGuidRequired = false) { SetType(type); m_fedGuidRequired = fedGuidRequired; }
    void SetType(Type type) { m_type = type; }
    void SetRelationship(Utf8CP relationship) { if (relationship && *relationship)m_relationship = relationship; }

public:
    Type GetType() const { return m_type; }
    bool IsFederationGuidRequired() const { return m_fedGuidRequired; }
    void SetFederationGuidRequired(bool yesNo) { m_fedGuidRequired = yesNo; }
    Utf8String GetRelationship() const { return m_relationship; }
    void ToJSON(BeJsValue json) const {
        json[json_type()] = (int)m_type;
        if (m_fedGuidRequired)
            json[json_fGuidRequired()] = true;
        if (!m_relationship.empty())
            json[json_relationship()] = m_relationship;
    }

    void FromJSON(BeJsConst json) {
        m_type = (Type)json[json_type()].asInt((int)Type::Repository);
        m_fedGuidRequired = json[json_fGuidRequired()].asBool();
        m_relationship = json[json_relationship()].asString();
    }

    static CodeScopeSpec CreateRepositoryScope(bool fedGuid = false) { return CodeScopeSpec(Type::Repository, fedGuid); }
    static CodeScopeSpec CreateModelScope(bool fedGuid = false) { return CodeScopeSpec(Type::Model, fedGuid); }
    static CodeScopeSpec CreateParentElementScope(bool fedGuid = false) { return CodeScopeSpec(Type::ParentElement, fedGuid); }
    static CodeScopeSpec CreateRelatedElementScope(Utf8CP relationship = nullptr, bool fedGuid = false) {
        CodeScopeSpec scopeSpec(Type::RelatedElement, fedGuid);
        scopeSpec.SetRelationship(relationship);
        return scopeSpec;
    }
};

//=======================================================================================
//! A CodeSpec captures the rules for encoding and decoding significant business information into and from a Code (string).
//! A CodeSpec determines how DgnCodes for elements are generated and validated.
//! There are 2 general types of codes:
//!   - User/Application-supplied: The user/application supplies a DgnCode and the CodeSpec
//!     simply enforces any constraints on e.g. allowable characters
//!   - Generated: The CodeSpec defines a CodeFragmentSpec array to indicate how to build up the overall CodeValue.
//! @note A CodeSpec is DgnDb-specific. That is, CodeSpec names are constant, but their CodeSpecIds may vary per DgnDb.
// @bsistruct
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CodeSpec : RefCountedBase, NonCopyableClass {
    friend struct DgnCodeSpecs;
    friend struct dgn_CodeSpecHandler::CodeSpec;

public:
    enum class Kind : uint32_t {
        RepositorySpecific = 1,     //!< Code-uniqueness checked in light of a single repository. Element-referencing based on ElementIds.
        BusinessRelated = 2,        //!< Universe of code-uniqueness for associated codes span multiple repositories. Element-referencing based on FederationGuids.
    };

    struct CreateParams {
        DgnDbR m_dgndb;
        CodeSpecId m_id;
        Utf8String m_name;
        CodeScopeSpec m_scopeSpec;
        CreateParams(DgnDbR dgndb, Utf8CP name, CodeSpecId id = CodeSpecId(), CodeScopeSpecCR scopeSpec = CodeScopeSpec()) : m_dgndb(dgndb), m_id(id), m_name(name), m_scopeSpec(scopeSpec) {}
    };

    BE_PROP_NAME(JsonProperties);
    BE_JSON_NAME(fragmentSpecs);
    BE_JSON_NAME(registrySuffix);
    BE_JSON_NAME(scopeSpec);
    BE_JSON_NAME(spec);
    BE_JSON_NAME(version);
    BE_JSON_NAME(isManagedWithDgnDb);
    BE_JSON_NAME(codeSpecKind);

private:
    DgnDbR m_dgndb;
    CodeSpecId m_codeSpecId;
    Utf8String m_name;
    BeJsDocument m_specProperties;
    CodeScopeSpec m_scopeSpec;

    DGNPLATFORM_EXPORT explicit CodeSpec(CreateParams const&);

    void ReadProperties(Utf8StringCR jsonStr);
    Utf8String SerializeProperties() const;

    void ToPropertiesJson(BeJsValue) const;    
    CodeSpecPtr CloneForImport(DgnDbStatus* status, DgnImportContext& importer) const;

    static DgnDbStatus Insert(DgnDbR db, Utf8CP name, CodeScopeSpecCR scope, CodeSpecId codeSpecId = CodeSpecId());

public:
    DGNPLATFORM_EXPORT void FromPropertiesJson(BeJsConst);

    DgnDbR GetDgnDb() const { return m_dgndb; }
    CodeSpecId GetCodeSpecId() const { return m_codeSpecId; }
    Utf8StringCR GetName() const { return m_name; }

    Utf8String GetRegistrySuffix() const {return m_specProperties[json_registrySuffix()].asString();}
    void SetRegistrySuffix(Utf8CP registrySuffix) {if (registrySuffix && *registrySuffix) m_specProperties[json_registrySuffix()] = registrySuffix;}

    //! Return true if the codes associated with this CodeSpec are managed along with the DgnDb. Return false if the codes associated with this CodeSpec are managed by an external service.
    //! @deprecated Use GetKind instead
    bool IsManagedWithDgnDb() const { return m_specProperties[json_isManagedWithDgnDb()].asBool(true); }
    //! Set whether the codes associated with this CodeSpec are managed along with the DgnDb or by an external service.
    //! @deprecated Use SetKind instead
    void SetIsManagedWithDgnDb(bool isManagedWithDgnDb) { m_specProperties[json_isManagedWithDgnDb()] = isManagedWithDgnDb; }

    Kind GetKind() const { return (Kind)m_specProperties[json_codeSpecKind()].asUInt((uint32_t)Kind::RepositorySpecific); }
    void SetKind(Kind codeSpecKind) { m_specProperties[json_codeSpecKind()] = (uint32_t)codeSpecKind; }

    //! Return the CodeSpecId of the NullCodeSpec
    static CodeSpecId GetNullCodeSpecId() { return CodeSpecId((uint64_t)1LL); }

    //! Return true if the specified CodeSpec is the NullCodeSpec
    bool IsNullCodeSpec() const { return GetCodeSpecId() == GetNullCodeSpecId(); }

    //! Make a writable copy of this CodeSpec so that the copy may be edited.
    //! @return a CodeSpecPtr that holds the editable copy of this codeSpec.
    //! @note This method may only be used on a CodeSpec this is the readonly persistent element returned by CodeSpecs::GetCodeSpec, and then
    //! only one editing copy of this codeSpec at a time may exist. If another copy is extant, this method will return an invalid CodeSpecPtr.
    DGNPLATFORM_EXPORT CodeSpecPtr CopyForEdit() const;

    DGNPLATFORM_EXPORT DgnDbStatus Insert();
    DGNPLATFORM_EXPORT DgnDbStatus Update();

    DGNPLATFORM_EXPORT static CodeSpecPtr Create(DgnDbR db, Utf8CP name, CodeScopeSpecCR scopeSpec = CodeScopeSpec::CreateRepositoryScope());
    DGNPLATFORM_EXPORT static CodeSpecPtr Create(DgnDbR db, Utf8CP name, BeJsConst jsonProperties);

    CodeScopeSpecCR GetScope() const { return m_scopeSpec; }
    bool IsRepositoryScope() const { return CodeScopeSpec::Type::Repository == GetScope().GetType(); }
    bool IsModelScope() const { return CodeScopeSpec::Type::Model == GetScope().GetType(); }
    bool IsParentElementScope() const { return CodeScopeSpec::Type::ParentElement == GetScope().GetType(); }
    bool IsRelatedElementScope() const { return CodeScopeSpec::Type::RelatedElement == GetScope().GetType(); }

    //! Return the DgnElementId of the scope element for the specified element.
    DGNPLATFORM_EXPORT DgnElementId GetScopeElementId(DgnElementCR element) const;

    DGNPLATFORM_EXPORT static DgnCode CreateCode(Utf8CP codeSpecName, DgnElementCR scopeElement, Utf8StringCR value);
    DGNPLATFORM_EXPORT DgnCode CreateCode(DgnElementCR scopeElement, Utf8StringCR value) const;

    DGNPLATFORM_EXPORT static DgnCode CreateCode(Utf8CP codeSpecName, DgnModelCR scopeModel, Utf8StringCR value);
    DGNPLATFORM_EXPORT DgnCode CreateCode(DgnModelCR scopeModel, Utf8StringCR value) const;

    DGNPLATFORM_EXPORT static DgnCode CreateCode(DgnDbR db, Utf8CP codeSpecName, Utf8StringCR value);
    DGNPLATFORM_EXPORT DgnCode CreateCode(Utf8StringCR value) const;

    DGNPLATFORM_EXPORT DgnDbStatus CloneCodeForImport(DgnCodeR newCode, DgnElementCR srcElem, DgnModelR destModel, DgnImportContext& importer) const;
    DGNPLATFORM_EXPORT static CodeSpecPtr Import(DgnDbStatus* status, CodeSpecCR sourceCodeSpec, DgnImportContext& importer);
};

//=======================================================================================
//! CodeSpec handlers
//=======================================================================================
namespace dgn_CodeSpecHandler
{
    struct EXPORT_VTABLE_ATTRIBUTE CodeSpec : DgnDomain::Handler
    {
        DOMAINHANDLER_DECLARE_MEMBERS (BIS_CLASS_CodeSpec, CodeSpec, DgnDomain::Handler, DGNPLATFORM_EXPORT)

    protected:
        virtual CodeSpecP _CreateInstance(Dgn::CodeSpec::CreateParams const& params) { return new Dgn::CodeSpec(params); }

    public:
        CodeSpecPtr Create(Dgn::CodeSpec::CreateParams const& params) { return _CreateInstance(params); }
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
