/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Node style override rule implementation. This rule is used to override default node style.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE StyleOverride : ConditionalCustomizationRule
{
    DEFINE_T_SUPER(ConditionalCustomizationRule)

private:
    Utf8String m_foreColor;
    Utf8String m_backColor;
    Utf8String m_fontStyle;

protected:
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
    ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    //! Accecpt nested customization rule visitor
    ECPRESENTATION_EXPORT void _Accept(CustomizationRuleVisitor& visitor) const override;

    //! Computes rule hash
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;

    //! Clones rule.
    CustomizationRule* _Clone() const override {return new StyleOverride(*this);}
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
