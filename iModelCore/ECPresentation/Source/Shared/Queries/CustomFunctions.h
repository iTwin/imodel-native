/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/UserSettings.h>
#include <ECPresentation/Rules/PresentationRuleSet.h>
#include "../ECExpressions/ECExpressionContextsProvider.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define FUNCTION_NAME_GetECInstanceDisplayLabel     "GetECInstanceDisplayLabel"
#define FUNCTION_NAME_GetECClassDisplayLabel        "GetECClassDisplayLabel"
#define FUNCTION_NAME_GetECPropertyDisplayLabel     "GetECPropertyDisplayLabel"
#define FUNCTION_NAME_GetNavigationPropertyLabel    "GetNavigationPropertyLabel"
#define FUNCTION_NAME_GetNavigationPropertyValue    "GetNavigationPropertyValue"
#define FUNCTION_NAME_GetSortingValue               "GetSortingValue"
#define FUNCTION_NAME_GetRangeIndex                 "GetRangeIndex"
#define FUNCTION_NAME_GetRangeImageId               "GetRangeImageId"
#define FUNCTION_NAME_IsOfClass                     "IsOfClass"
#define FUNCTION_NAME_GetECClassId                  "RulesEngine_GetECClassId"
#define FUNCTION_NAME_GetECClassName                "RulesEngine_GetECClassName"
#define FUNCTION_NAME_GetECClassLabel               "RulesEngine_GetECClassLabel"
#define FUNCTION_NAME_JsonConcat                    "JsonConcat"
#define FUNCTION_NAME_StringifiedJsonConcat         "StringifiedJsonConcat"
#define FUNCTION_NAME_GetPropertyValueJson          "GetPropertyValueJson"
#define FUNCTION_NAME_GetLimitedInstanceKeys        "GetLimitedInstanceKeys"
#define FUNCTION_NAME_GetInstanceKeys               "GetInstanceKeys"
#define FUNCTION_NAME_GetInstanceKey                "GetInstanceKey"
#define FUNCTION_NAME_GetVariableStringValue        "GetVariableStringValue"
#define FUNCTION_NAME_GetVariableIntValue           "GetVariableIntValue"
#define FUNCTION_NAME_GetVariableBoolValue          "GetVariableBoolValue"
#define FUNCTION_NAME_InVariableIntValues           "InVariableIntValues"
#define FUNCTION_NAME_HasVariable                   "HasVariable"
#define FUNCTION_NAME_EvaluateECExpression          "EvaluateECExpression"
#define FUNCTION_NAME_GetECEnumerationValue         "GetECEnumerationValue"
#ifdef wip_skipped_instance_keys_performance_issue
    #define FUNCTION_NAME_ECInstanceKeysArray       "ECInstanceKeysArray"
    #define FUNCTION_NAME_AggregateJsonArray        "AggregateJsonArray"
#endif
#define FUNCTION_NAME_GetPointAsJsonString          "GetPointAsJsonString"
#define FUNCTION_NAME_ArePointsEqualByValue         "ArePointsEqualByValue"
#define FUNCTION_NAME_AreDoublesEqualByValue        "AreDoublesEqualByValue"
#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
    #define FUNCTION_NAME_GetPropertyDisplayValue   "GetPropertyDisplayValue"
#endif
#define FUNCTION_NAME_GetRelatedDisplayLabel        "GetRelatedDisplayLabel"
#define FUNCTION_NAME_ToBase36                      "ToBase36"
#define FUNCTION_NAME_ParseBriefcaseId              "ParseBriefcaseId"
#define FUNCTION_NAME_ParseLocalId                  "ParseLocalId"
#define FUNCTION_NAME_JoinOptionallyRequired        "JoinOptionallyRequired"
#define FUNCTION_NAME_CompareDoubles                "CompareDoubles"
#define FUNCTION_NAME_GetECPropertyValueLabel       "GetECPropertyValueLabel"
#define FUNCTION_NAME_GetLabelDefinitionDisplayValue "GetLabelDisplayValue"
#define FUNCTION_NAME_GetFormattedValue             "GetFormattedValue"

struct NavNodesFactory;
struct ScalarFunction;
struct AggregateFunction;
struct CustomFunctionsContext;
struct IUsedClassesListener;
struct ECSchemaHelper;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ICustomFunctionsContextListener
    {
    virtual ~ICustomFunctionsContextListener() {}
    virtual void _OnContextDisposed(CustomFunctionsContext&) {}
    virtual void _OnUserSettingChanged(Utf8CP settingId) {}
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CustomFunctionsContext : IUserSettingsChangeListener
{
    struct FunctionCache
        {
        Utf8CP m_name;
        void* m_ptr;
        FunctionCache() : m_name(nullptr), m_ptr(nullptr) {}
        FunctionCache(Utf8CP name, void* ptr) : m_name(name), m_ptr(ptr) {}
        };

private:
    ECSchemaHelper const& m_schemaHelper;
    IConnectionManagerCR m_connections;
    IConnectionCR m_connection;
    Utf8StringCR m_rulesetId;
    IRulesPreprocessorR m_rules;
    NavNodesFactory const& m_nodesFactory;
    RulesetVariables const& m_rulesetVariables;
    IUsedRulesetVariablesListener* m_usedVariablesListener;
    ECExpressionsCache& m_ecexpressionsCache;
    IUsedClassesListener* m_usedClassesListener;
    NavNodeCP m_parentNode;
    rapidjson::Value const* m_extendedData;
    bvector<FunctionCache> m_caches;
    bset<ICustomFunctionsContextListener*> m_listeners;
    IECPropertyFormatter const* m_propertyFormatter;
    ECPresentation::UnitSystem m_unitSystem;

public:
    ECPRESENTATION_EXPORT CustomFunctionsContext(ECSchemaHelper const&, IConnectionManagerCR, IConnectionCR, Utf8StringCR rulesetId, IRulesPreprocessorR,
        RulesetVariables const&, IUsedRulesetVariablesListener*, ECExpressionsCache&, NavNodesFactory const&, IUsedClassesListener*,
        NavNodeCP parentNode, rapidjson::Value const* queryExtendedData, IECPropertyFormatter const* formatter = nullptr,
        ECPresentation::UnitSystem unitSystem = ECPresentation::UnitSystem::Undefined);
    ECPRESENTATION_EXPORT ~CustomFunctionsContext();

    ECPRESENTATION_EXPORT void _OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const override;

    ECSchemaHelper const& GetSchemaHelper() const {return m_schemaHelper;}
    IConnectionManagerCR GetConnections() const {return m_connections;}
    IConnectionCR GetConnection() const {return m_connection;}
    IRulesPreprocessorR GetRules() const {return m_rules;}
    RulesetVariables const& GetRulesetVariables() const {return m_rulesetVariables;}
    IUsedRulesetVariablesListener* GetUsedRulesetVariablesListener() const {return m_usedVariablesListener;}
    ECExpressionsCache& GetECExpressionsCache() const {return m_ecexpressionsCache;}
    NavNodesFactory const& GetNodesFactory() const {return m_nodesFactory;}
    IUsedClassesListener* GetUsedClassesListener() const {return m_usedClassesListener;}
    NavNodeCP GetParentNode() const {return m_parentNode;}
    rapidjson::Value const* GetQueryExtendedData() const {return m_extendedData;}
    IECPropertyFormatter const* GetPropertyFormatter() const {return m_propertyFormatter;}
    ECPresentation::UnitSystem GetUnitSystem() const {return m_unitSystem;}

    void* GetCache(Utf8CP id) const;
    void InsertCache(Utf8CP id, void* cache);
    void RemoveCache(Utf8CP id);

    void AddListener(ICustomFunctionsContextListener& listener) {m_listeners.insert(&listener);}
    void RemoveListener(ICustomFunctionsContextListener& listener) {m_listeners.erase(&listener);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CustomFunctionsInjector : IConnectionsListener
{
private:
    IConnectionManagerCR m_connections;
    bset<IConnectionCP> m_handledConnections;
    bvector<std::shared_ptr<DbFunction>> m_functions;
    mutable BeMutex m_mutex;

private:
    void CreateFunctions();
    void Register(IConnectionCR, bool primary = false);
    void Unregister(IConnectionCR, bool primary = false);
    void OnConnection(IConnectionCR);

protected:
    void _OnConnectionEvent(ConnectionEvent const&) override;

public:
    ECPRESENTATION_EXPORT CustomFunctionsInjector(IConnectionManagerCR);
    ECPRESENTATION_EXPORT CustomFunctionsInjector(IConnectionManagerCR, IConnectionCR);
    ECPRESENTATION_EXPORT ~CustomFunctionsInjector();
    ECPRESENTATION_EXPORT bool Handles(IConnectionCR connection) const;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CustomFunctionsManager : NonCopyableClass, IUserSettingsChangeListener
{
private:
    mutable BeMutex m_mutex;
    mutable BeThreadLocalStorage* m_contexts;
    mutable bvector<bvector<CustomFunctionsContext*>*> m_allContexts;
    bvector<CustomFunctionsContext*>& GetContexts() const;
public:
    CustomFunctionsManager();
    ~CustomFunctionsManager();
    ECPRESENTATION_EXPORT static CustomFunctionsManager& GetManager();
    void _OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const override;
    ECPRESENTATION_EXPORT CustomFunctionsContext& GetCurrentContext() const;
    void PushContext(CustomFunctionsContext& context);
    ECPRESENTATION_EXPORT CustomFunctionsContext* PopContext();
    ECPRESENTATION_EXPORT bool IsContextEmpty();
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
