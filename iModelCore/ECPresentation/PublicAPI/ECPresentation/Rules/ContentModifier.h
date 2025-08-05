/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/Rules/PresentationRulesTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentModifiersList : HashableBase
{
private:
    RelatedPropertiesSpecificationList      m_relatedProperties;
    CalculatedPropertiesSpecificationList   m_calculatedProperties;
    PropertyCategorySpecificationsList      m_propertyCategories;
    PropertySpecificationsList              m_propertyOverrides;

public:
    ECPRESENTATION_EXPORT bool ReadJson(BeJsConst json);
    ECPRESENTATION_EXPORT void WriteJson(BeJsValue json) const;

    //! Computes rule hash.
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

public:
    //! Constructor. It is used to initialize the rule with default settings.
    ContentModifiersList() {}

    //! Copy constructor.
    ECPRESENTATION_EXPORT ContentModifiersList(ContentModifiersList const&);

    //! Move constructor.
    ECPRESENTATION_EXPORT ContentModifiersList(ContentModifiersList&&);

    //! Destructor.
    ECPRESENTATION_EXPORT ~ContentModifiersList();

    RelatedPropertiesSpecificationList const& GetRelatedProperties() const {return m_relatedProperties;}
    ECPRESENTATION_EXPORT void AddRelatedProperty(RelatedPropertiesSpecificationR specification);
    ECPRESENTATION_EXPORT void ClearRelatedProperties();

    CalculatedPropertiesSpecificationList const& GetCalculatedProperties() const {return m_calculatedProperties;}
    ECPRESENTATION_EXPORT void AddCalculatedProperty(CalculatedPropertiesSpecificationR specification);
    ECPRESENTATION_EXPORT void ClearCalculatedProperties();

    PropertyCategorySpecificationsList const& GetPropertyCategories() const {return m_propertyCategories;}
    ECPRESENTATION_EXPORT void AddPropertyCategory(PropertyCategorySpecificationR);
    ECPRESENTATION_EXPORT void ClearPropertyCategories();

    PropertySpecificationsList const& GetPropertyOverrides() const {return m_propertyOverrides;}
    ECPRESENTATION_EXPORT void AddPropertyOverride(PropertySpecificationR);
    ECPRESENTATION_EXPORT void ClearPropertyOverrides();
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentModifier : PrioritizedPresentationKey
{
    DEFINE_T_SUPER(PrioritizedPresentationKey)

private:
    Utf8String m_schemaName;
    Utf8String m_className;
    ContentModifiersList m_modifiers;
    RequiredSchemaSpecificationsList m_requiredSchemas;
    bool m_applyOnNestedContent;

protected:
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementTypeAttributeName() const override;
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(BeJsConst json) override;
    ECPRESENTATION_EXPORT void _WriteJson(BeJsValue json) const override;

    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

public:
    ContentModifier() : m_applyOnNestedContent(false) {}
    ContentModifier(Utf8String schemaName, Utf8String className) : m_schemaName(schemaName), m_className(className), m_applyOnNestedContent(false) {}
    ECPRESENTATION_EXPORT ContentModifier(ContentModifier const&);
    ECPRESENTATION_EXPORT ContentModifier(ContentModifier&&);
    ECPRESENTATION_EXPORT ~ContentModifier();

    Utf8StringCR GetSchemaName() const {return m_schemaName;}
    Utf8StringCR GetClassName() const {return m_className;}
    bool HasClassSpecified() const {return !m_schemaName.empty() || !m_className.empty();}

    RequiredSchemaSpecificationsList const& GetRequiredSchemaSpecifications() const {return m_requiredSchemas;}
    ECPRESENTATION_EXPORT void ClearRequiredSchemaSpecifications();
    ECPRESENTATION_EXPORT void AddRequiredSchemaSpecification(RequiredSchemaSpecificationR spec);

    RelatedPropertiesSpecificationList const& GetRelatedProperties() const {return m_modifiers.GetRelatedProperties();}
    void AddRelatedProperty(RelatedPropertiesSpecificationR specification) {m_modifiers.AddRelatedProperty(specification);}

    CalculatedPropertiesSpecificationList const& GetCalculatedProperties() const {return m_modifiers.GetCalculatedProperties();}
    void AddCalculatedProperty(CalculatedPropertiesSpecificationR specification) {m_modifiers.AddCalculatedProperty(specification);}

    PropertyCategorySpecificationsList const& GetPropertyCategories() const {return m_modifiers.GetPropertyCategories();}
    void AddPropertyCategory(PropertyCategorySpecificationR specification) {m_modifiers.AddPropertyCategory(specification);}

    PropertySpecificationsList const& GetPropertyOverrides() const {return m_modifiers.GetPropertyOverrides();}
    void AddPropertyOverride(PropertySpecificationR specification) {m_modifiers.AddPropertyOverride(specification);}

    bool ShouldApplyOnNestedContent() const { return m_applyOnNestedContent; }
    void SetApplyOnNestedContent(bool value) { m_applyOnNestedContent = value; InvalidateHash(); }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
