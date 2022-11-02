/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RulesetVariables.h>
#include <ECPresentation/Rules/PresentationRules.h>
#include "../ECSchemaHelper.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryBuilderParameters
{
private:
    ECSchemaHelper const& m_schemaHelper;
    IConnectionManagerCR m_connections;
    IConnectionCR m_connection;
    ICancelationTokenCP m_cancellationToken;
    IRulesPreprocessorR m_rulesPreprocessor;
    PresentationRuleSetCP m_ruleset;
    RulesetVariables const& m_rulesetVariables;
    ECExpressionsCache& m_ecexpressionsCache;
    IJsonLocalState const* m_localState;
    IUsedRulesetVariablesListener* m_usedVariablesListener;

public:
    QueryBuilderParameters(ECSchemaHelper const& schemaHelper, IConnectionManagerCR connections, IConnectionCR connection, ICancelationTokenCP cancellationToken,
        IRulesPreprocessorR rulesPreprocessor, PresentationRuleSetCR ruleset, RulesetVariables const&  rulesetVariables, 
        ECExpressionsCache& ecexpressionsCache, IUsedRulesetVariablesListener* variablesListener, IJsonLocalState const* localState = nullptr)
        : m_schemaHelper(schemaHelper), m_connections(connections), m_connection(connection), m_cancellationToken(cancellationToken), m_ruleset(&ruleset), m_rulesPreprocessor(rulesPreprocessor),
        m_rulesetVariables(rulesetVariables), m_ecexpressionsCache(ecexpressionsCache), m_usedVariablesListener(variablesListener), m_localState(localState)
        {}
    void SetRuleset(PresentationRuleSetCR ruleset) {m_ruleset = &ruleset;}
    void SetLocalState(IJsonLocalState const& localState) {m_localState = &localState;}
    ECSchemaHelper const& GetSchemaHelper() const {return m_schemaHelper;}
    IConnectionManagerCR GetConnections() const {return m_connections;}
    IConnectionCR GetConnection() const {return m_connection;}
    ICancelationTokenCP GetCancellationToken() const {return m_cancellationToken;}
    IRulesPreprocessorR GetRulesPreprocessor() const {return m_rulesPreprocessor;}
    PresentationRuleSetCR GetRuleset() const {return *m_ruleset;}
    RulesetVariables const& GetRulesetVariables() const {return m_rulesetVariables;}
    ECExpressionsCache& GetECExpressionsCache() const {return m_ecexpressionsCache;}
    IJsonLocalState const* GetLocalState() const {return m_localState;}
    IUsedRulesetVariablesListener* GetUsedVariablesListener() const { return m_usedVariablesListener; }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
