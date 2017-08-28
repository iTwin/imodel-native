/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/RelatedPropertiesSpecification.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/RelatedInstanceNodesSpecification.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Related properties specification. It allow to extend a content ECQuery to include 
properties of related classes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelatedPropertiesSpecification
    {
    private:
        RequiredRelationDirection          m_requiredDirection;
        Utf8String                         m_relationshipClassNames;
        Utf8String                         m_relatedClassNames;
        Utf8String                         m_propertyNames;
        RelatedPropertiesSpecificationList m_nestedRelatedPropertiesSpecification;
        
    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT RelatedPropertiesSpecification ();
        
        //! Copy constructor.
        ECPRESENTATION_EXPORT RelatedPropertiesSpecification(RelatedPropertiesSpecification const&);

        //! Constructor.
        ECPRESENTATION_EXPORT RelatedPropertiesSpecification 
                                       (
                                        RequiredRelationDirection  requiredDirection,
                                        Utf8String                 relationshipClassNames,
                                        Utf8String                 relatedClassNames,
                                        Utf8String                 propertyNames
                                       );

        //! Destructor.
        ECPRESENTATION_EXPORT                              ~RelatedPropertiesSpecification (void);

        //! Reads specification from XML.
        ECPRESENTATION_EXPORT bool                         ReadXml (BeXmlNodeP xmlNode);

        //! Writes related properties to xml node.
        ECPRESENTATION_EXPORT void                         WriteXml (BeXmlNodeP parentXmlNode) const;

        //! Returns direction of relationship that should be selected in the query.
        ECPRESENTATION_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const;

        //! Relationship class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECPRESENTATION_EXPORT Utf8StringCR                 GetRelationshipClassNames (void) const;

        //! Related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECPRESENTATION_EXPORT Utf8StringCR                 GetRelatedClassNames (void) const;

        //! Set related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECPRESENTATION_EXPORT void                         SetRelatedClassNames(Utf8StringCR);

        //! Property names separated by comma. RelatedClasses are required if related properties are specified.
        //! These properties of RelatedClasses will be selected in the ECQuery and shown next to the parent ECInstance (the same row).
        //! If PropertyNames are not specified ALL visible properties will be selected. "_none_" keyword can be used to suppress all properties.
        ECPRESENTATION_EXPORT Utf8StringCR                 GetPropertyNames (void) const;
        
        //! @see GetPropertyNames
        ECPRESENTATION_EXPORT void                         SetPropertyNames(Utf8StringCR);

        //! Nested related properties, that will be shown next to ECInstance proerties (the same row for example).
        ECPRESENTATION_EXPORT RelatedPropertiesSpecificationList const& GetNestedRelatedProperties() const;
        
        //! Nested related properties, that will be shown next to ECInstance proerties (the same row for example).
        ECPRESENTATION_EXPORT RelatedPropertiesSpecificationList& GetNestedRelatedPropertiesR();
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
