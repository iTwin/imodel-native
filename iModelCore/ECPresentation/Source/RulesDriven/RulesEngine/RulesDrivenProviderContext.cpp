/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "RulesDrivenProviderContext.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2017
+===============+===============+===============+===============+===============+======*/
struct UsedRulesetVariablesListener : IUsedRulesetVariablesListener
{
private:
    bset<Utf8String> m_variableIds;
    mutable BeMutex m_mutex;
protected:
    void _OnVariableUsed(Utf8CP variableId) override {BeMutexHolder lock(m_mutex); m_variableIds.insert(variableId);}
public:
    bset<Utf8String> const& GetVariableIds() const {BeMutexHolder lock(m_mutex); return m_variableIds;}
};
END_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenProviderContext::RulesDrivenProviderContext(PresentationRuleSetCR ruleset, Utf8String locale, std::unique_ptr<RulesetVariables> rulesetVariables, ECExpressionsCache& ecexpressionsCache,
    RelatedPathsCache& relatedPathsCache, PolymorphicallyRelatedClassesCache& polymorphicallyRelatedClassesCache, JsonNavNodesFactory const& nodesFactory, IJsonLocalState const* localState)
    : m_ruleset(&ruleset), m_locale(locale), m_rulesetVariables(std::move(rulesetVariables)), m_relatedPathsCache(relatedPathsCache),
    m_polymorphicallyRelatedClassesCache(polymorphicallyRelatedClassesCache), m_ecexpressionsCache(ecexpressionsCache), m_nodesFactory(nodesFactory),
    m_localState(localState)
    {
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenProviderContext::RulesDrivenProviderContext(RulesDrivenProviderContextCR other)
    : m_ruleset(other.m_ruleset), m_locale(other.m_locale), m_rulesetVariables(std::make_unique<RulesetVariables>(*other.m_rulesetVariables)), m_relatedPathsCache(other.m_relatedPathsCache),
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
    m_unitSystem = ECPresentation::UnitSystem::Undefined;
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
void RulesDrivenProviderContext::SetPropertyFormattingContext(IECPropertyFormatter const& formatter, ECPresentation::UnitSystem unitSystem)
    {
    BeAssert(!IsPropertyFormattingContext());
    m_propertyFormatter = &formatter;
    m_unitSystem = unitSystem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::SetPropertyFormattingContext(RulesDrivenProviderContextCR other)
    {
    BeAssert(!IsPropertyFormattingContext());
    m_propertyFormatter = other.m_propertyFormatter;
    m_unitSystem = other.m_unitSystem;
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
IUsedRulesetVariablesListener& RulesDrivenProviderContext::GetUsedVariablesListener() const
    {
    if (m_usedVariablesListener.IsNull())
        m_usedVariablesListener = new UsedRulesetVariablesListener();
    return *m_usedVariablesListener;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::SetUsedVariablesListener(RulesDrivenProviderContextCR other)
    {
    m_usedVariablesListener = other.m_usedVariablesListener;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bset<Utf8String> RulesDrivenProviderContext::GetRelatedVariablesIds() const
    {
    if (m_usedVariablesListener.IsNull())
        return bset<Utf8String>();
    return m_usedVariablesListener->GetVariableIds();
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
