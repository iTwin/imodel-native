/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Specification for specifying single property that is Calculated.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct CalculatedPropertiesSpecification : PrioritizedPresentationKey
{
    DEFINE_T_SUPER(PrioritizedPresentationKey)

private:
    Utf8String m_label;
    Utf8String m_value;

protected:
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP xmlNode) const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR) const override;

public:
    CalculatedPropertiesSpecification() {}
    CalculatedPropertiesSpecification(Utf8String label, int priority, Utf8String value)
        : PrioritizedPresentationKey(priority), m_label(label), m_value(value)
        {}

    //! Get label expression.
    Utf8StringCR GetLabel() const {return m_label;}

    //! Get property value expression.
    Utf8StringCR GetValue() const {return m_value;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
