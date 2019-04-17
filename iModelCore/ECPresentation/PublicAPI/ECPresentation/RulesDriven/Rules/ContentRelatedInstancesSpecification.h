/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

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
        ECPRESENTATION_EXPORT void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;
        
        ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
        ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
        ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

        ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
        ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;
        
        //! Clones this content specification.
        ContentSpecification* _Clone() const override {return new ContentRelatedInstancesSpecification(*this);}

        //! Computes specification hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT ContentRelatedInstancesSpecification ();

        //! Constructor.
        ECPRESENTATION_EXPORT ContentRelatedInstancesSpecification 
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
        ECPRESENTATION_EXPORT int                          GetSkipRelatedLevel (void) const;

        //! Sets the SkipRelatedLevel of the specification.
        ECPRESENTATION_EXPORT void                         SetSkipRelatedLevel (int value);

        //! Should the relationship be followed recursively
        //! @note The followed relationship should be recursive (e.g. A to A)
        bool IsRecursive() const {return m_isRecursive;}

        //! Set whether the relationship be followed recursively
        //! @note The followed relationship should be recursive (e.g. A to A)
        void SetIsRecursive(bool value) {m_isRecursive = value;}

        //! InstanceFiler is spacially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results.
        ECPRESENTATION_EXPORT Utf8StringCR                 GetInstanceFilter (void) const;

        //! Sets the instance filter of the specification.
        ECPRESENTATION_EXPORT void                         SetInstanceFilter (Utf8StringCR value); 

        //! Returns direction of relationship that should be selected in the query.
        ECPRESENTATION_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const;

        //! Sets the required direction of the specification.
        ECPRESENTATION_EXPORT void                         SetRequiredRelationDirection (RequiredRelationDirection value);

        //! Relationship class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECPRESENTATION_EXPORT Utf8StringCR                 GetRelationshipClassNames (void) const;

        //! Sets the RelationshipClassNames of the specification.
        ECPRESENTATION_EXPORT void                         SetRelationshipClassNames (Utf8StringCR value);

        //! Related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECPRESENTATION_EXPORT Utf8StringCR                 GetRelatedClassNames (void) const;

        //! Sets the RelatedClassNames of the specification.
        ECPRESENTATION_EXPORT void                         SetRelatedClassNames (Utf8StringCR value);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
