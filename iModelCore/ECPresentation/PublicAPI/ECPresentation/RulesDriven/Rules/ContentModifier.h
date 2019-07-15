/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras               05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentModifier : PresentationKey
    {
    private:
        Utf8String                              m_schemaName;
        Utf8String                              m_className;
        RelatedPropertiesSpecificationList      m_relatedProperties;
        PropertiesDisplaySpecificationList      m_propertiesDisplaySpecification;
        CalculatedPropertiesSpecificationList   m_calculatedProperties;
        PropertyEditorsSpecificationList        m_propertyEditors;

    protected:
        ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
        ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
        ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

        ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
        ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

        //! Computes rule hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT ContentModifier();

        //! Copy constructor.
        ECPRESENTATION_EXPORT ContentModifier(ContentModifierCR);

        //! Constructor.
        ECPRESENTATION_EXPORT ContentModifier(Utf8String schemaName, Utf8String className);

        //! Destructor.
        ECPRESENTATION_EXPORT ~ContentModifier();

        //! Returns schema name
        ECPRESENTATION_EXPORT Utf8StringCR GetSchemaName() const;

        //! Returns class name
        ECPRESENTATION_EXPORT Utf8StringCR GetClassName() const;

        //! Returns related properties
        ECPRESENTATION_EXPORT RelatedPropertiesSpecificationList const& GetRelatedProperties() const;

        //! Adds related property
        ECPRESENTATION_EXPORT void AddRelatedProperty(RelatedPropertiesSpecificationR specification);

        //! Returns displayed/hidden properties
        ECPRESENTATION_EXPORT PropertiesDisplaySpecificationList const& GetPropertiesDisplaySpecifications() const;

        //! Add displayed/hidden property
        ECPRESENTATION_EXPORT void  AddPropertiesDisplaySpecification(PropertiesDisplaySpecificationR specification) ;

        //! Returns calculated properties
        ECPRESENTATION_EXPORT CalculatedPropertiesSpecificationList const& GetCalculatedProperties() const;

        //! Adds calculated property
        ECPRESENTATION_EXPORT void AddCalculatedProperty(CalculatedPropertiesSpecificationR specification);

        //! Returns property editors
        ECPRESENTATION_EXPORT PropertyEditorsSpecificationList const& GetPropertyEditors() const;

        //! Adds property editor
        ECPRESENTATION_EXPORT void AddPropertyEditor(PropertyEditorsSpecificationR specification);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
