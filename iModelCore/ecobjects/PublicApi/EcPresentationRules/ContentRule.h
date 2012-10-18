/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ContentRule.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/

typedef bvector<ContentSpecificationP> ContentSpecificationList;

/*---------------------------------------------------------------------------------**//**
ContentRule defines rules for generating content for selected items.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentRule : public PresentationRule
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        ContentSpecificationList  m_specifications;

    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual bool                   _ReadXml (BeXmlNodeP xmlNode) override;

    public:
        ECOBJECTS_EXPORT ContentRule ()
            : PresentationRule ()
            {
            }

        ECOBJECTS_EXPORT ContentRule (WStringCR condition, int priority, bool onlyIfNotHadled)
            : PresentationRule (condition, priority, onlyIfNotHadled)
            {
            }

        //! Destructor.
        ECOBJECTS_EXPORT                                ~ContentRule (void);

        //! Collection ContentSpecifications that will be used to provide content.
        ECOBJECTS_EXPORT ContentSpecificationList&      GetSpecifications (void)    { return m_specifications; }
    };

END_BENTLEY_ECOBJECT_NAMESPACE