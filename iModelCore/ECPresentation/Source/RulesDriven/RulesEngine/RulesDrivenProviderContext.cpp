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
    mutable BeMutex m_mutex;
protected:
    void _OnUserSettingUsed(Utf8CP settingId) override {BeMutexHolder lock(m_mutex); m_settingIds.insert(settingId);}
public:
    bset<Utf8String> const& GetSettingIds() const {BeMutexHolder lock(m_mutex); return m_settingIds;}
};
END_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenProviderContext::RulesDrivenProviderContext(PresentationRuleSetCR ruleset, Utf8String locale, IUserSettings const& settings, ECExpressionsCache& ecexpressionsCache,
    RelatedPathsCache& relatedPathsCache, PolymorphicallyRelatedClassesCache& polymorphicallyRelatedClassesCache, JsonNavNodesFactory const& nodesFactory, IJsonLocalState const* localState)
    : m_ruleset(&ruleset), m_locale(locale), m_userSettings(settings), m_relatedPathsCache(relatedPathsCache),
    m_polymorphicallyRelatedClassesCache(polymorphicallyRelatedClassesCache), m_ecexpressionsCache(ecexpressionsCache), m_nodesFactory(nodesFactory),
    m_localState(localState)
    {
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenProviderContext::RulesDrivenProviderContext(RulesDrivenProviderContextCR other)
    : m_ruleset(other.m_ruleset), m_locale(other.m_locale), m_userSettings(other.m_userSettings), m_relatedPathsCache(other.m_relatedPathsCache),
    m_polymorphicallyRelatedClassesCache(other.m_polymorphicallyRelatedClassesCache), m_ecexpressionsCache(other.m_ecexpressionsCache),
    m_nodesFactory(other.m_nodesFactory), m_localState(other.m_localState), m_cancelationToken(other.m_cancelationToken)
    {
    Init();

    if (other.IsPropertyFormattingContext())
        SetPropertyFormattingContext(other);

    if (other.IsLocalizationContext())
        SetLocalizationContext(other);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenProviderContext::~RulesDrivenProviderContext()
    {
    DELETE_AND_CLEAR(m_schemaHelper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::Init()
    {
    m_propertyFormatter = nullptr;
    m_isLocalizationContext = false;
    m_localizationProvider = nullptr;
    m_isQueryContext = false;
    m_connections = nullptr;
    m_connection = nullptr;
    m_schemaHelper = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::SetPropertyFormattingContext(IECPropertyFormatter const& formatter)
    {
    BeAssert(!IsPropertyFormattingContext());
    m_propertyFormatter = &formatter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::SetPropertyFormattingContext(RulesDrivenProviderContextCR other)
    {
    BeAssert(!IsPropertyFormattingContext());
    m_propertyFormatter = other.m_propertyFormatter;
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
void RulesDrivenProviderContext::SetQueryContext(IConnectionManagerCR connections, IConnectionCR connection)
    {
    BeAssert(!IsQueryContext());
    m_isQueryContext = true;
    m_connections = &connections;
    m_connection = &connection;
    m_schemaHelper = new ECSchemaHelper(connection, &m_relatedPathsCache, &m_polymorphicallyRelatedClassesCache, &m_ecexpressionsCache);
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
    m_schemaHelper = new ECSchemaHelper(*m_connection, &m_relatedPathsCache, &m_polymorphicallyRelatedClassesCache, &m_ecexpressionsCache);
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
void RulesDrivenProviderContext::SetUsedSettingsListener(RulesDrivenProviderContextCR other)
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
    if (m_cancelationToken.IsValid())
        return *m_cancelationToken;

    static ICancelationTokenPtr s_neverCanceled = new NeverCanceledToken();
    return *s_neverCanceled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::Adopt(IConnectionCR connection, ICancelationTokenCP token)
    {
    m_cancelationToken = token;
    if (m_connection != &connection)
        {
        m_connection = &connection;
        DELETE_AND_CLEAR(m_schemaHelper);
        m_schemaHelper = new ECSchemaHelper(connection, &m_relatedPathsCache, &m_polymorphicallyRelatedClassesCache, &m_ecexpressionsCache);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::Adopt(RulesDrivenProviderContextCR other)
    {
    m_cancelationToken = &other.GetCancelationToken();
    if (m_connection != &other.GetConnection())
        {
        m_connection = &other.GetConnection();
        DELETE_AND_CLEAR(m_schemaHelper);
        m_schemaHelper = new ECSchemaHelper(*m_connection, &m_relatedPathsCache, &m_polymorphicallyRelatedClassesCache, &m_ecexpressionsCache);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::AdoptToSameConnection(ICancelationTokenCP token)
    {
    Adopt(*m_connections->GetConnection(m_connection->GetId().c_str()), token);
    }
