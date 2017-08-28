/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/StyleOverride.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Node style override rule implementation. This rule is used to override default node style.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE StyleOverride : public CustomizationRule
    {
    private:
        Utf8String m_foreColor;
        Utf8String m_backColor;
        Utf8String m_fontStyle;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECPRESENTATION_EXPORT virtual CharCP   _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT virtual bool     _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT virtual void     _WriteXml (BeXmlNodeP xmlNode) const override;

        //! Accecpt nested customization rule visitor
        ECPRESENTATION_EXPORT void _Accept(CustomizationRuleVisitor& visitor) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT StyleOverride ();

        //! Constructor.
        ECPRESENTATION_EXPORT StyleOverride (Utf8StringCR condition, int priority, Utf8StringCR foreColor, Utf8StringCR backColor, Utf8StringCR fontStyle);

        //! Foreground color override value. Can be ECExpression string. If value is not set it will not affect original value.
        ECPRESENTATION_EXPORT Utf8StringCR        GetForeColor (void) const;

        //! Set foreground color override value. Can be ECExpression string. If value is not set it will not affect original value.
        ECPRESENTATION_EXPORT void             SetForeColor (Utf8String value);

        //! Background color override value. Can be ECExpression string. If value is not set it will not affect original value.
        ECPRESENTATION_EXPORT Utf8StringCR        GetBackColor (void) const;

        //! FontStyle override value. Can be ECExpression string. If value is not set it will not affect original value.
        ECPRESENTATION_EXPORT Utf8StringCR        GetFontStyle (void) const;

    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
