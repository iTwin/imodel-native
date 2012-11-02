/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/RelatedPropertiesSpecification.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
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
        RequiredRelationDirection  m_requiredDirection;
        WString                    m_relationshipSchemaName;
        WString                    m_relationshipClassNames;
        WString                    m_relatedSchemaName;
        WString                    m_relatedClassNames;

    public:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT RelatedPropertiesSpecification ()
            : m_requiredDirection (RequiredRelationDirection_Both), m_relationshipSchemaName (L""),
              m_relationshipClassNames (L""), m_relatedSchemaName (L""), m_relatedClassNames (L"")
            {
            }

        ECOBJECTS_EXPORT RelatedPropertiesSpecification 
                                       (
                                        RequiredRelationDirection  requiredDirection,
                                        WString                    relationshipSchemaName,
                                        WString                    relationshipClassNames,
                                        WString                    relatedSchemaName,
                                        WString                    relatedClassNames
                                       )
            : m_requiredDirection (requiredDirection), m_relationshipSchemaName (relationshipSchemaName),
              m_relationshipClassNames (relationshipClassNames), m_relatedSchemaName (relatedSchemaName),
              m_relatedClassNames (relatedClassNames)
            {
            }

        //! Reads specification from XML.
        ECOBJECTS_EXPORT bool                         ReadXml (BeXmlNodeP xmlNode);

        //! Writes related properties to xml node.
        ECOBJECTS_EXPORT void                         WriteXml (BeXmlNodeP parentXmlNode);

        //! Returns direction of relationship that should be selected in the query.
        ECOBJECTS_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const  { return m_requiredDirection; }

        //! Schema name of specified relationship classes.
        ECOBJECTS_EXPORT WStringCR                    GetRelationshipSchemaName (void) const     { return m_relationshipSchemaName; }

        //! Relationship class names separated by comma that should be used by this specification.
        ECOBJECTS_EXPORT WStringCR                    GetRelationshipClassNames (void) const     { return m_relationshipClassNames; }

        //! Schema name of specified related classes.
        ECOBJECTS_EXPORT WStringCR                    GetRelatedSchemaName (void) const          { return m_relatedSchemaName; }

        //! Related class names separated by comma that should be used by this specification.
        ECOBJECTS_EXPORT WStringCR                    GetRelatedClassNames (void) const          { return m_relatedClassNames; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE