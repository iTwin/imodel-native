/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/StyleOverride.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/
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
    /*__PUBLISH_SECTION_START__*/
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP   _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool     _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void     _WriteXml (BeXmlNodeP xmlNode) override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT StyleOverride ()
            : PresentationRule (), m_foreColor (L""), m_backColor (L""), m_fontStyle (L"")
            {
            }

        //! Constructor.
        ECOBJECTS_EXPORT StyleOverride (WStringCR condition, int priority, WStringCR foreColor, WStringCR backColor, WStringCR fontStyle)
            : PresentationRule (condition, priority, false), 
              m_foreColor (foreColor), m_backColor (backColor), m_fontStyle (fontStyle)
            {
            }

        //! Foreground color override value. Can be ECExpression string. If value is not set it will not affect original value.
        ECOBJECTS_EXPORT WStringCR        GetForeColor (void) const    { return m_foreColor; }

        //! Background color override value. Can be ECExpression string. If value is not set it will not affect original value.
        ECOBJECTS_EXPORT WStringCR        GetBackColor (void) const    { return m_backColor; }

        //! FontStyle override value. Can be ECExpression string. If value is not set it will not affect original value.
        ECOBJECTS_EXPORT WStringCR        GetFontStyle (void) const    { return m_fontStyle; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE