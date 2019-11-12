/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

typedef bvector<ContentSpecificationP> ContentSpecificationList;

/*---------------------------------------------------------------------------------**//**
ContentRule defines rules for generating content for selected items.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentRule : public ConditionalPresentationRule
    {
    private:
        ContentSpecificationList  m_specifications;
        Utf8String                m_customControl;

    protected:
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
        ECPRESENTATION_EXPORT ContentRule ();

        //! Constructor.
        ECPRESENTATION_EXPORT ContentRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled);

        //! Copy constructor.
        ECPRESENTATION_EXPORT ContentRule(ContentRuleCR);

        //! Destructor.
        ECPRESENTATION_EXPORT                                ~ContentRule (void);

        //! Collection ContentSpecifications that will be used to provide content.
        ECPRESENTATION_EXPORT ContentSpecificationList const& GetSpecifications(void) const;
        
        //! Adds ContentSpecification that will be used to provide content.
        ECPRESENTATION_EXPORT void AddSpecification(ContentSpecificationR specification);

        //! Returns display type of custom control which should display content of this rule.
        ECPRESENTATION_EXPORT Utf8StringCR                      GetCustomControl (void);

        //! Sets display type of custom control which should display content of this rule.
        ECPRESENTATION_EXPORT void                           SetCustomControl (Utf8StringCR customControl);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
