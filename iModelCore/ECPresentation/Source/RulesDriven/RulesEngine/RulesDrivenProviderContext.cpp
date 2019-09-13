/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "RulesDrivenProviderContext.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2017
+===============+===============+===============+===============+===============+======*/
struct UsedUserSettingsListener : IUsedUserSettingsListener
{
private:
    bset<Utf8String> m_settingIds;
protected:
    void _OnUserSettingUsed(Utf8CP settingId) override {m_settingIds.insert(settingId);}
public:
    bset<Utf8String> const& GetSettingIds() const {return m_settingIds;}
};
END_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenProviderContext::RulesDrivenProviderContext(PresentationRuleSetCR ruleset, bool holdRuleset, Utf8String locale, IUserSettings const& settings, ECExpressionsCache& ecexpressionsCache, 
    RelatedPathsCache& relatedPathsCache, PolymorphicallyRelatedClassesCache& polymorphicallyRelatedClassesCache, JsonNavNodesFactory const& nodesFactory, IJsonLocalState const* localState)
    : m_ruleset(ruleset), m_holdsRuleset(holdRuleset), m_locale(locale), m_userSettings(settings), m_relatedPathsCache(relatedPathsCache), 
    m_polymorphicallyRelatedClassesCache(polymorphicallyRelatedClassesCache), m_ecexpressionsCache(ecexpressionsCache), m_nodesFactory(nodesFactory), 
    m_localState(localState)
    {
    if (holdRuleset)
        m_ruleset.AddRef();

    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenProviderContext::RulesDrivenProviderContext(RulesDrivenProviderContextCR other)
    : m_ruleset(other.m_ruleset), m_holdsRuleset(false), m_locale(other.m_locale), m_userSettings(other.m_userSettings), m_relatedPathsCache(other.m_relatedPathsCache), 
    m_polymorphicallyRelatedClassesCache(other.m_polymorphicallyRelatedClassesCache), m_ecexpressionsCache(other.m_ecexpressionsCache), 
    m_nodesFactory(other.m_nodesFactory), m_localState(other.m_localState), m_cancelationToken(other.m_cancelationToken)
    {
    Init();

    if (other.IsLocalizationContext())
        SetLocalizationContext(other);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenProviderContext::~RulesDrivenProviderContext()
    {
    if (m_holdsRuleset)
        m_ruleset.Release();

    DELETE_AND_CLEAR(m_schemaHelper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::Init()
    {
    m_isLocalizationContext = false;
    m_localizationProvider = nullptr;
    m_isQueryContext = false;
    m_connections = nullptr;
    m_connection = nullptr;
    m_statementCache = nullptr;
    m_schemaHelper = nullptr;
    m_cancelationToken = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::SetLocalizationContext(ILocalizationProvider const& provider)
    {
    BeAssert(!IsLocalizationContext());
    m_isLocalizationContext = true;
    m_localizationProvider = &provider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::SetLocalizationContext(RulesDrivenProviderContextCR other)
    {
    BeAssert(!IsLocalizationContext());
    m_isLocalizationContext = true;
    m_localizationProvider = other.m_localizationProvider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::SetQueryContext(IConnectionManagerCR connections, IConnectionCR connection, ECSqlStatementCache const& statementCache, CustomFunctionsInjector& customFunctions)
    {
    BeAssert(!IsQueryContext());
    m_isQueryContext = true;
    m_connections = &connections;
    m_connection = &connection;
    m_statementCache = &statementCache;
    m_schemaHelper = new ECSchemaHelper(connection, &m_relatedPathsCache, &m_polymorphicallyRelatedClassesCache, m_statementCache, &m_ecexpressionsCache);
    customFunctions.OnConnection(connection);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::SetQueryContext(RulesDrivenProviderContextCR other)
    {
    BeAssert(!IsQueryContext());
    m_isQueryContext = true;
    m_connections = other.m_connections;
    m_connection = other.m_connection;
    m_statementCache = other.m_statementCache;
    m_schemaHelper = new ECSchemaHelper(*m_connection, &m_relatedPathsCache, &m_polymorphicallyRelatedClassesCache, m_statementCache, &m_ecexpressionsCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IUsedUserSettingsListener& RulesDrivenProviderContext::GetUsedSettingsListener() const
    {
    if (m_usedSettingsListener.IsNull())
        m_usedSettingsListener = new UsedUserSettingsListener();
    return *m_usedSettingsListener;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::SetUsedSettingsListener(RulesDrivenProviderContext const& other)
    {
    m_usedSettingsListener = other.m_usedSettingsListener;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bset<Utf8String> RulesDrivenProviderContext::GetRelatedSettingIds() const
    {
    if (m_usedSettingsListener.IsNull())
        return bset<Utf8String>();
    return m_usedSettingsListener->GetSettingIds();
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct NeverCanceledToken : ICancelationToken
    {
    bool _IsCanceled() const override {return false;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ICancelationTokenCR RulesDrivenProviderContext::GetCancelationToken() const
    {
    if (nullptr != m_cancelationToken)
        return *m_cancelationToken;

    static ICancelationTokenPtr s_neverCanceled = new NeverCanceledToken();
    return *s_neverCanceled;
    }