/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/PresentationRule.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
ImageId override rule implementation. This rule is used to override default ImageId
generation algorithm.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImageIdOverride : ConditionalCustomizationRule
{
    DEFINE_T_SUPER(ConditionalCustomizationRule)

private:
    Utf8String m_imageIdExpression;

protected:
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
    ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    //! Accecpt nested customization rule visitor
    ECPRESENTATION_EXPORT void _Accept(CustomizationRuleVisitor& visitor) const  override;

    //! Computes rule hash.
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;

    //! Clones rule.
    CustomizationRule* _Clone() const override {return new ImageIdOverride(*this);}

public:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT ImageIdOverride ();

    //! Constructor.
    ECPRESENTATION_EXPORT ImageIdOverride (Utf8StringCR condition, int priority, Utf8StringCR imageIdExpression);

    //! Returns ImageId override ECExpression string.
    ECPRESENTATION_EXPORT Utf8StringCR        GetImageId (void) const;

    //! Set imageId override ECExpression string.
    ECPRESENTATION_EXPORT void             SetImageId (Utf8String value);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
