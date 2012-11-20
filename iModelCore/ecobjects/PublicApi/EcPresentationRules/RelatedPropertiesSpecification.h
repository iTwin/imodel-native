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
        WString                    m_relationshipClassNames;
        WString                    m_relatedClassNames;

    public:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT RelatedPropertiesSpecification ()
            : m_requiredDirection (RequiredRelationDirection_Both), m_relationshipClassNames (L""), m_relatedClassNames (L"")
            {
            }

        ECOBJECTS_EXPORT RelatedPropertiesSpecification 
                                       (
                                        RequiredRelationDirection  requiredDirection,
                                        WString                    relationshipClassNames,
                                        WString                    relatedClassNames
                                       )
            : m_requiredDirection (requiredDirection), 
              m_relationshipClassNames (relationshipClassNames),
              m_relatedClassNames (relatedClassNames)
            {
            }

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

    };

END_BENTLEY_ECOBJECT_NAMESPACE