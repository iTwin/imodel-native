/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "../Content/ContentProviders.h"
#include "../Hierarchies/NavNodeProviders.h"
#include "../Hierarchies/NavNodesHelper.h"
#include "ECExpressions/ECExpressionContextsProvider.h"
#include "CustomizationHelper.h"
#include "ExtendedData.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionContext& NavNodeCustomizer::GetNodeExpressionContext()
    {
    if (m_nodeExpressionContext.IsNull())
        {
        ECExpressionContextsProvider::CustomizationRulesContextParameters params(m_node, m_parentNode,
            m_context.GetConnection(), m_context.GetRulesetVariables(), &m_context.GetUsedVariablesListener());
        m_nodeExpressionContext = ECExpressionContextsProvider::GetCustomizationRulesContext(params);
        }
    return *m_nodeExpressionContext;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodeCustomizer::ApplyLabelAndDescriptionOverride()
    {
    bool didOverride = false;
    IRulesPreprocessor::CustomizationRuleByNodeParameters params(m_node, m_parentNode);
    LabelOverrideCP labelOverride = m_context.GetRulesPreprocessor().GetLabelOverride(params);
    bool customizeLabel = !NavNodeExtendedData(m_node).IsLabelCustomized();
    if (nullptr != labelOverride)
        {
        DiagnosticsHelpers::ReportRule(*labelOverride);
        ECValue value;
        Utf8String valueStr;
        if (customizeLabel && !labelOverride->GetLabel().empty()
            && ECExpressionsHelper(m_context.GetECExpressionsCache()).EvaluateECExpression(value, labelOverride->GetLabel(), GetNodeExpressionContext())
            && value.CanConvertToPrimitiveType(PRIMITIVETYPE_String) && value.ConvertPrimitiveToString(valueStr))
            {
            LabelDefinitionPtr labelDefinition = value.IsString() ? LabelDefinition::FromString(valueStr.c_str()) : LabelDefinition::Create(value, valueStr.c_str());
            m_setter._SetLabelDefinition(*labelDefinition);
            didOverride = true;
            }
        if (!labelOverride->GetDescription().empty()
            && ECExpressionsHelper(m_context.GetECExpressionsCache()).EvaluateECExpression(value, labelOverride->GetDescription(), GetNodeExpressionContext())
            && value.CanConvertToPrimitiveType(PRIMITIVETYPE_String) && value.ConvertPrimitiveToString(valueStr))
            {
            m_setter._SetDescription(valueStr);
            didOverride = true;
            }
        }
    return didOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodeCustomizer::ApplyStyleOverride()
    {
    bool didOverride = false;
    IRulesPreprocessor::CustomizationRuleByNodeParameters params(m_node, m_parentNode);
    StyleOverrideCP styleOverride = m_context.GetRulesPreprocessor().GetStyleOverride(params);
    if (nullptr != styleOverride)
        {
        DiagnosticsHelpers::ReportRule(*styleOverride);
        ECValue value;
        Utf8String valueStr;
        if (!styleOverride->GetForeColor().empty()
            && ECExpressionsHelper(m_context.GetECExpressionsCache()).EvaluateECExpression(value, styleOverride->GetForeColor(), GetNodeExpressionContext())
            && value.CanConvertToPrimitiveType(PRIMITIVETYPE_String) && value.ConvertPrimitiveToString(valueStr))
            {
            m_setter._SetForeColor(valueStr);
            didOverride = true;
            }
        if (!styleOverride->GetBackColor().empty()
            && ECExpressionsHelper(m_context.GetECExpressionsCache()).EvaluateECExpression(value, styleOverride->GetBackColor(), GetNodeExpressionContext())
            && value.CanConvertToPrimitiveType(PRIMITIVETYPE_String) && value.ConvertPrimitiveToString(valueStr))
            {
            m_setter._SetBackColor(valueStr);
            didOverride = true;
            }
        if (!styleOverride->GetFontStyle().empty()
            && ECExpressionsHelper(m_context.GetECExpressionsCache()).EvaluateECExpression(value, styleOverride->GetFontStyle(), GetNodeExpressionContext())
            && value.CanConvertToPrimitiveType(PRIMITIVETYPE_String) && value.ConvertPrimitiveToString(valueStr))
            {
            m_setter._SetFontStyle(valueStr);
            didOverride = true;
            }
        }
    return didOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodeCustomizer::ApplyImageIdOverride()
    {
    bool didOverride = false;
    IRulesPreprocessor::CustomizationRuleByNodeParameters params(m_node, m_parentNode);
    ImageIdOverrideCP imageIdOverride = m_context.GetRulesPreprocessor().GetImageIdOverride(params);
    if (nullptr != imageIdOverride)
        {
        DiagnosticsHelpers::ReportRule(*imageIdOverride);
        ECValue value;
        Utf8String valueStr;
        if (ECExpressionsHelper(m_context.GetECExpressionsCache()).EvaluateECExpression(value, imageIdOverride->GetImageId(), GetNodeExpressionContext())
            && value.CanConvertToPrimitiveType(PRIMITIVETYPE_String) && value.ConvertPrimitiveToString(valueStr))
            {
            m_setter._SetImageId(valueStr);
            didOverride = true;
            }
        }
    return didOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodeCustomizer::ApplyCheckboxRules()
    {
    IRulesPreprocessor::CustomizationRuleByNodeParameters params(m_node, m_parentNode);
    CheckBoxRuleCP rule = m_context.GetRulesPreprocessor().GetCheckboxRule(params);
    if (nullptr == rule)
        return false;

    DiagnosticsHelpers::ReportRule(*rule);
    bool isChecked = false;
    bool isReadOnly = false;
    if (!rule->GetPropertyName().empty() && nullptr != m_node.GetKey()->AsECInstanceNodeKey())
        {
        ECClassInstanceKeyCP instanceKeyP = nullptr;
        ECPropertyCP boundProperty = nullptr;
        for (ECClassInstanceKeyCR instanceKey : m_node.GetKey()->AsECInstanceNodeKey()->GetInstanceKeys())
            {
            instanceKeyP = &instanceKey;
            ECPropertyCP prop = instanceKey.GetClass()->GetPropertyP(rule->GetPropertyName().c_str());
            if (nullptr != prop && prop->GetIsPrimitive())
                {
                boundProperty = prop;
                break;
                }
            }
        if (nullptr == boundProperty)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("None of ECInstances associated with the node have requested property: '%s'", rule->GetPropertyName().c_str()));

        ECValue boundValue = ECInstancesHelper::GetValue(m_context.GetConnection(), *instanceKeyP->GetClass(), instanceKeyP->GetId(), *boundProperty);
        if (!boundValue.IsBoolean() && !boundValue.IsNull())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Value of requested property '%s' is not a boolean", rule->GetPropertyName().c_str()));

        if (boundValue.IsNull())
            isChecked = rule->GetDefaultValue();
        else
            isChecked = rule->GetUseInversedPropertyValue() ? !boundValue.GetBoolean() : boundValue.GetBoolean();
        isReadOnly = m_context.GetConnection().IsReadOnly() || boundProperty->GetIsReadOnly() || boundValue.IsReadOnly();

        m_setter._SetCheckboxBoundInfo(rule->GetPropertyName(), rule->GetUseInversedPropertyValue());
        }
    else
        {
        ECValue value;
        isChecked = rule->GetDefaultValue();
        isReadOnly = (!rule->GetIsEnabled().empty()
            && ECExpressionsHelper(m_context.GetECExpressionsCache()).EvaluateECExpression(value, rule->GetIsEnabled(), GetNodeExpressionContext())
            && value.IsBoolean()
            && !value.GetBoolean());
        }

    m_setter._SetIsCheckboxVisible(true);
    m_setter._SetIsCheckboxEnabled(!isReadOnly);
    m_setter._SetIsChecked(isChecked);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodeCustomizer::ApplyExtendedDataRules()
    {
    IRulesPreprocessor::CustomizationRuleByNodeParameters params(m_node, m_parentNode);
    bvector<ExtendedDataRuleCP> rules = m_context.GetRulesPreprocessor().GetExtendedDataRules(params);
    bool didAddExtendedData = false;
    bset<Utf8String> usedKeys;
    for (ExtendedDataRuleCP rule : rules)
        {
        DiagnosticsHelpers::ReportRule(*rule);
        for (auto entry : rule->GetItemsMap())
            {
            Utf8StringCR key = entry.first;
            Utf8StringCR valueExpr = entry.second;
            ECValue value;
            if (usedKeys.find(key) == usedKeys.end() && ECExpressionsHelper(m_context.GetECExpressionsCache()).EvaluateECExpression(value, valueExpr, GetNodeExpressionContext()))
                {
                m_setter._AddExtendedData(key, value);
                didAddExtendedData = true;
                }
            usedKeys.insert(key);
            }
        }
    return didAddExtendedData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCustomizer::NavNodeCustomizer(RulesDrivenProviderContextCR context, NavNodeCR node, NavNodeCP parentNode, ICustomizablePropertiesSetter const& setter)
    : m_context(context), m_node(node), m_parentNode(parentNode), m_setter(setter)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCustomizer::~NavNodeCustomizer()
    {
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavNodePropertiesSetter : ICustomizablePropertiesSetter
{
private:
    NavNodeR m_node;
public:
    NavNodePropertiesSetter(NavNodeR node) : m_node(node) {}
    void _SetLabelDefinition(LabelDefinitionCR label) const override {m_node.SetLabelDefinition(label);}
    void _SetDescription(Utf8StringCR description) const override {m_node.SetDescription(description.c_str());}
    void _SetForeColor(Utf8StringCR color) const override {m_node.SetForeColor(color.c_str());}
    void _SetBackColor(Utf8StringCR color) const override {m_node.SetBackColor(color.c_str());}
    void _SetFontStyle(Utf8StringCR style) const override {m_node.SetFontStyle(style.c_str());}
    void _SetImageId(Utf8StringCR imageId) const override {m_node.SetImageId(imageId.c_str());}
    void _SetIsCheckboxVisible(bool value) const override {m_node.SetIsCheckboxVisible(value);}
    void _SetIsCheckboxEnabled(bool value) const override {m_node.SetIsCheckboxEnabled(value);}
    void _SetIsChecked(bool value) const override {m_node.SetIsChecked(value);}
    void _SetCheckboxBoundInfo(Utf8StringCR propertyName, bool inverse) const override
        {
        NavNodeExtendedData extendedData(m_node);
        extendedData.SetCheckboxBoundPropertyName(propertyName.c_str());
        extendedData.SetCheckboxBoundPropertyInversed(inverse);
        }
    void _AddExtendedData(Utf8StringCR key, ECValueCR value) const override {m_node.AddUsersExtendedData(key.c_str(), value);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentSetItemPropertiesSetter : ICustomizablePropertiesSetter
{
private:
    ContentSetItemR m_item;
public:
    ContentSetItemPropertiesSetter(ContentSetItemR item) : m_item(item) {}
    void _SetLabelDefinition(LabelDefinitionCR label) const override {}
    void _SetDescription(Utf8StringCR description) const override {}
    void _SetForeColor(Utf8StringCR color) const override {}
    void _SetBackColor(Utf8StringCR color) const override {}
    void _SetFontStyle(Utf8StringCR style) const override {}
    void _SetImageId(Utf8StringCR imageId) const override {m_item.SetImageId(imageId);}
    void _SetIsCheckboxVisible(bool value) const override {}
    void _SetIsCheckboxEnabled(bool value) const override {}
    void _SetIsChecked(bool value) const override {}
    void _SetCheckboxBoundInfo(Utf8StringCR propertyName, bool inverse) const override {}
    void _AddExtendedData(Utf8StringCR key, ECValueCR value) const override {m_item.AddUsersExtendedData(key.c_str(), value);}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomizationHelper::Customize(NavNodesProviderContextCR context, NavNodeCP parentNode, NavNodeR node)
    {
    NavNodeExtendedData extendedData(node);
    if (extendedData.IsCustomized())
        return;

    NavNodePropertiesSetter setter(node);
    NavNodeCustomizer customizer(context, node, parentNode, setter);
    customizer.ApplyLabelAndDescriptionOverride();
    customizer.ApplyStyleOverride();
    customizer.ApplyImageIdOverride();
    customizer.ApplyCheckboxRules();
    customizer.ApplyExtendedDataRules();

    NavNodeExtendedData(node).SetIsCustomized(true);
    NavNodeExtendedData(node).SetIsLabelCustomized(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomizationHelper::Customize(ContentProviderContextCR context, ContentDescriptorCR descriptor, ContentSetItemR item)
    {
    ContentSetItemExtendedData extendedData(item);
    if (extendedData.IsCustomized())
        return;

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
    if (item.GetKeys().empty())
        {
        // note: the record has no keys when we're selecting distinct values. Multiple
        // instances can have that value and it's not clear which one should be used
        // for customizing
        return;
        }
#endif

    if (item.GetKeys().size() > 1)
        {
        // note: the record consists of multiple merged ECInstance values - not clear
        // which one should be used for customizing
        return;
        }

    ECClassInstanceKeyCR itemKey = item.GetKeys().front();
    NavNodePtr node = context.GetNodesFactory().CreateECInstanceNode(context.GetConnection(), "", nullptr, itemKey.GetClass()->GetId(), itemKey.GetId(), *LabelDefinition::Create());
    NavNodeExtendedData(*node).SetRelatedInstanceKeys(extendedData.GetRelatedInstanceKeys());
    ContentSetItemPropertiesSetter setter(item);
    NavNodeCustomizer customizer(context, *node, nullptr, setter);
    if (descriptor.ShowImages())
        customizer.ApplyImageIdOverride();
    customizer.ApplyExtendedDataRules();

    extendedData.SetIsCustomized(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NodeArtifacts CustomizationHelper::EvaluateArtifacts(NavNodesProviderContextCR context, NavNodeCR node)
    {
    NavNodeCPtr parentNode = context.GetVirtualParentNode();

    bvector<NodeArtifactsRuleCP> rules = context.GetRulesPreprocessor().GetNodeArtifactRules(IRulesPreprocessor::CustomizationRuleByNodeParameters(node, parentNode.get()));

    ECExpressionContextsProvider::CustomizationRulesContextParameters evaluationParams(node, parentNode.get(),
        context.GetConnection(), context.GetRulesetVariables(), &context.GetUsedVariablesListener());
    ExpressionContextPtr evaluationContext = ECExpressionContextsProvider::GetCustomizationRulesContext(evaluationParams);

    NodeArtifacts artifacts;
    for (NodeArtifactsRuleCP rule : rules)
        {
        DiagnosticsHelpers::ReportRule(*rule);
        for (auto entry : rule->GetItemsMap())
            {
            Utf8StringCR key = entry.first;
            Utf8StringCR valueExpr = entry.second;
            ECValue value;
            if (ECExpressionsHelper(context.GetECExpressionsCache()).EvaluateECExpression(value, valueExpr, *evaluationContext))
                artifacts.Insert(key, value);
            }
        }
    return artifacts;
    }
