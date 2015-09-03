/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/LabelOverride.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/PresentationRule.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Label and Description override rule implementation. This rule is used to override default 
label and description generation algorithm.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct LabelOverride : public PresentationRule
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        Utf8String m_label;
        Utf8String m_description;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP   _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool     _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void     _WriteXml (BeXmlNodeP xmlNode) override;

    /*__PUBLISH_SECTION_START__*/
    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT LabelOverride ();

        //! Constructor.
        ECOBJECTS_EXPORT LabelOverride (Utf8StringCR condition, int priority, Utf8StringCR label, Utf8StringCR description);

        //! Label override value. Can be ECExpression string. If value is not set it will not affect original value defined by schema.
        ECOBJECTS_EXPORT Utf8StringCR        GetLabel (void) const;

        //! Set label override value. Can be ECExpression string. 
        ECOBJECTS_EXPORT void             SetLabel (Utf8String value);

        //! Description override value. Can be ECExpression string. If value is not set it will not affect original value defined by schema.
        ECOBJECTS_EXPORT Utf8StringCR        GetDescription (void) const;

        //! Set description override value. Can be ECExpression string. 
        ECOBJECTS_EXPORT void             SetDescription (Utf8String value);
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
