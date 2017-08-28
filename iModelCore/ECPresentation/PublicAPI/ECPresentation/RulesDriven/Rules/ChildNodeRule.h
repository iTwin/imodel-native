/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/ChildNodeRule.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
struct SubCondition
    {
    private:
        Utf8String                 m_condition;
        SubConditionList           m_subConditions;
        ChildNodeSpecificationList m_specifications;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT SubCondition ();

        //! Constructor.
        ECPRESENTATION_EXPORT SubCondition (Utf8StringCR condition);
        
        //! Copy constructor.
        ECPRESENTATION_EXPORT SubCondition(SubConditionCR);

        //! Destructor.
        ECPRESENTATION_EXPORT                                ~SubCondition (void);

        //! Reads SubCondition from xml node.
        ECPRESENTATION_EXPORT bool                           ReadXml (BeXmlNodeP xmlNode);

        //! Writes SubCondition to xml node.
        ECPRESENTATION_EXPORT void                           WriteXml (BeXmlNodeP parentXmlNode) const;

        //! Returns sub-condition string.
        ECPRESENTATION_EXPORT Utf8StringCR                   GetCondition (void);

        //! Collection of sub-conditions that can be used to separate specifications.
        ECPRESENTATION_EXPORT SubConditionList const&        GetSubConditions (void) const;
        ECPRESENTATION_EXPORT SubConditionList&              GetSubConditionsR (void);

        //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
        ECPRESENTATION_EXPORT ChildNodeSpecificationList const& GetSpecifications (void) const;
        ECPRESENTATION_EXPORT ChildNodeSpecificationList&    GetSpecificationsR (void);

    };

/*---------------------------------------------------------------------------------**//**
ChildNodeRule defines rules for generating child nodes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ChildNodeRule : public PresentationRule
    {
    private:
        RuleTargetTree             m_targetTree;
        SubConditionList           m_subConditions;
        ChildNodeSpecificationList m_specifications;
        ChildNodeCustomizationRuleList m_customizationRules;
        bool                       m_stopFurtherProcessing;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECPRESENTATION_EXPORT virtual CharCP                 _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT virtual bool                   _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT virtual void                   _WriteXml (BeXmlNodeP xmlNode) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT ChildNodeRule ();

        //! Constructor.
        ECPRESENTATION_EXPORT ChildNodeRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree);

        //! Copy constructor.
        ECPRESENTATION_EXPORT ChildNodeRule(ChildNodeRuleCR);

        //! Destructor.
        ECPRESENTATION_EXPORT virtual ~ChildNodeRule (void);

        //! Returns target tree for which rule should be applied.
        ECPRESENTATION_EXPORT RuleTargetTree                 GetTargetTree (void) const;

        //! Collection of sub-conditions that can be used to separate specifications.
        ECPRESENTATION_EXPORT SubConditionList&              GetSubConditionsR (void);
        ECPRESENTATION_EXPORT SubConditionList const&        GetSubConditions (void) const;

        //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
        ECPRESENTATION_EXPORT ChildNodeSpecificationList&        GetSpecificationsR (void);
        ECPRESENTATION_EXPORT ChildNodeSpecificationList const&  GetSpecifications (void) const;

        //! Nested customization rules applied on nodes created by this rule
        ECPRESENTATION_EXPORT ChildNodeCustomizationRuleList&         GetCustomizationRulesR (void);
        ECPRESENTATION_EXPORT ChildNodeCustomizationRuleList const&   GetCustomizationRules (void) const;

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
        //! Returns XmlElement name that is used to read/save this rule information.
        ECPRESENTATION_EXPORT virtual CharCP                 _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT virtual bool                   _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT virtual void                   _WriteXml (BeXmlNodeP xmlNode) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT RootNodeRule ();

        //! Constructor.
        ECPRESENTATION_EXPORT RootNodeRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree, bool autoExpand);

        //! Returns flag which determines if nodes have to be automatically expanded.
        ECPRESENTATION_EXPORT bool                           GetAutoExpand (void) const;
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
