/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/LabelDefinition.h>
#include <sstream>
#include <unordered_set>
#include "../../Hierarchies/NavNodesHelper.h"
#include "../../Hierarchies/NavNodeProviders.h"
#include "../../Hierarchies/NavigationQuery.h"
#include "../ECExpressions/ECExpressionContextsProvider.h"
#include "../ECSchemaHelper.h"
#include "CustomFunctions.h"
#include "QueryContracts.h"
#include "PresentationQuery.h"
#include "QueryBuilderHelpers.h"
#include "../NodeLabelCalculator.h"

#define HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN_CSTR(msg) \
    { \
    ctx.SetResultError(msg, BE_SQLITE_ERROR); \
    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, msg); \
    }

#define HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN(msg) \
    { \
    auto _msg = msg; \
    HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN_CSTR(_msg.c_str()); \
    }

#define ARGUMENTS_COUNT_PRECONDITION_CUSTOM(condition, description) \
    if (!(condition)) \
        HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN(Utf8PrintfString("Expecting " description " arguments, got %d", nArgs));

#define ARGUMENTS_COUNT_PRECONDITION(expectedCount) ARGUMENTS_COUNT_PRECONDITION_CUSTOM(nArgs == expectedCount, #expectedCount)


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessLabelOverride(LabelDefinitionPtr& labelDefinition, CustomFunctionsContext const& context, NavNodeCR node)
    {
    // look for label override
    IRulesPreprocessor::CustomizationRuleByNodeParameters preprocessorParams(node, context.GetParentNode());
    LabelOverrideCP labelOverride = context.GetRules().GetLabelOverride(preprocessorParams);
    if (nullptr != labelOverride && !labelOverride->GetLabel().empty())
        {
        // evaluate the ECExpression to get the label
        ECExpressionContextsProvider::CustomizationRulesContextParameters expressionContextParams(node, context.GetParentNode(),
            context.GetConnection(), context.GetRulesetVariables(), context.GetUsedRulesetVariablesListener());
        ExpressionContextPtr expressionContext = ECExpressionContextsProvider::GetCustomizationRulesContext(expressionContextParams);
        ECValue value;
        Utf8String displayValue;
        if (ECExpressionsHelper(context.GetECExpressionsCache()).EvaluateECExpression(value, labelOverride->GetLabel(), *expressionContext) && value.IsPrimitive() && value.ConvertPrimitiveToString(displayValue))
            {
            if (value.IsString())
                labelDefinition = LabelDefinition::FromString(displayValue.c_str());
            else
                labelDefinition->SetECValue(value, displayValue.c_str());
            if (nullptr != context.GetUsedClassesListener())
                {
                UsedClassesHelper::NotifyListenerWithUsedClasses(*context.GetUsedClassesListener(),
                    context.GetSchemaHelper(), labelOverride->GetLabel());
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static LabelDefinitionPtr GetLabel(CustomFunctionsContext const& ctx, ECClassInstanceKeyCR key, bvector<ECInstanceKey> const& prevLabelRequestsStack)
    {
    NodeLabelCalculator labelCalculator(ctx.GetSchemaHelper(), ctx.GetRules());
    return labelCalculator.GetNodeLabel(key, prevLabelRequestsStack);
    }

/*=================================================================================**//**
* @bsiclass
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
* @bsiclass
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
* @bsiclass
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

/*=================================================================================**//**
* Parameters:
* - ECClassId
* - ECInstanceId
* - Display label property value of this instance
* - Related instances' info JSON (serialized to string). Format: [{"Alias":"related_1","ECClassId":1,"ECInstanceId":1},{"Alias":"related_2","ECClassId":2,"ECInstanceId":2},...]
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetECInstanceDisplayLabelScalar : CachingScalarFunction<bmap<ECInstanceKey, std::shared_ptr<Utf8String>>>
{
private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ProcessLabelOverrides(LabelDefinitionPtr& labelDefinition, ECClassId const& classId, ECInstanceId const& instanceId, Utf8CP relatedInstanceInfo) const
        {
        NavNodePtr thisNode = GetContext().GetNodesFactory().CreateECInstanceNode(GetContext().GetConnection(), "", nullptr, classId, instanceId, *LabelDefinition::Create());
        NavNodesHelper::AddRelatedInstanceInfo(*thisNode, relatedInstanceInfo);

        ProcessLabelOverride(labelDefinition, GetContext(), *thisNode);
        }

public:
    GetECInstanceDisplayLabelScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_GetECInstanceDisplayLabel, 4, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(4);

        ECInstanceId instanceId (args[1].GetValueUInt64());
        ECClassId classId = args[0].GetValueId<ECClassId>();
        ECInstanceKey key(classId, instanceId);

        auto iter = GetCache().find(key);
        if (GetCache().end() == iter)
            {
            LabelDefinitionPtr labelDefinition = LabelDefinition::Create();

            if (key.IsValid())
                {
                ProcessLabelOverrides(labelDefinition, classId, instanceId, args[3].GetValueText());
                if (!labelDefinition->IsDefinitionValid())
                    {
                    ECClassCP ecClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(classId);
                    if (nullptr == ecClass)
                        HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN(Utf8PrintfString("Invalid ECClassId: %" PRIu64, classId.GetValue()));

                    // if the override didn't apply, look for instance label property
                    if (nullptr != ecClass->GetInstanceLabelProperty())
                        {
                        labelDefinition->SetECPropertyValue(*ecClass->GetInstanceLabelProperty(), args[2], args[2].GetValueText());
                        }
                    }
                }

            if (!labelDefinition->IsDefinitionValid())
                {
                labelDefinition->SetStringValue(CommonStrings::RULESENGINE_NOTSPECIFIED);
                }

            iter = GetCache().Insert(key, std::make_shared<Utf8String>(labelDefinition->ToJsonString())).first;
            }
        ctx.SetResultText(iter->second->c_str(), (int)iter->second->size(), BeSQLite::DbFunction::Context::CopyData::No);
        }
};

/*=================================================================================**//**
* Parameters:
* - ECClassId
* - ECInstanceId
* - (optional) Label requests stack (serialized JSON array of ECInstanceKey objects)
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetRelatedDisplayLabelScalar : CachingScalarFunction<bmap<ECInstanceKey, std::shared_ptr<Utf8String>>>
{
public:
    GetRelatedDisplayLabelScalar(CustomFunctionsManager const& manager, Utf8CP name)
        : CachingScalarFunction(name, -1, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION_CUSTOM(nArgs == 2 || nArgs == 3, "2 or 3");

        ECInstanceId instanceId(args[1].GetValueUInt64());
        ECClassId classId = args[0].GetValueId<ECClassId>();
        ECInstanceKey key(classId, instanceId);

        auto iter = GetCache().find(key);
        if (GetCache().end() == iter)
            {
            LabelDefinitionPtr label;
            if (key.IsValid())
                {
                ECClassCP ecClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(classId);
                if (nullptr == ecClass)
                    HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN(Utf8PrintfString("Invalid ECClassId: %" PRIu64, classId.GetValue()));

                ECInstanceKey requestKey(classId, instanceId);
                bvector<ECInstanceKey> labelRequestsStack = (nArgs == 3) ? ValueHelpers::GetECInstanceKeysFromJsonString(args[2].GetValueText()) : bvector<ECInstanceKey>();
                if (ContainerHelpers::Contains(labelRequestsStack, [&requestKey](auto const& key){return key == requestKey;}))
                    {
                    DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_WARNING, LOG_ERROR, Utf8PrintfString("Detected recursion in labels calculation. "
                        "ECInstance keys stack: `%s`. Trying to get label for: `%s`", args[2].GetValueText(), ValueHelpers::GetECInstanceKeyAsJsonString(requestKey).c_str()));
                    }
                else
                    {
                    label = GetLabel(GetContext(), ECClassInstanceKey(ecClass, instanceId), labelRequestsStack);
                    }
                }

            if (!label.IsValid())
                label = LabelDefinition::Create(CommonStrings::RULESENGINE_NOTSPECIFIED);

            iter = GetCache().Insert(key, std::make_shared<Utf8String>(label->ToJsonString())).first;
            }

        ctx.SetResultText(iter->second->c_str(), (int)iter->second->size(), BeSQLite::DbFunction::Context::CopyData::No);
        }
};

/*=================================================================================**//**
* Parameters:
* - ECClassId
* - Number of grouped instances
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetECClassDisplayLabelScalar : CachingScalarFunction<bmap<ECClassId, std::shared_ptr<Utf8String>>>
    {
    GetECClassDisplayLabelScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_GetECClassDisplayLabel, 2, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(2);

        ECClassId classId = args[0].GetValueId<ECClassId>();

        auto iter = GetCache().find(classId);
        if (GetCache().end() == iter)
            {
            ECClassCP ecClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(classId);
            if (nullptr == ecClass)
                HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN(Utf8PrintfString("Invalid ECClassId: %" PRIu64, classId.GetValue()));

            LabelDefinitionPtr labelDefinition = LabelDefinition::Create();

            // first, look for label override
            NavNodePtr thisNode;
            if (ecClass->IsRelationshipClass())
                thisNode = GetContext().GetNodesFactory().CreateECRelationshipGroupingNode(GetContext().GetConnection(), "", nullptr, *ecClass->GetRelationshipClassCP(), *LabelDefinition::Create(), args[1].GetValueInt64());
            else
                thisNode = GetContext().GetNodesFactory().CreateECClassGroupingNode(GetContext().GetConnection(), "", nullptr, *ecClass, false, *LabelDefinition::Create(), args[1].GetValueInt64());

            ProcessLabelOverride(labelDefinition, GetContext(), *thisNode);

            if (!labelDefinition->IsDefinitionValid())
                {
                // otherwise, use the display label
                labelDefinition->SetStringValue(ecClass->GetDisplayLabel().c_str());
                }

            if (!labelDefinition->IsDefinitionValid())
                {
                labelDefinition->SetStringValue(CommonStrings::RULESENGINE_NOTSPECIFIED);
                }

            iter = GetCache().Insert(classId, std::make_shared<Utf8String>(labelDefinition->ToJsonString())).first;
            }
        ctx.SetResultText(iter->second->c_str(), (int)iter->second->size(), BeSQLite::DbFunction::Context::CopyData::No);
        }
    };

/*=================================================================================**//**
* Parameters:
* - ECClassId
* - ECInstanceId
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetNavigationPropertyValueScalar : CachingScalarFunction<bmap<ECInstanceKey, std::shared_ptr<Utf8String>>>
{
public:
    GetNavigationPropertyValueScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_GetNavigationPropertyValue, 2, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(2);

        ECClassId classId = args[0].GetValueId<ECClassId>();
        ECInstanceId instanceId(args[1].GetValueUInt64());
        ECInstanceKey key(classId, instanceId);

        auto iter = GetCache().find(key);
        if (GetCache().end() == iter)
            {
            Utf8String result;
            if (key.IsValid())
                {
                ECClassCP ecClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(classId);
                if (nullptr == ecClass)
                    HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN(Utf8PrintfString("Invalid ECClassId: %" PRIu64, classId.GetValue()));

                result.append("{");
                result.append("\"label\":").append(GetLabel(GetContext(), ECClassInstanceKey(ecClass, instanceId), {})->ToJsonString()).append(",");
                result.append("\"key\":")
                    .append("{")
                    .append("\"c\":").append(std::to_string(classId.GetValueUnchecked())).append(",")
                    .append("\"i\":").append(std::to_string(instanceId.GetValueUnchecked()))
                    .append("}");
                result.append("}");
                }
            iter = GetCache().Insert(key, std::make_shared<Utf8String>(result)).first;
            }
        if (iter->second->empty())
            ctx.SetResultNull();
        else
            ctx.SetResultText(iter->second->c_str(), (int)iter->second->size(), BeSQLite::DbFunction::Context::CopyData::No);
        }
    };

/*=================================================================================**//**
* @bsiclass
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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EvaluateECExpressionScalar : CachingScalarFunction<bmap<ECExpressionScalarCacheKey, std::shared_ptr<Utf8String>>>
    {
    EvaluateECExpressionScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_EvaluateECExpression, 3, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(3);

        ECClassId classId = args[0].GetValueId<ECClassId>();
        ECInstanceId instanceId = args[1].GetValueId<ECInstanceId>();
        Utf8CP expression = args[2].GetValueText();
        ECExpressionScalarCacheKey key = {classId, instanceId, expression};

        auto iter = GetCache().find(key);
        if (GetCache().end() == iter)
            {
            NavNodePtr thisNode = GetContext().GetNodesFactory().CreateECInstanceNode(GetContext().GetConnection(), "", nullptr, classId, instanceId, *LabelDefinition::Create());
            ECExpressionContextsProvider::CalculatedPropertyContextParameters params(*thisNode, GetContext().GetConnection(),
                GetContext().GetRulesetVariables(), GetContext().GetUsedRulesetVariablesListener());
            ExpressionContextPtr expressionContext = ECExpressionContextsProvider::GetCalculatedPropertyContext(params);

            ECValue value;
            Utf8String expressionResult;
            ECExpressionsCache noCache;
            if (ECExpressionsHelper(noCache).EvaluateECExpression(value, expression, *expressionContext) && value.IsPrimitive() && value.ConvertPrimitiveToString(expressionResult))
                {
                if (nullptr != GetContext().GetUsedClassesListener())
                    {
                    UsedClassesHelper::NotifyListenerWithUsedClasses(*GetContext().GetUsedClassesListener(),
                        GetContext().GetSchemaHelper(), expression);
                    }
                }

            iter = GetCache().Insert(key, std::make_shared<Utf8String>(expressionResult)).first;
            }

        ctx.SetResultText(iter->second->c_str(), (int)iter->second->size(), BeSQLite::DbFunction::Context::CopyData::No);
        }
   };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetFormattedPropertyValue(PrimitiveECPropertyCR property, DbValue const& sqlValue, IECPropertyFormatter const* formatter, ECPresentation::UnitSystem unitSystem)
    {
    if (sqlValue.IsNull())
        return "";
    ECValue value = ValueHelpers::GetECValueFromSqlValue(property.GetType(), sqlValue);
    Utf8String formattedValue;
    if (nullptr != formatter && SUCCESS == formatter->GetFormattedPropertyValue(formattedValue, property, value, unitSystem))
        return formattedValue;
    return value.ToString();
    }

/*=================================================================================**//**
* @bsiclass
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
* - Property Value or LabelDefinition string
* - Number of grouped instances
* Note: the function expects the context to have "current query" which is used to retrieve
* value ranges if this is a range grouping
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetECPropertyDisplayLabelScalar : CachingScalarFunction<bmap<ECPropertyDisplayLabelScalarCacheKey, std::shared_ptr<Utf8String>>>
{
    GetECPropertyDisplayLabelScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_GetECPropertyDisplayLabel, 5, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(5);

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
                HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN(Utf8PrintfString("Invalid ECClassId: %" PRIu64, classId.GetValue()));

            ECPropertyCP ecProperty = ecClass->GetPropertyP(propertyName);
            if (nullptr == ecProperty)
                HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN(Utf8PrintfString("Property '%s' not found in ECClass '%s'", propertyName, ecClass->GetFullName()));

            LabelDefinitionPtr labelDefinition = LabelDefinition::Create();

            // first, look for label override
            NavNodePtr thisNode = GetContext().GetNodesFactory().CreateECPropertyGroupingNode(GetContext().GetConnection(), "", nullptr,
                *ecClass, *ecProperty, *LabelDefinition::Create(), nullptr, rapidjson::Value(rapidjson::kArrayType), false, groupedInstancesCount);

            ProcessLabelOverride(labelDefinition, GetContext(), *thisNode);

            // then, in case this is a range-based grouping node, use range labels
            if (!labelDefinition->IsDefinitionValid() && nullptr != GetContext().GetQueryExtendedData() && NavigationQueryExtendedData(*GetContext().GetQueryExtendedData()).HasRangesData())
                {
                NavigationQueryExtendedData extendedData(*GetContext().GetQueryExtendedData());
                int rangeIndex = extendedData.GetRangeIndex(args[3]);
                if (-1 == rangeIndex)
                    {
                    // no matching range - use "Other"
                    labelDefinition->SetStringValue(CommonStrings::RULESENGINE_OTHER);
                    }
                else
                    {
                    labelDefinition->SetStringValue(extendedData.GetRangeLabel(rangeIndex).c_str(), extendedData.GetRangeLabel(rangeIndex).c_str());
                    }
                }
            // lastly, use the property value as label
            if (!labelDefinition->IsDefinitionValid())
                {
                Utf8String formattedValue;
                if (SUCCESS != ValueHelpers::GetEnumPropertyDisplayValue(formattedValue, *ecProperty, args[3]))
                    {
                    if (ecProperty->GetIsPrimitive())
                        {
                        labelDefinition->SetECPropertyValue(*ecProperty, args[3], GetFormattedPropertyValue(*ecProperty->GetAsPrimitiveProperty(), args[3],
                            GetContext().GetPropertyFormatter(), GetContext().GetUnitSystem()).c_str());
                        }
                    else
                        {
                        labelDefinition = LabelDefinition::FromString(defaultLabel);
                        }
                    }
                else
                    {
                    labelDefinition->SetStringValue(formattedValue.c_str());
                    }
                }

            if (!labelDefinition->IsDefinitionValid())
                {
                labelDefinition->SetStringValue(CommonStrings::RULESENGINE_NOTSPECIFIED);
                }

            iter = GetCache().Insert(key, std::make_shared<Utf8String>(labelDefinition->ToJsonString())).first;
            }

        ctx.SetResultText(iter->second->c_str(), (int)iter->second->size(), BeSQLite::DbFunction::Context::CopyData::No);
        }
};

/*=================================================================================**//**
* Parameters:
* - Point coordinates
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetPointAsJsonStringScalar : ECPresentation::ScalarFunction
    {
    GetPointAsJsonStringScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_GetPointAsJsonString, -1, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION_CUSTOM(nArgs == 2 || nArgs == 3, "2 or 3");

        Utf8String str;
        str.append("{\"x\":");
        str.append(args[0].IsNull() ? "NULL" : args[0].GetValueText());
        str.append(",\"y\":");
        str.append(args[1].IsNull() ? "NULL" : args[1].GetValueText());
        if (3 == nArgs)
            {
            str.append(",\"z\":");
            str.append(args[2].IsNull() ? "NULL" : args[2].GetValueText());
            }
        str.append("}");

        ctx.SetResultText(str.c_str(), (int)str.size(), DbFunction::Context::CopyData::Yes);
        }
    };

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
/*=================================================================================**//**
* Parameters:
* - ECSchemaName
* - ECClassName
* - ECPropertyName
* - Property value
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetPropertyDisplayValueScalar : ECPresentation::ScalarFunction
    {
    GetPropertyDisplayValueScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_GetPropertyDisplayValue, 4, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(4);

        ECClassCP ecClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(args[0].GetValueText(), args[1].GetValueText());
        ECPropertyP ecProperty = ecClass->GetPropertyP(args[2].GetValueText());
        Utf8String formattedValue = GetFormattedPropertyValue(*ecProperty->GetAsPrimitiveProperty(), args[3], GetContext().GetPropertyFormatter(), GetContext().GetUnitSystem());
        ctx.SetResultText(formattedValue.c_str(), (int)formattedValue.size(), DbFunction::Context::CopyData::Yes);
        }
    };
#endif

/*=================================================================================**//**
* Parameters:
* - Point coordinates as Json string
* - Point x coordinate
* - Point y coordinate
* - Point z coordinate if Point is 3d
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ArePointsEqualByValueScalar : ECPresentation::ScalarFunction
    {
    ArePointsEqualByValueScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_ArePointsEqualByValue, -1, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION_CUSTOM(nArgs == 3 || nArgs == 4, "3 or 4");

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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct AreDoublesEqualByValueScalar : ECPresentation::ScalarFunction
    {
    AreDoublesEqualByValueScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_AreDoublesEqualByValue, 2, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(2);
        ctx.SetResultInt((int)0 == BeNumerical::Compare(args[0].GetValueDouble(), args[1].GetValueDouble()));
        }
    };

/*=================================================================================**//**
* Parameters:
* - LabelDefinition json string
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetSortingValueScalar : CachingScalarFunction<bmap<Utf8String, std::shared_ptr<Utf8String>>>
{
public:
    GetSortingValueScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_GetSortingValue, 1, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(1);

        Utf8CP input = args[0].GetValueText();

        auto iter = GetCache().find(input);
        if (GetCache().end() == iter)
            {
            Utf8String inputStr = LabelDefinition::FromString(input)->GetDisplayValue();
            iter = GetCache().Insert(inputStr, std::make_shared<Utf8String>(ValueHelpers::PadNumbersInString(inputStr))).first;
            }
        if (CommonStrings::RULESENGINE_NOTSPECIFIED == iter->first || CommonStrings::RULESENGINE_OTHER == iter->first)
            ctx.SetResultNull();
        else
            ctx.SetResultText(iter->second->c_str(), (int)iter->second->size(), BeSQLite::DbFunction::Context::CopyData::No);
        }
};

/*=================================================================================**//**
* Parameters:
* - Property value
* Note: the function expects the context to have "current query" which is used to retrieve
* value ranges.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetRangeIndexScalar : CachingScalarFunction<bmap<double, int>>
    {
    GetRangeIndexScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_GetRangeIndex, 1, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(1);

        auto iter = GetCache().find(args[0].GetValueDouble());
        if (GetCache().end() == iter)
            {
            if (nullptr == GetContext().GetQueryExtendedData())
                HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN_CSTR("Query extended data not set");

            NavigationQueryExtendedData extendedData(*GetContext().GetQueryExtendedData());
            if (!extendedData.HasRangesData())
                HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN_CSTR("Navigation query is missing range data");

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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetRangeImageIdScalar : CachingScalarFunction<bmap<double, std::shared_ptr<Utf8String>>>
    {
    GetRangeImageIdScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_GetRangeImageId, 1, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(1);

        auto iter = GetCache().find(args[0].GetValueDouble());
        if (GetCache().end() == iter)
            {
            if (nullptr == GetContext().GetQueryExtendedData())
                HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN_CSTR("Query extended data not set");

            NavigationQueryExtendedData extendedData(*GetContext().GetQueryExtendedData());
            if (!extendedData.HasRangesData())
                HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN_CSTR("Navigation query is missing range data");

            Utf8String imageId;
            int rangeIndex = extendedData.GetRangeIndex(args[0]);
            if (-1 != rangeIndex)
                imageId = extendedData.GetRangeImageId(rangeIndex);

            iter = GetCache().Insert(args[0].GetValueDouble(), std::make_shared<Utf8String>(imageId)).first;
            }

        ctx.SetResultText(iter->second->c_str(), (int)iter->second->size(), DbFunction::Context::CopyData::No);
        }
    };

/*=================================================================================**//**
* Parameters:
* - ECClass ID
* - Checked (base) ECClass name
* - Checked (base) ECClass schema name
* OR
* - ECClass (base) ID
* - Checked (base) ECClass ID
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IsOfClassScalar : ECPresentation::ScalarFunction
    {
    IsOfClassScalar(CustomFunctionsManager const& manager)
        : ECPresentation::ScalarFunction(FUNCTION_NAME_IsOfClass, -1, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION_CUSTOM(nArgs == 2 || nArgs == 3, "2 or 3");

        ECClassId classId = args[0].GetValueId<ECClassId>();
        ECClassCP ecClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(classId);
        if (!ecClass)
            HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN(Utf8PrintfString("Invalid ECClassId: %" PRIu64, classId.GetValue()));

        if (3 == nArgs)
            {
            Utf8CP className = args[1].GetValueText();
            Utf8CP schemaName = args[2].GetValueText();
            ctx.SetResultInt((int)ecClass->Is(schemaName, className));
            }
        else if (2 == nArgs)
            {
            ECClassId baseCassId = args[1].GetValueId<ECClassId>();
            ECClassCP baseClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(baseCassId);
            ctx.SetResultInt((int)ecClass->Is(baseClass));
            }
        }
    };

/*=================================================================================**//**
* Parameters:
* - ECClass name
* - ECClass schema name
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetECClassIdScalar : ECPresentation::ScalarFunction
    {
    GetECClassIdScalar(CustomFunctionsManager const& manager)
        : ECPresentation::ScalarFunction(FUNCTION_NAME_GetECClassId, 2, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(2);

        Utf8CP className = args[0].GetValueText();
        Utf8CP schemaName = args[1].GetValueText();
        ECClassId id = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClassId(schemaName, className);
        ctx.SetResultInt64(id.GetValueUnchecked());
        }
    };

/*=================================================================================**//**
* Parameters:
* - ECClass ID
* - Flag indicating whether full class name should be returned
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetECClassNameScalar : ECPresentation::ScalarFunction
    {
    GetECClassNameScalar(CustomFunctionsManager const& manager)
        : ECPresentation::ScalarFunction(FUNCTION_NAME_GetECClassName, 2, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(2);

        ECClassId classId = args[0].GetValueId<ECClassId>();
        bool full = (0 != args[1].GetValueInt());
        ECClassCP ecClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(classId);
        if (nullptr == ecClass)
            HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN(Utf8PrintfString("Invalid ECClassId: %" PRIu64, classId.GetValue()));

        Utf8String className = full ? ecClass->GetFullName() : ecClass->GetName();
        Utf8String valueJson = LabelDefinition::Create(className.c_str())->ToJsonString();

        ctx.SetResultText(valueJson.c_str(), valueJson.size(), DbFunction::Context::CopyData::Yes);
        }
    };

/*=================================================================================**//**
* Parameters:
* - ECClass ID
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetECClassLabelScalar : ECPresentation::ScalarFunction
    {
    GetECClassLabelScalar(CustomFunctionsManager const& manager)
        : ECPresentation::ScalarFunction(FUNCTION_NAME_GetECClassLabel, 1, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(1);

        ECClassId classId = args[0].GetValueId<ECClassId>();
        ECClassCP ecClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(classId);
        if (nullptr == ecClass)
            HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN(Utf8PrintfString("Invalid ECClassId: %" PRIu64, classId.GetValue()));

        Utf8String valueJson = LabelDefinition::Create(ecClass->GetDisplayLabel().c_str())->ToJsonString();
        ctx.SetResultText(valueJson.c_str(), valueJson.size(), DbFunction::Context::CopyData::Yes);
        }
    };

/*=================================================================================**//**
* Parameters:
* - property value
* - primitive type of the property
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetPropertyValueJsonScalar : ECPresentation::ScalarFunction
    {
    GetPropertyValueJsonScalar(CustomFunctionsManager const& manager)
        : ECPresentation::ScalarFunction(FUNCTION_NAME_GetPropertyValueJson, 2, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(2);

        rapidjson::Document value;
        if (!args[0].IsNull())
            {
            PrimitiveType type = (PrimitiveType)args[1].GetValueInt();
            switch (type)
                {
                case PRIMITIVETYPE_Boolean:
                    value.SetBool((bool)args[0].GetValueInt());
                    break;
                case PRIMITIVETYPE_Double:
                case PRIMITIVETYPE_DateTime:
                    value.SetDouble(args[0].GetValueDouble());
                    break;
                case PRIMITIVETYPE_Integer:
                    value.SetInt(args[0].GetValueInt());
                    break;
                case PRIMITIVETYPE_Long:
                    value.SetInt64(args[0].GetValueInt64());
                    break;
                case PRIMITIVETYPE_String:
                case PRIMITIVETYPE_Point2d:
                case PRIMITIVETYPE_Point3d:
                    value.SetString(args[0].GetValueText(), value.GetAllocator());
                    break;
                }
            }
        auto serialized = BeRapidJsonUtilities::ToString(value);
        ctx.SetResultText(serialized.c_str(), serialized.size(), DbFunction::Context::CopyData::Yes);
        }
    };

/*=================================================================================**//**
* Returns: a serialized json array containing given values.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct JsonConcatAggregateBase : ECPresentation::AggregateFunction
{
private:
    Utf8StringR GetSerializedAggregateJsonArray(Context& ctx)
        {
        Utf8String** jsonPP = (Utf8String**)ctx.GetAggregateContext(sizeof(Utf8String*));
        Utf8String*& jsonP = *jsonPP;
        if (nullptr == jsonP)
            jsonP = new Utf8String("[]");
        return *jsonP;
        }
protected:
    virtual Utf8String _GetValueToAppend(DbValue const& arg) const = 0;
    void _StepAggregate(Context& ctx, int nArgs, DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(1);
        Utf8StringR agg = GetSerializedAggregateJsonArray(ctx);
        agg.erase(agg.begin() + (agg.length() - 1)); // erase ending ']'
        if (agg.length() > 1)
            agg.append(",");
        agg.append(_GetValueToAppend(args[0])).append("]");
        }
    void _FinishAggregate(Context& ctx) override
        {
        Utf8StringCR agg = GetSerializedAggregateJsonArray(ctx);
        ctx.SetResultText(agg.c_str(), (int)agg.size(), DbFunction::Context::CopyData::Yes);
        delete &agg;
        }
public:
    JsonConcatAggregateBase(CustomFunctionsManager const& manager, Utf8CP name)
        : AggregateFunction(name, 1, DbValueType::TextVal, manager)
        {}
};

/*=================================================================================**//**
* Parameters:
* - primitive value
* Returns: a serialized json array containing given values.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct JsonConcatAggregate : JsonConcatAggregateBase
{
protected:
    Utf8String _GetValueToAppend(DbValue const& arg) const override
        {
        return arg.GetValueText();
        }
public:
    JsonConcatAggregate(CustomFunctionsManager const& manager)
        : JsonConcatAggregateBase(manager, FUNCTION_NAME_JsonConcat)
        {}
};

/*=================================================================================**//**
* Parameters:
* - stringified JSON value
* Returns: a serialized json array containing given values.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct StringifiedJsonConcatAggregate : JsonConcatAggregateBase
{
protected:
    Utf8String _GetValueToAppend(DbValue const& arg) const override
        {
        Utf8String unwrappedValue(arg.GetValueText());
        unwrappedValue.Trim("\"");
        unwrappedValue.ReplaceAll("\\\"", "\"");
        return unwrappedValue;
        }
public:
    StringifiedJsonConcatAggregate(CustomFunctionsManager const& manager)
        : JsonConcatAggregateBase(manager, FUNCTION_NAME_StringifiedJsonConcat)
        {}
};

/*=================================================================================**//**
* Parameters:
* - ECClassId
* - ECInstanceId
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetInstanceKeyScalar : ECPresentation::ScalarFunction
    {
    GetInstanceKeyScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_GetInstanceKey, 2, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(2);

        ECClassId classId = args[0].GetValueId<ECClassId>();
        ECInstanceId instanceId = args[1].GetValueId<ECInstanceId>();

        if (!instanceId.IsValid())
            {
            ctx.SetResultNull();
            return;
            }

        Utf8String result;
        result.append("{\"c\":").append(std::to_string(classId.GetValueUnchecked()).c_str());
        result.append(",\"i\":").append(std::to_string(instanceId.GetValueUnchecked()).c_str());
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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetInstanceKeysAggregate : ECPresentation::AggregateFunction
{
protected:
    bset<ECInstanceKey>& GetKeys(Context& ctx)
        {
        bset<ECInstanceKey>** setPP = (bset<ECInstanceKey>**)ctx.GetAggregateContext(sizeof(bset<ECInstanceKey>*));
        bset<ECInstanceKey>*& setP = *setPP;
        if (!setP)
            setP = new bset<ECInstanceKey>();
        return *setP;
        }
    virtual void _StepAggregate(Context& ctx, int nArgs, DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION_CUSTOM(nArgs == 1 || nArgs == 2, "1 or 2");
        bset<ECInstanceKey>& keys = GetKeys(ctx);
        if (1 == nArgs)
            {
            ECInstanceKey key = ValueHelpers::GetECInstanceKeyFromJsonString(args[0].GetValueText());
            if (key.IsValid())
                keys.insert(key);
            }
        else if (2 == nArgs)
            {
            ECClassId classId = args[0].GetValueId<ECClassId>();
            ECInstanceId instanceId = args[1].GetValueId<ECInstanceId>();
            if (instanceId.IsValid())
                keys.insert(ECInstanceKey(classId, instanceId));
            }
        }
    void _FinishAggregate(Context& ctx) override
        {
        bset<ECInstanceKey> const& keys = GetKeys(ctx);
        Utf8String serialized = ValueHelpers::GetECInstanceKeysAsJsonString(keys);
        ctx.SetResultText(serialized.c_str(), (int)serialized.size(), DbFunction::Context::CopyData::Yes);
        delete &keys;
        }
public:
    GetInstanceKeysAggregate(CustomFunctionsManager const& manager, Utf8CP name = FUNCTION_NAME_GetInstanceKeys)
        : AggregateFunction(name, -1, DbValueType::TextVal, manager)
        {}
};

/*=================================================================================**//**
* Parameters:
* - Max number of instance keys.
* - ECClassId
* - ECInstanceId
* OR
* - Max number of instance keys.
* - Serialized ECInstanceKey JSON
* Returns: a serialized json array containing ECInstanceKey objects.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetLimitedInstanceKeysAggregate : GetInstanceKeysAggregate
{
protected:
    void _StepAggregate(Context& ctx, int nArgs, DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION_CUSTOM(nArgs == 2 || nArgs == 3, "2 or 3");
        uint64_t maxKeys = args[0].GetValueUInt64();
        bset<ECInstanceKey>& keys = GetKeys(ctx);
        if ((uint64_t)keys.size() >= maxKeys)
            return;
        if (2 == nArgs)
            {
            ECInstanceKey key = ValueHelpers::GetECInstanceKeyFromJsonString(args[1].GetValueText());
            if (key.IsValid())
                keys.insert(key);
            }
        else if (3 == nArgs)
            {
            ECClassId classId = args[1].GetValueId<ECClassId>();
            ECInstanceId instanceId = args[2].GetValueId<ECInstanceId>();
            if (instanceId.IsValid())
                keys.insert(ECInstanceKey(classId, instanceId));
            }
        }
public:
    GetLimitedInstanceKeysAggregate(CustomFunctionsManager const& manager)
        : GetInstanceKeysAggregate(manager, FUNCTION_NAME_GetLimitedInstanceKeys)
        {}
};

/*=================================================================================**//**
* Parameters:
* - Variable ID
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetStringVariableValueScalar : ECPresentation::ScalarFunction
    {
    GetStringVariableValueScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_GetVariableStringValue, 1, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(1);

        Utf8CP variableId = args[0].GetValueText();
        Utf8String value = GetContext().GetRulesetVariables().GetStringValue(variableId);
        ctx.SetResultText(value.c_str(), (int)value.size(), DbFunction::Context::CopyData::Yes);

        if (nullptr != GetContext().GetUsedRulesetVariablesListener())
            GetContext().GetUsedRulesetVariablesListener()->OnVariableUsed(variableId);
        }
    };

/*=================================================================================**//**
* Parameters:
* - Variable ID
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetIntVariableValueScalar : ECPresentation::ScalarFunction
    {
    GetIntVariableValueScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_GetVariableIntValue, 1, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(1);

        Utf8CP variableId = args[0].GetValueText();
        ctx.SetResultInt64(GetContext().GetRulesetVariables().GetIntValue(variableId));

        if (nullptr != GetContext().GetUsedRulesetVariablesListener())
            GetContext().GetUsedRulesetVariablesListener()->OnVariableUsed(variableId);
        }
    };

/*=================================================================================**//**
* Parameters:
* - Variable ID
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetBoolVariableValueScalar : ECPresentation::ScalarFunction
    {
    GetBoolVariableValueScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_GetVariableBoolValue, 1, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(1);

        Utf8CP variableId = args[0].GetValueText();
        ctx.SetResultInt(GetContext().GetRulesetVariables().GetBoolValue(variableId) ? 1 : 0);

        if (nullptr != GetContext().GetUsedRulesetVariablesListener())
            GetContext().GetUsedRulesetVariablesListener()->OnVariableUsed(variableId);
        }
    };

/*=================================================================================**//**
* Parameters:
* - Variable ID
* - Integer value to find
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct InIntVariableValuesScalar : CachingScalarFunction<bmap<Utf8String, bvector<int64_t>>>
    {
    InIntVariableValuesScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_InVariableIntValues, 2, DbValueType::IntegerVal, manager)
        {}
    void _OnUserSettingChanged(Utf8CP settingId) override {GetCache().erase(settingId);}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(2);

        Utf8CP variableId = args[0].GetValueText();
        int64_t lookupValue = args[1].GetValueInt64();

        if (nullptr != GetContext().GetUsedRulesetVariablesListener())
            GetContext().GetUsedRulesetVariablesListener()->OnVariableUsed(variableId);

        bmap<Utf8String, bvector<int64_t>>& cache = GetCache();
        auto iter = cache.find(variableId);
        if (cache.end() == iter)
            iter = cache.Insert(variableId, GetContext().GetRulesetVariables().GetIntValues(variableId)).first;

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
* - Variable ID
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HasVariableScalar : ECPresentation::ScalarFunction
    {
    HasVariableScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_HasVariable, 1, DbValueType::IntegerVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(1);

        Utf8CP variableId = args[0].GetValueText();
        ctx.SetResultInt(GetContext().GetRulesetVariables().HasValue(variableId) ? 1 : 0);
        }
    };

/*=================================================================================**//**
* Parameters:
* - Enumeration ID
* - Enumeration value
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetECEnumerationValueScalar : ECPresentation::ScalarFunction
    {
    GetECEnumerationValueScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_GetECEnumerationValue, 3, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(3);

        Utf8CP enumSchema = args[0].GetValueText();
        Utf8CP enumClass = args[1].GetValueText();
        ECEnumerationCP enumeration = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetEnumeration(enumSchema, enumClass);
        if (nullptr == enumeration)
            HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN(Utf8PrintfString("Enumeration '%s' not found in schema '%s'", enumClass, enumSchema));

        ECEnumeratorCP enumerator = nullptr;
        switch (enumeration->GetType())
            {
            case PRIMITIVETYPE_Integer:
                {
                if (args[2].GetValueType() != DbValueType::IntegerVal)
                    HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN(Utf8PrintfString("Enumeration '%s' values should be of integer type, but got: %d", enumeration->GetFullName().c_str(), args[2].GetValueType()));

                int valueId = args[2].GetValueInt();
                enumerator = enumeration->FindEnumerator(valueId);
                break;
                }
            case PRIMITIVETYPE_String:
                {
                if (args[2].GetValueType() != DbValueType::TextVal)
                    HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN(Utf8PrintfString("Enumeration '%s' values should be of string type, but got: %d", enumeration->GetFullName().c_str(), args[2].GetValueType()));

                Utf8CP valueId = args[2].GetValueText();
                enumerator = enumeration->FindEnumerator(valueId);
                break;
                }
            }
        if (nullptr == enumerator)
            HANDLE_CUSTOM_FUNCTION_FAILURE_RETURN(Utf8PrintfString("Invalid enumeration '%s' value: '%s'", enumeration->GetFullName().c_str(), args[2].GetValueText()));

        Utf8String json = LabelDefinition::Create(enumerator->GetDisplayLabel().c_str())->ToJsonString();
        ctx.SetResultText(json.c_str(), json.size(), Context::CopyData::Yes);
        }
    };

#ifdef wip_skipped_instance_keys_performance_issue
/*=================================================================================**//**
* Parameters:
* - ECClassId
* - ECInstanceId
* Returns: A serialized JSON array of ECInstanceKey objects.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECInstanceKeysArrayScalar : ECPresentation::ScalarFunction
{
protected:
    void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION_CUSTOM(nArgs % 2 == 0, "equal number of");

        Utf8String json;
        // reserve the right amount of space to avoid re-allocations
        json.reserve(2 + (nArgs / 2) * (12 + 2 * 20));
        json.append("[");
        for (int i = 0; i < nArgs; i += 2)
            {
            ECClassId classId = args[i].GetValueId<ECClassId>();
            ECInstanceId instanceId = args[i + 1].GetValueId<ECInstanceId>();
            if (i != 0)
                json.append(",");
            json.append("{\"c\":").append(std::to_string(classId.GetValue()).c_str());
            json.append(",\"i\":").append(std::to_string(instanceId.GetValue()).c_str());
            json.append("}");
            }
        json.append("]");
        ctx.SetResultText(json.c_str(), json.size(), Context::CopyData::Yes);
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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct JsonArrayAggregate : ECPresentation::AggregateFunction
{
private:
    Utf8StringR GetAggregatedJsonString(Context& ctx)
        {
        Utf8String** strPP = (Utf8String**)ctx.GetAggregateContext(sizeof(Utf8String*));
        Utf8String*& strP = *strPP;
        if (nullptr == strP)
            strP = new Utf8String();
        return *strP;
        }
protected:
    void _StepAggregate(Context& ctx, int nArgs, DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(1);

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
#endif

/*=================================================================================**//**
* Parameters:
* - NavNodeLabelDefinition json string
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ToBase36Scalar : ECPresentation::ScalarFunction
    {
    ToBase36Scalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_ToBase36, 1, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(1);

        LabelDefinitionPtr labelDefinition = LabelDefinition::FromString(args[0].GetValueText());
        uint64_t rawValue = labelDefinition->GetRawValue()->AsSimpleValue()->GetValue().GetUint64();
        Utf8String base36 = CommonTools::ToBase36String(rawValue);
        Utf8String newDefinition = labelDefinition->SetStringValue(base36.c_str()).ToJsonString();
        ctx.SetResultText(newDefinition.c_str(), newDefinition.size(), DbFunction::Context::CopyData::Yes);
        }
    };

/*=================================================================================**//**
* WIP temporary until SQL supports bitwise operators
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ParseBriefcaseIdScalar : ECPresentation::ScalarFunction
    {
    ParseBriefcaseIdScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_ParseBriefcaseId, 1, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(1);

        uint64_t id = args[0].GetValueUInt64();
        uint64_t briefcaseId = id >> 40;
        rapidjson::Value value(briefcaseId);
        Utf8String valueJson = LabelDefinition::Create(value, "uint64", std::to_string(briefcaseId).c_str())->ToJsonString();
        ctx.SetResultText(valueJson.c_str(), valueJson.size(), DbFunction::Context::CopyData::Yes);
        }
    };

/*=================================================================================**//**
* WIP temporary until SQL supports bitwise operators
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ParseLocalIdScalar : ECPresentation::ScalarFunction
    {
    ParseLocalIdScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_ParseLocalId, 1, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(1);

        uint64_t id = args[0].GetValueUInt64();
        uint64_t localId = id & (((uint64_t)1 << 40) - 1);
        rapidjson::Value value(localId);
        Utf8String valueJson = LabelDefinition::Create(value, "uint64", std::to_string(localId).c_str())->ToJsonString();
        ctx.SetResultText(valueJson.c_str(), valueJson.size(), DbFunction::Context::CopyData::Yes);
        }
    };

/*=================================================================================**//**
* Parameters:
* - number
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct JoinOptionallyRequiredScalar : ECPresentation::ScalarFunction
    {
    struct AggregatedStringLabelDefinition
        {
        private:
            Utf8String m_displayValue;
            Utf8String m_rawValue;

        public:
            void AddValue(LabelDefinitionCR value, Utf8CP separator)
                {
                if (!m_rawValue.empty())
                    m_rawValue.append(separator);
                if (!m_displayValue.empty())
                    m_displayValue.append(separator);

                m_rawValue.append(value.GetRawValue()->AsSimpleValue()->GetValue().GetString());
                m_displayValue.append(value.GetDisplayValue());
                }
            bool Empty() { return m_displayValue.empty() && m_rawValue.empty(); }
            LabelDefinitionPtr GetValueAndReset()
                {
                LabelDefinitionPtr value = LabelDefinition::Create(m_rawValue.c_str(), m_displayValue.c_str());
                m_displayValue.clear();
                m_rawValue.clear();
                return value;
                }
        };

    JoinOptionallyRequiredScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_JoinOptionallyRequired, -1, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION_CUSTOM(1 == (nArgs % 2), "odd number of");

        Utf8String displayLabel;
        Utf8CP separator = args[0].GetValueText();

        bvector<LabelDefinitionCPtr> parts;
        AggregatedStringLabelDefinition aggregatedValue;
        for (int i = 1; i < nArgs; i += 2)
            {
            Utf8CP valueJsonString = args[i].GetValueText();
            bool isRequired = (0 != args[i + 1].GetValueInt());
            LabelDefinitionPtr value = LabelDefinition::FromString(valueJsonString);
            if (!value->IsDefinitionValid())
                {
                if (isRequired)
                    {
                    displayLabel.clear();
                    break;
                    }

                continue;
                }

            if (!displayLabel.empty())
                displayLabel.append(separator);
            displayLabel.append(value->GetDisplayValue());

            // aggregate consecutive string label definitions into one
            if (value->GetTypeName().EqualsI("string"))
                {
                aggregatedValue.AddValue(*value, separator);
                }
            else
                {
                if (!aggregatedValue.Empty())
                    parts.push_back(aggregatedValue.GetValueAndReset());
                parts.push_back(value);
                }
            }

        if (displayLabel.empty())
            {
            ctx.SetResultNull();
            return;
            }

        if (!aggregatedValue.Empty())
            parts.push_back(aggregatedValue.GetValueAndReset());

        if (1 == parts.size())
            {
            Utf8String resultJson = parts.front()->ToJsonString();
            ctx.SetResultText(resultJson.c_str(), resultJson.size(), DbFunction::Context::CopyData::Yes);
            return;
            }

        std::unique_ptr<LabelDefinition::CompositeRawValue> compositeValue = std::make_unique<LabelDefinition::CompositeRawValue>(separator, parts);
        Utf8String resultJson = LabelDefinition::Create(displayLabel.c_str(), std::move(compositeValue))->ToJsonString();
        ctx.SetResultText(resultJson.c_str(), resultJson.size(), DbFunction::Context::CopyData::Yes);
        }
    };

/*=================================================================================**//**
* Based on https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CompareDoublesScalar : ECPresentation::ScalarFunction
    {
    union Double_t
        {
        int64_t i;
        double d;
        Double_t(double num = 0) : d(num) {}
        bool Negative() const { return i < 0; }
        int64_t RawMantissa() const { return i & (((int64_t)1 << 52) - 1); }
        int64_t RawExponent() const { return (i >> 52) & 0xFF; }
#ifndef NDEBUG
        struct
            {
            int64_t mantissa : 52;
            int64_t exponent : 11;
            int64_t sign : 1;
            } parts;
#endif
        };
    CompareDoublesScalar(CustomFunctionsManager const& manager)
        : ScalarFunction(FUNCTION_NAME_CompareDoubles, 2, DbValueType::IntegerVal, manager)
        {}
    int AlmostEqualUlpsAndAbs(double lhs, double rhs, double maxDiff, int maxUlpsDiff)
        {
        // Check if the numbers are really close -- needed
        // when comparing numbers near zero.
        double absDiff = fabs(lhs - rhs);
        if (absDiff <= maxDiff)
            return 0;

        Double_t uLhs(lhs);
        Double_t uRhs(rhs);

        // Different signs means they do not match.
        if (uLhs.Negative() && !uRhs.Negative())
            return -1;
        if (!uLhs.Negative() && uRhs.Negative())
            return 1;

        // Find the difference in ULPs.
        int64_t sub = uLhs.i - uRhs.i;
        if (llabs(sub) <= maxUlpsDiff)
            return 0;

        return (sub < 0) ? -1 : 1;
        }
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(2);

        bool isLhsNull = args[0].IsNull();
        bool isRhsNull = args[1].IsNull();
        if (isLhsNull && isRhsNull)
            ctx.SetResultInt(0);
        else if (isLhsNull && !isRhsNull)
            ctx.SetResultInt(-1);
        else if (!isLhsNull && isRhsNull)
            ctx.SetResultInt(1);
        else
            {
            double lhs = args[0].GetValueDouble();
            double rhs = args[1].GetValueDouble();
            ctx.SetResultInt(AlmostEqualUlpsAndAbs(lhs, rhs, FLT_EPSILON, 4));
            }
        }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPropertyValueScalarCacheKey
    {
    ECClassId m_classId;
    Utf8String m_propertyName;
    bool operator<(ECPropertyValueScalarCacheKey const& other) const
        {
        return m_classId < other.m_classId
            || m_classId == other.m_classId && m_propertyName.CompareTo(other.m_propertyName) < 0;
        }
    };

/*=================================================================================**//**
* Base class for customs functions that computes formatted property value.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPropertyValueScalarBase : CachingScalarFunction<bmap<ECPropertyValueScalarCacheKey, ECPropertyCP>>
{
protected:
    template<typename... Args> ECPropertyValueScalarBase(Args&&... args)
        : CachingScalarFunction(std::forward<Args>(args)...)
        {}

    ECPropertyCP GetProperty(ECPropertyValueScalarCacheKey const& key)
        {
        auto iter = GetCache().find(key);
        if (GetCache().end() == iter)
            {
            ECClassCP ecClass = GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas().GetClass(key.m_classId);
            if (nullptr == ecClass)
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Invalid ECClassId: %" PRIu64, key.m_classId.GetValue()));

            ECPropertyCP ecProperty = ecClass->GetPropertyP(key.m_propertyName);
            if (nullptr == ecProperty)
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Property '%s' not found in ECClass '%s'", key.m_propertyName.c_str(), ecClass->GetFullName()));

            iter = GetCache().Insert(key, ecProperty).first;
            }

        return iter->second;
        }

    bool GetFormattedValue(Utf8String& formattedValue, ECPropertyValueScalarCacheKey const& key, DbValue const& sqlValue, ECPresentation::UnitSystem unitSystem)
        {
        ECPropertyCP ecProperty = GetProperty(key);
        if (nullptr == ecProperty || !ecProperty->GetIsPrimitive())
            return false;

        PrimitiveECPropertyCR primitiveProperty = *ecProperty->GetAsPrimitiveProperty();
        formattedValue = GetFormattedPropertyValue(primitiveProperty, sqlValue, GetContext().GetPropertyFormatter(), unitSystem);
        return true;
        }
};

/*=================================================================================**//**
* Parameters:
* - ECClassId
* - ECProperty name
* - ECProperty value
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetECPropertyValueDisplayLabelScalar : ECPropertyValueScalarBase
    {
    GetECPropertyValueDisplayLabelScalar(CustomFunctionsManager const& manager)
        : ECPropertyValueScalarBase(FUNCTION_NAME_GetECPropertyValueLabel, 3, DbValueType::TextVal, manager)
        {}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(3);

        ECClassId classId = args[0].GetValueId<ECClassId>();
        Utf8CP propertyName = args[1].GetValueText();
        Utf8CP valueStr = args[2].GetValueText();

        if (nullptr == valueStr || 0 == *valueStr)
            {
            ctx.SetResultNull();
            return;
            }

        LabelDefinitionPtr labelDefinition = LabelDefinition::Create();
        ECPropertyCP ecProperty = GetProperty({ classId, propertyName });
        if (nullptr == ecProperty)
            {
            ctx.SetResultError("Could not find ECProperty");
            return;
            }

        if (ecProperty->GetIsPrimitive())
            {
            PrimitiveECPropertyCR primitiveProperty = *ecProperty->GetAsPrimitiveProperty();
            ECValue value = ValueHelpers::GetECValueFromSqlValue(primitiveProperty.GetType(), args[2]);
            Utf8String formattedValue = GetFormattedPropertyValue(primitiveProperty, args[2], GetContext().GetPropertyFormatter(), GetContext().GetUnitSystem());
            labelDefinition->SetECValue(value, formattedValue.c_str());
            }

        if (!labelDefinition->IsDefinitionValid())
            labelDefinition->SetStringValue(valueStr);

        if (!labelDefinition->IsDefinitionValid())
            {
            ctx.SetResultNull();
            return;
            }

        Utf8String result = labelDefinition->ToJsonString();
        ctx.SetResultText(result.c_str(), result.size(), DbFunction::Context::CopyData::Yes);
        }
    };

/*=================================================================================**//**
* Parameters:
* - LabelDefinition json string
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetLabelDefinitionDisplayValueScalar : CachingScalarFunction<bmap<Utf8String, std::shared_ptr<Utf8String>>>
    {
    GetLabelDefinitionDisplayValueScalar(CustomFunctionsManager const& manager)
        : CachingScalarFunction(FUNCTION_NAME_GetLabelDefinitionDisplayValue, 1, DbValueType::TextVal, manager)
        {}

    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION(1);

        Utf8CP displayLabelDefinitionJson = args[0].GetValueText();

        auto iter = GetCache().find(displayLabelDefinitionJson);
        if (GetCache().end() == iter)
            {
            Utf8String displayValue = LabelDefinition::FromString(displayLabelDefinitionJson)->GetDisplayValue();
            iter = GetCache().Insert(displayLabelDefinitionJson, std::make_shared<Utf8String>(displayValue)).first;
            }

        ctx.SetResultText(iter->second->c_str(), iter->second->size(), DbFunction::Context::CopyData::No);
        }
    };

/*=================================================================================**//**
* Parameters:
* - ECClassId
* - ECProperty name
* - ECProperty value
* - Unit system
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GetFormattedValueScalar : ECPropertyValueScalarBase
{
private:
    ECPresentation::UnitSystem GetUnitSystem(Utf8CP unitSystem)
        {
        if (0 == BeStringUtilities::Stricmp(unitSystem, "Metric"))
            return ECPresentation::UnitSystem::Metric;
        if (0 == BeStringUtilities::Stricmp(unitSystem, "BritishImperial"))
            return ECPresentation::UnitSystem::BritishImperial;
        if (0 == BeStringUtilities::Stricmp(unitSystem, "UsCustomary"))
            return ECPresentation::UnitSystem::UsCustomary;
        if (0 == BeStringUtilities::Stricmp(unitSystem, "UsSurvey"))
            return ECPresentation::UnitSystem::UsSurvey;

        return ECPresentation::UnitSystem::Undefined;
        }

public:
    GetFormattedValueScalar(CustomFunctionsManager const& manager)
        : ECPropertyValueScalarBase(FUNCTION_NAME_GetFormattedValue, -1, DbValueType::TextVal, manager)
        {}

    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        ARGUMENTS_COUNT_PRECONDITION_CUSTOM(3 == nArgs || 4 == nArgs, "3 or 4");

        ECClassId classId = args[0].GetValueId<ECClassId>();
        Utf8CP propertyName = args[1].GetValueText();
        ECPresentation::UnitSystem unitSystem = 4 == nArgs ? GetUnitSystem(args[3].GetValueText()) : ECPresentation::UnitSystem::Undefined;

        Utf8String formattedValue;
        if (!GetFormattedValue(formattedValue, { classId, propertyName }, args[2], unitSystem))
            {
            ctx.SetResultError("Could not format property value");
            return;
            }

        ctx.SetResultText(formattedValue.c_str(), formattedValue.size(), DbFunction::Context::CopyData::Yes);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsContext::CustomFunctionsContext(ECSchemaHelper const& schemaHelper, IConnectionManagerCR connections, IConnectionCR connection,
    Utf8StringCR rulesetId, IRulesPreprocessorR rules, RulesetVariables const& rulesetVariables, IUsedRulesetVariablesListener* usedVariablesListener,
    ECExpressionsCache& ecexpressionsCache, NavNodesFactory const& nodesFactory, IUsedClassesListener* usedClassesListener,
    NavNodeCP parentNode, rapidjson::Value const* queryExtendedData, IECPropertyFormatter const* formatter, ECPresentation::UnitSystem unitSystem)
    : m_schemaHelper(schemaHelper), m_connections(connections), m_connection(connection), m_rules(rules),
    m_rulesetVariables(rulesetVariables), m_usedVariablesListener(usedVariablesListener), m_ecexpressionsCache(ecexpressionsCache),
    m_parentNode(parentNode), m_nodesFactory(nodesFactory), m_usedClassesListener(usedClassesListener), m_rulesetId(rulesetId),
    m_extendedData(queryExtendedData), m_propertyFormatter(formatter), m_unitSystem(unitSystem)
    {
    CustomFunctionsManager::GetManager().PushContext(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsContext::~CustomFunctionsContext()
    {
    for (ICustomFunctionsContextListener* listener : m_listeners)
        listener->_OnContextDisposed(*this);

    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, m_caches.empty(), "Expecting all custom function caches to be destroyed by now");

    CustomFunctionsContext* ctx = CustomFunctionsManager::GetManager().PopContext();
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, this == ctx, "Expecting popped context to equal `this`");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsContext::InsertCache(Utf8CP id, void* cache)
    {
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, nullptr == GetCache(id), Utf8PrintfString("Inserting cache that already exists: '%s'", id));
    m_caches.push_back(FunctionCache(id, cache));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsContext::_OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const
    {
    if (nullptr != rulesetId && !m_rulesetId.Equals(rulesetId))
        return;

    for (ICustomFunctionsContextListener* listener : m_listeners)
        listener->_OnUserSettingChanged(settingId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsManager& CustomFunctionsManager::GetManager()
    {
    static CustomFunctionsManager s_manager;
    return s_manager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsManager::CustomFunctionsManager()
    : m_contexts(new BeThreadLocalStorage())
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsManager::~CustomFunctionsManager()
    {
    BeMutexHolder lock(m_mutex);
    for (bvector<CustomFunctionsContext*>* contexts : m_allContexts)
        {
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, contexts->empty(), "All contexts are expected to be empty by now");
        delete contexts;
        }
    DELETE_AND_CLEAR(m_contexts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<CustomFunctionsContext*>& CustomFunctionsManager::GetContexts() const
    {
    void* ptr = m_contexts->GetValueAsPointer();
    bvector<CustomFunctionsContext*>* contexts = nullptr;
    if (nullptr != ptr)
        contexts = static_cast<bvector<CustomFunctionsContext*>*>(ptr);

    if (nullptr == contexts)
        {
        contexts = new bvector<CustomFunctionsContext*>();
        m_contexts->SetValueAsPointer(contexts);
        BeMutexHolder lock(m_mutex);
        m_allContexts.push_back(contexts);
        }
    return *contexts;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsManager::_OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const
    {
    for (CustomFunctionsContext* ctx : GetContexts())
        ctx->_OnSettingChanged(rulesetId, settingId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsContext& CustomFunctionsManager::GetCurrentContext() const {return *GetContexts().back();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsManager::PushContext(CustomFunctionsContext& context)
    {
    GetContexts().push_back(&context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsContext* CustomFunctionsManager::PopContext()
    {
    bvector<CustomFunctionsContext*>& contexts = GetContexts();
    if (contexts.empty())
        return nullptr;

    CustomFunctionsContext* back = contexts.back();
    contexts.pop_back();
    return back;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomFunctionsManager::IsContextEmpty()
    {
    return GetContexts().empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsInjector::CustomFunctionsInjector(IConnectionManagerCR connections) : m_connections(connections)
    {
    m_connections.AddListener(*this);
    CreateFunctions();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsInjector::CustomFunctionsInjector(IConnectionManagerCR connections, IConnectionCR connection)
    : m_connections(connections)
    {
    m_connections.AddListener(*this);
    CreateFunctions();
    OnConnection(connection);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFunctionsInjector::~CustomFunctionsInjector()
    {
    BeMutexHolder lock(m_mutex);
    for (IConnectionCP connection : m_handledConnections)
        {
        DisableProxyConnectionThreadVerification disableThreadVerification(*connection);
        Unregister(*connection);
        }
    m_connections.DropListener(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsInjector::CreateFunctions()
    {
    m_functions.push_back(std::make_shared<GetECInstanceDisplayLabelScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetECClassDisplayLabelScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetECPropertyDisplayLabelScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetSortingValueScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetRangeIndexScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetRangeImageIdScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<IsOfClassScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetECClassIdScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetECClassNameScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetECClassLabelScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetStringVariableValueScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetIntVariableValueScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetBoolVariableValueScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<InIntVariableValuesScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<HasVariableScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<EvaluateECExpressionScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetECEnumerationValueScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetInstanceKeyScalar>(CustomFunctionsManager::GetManager()));
#ifdef wip_skipped_instance_keys_performance_issue
    m_functions.push_back(std::make_shared<ECInstanceKeysArrayScalar>(CustomFunctionsManager::GetManager()));
#endif
    m_functions.push_back(std::make_shared<GetPointAsJsonStringScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<ArePointsEqualByValueScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<AreDoublesEqualByValueScalar>(CustomFunctionsManager::GetManager()));
#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
    m_functions.push_back(std::make_shared<GetPropertyDisplayValueScalar>(CustomFunctionsManager::GetManager()));
#endif
    m_functions.push_back(std::make_shared<GetNavigationPropertyValueScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetRelatedDisplayLabelScalar>(CustomFunctionsManager::GetManager(), FUNCTION_NAME_GetNavigationPropertyLabel));
    m_functions.push_back(std::make_shared<GetRelatedDisplayLabelScalar>(CustomFunctionsManager::GetManager(), FUNCTION_NAME_GetRelatedDisplayLabel));
    m_functions.push_back(std::make_shared<ToBase36Scalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<ParseBriefcaseIdScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<ParseLocalIdScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<JoinOptionallyRequiredScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<CompareDoublesScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetECPropertyValueDisplayLabelScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetLabelDefinitionDisplayValueScalar>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetPropertyValueJsonScalar>(CustomFunctionsManager::GetManager()));

    m_functions.push_back(std::make_shared<GetInstanceKeysAggregate>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetLimitedInstanceKeysAggregate>(CustomFunctionsManager::GetManager()));
#ifdef wip_skipped_instance_keys_performance_issue
    m_functions.push_back(std::make_shared<JsonArrayAggregate>(CustomFunctionsManager::GetManager()));
#endif
    m_functions.push_back(std::make_shared<JsonConcatAggregate>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<StringifiedJsonConcatAggregate>(CustomFunctionsManager::GetManager()));
    m_functions.push_back(std::make_shared<GetFormattedValueScalar>(CustomFunctionsManager::GetManager()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool RegisterFunctionsInDb(DbCR db, bvector<std::shared_ptr<DbFunction>> const& functions, std::function<bool(DbFunction const&)> const& pred)
    {
    bool didRegister = false;
    for (auto const& func : functions)
        {
        if (pred && !pred(*func))
            continue;

        DbResult result = (DbResult)db.AddFunction(*func);
        if (DbResult::BE_SQLITE_OK != result)
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_ERROR, Utf8PrintfString("Failed to add custom function '%s'. Result: %d", func->GetName(), (int)result))
        else
            didRegister = true;
        }
    return didRegister;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void UnregisterFunctionsFromDb(DbCR db, bvector<std::shared_ptr<DbFunction>> const& functions)
    {
    for (auto const& func : functions)
        {
        DbResult result = (DbResult)db.RemoveFunction(*func);
        if (DbResult::BE_SQLITE_OK != result)
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_ERROR, Utf8PrintfString("Failed to remove custom function '%s'. Result: %d", func->GetName(), (int)result));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct CustomFunctionsHolder : Db::AppData
{
private:
    Key m_key;
    bvector<std::shared_ptr<DbFunction>> m_functions;
public:
    CustomFunctionsHolder(bvector<std::shared_ptr<DbFunction>> funcs)
        : m_functions(funcs)
        {}
    ~CustomFunctionsHolder()
        {
        // note: we don't need the functions to be unregistered, we just need to make sure they're
        // valid until the db is closed
        }
    Key const& GetKey() const { return m_key; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsInjector::Register(IConnectionCR connection, bool primary)
    {
    bool isPrimary = (&connection.GetECDb() == &connection.GetDb());
    if (!isPrimary && connection.GetDb().IsDbOpen())
        RegisterFunctionsInDb(connection.GetDb(), m_functions, nullptr);

    BeSQLite::DbFunction* existingFuncPtr;
    bool didRegister = RegisterFunctionsInDb(connection.GetECDb(), m_functions, [&](DbFunction const& f)
        {
        return !connection.GetECDb().TryGetSqlFunction(existingFuncPtr, f.GetName(), f.GetNumArgs());
        });
    if (didRegister)
        {
        RefCountedPtr<CustomFunctionsHolder> functionsHolder = new CustomFunctionsHolder(m_functions);
        connection.GetECDb().AddAppData(functionsHolder->GetKey(), functionsHolder.get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsInjector::Unregister(IConnectionCR connection, bool primary)
    {
    bool isPrimary = (&connection.GetECDb() == &connection.GetDb());
    if (!isPrimary && connection.GetDb().IsDbOpen())
        UnregisterFunctionsFromDb(connection.GetDb(), m_functions);

    // note: no need to unregister from primary - we hold the registered functions in its app data
    // and they will be available there until the db is closed
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomFunctionsInjector::Handles(IConnectionCR connection) const
    {
    return m_handledConnections.end() != m_handledConnections.find(&connection);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsInjector::OnConnection(IConnectionCR connection)
    {
    BeMutexHolder lock(m_mutex);

    if (Handles(connection))
        return;

    m_handledConnections.insert(&connection);
    Register(connection);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFunctionsInjector::_OnConnectionEvent(ConnectionEvent const& evt)
    {
    if (ConnectionEventType::Closed == evt.GetEventType())
        {
        BeMutexHolder lock(m_mutex);
        bvector<IConnectionCP> toErase;
        for (IConnectionCP conn : m_handledConnections)
            {
            if (conn->GetId() != evt.GetConnection().GetId())
                continue;

            Unregister(*conn);
            toErase.push_back(conn);
            }
        for (IConnectionCP conn : toErase)
            m_handledConnections.erase(conn);
        }
    else if (ConnectionEventType::Opened == evt.GetEventType() || ConnectionEventType::ProxyCreated == evt.GetEventType())
        {
        OnConnection(evt.GetConnection());
        }
    }
