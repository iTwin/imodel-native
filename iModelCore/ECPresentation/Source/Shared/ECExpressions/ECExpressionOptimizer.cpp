/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentation.h>
#include <ECObjects/ECExpressions.h>
#include <ECObjects/ECExpressionNode.h>
#include "ECExpressionContextsProvider.h"

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+--*/
bool LogicalOptimizedExpression::_Value(OptimizedExpressionsParameters const& params)
    {
    if (m_useAndOperator)
        return m_left->Value(params) && m_right->Value(params);

    return m_left->Value(params) || m_right->Value(params);
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+--*/
bool LogicalOptimizedExpression::_IsEqual(OptimizedExpression const& other) const
    {
    LogicalOptimizedExpression const* otherExp = other.AsLogicalOptimizedExpression();
    if (nullptr == otherExp)
        return false;
    return m_right->IsEqual(*otherExp->m_right) && m_left->IsEqual(*otherExp->m_left) && m_useAndOperator == otherExp->m_useAndOperator;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+--*/
bool DisplayTypeOptimizedExpression::_Value(OptimizedExpressionsParameters const& params)
    {
    if (m_expectEqual)
        return params.GetContentDisplayType() == m_preferredDisplayType;

    return params.GetContentDisplayType() != m_preferredDisplayType;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+--*/
bool DisplayTypeOptimizedExpression::_IsEqual(OptimizedExpression const& other) const
    {
    DisplayTypeOptimizedExpression const* otherExp = other.AsDisplayTypeOptimizedExpression();
    if (nullptr == otherExp)
        return false;
    return m_preferredDisplayType == otherExp->m_preferredDisplayType && m_expectEqual == otherExp->m_expectEqual;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+--*/
static bset<ECClassId> GetClassIds(NavNodeKeyCR key)
    {
    bset<ECClassId> ids;
    if (key.AsECInstanceNodeKey())
        {
        std::transform(key.AsECInstanceNodeKey()->GetInstanceKeys().begin(), key.AsECInstanceNodeKey()->GetInstanceKeys().end(),
            std::inserter(ids, ids.end()), [](ECClassInstanceKeyCR k){return k.GetClass()->GetId();});
        }
    else if (key.AsECClassGroupingNodeKey())
        ids.insert(key.AsECClassGroupingNodeKey()->GetECClassId());
    else if (key.AsECPropertyGroupingNodeKey())
        ids.insert(key.AsECPropertyGroupingNodeKey()->GetECClassId());
    else if (key.AsLabelGroupingNodeKey() && key.AsLabelGroupingNodeKey()->GetGroupedInstanceKeys() != nullptr)
        {
        for (auto const& instanceKey : *key.AsLabelGroupingNodeKey()->GetGroupedInstanceKeys())
            ids.insert(instanceKey.GetClassId());
        }
    return ids;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+--*/
IsOfClassOptimizedExpression::~IsOfClassOptimizedExpression()
    {
    if (nullptr != m_connections)
        m_connections->DropListener(*this);
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+--*/
bool IsOfClassOptimizedExpression::_Value(OptimizedExpressionsParameters const& params)
    {
    if (params.GetInputNodeKey().IsNull())
        return false;

    BeMutexHolder lock(m_cacheMutex);
    auto cacheIter = m_cache.find(&params.GetConnection().GetECDb());
    if (m_cache.end() == cacheIter)
        {
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::ECExpressions, (nullptr == m_connections || m_connections == &params.GetConnections()), "Optimized expression used with invalid connection manager");
        m_connections = &params.GetConnections();
        params.GetConnections().AddListener(*this);
        ECClassCP expectedClass = params.GetConnection().GetECDb().Schemas().GetClass(m_schemaName, m_className);
        cacheIter = m_cache.Insert(&params.GetConnection().GetECDb(), Cache(expectedClass)).first;
        }
    if (nullptr == cacheIter->second.m_expectedClass)
        return false;

    NavNodeKeyCPtr nodeKey = params.GetInputNodeKey();
    bset<ECClassId> lookupClassIds = GetClassIds(*nodeKey);
    if (lookupClassIds.empty())
        return false;

    for (ECClassId lookupClassId : lookupClassIds)
        {
        bmap<ECClassId, bool>& resultsCache = cacheIter->second.m_results;
        auto iter = resultsCache.find(lookupClassId);
        if (resultsCache.end() != iter && !iter->second)
            return false;

        ECClassCP selectedClass = params.GetConnection().GetECDb().Schemas().GetClass(lookupClassId);
        if (nullptr == selectedClass)
            return false;

        bool result = selectedClass->Is(cacheIter->second.m_expectedClass);
        resultsCache[lookupClassId] = result;
        if (!result)
            return false;
        }
    return true;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+--*/
void IsOfClassOptimizedExpression::_OnConnectionEvent(ConnectionEvent const& evt)
    {
    BeMutexHolder lock(m_cacheMutex);
    m_cache.erase(&evt.GetConnection().GetECDb());
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+--*/
bool IsOfClassOptimizedExpression::_IsEqual(OptimizedExpression const& other) const
    {
    IsOfClassOptimizedExpression const* otherExp = other.AsIsOfClassOptimizedExpression();
    if (nullptr == otherExp)
        return false;
    return m_schemaName == otherExp->m_schemaName && m_className == otherExp->m_className;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+--*/
ClassNameOptimizedExpression::~ClassNameOptimizedExpression()
    {
    if (nullptr != m_connections)
        m_connections->DropListener(*this);
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+--*/
bool ClassNameOptimizedExpression::_Value(OptimizedExpressionsParameters const& params)
    {
    if (params.GetInputNodeKey().IsNull())
        return false;

    BeMutexHolder lock(m_cacheMutex);
    auto cacheIter = m_resultsCache.find(&params.GetConnection().GetECDb());
    if (m_resultsCache.end() == cacheIter)
        {
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::ECExpressions, (nullptr == m_connections || m_connections == &params.GetConnections()), "Optimized expression used with invalid connection manager");
        m_connections = &params.GetConnections();
        params.GetConnections().AddListener(*this);
        cacheIter = m_resultsCache.Insert(&params.GetConnection().GetECDb(), bmap<ECClassId, bool>()).first;
        }

    NavNodeKeyCPtr nodeKey = params.GetInputNodeKey();
    bset<ECClassId> lookupClassIds = GetClassIds(*nodeKey);
    if (lookupClassIds.empty())
        return false;

    for (ECClassId lookupClassId : lookupClassIds)
        {
        auto iter = cacheIter->second.find(lookupClassId);
        if (cacheIter->second.end() != iter && !iter->second)
            return false;

        ECClassCP selectedClass = params.GetConnection().GetECDb().Schemas().GetClass(lookupClassId);
        if (nullptr == selectedClass)
            return false;

        bool result = (selectedClass->GetName() == m_className);
        cacheIter->second[lookupClassId] = result;
        if (!result)
            return false;
        }
    return true;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+--*/
void ClassNameOptimizedExpression::_OnConnectionEvent(ConnectionEvent const& evt)
    {
    BeMutexHolder lock(m_cacheMutex);
    m_resultsCache.erase(&evt.GetConnection().GetECDb());
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+--*/
bool ClassNameOptimizedExpression::_IsEqual(OptimizedExpression const& other) const
    {
    ClassNameOptimizedExpression const* otherExp = other.AsClassNameOptimizedExpression();
    if (nullptr == otherExp)
        return false;
    return m_className == otherExp->m_className;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+--*/
bool InstanceIdOptimizedExpression::_Value(OptimizedExpressionsParameters const& params)
    {
    if (params.GetInputNodeKey().IsNull() || nullptr == params.GetInputNodeKey()->AsECInstanceNodeKey())
        return false;

    ECInstancesNodeKey const& nodeKey = *params.GetInputNodeKey()->AsECInstanceNodeKey();
    return nodeKey.GetInstanceKeys().end() != std::find_if(nodeKey.GetInstanceKeys().begin(), nodeKey.GetInstanceKeys().end(),
        [&](ECClassInstanceKeyCR k){return k.GetId() == m_instanceId;});
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+--*/
bool InstanceIdOptimizedExpression::_IsEqual(OptimizedExpression const& other) const
    {
    InstanceIdOptimizedExpression const* otherExp = other.AsInstanceIdOptimizedExpression();
    if (nullptr == otherExp)
        return false;
    return m_instanceId == otherExp->m_instanceId;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECExpressionToOptimizedExpressionConverter : NodeVisitor
{
    enum class ConverterState
        {
        NotParsing,
        ParsingDisplayType,
        ParsingNode,
        ParsingIsOfClass,
        ParsingIsInstanceNode,
        ParsingIsPropertyGroupingNode,
        ParsingIsECClassGroupingNode,
        ParsingClassName,
        ParsingInstanceId,
        };

private:
    RefCountedPtr<OptimizedExpression> m_expression;
    NodeCP m_lastLogicalNode;
    NodeCR m_startNode;
    Utf8String m_preferredDisplayType;
    Utf8String m_schemaName;
    Utf8String m_className;
    BeInt64Id m_instanceId;
    ConverterState m_state;
    bool m_equalOperator;
    bool m_andOperator;
    bool m_ignoreOpenParens;
    bool m_ignoreArguments;
    int m_openedParens;
    BeMutex& m_cacheMutex;
private:
    bool HandleIdent(IdentNodeCR node)
        {
        if (0 == strcmp("ContentDisplayType", node.GetName()))
            {
            m_state = ConverterState::ParsingDisplayType;
            return true;
            }
        if (0 == strcmp("SelectedNode", node.GetName()))
            {
            m_state = ConverterState::ParsingNode;
            return true;
            }
        if (0 == strcmp("ParentNode", node.GetName()))
            {
            m_state = ConverterState::ParsingNode;
            return true;
            }
        if (0 == strcmp("ThisNode", node.GetName()))
            {
            m_state = ConverterState::ParsingNode;
            return true;
            }

        return false;
        }

    bool HandleEqualityOperator(bool equalOperator)
        {
        if (ConverterState::ParsingDisplayType != m_state && ConverterState::ParsingClassName != m_state && ConverterState::ParsingInstanceId != m_state)
            return false;

        m_equalOperator = equalOperator;
        return true;
        }

    bool HandleDotNode(DotNodeCR node)
        {
        if (ConverterState::ParsingNode != m_state)
            return false;

        if (0 == strcmp("IsInstanceNode", node.GetName()))
            {
            m_state = ConverterState::ParsingIsInstanceNode;
            return true;
            }
        if (0 == strcmp("IsPropertyGroupingNode", node.GetName()))
            {
            m_state = ConverterState::ParsingIsPropertyGroupingNode;
            return true;
            }
        if (0 == strcmp("IsClassGroupingNode", node.GetName()))
            {
            m_state = ConverterState::ParsingIsECClassGroupingNode;
            return true;
            }
        if (0 == strcmp("ClassName", node.GetName()))
            {
            m_state = ConverterState::ParsingClassName;
            return true;
            }
        if (0 == strcmp("InstanceId", node.GetName()))
            {
            m_state = ConverterState::ParsingInstanceId;
            return true;
            }
        return false;
        }

    bool HandleIsOfClassArguments(CallNodeCR node)
        {
        ArgumentTreeNodeCP args = node.GetArguments();
        if (nullptr == args || 2 != args->GetArgumentCount())
            return false;

        m_className = args->GetArgument(0)->ToString().Trim("\"");
        m_schemaName = args->GetArgument(1)->ToString().Trim("\"");
        return true;
        }

    bool HandleMethod(CallNodeCR node)
        {
        if (ConverterState::ParsingNode != m_state)
            return false;

        if (0 == strcmp("IsOfClass", node.GetMethodName()))
            {
            m_state = ConverterState::ParsingIsOfClass;
            return HandleIsOfClassArguments(node);
            }

        return false;
        }

    bool HandleString(Utf8CP string)
        {
        if (ConverterState::ParsingDisplayType == m_state && m_preferredDisplayType.empty())
            {
            m_preferredDisplayType = string;
            return true;
            }
        if (ConverterState::ParsingClassName == m_state && m_className.empty())
            {
            m_className = string;
            return true;
            }
        if (ConverterState::ParsingInstanceId == m_state && !m_instanceId.IsValid())
            {
            m_instanceId = ECInstanceId::FromString(string);
            return true;
            }
        return false;
        }

    bool HandleParens()
        {
        if (nullptr == m_lastLogicalNode && nullptr == dynamic_cast<LogicalNodeCP>(&m_startNode))
            return false;

        NodeCP parensNode = nullptr == m_lastLogicalNode ? m_startNode.GetLeftCP() : m_lastLogicalNode->GetRightCP();
        if (nullptr == parensNode)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Unexpected NULL parens node");

        ECExpressionToOptimizedExpressionConverter parensConverter(*parensNode, m_cacheMutex, true);
        RefCountedPtr<OptimizedExpression> parensExpression = parensConverter.ConvertToOptimizedExpression();
        if (!parensExpression.IsNull())
            {
            CreateLogicalExpressionIfNecessary(parensExpression);
            return true;
            }
        return false;
        }

    bool HandleLogicalOperator()
        {
        if (ConverterState::NotParsing == m_state)
            return true;
        if (ConverterState::ParsingDisplayType == m_state)
            {
            CreateDisplayTypeExpression();
            m_preferredDisplayType.clear();
            return true;
            }
        if (ConverterState::ParsingIsOfClass == m_state)
            {
            CreateSelectedNodeExpression();
            m_schemaName.clear();
            m_className.clear();
            return true;
            }
        if (ConverterState::ParsingClassName == m_state)
            {
            CreateClassNameNodeExpression();
            m_className.clear();
            return true;
            }
        if (ConverterState::ParsingIsInstanceNode == m_state)
            {
            CreateIsInstanceNodeExpression();
            return true;
            }
        if (ConverterState::ParsingIsPropertyGroupingNode == m_state)
            {
            CreateIsPropertyGroupingExpression();
            return true;
            }
        if (ConverterState::ParsingIsECClassGroupingNode == m_state)
            {
            CreateIsECClassGroupingExpression();
            return true;
            }
        if (ConverterState::ParsingInstanceId == m_state)
            {
            CreateInstanceIdExpression();
            m_instanceId.Invalidate();
            return true;
            }
        return false;
        }

    void CreateLogicalExpressionIfNecessary(OptimizedExpressionPtr expression)
        {
        m_state = ConverterState::NotParsing;
        if (m_expression.IsNull())
            m_expression = expression;
        else
            m_expression = LogicalOptimizedExpression::Create(m_expression, expression, m_andOperator);
        }

    void CreateDisplayTypeExpression()
        {
        CreateLogicalExpressionIfNecessary(DisplayTypeOptimizedExpression::Create(m_preferredDisplayType.c_str(), m_equalOperator));
        }

    void CreateSelectedNodeExpression()
        {
        CreateLogicalExpressionIfNecessary(IsOfClassOptimizedExpression::Create(m_schemaName.c_str(), m_className.c_str(), m_cacheMutex));
        }

    void CreateClassNameNodeExpression()
        {
        CreateLogicalExpressionIfNecessary(ClassNameOptimizedExpression::Create(m_className.c_str(), m_cacheMutex));
        }

    void CreateIsInstanceNodeExpression()
        {
        CreateLogicalExpressionIfNecessary(IsInstanceNodeOptimizedExpression::Create());
        }

    void CreateIsPropertyGroupingExpression()
        {
        CreateLogicalExpressionIfNecessary(IsPropertyGroupingOptimizedExpression::Create());
        }

    void CreateIsECClassGroupingExpression()
        {
        CreateLogicalExpressionIfNecessary(IsECClassGroupingOptimizedExpression::Create());
        }

    void CreateInstanceIdExpression()
        {
        CreateLogicalExpressionIfNecessary(InstanceIdOptimizedExpression::Create(m_instanceId));
        }

public:
    ECExpressionToOptimizedExpressionConverter(NodeCR startNode, BeMutex& cacheMutex, bool ignoreFirstParens = false)
        : m_state(ConverterState::NotParsing), m_expression(nullptr), m_openedParens(0), m_startNode(startNode), m_lastLogicalNode(nullptr),
        m_ignoreOpenParens(ignoreFirstParens), m_ignoreArguments(false), m_cacheMutex(cacheMutex)
        {}

    bool StartArrayIndex(NodeCR node) override { return false; }
    bool EndArrayIndex(NodeCR node) override { return false; }

    bool Comma() override
        {
        if (!m_ignoreArguments)
            return false;
        return true;
        }
    bool StartArguments(NodeCR node) override
        {
        if (0 == m_openedParens && ConverterState::ParsingIsOfClass != m_state)
            return false;

        m_ignoreArguments = true;
        return true;
        }

    bool EndArguments(NodeCR node) override
        {
        m_ignoreArguments = false;
        return true;
        }

    bool OpenParens() override
        {
        if (0 != m_openedParens)
            {
            m_openedParens++;
            return true;
            }
        if (m_ignoreOpenParens)
            {
            m_ignoreOpenParens = false;
            return true;
            }

        m_openedParens++;
        return HandleParens();
        }

    bool CloseParens() override
        {
        if (m_openedParens > 0)
            m_openedParens--;

        return true;
        }

    bool ProcessNode(NodeCR node) override
        {
        if (0 != m_openedParens || m_ignoreArguments)
            return true;

        bool success;
        switch (node.GetOperation())
            {
            case TOKEN_And:
            case TOKEN_AndAlso:
                if (nullptr == dynamic_cast<LogicalNodeCP>(&node))
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Got TOKEN_And or TOKEN_AndAlso, but Node is not a LogicalNode");
                m_lastLogicalNode = static_cast<LogicalNodeCP>(&node);
                success = HandleLogicalOperator();
                m_andOperator = true;
                return success;
            case TOKEN_Or:
            case TOKEN_OrElse:
                if (nullptr == dynamic_cast<LogicalNodeCP>(&node))
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Got TOKEN_Or or TOKEN_OrElse, but Node is not a LogicalNode");
                m_lastLogicalNode = static_cast<LogicalNodeCP>(&node);
                success = HandleLogicalOperator();
                m_andOperator = false;
                return success;
            case TOKEN_Equal:
                return HandleEqualityOperator(true);
            case TOKEN_NotEqual:
                return HandleEqualityOperator(false);
            case TOKEN_StringConst:
                if (nullptr == dynamic_cast<LiteralNode const*>(&node))
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Got TOKEN_StringConst, but Node is not a LiteralNode");
                return HandleString(static_cast<LiteralNode const*>(&node)->GetInternalValue().GetUtf8CP());
            case TOKEN_Ident:
                if (nullptr == dynamic_cast<IdentNodeCP>(&node))
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Got TOKEN_Ident, but Node is not a IdentNode");
                return HandleIdent(static_cast<IdentNodeCR>(node));
            case TOKEN_LParen:
                if (nullptr == dynamic_cast<CallNodeCP>(&node))
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Got TOKEN_LParen, but Node is not a CallNode");
                return HandleMethod(static_cast<CallNodeCR>(node));
            case TOKEN_Dot:
                if (nullptr == dynamic_cast<DotNodeCP>(&node))
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Got TOKEN_Dot, but Node is not a DotNode");
                return HandleDotNode(static_cast<DotNodeCR>(node));
            case TOKEN_PrimaryList:
                return true;
            }
        return false;
        }

    OptimizedExpressionPtr GetFinalExpression()
        {
        if (ConverterState::NotParsing != m_state)
            HandleLogicalOperator();

        return m_expression;
        }

    OptimizedExpressionPtr ConvertToOptimizedExpression()
        {
        if (m_startNode.Traverse(*this))
            return GetFinalExpression();

        return nullptr;
        }
};

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+--*/
OptimizedExpressionPtr ECExpressionOptimizer::GetOptimizedExpression(Utf8CP expression)
    {
    OptimizedExpressionPtr optimizedExp;
    if (SUCCESS == m_expressionsCache.Get(optimizedExp, expression))
        return optimizedExp;

    NodePtr node = ECExpressionsHelper(m_expressionsCache).GetNodeFromExpression(expression);
    if (node.IsValid())
        {
        ECExpressionToOptimizedExpressionConverter converter(*node, m_expressionsCache.GetMutex());
        optimizedExp = converter.ConvertToOptimizedExpression();
        }

    m_expressionsCache.Add(expression, optimizedExp);
    return optimizedExp;
    }
