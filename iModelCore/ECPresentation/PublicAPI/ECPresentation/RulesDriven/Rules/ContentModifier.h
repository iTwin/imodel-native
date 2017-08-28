/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/ContentModifier.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
        //! Returns XmlElement name that is used to read/save this rule information.
        ECPRESENTATION_EXPORT CharCP _GetXmlElementName() const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP xmlNode) const override;

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

        //! Returns related properties
        ECPRESENTATION_EXPORT RelatedPropertiesSpecificationList&  GetRelatedPropertiesR();

        //! Returns displayed/hidden properties
        ECPRESENTATION_EXPORT PropertiesDisplaySpecificationList const& GetPropertiesDisplaySpecifications() const;

        //! Returns displayed/hidden properties
        ECPRESENTATION_EXPORT PropertiesDisplaySpecificationList&  GetPropertiesDisplaySpecificationsR() ;

        //! Returns calculated properties
        ECPRESENTATION_EXPORT CalculatedPropertiesSpecificationList const& GetCalculatedProperties() const;

        //! Returns calculated properties
        ECPRESENTATION_EXPORT CalculatedPropertiesSpecificationList& GetCalculatedPropertiesR();

        //! Returns property editors
        ECPRESENTATION_EXPORT PropertyEditorsSpecificationList const& GetPropertyEditors() const;

        //! Returns property editors
        ECPRESENTATION_EXPORT PropertyEditorsSpecificationList& GetPropertyEditorsR();
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
