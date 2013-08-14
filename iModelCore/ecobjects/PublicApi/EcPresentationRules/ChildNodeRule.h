/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ChildNodeRule.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef bvector<ChildNodeSpecificationP> ChildNodeSpecificationList;
typedef bvector<SubConditionP>           SubConditionList;

/*---------------------------------------------------------------------------------**//**
SubCondition can be used in ChildNodeRule or RootNodeRule in order to separate 
specifications by using sub-conditions.
* @bsiclass                                     Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct SubCondition
    {
    private:
        WString                    m_condition;
        SubConditionList           m_subConditions;
        ChildNodeSpecificationList m_specifications;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT SubCondition ();

        //! Constructor.
        ECOBJECTS_EXPORT SubCondition (WStringCR condition);

        //! Destructor.
        ECOBJECTS_EXPORT                                ~SubCondition (void);

        //! Reads SubCondition from xml node.
        ECOBJECTS_EXPORT bool                           ReadXml (BeXmlNodeP xmlNode);

        //! Writes SubCondition to xml node.
        ECOBJECTS_EXPORT void                           WriteXml (BeXmlNodeP parentXmlNode);

        //! Returns sub-condition string.
        ECOBJECTS_EXPORT WStringCR                      GetCondition (void);

        //! Collection of sub-conditions that can be used to separate specifications.
        ECOBJECTS_EXPORT SubConditionList&              GetSubConditions (void);

        //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
        ECOBJECTS_EXPORT ChildNodeSpecificationList&    GetSpecifications (void);

    };

/*---------------------------------------------------------------------------------**//**
ChildNodeRule defines rules for generating child nodes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChildNodeRule : public PresentationRule
    {
    private:
        RuleTargetTree             m_targetTree;
        SubConditionList           m_subConditions;
        ChildNodeSpecificationList m_specifications;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP                 _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                   _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                   _WriteXml (BeXmlNodeP xmlNode) override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT ChildNodeRule ();

        //! Constructor.
        ECOBJECTS_EXPORT ChildNodeRule (WStringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree);

        //! Destructor.
        ECOBJECTS_EXPORT                                ~ChildNodeRule (void);

        //! Returns target tree for which rule should be applied.
        ECOBJECTS_EXPORT RuleTargetTree                 GetTargetTree (void);

        //! Collection of sub-conditions that can be used to separate specifications.
        ECOBJECTS_EXPORT SubConditionList&              GetSubConditions (void);

        //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
        ECOBJECTS_EXPORT ChildNodeSpecificationList&    GetSpecifications (void);

    };

/*---------------------------------------------------------------------------------**//**
RootNodeRule defines rules for generating root nodes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct RootNodeRule : public ChildNodeRule
    {
    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP                 _GetXmlElementName () override;
        bool m_autoExpand;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT RootNodeRule ();

        //! Constructor.
        ECOBJECTS_EXPORT RootNodeRule (WStringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree, bool autoExpand);

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                   _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                   _WriteXml (BeXmlNodeP xmlNode) override;

        //! Returns flag which determines if nodes have to be automatically expanded.
        ECOBJECTS_EXPORT bool                           GetAutoExpand (void);
    };

END_BENTLEY_ECOBJECT_NAMESPACE