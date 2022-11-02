/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct CustomRendererSpecification : NoXmlSupport<PresentationKey>
{
    DEFINE_T_SUPER(NoXmlSupport<PresentationKey>)

private:
    Utf8String m_rendererName;

protected:
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR) const override;

public:
    CustomRendererSpecification() {}
    CustomRendererSpecification(Utf8String rendererName): m_rendererName(rendererName) {}

    Utf8StringCR GetRendererName() const {return m_rendererName;}
    void SetRendererName(Utf8String value) {m_rendererName = value; InvalidateHash();}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
