/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ContentRule.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
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
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP                 _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                   _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                   _WriteXml (BeXmlNodeP xmlNode) override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT ContentRule ()
            : PresentationRule ()
            {
            }

        //! Constructor.
        ECOBJECTS_EXPORT ContentRule (WStringCR condition, int priority, bool onlyIfNotHandled)
            : PresentationRule (condition, priority, onlyIfNotHandled)
            {
            }

        //! Destructor.
        ECOBJECTS_EXPORT                                ~ContentRule (void);

        //! Collection ContentSpecifications that will be used to provide content.
        ECOBJECTS_EXPORT ContentSpecificationList&      GetSpecifications (void)    { return m_specifications; }
    };

END_BENTLEY_ECOBJECT_NAMESPACE