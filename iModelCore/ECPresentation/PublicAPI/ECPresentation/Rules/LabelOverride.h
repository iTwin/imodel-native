/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/PresentationRule.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Label and Description override rule implementation. This rule is used to override default
label and description generation algorithm.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE LabelOverride : ConditionalCustomizationRule
{
    DEFINE_T_SUPER(ConditionalCustomizationRule)

private:
    Utf8String m_label;
    Utf8String m_description;

protected:
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
    ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(BeJsConst json) override;
    ECPRESENTATION_EXPORT void _WriteJson(BeJsValue json) const override;

    //! Accept nested customization rule visitor
    ECPRESENTATION_EXPORT void _Accept(CustomizationRuleVisitor& visitor) const override;

    //! Computes rule hash.
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    //! Clones rule.
    virtual CustomizationRule* _Clone() const override {return new LabelOverride(*this);}

public:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT LabelOverride ();

    //! Constructor.
    ECPRESENTATION_EXPORT LabelOverride (Utf8StringCR condition, int priority, Utf8StringCR label, Utf8StringCR description);

    //! Label override value. Can be ECExpression string. If value is not set it will not affect original value defined by schema.
    ECPRESENTATION_EXPORT Utf8StringCR        GetLabel (void) const;

    //! Set label override value. Can be ECExpression string.
    ECPRESENTATION_EXPORT void             SetLabel (Utf8String value);

    //! Description override value. Can be ECExpression string. If value is not set it will not affect original value defined by schema.
    ECPRESENTATION_EXPORT Utf8StringCR        GetDescription (void) const;

    //! Set description override value. Can be ECExpression string.
    ECPRESENTATION_EXPORT void             SetDescription (Utf8String value);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
