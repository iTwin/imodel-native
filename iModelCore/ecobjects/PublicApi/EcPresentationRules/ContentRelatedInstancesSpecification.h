/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ContentRelatedInstancesSpecification.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Specification that creates content ECQueries for predefined relationships and/or 
related ECClasses of the selected node.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ContentRelatedInstancesSpecification : public ContentSpecification
    {
    private:
        int                        m_skipRelatedLevel;
        bool                       m_isRecursive;
        Utf8String                 m_instanceFilter;
        RequiredRelationDirection  m_requiredDirection;
        Utf8String                 m_relationshipClassNames;
        Utf8String                 m_relatedClassNames;

    protected:
        //! Allows the visitor to visit this specification.
        ECOBJECTS_EXPORT virtual void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;

        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode) const override;
        
        //! Clones this content specification.
        virtual ContentSpecification* _Clone() const override {return new ContentRelatedInstancesSpecification(*this);}

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT ContentRelatedInstancesSpecification ();

        //! Constructor.
        ECOBJECTS_EXPORT ContentRelatedInstancesSpecification 
                                             (
                                              int                        priority,
                                              int                        skipRelatedLevel,
                                              bool                       isRecursive,
                                              Utf8String                 instanceFilter,
                                              RequiredRelationDirection  requiredDirection,
                                              Utf8String                 relationshipClassNames,
                                              Utf8String                 relatedClassNames
                                             );

        //! Returns level of related instances to skip.
        ECOBJECTS_EXPORT int                          GetSkipRelatedLevel (void) const;

        //! Sets the SkipRelatedLevel of the specification.
        ECOBJECTS_EXPORT void                         SetSkipRelatedLevel (int value);

        //! Should the relationship be followed recursively
        //! @note The followed relationship should be recursive (e.g. A to A)
        bool IsRecursive() const {return m_isRecursive;}

        //! Set whether the relationship be followed recursively
        //! @note The followed relationship should be recursive (e.g. A to A)
        void SetIsRecursive(bool value) {m_isRecursive = value;}

        //! InstanceFiler is spacially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results.
        ECOBJECTS_EXPORT Utf8StringCR                 GetInstanceFilter (void) const;

        //! Sets the instance filter of the specification.
        ECOBJECTS_EXPORT void                         SetInstanceFilter (Utf8StringCR value); 

        //! Returns direction of relationship that should be selected in the query.
        ECOBJECTS_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const;

        //! Sets the required direction of the specification.
        ECOBJECTS_EXPORT void                         SetRequiredRelationDirection (RequiredRelationDirection value);

        //! Relationship class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT Utf8StringCR                 GetRelationshipClassNames (void) const;

        //! Sets the RelationshipClassNames of the specification.
        ECOBJECTS_EXPORT void                         SetRelationshipClassNames (Utf8StringCR value);

        //! Related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT Utf8StringCR                 GetRelatedClassNames (void) const;

        //! Sets the RelatedClassNames of the specification.
        ECOBJECTS_EXPORT void                         SetRelatedClassNames (Utf8StringCR value);
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
