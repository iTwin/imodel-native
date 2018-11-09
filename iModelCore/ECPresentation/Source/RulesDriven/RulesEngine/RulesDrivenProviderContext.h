/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/RulesDrivenProviderContext.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <Logging/bentleylogging.h>
#include "RulesEngineTypes.h"
#include "CustomFunctions.h"
#include "LocalizationHelper.h"
#include "ECSchemaHelper.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct UsedUserSettingsListener;

struct UsedUserSettingsListener;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenProviderContext : RefCountedBase
{
private:
    // common
    PresentationRuleSetCR m_ruleset;
    bool m_holdsRuleset;
    Utf8String m_locale;
    IJsonLocalState const* m_localState;
    IUserSettings const& m_userSettings;
    mutable UsedUserSettingsListener* m_usedSettingsListener;
    RelatedPathsCache& m_relatedPathsCache;
    PolymorphicallyRelatedClassesCache& m_polymorphicallyRelatedClassesCache;
    ECExpressionsCache& m_ecexpressionsCache;
    JsonNavNodesFactory const& m_nodesFactory;
    ICancelationTokenCP m_cancelationToken;
    
    // localization context
    bool m_isLocalizationContext;
    ILocalizationProvider const* m_localizationProvider;
    
    // ECDb context
    bool m_isQueryContext;
    IConnectionManagerCP m_connections;
    IConnectionCP m_connection;
    ECSqlStatementCache const* m_statementCache;
    ECSchemaHelper* m_schemaHelper;

private:
    void Init();

protected:
    ECPRESENTATION_EXPORT RulesDrivenProviderContext(PresentationRuleSetCR, bool holdRuleset, Utf8String locale, IUserSettings const&, ECExpressionsCache&, 
        RelatedPathsCache&, PolymorphicallyRelatedClassesCache&, JsonNavNodesFactory const&, IJsonLocalState const*);
    ECPRESENTATION_EXPORT RulesDrivenProviderContext(RulesDrivenProviderContextCR other);
    
    ECPRESENTATION_EXPORT void SetQueryContext(IConnectionManagerCR, IConnectionCR, ECSqlStatementCache const&, CustomFunctionsInjector&);
    ECPRESENTATION_EXPORT void SetQueryContext(RulesDrivenProviderContextCR other);

public:
    ECPRESENTATION_EXPORT ~RulesDrivenProviderContext();

    // common
    PresentationRuleSetCR GetRuleset() const {return m_ruleset;}
    JsonNavNodesFactory const& GetNodesFactory() const {return m_nodesFactory;}
    IUserSettings const& GetUserSettings() const {return m_userSettings;}
    ECPRESENTATION_EXPORT IUsedUserSettingsListener& GetUsedSettingsListener() const;
    bvector<Utf8String> GetRelatedSettingIds() const;
    ECExpressionsCache& GetECExpressionsCache() const {return m_ecexpressionsCache;}
    IJsonLocalState const* GetLocalState() const {return m_localState;}
    ECPRESENTATION_EXPORT ICancelationTokenCR GetCancelationToken() const;
    void SetCancelationToken(ICancelationTokenCP token) {m_cancelationToken = token;}
    Utf8StringCR GetLocale() const {return m_locale;}
    
    // localization context
    ECPRESENTATION_EXPORT void SetLocalizationContext(ILocalizationProvider const& provider);
    ECPRESENTATION_EXPORT void SetLocalizationContext(RulesDrivenProviderContextCR other);
    bool IsLocalizationContext() const {return m_isLocalizationContext;}
    ILocalizationProvider const& GetLocalizationProvider() const {BeAssert(IsLocalizationContext()); return *m_localizationProvider;}

    // ECDb context
    bool IsQueryContext() const {return m_isQueryContext;}
    IConnectionManagerCR GetConnections() const {BeAssert(IsQueryContext()); return *m_connections;}
    IConnectionCR GetConnection() const {BeAssert(IsQueryContext()); return *m_connection;}
    ECSqlStatementCache const& GetStatementCache() const {BeAssert(IsQueryContext()); return *m_statementCache;}
    ECSchemaHelper const& GetSchemaHelper() const {BeAssert(IsQueryContext()); return *m_schemaHelper;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
