/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/CustomizationHelper.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
        ECClassCP boundPropertyClass = m_context.GetConnection().GetECDb().Schemas().GetClass(m_node.GetKey()->AsECInstanceNodeKey()->GetECClassId());
        if (nullptr == boundPropertyClass || !boundPropertyClass->IsEntityClass())
            {
            BeAssert(false);
            return false;
            }
        ECPropertyCP boundProperty = boundPropertyClass->GetPropertyP(rule->GetPropertyName().c_str());
        if (nullptr == boundProperty || !boundProperty->GetIsPrimitive())
            {
            BeAssert(false);
            return false;
            }
        ECValue boundValue = ECInstancesHelper::GetValue(m_context.GetConnection(), *boundPropertyClass, m_node.GetKey()->AsECInstanceNodeKey()->GetInstanceId(), *boundProperty);
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
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCustomizer::NavNodeCustomizer(RulesDrivenProviderContextCR context, JsonNavNodeCR node, JsonNavNodeCP parentNode, ICustomizablePropertiesSetter const& setter)
    : m_context(context), m_node(node), m_parentNode(parentNode), m_setter(setter), m_ecdbSymbolsContext(nullptr)
    {
    if (m_context.IsQueryContext())
        m_ecdbSymbolsContext = new ECDbExpressionSymbolContext(m_context.GetConnection().GetECDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCustomizer::~NavNodeCustomizer()
    {
    DELETE_AND_CLEAR(m_ecdbSymbolsContext);
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

    NavNodeExtendedData(node).SetIsCustomized(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomizationHelper::Customize(ContentProviderContextCR context, ContentSetItemR item)
    {
    ContentSetItemExtendedData extendedData(item);
    if (extendedData.IsCustomized())
        return;

    if (item.GetKeys().size() > 1)
        {
        // note: the record consists of multiple merged ECInstance values - not clear
        // which one should be used for customizing
        return;
        }

    ECClassInstanceKeyCR itemKey = item.GetKeys().front();
    Utf8String locale = context.IsLocalizationContext() ? context.GetLocale() : "";
    JsonNavNodePtr node = context.GetNodesFactory().CreateECInstanceNode(context.GetConnection(), locale, itemKey.GetClass()->GetId(), itemKey.GetId(), "");
    NavNodeExtendedData(*node).SetRelatedInstanceKeys(extendedData.GetRelatedInstanceKeys());
    // create temporary key
    node->SetNodeKey(*NavNodesHelper::CreateNodeKey(context.GetConnection(), *node, bvector<Utf8String>()));
    ContentSetItemPropertiesSetter setter(item);
    NavNodeCustomizer customizer(context, *node, nullptr, setter);
    customizer.ApplyImageIdOverride();

    extendedData.SetIsCustomized(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomizationHelper::NotifyCheckedStateChanged(IConnectionCR connection, JsonNavNodeCR node, bool isChecked)
    {
    NavNodeExtendedData extendedData(node);
    if (extendedData.HasCheckboxBoundPropertyName())
        {
        if (extendedData.IsCheckboxBoundPropertyInversed())
            isChecked = !isChecked;

        ECClassCP boundPropertyClass = connection.GetECDb().Schemas().GetClass(node.GetKey()->AsECInstanceNodeKey()->GetECClassId());
        if (nullptr == boundPropertyClass || !boundPropertyClass->IsEntityClass())
            {
            BeAssert(false);
            return;
            }
        ECPropertyCP boundProperty = boundPropertyClass->GetPropertyP(extendedData.GetCheckboxBoundPropertyName());
        if (nullptr == boundProperty || !boundProperty->GetIsPrimitive())
            {
            BeAssert(false);
            return;
            }

        BeAssert(!boundProperty->GetIsReadOnly());
        BeAssert(!connection.IsReadOnly());
        BeAssert(!connection.GetECDb().GetECDbSettings().RequiresECCrudWriteToken());

        ECInstancesHelper::SetValue(connection, *boundPropertyClass, node.GetKey()->AsECInstanceNodeKey()->GetInstanceId(),
            *boundProperty, ECValue(isChecked));

        connection.GetECDb().SaveChanges();
        }
    }
