/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/StyleOverride.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
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
        ECOBJECTS_EXPORT virtual CharCP   _GetXmlElementName ();
        ECOBJECTS_EXPORT virtual bool     _ReadXml (BeXmlNodeP xmlNode) override;
        ECOBJECTS_EXPORT virtual void     _WriteXml (BeXmlNodeP xmlNode) override;

    public:
        ECOBJECTS_EXPORT StyleOverride ()
            : PresentationRule (), m_foreColor (L""), m_backColor (L""), m_fontStyle (L"")
            {
            }

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