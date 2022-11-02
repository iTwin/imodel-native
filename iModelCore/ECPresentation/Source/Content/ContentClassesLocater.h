/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/IRulesPreprocessor.h>
#include "ContentSpecificationsHandler.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentClassesLocater
{
    struct Context : ContentSpecificationsHandler::Context
    {
    private:
        INavNodeLocater const& m_nodesLocater;
        NavNodesFactory const& m_navNodeFactory;
    public:
        Context(ECSchemaHelper const& helper, IConnectionManagerCR connections, IConnectionCR connection, ICancelationTokenCP cancellationToken, 
            IRulesPreprocessorR rulesPreprocessor, PresentationRuleSetCR ruleset,
            Utf8CP preferredDisplayType, RulesetVariables const& rulesetVariables, INavNodeLocater const& nodesLocater, NavNodesFactory const& navNodeFactory)
            : ContentSpecificationsHandler::Context(helper, connections, connection, cancellationToken, rulesPreprocessor, ruleset, rulesetVariables, preferredDisplayType, *NavNodeKeyListContainer::Create()),
            m_nodesLocater(nodesLocater), m_navNodeFactory(navNodeFactory)
            {}
        INavNodeLocater const& GetNodesLocater() const {return m_nodesLocater;}
        NavNodesFactory const& GetNavNodeFactory() const {return m_navNodeFactory;}
    };

private:
    Context& m_context;

private:
    bvector<NavNodeKeyCPtr> GetClassKeys(IRulesPreprocessor&, bvector<ECClassCP> const&) const;

public:
    ContentClassesLocater(Context& context) : m_context(context) {}
    bvector<SelectClassInfo> Locate(bvector<ECClassCP> const&) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
