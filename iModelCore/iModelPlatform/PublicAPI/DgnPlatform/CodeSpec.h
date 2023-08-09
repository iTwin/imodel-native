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
    // declares the scope of uniqueness for codes that use this spec.
    enum class Type {
        Repository = 1,     // an iModel
        Model = 2,          // a single Model
        ParentElement = 3,  // a parent element
        RelatedElement = 4, // another (related) element
    };

private:
    BE_JSON_NAME(type);
    BE_JSON_NAME(relationship); // the full classname of the relationship for Type::RelatedElement.
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
};

//=======================================================================================
//! A CodeSpec captures the rules for encoding significant business information into a Code (string).
// @bsistruct
//=======================================================================================
struct CodeSpec : RefCountedBase, NonCopyableClass {
    friend struct DgnCodeSpecs;
    friend struct dgn_CodeSpecHandler::CodeSpec;

public:
    struct CreateParams {
        DgnDbR m_dgndb;
        CodeSpecId m_id;
        Utf8String m_name;
        CodeScopeSpec m_scopeSpec;
        CreateParams(DgnDbR dgndb, Utf8CP name, CodeSpecId id = CodeSpecId(), CodeScopeSpecCR scopeSpec = CodeScopeSpec()) : m_dgndb(dgndb), m_id(id), m_name(name), m_scopeSpec(scopeSpec) {}
    };

    BE_PROP_NAME(JsonProperties);
    BE_JSON_NAME(scopeSpec);
    BE_JSON_NAME(spec);
    BE_JSON_NAME(version);
    BE_JSON_NAME(isManagedWithDgnDb);

private:
    DgnDbR m_dgndb;
    CodeSpecId m_codeSpecId;
    Utf8String m_name;
    BeJsDocument m_specProperties;
    CodeScopeSpec m_scopeSpec;

    explicit CodeSpec(CreateParams const& params) : m_dgndb(params.m_dgndb), m_codeSpecId(params.m_id), m_name(params.m_name), m_scopeSpec(params.m_scopeSpec) {}
    void ReadProperties(Utf8StringCR jsonStr);
    Utf8String SerializeProperties() const;

    void ToPropertiesJson(BeJsValue) const;
    void FromPropertiesJson(BeJsConst);
    CodeSpecPtr CloneForImport(DgnDbStatus* status, DgnImportContext& importer) const;

public:
    DgnDbR GetDgnDb() const { return m_dgndb; }
    CodeSpecId GetCodeSpecId() const { return m_codeSpecId; }
    Utf8StringCR GetName() const { return m_name; }

    // the CodeSpecId of the NullCodeSpec
    static CodeSpecId GetNullCodeSpecId() { return CodeSpecId((uint64_t)1LL); }

    // true if the specified CodeSpec is the NullCodeSpec
    bool IsNullCodeSpec() const { return GetCodeSpecId() == GetNullCodeSpecId(); }

    DGNPLATFORM_EXPORT DgnDbStatus Insert();

    DGNPLATFORM_EXPORT static CodeSpecPtr CreateRepositorySpec(DgnDbR db, Utf8CP name) { return Create(db, name, CodeScopeSpec::CreateRepositoryScope()); }
    DGNPLATFORM_EXPORT static CodeSpecPtr Create(DgnDbR db, Utf8CP name, CodeScopeSpecCR scopeSpec);
    DGNPLATFORM_EXPORT static CodeSpecPtr Create(DgnDbR db, Utf8CP name, BeJsConst jsonProperties);

    CodeScopeSpecCR GetScope() const { return m_scopeSpec; }
    bool IsRepositoryScope() const { return CodeScopeSpec::Type::Repository == GetScope().GetType(); }
    bool IsModelScope() const { return CodeScopeSpec::Type::Model == GetScope().GetType(); }
    bool IsParentElementScope() const { return CodeScopeSpec::Type::ParentElement == GetScope().GetType(); }

    DGNPLATFORM_EXPORT static DgnCode CreateCode(Utf8CP codeSpecName, DgnElementCR scopeElement, Utf8StringCR value);
    DGNPLATFORM_EXPORT DgnCode CreateCode(DgnElementCR scopeElement, Utf8StringCR value) const;

    DGNPLATFORM_EXPORT static DgnCode CreateCode(Utf8CP codeSpecName, DgnModelCR scopeModel, Utf8StringCR value);
    DGNPLATFORM_EXPORT DgnCode CreateCode(DgnModelCR scopeModel, Utf8StringCR value) const;

    DGNPLATFORM_EXPORT static DgnCode CreateRepositoryScopedCode(DgnDbR db, Utf8CP codeSpecName, Utf8StringCR value);
    DGNPLATFORM_EXPORT DgnCode CreateRepositoryScopedCode(Utf8StringCR value, DgnCodeValue::Behavior) const;

    DgnDbStatus CloneCodeForImport(DgnCodeR newCode, DgnElementCR srcElem, DgnModelR destModel, DgnImportContext& importer) const;
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
