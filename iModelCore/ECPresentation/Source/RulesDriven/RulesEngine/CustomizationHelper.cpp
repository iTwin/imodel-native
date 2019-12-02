/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "CustomizationHelper.h"
#include "NavNodeProviders.h"
#include "ContentProviders.h"
#include "NavNodesCache.h"
#include "ECExpressionContextsProvider.h"
#include "ExtendedData.h"
#include "LoggingHelper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionContext& NavNodeCustomizer::GetNodeExpressionContext()
    {
    if (m_nodeExpressionContext.IsNull())
        {
        ECExpressionContextsProvider::CustomizationRulesContextParameters params(m_node, m_parentNode,
            m_context.GetConnection(), m_context.GetLocale(), m_context.GetUserSettings(), &m_context.GetUsedSettingsListener());
        m_nodeExpressionContext = ECExpressionContextsProvider::GetCustomizationRulesContext(params);
        }
    return *m_nodeExpressionContext;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodeCustomizer::ApplyLabelAndDescriptionOverride(bool customizeLabel)
    {
    bool didOverride = false;
    RulesPreprocessor preprocessor(m_context.GetConnections(), m_context.GetConnection(), m_context.GetRuleset(), m_context.GetLocale(),
        m_context.GetUserSettings(), &m_context.GetUsedSettingsListener(), m_context.GetECExpressionsCache());
    RulesPreprocessor::CustomizationRuleParameters params(m_node, m_parentNode);
    LabelOverrideCP labelOverride = preprocessor.GetLabelOverride(params);
    if (nullptr != labelOverride)
        {
        ECValue value;
        Utf8String valueStr;
        if (customizeLabel && !labelOverride->GetLabel().empty()
            && ECExpressionsHelper(m_context.GetECExpressionsCache()).EvaluateECExpression(value, labelOverride->GetLabel(), GetNodeExpressionContext())
            && value.CanConvertToPrimitiveType(PRIMITIVETYPE_String) && value.ConvertPrimitiveToString(valueStr))
            {
            m_setter._SetLabel(valueStr);
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
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodeCustomizer::ApplyStyleOverride()
    {
    bool didOverride = false;
    RulesPreprocessor preprocessor(m_context.GetConnections(), m_context.GetConnection(), m_context.GetRuleset(), m_context.GetLocale(),
        m_context.GetUserSettings(), &m_context.GetUsedSettingsListener(), m_context.GetECExpressionsCache());
    RulesPreprocessor::CustomizationRuleParameters params(m_node, m_parentNode);
    StyleOverrideCP styleOverride = preprocessor.GetStyleOverride(params);
    if (nullptr != styleOverride)
        {
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
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodeCustomizer::ApplyImageIdOverride()
    {
    bool didOverride = false;
    RulesPreprocessor preprocessor(m_context.GetConnections(), m_context.GetConnection(), m_context.GetRuleset(), m_context.GetLocale(),
        m_context.GetUserSettings(), &m_context.GetUsedSettingsListener(), m_context.GetECExpressionsCache());
    RulesPreprocessor::CustomizationRuleParameters params(m_node, m_parentNode);
    ImageIdOverrideCP imageIdOverride = preprocessor.GetImageIdOverride(params);
    if (nullptr != imageIdOverride)
        {
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
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodeCustomizer::ApplyLocalization()
    {
    if (m_context.IsLocalizationContext())
        {
        bool didLocalize = false;
        LocalizationHelper helper(m_context.GetLocalizationProvider(), m_context.GetLocale(), &m_context.GetRuleset());

        Utf8String label = m_node.GetLabel();
        if (!label.empty() && helper.LocalizeString(label))
            {
            m_setter._SetLabel(label);
            didLocalize = true;
            }

        Utf8String description = m_node.GetDescription();
        if (!description.empty() && helper.LocalizeString(description))
            {
            m_setter._SetDescription(description);
            didLocalize = true;
            }

        return didLocalize;
        }

    LoggingHelper::LogMessage(Log::Localization, "Localization is not available as the localization provider is not set", NativeLogging::LOG_ERROR, true);
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodeCustomizer::ApplyCheckboxRules()
    {
    RulesPreprocessor preprocessor(m_context.GetConnections(), m_context.GetConnection(), m_context.GetRuleset(), m_context.GetLocale(),
        m_context.GetUserSettings(), &m_context.GetUsedSettingsListener(), m_context.GetECExpressionsCache());
    RulesPreprocessor::CustomizationRuleParameters params(m_node, m_parentNode);
    CheckBoxRuleCP rule = preprocessor.GetCheckboxRule(params);
    if (nullptr == rule)
        return false;

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
            {
            BeAssert(false);
            return false;
            }
        ECValue boundValue = ECInstancesHelper::GetValue(m_context.GetConnection(), *instanceKeyP->GetClass(), instanceKeyP->GetId(), *boundProperty);
        if (!boundValue.IsBoolean())
            {
            BeAssert(false);
            return false;
            }

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
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodeCustomizer::ApplyExtendedDataRules()
    {
    RulesPreprocessor preprocessor(m_context.GetConnections(), m_context.GetConnection(), m_context.GetRuleset(), m_context.GetLocale(),
        m_context.GetUserSettings(), &m_context.GetUsedSettingsListener(), m_context.GetECExpressionsCache());
    RulesPreprocessor::CustomizationRuleParameters params(m_node, m_parentNode);
    bvector<ExtendedDataRuleCP> rules = preprocessor.GetExtendedDataRules(params);
    bool didAddExtendedData = false;
    for (ExtendedDataRuleCP rule : rules)
        {
        for (auto entry : rule->GetItemsMap())
            {
            Utf8StringCR key = entry.first;
            Utf8StringCR valueExpr = entry.second;
            ECValue value;
            if (ECExpressionsHelper(m_context.GetECExpressionsCache()).EvaluateECExpression(value, valueExpr, GetNodeExpressionContext()))
                {
                m_setter._AddExtendedData(key, value);
                didAddExtendedData = true;
                }
            }
        }
    return didAddExtendedData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCustomizer::NavNodeCustomizer(RulesDrivenProviderContextCR context, JsonNavNodeCR node, JsonNavNodeCP parentNode, ICustomizablePropertiesSetter const& setter)
    : m_context(context), m_node(node), m_parentNode(parentNode), m_setter(setter)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCustomizer::~NavNodeCustomizer()
    {
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2016
+===============+===============+===============+===============+===============+======*/
struct JsonNavNodePropertiesSetter : ICustomizablePropertiesSetter
{
private:
    JsonNavNodeR m_node;
public:
    JsonNavNodePropertiesSetter(JsonNavNodeR node) : m_node(node) {}
    void _SetLabel(Utf8StringCR label) const override {m_node.SetLabel(label.c_str());}
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
* @bsiclass                                     Grigas.Petraitis                07/2016
+===============+===============+===============+===============+===============+======*/
struct ContentSetItemPropertiesSetter : ICustomizablePropertiesSetter
{
private:
    ContentSetItemR m_item;
public:
    ContentSetItemPropertiesSetter(ContentSetItemR item) : m_item(item) {}
    void _SetLabel(Utf8StringCR label) const override {}
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
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomizationHelper::Customize(NavNodesProviderContextCR context, JsonNavNodeR node, bool customizeLabel)
    {
    NavNodeExtendedData extendedData(node);
    if (extendedData.IsCustomized())
        return;

    JsonNavNodeCPtr parentNode;
    if (extendedData.HasVirtualParentId())
        parentNode = context.GetNodesCache().GetNode(extendedData.GetVirtualParentId());

    JsonNavNodePropertiesSetter setter(node);
    NavNodeCustomizer customizer(context, node, parentNode.get(), setter);
    customizer.ApplyLabelAndDescriptionOverride(customizeLabel);
    customizer.ApplyStyleOverride();
    customizer.ApplyImageIdOverride();
    customizer.ApplyCheckboxRules();
    customizer.ApplyLocalization();
    customizer.ApplyExtendedDataRules();

    NavNodeExtendedData(node).SetIsCustomized(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomizationHelper::Customize(ContentProviderContextCR context, ContentDescriptorCR descriptor, ContentSetItemR item)
    {
    ContentSetItemExtendedData extendedData(item);
    if (extendedData.IsCustomized())
        return;

    if (item.GetKeys().empty())
        {
        // note: the record has no keys when we're selecting distinct values. Multiple
        // instances can have that value and it's not clear which one should be used
        // for customizing
        return;
        }

    if (item.GetKeys().size() > 1)
        {
        // note: the record consists of multiple merged ECInstance values - not clear
        // which one should be used for customizing
        return;
        }

    ECClassInstanceKeyCR itemKey = item.GetKeys().front();
    Utf8String locale = context.IsLocalizationContext() ? context.GetLocale() : "";
    JsonNavNodePtr node = context.GetNodesFactory().CreateECInstanceNode(context.GetConnection().GetId(), locale, itemKey.GetClass()->GetId(), itemKey.GetId(), "");
    NavNodeExtendedData(*node).SetRelatedInstanceKeys(extendedData.GetRelatedInstanceKeys());
    node->SetNodeKey(*NavNodesHelper::CreateFakeNodeKey(context.GetConnection(), *node));
    ContentSetItemPropertiesSetter setter(item);
    NavNodeCustomizer customizer(context, *node, nullptr, setter);
    if (descriptor.ShowImages())
        customizer.ApplyImageIdOverride();
    customizer.ApplyExtendedDataRules();

    extendedData.SetIsCustomized(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
NodeArtifacts CustomizationHelper::EvaluateArtifacts(NavNodesProviderContextCR context, JsonNavNodeCR node)
    {
    JsonNavNodeCPtr parentNode = context.GetVirtualParentNode();

    RulesPreprocessor preprocessor(context.GetConnections(), context.GetConnection(), context.GetRuleset(), context.GetLocale(),
        context.GetUserSettings(), &context.GetUsedSettingsListener(), context.GetECExpressionsCache());
    bvector<NodeArtifactsRuleCP> rules = preprocessor.GetNodeArtifactRules(RulesPreprocessor::CustomizationRuleParameters(node, parentNode.get()));

    ECExpressionContextsProvider::CustomizationRulesContextParameters evaluationParams(node, parentNode.get(),
        context.GetConnection(), context.GetLocale(), context.GetUserSettings(), &context.GetUsedSettingsListener());
    ExpressionContextPtr evaluationContext = ECExpressionContextsProvider::GetCustomizationRulesContext(evaluationParams);

    NodeArtifacts artifacts;
    for (NodeArtifactsRuleCP rule : rules)
        {
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
