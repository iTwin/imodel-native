/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/CustomizationRules.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRulesTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE
struct EXPORT_VTABLE_ATTRIBUTE CustomizationRuleVisitor{
    friend struct GroupingRule;
    friend struct ImageIdOverride;
    friend struct LabelOverride;
    friend struct StyleOverride;
    friend struct CheckBoxRule;
    friend struct RenameNodeRule;
    friend struct SortingRule;

protected:
    virtual void _Visit(GroupingRuleCR rule) {}
    virtual void _Visit(ImageIdOverrideCR rule) {}
    virtual void _Visit(LabelOverrideCR rule) {}
    virtual void _Visit(StyleOverrideCR rule) {}
    virtual void _Visit(CheckBoxRuleCR rule) {}
    virtual void _Visit(RenameNodeRuleCR rule) {}
    virtual void _Visit(SortingRuleCR rule) {}
    };

struct EXPORT_VTABLE_ATTRIBUTE CustomizationRule : public PresentationRule {
protected:
    virtual void _Accept(CustomizationRuleVisitor& visitor) const {}

public:
    //! Constructor. It is used to initialize the rule with default settings.
    CustomizationRule() {}

    CustomizationRule(Utf8StringCR condition, int priority, bool onlyIfNotHandled) : PresentationRule(condition, priority, onlyIfNotHandled) {}

    void Accept(CustomizationRuleVisitor& visitor) const { _Accept(visitor); }
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE