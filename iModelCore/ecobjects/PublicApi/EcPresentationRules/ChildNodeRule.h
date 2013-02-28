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

/*__PUBLISH_SECTION_START__*/

typedef bvector<ChildNodeSpecificationP> ChildNodeSpecificationList;
typedef bvector<SubConditionP>           SubConditionList;

/*---------------------------------------------------------------------------------**//**
SubCondition can be used in ChildNodeRule or RootNodeRule in order to separate 
specifications by using sub-conditions.
* @bsiclass                                     Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct SubCondition
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        WString                    m_condition;
        SubConditionList           m_subConditions;
        ChildNodeSpecificationList m_specifications;

    public:
        /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT SubCondition ()
            : m_condition (L"")
            {
            }

        ECOBJECTS_EXPORT SubCondition (WStringCR condition)
            : m_condition (condition)
            {
            }

        //! Destructor.
        ECOBJECTS_EXPORT                                ~SubCondition (void);

        //! Reads SubCondition from xml node.
        ECOBJECTS_EXPORT bool                           ReadXml (BeXmlNodeP xmlNode);

        //! Writes SubCondition to xml node.
        ECOBJECTS_EXPORT void                           WriteXml (BeXmlNodeP parentXmlNode);

        //! Returns sub-condition string.
        ECOBJECTS_EXPORT WStringCR                      GetCondition (void)         { return m_condition;  }

        //! Collection of sub-conditions that can be used to separate specifications.
        ECOBJECTS_EXPORT SubConditionList&              GetSubConditions (void)     { return m_subConditions;  }

        //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
        ECOBJECTS_EXPORT ChildNodeSpecificationList&    GetSpecifications (void)    { return m_specifications; }

    };

/*---------------------------------------------------------------------------------**//**
ChildNodeRule defines rules for generating child nodes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChildNodeRule : public PresentationRule
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        RuleTargetTree             m_targetTree;
        SubConditionList           m_subConditions;
        ChildNodeSpecificationList m_specifications;

    protected:
        /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual CharCP                 _GetXmlElementName ();
        ECOBJECTS_EXPORT virtual bool                   _ReadXml (BeXmlNodeP xmlNode) override;
        ECOBJECTS_EXPORT virtual void                   _WriteXml (BeXmlNodeP xmlNode) override;

    public:
        ECOBJECTS_EXPORT ChildNodeRule ()
            : PresentationRule ()
            {
            }

        ECOBJECTS_EXPORT ChildNodeRule (WStringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree)
            : PresentationRule (condition, priority, onlyIfNotHandled), m_targetTree (targetTree)
            {
            }

        //! Destructor.
        ECOBJECTS_EXPORT                                ~ChildNodeRule (void);

        //! Returns target tree for which rule should be applied.
        ECOBJECTS_EXPORT RuleTargetTree                 GetTargetTree (void)        { return m_targetTree; }

        //! Collection of sub-conditions that can be used to separate specifications.
        ECOBJECTS_EXPORT SubConditionList&              GetSubConditions (void)     { return m_subConditions;  }

        //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
        ECOBJECTS_EXPORT ChildNodeSpecificationList&    GetSpecifications (void)    { return m_specifications; }

    };

/*---------------------------------------------------------------------------------**//**
RootNodeRule defines rules for generating root nodes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct RootNodeRule : public ChildNodeRule
    {
    /*__PUBLISH_SECTION_END__*/
    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual CharCP                 _GetXmlElementName () override;

    public:
        ECOBJECTS_EXPORT RootNodeRule ()
            : ChildNodeRule ()
            {
            }

        ECOBJECTS_EXPORT RootNodeRule (WStringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree)
            : ChildNodeRule (condition, priority, onlyIfNotHandled, targetTree)
            {
            }
    };

END_BENTLEY_ECOBJECT_NAMESPACE