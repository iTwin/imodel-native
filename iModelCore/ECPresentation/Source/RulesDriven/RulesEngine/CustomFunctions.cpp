/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/CustomFunctions.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECDb/ECDbExpressionSymbolContext.h>
#include "CustomFunctions.h"
#include "RulesPreprocessor.h"
#include "JsonNavNode.h"
#include "ECExpressionContextsProvider.h"
#include "LocalizationHelper.h"
#include "QueryContracts.h"
#include "NavigationQuery.h"
#include "QueryBuilder.h"
#include "LoggingHelper.h"
#include "NavNodeProviders.h"
#include <sstream>
#include <unordered_set>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void ApplyLocalization(Utf8StringR str, CustomFunctionsContext const& context)
    {
    if (nullptr == context.GetLocalizationProvider())
        {
        static bool s_didOutputWarning = false;
        if (!s_didOutputWarning)
            {
            LoggingHelper::LogMessage(Log::Localization, "Localization is not available as the localization provider is not set", NativeLogging::LOG_ERROR);
            s_didOutputWarning = true;
            }
        return;
        }

    LocalizationHelper helper(*context.GetLocalizationProvider(), &context.GetRuleset());
    if (!helper.LocalizeString(str))
        LoggingHelper::LogMessage(Log::Localization, Utf8PrintfString("Localization provider failed to localize string: '%s'", str.c_str()).c_str(), NativeLogging::LOG_ERROR);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct ECPresentation::ScalarFunction : BeSQLite::ScalarFunction
{
private:
    CustomFunctionsManager const& m_manager;
protected:
    ScalarFunction(Utf8CP name, int argsCount, DbValueType returnType, CustomFunctionsManager const& manager)
        : BeSQLite::ScalarFunction(name, argsCount, returnType), m_manager(manager)
        {}
    CustomFunctionsContext& GetContext() const {return m_manager.GetCurrentContext();}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
template<typename CacheType>
struct CachingScalarFunction : ECPresentation::ScalarFunction, ICustomFunctionsContextListener
    {
    CachingScalarFunction(Utf8CP name, int argsCount, DbValueType returnType, CustomFunctionsManager const& manager)
        : ECPresentation::ScalarFunction(name, argsCount, returnType, manager)
        {}
    CacheType& GetCache()
        {
        void* cacheP = GetContext().GetCache(GetName());
        if (nullptr == cacheP)
            {
            cacheP = new CacheType();
            GetContext().InsertCache(GetName(), cacheP);
            GetContext().AddListener(*this);
            }
        return *static_cast<CacheType*>(cacheP);
        }
    void _OnContextDisposed(CustomFunctionsContext& context) override
        {
        void* cacheP = context.GetCache(GetName());
        if (nullptr != cacheP)
            {
            delete static_cast<CacheType*>(cacheP);
            context.RemoveCache(GetName());
            }
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
+===============+===============+===============+===============+===============+======*/
struct ECPresentation::AggregateFunction : BeSQLite::AggregateFunction
{
private:
    CustomFunctionsManager const& m_manager;
protected:
    AggregateFunction(Utf8CP name, int argsCount, DbValueType returnType, CustomFunctionsManager const& manager)
        : BeSQLite::AggregateFunction(name, argsCount, returnType), m_manager(manager)
        {}
    CustomFunctionsContext& GetContext() const {return m_manager.GetCurrentContext();}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessLabelOverrides(Utf8StringR label, CustomFunctionsContext const& context, ECClassId const& classId, ECInstanceId const& instanceId, Utf8CP relatedInstanceInfo)
    {
    JsonNavNodePtr thisNode = context.GetNodesFactory().CreateECInstanceNode(context.GetConnection(), classId, instanceId, "");
    thisNode->SetNodeKey(*NavNodesHelper::CreateNodeKey(context.GetConnection(), *thisNode, bvector<Utf8String>()));
    NavNodesHelper::AddRelatedInstanceInfo(*thisNode, relatedInstanceInfo);

    // look for label override
    ECDbExpressionSymbolContext ecdbSymbols(context.GetSchemaHelper().GetConnection().GetECDb());
    RulesPreprocessor::CustomizationRuleParameters params(context.GetConnections(), context.GetConnection(), *thisNode, context.GetParentNode(),
        context.GetRuleset(), context.GetUserSettings(), context.GetUsedUserSettingsListener(), context.GetECExpressionsCache());
    LabelOverrideCP labelOverride = RulesPreprocessor::GetLabelOverride(params);
    if (nullptr != labelOverride && !labelOverride->GetLabel().empty())
        {
        // evaluate the ECExpression to get the label
        ECExpressionContextsProvider::CustomizationRulesContextParameters params(*thisNode, context.GetParentNode(),
            context.GetConnection(), context.GetUserSettings(), context.GetUsedUserSettingsListener());
        ExpressionContextPtr expressionContext = ECExpressionContextsProvider::GetCustomizationRulesContext(params);
        ECValue value;
        if (ECExpressionsHelper(context.GetECExpressionsCache()).EvaluateECExpression(value, labelOverride->GetLabel(), *expressionContext) && value.IsPrimitive() && value.ConvertPrimitiveToString(label))
            {
            if (nullptr != context.GetUsedClassesListener())
                {
                UsedClassesHelper::NotifyListenerWithUsedClasses(*context.GetUsedClassesListener(),
                    context.GetSchemaHelper(), context.GetECExpressionsCache(), labelOverride->GetLabel());
                }
            }
        }
    }

/*=================================================================================**//**
* Parameters:
* - ECClassId
* - ECInstanceId
* - Display label property value of this instance
* - Related instances' info JSON (serialized to string). Format: [{"Alias":"related_1","ECClassId":1,"ECInstanceId":1},{"Alias":"related_2","ECClassId":2,"ECInstanceId":2},...]
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct GetECInstanceDisplayLabelScalar : CachingScalarFunction<bmap<ECInstanceKey, Utf8String>>
    {
    GetECInstanceDisplayLabelScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_GetECInstanceDisplayLabel, 4, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        BeAssert(4 == nArgs);
        ECInstanceId instanceId (args[1].GetValueUInt64());
        ECClassId classId = args[0].GetValueId<ECClassId>();
        ECInstanceKey key(classId, instanceId);

        auto iter = GetCache().find(key);
        if (GetCache().end() == iter)
            {
            Utf8String label;

            if (key.IsValid())
                {
                ProcessLabelOverrides(label, GetContext(), classId, instanceId, args[3].GetValueText());
                if (label.empty())
                    {
                    ECClassCP ecClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(classId);
                    BeAssert(nullptr != ecClass);

                    // if the override didn't apply, look for instance label property
                    if (nullptr != ecClass->GetInstanceLabelProperty())
                        label = args[2].GetValueText();

                    // if all failed, use ECClass display label
                    if (label.empty())
                        label = CommonTools::GetDefaultDisplayLabel(ecClass->GetDisplayLabel(), instanceId.GetValue());
                    }
                }

            if (label.empty())
                label = RULESENGINE_LOCALIZEDSTRING_NotSpecified;

            ApplyLocalization(label, GetContext());
            iter = GetCache().Insert(key, label).first;
            }
        ctx.SetResultText(iter->second.c_str(), (int)iter->second.size(), BeSQLite::DbFunction::Context::CopyData::No);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ApplyProperties(Utf8StringR label, bvector<ECPropertyCP> properties, IECInstanceCR instance)
    {
    ECValue value;
    for (ECPropertyCP ecProperty : properties)
        {           
        if (ECObjectsStatus::Success == instance.GetValue(value, ecProperty->GetName().c_str()))
            {
            value.ConvertPrimitiveToString(label);
            if (!label.empty())
                return true;                            
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessIntanceLabelOverrides(Utf8StringR label, ECClassCR ecClass, bmap<ECClassCP, bvector<ECPropertyCP>> const& labelOverrides, IECInstanceCR instance)
    {
    auto iter = labelOverrides.find(&ecClass);
    if (iter != labelOverrides.end())
        {
        if (ApplyProperties(label, iter->second, instance))
            return;
        }
    for (ECClassCP baseClass : ecClass.GetBaseClasses())
        {
        iter = labelOverrides.find(baseClass);
        if (iter != labelOverrides.end())
            {
            if (ApplyProperties(label, iter->second, instance))
                return;
            }
        ProcessIntanceLabelOverrides(label, *baseClass, labelOverrides, instance);
        if (!label.empty())
            return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessLabelAndInstanceLabelOverrides(Utf8StringR label, CustomFunctionsContext const& context, ECInstanceKeyCR key, ECClassCR ecClass)
    {
    bmap<ECClassCP, bvector<ECPropertyCP>> instanceLabelOverrides = QueryBuilderHelpers::GetMappedLabelOverridingProperties(context.GetSchemaHelper(), context.GetRuleset().GetInstanceLabelOverrides());
    if (!instanceLabelOverrides.empty())
        {
        IECInstancePtr instance;
        ECInstancesHelper::LoadInstance(instance, context.GetSchemaHelper().GetConnection(), key);
        if (instance.IsValid())
            ProcessIntanceLabelOverrides(label, ecClass, instanceLabelOverrides, *instance);
        }

    if (label.empty() && !context.GetRuleset().GetLabelOverrides().empty())
        ProcessLabelOverrides(label, context, key.GetClassId(), key.GetInstanceId(), "");
    }

/*=================================================================================**//**
* Parameters:
* - ECClassId
* - ECInstanceId
* @bsiclass                                                Mantas.Kontrimas    05/2018
+===============+===============+===============+===============+===============+======*/
struct GetRelatedDisplayLabelScalar : CachingScalarFunction<bmap<ECInstanceKey, Utf8String>>
    {
private:
    bool m_applyLocalization;

public:
    GetRelatedDisplayLabelScalar(CustomFunctionsManager const& manager, Utf8CP name, bool applyLocalization = false)
        : CachingScalarFunction(name, 2, DbValueType::TextVal, manager), m_applyLocalization(applyLocalization)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        BeAssert(2 == nArgs);
        ECInstanceId instanceId(args[1].GetValueUInt64());
        ECClassId classId = args[0].GetValueId<ECClassId>();
        ECInstanceKey key(classId, instanceId);
  
        auto iter = GetCache().find(key);
        if (GetCache().end() == iter)
            {
            Utf8String label;
            if (key.IsValid())
                {
                ECClassCP ecClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(classId);
                if (nullptr == ecClass)
                    {
                    BeAssert(false && "Invalid class");
                    ctx.SetResultError("Invalid ECClassId", BE_SQLITE_ERROR);
                    return;
                    }  

                ProcessLabelAndInstanceLabelOverrides(label, GetContext(), key, *ecClass);

                if (label.empty())
                    label = CommonTools::GetDefaultDisplayLabel(ecClass->GetDisplayLabel(), instanceId.GetValue());
                }
            if (label.empty())
                label = RULESENGINE_LOCALIZEDSTRING_NotSpecified;
            if (m_applyLocalization)
                ApplyLocalization(label, GetContext());
            iter = GetCache().Insert(key, label).first;
            }

        ctx.SetResultText(iter->second.c_str(), (int)iter->second.size(), BeSQLite::DbFunction::Context::CopyData::No);
        }
    };

/*=================================================================================**//**
* Parameters:
* - ECClassId
* - Number of grouped instances
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct GetECClassDisplayLabelScalar : CachingScalarFunction<bmap<ECClassId, Utf8String>>
    {
    GetECClassDisplayLabelScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_GetECClassDisplayLabel, 2, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        BeAssert(2 == nArgs);
        ECClassId classId = args[0].GetValueId<ECClassId>();

        auto iter = GetCache().find(classId);
        if (GetCache().end() == iter)
            {
            ECClassCP ecClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(classId);
            BeAssert(nullptr != ecClass);

            Utf8String label;

            // first, look for label override
            JsonNavNodePtr thisNode;
            GroupedInstanceKeysList groupedInstanceKeys;
            groupedInstanceKeys.resize(args[1].GetValueInt64()); // note: this creates a list of invalid ECInstanceKeys. but it's okay as we only need it for count
            if (ecClass->IsRelationshipClass())
                thisNode = GetContext().GetNodesFactory().CreateECRelationshipGroupingNode(GetContext().GetConnection().GetId(), *ecClass->GetRelationshipClassCP(), "", groupedInstanceKeys);
            else
                thisNode = GetContext().GetNodesFactory().CreateECClassGroupingNode(GetContext().GetConnection().GetId(), *ecClass, "", groupedInstanceKeys);

            // create temporary key
            thisNode->SetNodeKey(*NavNodesHelper::CreateNodeKey(GetContext().GetConnection(), *thisNode, bvector<Utf8String>()));

            RulesPreprocessor::CustomizationRuleParameters params(GetContext().GetConnections(), GetContext().GetConnection(), *thisNode, GetContext().GetParentNode(),
                GetContext().GetRuleset(), GetContext().GetUserSettings(), GetContext().GetUsedUserSettingsListener(), GetContext().GetECExpressionsCache());
            LabelOverrideCP labelOverride = RulesPreprocessor::GetLabelOverride(params);
            if (nullptr != labelOverride && !labelOverride->GetLabel().empty())
                {
                ECExpressionContextsProvider::CustomizationRulesContextParameters params(*thisNode, GetContext().GetParentNode(),
                    GetContext().GetConnection(), GetContext().GetUserSettings(), GetContext().GetUsedUserSettingsListener());
                ExpressionContextPtr expressionContext = ECExpressionContextsProvider::GetCustomizationRulesContext(params);
                ECValue value;
                if (ECExpressionsHelper(GetContext().GetECExpressionsCache()).EvaluateECExpression(value, labelOverride->GetLabel(), *expressionContext) && value.IsPrimitive() && value.ConvertPrimitiveToString(label))
                    {
                    if (nullptr != GetContext().GetUsedClassesListener())
                        {
                        UsedClassesHelper::NotifyListenerWithUsedClasses(*GetContext().GetUsedClassesListener(),
                            GetContext().GetSchemaHelper(), GetContext().GetECExpressionsCache(), labelOverride->GetLabel());
                        }
                    }
                }
            if (label.empty())
                {
                // otherwise, use the display label
                label.assign(ecClass->GetDisplayLabel());
                }

            if (label.empty())
                label = RULESENGINE_LOCALIZEDSTRING_NotSpecified;

            ApplyLocalization(label, GetContext());
            iter = GetCache().Insert(classId, label).first;
            }
        ctx.SetResultText(iter->second.c_str(), (int)iter->second.size(), BeSQLite::DbFunction::Context::CopyData::No);
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Tautvydas.Zinys                 10/2016
+===============+===============+===============+===============+===============+======*/
struct ECExpressionScalarCacheKey
    {
    ECClassId m_classId;
    ECInstanceId m_instanceId;
    Utf8String m_expression;
    bool operator<(ECExpressionScalarCacheKey const& other) const
        {
        return m_classId < other.m_classId
            || m_classId == other.m_classId && m_instanceId < other.m_instanceId
            || m_classId == other.m_classId && m_instanceId == other.m_instanceId && m_expression.CompareTo(other.m_expression) < 0;
        }
    };

/*=================================================================================**//**
* Parameters:
* - ECClassId
* - ECInstanceId
* - Expression
* @bsiclass                                     Tautvydas.Zinys                10/2016
+===============+===============+===============+===============+===============+======*/
struct EvaluateECExpressionScalar : CachingScalarFunction<bmap<ECExpressionScalarCacheKey, Utf8String>>
    {
    EvaluateECExpressionScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_EvaluateECExpression, 3, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ECClassId classId = args[0].GetValueId<ECClassId>();
        ECInstanceId instanceId = args[1].GetValueId<ECInstanceId>();
        Utf8CP expression = args[2].GetValueText();
        ECExpressionScalarCacheKey key = {classId, instanceId, expression};

        auto iter = GetCache().find(key);
        if (GetCache().end() == iter)
            {
            JsonNavNodePtr thisNode = GetContext().GetNodesFactory().CreateECInstanceNode(GetContext().GetConnection(), classId, instanceId, "");
            // create temporary key
            thisNode->SetNodeKey(*NavNodesHelper::CreateNodeKey(GetContext().GetConnection(), *thisNode, bvector<Utf8String>()));
            ECExpressionContextsProvider::CalculatedPropertyContextParameters params(*thisNode, GetContext().GetConnection(), 
                GetContext().GetUserSettings(), GetContext().GetUsedUserSettingsListener());
            ExpressionContextPtr expressionContext = ECExpressionContextsProvider::GetCalculatedPropertyContext(params);

            ECValue value;
            Utf8String expressionResult;
            ECExpressionsCache noCache;
            if (ECExpressionsHelper(noCache).EvaluateECExpression(value, expression, *expressionContext) && value.IsPrimitive() && value.ConvertPrimitiveToString(expressionResult))
                {
                if (nullptr != GetContext().GetUsedClassesListener())
                    {
                    UsedClassesHelper::NotifyListenerWithUsedClasses(*GetContext().GetUsedClassesListener(),
                        GetContext().GetSchemaHelper(), GetContext().GetECExpressionsCache(), expression);
                    }
                }

            ApplyLocalization(expressionResult, GetContext());
            iter = GetCache().Insert(key, expressionResult).first;
            }

        ctx.SetResultText(iter->second.c_str(), (int)iter->second.size(), BeSQLite::DbFunction::Context::CopyData::No);
        }
   };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct ECPropertyDisplayLabelScalarCacheKey
    {
    ECClassId m_classId;
    Utf8String m_propertyName;
    Utf8String m_defaultLabel;
    uint64_t m_groupedInstancesCount;
    bool operator<(ECPropertyDisplayLabelScalarCacheKey const& other) const
        {
        return m_classId < other.m_classId
            || m_classId == other.m_classId
                && m_groupedInstancesCount < other.m_groupedInstancesCount
            || m_classId == other.m_classId
                && m_groupedInstancesCount == other.m_groupedInstancesCount
                && m_propertyName.CompareTo(other.m_propertyName) < 0
            || m_classId == other.m_classId
                && m_groupedInstancesCount == other.m_groupedInstancesCount
                && m_propertyName.Equals(other.m_propertyName)
                && m_defaultLabel.CompareTo(other.m_defaultLabel) < 0;
        }
    };

/*=================================================================================**//**
* Parameters:
* - ECClassId
* - Property Name
* - ECInstanceId
* - Property Value
* - Number of grouped instances
* Note: the function expects the context to have "current query" which is used to retrieve
* value ranges if this is a range grouping
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct GetECPropertyDisplayLabelScalar : CachingScalarFunction<bmap<ECPropertyDisplayLabelScalarCacheKey, Utf8String>>
{
private:
    static Utf8String GetValueAsString(double value)
        {
        Utf8String str;
        str.Sprintf("%.2f", value);
        if (str.Equals("-0.00"))
            str = "0.00";
        return str;
        }
public:
    GetECPropertyDisplayLabelScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_GetECPropertyDisplayLabel, 5, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        if (5 != nArgs)
            {
            BeAssert(false);
            ctx.SetResultError("Invalid number of arguments", BE_SQLITE_ERROR);
            return;
            }

        ECClassId classId = args[0].GetValueId<ECClassId>();
        Utf8CP propertyName = args[1].GetValueText();
        Utf8CP defaultLabel = args[3].GetValueText();
        uint64_t groupedInstancesCount = args[4].GetValueInt64();
        ECPropertyDisplayLabelScalarCacheKey key = {classId, propertyName, defaultLabel, groupedInstancesCount};

        auto iter = GetCache().find(key);
        if (GetCache().end() == iter)
            {
            ECClassCP ecClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(classId);
            if (nullptr == ecClass)
                {
                BeAssert(false);
                ctx.SetResultError("Invalid ECClass ID", BE_SQLITE_ERROR);
                return;
                }

            ECPropertyCP ecProperty = ecClass->GetPropertyP(propertyName);
            if (nullptr == ecProperty)
                {
                BeAssert(false);
                ctx.SetResultError("ECClass does not have the specified ECProperty", BE_SQLITE_ERROR);
                return;
                }

            Utf8String label;

            // first, look for label override
            GroupedInstanceKeysList groupedInstanceKeys;
            groupedInstanceKeys.resize(groupedInstancesCount); // note: this creates a list of invalid ECInstanceKeys. but it's okay as we only need it for count
            JsonNavNodePtr thisNode = GetContext().GetNodesFactory().CreateECPropertyGroupingNode(GetContext().GetConnection().GetId(), *ecClass, *ecProperty, "", nullptr, rapidjson::Document(), false, groupedInstanceKeys);
            // create temporary node key
            thisNode->SetNodeKey(*NavNodesHelper::CreateNodeKey(GetContext().GetConnection(), *thisNode, bvector<Utf8String>()));
            RulesPreprocessor::CustomizationRuleParameters params(GetContext().GetConnections(), GetContext().GetConnection(), *thisNode, GetContext().GetParentNode(),
                GetContext().GetRuleset(), GetContext().GetUserSettings(), GetContext().GetUsedUserSettingsListener(), GetContext().GetECExpressionsCache());
            LabelOverrideCP labelOverride = RulesPreprocessor::GetLabelOverride(params);
            if (nullptr != labelOverride && !labelOverride->GetLabel().empty())
                {
                ECExpressionContextsProvider::CustomizationRulesContextParameters params(*thisNode, GetContext().GetParentNode(),
                    GetContext().GetConnection(), GetContext().GetUserSettings(), GetContext().GetUsedUserSettingsListener());
                ExpressionContextPtr expressionContext = ECExpressionContextsProvider::GetCustomizationRulesContext(params);
                ECValue value;
                if (ECExpressionsHelper(GetContext().GetECExpressionsCache()).EvaluateECExpression(value, labelOverride->GetLabel(), *expressionContext) && value.IsPrimitive() && value.ConvertPrimitiveToString(label))
                    {
                    if (nullptr != GetContext().GetUsedClassesListener())
                        {
                        UsedClassesHelper::NotifyListenerWithUsedClasses(*GetContext().GetUsedClassesListener(),
                            GetContext().GetSchemaHelper(), GetContext().GetECExpressionsCache(), labelOverride->GetLabel());
                        }
                    }
                }
            // then, in case this is a range-based grouping node, use range labels
            if (label.empty() && nullptr != GetContext().GetQueryExtendedData() && NavigationQueryExtendedData(*GetContext().GetQueryExtendedData()).HasRangesData())
                {
                NavigationQueryExtendedData extendedData(*GetContext().GetQueryExtendedData());
                int rangeIndex = extendedData.GetRangeIndex(args[3]);
                if (-1 == rangeIndex)
                    {
                    // no matching range - use "Other"
                    label = RULESENGINE_LOCALIZEDSTRING_Other;
                    }
                else
                    {
                    label = extendedData.GetRangeLabel(rangeIndex);
                    }
                }
            // lastly, use the property value as label
            if (label.empty())
                {
                if (SUCCESS != ValueHelpers::GetEnumPropertyDisplayValue(label, *ecProperty, args[3]))
                    {
                    if (ecProperty->GetIsPrimitive())
                        {
                        switch (ecProperty->GetAsPrimitiveProperty()->GetType())
                            {
                            case PRIMITIVETYPE_Double:
                                label = GetValueAsString(args[3].GetValueDouble());
                                break;
                            case PRIMITIVETYPE_Point2d:
                            case PRIMITIVETYPE_Point3d:
                                {
                                rapidjson::Document json;
                                json.Parse(args[3].GetValueText());
                                label.append("X: ").append(GetValueAsString(json["x"].GetDouble()));
                                label.append(" Y: ").append(GetValueAsString(json["y"].GetDouble()));
                                if (json.HasMember("z"))
                                    label.append(" Z: ").append(GetValueAsString(json["z"].GetDouble()));
                                break;
                                }
                            default:
                                label = args[3].GetValueText();
                            }
                        }
                    else
                        {
                        label = args[3].GetValueText();
                        }
                    }
                }

            if (label.empty())
                label = RULESENGINE_LOCALIZEDSTRING_NotSpecified;

            ApplyLocalization(label, GetContext());
            iter = GetCache().Insert(key, label).first;
            }

        ctx.SetResultText(iter->second.c_str(), (int)iter->second.size(), BeSQLite::DbFunction::Context::CopyData::No);
        }
};

/*=================================================================================**//**
* Parameters:
* - Point coordinates
* @bsiclass                                     Saulius.Skliutas                06/2017
+===============+===============+===============+===============+===============+======*/
struct GetPointAsJsonStringScalar : ECPresentation::ScalarFunction
    {
    GetPointAsJsonStringScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_GetPointAsJsonString, -1, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        if (2 != nArgs && 3 != nArgs)
            {
            BeAssert(false);
            ctx.SetResultError("Invalid number of arguments", BE_SQLITE_ERROR);
            return;
            }

        Utf8String str;
        str.append("{\"x\":");
        str.append(std::to_string(args[0].GetValueDouble()).c_str());
        str.append(",\"y\":");
        str.append(std::to_string(args[1].GetValueDouble()).c_str());
        if (3 == nArgs)
            {
            str.append(",\"z\":");
            str.append(std::to_string(args[2].GetValueDouble()).c_str());
            }
        str.append("}");

        ctx.SetResultText(str.c_str(), (int)str.size(), DbFunction::Context::CopyData::Yes);
        }
    };

/*=================================================================================**//**
* Parameters:
* - ECSchemaName
* - ECClassName
* - ECPropertyName
* - Property value
* @bsiclass                                     Aidas.Vaiksnoras                11/2017
+===============+===============+===============+===============+===============+======*/
struct GetPropertyDisplayValueScalar : ECPresentation::ScalarFunction
    {
    GetPropertyDisplayValueScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_GetPropertyDisplayValue, 4, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        if (4 != nArgs)
            {
            BeAssert(false);
            ctx.SetResultError("Invalid number of arguments", BE_SQLITE_ERROR);
            return;
            }
        ECClassCP ecClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(args[0].GetValueText(), args[1].GetValueText());
        ECPropertyP ecProperty = ecClass->GetPropertyP(args[2].GetValueText());
        ECValue value;

        switch (ecProperty->GetAsPrimitiveProperty()->GetType())
            {
            case PRIMITIVETYPE_Point2d:
                {
                value.SetPoint2d(ValueHelpers::GetPoint2dFromJsonString(args[3].GetValueText()));
                break;
                }
            case PRIMITIVETYPE_Point3d:
                {
                value.SetPoint3d(ValueHelpers::GetPoint3dFromJsonString(args[3].GetValueText()));
                break;
                }
            case PRIMITIVETYPE_Double:
                {
                value.SetDouble(args[3].GetValueDouble());
                break;
                }
            default:
                value = ValueHelpers::GetECValueFromString(ecProperty->GetAsPrimitiveProperty()->GetType(), args[3].GetValueText());
            }
        
        Utf8String formattedValue(value.ToString());
        if (nullptr != GetContext().GetPropertyFormatter())
            GetContext().GetPropertyFormatter()->GetFormattedPropertyValue(formattedValue, *ecProperty, value);  
        ctx.SetResultText(formattedValue.c_str(), (int)formattedValue.size(), DbFunction::Context::CopyData::Yes);
        }
    };

/*=================================================================================**//**
* Parameters:
* - Point coordinates as Json string
* - Point x coordinate
* - Point y coordinate
* - Point z coordinate if Point is 3d
* @bsiclass                                     Aidas.Vaiksnoras                11/2017
+===============+===============+===============+===============+===============+======*/
struct ArePointsEqualByValueScalar : ECPresentation::ScalarFunction
    {
    ArePointsEqualByValueScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_ArePointsEqualByValue, -1, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        if (3 != nArgs && 4 != nArgs)
            {
            BeAssert(false);
            ctx.SetResultError("Invalid number of arguments", BE_SQLITE_ERROR);
            return;
            }
        if (3 == nArgs)
            {
            DPoint2d point = ValueHelpers::GetPoint2dFromJsonString(args[0].GetValueText());
            ctx.SetResultInt((int) 0 == BeNumerical::Compare(point.x, args[1].GetValueDouble())
                                && 0 == BeNumerical::Compare(point.y, args[2].GetValueDouble()));
            }
        else
            {
            DPoint3d point = ValueHelpers::GetPoint3dFromJsonString(args[0].GetValueText());
            ctx.SetResultInt((int) 0 == BeNumerical::Compare(point.x, args[1].GetValueDouble())
                                && 0 == BeNumerical::Compare(point.y, args[2].GetValueDouble())
                                && 0 == BeNumerical::Compare(point.z, args[3].GetValueDouble()));
            }
        }
    };

/*=================================================================================**//**
* Parameters:
* - Point coordinates as Json string
* - Point x coordinate
* - Point y coordinate
* - Point z coordinate if Point is 3d
* @bsiclass                                     Aidas.Vaiksnoras                11/2017
+===============+===============+===============+===============+===============+======*/
struct AreDoublesEqualByValueScalar : ECPresentation::ScalarFunction
    {
    AreDoublesEqualByValueScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_AreDoublesEqualByValue, 2, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        if (2 != nArgs)
            {
            BeAssert(false);
            ctx.SetResultError("Invalid number of arguments", BE_SQLITE_ERROR);
            return;
            }
        ctx.SetResultInt((int)0 == BeNumerical::Compare(args[0].GetValueDouble(), args[1].GetValueDouble()));
        }
    };

/*=================================================================================**//**
* Parameters:
* - Any value
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct GetSortingValueScalar : CachingScalarFunction<bmap<Utf8String, Utf8String>>
{
private:
    static const unsigned PADDING = 10;

private:
    static bool IsInteger(int c) {return '0' <= c && c <= '9';}
    static Utf8String GetPaddedNumber(Utf8CP chars, int length)
        {
        Utf8String padded;
        padded.reserve(PADDING);
        for (int i = length; i < PADDING; i++)
            padded.append("0");
        padded.append(chars, length);
        return padded;
        }

public:
    GetSortingValueScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_GetSortingValue, 1, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        BeAssert(1 == nArgs);
        Utf8CP input = args[0].GetValueText();

        auto iter = GetCache().find(input);
        if (GetCache().end() == iter)
            {
            Utf8String inputStr = input;
            Utf8CP numberBegin = nullptr;
            Utf8String output;
            output.reserve(strlen(input));
            while (nullptr != input && 0 != *input)
                {
                if (IsInteger(*input))
                    {
                    if (nullptr == numberBegin)
                        {
                        numberBegin = input;
                        output.reserve(output.size() + PADDING + strlen(input));
                        }
                    }
                else
                    {
                    if (nullptr != numberBegin)
                        {
                        output.append(GetPaddedNumber(numberBegin, (int)(input - numberBegin)));
                        numberBegin = nullptr;
                        }
                    Utf8Char c = *input;
                    output.append(1, (Utf8Char)std::tolower(c));
                    }
                input++;
                }
            if (nullptr != numberBegin)
                output.append(GetPaddedNumber(numberBegin, (int)(input - numberBegin)));

            iter = GetCache().Insert(inputStr, output).first;
            }

        ctx.SetResultText(iter->second.c_str(), (int)iter->second.size(), BeSQLite::DbFunction::Context::CopyData::No);
        }
};

/*=================================================================================**//**
* Parameters:
* - Property value
* Note: the function expects the context to have "current query" which is used to retrieve
* value ranges.
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct GetRangeIndexScalar : CachingScalarFunction<bmap<double, int>>
    {
    GetRangeIndexScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_GetRangeIndex, 1, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        if (1 != nArgs)
            {
            BeAssert(false);
            ctx.SetResultError("Invalid number of arguments", BE_SQLITE_ERROR);
            return;
            }

        auto iter = GetCache().find(args[0].GetValueDouble());
        if (GetCache().end() == iter)
            {
            if (nullptr == GetContext().GetQueryExtendedData())
                {
                BeAssert(false);
                ctx.SetResultError("Query extended data not set", BE_SQLITE_ERROR);
                return;
                }

            NavigationQueryExtendedData extendedData(*GetContext().GetQueryExtendedData());
            if (!extendedData.HasRangesData())
                {
                BeAssert(false);
                ctx.SetResultError("Navigation query is missing range data", BE_SQLITE_ERROR);
                return;
                }

            iter = GetCache().Insert(args[0].GetValueDouble(), extendedData.GetRangeIndex(args[0])).first;
            }

        ctx.SetResultInt(iter->second);
        }
    };

/*=================================================================================**//**
* Parameters:
* - Property value
* Note: the function expects the context to have "current query" which is used to retrieve
* value ranges.
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct GetRangeImageIdScalar : CachingScalarFunction<bmap<double, Utf8String>>
    {
    GetRangeImageIdScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_GetRangeImageId, 1, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        if (1 != nArgs)
            {
            BeAssert(false);
            ctx.SetResultError("Invalid number of arguments", BE_SQLITE_ERROR);
            return;
            }

        auto iter = GetCache().find(args[0].GetValueDouble());
        if (GetCache().end() == iter)
            {
            if (nullptr == GetContext().GetQueryExtendedData())
                {
                BeAssert(false);
                ctx.SetResultError("Query extended data not set", BE_SQLITE_ERROR);
                return;
                }

            NavigationQueryExtendedData extendedData(*GetContext().GetQueryExtendedData());
            if (!extendedData.HasRangesData())
                {
                BeAssert(false);
                ctx.SetResultError("Navigation query is missing range data", BE_SQLITE_ERROR);
                return;
                }

            Utf8String imageId;
            int rangeIndex = extendedData.GetRangeIndex(args[0]);
            if (-1 != rangeIndex)
                imageId = extendedData.GetRangeImageId(rangeIndex);

            iter = GetCache().Insert(args[0].GetValueDouble(), imageId).first;
            }

        ctx.SetResultText(iter->second.c_str(), (int)iter->second.size(), DbFunction::Context::CopyData::No);
        }
    };

/*=================================================================================**//**
* Parameters:
* - ECClass ID
* - Checked ECClass name
* - Checked ECClass schema name
* @bsiclass                                     Grigas.Petraitis                12/2015
+===============+===============+===============+===============+===============+======*/
struct IsOfClassScalar : ECPresentation::ScalarFunction
    {
    IsOfClassScalar(CustomFunctionsManager const& manager)
        : ECPresentation::ScalarFunction(FUNCTION_NAME_IsOfClass, 3, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        BeAssert(3 == nArgs);
        ECClassId classId = args[0].GetValueId<ECClassId>();
        ECClassCP ecClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(classId);
        BeAssert(nullptr != ecClass);

        Utf8CP className = args[1].GetValueText();
        Utf8CP schemaName = args[2].GetValueText();
        bool result = ecClass->Is(schemaName, className);
        ctx.SetResultInt((int)result);
        }
    };

/*=================================================================================**//**
* Parameters:
* - ECClass name
* - ECClass schema name
* @bsiclass                                     Grigas.Petraitis                07/2016
+===============+===============+===============+===============+===============+======*/
struct GetECClassIdScalar : ECPresentation::ScalarFunction
    {
    GetECClassIdScalar(CustomFunctionsManager const& manager)
        : ECPresentation::ScalarFunction(FUNCTION_NAME_GetECClassId, 2, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        BeAssert(2 == nArgs);
        Utf8CP className = args[0].GetValueText();
        Utf8CP schemaName = args[1].GetValueText();
        ECClassId id = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClassId(schemaName, className);
        ctx.SetResultInt64(id.GetValueUnchecked());
        }
    };

/*=================================================================================**//**
* Parameters:
* - ECClassId
* - ECInstanceId
* @bsiclass                                     Grigas.Petraitis                05/2017
+===============+===============+===============+===============+===============+======*/
struct GetInstanceKeyScalar : ECPresentation::ScalarFunction
    {
    GetInstanceKeyScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_GetInstanceKey, 2, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ECClassId classId = args[0].GetValueId<ECClassId>();
        ECInstanceId instanceId = args[1].GetValueId<ECInstanceId>();

        Utf8String result;
        result.append("{\"ECClassId\":").append(std::to_string(classId.GetValueUnchecked()).c_str());
        result.append(",\"ECInstanceId\":").append(std::to_string(instanceId.GetValueUnchecked()).c_str());
        result.append("}");

        ctx.SetResultText(result.c_str(), (int)result.size(), Context::CopyData::Yes);
        }
    };

/*=================================================================================**//**
* Parameters:
* - ECClassId
* - ECInstanceId
* OR
* - Serialized ECInstanceKey JSON
* Returns: a serialized json array containing ECInstanceKey objects.
* @bsiclass                                     Grigas.Petraitis                01/2016
+===============+===============+===============+===============+===============+======*/
struct GetGroupedInstanceKeysAggregate : ECPresentation::AggregateFunction
{
private:
    rapidjson::Document& GetJson(Context& ctx)
        {
        rapidjson::Document** jsonPP = (rapidjson::Document**)ctx.GetAggregateContext(sizeof(rapidjson::Document*));
        BeAssert(nullptr != jsonPP);

        rapidjson::Document*& jsonP = *jsonPP;
        if (nullptr == jsonP)
            {
            jsonP = new rapidjson::Document();
            jsonP->SetArray();
            }
        return *jsonP;
        }
protected:
    void _StepAggregate(Context& ctx, int nArgs, DbValue* args) override
        {
        rapidjson::Document& json = GetJson(ctx);
        if (1 == nArgs)
            {
            rapidjson::Document key(&json.GetAllocator());
            if (!args[0].IsNull())
                key.Parse(args[0].GetValueText());
            if (!key.IsNull() && key.IsObject())
                json.PushBack(key, json.GetAllocator());
            }
        else if (2 == nArgs)
            {
            ECClassId classId = args[0].GetValueId<ECClassId>();
            ECInstanceId instanceId = args[1].GetValueId<ECInstanceId>();
            rapidjson::Value key(rapidjson::kObjectType);
            key.AddMember("ECClassId", rapidjson::Value(classId.GetValueUnchecked()), json.GetAllocator());
            key.AddMember("ECInstanceId", rapidjson::Value(instanceId.GetValueUnchecked()), json.GetAllocator());
            json.PushBack(key, json.GetAllocator());
            }
        else
            {
            BeAssert(false);
            }
        }
    void _FinishAggregate(Context& ctx) override
        {
        rapidjson::Document const& json = GetJson(ctx);
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> w(buf);
        json.Accept(w);
        ctx.SetResultText(buf.GetString(), (int)buf.GetLength(), DbFunction::Context::CopyData::Yes);
        delete &json;
        }
public:
    GetGroupedInstanceKeysAggregate(CustomFunctionsManager const& manager)
        : AggregateFunction(FUNCTION_NAME_GetGroupedInstanceKeys, -1, DbValueType::TextVal, manager)
        {}
};

/*=================================================================================**//**
* Parameters:
* - Value
* - (optional) Result for when the values are different. Defaults to "*** @RulesEngine::LABEL_General_Varies@ ***"
* Returns: the value if all aggregate values were equal or a localized "varies" string if not.
* @bsiclass                                     Grigas.Petraitis                10/2016
+===============+===============+===============+===============+===============+======*/
struct GetMergedValueAggregate : ECPresentation::AggregateFunction
{
    struct MergedValueInfo
        {
        Utf8String m_value;
        bool m_isNull;
        bool m_isSet;
        bool m_isMerged;
        MergedValueInfo() : m_isSet(false), m_isMerged(false), m_isNull(true) {}
    };

private:
    MergedValueInfo& GetValueInfo(Context& ctx)
        {
        MergedValueInfo** infoPP = (MergedValueInfo**)ctx.GetAggregateContext(sizeof(MergedValueInfo*));
        BeAssert(nullptr != infoPP);

        MergedValueInfo*& infoP = *infoPP;
        if (nullptr == infoP)
            infoP = new MergedValueInfo();
        return *infoP;
        }
protected:
    void _StepAggregate(Context& ctx, int nArgs, DbValue* args) override
        {
        if (1 != nArgs && 2 != nArgs)
            {
            BeAssert(false);
            return;
            }

        Utf8CP value = args[0].GetValueText();
        bool isValueNull = (nullptr == value);
        MergedValueInfo& info = GetValueInfo(ctx);
        if (!info.m_isSet)
            {
            info.m_value = value;
            info.m_isNull = isValueNull;
            info.m_isSet = true;
            }
        else if (!info.m_isMerged && (info.m_isNull != isValueNull || !isValueNull && !info.m_value.Equals(value)))
            {
            if (nArgs > 1)
                {
                info.m_value = args[1].GetValueText();
                }
            else
                {
                Utf8String localizationId = PRESENTATION_LOCALIZEDSTRING(RulesEngineL10N::GetNameSpace().m_namespace, RulesEngineL10N::LABEL_General_Varies().m_str);
                info.m_value = Utf8PrintfString(CONTENTRECORD_MERGED_VALUE_FORMAT, localizationId.c_str());
                }
            ApplyLocalization(info.m_value, GetContext());
            info.m_isMerged = true;
            info.m_isNull = false;
            }
        }
    void _FinishAggregate(Context& ctx) override
        {
        MergedValueInfo const& info = GetValueInfo(ctx);
        if (info.m_isNull)
            ctx.SetResultNull();
        else
            ctx.SetResultText(info.m_value.c_str(), (int)info.m_value.size(), DbFunction::Context::CopyData::Yes);
        delete &info;
        }
public:
    GetMergedValueAggregate(CustomFunctionsManager const& manager)
        : AggregateFunction(FUNCTION_NAME_GetMergedValue, -1, DbValueType::TextVal, manager)
        {}
};

/*=================================================================================**//**
* Parameters:
* - Setting ID
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct GetSettingValueScalar : ECPresentation::ScalarFunction
    {
    GetSettingValueScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_GetSettingValue, 1, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        BeAssert(1 == nArgs);
        Utf8CP settingId = args[0].GetValueText();
        Utf8String value = GetContext().GetUserSettings().GetSettingValue(settingId);
        ctx.SetResultText(value.c_str(), (int)value.size(), DbFunction::Context::CopyData::Yes);

        if (nullptr != GetContext().GetUsedUserSettingsListener())
            GetContext().GetUsedUserSettingsListener()->_OnUserSettingUsed(settingId);
        }
    };

/*=================================================================================**//**
* Parameters:
* - Setting ID
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct GetSettingIntValueScalar : ECPresentation::ScalarFunction
    {
    GetSettingIntValueScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_GetSettingIntValue, 1, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        BeAssert(1 == nArgs);
        Utf8CP settingId = args[0].GetValueText();
        ctx.SetResultInt64(GetContext().GetUserSettings().GetSettingIntValue(settingId));

        if (nullptr != GetContext().GetUsedUserSettingsListener())
            GetContext().GetUsedUserSettingsListener()->_OnUserSettingUsed(settingId);
        }
    };

/*=================================================================================**//**
* Parameters:
* - Setting ID
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct GetSettingBoolValueScalar : ECPresentation::ScalarFunction
    {
    GetSettingBoolValueScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_GetSettingBoolValue, 1, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        BeAssert(1 == nArgs);
        Utf8CP settingId = args[0].GetValueText();
        ctx.SetResultInt(GetContext().GetUserSettings().GetSettingBoolValue(settingId) ? 1 : 0);

        if (nullptr != GetContext().GetUsedUserSettingsListener())
            GetContext().GetUsedUserSettingsListener()->_OnUserSettingUsed(settingId);
        }
    };

/*=================================================================================**//**
* Parameters:
* - Setting ID
* - Integer value to find
* @bsiclass                                     Grigas.Petraitis                04/2017
+===============+===============+===============+===============+===============+======*/
struct InSettingIntValuesScalar : CachingScalarFunction<bmap<Utf8String, bvector<int64_t>>>
    {
    InSettingIntValuesScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_InSettingIntValues, 2, DbValueType::IntegerVal, manager)
        {}
    void _OnUserSettingChanged(Utf8CP settingId) override {GetCache().erase(settingId);}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        BeAssert(2 == nArgs);
        Utf8CP settingId = args[0].GetValueText();
        int64_t lookupValue = args[1].GetValueInt64();

        if (nullptr != GetContext().GetUsedUserSettingsListener())
            GetContext().GetUsedUserSettingsListener()->_OnUserSettingUsed(settingId);

        bmap<Utf8String, bvector<int64_t>>& cache = GetCache();
        auto iter = cache.find(settingId);
        if (cache.end() == iter)
            iter = cache.Insert(settingId, GetContext().GetUserSettings().GetSettingIntValues(settingId)).first;

        bvector<int64_t> const& values = iter->second;
        for (int64_t value : values)
            {
            if (value == lookupValue)
                {
                ctx.SetResultInt(1);
                return;
                }
            }
        ctx.SetResultInt(0);
        }
    };

/*=================================================================================**//**
* Parameters:
* - Setting ID
* @bsiclass                                     Grigas.Petraitis                05/2016
+===============+===============+===============+===============+===============+======*/
struct HasSettingScalar : ECPresentation::ScalarFunction
    {
    HasSettingScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_HasSetting, 1, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        BeAssert(1 == nArgs);
        Utf8CP settingId = args[0].GetValueText();
        ctx.SetResultInt(GetContext().GetUserSettings().HasSetting(settingId) ? 1 : 0);
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2016
+===============+===============+===============+===============+===============+======*/
struct IsRecursivelyRelatedFunctionCache
    {
    struct CacheKey
        {
        Utf8CP m_fullRelationshipName;
        Utf8CP m_relationshipDirection;
        ECInstanceId m_instanceId;

        CacheKey() : m_fullRelationshipName(nullptr), m_relationshipDirection(nullptr) {}
        CacheKey(BeSQLite::DbValue* args)
            {
            m_fullRelationshipName = args[0].GetValueText();
            m_relationshipDirection = args[1].GetValueText();
            m_instanceId = args[5].GetValueId<ECInstanceId>();
            }
        bool operator==(CacheKey const& other) const
            {
            return m_fullRelationshipName == other.m_fullRelationshipName
                && m_relationshipDirection == other.m_relationshipDirection
                && m_instanceId == other.m_instanceId;
            }
        bool operator<(CacheKey const& other) const
            {
            if (m_fullRelationshipName < other.m_fullRelationshipName)
                return true;
            if (m_fullRelationshipName > other.m_fullRelationshipName)
                return false;
            if (m_relationshipDirection < other.m_relationshipDirection)
                return true;
            if (m_relationshipDirection > other.m_relationshipDirection)
                return false;
            return (m_instanceId < other.m_instanceId);
            }
        };
    typedef bmap<CacheKey, std::unordered_set<uint64_t>> RelatedKeysCache;

    RelatedKeysCache m_relatedKeys;
    ECSqlStatementCache m_statements;

    RelatedKeysCache::const_iterator m_lastMatch;

    IsRecursivelyRelatedFunctionCache()
        : m_statements(10), m_lastMatch(m_relatedKeys.end())
        {}
    };

/*=================================================================================**//**
* Parameters:
* - Relationship name, e.g. RulesEngineTest:WidgetHasGadgets
* - Relationship direction: Backward or Forward.
* - Source ECClass Id
* - Source ECInstance Id
* - Target ECClassId
* - Target ECInstance Id
* @bsiclass                                     Grigas.Petraitis                12/2016
+===============+===============+===============+===============+===============+======*/
struct IsRecursivelyRelatedScalar : CachingScalarFunction<IsRecursivelyRelatedFunctionCache>
    {
    IsRecursivelyRelatedScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_IsRecursivelyRelated, 6, DbValueType::IntegerVal, manager)
        {}
    void RecursivelySelectAndAddRelatedKeys(std::unordered_set<uint64_t>& relatedInstanceKeys, ECSqlStatement& stmt, ECInstanceId const& instanceId) const
        {
        stmt.Reset();
        if (ECSqlStatus::Success != stmt.BindId(1, instanceId))
            {
            BeAssert(false);
            return;
            }

        bvector<ECInstanceId> tempKeys;
        while (BeSQLite::DbResult::BE_SQLITE_ROW == stmt.Step())
            {
            ECInstanceId id = stmt.GetValueId<ECInstanceId>(0);
            relatedInstanceKeys.insert(id.GetValue());
            tempKeys.push_back(id);
            }

        for (ECInstanceId const& key : tempKeys)
            RecursivelySelectAndAddRelatedKeys(relatedInstanceKeys, stmt, key);
        }
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        if (6 != nArgs)
            {
            BeAssert(false);
            ctx.SetResultError("Invalid number of arguments", BE_SQLITE_ERROR);
            return;
            }

        IsRecursivelyRelatedFunctionCache::CacheKey key(args);
        IsRecursivelyRelatedFunctionCache& cache = GetCache();
        IsRecursivelyRelatedFunctionCache::RelatedKeysCache::const_iterator iter = cache.m_relatedKeys.end();
        if (cache.m_lastMatch != iter && cache.m_lastMatch->first == key)
            iter = cache.m_lastMatch;
        else
            iter = cache.m_relatedKeys.find(key);

        if (cache.m_relatedKeys.end() == iter)
            {
            Utf8String relationshipSchemaName, relationshipClassName;
            ECClassCP relationshipClass = nullptr;
            if (ECObjectsStatus::Success != ECClass::ParseClassName(relationshipSchemaName, relationshipClassName, key.m_fullRelationshipName)
                || nullptr == (relationshipClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(relationshipSchemaName, relationshipClassName))
                || !relationshipClass->IsRelationshipClass())
                {
                BeAssert(false);
                ctx.SetResultError("Invalid relationship name");
                return;
                }

            static Utf8CP s_sourceECInstanceIdField = "SourceECInstanceId";
            static Utf8CP s_targetECInstanceIdField = "TargetECInstanceId";
            static Utf8CP s_queryFormat = ""
                "SELECT %s "
                "  FROM %s"
                " WHERE %s = ?";

            Utf8String query;
            if (0 == strcmp("Forward", key.m_relationshipDirection))
                query.Sprintf(s_queryFormat, s_sourceECInstanceIdField, relationshipClass->GetECSqlName().c_str(), s_targetECInstanceIdField);
            else if (0 == strcmp("Backward", key.m_relationshipDirection))
                query.Sprintf(s_queryFormat, s_targetECInstanceIdField, relationshipClass->GetECSqlName().c_str(), s_sourceECInstanceIdField);
            else
                {
                BeAssert(false);
                ctx.SetResultError("Invalid relationship direction");
                return;
                }

            IConnectionCR connection = GetContext().GetSchemaHelper().GetConnection();
            CachedECSqlStatementPtr stmt = cache.m_statements.GetPreparedStatement(connection.GetECDb().Schemas(), connection.GetDb(), query.c_str());
            if (stmt.IsNull())
                {
                BeAssert(false);
                ctx.SetResultError("Failed to get related instances.");
                return;
                }

            std::unordered_set<uint64_t> relatedInstanceKeys;
            RecursivelySelectAndAddRelatedKeys(relatedInstanceKeys, *stmt, key.m_instanceId);
            iter = cache.m_lastMatch = cache.m_relatedKeys.Insert(key, relatedInstanceKeys).first;
            }

        ctx.SetResultInt((int)(iter->second.end() != iter->second.find(args[3].GetValueId<ECInstanceId>().GetValue())));
        }
    };

/*=================================================================================**//**
* Parameters:
* - Enumeration ID
* - Enumeration value
* @bsiclass                                     Grigas.Petraitis                05/2017
+===============+===============+===============+===============+===============+======*/
struct GetECEnumerationValueScalar : ECPresentation::ScalarFunction
    {
    GetECEnumerationValueScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_GetECEnumerationValue, 3, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        BeAssert(3 == nArgs);
        Utf8CP enumSchema = args[0].GetValueText();
        Utf8CP enumClass = args[1].GetValueText();
        ECEnumerationCP enumeration = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetEnumeration(enumSchema, enumClass);
        if (nullptr == enumeration)
            {
            BeAssert(false);
            ctx.SetResultError("Invalid enumeration.");
            return;
            }
        ECEnumeratorCP enumerator = nullptr;
        switch (enumeration->GetType())
            {
            case PRIMITIVETYPE_Integer:
                {
                BeAssert(args[2].GetValueType() == DbValueType::IntegerVal);
                int valueId = args[2].GetValueInt();
                enumerator = enumeration->FindEnumerator(valueId);
                break;
                }
            case PRIMITIVETYPE_String:
                {
                BeAssert(args[2].GetValueType() == DbValueType::TextVal);
                Utf8CP valueId = args[2].GetValueText();
                enumerator = enumeration->FindEnumerator(valueId);
                break;
                }
            }
        if (nullptr == enumerator)
            {
            BeAssert(false);
            ctx.SetResultError("Invalid enumerator value.");
            return;
            }
        ctx.SetResultText(enumerator->GetDisplayLabel().c_str(), -1, Context::CopyData::No);
        }
    };

/*=================================================================================**//**
* Parameters:
* - ECClassId
* - ECInstanceId
* Returns: A serialized JSON array of ECInstanceKey objects.
* @bsiclass                                     Grigas.Petraitis                06/2017
+===============+===============+===============+===============+===============+======*/
struct ECInstanceKeysArrayScalar : ECPresentation::ScalarFunction
{
protected:
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        BeAssert(nArgs % 2 == 0);
        rapidjson::Document json;
        json.SetArray();
        for (int i = 0; i < nArgs; i += 2)
            {
            ECClassId classId = args[i].GetValueId<ECClassId>();
            ECInstanceId instanceId = args[i + 1].GetValueId<ECInstanceId>();
            rapidjson::Value keyJson;
            keyJson.SetObject();
            keyJson.AddMember("ECClassId", classId.GetValue(), json.GetAllocator());
            keyJson.AddMember("ECInstanceId", instanceId.GetValue(), json.GetAllocator());
            json.PushBack(keyJson, json.GetAllocator());
            }
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        json.Accept(writer);
        ctx.SetResultText(buf.GetString(), (int)buf.GetLength(), DbFunction::Context::CopyData::Yes);
        }
public:
    ECInstanceKeysArrayScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_ECInstanceKeysArray, -1, DbValueType::TextVal, manager)
        {}
};

/*=================================================================================**//**
* Parameters:
* - A serialized JSON array
* Returns: An aggregated serialized JSON array.
* @bsiclass                                     Grigas.Petraitis                06/2017
+===============+===============+===============+===============+===============+======*/
struct JsonArrayAggregate : ECPresentation::AggregateFunction
{
private:
    Utf8StringR GetAggregatedJsonString(Context& ctx)
        {
        Utf8String** strPP = (Utf8String**)ctx.GetAggregateContext(sizeof(Utf8String*));
        BeAssert(nullptr != strPP);

        Utf8String*& strP = *strPP;
        if (nullptr == strP)
            strP = new Utf8String();
        return *strP;
        }
protected:
    void _StepAggregate(Context& ctx, int nArgs, DbValue* args) override
        {
        BeAssert(1 == nArgs);
        if (args[0].IsNull() || 0 == *args[0].GetValueText())
            return;

        Utf8StringR aggregate = GetAggregatedJsonString(ctx);
        if (aggregate.empty())
            {
            aggregate = args[0].GetValueText();
            }
        else
            {
            aggregate.erase(aggregate.size() - 1, 1);
            aggregate.append(",");
            aggregate.append(args[0].GetValueText() + 1);
            }
        }
    void _FinishAggregate(Context& ctx) override
        {
        Utf8StringCR str = GetAggregatedJsonString(ctx);
        ctx.SetResultText(str.c_str(), (int)str.size(), DbFunction::Context::CopyData::Yes);
        delete &str;
        }
public:
    JsonArrayAggregate(CustomFunctionsManager const& manager)
        : AggregateFunction(FUNCTION_NAME_AggregateJsonArray, 1, DbValueType::TextVal, manager)
        {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsContext::CustomFunctionsContext(ECSchemaHelper const& schemaHelper, IConnectionManagerCR connections, IConnectionCR connection,
    PresentationRuleSetCR ruleset, IUserSettings const& userSettings, IUsedUserSettingsListener* usedSettingsListener,
    ECExpressionsCache& ecexpressionsCache, JsonNavNodesFactory const& nodesFactory, IUsedClassesListener* usedClassesListener,
    JsonNavNodeCP parentNode, rapidjson::Value const* queryExtendedData, IECPropertyFormatter const* formatter)
    : m_schemaHelper(schemaHelper), m_connections(connections), m_connection(connection), m_ruleset(ruleset), m_userSettings(userSettings),
    m_usedSettingsListener(usedSettingsListener), m_ecexpressionsCache(ecexpressionsCache),
    m_parentNode(parentNode), m_nodesFactory(nodesFactory), m_usedClassesListener(usedClassesListener),
    m_extendedData(queryExtendedData), m_localizationProvider(nullptr), m_propertyFormatter(formatter)
    {
    CustomFunctionsManager::GetManager().PushContext(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsContext::~CustomFunctionsContext()
    {
    for (ICustomFunctionsContextListener* listener : m_listeners)
        listener->_OnContextDisposed(*this);

    BeAssert(m_caches.empty());

    CustomFunctionsContext* ctx = CustomFunctionsManager::GetManager().PopContext();
    UNUSED_VARIABLE(ctx);
    BeAssert(this == ctx);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void* CustomFunctionsContext::GetCache(Utf8CP id) const
    {
    for (size_t i = 0; i < m_caches.size(); ++i)
        {
        FunctionCache const& cache = m_caches[i];
        if (cache.m_name == id)
            return cache.m_ptr;
        }
    return nullptr;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsContext::InsertCache(Utf8CP id, void* cache)
    {
    BeAssert(nullptr == GetCache(id));
    m_caches.push_back(FunctionCache(id, cache));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsContext::RemoveCache(Utf8CP id)
    {
    for (size_t i = 0; i < m_caches.size(); ++i)
        {
        FunctionCache const& cache = m_caches[i];
        if (cache.m_name == id)
            {
            m_caches.erase(m_caches.begin() + i);
            return;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsContext::_OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const
    {
    if (nullptr != rulesetId && !m_ruleset.GetRuleSetId().Equals(rulesetId))
        return;

    for (ICustomFunctionsContextListener* listener : m_listeners)
        listener->_OnUserSettingChanged(settingId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsManager& CustomFunctionsManager::GetManager()
    {
    static CustomFunctionsManager* s_manager = nullptr;
    if (nullptr == s_manager)
        s_manager = new CustomFunctionsManager();
    return *s_manager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsManager::_OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const
    {
    for (CustomFunctionsContext* ctx : m_contexts)
        ctx->_OnSettingChanged(rulesetId, settingId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsContext& CustomFunctionsManager::GetCurrentContext() const
    {
    BeAssert(!m_contexts.empty());
    return *m_contexts.back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsManager::PushContext(CustomFunctionsContext& context)
    {
    m_contexts.push_back(&context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsContext* CustomFunctionsManager::PopContext()
    {
    CustomFunctionsContext* context = m_contexts.back();
    if (nullptr != context)
        m_contexts.pop_back();
    return context;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsInjector::CustomFunctionsInjector(IConnectionManagerCR connections) : m_connections(connections)
    {
    m_connections.AddListener(*this);
    CreateFunctions();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsInjector::CustomFunctionsInjector(IConnectionManagerCR connections, IConnectionCR connection)
    : m_connections(connections)
    {
    m_connections.AddListener(*this);
    CreateFunctions();
    OnConnection(connection);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsInjector::~CustomFunctionsInjector()
    {
    for (Utf8StringCR connectionId : m_handledConnectionIds)
        {
        IConnectionPtr connection = m_connections.GetConnection(connectionId.c_str());
        if (connection.IsValid())
            Unregister(*connection);
        }
    DestroyFunctions();
    m_connections.DropListener(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsInjector::CreateFunctions()
    {
    m_scalarFunctions.push_back(new GetECInstanceDisplayLabelScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new GetECClassDisplayLabelScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new GetECPropertyDisplayLabelScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new GetSortingValueScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new GetRangeIndexScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new GetRangeImageIdScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new IsOfClassScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new GetECClassIdScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new GetSettingValueScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new GetSettingIntValueScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new GetSettingBoolValueScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new InSettingIntValuesScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new HasSettingScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new EvaluateECExpressionScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new IsRecursivelyRelatedScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new GetECEnumerationValueScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new GetInstanceKeyScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new ECInstanceKeysArrayScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new GetPointAsJsonStringScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new ArePointsEqualByValueScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new AreDoublesEqualByValueScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new GetPropertyDisplayValueScalar(CustomFunctionsManager::GetManager()));
    m_scalarFunctions.push_back(new GetRelatedDisplayLabelScalar(CustomFunctionsManager::GetManager(), FUNCTION_NAME_GetNavigationPropertyLabel, true));
    m_scalarFunctions.push_back(new GetRelatedDisplayLabelScalar(CustomFunctionsManager::GetManager(), FUNCTION_NAME_GetRelatedDisplayLabel));

    m_aggregateFunctions.push_back(new GetGroupedInstanceKeysAggregate(CustomFunctionsManager::GetManager()));
    m_aggregateFunctions.push_back(new GetMergedValueAggregate(CustomFunctionsManager::GetManager()));
    m_aggregateFunctions.push_back(new JsonArrayAggregate(CustomFunctionsManager::GetManager()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsInjector::DestroyFunctions()
    {
    for (ScalarFunction* func : m_scalarFunctions)
        delete func;

    for (AggregateFunction* func : m_aggregateFunctions)
        delete func;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsInjector::Register(IConnectionCR connection)
    {
    for (ScalarFunction* func : m_scalarFunctions)
        {
        DbResult result = (DbResult)connection.GetDb().AddFunction(*func);
        if (DbResult::BE_SQLITE_OK != result)
            {
            BeAssert(false);
            NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE)->errorv("Failed to add custom function '%s'. Result = %d", func->GetName(), (int)result);
            }
        }

    for (AggregateFunction* func : m_aggregateFunctions)
        {
        DbResult result = (DbResult)connection.GetDb().AddFunction(*func);
        if (DbResult::BE_SQLITE_OK != result)
            {
            BeAssert(false);
            NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE)->errorv("Failed to add custom function '%s'. Result = %d", func->GetName(), (int)result);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsInjector::Unregister(IConnectionCR connection)
    {
    if (!connection.IsOpen())
        return;

    for (ScalarFunction* func : m_scalarFunctions)
        {
        DbResult result = (DbResult)connection.GetDb().RemoveFunction(*func);
        if (DbResult::BE_SQLITE_OK != result)
            {
            BeAssert(false);
            NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE)->errorv("Failed to remove custom function '%s'. Result = %d", func->GetName(), (int)result);
            }
        }
    for (AggregateFunction* func : m_aggregateFunctions)
        {
        DbResult result = (DbResult)connection.GetDb().RemoveFunction(*func);
        if (DbResult::BE_SQLITE_OK != result)
            {
            BeAssert(false);
            NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE)->errorv("Failed to remove custom function '%s'. Result = %d", func->GetName(), (int)result);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsInjector::OnConnection(IConnectionCR connection)
    {
    if (Handles(connection))
        return;

    m_handledConnectionIds.insert(connection.GetId());
    Register(connection);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsInjector::_OnConnectionEvent(ConnectionEvent const& evt)
    {
    if (ConnectionEventType::Closed == evt.GetEventType())
        {
        Unregister(evt.GetConnection());
        m_handledConnectionIds.erase(evt.GetConnection().GetId());
        }
    }
