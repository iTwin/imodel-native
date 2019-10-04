/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenProviderContext : RefCountedBase
{
private:
    // common
    PresentationRuleSetCPtr m_ruleset;
    Utf8String m_locale;
    IJsonLocalState const* m_localState;
    IUserSettings const& m_userSettings;
    mutable RefCountedPtr<UsedUserSettingsListener> m_usedSettingsListener;
    RelatedPathsCache& m_relatedPathsCache;
    PolymorphicallyRelatedClassesCache& m_polymorphicallyRelatedClassesCache;
    ECExpressionsCache& m_ecexpressionsCache;
    JsonNavNodesFactory const& m_nodesFactory;
    ICancelationTokenCPtr m_cancelationToken;

    // Property formatting context
    IECPropertyFormatter const* m_propertyFormatter;
    
    // localization context
    bool m_isLocalizationContext;
    ILocalizationProvider const* m_localizationProvider;
    
    // ECDb context
    bool m_isQueryContext;
    IConnectionManagerCP m_connections;
    IConnectionCP m_connection;
    ECSchemaHelper* m_schemaHelper;

private:
    void Init();

protected:
    ECPRESENTATION_EXPORT RulesDrivenProviderContext(PresentationRuleSetCR, Utf8String locale, IUserSettings const&, ECExpressionsCache&, 
        RelatedPathsCache&, PolymorphicallyRelatedClassesCache&, JsonNavNodesFactory const&, IJsonLocalState const*);
    ECPRESENTATION_EXPORT RulesDrivenProviderContext(RulesDrivenProviderContextCR other);
    
    ECPRESENTATION_EXPORT void SetQueryContext(IConnectionManagerCR, IConnectionCR);
    ECPRESENTATION_EXPORT void SetQueryContext(RulesDrivenProviderContextCR other);

public:
    ECPRESENTATION_EXPORT ~RulesDrivenProviderContext();

    // common
    PresentationRuleSetCR GetRuleset() const {return *m_ruleset;}
    JsonNavNodesFactory const& GetNodesFactory() const {return m_nodesFactory;}
    IUserSettings const& GetUserSettings() const {return m_userSettings;}
    ECPRESENTATION_EXPORT IUsedUserSettingsListener& GetUsedSettingsListener() const;
    ECPRESENTATION_EXPORT void SetUsedSettingsListener(RulesDrivenProviderContextCR other);
    bset<Utf8String> GetRelatedSettingIds() const;
    ECExpressionsCache& GetECExpressionsCache() const {return m_ecexpressionsCache;}
    IJsonLocalState const* GetLocalState() const {return m_localState;}
    ECPRESENTATION_EXPORT ICancelationTokenCR GetCancelationToken() const;
    void SetCancelationToken(ICancelationTokenCP token) {m_cancelationToken = token;}
    Utf8StringCR GetLocale() const {return m_locale;}
    ECPRESENTATION_EXPORT void Adopt(IConnectionCR, ICancelationTokenCP);
    ECPRESENTATION_EXPORT void Adopt(RulesDrivenProviderContextCR);
    ECPRESENTATION_EXPORT void AdoptToSameConnection(ICancelationTokenCP);

    // Property formatting context
    ECPRESENTATION_EXPORT void SetPropertyFormattingContext(IECPropertyFormatter const&);
    ECPRESENTATION_EXPORT void SetPropertyFormattingContext(RulesDrivenProviderContextCR);
    bool IsPropertyFormattingContext() const {return nullptr != m_propertyFormatter;}
    IECPropertyFormatter const& GetECPropertyFormatter() const {BeAssert(IsPropertyFormattingContext()); return *m_propertyFormatter;}
    
    // localization context
    ECPRESENTATION_EXPORT void SetLocalizationContext(ILocalizationProvider const& provider);
    ECPRESENTATION_EXPORT void SetLocalizationContext(RulesDrivenProviderContextCR other);
    bool IsLocalizationContext() const {return m_isLocalizationContext;}
    ILocalizationProvider const& GetLocalizationProvider() const {BeAssert(IsLocalizationContext()); return *m_localizationProvider;}

    // ECDb context
    bool IsQueryContext() const {return m_isQueryContext;}
    IConnectionManagerCR GetConnections() const {BeAssert(IsQueryContext()); return *m_connections;}
    IConnectionCR GetConnection() const {BeAssert(IsQueryContext()); return *m_connection;}
    ECSchemaHelper const& GetSchemaHelper() const {BeAssert(IsQueryContext()); return *m_schemaHelper;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
