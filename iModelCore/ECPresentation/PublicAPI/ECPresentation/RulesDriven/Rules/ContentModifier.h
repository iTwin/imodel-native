/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentModifiersList : HashableBase
{
private:
    RelatedPropertiesSpecificationList      m_relatedProperties;
    CalculatedPropertiesSpecificationList   m_calculatedProperties;
    PropertyCategorySpecificationsList      m_propertyCategories;
    PropertySpecificationsList              m_propertyOverrides;

public:
    ECPRESENTATION_EXPORT bool ReadXml(BeXmlNodeP xmlNode);
    ECPRESENTATION_EXPORT void WriteXml(BeXmlNodeP xmlNode) const;

    ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);
    ECPRESENTATION_EXPORT void WriteJson(JsonValueR json) const;

    //! Computes rule hash.
    ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

public:
    //! Constructor. It is used to initialize the rule with default settings.
    ContentModifiersList() {}

    //! Copy constructor.
    ECPRESENTATION_EXPORT ContentModifiersList(ContentModifiersList const&);

    //! Move constructor.
    ECPRESENTATION_EXPORT ContentModifiersList(ContentModifiersList&&);
    
    //! Destructor.
    ECPRESENTATION_EXPORT ~ContentModifiersList();

    //! Returns related properties
    RelatedPropertiesSpecificationList const& GetRelatedProperties() const {return m_relatedProperties;}

    //! Adds related property
    ECPRESENTATION_EXPORT void AddRelatedProperty(RelatedPropertiesSpecificationR specification);

    //! Returns calculated properties
    CalculatedPropertiesSpecificationList const& GetCalculatedProperties() const {return m_calculatedProperties;}

    //! Adds calculated property
    ECPRESENTATION_EXPORT void AddCalculatedProperty(CalculatedPropertiesSpecificationR specification);

    //! Returns custom property categories 
    PropertyCategorySpecificationsList const& GetPropertyCategories() const {return m_propertyCategories;}

    //! Adds a custom property category
    ECPRESENTATION_EXPORT void AddPropertyCategory(PropertyCategorySpecificationR);

    //! Returns property overrides
    PropertySpecificationsList const& GetPropertyOverrides() const { return m_propertyOverrides; }

    //! Adds a property override
    ECPRESENTATION_EXPORT void AddPropertyOverride(PropertySpecificationR);
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras               05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentModifier : PresentationKey
{
private:
    Utf8String m_schemaName;
    Utf8String m_className;
    ContentModifiersList m_modifiers;

protected:
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
    ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

public:
    ContentModifier() {}
    ContentModifier(Utf8String schemaName, Utf8String className) : m_schemaName(schemaName), m_className(className) {}
    Utf8StringCR GetSchemaName() const {return m_schemaName;}
    Utf8StringCR GetClassName() const {return m_className;}
    RelatedPropertiesSpecificationList const& GetRelatedProperties() const {return m_modifiers.GetRelatedProperties();}
    void AddRelatedProperty(RelatedPropertiesSpecificationR specification) {m_modifiers.AddRelatedProperty(specification);}
    CalculatedPropertiesSpecificationList const& GetCalculatedProperties() const {return m_modifiers.GetCalculatedProperties();}
    void AddCalculatedProperty(CalculatedPropertiesSpecificationR specification) {m_modifiers.AddCalculatedProperty(specification);}
    PropertyCategorySpecificationsList const& GetPropertyCategories() const {return m_modifiers.GetPropertyCategories();}
    void AddPropertyCategory(PropertyCategorySpecificationR specification) {m_modifiers.AddPropertyCategory(specification);}
    PropertySpecificationsList const& GetPropertyOverrides() const {return m_modifiers.GetPropertyOverrides();}
    void AddPropertyOverride(PropertySpecificationR specification) {m_modifiers.AddPropertyOverride(specification);}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
