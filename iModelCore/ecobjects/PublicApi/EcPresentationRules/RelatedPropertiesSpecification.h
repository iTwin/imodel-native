/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/RelatedPropertiesSpecification.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
Related properties specification. It allow to extend a content ECQuery to include 
properties of related classes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelatedPropertiesSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        RequiredRelationDirection          m_requiredDirection;
        WString                            m_relationshipClassNames;
        WString                            m_relatedClassNames;
        WString                            m_propertyNames;
        RelatedPropertiesSpecificationList m_nestedRelatedPropertiesSpecification;

    public:
    /*__PUBLISH_SECTION_START__*/
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT RelatedPropertiesSpecification ()
            : m_requiredDirection (RequiredRelationDirection_Both), m_relationshipClassNames (L""), m_relatedClassNames (L""), m_propertyNames (L"")
            {
            }

        //! Constructor.
        ECOBJECTS_EXPORT RelatedPropertiesSpecification 
                                       (
                                        RequiredRelationDirection  requiredDirection,
                                        WString                    relationshipClassNames,
                                        WString                    relatedClassNames,
                                        WString                    propertyNames
                                       )
            : m_requiredDirection (requiredDirection), 
              m_relationshipClassNames (relationshipClassNames),
              m_relatedClassNames (relatedClassNames),
              m_propertyNames (propertyNames)
            {
            }

        //! Destructor.
        ECOBJECTS_EXPORT                              ~RelatedPropertiesSpecification (void);

        //! Reads specification from XML.
        ECOBJECTS_EXPORT bool                         ReadXml (BeXmlNodeP xmlNode);

        //! Writes related properties to xml node.
        ECOBJECTS_EXPORT void                         WriteXml (BeXmlNodeP parentXmlNode);

        //! Returns direction of relationship that should be selected in the query.
        ECOBJECTS_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const  { return m_requiredDirection; }

        //! Relationship class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT WStringCR                    GetRelationshipClassNames (void) const     { return m_relationshipClassNames; }

        //! Related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT WStringCR                    GetRelatedClassNames (void) const          { return m_relatedClassNames; }

        //! Property names separated by comma. RelatedClasses are required if related properties are specified.
        //! These properties of RelatedClasses will be selected in the ECQuery and shown next to the parent ECInstance (the same row).
        //! If PropertyNames are not specified ALL visible properties will be selected. "_none_" keyword can be used to suppress all properties.
        ECOBJECTS_EXPORT WStringCR                    GetPropertyNames (void) const              { return m_propertyNames; }

        //! Nested related properties, that will be shown next to ECInstance proerties (the same row for example).
        ECOBJECTS_EXPORT RelatedPropertiesSpecificationList& GetNestedRelatedProperties (void)   { return m_nestedRelatedPropertiesSpecification; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE