/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/Rules/PresentationRules.h>
#include <Bentley/Logging.h>
#include "../RulesEngineTypes.h"
#include "Queries/CustomFunctions.h"
#include "ECSchemaHelper.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct UsedRulesetVariablesListener;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct INodeInstanceKeysProvider
{
protected:
    virtual void _IterateInstanceKeys(NavNodeCR, std::function<bool(ECInstanceKey)>) const = 0;
    virtual bool _ContainsInstanceKey(NavNodeCR, ECInstanceKeyCR) const = 0;
public:
    virtual ~INodeInstanceKeysProvider() {}
    void IterateInstanceKeys(NavNodeCR node, std::function<bool(ECInstanceKey)> cb) const { _IterateInstanceKeys(node, cb); }
    bool ContainsInstanceKey(NavNodeCR node, ECInstanceKeyCR instanceKey) const { return _ContainsInstanceKey(node, instanceKey); }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenProviderContext : RefCountedBase
{
private:
    // common
    PresentationRuleSetCPtr m_ruleset;
    IJsonLocalState const* m_localState;
    std::unique_ptr<RulesetVariables> m_rulesetVariables;
    mutable RefCountedPtr<UsedRulesetVariablesListener> m_usedVariablesListener;
    RelatedPathsCache& m_relatedPathsCache;
    ECExpressionsCache& m_ecexpressionsCache;
    NavNodesFactory const& m_nodesFactory;
    mutable std::unique_ptr<IRulesPreprocessor> m_rulesPreprocessor;
    ICancelationTokenCPtr m_cancelationToken;

    // Property formatting context
    IECPropertyFormatter const* m_propertyFormatter;
    ECPresentation::UnitSystem m_unitSystem;

    // ECDb context
    bool m_isQueryContext;
    IConnectionManagerCP m_connections;
    IConnectionCP m_connection;
    ECSchemaHelper* m_schemaHelper;

private:
    void Init();

protected:
    ECPRESENTATION_EXPORT RulesDrivenProviderContext(PresentationRuleSetCR, std::unique_ptr<RulesetVariables>, ECExpressionsCache&,
        RelatedPathsCache&, NavNodesFactory const&, IJsonLocalState const*);
    ECPRESENTATION_EXPORT RulesDrivenProviderContext(RulesDrivenProviderContextCR other);

    ECPRESENTATION_EXPORT void SetQueryContext(IConnectionManagerCR, IConnectionCR);
    ECPRESENTATION_EXPORT void SetQueryContext(RulesDrivenProviderContextCR other);

    ECPRESENTATION_EXPORT virtual void _ShallowAdopt(IConnectionCR, ICancelationTokenCP);

public:
    ECPRESENTATION_EXPORT ~RulesDrivenProviderContext();

    // common
    PresentationRuleSetCR GetRuleset() const {return *m_ruleset;}
    NavNodesFactory const& GetNodesFactory() const {return m_nodesFactory;}
    RulesetVariables const& GetRulesetVariables() const {return *m_rulesetVariables;}
    ECPRESENTATION_EXPORT IUsedRulesetVariablesListener& GetUsedVariablesListener() const;
    ECPRESENTATION_EXPORT void InitUsedVariablesListener(bset<Utf8String> const& parentVariables, IUsedRulesetVariablesListener* parentListener);
    bset<Utf8String> const& GetRelatedVariablesIds() const;
    ECExpressionsCache& GetECExpressionsCache() const {return m_ecexpressionsCache;}
    IJsonLocalState const* GetLocalState() const {return m_localState;}
    ECPRESENTATION_EXPORT IRulesPreprocessorR GetRulesPreprocessor() const;
    ECPRESENTATION_EXPORT ICancelationTokenCR GetCancelationToken() const;
    void SetCancelationToken(ICancelationTokenCP token) {m_cancelationToken = token;}
    void ShallowAdopt(IConnectionCR connection, ICancelationTokenCP cancelationToken) {_ShallowAdopt(connection, cancelationToken);}
    ECPRESENTATION_EXPORT void ShallowAdopt(RulesDrivenProviderContextCR);
    ECPRESENTATION_EXPORT void ShallowAdoptToSameConnection(ICancelationTokenCP);

    // Property formatting context
    ECPRESENTATION_EXPORT void SetPropertyFormattingContext(IECPropertyFormatter const&, ECPresentation::UnitSystem);
    ECPRESENTATION_EXPORT void SetPropertyFormattingContext(RulesDrivenProviderContextCR);
    bool IsPropertyFormattingContext() const {return nullptr != m_propertyFormatter;}
    IECPropertyFormatter const& GetECPropertyFormatter() const {return *m_propertyFormatter;}
    ECPresentation::UnitSystem GetUnitSystem() const {return m_unitSystem;}

    // ECDb context
    bool IsQueryContext() const {return m_isQueryContext;}
    IConnectionManagerCR GetConnections() const {return *m_connections;}
    IConnectionCR GetConnection() const {return *m_connection;}
    ECSchemaHelper const& GetSchemaHelper() const {return *m_schemaHelper;}

    // node instance keys
    std::unique_ptr<INodeInstanceKeysProvider> CreateNodeInstanceKeysProvider() const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
