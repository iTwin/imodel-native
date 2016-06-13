/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ContentRule.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/PresentationRule.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef bvector<ContentSpecificationCP> ContentSpecificationList;

/*---------------------------------------------------------------------------------**//**
ContentRule defines rules for generating content for selected items.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentRule : public PresentationRule
    {
    private:
        ContentSpecificationList  m_specifications;
        Utf8String                   m_customControl;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP                 _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                   _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                   _WriteXml (BeXmlNodeP xmlNode) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT ContentRule ();

        //! Constructor.
        ECOBJECTS_EXPORT ContentRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled);

        //! Destructor.
        ECOBJECTS_EXPORT                                ~ContentRule (void);

        //! Collection ContentSpecifications that will be used to provide content.
        ECOBJECTS_EXPORT ContentSpecificationList const& GetSpecifications(void) const;
        
        //! Collection ContentSpecifications that will be used to provide content.
        ECOBJECTS_EXPORT ContentSpecificationList&      GetSpecificationsR();

        //! Returns display type of custom control which should display content of this rule.
        ECOBJECTS_EXPORT Utf8StringCR                      GetCustomControl (void);

        //! Sets display type of custom control which should display content of this rule.
        ECOBJECTS_EXPORT void                           SetCustomControl (Utf8StringCR customControl);
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
