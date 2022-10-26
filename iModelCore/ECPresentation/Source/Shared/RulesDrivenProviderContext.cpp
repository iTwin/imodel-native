/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "RulesDrivenProviderContext.h"
#include "RulesPreprocessor.h"
#include "Queries/PresentationQuery.h"
#include "Queries/QueryExecutor.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct UsedRulesetVariablesListener : IUsedRulesetVariablesListener
{
private:
    IUsedRulesetVariablesListener* m_parent;
    bset<Utf8String> m_variableIds;
    mutable BeMutex m_mutex;
protected:
    void _OnVariableUsed(Utf8CP variableId) override
        {
        BeMutexHolder lock(m_mutex);
        m_variableIds.insert(variableId);
        if (m_parent)
            m_parent->OnVariableUsed(variableId);
        }
public:
    UsedRulesetVariablesListener(bset<Utf8String> variableIds = {}) : m_parent(nullptr), m_variableIds(variableIds) {}
    bset<Utf8String> const& GetVariableIds() const {BeMutexHolder lock(m_mutex); return m_variableIds;}
    void SetParent(IUsedRulesetVariablesListener* parent) {m_parent = parent;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodeInstanceKeysProvider : INodeInstanceKeysProvider
{
private:
    RulesDrivenProviderContextCPtr m_context;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    std::unique_ptr<CustomFunctionsContext> CreateCustomFunctionsContext() const
        {
        // set up the custom functions context
        IECPropertyFormatter const* formatter = m_context->IsPropertyFormattingContext() ? &m_context->GetECPropertyFormatter() : nullptr;
        ECPresentation::UnitSystem unitSystem = m_context->IsPropertyFormattingContext() ? m_context->GetUnitSystem() : ECPresentation::UnitSystem::Undefined;
        return std::make_unique<CustomFunctionsContext>(m_context->GetSchemaHelper(), m_context->GetConnections(), m_context->GetConnection(), m_context->GetRuleset().GetRuleSetId(),
            m_context->GetRulesPreprocessor(), m_context->GetRulesetVariables(), &m_context->GetUsedVariablesListener(), m_context->GetECExpressionsCache(),
            m_context->GetNodesFactory(), nullptr, nullptr, nullptr, formatter, unitSystem);
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _IterateInstanceKeys(NavNodeKeyCR nodeKey, std::function<bool(ECInstanceKey)> cb) const override
        {
        auto scope = Diagnostics::Scope::Create("Iterate node instance keys");

        if (nodeKey.GetInstanceKeysSelectQuery() == nullptr)
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_DEBUG, "Node has no instance keys query assigned - nothing to iterate over.");
            return;
            }

        auto supportCustomFunctions = CreateCustomFunctionsContext();
        CachedECSqlStatementPtr statement = m_context->GetConnection().GetStatementCache().GetPreparedStatement(m_context->GetConnection().GetECDb().Schemas(),
            m_context->GetConnection().GetDb(), nodeKey.GetInstanceKeysSelectQuery()->GetQueryString().c_str());
        if (statement.IsNull())
            {
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to prepare instance keys query. Error: '%s'. Query: %s",
                m_context->GetConnection().GetDb().GetLastError().c_str(), nodeKey.GetInstanceKeysSelectQuery()->GetQueryString().c_str()));
            }

        if (SUCCESS != nodeKey.GetInstanceKeysSelectQuery()->BindValues(*statement))
            {
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to bind values for instance keys query. Error: '%s'. Query: %s",
                m_context->GetConnection().GetDb().GetLastError().c_str(), nodeKey.GetInstanceKeysSelectQuery()->GetQueryString().c_str()));
            }

        while (BE_SQLITE_ROW == QueryExecutorHelper::Step(*statement))
            {
            bool res = cb(ECInstanceKey(statement->GetValueId<ECClassId>(0), statement->GetValueId<ECInstanceId>(1)));
            if (!res)
                return;
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _ContainsInstanceKey(NavNodeCR node, ECInstanceKeyCR instanceKey) const override
        {
        auto scope = Diagnostics::Scope::Create("Checking if node contains given instance key");

        if (node.GetKey()->GetInstanceKeysSelectQuery() == nullptr)
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_DEBUG, "Node has no instance keys query assigned - nothing to iterate over.");
            return false;
            }

        auto query = ComplexQueryBuilder::Create();
        query->SelectAll();
        query->From(*node.GetKey()->GetInstanceKeysSelectQuery(), "keys");
        query->Where("[keys].[ECClassId] = ? AND [keys].[ECInstanceId] = ?", { std::make_shared<BoundQueryId>(instanceKey.GetClassId()), std::make_shared<BoundQueryId>(instanceKey.GetInstanceId()) });

        auto supportCustomFunctions = CreateCustomFunctionsContext();
        CachedECSqlStatementPtr statement = m_context->GetConnection().GetStatementCache().GetPreparedStatement(m_context->GetConnection().GetECDb().Schemas(),
            m_context->GetConnection().GetDb(), query->GetQuery()->GetQueryString().c_str());
        if (statement.IsNull())
            {
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to prepare instance keys check query. Error: '%s'. Query: %s",
                m_context->GetConnection().GetDb().GetLastError().c_str(), query->GetQuery()->GetQueryString().c_str()));
            }

        if (SUCCESS != query->GetQuery()->BindValues(*statement))
            {
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to bind values for instance keys query. Error: '%s'. Query: %s",
                m_context->GetConnection().GetDb().GetLastError().c_str(), query->GetQuery()->GetQueryString().c_str()));
            }

        return BE_SQLITE_ROW == QueryExecutorHelper::Step(*statement);
        }

public:
    NodeInstanceKeysProvider(RulesDrivenProviderContextCR context) : m_context(&context) {}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenProviderContext::RulesDrivenProviderContext(PresentationRuleSetCR ruleset, std::unique_ptr<RulesetVariables> rulesetVariables, ECExpressionsCache& ecexpressionsCache,
    RelatedPathsCache& relatedPathsCache, NavNodesFactory const& nodesFactory, IJsonLocalState const* localState)
    : m_ruleset(&ruleset), m_rulesetVariables(std::move(rulesetVariables)), m_relatedPathsCache(relatedPathsCache),
    m_ecexpressionsCache(ecexpressionsCache), m_nodesFactory(nodesFactory),
    m_localState(localState)
    {
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenProviderContext::RulesDrivenProviderContext(RulesDrivenProviderContextCR other)
    : m_ruleset(other.m_ruleset), m_rulesetVariables(std::make_unique<RulesetVariables>(*other.m_rulesetVariables)), m_relatedPathsCache(other.m_relatedPathsCache),
    m_ecexpressionsCache(other.m_ecexpressionsCache), m_nodesFactory(other.m_nodesFactory), m_localState(other.m_localState), m_cancelationToken(other.m_cancelationToken),
    m_usedVariablesListener(other.m_usedVariablesListener)
    {
    Init();

    if (other.IsPropertyFormattingContext())
        SetPropertyFormattingContext(other);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenProviderContext::~RulesDrivenProviderContext()
    {
    DELETE_AND_CLEAR(m_schemaHelper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::Init()
    {
    m_propertyFormatter = nullptr;
    m_unitSystem = ECPresentation::UnitSystem::Undefined;
    m_isQueryContext = false;
    m_connections = nullptr;
    m_connection = nullptr;
    m_schemaHelper = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::SetPropertyFormattingContext(IECPropertyFormatter const& formatter, ECPresentation::UnitSystem unitSystem)
    {
    m_propertyFormatter = &formatter;
    m_unitSystem = unitSystem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::SetPropertyFormattingContext(RulesDrivenProviderContextCR other)
    {
    m_propertyFormatter = other.m_propertyFormatter;
    m_unitSystem = other.m_unitSystem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::SetQueryContext(IConnectionManagerCR connections, IConnectionCR connection)
    {
    m_isQueryContext = true;
    m_connections = &connections;
    m_connection = &connection;
    m_schemaHelper = new ECSchemaHelper(connection, &m_relatedPathsCache, &m_ecexpressionsCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::SetQueryContext(RulesDrivenProviderContextCR other)
    {
    m_isQueryContext = true;
    m_connections = other.m_connections;
    m_connection = other.m_connection;
    m_schemaHelper = new ECSchemaHelper(*m_connection, &m_relatedPathsCache, &m_ecexpressionsCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IUsedRulesetVariablesListener& RulesDrivenProviderContext::GetUsedVariablesListener() const
    {
    if (m_usedVariablesListener.IsNull())
        m_usedVariablesListener = new UsedRulesetVariablesListener();
    return *m_usedVariablesListener;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::InitUsedVariablesListener(bset<Utf8String> const& parentVariables, IUsedRulesetVariablesListener* parentListener)
    {
    m_usedVariablesListener = new UsedRulesetVariablesListener(parentVariables);
    m_usedVariablesListener->SetParent(parentListener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bset<Utf8String> const& RulesDrivenProviderContext::GetRelatedVariablesIds() const
    {
    if (m_usedVariablesListener.IsNull())
        {
        static bset<Utf8String> const s_empty;
        return s_empty;
        }
    return m_usedVariablesListener->GetVariableIds();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IRulesPreprocessorR RulesDrivenProviderContext::GetRulesPreprocessor() const
    {
    if (nullptr == m_rulesPreprocessor)
        {
        m_rulesPreprocessor = std::make_unique<RulesPreprocessor>(GetConnections(), GetConnection(), GetRuleset(),
            GetRulesetVariables(), &GetUsedVariablesListener(), GetECExpressionsCache());
        }
    return *m_rulesPreprocessor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ICancelationTokenCR RulesDrivenProviderContext::GetCancelationToken() const
    {
    if (m_cancelationToken.IsValid())
        return *m_cancelationToken;

    static ICancelationTokenPtr s_neverCanceled = new NeverCanceledToken();
    return *s_neverCanceled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::_ShallowAdopt(IConnectionCR connection, ICancelationTokenCP token)
    {
    m_cancelationToken = token;
    if (m_connection != &connection)
        {
        m_connection = &connection;
        DELETE_AND_CLEAR(m_schemaHelper);
        m_schemaHelper = new ECSchemaHelper(connection, &m_relatedPathsCache, &m_ecexpressionsCache);
        m_rulesPreprocessor = nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::ShallowAdopt(RulesDrivenProviderContextCR other)
    {
    ShallowAdopt(other.GetConnection(), &other.GetCancelationToken());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenProviderContext::ShallowAdoptToSameConnection(ICancelationTokenCP token)
    {
    ShallowAdopt(*m_connections->GetConnection(m_connection->GetId().c_str()), token);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<INodeInstanceKeysProvider> RulesDrivenProviderContext::CreateNodeInstanceKeysProvider() const
    {
    return std::make_unique<NodeInstanceKeysProvider>(*this);
    }
