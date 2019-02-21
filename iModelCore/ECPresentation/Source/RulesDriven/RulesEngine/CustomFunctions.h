/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/CustomFunctions.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RulesDriven/UserSettings.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>
#include "ECExpressionContextsProvider.h"
#include "ECSchemaHelper.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define FUNCTION_NAME_GetECInstanceDisplayLabel     "GetECInstanceDisplayLabel"
#define FUNCTION_NAME_GetECClassDisplayLabel        "GetECClassDisplayLabel"
#define FUNCTION_NAME_GetECPropertyDisplayLabel     "GetECPropertyDisplayLabel"
#define FUNCTION_NAME_GetNavigationPropertyLabel    "GetNavigationPropertyLabel"
#define FUNCTION_NAME_GetSortingValue               "GetSortingValue"
#define FUNCTION_NAME_GetRangeIndex                 "GetRangeIndex"
#define FUNCTION_NAME_GetRangeImageId               "GetRangeImageId"
#define FUNCTION_NAME_IsOfClass                     "IsOfClass"
#define FUNCTION_NAME_GetECClassId                  "RulesEngine_GetECClassId"
#define FUNCTION_NAME_GetGroupedInstanceKeys        "GetGroupedInstanceKeys"
#define FUNCTION_NAME_GetInstanceKey                "GetInstanceKey"
#define FUNCTION_NAME_GetMergedValue                "GetMergedValue"
#define FUNCTION_NAME_GetVariableStringValue        "GetVariableStringValue"
#define FUNCTION_NAME_GetVariableIntValue           "GetVariableIntValue"
#define FUNCTION_NAME_GetVariableBoolValue          "GetVariableBoolValue"
#define FUNCTION_NAME_InVariableIntValues           "InVariableIntValues"
#define FUNCTION_NAME_HasVariable                   "HasVariable"
#define FUNCTION_NAME_EvaluateECExpression          "EvaluateECExpression"
#define FUNCTION_NAME_IsRecursivelyRelated          "IsRecursivelyRelated"
#define FUNCTION_NAME_GetECEnumerationValue         "GetECEnumerationValue"
#define FUNCTION_NAME_ECInstanceKeysArray           "ECInstanceKeysArray"
#define FUNCTION_NAME_AggregateJsonArray            "AggregateJsonArray"
#define FUNCTION_NAME_GetPointAsJsonString          "GetPointAsJsonString"
#define FUNCTION_NAME_ArePointsEqualByValue         "ArePointsEqualByValue"
#define FUNCTION_NAME_AreDoublesEqualByValue        "AreDoublesEqualByValue"
#define FUNCTION_NAME_GetPropertyDisplayValue       "GetPropertyDisplayValue"
#define FUNCTION_NAME_GetRelatedDisplayLabel        "GetRelatedDisplayLabel"

struct JsonNavNodesFactory;
struct ILocalizationProvider;
struct NavigationQuery;
struct ScalarFunction;
struct AggregateFunction;
struct CustomFunctionsContext;
struct IUsedClassesListener;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct ICustomFunctionsContextListener
    {
    virtual ~ICustomFunctionsContextListener() {}
    virtual void _OnContextDisposed(CustomFunctionsContext&) {}
    virtual void _OnUserSettingChanged(Utf8CP settingId) {}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
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
    PresentationRuleSetCR m_ruleset;
    JsonNavNodesFactory const& m_nodesFactory;
    IUserSettings const& m_userSettings;
    IUsedUserSettingsListener* m_usedSettingsListener;
    ECExpressionsCache& m_ecexpressionsCache;
    ILocalizationProvider const* m_localizationProvider;
    Utf8String m_locale;
    IUsedClassesListener* m_usedClassesListener;
    JsonNavNodeCP m_parentNode;
    rapidjson::Value const* m_extendedData;
    bvector<FunctionCache> m_caches;
    bset<ICustomFunctionsContextListener*> m_listeners;
    IECPropertyFormatter const* m_propertyFormatter;
    
public:
    ECPRESENTATION_EXPORT CustomFunctionsContext(ECSchemaHelper const&, IConnectionManagerCR, IConnectionCR, PresentationRuleSetCR, Utf8String locale, IUserSettings const&, IUsedUserSettingsListener*,
        ECExpressionsCache&, JsonNavNodesFactory const&, IUsedClassesListener*, JsonNavNodeCP parentNode, rapidjson::Value const* queryExtendedData, IECPropertyFormatter const* formatter = nullptr);
    ECPRESENTATION_EXPORT ~CustomFunctionsContext();

    ECPRESENTATION_EXPORT void _OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const override;

    void SetLocalizationProvider(ILocalizationProvider const& provider) {m_localizationProvider = &provider;}
    ILocalizationProvider const* GetLocalizationProvider() const {return m_localizationProvider;}
    Utf8StringCR GetLocale() const {return m_locale;}
    ECSchemaHelper const& GetSchemaHelper() const {return m_schemaHelper;}
    IConnectionManagerCR GetConnections() const {return m_connections;}
    IConnectionCR GetConnection() const {return m_connection;}
    PresentationRuleSetCR GetRuleset() const {return m_ruleset;}
    IUserSettings const& GetUserSettings() const {return m_userSettings;}
    IUsedUserSettingsListener* GetUsedUserSettingsListener() const {return m_usedSettingsListener;}
    ECExpressionsCache& GetECExpressionsCache() const {return m_ecexpressionsCache;}
    JsonNavNodesFactory const& GetNodesFactory() const {return m_nodesFactory;}
    IUsedClassesListener* GetUsedClassesListener() const {return m_usedClassesListener;}
    JsonNavNodeCP GetParentNode() const {return m_parentNode;}
    rapidjson::Value const* GetQueryExtendedData() const {return m_extendedData;}
    IECPropertyFormatter const* GetPropertyFormatter() const {return m_propertyFormatter;}

    void* GetCache(Utf8CP id) const;
    void InsertCache(Utf8CP id, void* cache);
    void RemoveCache(Utf8CP id);

    void AddListener(ICustomFunctionsContextListener& listener) {m_listeners.insert(&listener);}
    void RemoveListener(ICustomFunctionsContextListener& listener) {m_listeners.erase(&listener);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct CustomFunctionsInjector : IConnectionsListener
{
private:
    IConnectionManagerCR m_connections;
    bset<IConnectionCP> m_handledConnections;
    bvector<ScalarFunction*> m_scalarFunctions;
    bvector<AggregateFunction*> m_aggregateFunctions;

private:
    void CreateFunctions();
    void DestroyFunctions();
    void Register(IConnectionCR, bool primary = false);
    void Unregister(IConnectionCR, bool primary = false);
    bool HandlesPrimaryConnection(Utf8StringCR connectionId) const;

protected:
    void _OnConnectionEvent(ConnectionEvent const&) override;

public:
    ECPRESENTATION_EXPORT CustomFunctionsInjector(IConnectionManagerCR);
    ECPRESENTATION_EXPORT CustomFunctionsInjector(IConnectionManagerCR, IConnectionCR);
    ECPRESENTATION_EXPORT ~CustomFunctionsInjector();
    ECPRESENTATION_EXPORT void OnConnection(IConnectionCR);
    ECPRESENTATION_EXPORT bool Handles(IConnectionCR connection) const;
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct CustomFunctionsManager : NonCopyableClass, IUserSettingsChangeListener
{
private:
    mutable bvector<CustomFunctionsContext*> m_contexts;

public:
    static CustomFunctionsManager& GetManager();
    void _OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const override;
    CustomFunctionsContext& GetCurrentContext() const;
    void PushContext(CustomFunctionsContext& context);
    CustomFunctionsContext* PopContext();
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
