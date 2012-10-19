/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ChildNodeRule.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/

typedef bvector<ChildNodeSpecificationP> ChildNodeSpecificationList;

/*---------------------------------------------------------------------------------**//**
ChildNodeRule defines rules for generating child nodes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChildNodeRule : public PresentationRule
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        RuleTargetTree             m_targetTree;
        ChildNodeSpecificationList m_specifications;

    protected:
        /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual bool                   _ReadXml (BeXmlNodeP xmlNode) override;

    public:
        ECOBJECTS_EXPORT ChildNodeRule ()
            : PresentationRule ()
            {
            }

        ECOBJECTS_EXPORT ChildNodeRule (WStringCR condition, int priority, bool onlyIfNotHadled, RuleTargetTree targetTree)
            : PresentationRule (condition, priority, onlyIfNotHadled), m_targetTree (targetTree)
            {
            }

        //! Destructor.
        ECOBJECTS_EXPORT                                ~ChildNodeRule (void);

        //! Returns target tree for which rule should be applied.
        ECOBJECTS_EXPORT RuleTargetTree                 GetTargetTree (void)        { return m_targetTree; }

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
    public:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT RootNodeRule ()
            : ChildNodeRule ()
            {
            }

        ECOBJECTS_EXPORT RootNodeRule (WStringCR condition, int priority, bool onlyIfNotHadled, RuleTargetTree targetTree)
            : ChildNodeRule (condition, priority, onlyIfNotHadled, targetTree)
            {
            }
    };

END_BENTLEY_ECOBJECT_NAMESPACE