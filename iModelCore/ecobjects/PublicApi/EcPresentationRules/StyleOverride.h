/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/StyleOverride.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Node style override rule implementation. This rule is used to override default node style.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct StyleOverride : public PresentationRule
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        WString m_foreColor;
        WString m_backColor;
        WString m_fontStyle;

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
        ECOBJECTS_EXPORT StyleOverride ();

        //! Constructor.
        ECOBJECTS_EXPORT StyleOverride (WStringCR condition, int priority, WStringCR foreColor, WStringCR backColor, WStringCR fontStyle);

        //! Foreground color override value. Can be ECExpression string. If value is not set it will not affect original value.
        ECOBJECTS_EXPORT WStringCR        GetForeColor (void) const;

        //! Set foreground color override value. Can be ECExpression string. If value is not set it will not affect original value.
        ECOBJECTS_EXPORT void             SetForeColor (WString value);

        //! Background color override value. Can be ECExpression string. If value is not set it will not affect original value.
        ECOBJECTS_EXPORT WStringCR        GetBackColor (void) const;

        //! FontStyle override value. Can be ECExpression string. If value is not set it will not affect original value.
        ECOBJECTS_EXPORT WStringCR        GetFontStyle (void) const;

    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
