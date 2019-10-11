/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRulesTypes.h>
#include <ECPresentation/RulesDriven/Rules/CustomizationRules.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

typedef bvector<ChildNodeSpecificationP> ChildNodeSpecificationList;
typedef bvector<SubConditionP>           SubConditionList;
typedef bvector<CustomizationRuleP>      ChildNodeCustomizationRuleList;

/*---------------------------------------------------------------------------------**//**
SubCondition can be used in ChildNodeRule or RootNodeRule in order to separate 
specifications by using sub-conditions.
* @bsiclass                                     Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct SubCondition : HashableBase
    {
    private:
        Utf8String                 m_condition;
        SubConditionList           m_subConditions;
        ChildNodeSpecificationList m_specifications;

    protected:
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT SubCondition ();

        //! Constructor.
        ECPRESENTATION_EXPORT SubCondition (Utf8StringCR condition);
        
        //! Copy constructor.
        ECPRESENTATION_EXPORT SubCondition(SubConditionCR);

        //! Destructor.
        ECPRESENTATION_EXPORT ~SubCondition (void);

        //! Does shallow comparison between this subcondition and other subcondition
        ECPRESENTATION_EXPORT bool ShallowEqual(SubConditionCR other) const;

        //! Reads SubCondition from xml node.
        ECPRESENTATION_EXPORT bool ReadXml (BeXmlNodeP xmlNode);

        //! Writes SubCondition to xml node.
        ECPRESENTATION_EXPORT void WriteXml (BeXmlNodeP parentXmlNode) const;

        //! Reads SubCondition from json.
        ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);

        //! Writes SubCondition to json.
        ECPRESENTATION_EXPORT Json::Value WriteJson() const;

        //! Returns sub-condition string.
        ECPRESENTATION_EXPORT Utf8StringCR GetCondition (void);

        //! Collection of sub-conditions that can be used to separate specifications.
        ECPRESENTATION_EXPORT SubConditionList const& GetSubConditions (void) const;

        //! Add sub-condition.
        ECPRESENTATION_EXPORT void AddSubCondition(SubConditionR subCondition);

        //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
        ECPRESENTATION_EXPORT ChildNodeSpecificationList const& GetSpecifications (void) const;

        //! Add ChildNodeSpecification.
        ECPRESENTATION_EXPORT void AddSpecification(ChildNodeSpecificationR specification);
    };

/*---------------------------------------------------------------------------------**//**
ChildNodeRule defines rules for generating child nodes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ChildNodeRule : public ConditionalPresentationRule
    {
    private:
        RuleTargetTree             m_targetTree;
        SubConditionList           m_subConditions;
        ChildNodeSpecificationList m_specifications;
        ChildNodeCustomizationRuleList m_customizationRules;
        bool                       m_stopFurtherProcessing;

    protected:
        ECPRESENTATION_EXPORT virtual bool _ShallowEqual(PresentationKeyCR other) const override;

        ECPRESENTATION_EXPORT virtual Utf8CP _GetXmlElementName() const override;
        ECPRESENTATION_EXPORT virtual bool _ReadXml(BeXmlNodeP xmlNode) override;
        ECPRESENTATION_EXPORT virtual void _WriteXml(BeXmlNodeP xmlNode) const override;
        
        ECPRESENTATION_EXPORT virtual Utf8CP _GetJsonElementType() const override;
        ECPRESENTATION_EXPORT virtual bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT virtual void _WriteJson(JsonValueR json) const override;

        //! Computes rule hash.
        ECPRESENTATION_EXPORT virtual MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT ChildNodeRule ();

        //! Constructor.
        ECPRESENTATION_EXPORT ChildNodeRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree = TargetTree_Both);

        //! Copy constructor.
        ECPRESENTATION_EXPORT ChildNodeRule(ChildNodeRuleCR);

        //! Destructor.
        ECPRESENTATION_EXPORT virtual ~ChildNodeRule (void);

        //! Returns target tree for which rule should be applied.
        ECPRESENTATION_EXPORT RuleTargetTree                 GetTargetTree (void) const;

        //! Collection of sub-conditions that can be used to separate specifications.
        ECPRESENTATION_EXPORT SubConditionList const&        GetSubConditions (void) const;

        //! Add sub-condition.
        ECPRESENTATION_EXPORT void AddSubCondition(SubConditionR subCondition);

        //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
        ECPRESENTATION_EXPORT ChildNodeSpecificationList const&  GetSpecifications (void) const;

        //! Add ChildNodesSpecification.
        ECPRESENTATION_EXPORT void AddSpecification(ChildNodeSpecificationR specification);

        //! Nested customization rules applied on nodes created by this rule
        ECPRESENTATION_EXPORT ChildNodeCustomizationRuleList const&   GetCustomizationRules (void) const;

        //! Add customization rule.
        ECPRESENTATION_EXPORT void AddCustomizationRule(CustomizationRuleR customizationRule);

        //! If this flag is set, this rule will stop any further processing of rules.
        //! This helps in cases when recursion suppression is needed.
        //! Note: such rules should not contain any SubConditions or specifications,
        //! because they will not be applied.
        ECPRESENTATION_EXPORT void                           SetStopFurtherProcessing (bool stopFurtherProcessing);

        //! If this flag is set, this rule will stop any further processing of rules.
        ECPRESENTATION_EXPORT bool                           GetStopFurtherProcessing (void) const;

    };

/*---------------------------------------------------------------------------------**//**
RootNodeRule defines rules for generating root nodes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE RootNodeRule : public ChildNodeRule
    {
    private:
        bool m_autoExpand;

    protected:
        ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;

        ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
        ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
        ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;
        
        ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
        ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

        //! Computes rule hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT RootNodeRule ();

        //! Constructor.
        ECPRESENTATION_EXPORT RootNodeRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled, 
            RuleTargetTree targetTree = TargetTree_Both, bool autoExpand = false);

        //! Returns flag which determines if nodes have to be automatically expanded.
        ECPRESENTATION_EXPORT bool                           GetAutoExpand (void) const;
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
