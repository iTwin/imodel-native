/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include "ContentSpecificationsHandler.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct ContentClassesLocater
{
    struct Context : ContentSpecificationsHandler::Context
    {
    private:
        RulesetVariables const& m_rulesetVariables;
        ECExpressionsCache& m_ecexpressionsCache;
        INavNodeLocater const& m_nodesLocater;
    public:
        Context(ECSchemaHelper const& helper, IConnectionManagerCR connections, IConnectionCR connection,  PresentationRuleSetCR ruleset, Utf8StringCR locale,
            Utf8CP preferredDisplayType, RulesetVariables const& rulesetVariables, ECExpressionsCache& expressionsCache, INavNodeLocater const& nodesLocater)
            : ContentSpecificationsHandler::Context(helper, connections, connection, ruleset, locale, preferredDisplayType), 
            m_rulesetVariables(rulesetVariables), m_nodesLocater(nodesLocater), m_ecexpressionsCache(expressionsCache)
            {}
        RulesetVariables const& GetRulesetVariables() const {return m_rulesetVariables;}
        ECExpressionsCache& GetECExpressionsCache() const {return m_ecexpressionsCache;}
        INavNodeLocater const& GetNodesLocater() const {return m_nodesLocater;}
    };

private:
    Context& m_context;

private:
    bvector<NavNodeKeyCPtr> GetClassKeys(bvector<ECClassCP> const&) const;
    
public:
    ContentClassesLocater(Context& context) : m_context(context) {}
    bvector<SelectClassInfo> Locate(bvector<ECClassCP> const&) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
