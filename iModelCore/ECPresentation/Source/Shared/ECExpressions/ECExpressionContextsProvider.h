/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECObjects/ECExpressions.h>
#include <ECPresentation/NavNode.h>
#include <ECPresentation/RulesetVariables.h>
#include <ECDb/ECDbTypes.h>
#include "../Queries/PresentationQuery.h"
#include "ECExpressionOptimizer.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECExpressionContextsProvider : NonCopyableClass
{
    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct ContextParametersBase
    {
    private:
        IConnectionCR m_connection;
        RulesetVariables const& m_rulesetVariables;
        IUsedRulesetVariablesListener* m_usedVariablesListener;
    public:
        ContextParametersBase(IConnectionCR connection, RulesetVariables const& rulesetVariables, IUsedRulesetVariablesListener* usedVariablesListener)
            : m_connection(connection), m_rulesetVariables(rulesetVariables), m_usedVariablesListener(usedVariablesListener)
            {}
        IConnectionCR GetConnection() const {return m_connection;}
        RulesetVariables const& GetRulesetVariables() const {return m_rulesetVariables;}
        IUsedRulesetVariablesListener* GetUsedVariablesListener() const {return m_usedVariablesListener;}
    };

    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct NodeRulesContextParameters : ContextParametersBase
    {
    private:
        NavNodeCP m_parentNode;
    public:
        NodeRulesContextParameters(NavNodeCP parentNode, IConnectionCR connection,
            RulesetVariables const& rulesetVariables, IUsedRulesetVariablesListener* usedVariablesListener)
            : ContextParametersBase(connection, rulesetVariables, usedVariablesListener), m_parentNode(parentNode)
            {}
        NavNodeCP GetParentNode() const {return m_parentNode;}
    };

    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct ContentRulesContextParameters : ContextParametersBase
    {
    private:
        INavNodeLocaterCP m_nodeLocater;
        Utf8String m_rulesetId;
        Utf8String m_contentDisplayType;
        Utf8String m_selectionProviderName;
        bool m_isSubSelection;
        NavNodeKeyCP m_selectedNodeKey;
        INodeLabelCalculator const& m_nodeLabelCalculator;
    public:
        ContentRulesContextParameters(Utf8CP contentDisplayType, Utf8CP selectionProviderName, bool isSubSelection, IConnectionCR connection, Utf8String rulesetId,
            INavNodeLocaterCP nodeLocater, NavNodeKeyCP selectedNodeKey, RulesetVariables const& rulesetVariables, IUsedRulesetVariablesListener* usedVariablesListener, INodeLabelCalculator const& nodeLabelCalculator)
            : ContextParametersBase(connection, rulesetVariables, usedVariablesListener), m_rulesetId(rulesetId), m_contentDisplayType(contentDisplayType), m_selectionProviderName(selectionProviderName),
            m_isSubSelection(isSubSelection), m_nodeLocater(nodeLocater), m_selectedNodeKey(selectedNodeKey), m_nodeLabelCalculator(nodeLabelCalculator)
            {}
        Utf8StringCR GetRulesetId() const {return m_rulesetId;}
        Utf8StringCR GetContentDisplayType() const {return m_contentDisplayType;}
        Utf8StringCR GetSelectionProviderName() const {return m_selectionProviderName;}
        bool IsSubSelection() const {return m_isSubSelection;}
        INavNodeLocaterCP GetNodeLocater() const {return m_nodeLocater;}
        NavNodeKeyCP GetSelectedNodeKey() const {return m_selectedNodeKey;}
        INodeLabelCalculator const& GetNodeLabelCalculator() const {return m_nodeLabelCalculator;}
    };

    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct ContentSpecificationInstanceFilterContextParameters : ContextParametersBase
    {
    private:
        INavNodeKeysContainerCPtr m_input;
    public:
        ContentSpecificationInstanceFilterContextParameters(IConnectionCR connection, INavNodeKeysContainerCR input,
            RulesetVariables const& rulesetVariables, IUsedRulesetVariablesListener* usedVariablesListener)
            : ContextParametersBase(connection, rulesetVariables, usedVariablesListener), m_input(&input)
            {}
        INavNodeKeysContainerCR GetInput() const {return *m_input;}
    };

    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct CustomizationRulesContextParameters : ContextParametersBase
    {
    private:
        NavNodeCR m_node;
        NavNodeCPtr m_parentNode;
    public:
        CustomizationRulesContextParameters(NavNodeCR node, NavNodeCP parentNode, IConnectionCR connection,
            RulesetVariables const& rulesetVariables, IUsedRulesetVariablesListener* usedVariablesListener)
            : ContextParametersBase(connection, rulesetVariables, usedVariablesListener), m_node(node), m_parentNode(parentNode)
            {}
        NavNodeCR GetNode() const {return m_node;}
        NavNodeCP GetParentNode() const {return m_parentNode.get();}
    };

    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct CalculatedPropertyContextParameters : ContextParametersBase
    {
    private:
        NavNodeCR m_node;
    public:
        CalculatedPropertyContextParameters(NavNodeCR node, IConnectionCR connection,
            RulesetVariables const& rulesetVariables, IUsedRulesetVariablesListener* usedVariablesListener)
            : ContextParametersBase(connection, rulesetVariables, usedVariablesListener), m_node(node)
            {}
        NavNodeCR GetNode() const {return m_node;}
    };

private:
    ECExpressionContextsProvider() {};

public:
    ECPRESENTATION_EXPORT static ExpressionContextPtr GetNodeRulesContext(NodeRulesContextParameters const&);
    ECPRESENTATION_EXPORT static ExpressionContextPtr GetContentRulesContext(ContentRulesContextParameters const&);
    ECPRESENTATION_EXPORT static ExpressionContextPtr GetContentSpecificationInstanceFilterContext(ContentSpecificationInstanceFilterContextParameters const&);
    ECPRESENTATION_EXPORT static ExpressionContextPtr GetCustomizationRulesContext(CustomizationRulesContextParameters const&);
    ECPRESENTATION_EXPORT static ExpressionContextPtr GetCalculatedPropertyContext(CalculatedPropertyContextParameters const&);
    ECPRESENTATION_EXPORT static ExpressionContextPtr GetRulesEngineRootContext(ContextParametersBase const&);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECExpressionsCache
{
private:
    bmap<Utf8String, NodePtr> m_cache;
    bmap<Utf8String, OptimizedExpressionPtr> m_optimizedCache;
    bmap<Utf8String, bvector<Utf8String>> m_usedClasses;
    mutable BeMutex m_mutex;

public:
    ECExpressionsCache() {}
    ECPRESENTATION_EXPORT BentleyStatus Get(NodePtr&, Utf8CP expression) const;
    ECPRESENTATION_EXPORT BentleyStatus Get(OptimizedExpressionPtr&, Utf8CP expression) const;
    bvector<Utf8String> const* GetUsedClasses(Utf8CP expression) const;
    bool HasOptimizedExpression(Utf8CP expression) const;
    void Add(Utf8CP expression, NodePtr);
    void Add(Utf8CP expression, OptimizedExpressionPtr);
    bvector<Utf8String> const& Add(Utf8CP expression, bvector<Utf8String>&);
    void Clear();
    BeMutex& GetMutex() const {return m_mutex;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IECExpressionsCacheProvider
{
protected:
    virtual ECExpressionsCache& _Get(Utf8CP rulesetId) = 0;
public:
    virtual ~IECExpressionsCacheProvider() {}
    ECExpressionsCache& Get(Utf8CP rulesetId) {return _Get(rulesetId);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECExpressionsHelper : NonCopyableClass
{
private:
    ECExpressionsCache& m_cache;
public:
    ECExpressionsHelper(ECExpressionsCache& cache) : m_cache(cache) {}
    bool EvaluateECExpression(ECValueR result, Utf8StringCR expression, ExpressionContextR context);
    ECPRESENTATION_EXPORT NodePtr GetNodeFromExpression(Utf8CP expression);
    ECPRESENTATION_EXPORT QueryClauseAndBindings ConvertToECSql(Utf8StringCR expression, IPresentationQueryFieldTypesProvider const*, ExpressionContext*);
    ECPRESENTATION_EXPORT bvector<Utf8String> GetUsedClasses(Utf8StringCR expression);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECExpressionOptimizer
{
private:
    ECExpressionsCache& m_expressionsCache;
public:
    ECExpressionOptimizer(ECExpressionsCache& cache) : m_expressionsCache(cache)
        {}
    ECPRESENTATION_EXPORT OptimizedExpressionPtr GetOptimizedExpression(Utf8CP expression);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
