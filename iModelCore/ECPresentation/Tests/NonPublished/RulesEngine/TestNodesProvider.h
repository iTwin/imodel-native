/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include "../../../Source/RulesDriven/RulesEngine/NavNodeProviders.h"

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2015
+===============+===============+===============+===============+===============+======*/
struct TestNodesProvider : NavNodesProvider
{
    typedef std::function<bool(JsonNavNodePtr&, size_t)> GetNodeHandler;
    typedef std::function<bool()> HasNodesHandler;
    typedef std::function<size_t()> GetNodesCountHandler;

private:
    GetNodeHandler m_getNodeHandler;
    HasNodesHandler m_hasNodesHandler;
    GetNodesCountHandler m_getNodesCountHandler;

private:
    TestNodesProvider(NavNodesProviderContext const& context) : NavNodesProvider(context) {}

protected:
    bool _IsCacheable() const override {return false;}
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override {return m_getNodeHandler ? m_getNodeHandler(node, index) : false;}
    bool _HasNodes() const override {return m_hasNodesHandler ? m_hasNodesHandler() : false;}
    size_t _GetNodesCount() const override {return m_getNodesCountHandler ? m_getNodesCountHandler() : 0;}

public:
    static RefCountedPtr<TestNodesProvider> Create(NavNodesProviderContext const& context) {return new TestNodesProvider(context);}
    void SetGetNodeHandler(GetNodeHandler const& handler) {m_getNodeHandler = handler;}
    void SetHasNodesHandler(HasNodesHandler const& handler) {m_hasNodesHandler = handler;}
    void SetGetNodesCountHandler(GetNodesCountHandler const& handler) {m_getNodesCountHandler = handler;}
};