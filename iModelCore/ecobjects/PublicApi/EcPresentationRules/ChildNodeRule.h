/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ChildNodeRule.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/PresentationRule.h>
#include <ECPresentationRules/PresentationRulesTypes.h>

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
        Utf8String                 m_condition;
        SubConditionList           m_subConditions;
        ChildNodeSpecificationList m_specifications;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT SubCondition ();

        //! Constructor.
        ECOBJECTS_EXPORT SubCondition (Utf8StringCR condition);

        //! Destructor.
        ECOBJECTS_EXPORT                                ~SubCondition (void);

        //! Reads SubCondition from xml node.
        ECOBJECTS_EXPORT bool                           ReadXml (BeXmlNodeP xmlNode);

        //! Writes SubCondition to xml node.
        ECOBJECTS_EXPORT void                           WriteXml (BeXmlNodeP parentXmlNode) const;

        //! Returns sub-condition string.
        ECOBJECTS_EXPORT Utf8StringCR                   GetCondition (void);

        //! Collection of sub-conditions that can be used to separate specifications.
        ECOBJECTS_EXPORT SubConditionList const&        GetSubConditions (void) const;
        ECOBJECTS_EXPORT SubConditionList&              GetSubConditionsR (void);

        //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
        ECOBJECTS_EXPORT ChildNodeSpecificationList const& GetSpecifications (void) const;
        ECOBJECTS_EXPORT ChildNodeSpecificationList&    GetSpecificationsR (void);

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
        bool                       m_stopFurtherProcessing;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP                 _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                   _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                   _WriteXml (BeXmlNodeP xmlNode) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT ChildNodeRule ();

        //! Constructor.
        ECOBJECTS_EXPORT ChildNodeRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree);

        //! Destructor.
        ECOBJECTS_EXPORT                                ~ChildNodeRule (void);

        //! Returns target tree for which rule should be applied.
        ECOBJECTS_EXPORT RuleTargetTree                 GetTargetTree (void) const;

        //! Collection of sub-conditions that can be used to separate specifications.
        ECOBJECTS_EXPORT SubConditionList&              GetSubConditionsR (void);
        ECOBJECTS_EXPORT SubConditionList const&        GetSubConditions (void) const;

        //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
        ECOBJECTS_EXPORT ChildNodeSpecificationList&        GetSpecificationsR (void);
        ECOBJECTS_EXPORT ChildNodeSpecificationList const&  GetSpecifications (void) const;

        //! If this flag is set, this rule will stop any further processing of rules.
        //! This helps in cases when recursion suppression is needed.
        //! Note: such rules should not contain any SubConditions or specifications,
        //! because they will not be applied.
        ECOBJECTS_EXPORT void                           SetStopFurtherProcessing (bool stopFurtherProcessing);

        //! If this flag is set, this rule will stop any further processing of rules.
        ECOBJECTS_EXPORT bool                           GetStopFurtherProcessing (void) const;

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
        ECOBJECTS_EXPORT virtual CharCP                 _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                   _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                   _WriteXml (BeXmlNodeP xmlNode) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT RootNodeRule ();

        //! Constructor.
        ECOBJECTS_EXPORT RootNodeRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree, bool autoExpand);

        //! Returns flag which determines if nodes have to be automatically expanded.
        ECOBJECTS_EXPORT bool                           GetAutoExpand (void) const;
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
