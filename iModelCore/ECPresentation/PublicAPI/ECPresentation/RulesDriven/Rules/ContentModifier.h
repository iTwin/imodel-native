/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECPresentationRules/ContentModifier.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentationRules/PresentationRule.h>
#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras               05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentModifier : public CustomizationRule 
    {
    private:
        Utf8String                              m_schemaName;
        Utf8String                              m_className;
        RelatedPropertiesSpecificationList      m_relatedProperties;
        HiddenPropertiesSpecificationList       m_hiddenProperties;
        CalculatedPropertiesSpecificationList   m_calculatedProperties;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP _GetXmlElementName() const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool _ReadXml(BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void _WriteXml(BeXmlNodeP xmlNode) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT ContentModifier();

        //! Copy constructor.
        ECOBJECTS_EXPORT ContentModifier(ContentModifierCR);

        //! Constructor.
        ECOBJECTS_EXPORT ContentModifier(Utf8String schemaName, Utf8String className);

        //! Destructor.
        ECOBJECTS_EXPORT ~ContentModifier();

        //! Reads specification from XML.
        ECOBJECTS_EXPORT bool ReadXml(BeXmlNodeP xmlNode);

        //! Writes related properties to xml node.
        ECOBJECTS_EXPORT void WriteXml(BeXmlNodeP parentXmlNode) const;

        //! Returns schema name
        ECOBJECTS_EXPORT Utf8StringCR GetSchemaName() const;

        //! Returns class name
        ECOBJECTS_EXPORT Utf8StringCR GetClassName() const;

        //! Returns related properties
        ECOBJECTS_EXPORT RelatedPropertiesSpecificationList const& GetRelatedProperties() const;

        //! Returns related properties
        ECOBJECTS_EXPORT RelatedPropertiesSpecificationList&  GetRelatedPropertiesR();

        //! Returns hidden properties
        ECOBJECTS_EXPORT HiddenPropertiesSpecificationList const& GetHiddenProperties() const;

        //! Returns hidden properties
        ECOBJECTS_EXPORT HiddenPropertiesSpecificationList&  GetHiddenPropertiesR() ;

        //! Returns calculated properties
        ECOBJECTS_EXPORT CalculatedPropertiesSpecificationList const& GetCalculatedProperties() const;

        //! Returns calculated properties
        ECOBJECTS_EXPORT CalculatedPropertiesSpecificationList& GetCalculatedPropertiesR();
    };

END_BENTLEY_ECOBJECT_NAMESPACE
