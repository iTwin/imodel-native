 /*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/ChildNodeSpecification.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>
#include <ECPresentation/RulesDriven/Rules/RelatedInstanceNodesSpecification.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
This specification returns all instance nodes available in the repository.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE AllRelatedInstanceNodesSpecification : public ChildNodeSpecification
    {
    private:
        bool                       m_groupByClass;
        bool                       m_groupByRelationship;
        bool                       m_groupByLabel;
        int                        m_skipRelatedLevel;
        Utf8String                 m_supportedSchemas;
        RequiredRelationDirection  m_requiredDirection;

    protected:
        //! Allows the visitor to visit this specification.
        ECPRESENTATION_EXPORT void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;
        
        ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationRuleSpecification const& other) const override;

        ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
        ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
        ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;
        
        ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
        ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;
        
        //! Clones this specification.
        ChildNodeSpecification* _Clone() const override {return new AllRelatedInstanceNodesSpecification(*this);}

        //! Computes specification hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT AllRelatedInstanceNodesSpecification ();
        
        //! Constructor.
        //! @deprecated Use AllRelatedInstanceNodesSpecification(int, ChildrenHint, bool, bool, bool, bool, int, Utf8StringCR)
        ECPRESENTATION_EXPORT AllRelatedInstanceNodesSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, bool hideIfNoChildren,
            bool groupByClass, bool groupByRelationship, bool groupByLabel, int skipRelatedLevel, Utf8StringCR supportedSchemas);

        //! Constructor.
        ECPRESENTATION_EXPORT AllRelatedInstanceNodesSpecification (int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy, bool hideIfNoChildren,
            bool groupByClass, bool groupByLabel, int skipRelatedLevel, Utf8StringCR supportedSchemas);

        //! Returns true if grouping by class should be applied.
        ECPRESENTATION_EXPORT bool                         GetGroupByClass (void) const;

        //! Sets the GroupByClass value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetGroupByClass (bool value);

        //! Returns true if grouping by relationship should be applied.
        //! @deprecated
        ECPRESENTATION_EXPORT bool                         GetGroupByRelationship (void) const;

        //! Sets the GroupByRelationship value. Can be boolean.
        //! @deprecated
        ECPRESENTATION_EXPORT void                         SetGroupByRelationship (bool value);

        //! Returns true if grouping by label should be applied.
        ECPRESENTATION_EXPORT bool                         GetGroupByLabel (void) const;

        //! Sets the GroupByLabel value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetGroupByLabel (bool value);

        //! Returns level of related instances to skip.
        ECPRESENTATION_EXPORT int                          GetSkipRelatedLevel (void) const;
        
        //! Sets the SkipRelatedLevel value. Can be int.
        ECPRESENTATION_EXPORT void                         SetSkipRelatedLevel (int value);

        //! Returns supported schemas that should be used by this specification.
        ECPRESENTATION_EXPORT Utf8StringCR                 GetSupportedSchemas (void) const;

        //! Sets the SupportedSchemas value. Can be WString.
        ECPRESENTATION_EXPORT void                         SetSupportedSchemas (Utf8String value);

        //! Returns direction of relationship that should be selected in the query.
        ECPRESENTATION_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const;

        //! Sets direction of relationship that should be selected in the query.
        ECPRESENTATION_EXPORT void                         SetRequiredRelationDirection (RequiredRelationDirection requiredDirection);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
