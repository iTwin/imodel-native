/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/RelatedPropertiesSpecification.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/RelatedInstanceNodesSpecification.h>
#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Related properties specification. It allow to extend a content ECQuery to include 
properties of related classes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelatedPropertiesSpecification : PresentationRuleSpecification
    {
    private:
        RequiredRelationDirection          m_requiredDirection;
        Utf8String                         m_relationshipClassNames;
        Utf8String                         m_relatedClassNames;
        Utf8String                         m_propertyNames;
        RelatedPropertiesSpecificationList m_nestedRelatedPropertiesSpecification;

    protected:
        //! Allows the visitor to visit this specification.
        ECOBJECTS_EXPORT virtual void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT RelatedPropertiesSpecification ();

        //! Constructor.
        ECOBJECTS_EXPORT RelatedPropertiesSpecification 
                                       (
                                        RequiredRelationDirection  requiredDirection,
                                        Utf8String                 relationshipClassNames,
                                        Utf8String                 relatedClassNames,
                                        Utf8String                 propertyNames
                                       );

        //! Destructor.
        ECOBJECTS_EXPORT                              ~RelatedPropertiesSpecification (void);

        //! Reads specification from XML.
        ECOBJECTS_EXPORT bool                         ReadXml (BeXmlNodeP xmlNode);

        //! Writes related properties to xml node.
        ECOBJECTS_EXPORT void                         WriteXml (BeXmlNodeP parentXmlNode) const;

        //! Returns direction of relationship that should be selected in the query.
        ECOBJECTS_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const;

        //! Relationship class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT Utf8StringCR                 GetRelationshipClassNames (void) const;

        //! Related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT Utf8StringCR                 GetRelatedClassNames (void) const;

        //! Property names separated by comma. RelatedClasses are required if related properties are specified.
        //! These properties of RelatedClasses will be selected in the ECQuery and shown next to the parent ECInstance (the same row).
        //! If PropertyNames are not specified ALL visible properties will be selected. "_none_" keyword can be used to suppress all properties.
        ECOBJECTS_EXPORT Utf8StringCR                 GetPropertyNames (void) const;

        //! Nested related properties, that will be shown next to ECInstance proerties (the same row for example).
        ECOBJECTS_EXPORT RelatedPropertiesSpecificationList const& GetNestedRelatedProperties() const;
        
        //! Nested related properties, that will be shown next to ECInstance proerties (the same row for example).
        ECOBJECTS_EXPORT RelatedPropertiesSpecificationList& GetNestedRelatedPropertiesR();
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
