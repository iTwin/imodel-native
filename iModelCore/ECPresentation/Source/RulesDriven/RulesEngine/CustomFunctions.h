/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/CustomFunctions.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RulesDriven/UserSettings.h>
#include <ECPresentationRules/PresentationRuleSet.h>
#include "ECExpressionContextsProvider.h"
#include "../../ECDbBasedCache.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define FUNCTION_NAME_GetECInstanceDisplayLabel     "GetECInstanceDisplayLabel"
#define FUNCTION_NAME_GetECClassDisplayLabel        "GetECClassDisplayLabel"
#define FUNCTION_NAME_GetECPropertyDisplayLabel     "GetECPropertyDisplayLabel"
#define FUNCTION_NAME_GetSortingValue               "GetSortingValue"
#define FUNCTION_NAME_GetECClassPriority            "GetECClassPriority"
#define FUNCTION_NAME_GetRangeIndex                 "GetRangeIndex"
#define FUNCTION_NAME_GetRangeImageId               "GetRangeImageId"
#define FUNCTION_NAME_IsOfClass                     "IsOfClass"
#define FUNCTION_NAME_GetECClassId                  "RulesEngine_GetECClassId"
#define FUNCTION_NAME_GetGroupedInstanceKeys        "GetGroupedInstanceKeys"
#define FUNCTION_NAME_GetInstanceKey                "GetInstanceKey"
#define FUNCTION_NAME_GetMergedValue                "GetMergedValue"
#define FUNCTION_NAME_GetSettingValue               "GetSettingValue"
#define FUNCTION_NAME_GetSettingIntValue            "GetSettingIntValue"
#define FUNCTION_NAME_GetSettingBoolValue           "GetSettingBoolValue"
#define FUNCTION_NAME_InSettingIntValues            "InSettingIntValues"
#define FUNCTION_NAME_HasSetting                    "HasSetting"
#define FUNCTION_NAME_EvaluateECExpression          "EvaluateECExpression"
#define FUNCTION_NAME_IsRecursivelyRelated          "IsRecursivelyRelated"
#define FUNCTION_NAME_GetECEnumerationValue         "GetECEnumerationValue"
#define FUNCTION_NAME_ECInstanceKeysArray           "ECInstanceKeysArray"
#define FUNCTION_NAME_AggregateJsonArray            "AggregateJsonArray"
#define FUNCTION_NAME_GetPointAsJsonString          "GetPointAsJsonString"

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
    BeSQLite::EC::ECDbR m_db;
    ECN::PresentationRuleSetCR m_ruleset;
    JsonNavNodesFactory const& m_nodesFactory;
    IUserSettings const& m_userSettings;
    IUsedUserSettingsListener* m_usedSettingsListener;
    ECExpressionsCache& m_ecexpressionsCache;
    ILocalizationProvider const* m_localizationProvider;
    IUsedClassesListener* m_usedClassesListener;
    NavNodeCP m_parentNode;
    rapidjson::Value const* m_extendedData;
    bvector<FunctionCache> m_caches;
    bset<ICustomFunctionsContextListener*> m_listeners;
    
public:
    ECPRESENTATION_EXPORT CustomFunctionsContext(BeSQLite::EC::ECDbR, ECN::PresentationRuleSetCR, IUserSettings const&, IUsedUserSettingsListener*,
        ECExpressionsCache&, JsonNavNodesFactory const&, IUsedClassesListener*, NavNodeCP parentNode, rapidjson::Value const* queryExtendedData);
    ECPRESENTATION_EXPORT ~CustomFunctionsContext();

    ECPRESENTATION_EXPORT void _OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const override;

    void SetLocalizationProvider(ILocalizationProvider const& provider) {m_localizationProvider = &provider;}
    ILocalizationProvider const* GetLocalizationProvider() const {return m_localizationProvider;}
    BeSQLite::EC::ECDbR GetDb() const {return m_db;}
    ECN::PresentationRuleSetCR GetRuleset() const {return m_ruleset;}
    IUserSettings const& GetUserSettings() const {return m_userSettings;}
    IUsedUserSettingsListener* GetUsedUserSettingsListener() const {return m_usedSettingsListener;}
    ECExpressionsCache& GetECExpressionsCache() const {return m_ecexpressionsCache;}
    JsonNavNodesFactory const& GetNodesFactory() const {return m_nodesFactory;}
    IUsedClassesListener* GetUsedClassesListener() const {return m_usedClassesListener;}
    NavNodeCP GetParentNode() const {return m_parentNode;}
    rapidjson::Value const* GetQueryExtendedData() const {return m_extendedData;}

    void* GetCache(Utf8CP id) const;
    void InsertCache(Utf8CP id, void* cache);
    void RemoveCache(Utf8CP id);

    void AddListener(ICustomFunctionsContextListener& listener) {m_listeners.insert(&listener);}
    void RemoveListener(ICustomFunctionsContextListener& listener) {m_listeners.erase(&listener);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct CustomFunctionsInjector : ECDbBasedCache
{
private:
    bset<BeSQLite::EC::ECDb const*> m_handledDbs;
    bvector<ScalarFunction*> m_scalarFunctions;
    bvector<AggregateFunction*> m_aggregateFunctions;

private:
    void CreateFunctions();
    void DestroyFunctions();
    void RegisterInDb(BeSQLite::EC::ECDbCR);
    void UnregisterFromDb(BeSQLite::EC::ECDbCR);

protected:
    void _ClearECDbCache(BeSQLite::EC::ECDbCR) override;

public:
    ECPRESENTATION_EXPORT CustomFunctionsInjector();
    ECPRESENTATION_EXPORT CustomFunctionsInjector(BeSQLite::EC::ECDbR);
    ECPRESENTATION_EXPORT ~CustomFunctionsInjector();
    ECPRESENTATION_EXPORT void OnConnection(BeSQLite::EC::ECDbCR);
    bool Handles(BeSQLite::EC::ECDbCR connection) const {return m_handledDbs.end() != m_handledDbs.find(&connection);}
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
