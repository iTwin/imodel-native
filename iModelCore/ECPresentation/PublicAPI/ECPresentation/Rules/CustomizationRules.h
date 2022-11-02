/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/PresentationRule.h>
#include <ECPresentation/Rules/PresentationRulesTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE CustomizationRuleVisitor
{
    friend struct GroupingRule;
    friend struct ImageIdOverride;
    friend struct LabelOverride;
    friend struct StyleOverride;
    friend struct CheckBoxRule;
    friend struct SortingRule;
    friend struct InstanceLabelOverride;
    friend struct ExtendedDataRule;
    friend struct NodeArtifactsRule;

protected:
    virtual void _Visit(GroupingRuleCR rule) {}
    virtual void _Visit(ImageIdOverrideCR rule) {}
    virtual void _Visit(LabelOverrideCR rule) {}
    virtual void _Visit(StyleOverrideCR rule) {}
    virtual void _Visit(CheckBoxRuleCR rule) {}
    virtual void _Visit(SortingRuleCR rule) {}
    virtual void _Visit(InstanceLabelOverrideCR rule) {}
    virtual void _Visit(ExtendedDataRuleCR rule) {}
    virtual void _Visit(NodeArtifactsRuleCR rule) {}
};

/*---------------------------------------------------------------------------------**//**
* Base class for CustomizationRules.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE CustomizationRule : PresentationRule
{
protected:
    virtual void _Accept(CustomizationRuleVisitor& visitor) const {}
    virtual CustomizationRule* _Clone() const = 0;

public:
    ECPRESENTATION_EXPORT static CustomizationRuleP Create(JsonValueCR);
    CustomizationRule() {}
    CustomizationRule(int priority, bool onlyIfNotHandled) : PresentationRule(priority, onlyIfNotHandled) {}
    void Accept(CustomizationRuleVisitor& visitor) const { _Accept(visitor); }
    CustomizationRule* Clone() const {return _Clone();}
};

/*---------------------------------------------------------------------------------**//**
* Base class for CustomizationRules with conditions.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ConditionalCustomizationRule : CustomizationRule
{
    DEFINE_T_SUPER(CustomizationRule)

private:
    Utf8String m_condition;

protected:
    ECPRESENTATION_EXPORT virtual bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT virtual void _WriteXml (BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT virtual bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT virtual void _WriteJson(JsonValueR json) const override;

    //! Compute rule hash.
    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT virtual bool _ShallowEqual(PresentationKeyCR other) const override;

public:
    ConditionalCustomizationRule() {}

    ConditionalCustomizationRule(Utf8String condition, int priority, bool onlyIfNotHandled)
        : CustomizationRule(priority, onlyIfNotHandled), m_condition(condition)
        {}

    ECPRESENTATION_EXPORT Utf8StringCR GetCondition() const;

    ECPRESENTATION_EXPORT void SetCondition(Utf8String value);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
