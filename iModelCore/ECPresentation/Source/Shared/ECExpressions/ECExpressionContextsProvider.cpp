/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/ECPresentation.h>
#include <ECObjects/ECExpressions.h>
#include <ECObjects/ECExpressionNode.h>
#include <ECObjects/SystemSymbolProvider.h>
#include "ECExpressionContextsProvider.h"
#include "../../Hierarchies/NavNodesHelper.h"
#include "../Queries/CustomFunctions.h"
#include "../Queries/QueryExecutor.h"
#include "../ExtendedData.h"
#include "../ECSchemaHelper.h"
#include "../NavNodeLocater.h"

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ProviderContext
    {
    virtual ~ProviderContext() {}
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct CommonRulesEngineSymbolsProvider : IECSymbolProvider
{
private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ExpressionStatus CreateSet(EvaluationResult& evalResult, void*, EvaluationResultVector& args)
        {
        bvector<EvaluationResult> values;
        for (EvaluationResultCR arg : args)
            {
            if (!arg.IsECValue())
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("All arguments in `Set` function should be primitive, got: %d. Skipping", (int)arg.GetValueType()));
                continue;
                }

            values.push_back(arg);
            }
        evalResult.SetValueList(*IValueListResult::Create(values));
        return ExpressionStatus::Success;
        }
protected:
    Utf8CP _GetName() const override {return "CommonRulesEngineSymbols";}
    void _PublishSymbols(SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const override
        {
        context.AddSymbol(*MethodSymbol::Create("Set", &CommonRulesEngineSymbolsProvider::CreateSet, nullptr, nullptr));
        }
public:
    CommonRulesEngineSymbolsProvider() {}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RulesEngineRootSymbolsContext : ExpressionContext
{
private:
    SymbolExpressionContextPtr m_internalContext;
    bvector<ProviderContext const*> m_contexts;

private:
    RulesEngineRootSymbolsContext()
        : ExpressionContext(nullptr)
        {
        m_internalContext = SymbolExpressionContext::Create(bvector<Utf8String>(), nullptr);

        CommonRulesEngineSymbolsProvider commonSymbols;
        commonSymbols.PublishSymbols(*m_internalContext, bvector<Utf8String>());
        }

protected:
    ExpressionStatus _ResolveMethod(MethodReferencePtr& result, Utf8CP ident, bool useOuterIfNecessary, ExpressionMethodType methodType) override {return m_internalContext->ResolveMethod(result, ident, useOuterIfNecessary, methodType);}
    bool _IsNamespace() const override {return m_internalContext->IsNamespace();}
    ExpressionStatus _GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contexts, ::uint32_t startIndex) override {return m_internalContext->GetValue(evalResult, primaryList, contexts, startIndex);}
    ExpressionStatus _GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contexts, ::uint32_t startIndex) override {return m_internalContext->GetReference(evalResult, refResult, primaryList, contexts, startIndex);}

public:
    static RefCountedPtr<RulesEngineRootSymbolsContext> Create()
        {
        return new RulesEngineRootSymbolsContext();
        }

    ~RulesEngineRootSymbolsContext()
        {
        for (ProviderContext const* ctx : m_contexts)
            delete ctx;
        }

    SymbolExpressionContext& GetSymbolsContext() const {return *m_internalContext;}

    template<class TProviderContext>
    TProviderContext const& AddContext(TProviderContext const& ctx)
        {
        m_contexts.push_back(&ctx);
        return ctx;
        }
};
typedef RefCountedPtr<RulesEngineRootSymbolsContext> RulesEngineRootSymbolsContextPtr;

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ECInstanceContextEvaluator : PropertySymbol::ContextEvaluator
{
private:
    IConnectionCR m_connection;
    ECInstanceKey m_key;
    ECInstanceContextEvaluator(IConnectionCR connection, ECInstanceKey key)
        : m_connection(connection), m_key(key)
        {}
public:
    static RefCountedPtr<ECInstanceContextEvaluator> Create(IConnectionCR connection, ECInstanceKey key)
        {
        return new ECInstanceContextEvaluator(connection, key);
        }
    ExpressionContextPtr _GetContext() override
        {
        IECInstancePtr instance;
        InstanceExpressionContextPtr instanceContext = InstanceExpressionContext::Create(nullptr);
        if (m_key.IsValid())
            {
            // load the instance
            ECInstancesHelper::LoadInstance(instance, m_connection, m_key);
            }
        else if (m_key.GetClassId().IsValid())
            {
            // if the instance id is not valid, create an empty instance - this
            // makes sure we can further successfully use this instance in ECExpressions
            // and all its properties are NULL
            ECClassCP ecClass = m_connection.GetECDb().Schemas().GetClass(m_key.GetClassId());
            instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
            }
        else
            {
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Invalid ECInstanceKey");
            }
        if (instance.IsValid())
            instanceContext->SetInstance(*instance);
        return instanceContext;
        }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NodeECInstanceContextEvaluator : PropertySymbol::ContextEvaluator
{
private:
    IConnectionCR m_connection;
    NavNodeCPtr m_node;
    NodeECInstanceContextEvaluator(IConnectionCR connection, NavNodeCR node)
        : m_connection(connection), m_node(&node)
        {}
public:
    static RefCountedPtr<NodeECInstanceContextEvaluator> Create(IConnectionCR connection, NavNodeCR node)
        {
        return new NodeECInstanceContextEvaluator(connection, node);
        }
    ExpressionContextPtr _GetContext() override
        {
        ECInstancesNodeKey const* key = m_node->GetKey()->AsECInstanceNodeKey();
        if (nullptr == key || key->GetInstanceKeys().empty())
            return nullptr;

        // TODO: returning first instance key - what if node groups multiple instances???
        ECInstanceKey instanceKey(key->GetInstanceKeys().front().GetClass()->GetId(), key->GetInstanceKeys().front().GetId());
        InstanceExpressionContextPtr instanceContext = InstanceExpressionContext::Create(nullptr);
        IECInstancePtr instance;
        ECInstancesHelper::LoadInstance(instance, m_connection, instanceKey);
        if (instance.IsValid())
            instanceContext->SetInstance(*instance);

        return instanceContext;
        }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ECInstanceMethodSymbolsProvider : IECSymbolProvider
{
private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ExpressionStatus IsOfClass (EvaluationResult& evalResult, void*, ECInstanceListCR instanceData, EvaluationResultVector& args)
        {
        if (2 != args.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(LOG_ERROR, Utf8PrintfString("ECInstanceMethodSymbolsProvider::IsOfClass: WrongNumberOfArguments. Expected 2, actually: %" PRIu64, (uint64_t)args.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        Utf8CP schemaname, classname;
        if (!SystemSymbolProvider::ExtractArg(classname, args[0])|| !SystemSymbolProvider::ExtractArg(schemaname, args[1]))
            {
            ECEXPRESSIONS_EVALUATE_LOG(LOG_ERROR, "ECInstanceMethodSymbolsProvider::IsOfClass class name or schema name is not a string");
            return ExpressionStatus::UnknownError;
            }

        evalResult.InitECValue().SetBoolean(false);
        for (IECInstancePtr const& instance : instanceData)
            {
            if (instance->GetClass().Is(schemaname, classname))
                {
                evalResult.GetECValue()->SetBoolean(true);
                break;
                }
            }

        ECEXPRESSIONS_EVALUATE_LOG(LOG_TRACE, Utf8PrintfString("ECInstanceMethodSymbolsProvider::IsOfClass: Result: %s", evalResult.ToString().c_str()).c_str());
        return ExpressionStatus::Success;
        }
protected:
    Utf8CP _GetName() const override {return "ECInstanceMethods";}
    void _PublishSymbols(SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const override
        {
        context.AddSymbol(*MethodSymbol::Create("IsOfClass", NULL, &IsOfClass));
        }
public:
    ECInstanceMethodSymbolsProvider() {}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ChildrenArtifactsExpressionContext : ExpressionContext
{
private:
    IValueListResultPtr m_artifactsValueList;
protected:
    ExpressionStatus _GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) override
        {
        ExpressionToken nextOperation = primaryList.GetOperation(startIndex);
        if (TOKEN_LParen != nextOperation)
            return ExpressionStatus::MethodRequired;
        CallNodeP callNode = static_cast<CallNodeP>(primaryList.GetOperatorNode(startIndex));
        return callNode->InvokeValueListMethod(evalResult, *m_artifactsValueList, contextsStack);
        }
public:
    ChildrenArtifactsExpressionContext(bvector<NodeArtifacts> artifacts)
        : ExpressionContext(nullptr)
        {
        bvector<EvaluationResult> valuesResult;
        std::transform(artifacts.begin(), artifacts.end(), std::back_inserter(valuesResult), [](NodeArtifacts const& nodeArtifacts)
            {
            EvaluationResult r;
            SymbolExpressionContextPtr context = SymbolExpressionContext::Create(nullptr);
            for (auto const& entry : nodeArtifacts)
                context->AddSymbol(*ValueSymbol::Create(entry.first.c_str(), entry.second));
            r.SetContext(*context);
            return r;
            });
        m_artifactsValueList = IValueListResult::Create(valuesResult);
        }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NodeSymbolsProvider : IECSymbolProvider
{
    struct Context : ProviderContext
        {
        IConnectionCR m_connection;
        NavNodeCPtr m_node;
        Context(IConnectionCR connection, NavNodeCP node)
            : m_connection(connection), m_node(node)
            {}
        };

private:
    Context const& m_context;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ExpressionStatus IsOfClass(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        if (2 != args.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(LOG_ERROR, Utf8PrintfString("NodeSymbolsProvider::IsOfClass: WrongNumberOfArguments. Expected 2, actually: %" PRIu64, (uint64_t)args.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (nullptr == context)
            {
            evalResult.InitECValue().SetBoolean(false);
            ECEXPRESSIONS_EVALUATE_LOG(LOG_TRACE, "NodeSymbolsProvider::IsOfClass: Result: false (context == nullptr)" );
            return ExpressionStatus::Success;
            }

        Context const& ctx = *(Context*)context;
        if (ctx.m_node.IsNull() || !ctx.m_connection.IsOpen())
            {
            evalResult.InitECValue().SetBoolean(false);
            ECEXPRESSIONS_EVALUATE_LOG(LOG_TRACE, "NodeSymbolsProvider::IsOfClass: Result: false (node is null or connection is closed)");
            return ExpressionStatus::Success;
            }

        Utf8CP schemaname, classname;
        if (!SystemSymbolProvider::ExtractArg(classname, args[0]) || !SystemSymbolProvider::ExtractArg(schemaname, args[1]))
            {
            ECEXPRESSIONS_EVALUATE_LOG(LOG_ERROR, "NodeSymbolsProvider::IsOfClass: UnknownError. Invalid class name or schema name");
            return ExpressionStatus::UnknownError;
            }

        NavNodeExtendedData extendedData(*ctx.m_node);
        bset<ECClassId> classIds;
        if (extendedData.HasECClassId())
            classIds.insert(extendedData.GetECClassId());
        else if (ctx.m_node->GetKey()->AsECInstanceNodeKey())
            {
            for (auto const& key : ctx.m_node->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys())
                classIds.insert(key.GetClass()->GetId());
            }

        evalResult.InitECValue().SetBoolean(false);
        for (auto const& classId : classIds)
            {
            ECClassCP nodeClass = ctx.m_connection.GetECDb().Schemas().GetClass(classId);
            if (nullptr == nodeClass)
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, Utf8PrintfString("Invalid ECClass ID associated with the node: %" PRIu64, classId.GetValue()));

            if (nodeClass->Is(schemaname, classname))
                {
                evalResult.GetECValue()->SetBoolean(true);
                break;
                }
            }
        ECEXPRESSIONS_EVALUATE_LOG(LOG_TRACE, Utf8PrintfString("NodeSymbolsProvider::IsOfClass(%s, %s) = %s", classname, schemaname, evalResult.ToString().c_str()).c_str());
        return ExpressionStatus::Success;
        }
protected:
    Utf8CP _GetName() const override {return "Node";}
    void _PublishSymbols(SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const override
        {
        if (m_context.m_node.IsNull())
            {
            context.AddSymbol(*ValueSymbol::Create("IsNull", ECValue(true)));
            context.AddSymbol(*ValueSymbol::Create("Type", ECValue("", false)));
            context.AddSymbol(*ValueSymbol::Create("Label", ECValue("", false)));
            context.AddSymbol(*ValueSymbol::Create("Description", ECValue("", false)));
            context.AddSymbol(*ValueSymbol::Create("ClassName", ECValue("", false)));
            context.AddSymbol(*ValueSymbol::Create("ClassLabel", ECValue("", false)));
            context.AddSymbol(*ValueSymbol::Create("SchemaName", ECValue("", false)));
            context.AddSymbol(*ValueSymbol::Create("SchemaLabel", ECValue("", false)));
            context.AddSymbol(*ValueSymbol::Create("SchemaMajorVersion", ECValue(0)));
            context.AddSymbol(*ValueSymbol::Create("SchemaMinorVersion", ECValue(0)));
            context.AddSymbol(*ValueSymbol::Create("InstanceId", ECValue("", false)));
            context.AddSymbol(*ValueSymbol::Create("BriefcaseId", ECValue("", false)));
            context.AddSymbol(*ValueSymbol::Create("LocalId", ECValue("", false)));
            context.AddSymbol(*ValueSymbol::Create("IsInstanceNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsClassNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsRelationshipClassNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsSchemaNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsSearchNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsClassGroupingNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsPropertyGroupingNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("GroupedInstancesCount", ECValue(0)));
            context.AddSymbol(*ValueSymbol::Create("ECInstance", ECValue()));
            context.AddSymbol(*ValueSymbol::Create("HasChildren", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("ChildrenArtifacts", ECValue()));
            }
        else
            {
            NavNodeCR node = *m_context.m_node;
            NavNodeExtendedData nodeExtendedData(node);

            ECClassCP nodeClass = nullptr;
            if (nodeExtendedData.HasECClassId())
                nodeClass = m_context.m_connection.GetECDb().Schemas().GetClass(nodeExtendedData.GetECClassId());
            else if (node.GetKey()->AsECInstanceNodeKey() && !node.GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().empty())
                nodeClass = node.GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().front().GetClass();

            context.AddSymbol(*ValueSymbol::Create("IsNull", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("Type", ECValue(node.GetType().c_str(), false)));
            context.AddSymbol(*ValueSymbol::Create("Label", ECValue(node.GetLabelDefinition().GetDisplayValue().c_str(), false)));
            context.AddSymbol(*ValueSymbol::Create("Description", ECValue(node.GetDescription().c_str(), false)));
            context.AddSymbol(*ValueSymbol::Create("ClassName", nullptr != nodeClass ? ECValue(nodeClass->GetName().c_str(), false) : ECValue()));
            context.AddSymbol(*ValueSymbol::Create("ClassLabel", nullptr != nodeClass ? ECValue(nodeClass->GetDisplayLabel().c_str(), false) : ECValue()));
            context.AddSymbol(*ValueSymbol::Create("SchemaName", nullptr != nodeClass ? ECValue(nodeClass->GetSchema().GetName().c_str(), true) : ECValue()));
            context.AddSymbol(*ValueSymbol::Create("SchemaLabel", nullptr != nodeClass ? ECValue(nodeClass->GetSchema().GetDisplayLabel().c_str(), true) : ECValue()));
            context.AddSymbol(*ValueSymbol::Create("SchemaMajorVersion", nullptr != nodeClass ? ECValue((int)nodeClass->GetSchema().GetVersionRead()) : ECValue(0)));
            context.AddSymbol(*ValueSymbol::Create("SchemaMinorVersion", nullptr != nodeClass ? ECValue((int)nodeClass->GetSchema().GetVersionMinor()) : ECValue(0)));
            context.AddSymbol(*ValueSymbol::Create("IsClassNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsRelationshipClassNode", ECValue(node.GetType().Equals(NAVNODE_TYPE_ECRelationshipGroupingNode))));
            context.AddSymbol(*ValueSymbol::Create("IsSchemaNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsSearchNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsClassGroupingNode", ECValue(node.GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode))));
            context.AddSymbol(*ValueSymbol::Create("IsPropertyGroupingNode", ECValue(node.GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode) || node.GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode))));
            context.AddSymbol(*ValueSymbol::Create("GroupedInstancesCount", ECValue(node.GetKey()->AsGroupingNodeKey() ? node.GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount() : 0)));
            context.AddSymbol(*ValueSymbol::Create("HasChildren", ECValue(node.DeterminedChildren() && node.HasChildren())));
            context.AddSymbol(*ContextSymbol::CreateContextSymbol("ChildrenArtifacts", *new ChildrenArtifactsExpressionContext(nodeExtendedData.GetChildrenArtifacts())));

            bool didAddECInstanceSymbols = false;
            if (node.GetType().Equals(NAVNODE_TYPE_ECInstancesNode))
                {
                if (nullptr == node.GetKey()->AsECInstanceNodeKey() || node.GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().empty())
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "ECInstance node is not associated with any ECInstance");

                // TODO: returning first instance key - what if node groups multiple instances???
                ECClassInstanceKeyCR key = node.GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().front();
                context.AddSymbol(*ValueSymbol::Create("InstanceId", ECValue(key.GetId().GetValueUnchecked())));
                context.AddSymbol(*ValueSymbol::Create("BriefcaseId", ECValue(CommonTools::ToBase36String(CommonTools::GetBriefcaseId(key.GetId())).c_str(), false)));
                context.AddSymbol(*ValueSymbol::Create("LocalId", ECValue(CommonTools::ToBase36String(CommonTools::GetLocalId(key.GetId())).c_str(), false)));
                context.AddSymbol(*ValueSymbol::Create("IsInstanceNode", ECValue(true)));
                context.AddSymbol(*PropertySymbol::Create("ECInstance", *NodeECInstanceContextEvaluator::Create(m_context.m_connection, node)));
                didAddECInstanceSymbols = true;
                }

            if (!didAddECInstanceSymbols)
                {
                context.AddSymbol(*ValueSymbol::Create("InstanceId", ECValue()));
                context.AddSymbol(*ValueSymbol::Create("BriefcaseId", ECValue()));
                context.AddSymbol(*ValueSymbol::Create("LocalId", ECValue()));
                context.AddSymbol(*ValueSymbol::Create("IsInstanceNode", ECValue(false)));
                context.AddSymbol(*ValueSymbol::Create("ECInstance", ECValue()));
                }
            }

        context.AddSymbol(*MethodSymbol::Create("IsOfClass", &NodeSymbolsProvider::IsOfClass, nullptr, const_cast<Context*>(&m_context)));
        }
public:
    NodeSymbolsProvider(Context const& context)
        : m_context(context)
        {}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NodeContextEvaluator : PropertySymbol::ContextEvaluator
{
private:
    RulesEngineRootSymbolsContext& m_rootContext;
    IConnectionCR m_connection;
    Utf8String m_rulesetId;
    INavNodeLocaterCP m_locater;
    NavNodeKeyCP m_key;
    SymbolExpressionContextPtr m_context;
    INodeLabelCalculator const& m_nodeLabelCalculator;
    NodeContextEvaluator(RulesEngineRootSymbolsContext& rootContext, IConnectionCR connection, Utf8String rulesetId, INavNodeLocaterCP locater, NavNodeKeyCP key, INodeLabelCalculator const& nodeLabelCalculator)
        : m_rootContext(rootContext), m_connection(connection), m_rulesetId(rulesetId), m_locater(locater), m_key(key), m_context(nullptr), m_nodeLabelCalculator(nodeLabelCalculator)
        {}
public:
    static RefCountedPtr<NodeContextEvaluator> Create(RulesEngineRootSymbolsContext& rootContext, IConnectionCR connection,
        Utf8String rulesetId, INavNodeLocaterCP locater, NavNodeKeyCP key,  INodeLabelCalculator const& nodeLabelCalculator)
        {
        return new NodeContextEvaluator(rootContext, connection, rulesetId, locater, key, nodeLabelCalculator);
        }
    virtual ExpressionContextPtr _GetContext() override
        {
        if (m_context.IsValid())
            return m_context;

        NavNodeCPtr node = (nullptr != m_key && nullptr != m_locater) ? m_locater->LocateNode(m_connection, m_rulesetId, *m_key) : nullptr;
        if (node.IsNull() && nullptr != m_key && nullptr != m_key->AsECInstanceNodeKey())
            {
            ECInstancesNodeKey const* instancesNodeKey = m_key->AsECInstanceNodeKey();
            LabelDefinitionPtr label = m_nodeLabelCalculator.GetNodeLabel(instancesNodeKey->GetInstanceKeys().front());
            NavNodePtr temp = NavNodesFactory().CreateECInstanceNode(m_connection, "", nullptr, instancesNodeKey->GetInstanceKeys(), *label);
            node = temp;
            }

        NodeSymbolsProvider nodeSymbols(m_rootContext.AddContext(*new NodeSymbolsProvider::Context(m_connection, node.get())));
        m_context = SymbolExpressionContext::Create(nullptr);
        nodeSymbols.PublishSymbols(*m_context, bvector<Utf8String>());
        return m_context;
        }
};

//=======================================================================================
//! Provides functions to access user settings / ruleset variables
// @bsiclass
//=======================================================================================
struct RulesetVariablesSymbolsProvider : IECSymbolProvider
{
    struct Context : ProviderContext
        {
        RulesetVariables const& m_rulesetVariables;
        IUsedRulesetVariablesListener* m_usedVariablesListener;
        Context(RulesetVariables const& rulesetVariables, IUsedRulesetVariablesListener* usedVariablesListener)
            : m_rulesetVariables(rulesetVariables), m_usedVariablesListener(usedVariablesListener)
            {}
        };
private:
    Context const& m_context;

private:
    static void OnVariableUsed(Context const& context, Utf8CP variableId)
        {
        if (nullptr != context.m_usedVariablesListener)
            context.m_usedVariablesListener->OnVariableUsed(variableId);
        }
    static ExpressionStatus GetStringVariableValue(EvaluationResult& evalResult, void* methodContext, EvaluationResultVector& arguments)
        {
        if (1 != arguments.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(LOG_ERROR, Utf8PrintfString("RulesetVariablesSymbolsProvider::GetStringVariableValue: WrongNumberOfArguments. Expected 1, actually: %" PRIu64, (uint64_t)arguments.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (!arguments[0].IsECValue() || !arguments[0].GetECValue()->IsString())
            {
            ECEXPRESSIONS_EVALUATE_LOG(LOG_ERROR, "RulesetVariablesSymbolsProvider::GetStringVariableValue: Wrong argument type (first argument is not ECValue, or is not a string)");
            return ExpressionStatus::WrongType;
            }

        Utf8CP variableId = arguments[0].GetECValue()->GetUtf8CP();
        Context const& context = *static_cast<Context*>(methodContext);
        evalResult.InitECValue().SetUtf8CP(context.m_rulesetVariables.GetStringValue(variableId));

        OnVariableUsed(context, variableId);

        return ExpressionStatus::Success;
        }
    static ExpressionStatus GetIntVariableValue(EvaluationResult& evalResult, void* methodContext, EvaluationResultVector& arguments)
        {
        if (1 != arguments.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(LOG_ERROR, Utf8PrintfString("RulesetVariablesSymbolsProvider::GetIntVariableValue: WrongNumberOfArguments. Expected 1, actually: %" PRIu64, (uint64_t)arguments.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (!arguments[0].IsECValue() || !arguments[0].GetECValue()->IsString())
            {
            ECEXPRESSIONS_EVALUATE_LOG(LOG_ERROR, "RulesetVariablesSymbolsProvider::GetIntVariableValue: Wrong argument type (first argument is not ECValue, or is not a string)");
            return ExpressionStatus::WrongType;
            }

        Utf8CP variableId = arguments[0].GetECValue()->GetUtf8CP();
        Context const& context = *static_cast<Context*>(methodContext);
        evalResult.InitECValue().SetLong(context.m_rulesetVariables.GetIntValue(variableId));

        OnVariableUsed(context, variableId);

        return ExpressionStatus::Success;
        }
    static ExpressionStatus GetIntArrayVariableValue(EvaluationResult& evalResult, void* methodContext, EvaluationResultVector& arguments)
        {
        if (1 != arguments.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(LOG_ERROR, Utf8PrintfString("RulesetVariablesSymbolsProvider::GetIntArrayVariableValue: WrongNumberOfArguments. Expected 1, actually: %" PRIu64, (uint64_t)arguments.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (!arguments[0].IsECValue() || !arguments[0].GetECValue()->IsString())
            {
            ECEXPRESSIONS_EVALUATE_LOG(LOG_ERROR, "RulesetVariablesSymbolsProvider::GetIntArrayVariableValue: Wrong argument type (first argument is not ECValue, or is not a string)");
            return ExpressionStatus::WrongType;
            }

        if (nullptr == methodContext)
            {
            ECEXPRESSIONS_EVALUATE_LOG(LOG_ERROR, "RulesetVariablesSymbolsProvider::GetIntArrayVariableValue: UnknownError (nullptr == methodContext)");
            return ExpressionStatus::UnknownError;
            }

        Utf8CP variableId = arguments[0].GetECValue()->GetUtf8CP();
        Context const& context = *static_cast<Context*>(methodContext);
        bvector<int64_t> values = context.m_rulesetVariables.GetIntValues(variableId);
        bvector<EvaluationResult> resultValues;
        resultValues.reserve(values.size());
        for (int64_t value : values)
            {
            EvaluationResult r;
            r.InitECValue().SetLong(value);
            resultValues.push_back(r);
            }
        evalResult.SetValueList(*IValueListResult::Create(resultValues));

        OnVariableUsed(context, variableId);

        return ExpressionStatus::Success;
        }
    static ExpressionStatus GetBoolVariableValue(EvaluationResult& evalResult, void* methodContext, EvaluationResultVector& arguments)
        {
        if (1 != arguments.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(LOG_ERROR, Utf8PrintfString("RulesetVariablesSymbolsProvider::GetBoolVariableValue: WrongNumberOfArguments. Expected 1, actually: %" PRIu64, (uint64_t)arguments.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (!arguments[0].IsECValue() || !arguments[0].GetECValue()->IsString())
            {
            ECEXPRESSIONS_EVALUATE_LOG(LOG_ERROR, "RulesetVariablesSymbolsProvider::GetBoolVariableValue: Wrong argument type (first argument is not ECValue, or is not a string)");
            return ExpressionStatus::WrongType;
            }

        Utf8CP variableId = arguments[0].GetECValue()->GetUtf8CP();
        Context const& context = *static_cast<Context*>(methodContext);
        evalResult.InitECValue().SetBoolean(context.m_rulesetVariables.GetBoolValue(variableId));

        OnVariableUsed(context, variableId);
        return ExpressionStatus::Success;
        }
    static ExpressionStatus HasVariable(EvaluationResult& evalResult, void* methodContext, EvaluationResultVector& arguments)
        {
        if (1 != arguments.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(LOG_ERROR, Utf8PrintfString("RulesetVariablesSymbolsProvider::HasVariable: WrongNumberOfArguments. Expected 1, actually: %" PRIu64, (uint64_t)arguments.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (!arguments[0].IsECValue() || !arguments[0].GetECValue()->IsString())
            {
            ECEXPRESSIONS_EVALUATE_LOG(LOG_ERROR, "RulesetVariablesSymbolsProvider::HasVariable: Wrong argument type (first argument is not ECValue, or is not a string)");
            return ExpressionStatus::WrongType;
            }

        Utf8CP variableId = arguments[0].GetECValue()->GetUtf8CP();
        Context const& context = *static_cast<Context*>(methodContext);
        evalResult.InitECValue().SetBoolean(context.m_rulesetVariables.HasValue(variableId));

        OnVariableUsed(context, variableId);
        return ExpressionStatus::Success;
        }
protected:
    Utf8CP _GetName() const override {return "RulesetVariables";}
    void _PublishSymbols(SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const override
        {
        void* methodContext = const_cast<Context*>(&m_context);

        // deprecated / DgnClientSdk
        context.AddSymbol(*MethodSymbol::Create("GetSettingValue", &RulesetVariablesSymbolsProvider::GetStringVariableValue, nullptr, methodContext));
        context.AddSymbol(*MethodSymbol::Create("GetSettingIntValue", &RulesetVariablesSymbolsProvider::GetIntVariableValue, nullptr, methodContext));
        context.AddSymbol(*MethodSymbol::Create("GetSettingIntValues", &RulesetVariablesSymbolsProvider::GetIntArrayVariableValue, nullptr, methodContext));
        context.AddSymbol(*MethodSymbol::Create("GetSettingBoolValue", &RulesetVariablesSymbolsProvider::GetBoolVariableValue, nullptr, methodContext));
        context.AddSymbol(*MethodSymbol::Create("HasSetting", &RulesetVariablesSymbolsProvider::HasVariable, nullptr, methodContext));

        // current / iModelJS
        context.AddSymbol(*MethodSymbol::Create("GetVariableStringValue", &RulesetVariablesSymbolsProvider::GetStringVariableValue, nullptr, methodContext));
        context.AddSymbol(*MethodSymbol::Create("GetVariableIntValue", &RulesetVariablesSymbolsProvider::GetIntVariableValue, nullptr, methodContext));
        context.AddSymbol(*MethodSymbol::Create("GetVariableIntValues", &RulesetVariablesSymbolsProvider::GetIntArrayVariableValue, nullptr, methodContext));
        context.AddSymbol(*MethodSymbol::Create("GetVariableBoolValue", &RulesetVariablesSymbolsProvider::GetBoolVariableValue, nullptr, methodContext));
        context.AddSymbol(*MethodSymbol::Create("HasVariable", &RulesetVariablesSymbolsProvider::HasVariable, nullptr, methodContext));
        }
public:
    RulesetVariablesSymbolsProvider(Context const& context) : m_context(context) {}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct LabelSymbolsProvider : IECSymbolProvider
{
    struct Context : ProviderContext
        {
        IConnectionCR m_connection;
        Context(IConnectionCR connection) : m_connection(connection) {}
        };

private:
    Context const& m_context;

private:
    static ExpressionStatus GetRelatedDisplayLabelQuery(Utf8StringR query, Utf8CP relationshipName, Utf8CP direction, Utf8CP className)
        {
        Utf8CP thisInstanceIdColumnName, thisClassIdColumnName,
            relatedInstanceIdColumnName, relatedClassIdColumnName;
        if (0 == BeStringUtilities::Stricmp("Forward", direction))
            {
            thisInstanceIdColumnName = "SourceECInstanceId";
            thisClassIdColumnName = "SourceECClassId";
            relatedInstanceIdColumnName = "TargetECInstanceId";
            relatedClassIdColumnName = "TargetECClassId";
            }
        else if (0 == BeStringUtilities::Stricmp("Backward", direction))
            {
            thisInstanceIdColumnName = "TargetECInstanceId";
            thisClassIdColumnName = "TargetECClassId";
            relatedInstanceIdColumnName = "SourceECInstanceId";
            relatedClassIdColumnName = "SourceECClassId";
            }
        else
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid relationship direction specified. Expected 'Forward' or 'Backward', got: '%s'", direction));
            return ExpressionStatus::UnknownError;
            }

        Utf8String relationshipSchemaName, relationshipClassName;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(relationshipSchemaName, relationshipClassName, relationshipName))
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Failed to parse specified relationship name: '%s'", relationshipName));
            return ExpressionStatus::UnknownError;
            }

        Utf8String relatedClassSchemaName, relatedClassName;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(relatedClassSchemaName, relatedClassName, className))
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Failed to parse specified class name: '%s'", className));
            return ExpressionStatus::UnknownError;
            }

        query = Utf8PrintfString("SELECT " FUNCTION_NAME_GetRelatedDisplayLabel "([related].[ECClassId], [related].[ECInstanceId]) "
                                 "FROM %%s this, [%s].[%s] relationship, [%s].[%s] related "
                                 "WHERE this.[ECInstanceId]=? AND "
                                 "      this.[ECInstanceId]=relationship.[%s] AND this.[ECClassId]=relationship.[%s] AND "
                                 "      related.[ECInstanceId]=relationship.[%s] AND related.[ECClassId]=relationship.[%s]",
                                 relationshipSchemaName.c_str(), relationshipClassName.c_str(),
                                 relatedClassSchemaName.c_str(), relatedClassName.c_str(),
                                 thisInstanceIdColumnName, thisClassIdColumnName,
                                 relatedInstanceIdColumnName, relatedClassIdColumnName);
        return ExpressionStatus::Success;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ExpressionStatus GetRelatedDisplayLabel (EvaluationResult& evalResult, void* context, ECInstanceListCR instanceData, EvaluationResultVector& args)
        {
        if (instanceData.empty())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_TRACE, LOG_INFO, "Requesting display label on an empty ECInstances list. Returning NULL.");
            evalResult.InitECValue().SetToNull();
            return ExpressionStatus::Success;
            }

        if (3 != args.size())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Wrong number of arguments. Expected 3, got: %" PRIu64, (uint64_t)args.size()));
            return ExpressionStatus::WrongNumberOfArguments;
            }

        for (size_t i = 0; i < 3; ++i)
            {
            if (!args[i].IsECValue() || !args[i].GetECValue()->IsString())
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid type of argument %" PRIu64 " - expected a string", (uint64_t)i));
                return ExpressionStatus::WrongType;
                }
            }

        Context ctx = *static_cast<Context*>(context);

        Utf8String queryFormat;
        ExpressionStatus stat = GetRelatedDisplayLabelQuery(queryFormat, args[0].GetECValue()->GetUtf8CP(), args[1].GetECValue()->GetUtf8CP(), args[2].GetECValue()->GetUtf8CP());
        if (ExpressionStatus::Success != stat)
            return stat;

        for (IECInstancePtr const& instance : instanceData)
            {
            Utf8PrintfString query(queryFormat.c_str(), instance->GetClass().GetECSqlName().c_str());

            ECSqlStatement stmt;
            ECSqlStatus status = stmt.Prepare(ctx.m_connection.GetECDb().Schemas(), ctx.m_connection.GetDb(), query.c_str());
            if (!status.IsSuccess())
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Failed to prepare related display label query");

            ECInstanceId id;
            ECInstanceId::FromString(id, instance->GetInstanceId().c_str());
            status = stmt.BindId(1, id);
            if (!status.IsSuccess())
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Failed to bind related display label query parameter");

            if (BE_SQLITE_ROW == QueryExecutorHelper::Step(stmt))
                {
                evalResult.InitECValue() = ECValue(stmt.GetValueText(0));
                ECEXPRESSIONS_EVALUATE_LOG(LOG_TRACE, Utf8PrintfString("LabelOverrideSymbolsProvider::GetRelatedDisplayLabel: Result: %s", evalResult.ToString().c_str()).c_str());
                return ExpressionStatus::Success;
                }
            }

        evalResult.InitECValue().SetToNull();
        ECEXPRESSIONS_EVALUATE_LOG(LOG_TRACE, Utf8PrintfString("LabelOverrideSymbolsProvider::GetRelatedDisplayLabel: Result: %s", evalResult.ToString().c_str()).c_str());
        return ExpressionStatus::Success;
        }

protected:
    Utf8CP _GetName() const override {return "Label";}
    void _PublishSymbols(SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const override
        {
        void* methodContext = const_cast<Context*>(&m_context);
        context.AddSymbol(*MethodSymbol::Create("GetRelatedDisplayLabel", nullptr, &GetRelatedDisplayLabel, methodContext));
        }

public:
    LabelSymbolsProvider(Context const& context) : m_context(context) {}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ECDbSymbolsProvider : IECSymbolProvider
{
    struct Context : ProviderContext
        {
        ECDbExpressionSymbolProvider m_provider;
        Context(IConnectionCR connection) : m_provider(connection.GetECDb(), connection.GetStatementCache()) {}
        };

private:
    Context const& m_context;

protected:
    Utf8CP _GetName() const override { return "ECDbSymbols"; }
    void _PublishSymbols(SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const override
        {
        m_context.m_provider.PublishSymbols(context, requestedSymbolSets);
        }

public:
    ECDbSymbolsProvider(Context const& context) : m_context(context) {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static RulesEngineRootSymbolsContextPtr CreateRulesEngineRootContext(ECExpressionContextsProvider::ContextParametersBase const& params)
    {
    RulesEngineRootSymbolsContextPtr rootCtx = RulesEngineRootSymbolsContext::Create();

    // Ruleset variables
    RulesetVariablesSymbolsProvider rulesetVariablesSymbols(rootCtx->AddContext(*new RulesetVariablesSymbolsProvider::Context(params.GetRulesetVariables(), params.GetUsedVariablesListener())));
    rulesetVariablesSymbols.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());

    // ECInstance methods
    ECInstanceMethodSymbolsProvider ecInstanceMethods;
    ecInstanceMethods.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());

    // ECDb methods
    ECDbSymbolsProvider ecdbSymbols(rootCtx->AddContext(*new ECDbSymbolsProvider::Context(params.GetConnection())));
    ecdbSymbols.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());

    return rootCtx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionContextPtr ECExpressionContextsProvider::GetRulesEngineRootContext(ContextParametersBase const& params)
    {
    return CreateRulesEngineRootContext(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionContextPtr ECExpressionContextsProvider::GetNodeRulesContext(NodeRulesContextParameters const& params)
    {
    RulesEngineRootSymbolsContextPtr rootCtx = CreateRulesEngineRootContext(params);

    // ParentNode
    NodeSymbolsProvider nodeSymbols(rootCtx->AddContext(*new NodeSymbolsProvider::Context(params.GetConnection(), params.GetParentNode())));
    SymbolExpressionContextPtr parentNodeCtx = SymbolExpressionContext::Create(nullptr);
    nodeSymbols.PublishSymbols(*parentNodeCtx, bvector<Utf8String>());
    rootCtx->GetSymbolsContext().AddSymbol(*ContextSymbol::CreateContextSymbol("ParentNode", *parentNodeCtx));

    return rootCtx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionContextPtr ECExpressionContextsProvider::GetContentRulesContext(ContentRulesContextParameters const& params)
    {
    RulesEngineRootSymbolsContextPtr rootCtx = CreateRulesEngineRootContext(params);

    // SelectedNode
    rootCtx->GetSymbolsContext().AddSymbol(*PropertySymbol::Create("SelectedNode", *NodeContextEvaluator::Create(*rootCtx, params.GetConnection(), params.GetRulesetId(),
        params.GetNodeLocater(), params.GetSelectedNodeKey(), params.GetNodeLabelCalculator())));

    // Content-specific
    rootCtx->GetSymbolsContext().AddSymbol(*ValueSymbol::Create("ContentDisplayType", ECValue(params.GetContentDisplayType().c_str())));
    rootCtx->GetSymbolsContext().AddSymbol(*ValueSymbol::Create("SelectionProviderName", ECValue(params.GetSelectionProviderName().c_str())));
    rootCtx->GetSymbolsContext().AddSymbol(*ValueSymbol::Create("IsSubSelection", ECValue(params.IsSubSelection())));

    return rootCtx;
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NodeInstanceKeysEvaluator : PropertySymbol::ValueListEvaluator
{
private:
    INavNodeKeysContainerCR m_nodeKeys;
public:
    NodeInstanceKeysEvaluator(INavNodeKeysContainerCR nodeKeys) : m_nodeKeys(nodeKeys) {}
    IValueListResultPtr _EvaluateValueList() override
        {
        bvector<EvaluationResult> instanceKeySymbols;
        for (NavNodeKeyCPtr nodeKey : m_nodeKeys)
            {
            if (!nodeKey->AsECInstanceNodeKey())
                continue;

            for (ECClassInstanceKeyCR instanceKey : nodeKey->AsECInstanceNodeKey()->GetInstanceKeys())
                {
                EvaluationResult r;
                SymbolExpressionContextPtr context = SymbolExpressionContext::Create(nullptr);
                context->AddSymbol(*ValueSymbol::Create("ECClassId", ECValue(instanceKey.GetClass()->GetId().GetValue())));
                context->AddSymbol(*ValueSymbol::Create("ECInstanceId", ECValue(instanceKey.GetId().GetValue())));
                r.SetContext(*context);
                instanceKeySymbols.push_back(r);
                }
            }
        return IValueListResult::Create(instanceKeySymbols);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionContextPtr ECExpressionContextsProvider::GetContentSpecificationInstanceFilterContext(ContentSpecificationInstanceFilterContextParameters const& params)
    {
    RulesEngineRootSymbolsContextPtr rootCtx = CreateRulesEngineRootContext(params);

    rootCtx->GetSymbolsContext().AddSymbol(*PropertySymbol::Create("SelectedInstanceKeys", *new NodeInstanceKeysEvaluator(params.GetInput())));

    return rootCtx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionContextPtr ECExpressionContextsProvider::GetCustomizationRulesContext(CustomizationRulesContextParameters const& params)
    {
    RulesEngineRootSymbolsContextPtr rootCtx = CreateRulesEngineRootContext(params);

    // ParentNode
    NodeSymbolsProvider parentNodeSymbols(rootCtx->AddContext(*new NodeSymbolsProvider::Context(params.GetConnection(), params.GetParentNode())));
    SymbolExpressionContextPtr parentNodeCtx = SymbolExpressionContext::Create(nullptr);
    parentNodeSymbols.PublishSymbols(*parentNodeCtx, bvector<Utf8String>());
    rootCtx->GetSymbolsContext().AddSymbol(*ContextSymbol::CreateContextSymbol("ParentNode", *parentNodeCtx));

    // ThisNode
    NodeSymbolsProvider nodeSymbols(rootCtx->AddContext(*new NodeSymbolsProvider::Context(params.GetConnection(), &params.GetNode())));
    SymbolExpressionContextPtr thisNodeCtx = SymbolExpressionContext::Create(nullptr);
    nodeSymbols.PublishSymbols(*thisNodeCtx, bvector<Utf8String>());
    rootCtx->GetSymbolsContext().AddSymbol(*ContextSymbol::CreateContextSymbol("ThisNode", *thisNodeCtx));

    // this
    if (params.GetNode().GetType().Equals(NAVNODE_TYPE_ECInstancesNode))
        rootCtx->GetSymbolsContext().AddSymbol(*PropertySymbol::Create("this", *NodeECInstanceContextEvaluator::Create(params.GetConnection(), params.GetNode())));
    else
        rootCtx->GetSymbolsContext().AddSymbol(*ValueSymbol::Create("this", ECValue()));

    // related instance contexts
    NavNodeExtendedData extendedData(params.GetNode());
    bvector<ItemExtendedData::RelatedInstanceKey> relatedInstanceKeys = extendedData.GetRelatedInstanceKeys();
    for (ItemExtendedData::RelatedInstanceKey const& key : relatedInstanceKeys)
        {
        rootCtx->GetSymbolsContext().AddSymbol(*PropertySymbol::Create(key.GetAlias(),
            *ECInstanceContextEvaluator::Create(params.GetConnection(), key.GetInstanceKey())));
        }

    // Label related methods
    LabelSymbolsProvider labelOverrrideMethods(rootCtx->AddContext(*new LabelSymbolsProvider::Context(params.GetConnection())));
    labelOverrrideMethods.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());

    return rootCtx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionContextPtr ECExpressionContextsProvider::GetCalculatedPropertyContext(CalculatedPropertyContextParameters const& params)
    {
    RulesEngineRootSymbolsContextPtr rootCtx = CreateRulesEngineRootContext(params);

    // this
    rootCtx->GetSymbolsContext().AddSymbol(*PropertySymbol::Create("this", *NodeECInstanceContextEvaluator::Create(params.GetConnection(), params.GetNode())));

    return rootCtx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECExpressionsHelper::EvaluateECExpression(ECValueR result, Utf8StringCR expression, ExpressionContextR context)
    {
    auto scope = Diagnostics::Scope::Create("EvaluateECExpression");

    NodePtr node = GetNodeFromExpression(expression.c_str());
    if (node.IsNull())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Failed to parse ECExpression: %s", expression.c_str()));
        return false;
        }

    ValueResultPtr valueResult;
    if (ExpressionStatus::Success != node->GetValue(valueResult, context))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Failed to evaluate ECExpression: %s", expression.c_str()));
        return false;
        }

    if (ExpressionStatus::Success != valueResult->GetECValue(result))
        {
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Could not get ECValue from value result");
        return false;
        }

    return true;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SelectColumnsInfo
    {
    Utf8String ClassIdColumnName;
    Utf8String InstanceIdColumnName;
    };
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SelectClassNames
    {
    Utf8String SchemaName;
    Utf8String ClassName;
    };
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RelatedClassInfo
    {
    Utf8String Direction;
    SelectColumnsInfo ThisColumn;
    SelectColumnsInfo RelatedColumn;
    SelectClassNames RelationshipNames;
    SelectClassNames RelatedClassNames;
    };
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct JoinClassInfo
    {
    SelectClassNames RelatedClassName;
    Utf8String RelatedClassAlias;
    QueryClauseAndBindings WhereClause;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECExpressionToECSqlConverter : NodeVisitor
{
#define ARGUMENTS_PRECONDITION() if (m_inArguments && m_ignoreArguments) return true;

private:
    IPresentationQueryFieldTypesProvider const* m_fieldTypes;
    ExpressionContext* m_expressionContext;
    bvector<Utf8String> m_usedClasses;
    bool m_shouldEvaluateValueLists;
    Utf8String m_ecsql;
    BoundQueryValuesList m_bindings;
    RefCountedPtr<CallNode const> m_currentValueListMethodNode;
    RefCountedPtr<IdentNode const> m_currentValueListIdentNode;
    bool m_skipNextArgumentsOpeningBrace;
    bool m_ignoreNextArguments;
    int32_t m_inArguments;
    int32_t m_ignoreArguments;
    int32_t m_inStructProperty;
    ExpressionToken m_previousToken;
    bvector<Utf8String> m_nodesStack;
    bset<NodeCP> m_ignoredNodes;

private:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    static bool NeedsSpaceAfter(Utf8StringCR str)
        {
        if (str.empty())
            return false;

        Utf8Char c = str[str.size() - 1];
        switch (c)
            {
            case '(':
            case '[':
            case '.':
                return false;
            }
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    static bool NeedsSpaceBefore(Utf8StringCR str)
        {
        if (str.empty())
            return false;

        Utf8Char c = *str.begin();
        switch (c)
            {
            case ')':
            case ']':
            case '.':
            case ',':
            case ';':
            case ':':
                return false;
            }
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    void Append(Utf8StringCR value)
        {
        if (!m_ecsql.empty() && NeedsSpaceAfter(m_ecsql) && NeedsSpaceBefore(value))
            {
            m_ecsql.append(" ");
            m_nodesStack.back().append(" ");
            }

        if (m_inStructProperty)
            m_nodesStack.back().append(value);
        else
            m_nodesStack.push_back(value);

        m_ecsql.append(value);
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    void Append(Utf8StringCR value, bool prependSpace)
        {
        if (!m_ecsql.empty() && prependSpace)
            {
            m_ecsql.append(" ");
            m_nodesStack.back().append(" ");
            }

        if (m_inStructProperty)
            m_nodesStack.back().append(value);
        else
            m_nodesStack.push_back(value);

        m_ecsql.append(value);
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    void WrapPreviousNode(Utf8StringCR before, Utf8StringCR after)
        {
        Utf8String::size_type previousNodePos = m_ecsql.rfind(m_nodesStack.back());
        if (previousNodePos == Utf8String::npos)
            return;

        m_ecsql.insert(previousNodePos, before);
        Append(after);
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleLikeToken(NodeCR node)
        {
        if (m_previousToken != TOKEN_StringConst)
            WrapPreviousNode("CAST(", "AS TEXT)");

        Append("LIKE");
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleIdentNode(IdentNodeCR node)
        {
        if (0 == strcmp("SelectedInstanceKeys", node.GetName()))
            {
            m_currentValueListIdentNode = &node;
            return;
            }
        Append(Utf8PrintfString("[%s]", node.GetName()));
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleNullNode()
        {
        if (TOKEN_Equal == m_previousToken)
            {
            DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::ECExpressions, m_ecsql.EndsWith("="), Utf8PrintfString("Expecting ECSQL to end with `=`, actual: '%s'", m_ecsql.c_str()));
            m_ecsql.replace(m_ecsql.size() - 1, 1, "IS");
            }
        if (TOKEN_NotEqual == m_previousToken)
            {
            DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::ECExpressions, m_ecsql.EndsWith("<>"), Utf8PrintfString("Expecting ECSQL to end with `<>`, actual: '%s'", m_ecsql.c_str()));
            m_ecsql.replace(m_ecsql.size() - 2, 2, "IS NOT");
            }
        Append("NULL");
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleScopeEnd()
        {
        if (m_inStructProperty)
            {
            Append("]");
            --m_inStructProperty;
            }
        if (0 == m_inStructProperty && !m_nodesStack.empty() && m_fieldTypes != nullptr && (m_previousToken == TOKEN_Ident || m_previousToken == TOKEN_Dot))
            {
            Utf8String name = m_nodesStack.back();
            PresentationQueryFieldType fieldType = m_fieldTypes->GetFieldType(name.Trim("[]"));
            switch (fieldType)
                {
                case PresentationQueryFieldType::LabelDefinition:
                    WrapPreviousNode(Utf8String(FUNCTION_NAME_GetLabelDefinitionDisplayValue).append("("), ")");
                    break;
                case PresentationQueryFieldType::NavigationPropertyValue:
                    WrapPreviousNode(Utf8String(FUNCTION_NAME_GetLabelDefinitionDisplayValue).append("(").append("json_extract("), ", '$.label'))");
                    break;
                }
            }
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    static Utf8String GetInstanceCallArgument(Utf8String instanceExpression, CallNodeCR callNode)
        {
        if (instanceExpression.Equals("[ThisNode]") || instanceExpression.Equals("ThisNode"))
            instanceExpression = "[this]";

        if (0 == strcmp("IsOfClass", callNode.GetMethodName()))
            instanceExpression.append(".[ECClassId]");

        return instanceExpression;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    Utf8String GetCallNodePrefix(CallNodeCR node)
        {
        Utf8String prefix;
        if (node.ToString().StartsWith("."))
            {
            size_t lastSpaceIndex = m_ecsql.find_last_of('[');
            if (Utf8String::npos != lastSpaceIndex)
                {
                prefix = m_ecsql.substr(lastSpaceIndex);
                m_ecsql = m_ecsql.substr(0, lastSpaceIndex);
                }
            else
                {
                prefix.swap(m_ecsql);
                }
            }
        return prefix;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleIsOfClassSpecialCase(CallNodeCR node)
        {
        auto instanceArg = GetInstanceCallArgument(GetCallNodePrefix(node), node);
        Append(FUNCTION_NAME_IsOfClass);
        StartArguments(node);
        Append(instanceArg);
        Comma();

        if (nullptr != node.GetArguments() && 2 == node.GetArguments()->GetArgumentCount())
            {
            NodeCP classNameArg = node.GetArguments()->GetArgument(0);
            NodeCP schemaNameArg = node.GetArguments()->GetArgument(1);
            Utf8String qualifiedClassName = schemaNameArg->ToString().Trim("\"");
            qualifiedClassName.append(":").append(classNameArg->ToString().Trim("\""));
            m_usedClasses.push_back(qualifiedClassName);
            }

        m_skipNextArgumentsOpeningBrace = true;
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleGetECClassIdSpecialCase(CallNodeCR node)
        {
        Append(FUNCTION_NAME_GetECClassId);
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    BentleyStatus ParseRelatedClassInfo(ArgumentTreeNodeCR args, RelatedClassInfo& info)
        {
        info.Direction = args.GetArgument(1)->ToString().Trim("\"");
        if (info.Direction.EqualsI("Forward"))
            {
            info.ThisColumn.InstanceIdColumnName = "SourceECInstanceId";
            info.ThisColumn.ClassIdColumnName = "SourceECClassId";
            info.RelatedColumn.InstanceIdColumnName = "TargetECInstanceId";
            info.RelatedColumn.ClassIdColumnName = "TargetECClassId";
            }
        else if (info.Direction.EqualsI("Backward"))
            {
            info.ThisColumn.InstanceIdColumnName = "TargetECInstanceId";
            info.ThisColumn.ClassIdColumnName = "TargetECClassId";
            info.RelatedColumn.InstanceIdColumnName = "SourceECInstanceId";
            info.RelatedColumn.ClassIdColumnName = "SourceECClassId";
            }
        else
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid relationship direction. Expected 'Forward' or 'Backward', got: '%s'", info.Direction.c_str()));
            return ERROR;
            }

        Utf8String relationshipSchemaAndClassName = args.GetArgument(0)->ToString().Trim("\"");
        if (ECObjectsStatus::Success != ECClass::ParseClassName(info.RelationshipNames.SchemaName, info.RelationshipNames.ClassName, relationshipSchemaAndClassName))
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Failed to parse relationship name: '%s'", relationshipSchemaAndClassName.c_str()));
            return ERROR;
            }

        Utf8String relatedSchemaAndClassName = args.GetArgument(2)->ToString().Trim("\"");
        if (ECObjectsStatus::Success != ECClass::ParseClassName(info.RelatedClassNames.SchemaName, info.RelatedClassNames.ClassName, relatedSchemaAndClassName))
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Failed to parse related class name: '%s'", relatedSchemaAndClassName.c_str()));
            return ERROR;
            }

        m_usedClasses.push_back(relationshipSchemaAndClassName);
        m_usedClasses.push_back(relatedSchemaAndClassName);
        return SUCCESS;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    BentleyStatus ParseJoinClassInfo(ArgumentTreeNodeCR args, JoinClassInfo& info)
        {
        Utf8String relatedSchemaAndClassName = args.GetArgument(0)->ToString().Trim("\"");
        if (ECObjectsStatus::Success != ECClass::ParseClassName(info.RelatedClassName.SchemaName, info.RelatedClassName.ClassName, relatedSchemaAndClassName))
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Failed to parse related class name: '%s'", relatedSchemaAndClassName.c_str()));
            return ERROR;
            }

        NodeCP secondArg = args.GetArgument(1);
        LambdaNodeCP lambda = dynamic_cast<LambdaNodeCP>(secondArg);
        if (nullptr == lambda)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, "Expecting second argument to be a lambda");
            return ERROR;
            }
        info.RelatedClassAlias = lambda->GetSymbolName();
        info.WhereClause = ECExpressionToECSqlConverter(m_fieldTypes, m_expressionContext).GetECSql(lambda->GetExpression());

        m_usedClasses.push_back(relatedSchemaAndClassName);
        return SUCCESS;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleHasRelatedInstanceSpecialCaseWith3Args(CallNodeCR node)
        {
        RelatedClassInfo info;
        if (SUCCESS != ParseRelatedClassInfo(*node.GetArguments(), info))
            return false;

        // TODO: remove '+' in WHERE clause when Sqlite fixes performance problem with EXISTS operator.
        static Utf8CP s_fmt = " EXISTS ("
            "SELECT 1 "
            "FROM [%s].[%s] relationship "
            "JOIN [%s].[%s] related ON [relationship].[%s] = [related].[ECClassId] AND [relationship].[%s] = [related].[ECInstanceId] "
            "WHERE [relationship].[%s] = +%s.[ECInstanceId]"
            ")";
        Utf8String prefix = GetCallNodePrefix(node);
        Utf8PrintfString ecsql(s_fmt,
            info.RelationshipNames.SchemaName.c_str(), info.RelationshipNames.ClassName.c_str(),
            info.RelatedClassNames.SchemaName.c_str(), info.RelatedClassNames.ClassName.c_str(),
            info.RelatedColumn.ClassIdColumnName.c_str(), info.RelatedColumn.InstanceIdColumnName.c_str(),
            info.ThisColumn.InstanceIdColumnName.c_str(), prefix.c_str());
        m_ecsql.append(ecsql);
        m_ignoreNextArguments = true;
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleHasRelatedInstanceSpecialCaseWith2Args(CallNodeCR node)
        {
        JoinClassInfo info;
        if (SUCCESS != ParseJoinClassInfo(*node.GetArguments(), info))
            return false;

        // call to remove call node prefix from m_ecsql
        GetCallNodePrefix(node);


        // TODO: remove '+' in WHERE clause when Sqlite fixes performance problem with EXISTS operator.
        static Utf8CP s_fmt = " EXISTS (SELECT 1 FROM [%s].[%s] [%s] WHERE +(%s))";
        m_ecsql.append(Utf8PrintfString(s_fmt, info.RelatedClassName.SchemaName.c_str(), info.RelatedClassName.ClassName.c_str(),
            info.RelatedClassAlias.c_str(), info.WhereClause.GetClause().c_str()));
        ContainerHelpers::Push(m_bindings, info.WhereClause.GetBindings());
        m_ignoreNextArguments = true;
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleHasRelatedInstanceSpecialCase(CallNodeCR node)
        {
        ArgumentTreeNodeCP args = node.GetArguments();
        if (3 == args->GetArgumentCount())
            return HandleHasRelatedInstanceSpecialCaseWith3Args(node);
        if (2 == args->GetArgumentCount())
            return HandleHasRelatedInstanceSpecialCaseWith2Args(node);
        DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid number of arguments. Expecting 2 or 3, got: %" PRIu64, args->GetArgumentCount()));
        return false;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleGetRelatedValueSpecialCaseWith4Args(CallNodeCR node)
        {
        RelatedClassInfo info;
        if (SUCCESS != ParseRelatedClassInfo(*node.GetArguments(), info))
            return false;

        Utf8String propertyName = node.GetArguments()->GetArgument(3)->ToString().Trim("\"");
        propertyName.ReplaceAll(".", "].["); // Needed to handle struct properties

        static Utf8CP s_fmt = "("
            "SELECT [related].[%s] "
            "FROM [%s].[%s] relationship, [%s].[%s] related "
            "WHERE [relationship].[%s] = %s.[ECClassId] AND [relationship].[%s] = %s.[ECInstanceId] "
            "AND [relationship].[%s] = [related].[ECClassId] AND [relationship].[%s] = [related].[ECInstanceId] "
            "LIMIT 1"
            ")";
        Utf8String prefix = GetCallNodePrefix(node);
        Utf8PrintfString ecsql(s_fmt,
            propertyName.c_str(),
            info.RelationshipNames.SchemaName.c_str(), info.RelationshipNames.ClassName.c_str(),
            info.RelatedClassNames.SchemaName.c_str(), info.RelatedClassNames.ClassName.c_str(),
            info.ThisColumn.ClassIdColumnName.c_str(), prefix.c_str(), info.ThisColumn.InstanceIdColumnName.c_str(), prefix.c_str(),
            info.RelatedColumn.ClassIdColumnName.c_str(), info.RelatedColumn.InstanceIdColumnName.c_str());
        m_ecsql.append(ecsql);
        m_ignoreNextArguments = true;
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleGetRelatedValueSpecialCaseWith3Args(CallNodeCR node)
        {
        JoinClassInfo info;
        if (SUCCESS != ParseJoinClassInfo(*node.GetArguments(), info))
            return false;

        // handle property name arg
        NodeCP thirdArg = node.GetArguments()->GetArgument(2);
        Utf8String propertyName = thirdArg->ToString().Trim("\"");
        propertyName.ReplaceAll(".", "].["); // needed to handle struct properties

        // call to remove call node prefix from m_ecsql
        GetCallNodePrefix(node);

        // create the clause
        static Utf8CP s_fmt = "(SELECT [%s].[%s] FROM [%s].[%s] [%s] WHERE %s LIMIT 1)";
        m_ecsql.append(Utf8PrintfString(s_fmt, info.RelatedClassAlias.c_str(), propertyName.c_str(),
            info.RelatedClassName.SchemaName.c_str(), info.RelatedClassName.ClassName.c_str(), info.RelatedClassAlias.c_str(),
            info.WhereClause.GetClause().c_str()));
        ContainerHelpers::Push(m_bindings, info.WhereClause.GetBindings());
        m_ignoreNextArguments = true;
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleGetRelatedValueSpecialCase(CallNodeCR node)
        {
        ArgumentTreeNodeCP args = node.GetArguments();
        if (4 == args->GetArgumentCount())
            return HandleGetRelatedValueSpecialCaseWith4Args(node);
        if (3 == args->GetArgumentCount())
            return HandleGetRelatedValueSpecialCaseWith3Args(node);
        DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid number of arguments. Expecting 3 or 4, got: %" PRIu64, args->GetArgumentCount()));
        return false;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleGetRelatedInstancesCountSpecialCaseWith3Args(CallNodeCR node)
        {
        RelatedClassInfo info;
        if (SUCCESS != ParseRelatedClassInfo(*node.GetArguments(), info))
            return false;

        static Utf8CP s_fmt = "("
            "SELECT COUNT(1) "
            "FROM [%s].[%s] relationship "
            "JOIN [%s].[%s] related "
            "ON [related].[ECClassId] = [relationship].[%s] AND [related].[ECInstanceId] = [relationship].[%s] "
            "WHERE [relationship].[%s] = %s.[ECClassId] AND [relationship].[%s] = %s.[ECInstanceId]"
            ")";
        Utf8String prefix = GetCallNodePrefix(node);
        Utf8PrintfString ecsql(s_fmt,
            info.RelationshipNames.SchemaName.c_str(), info.RelationshipNames.ClassName.c_str(),
            info.RelatedClassNames.SchemaName.c_str(), info.RelatedClassNames.ClassName.c_str(),
            info.RelatedColumn.ClassIdColumnName.c_str(), info.RelatedColumn.InstanceIdColumnName.c_str(),
            info.ThisColumn.ClassIdColumnName.c_str(), prefix.c_str(), info.ThisColumn.InstanceIdColumnName.c_str(), prefix.c_str());
        m_ecsql.append(ecsql);
        m_ignoreNextArguments = true;
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleGetRelatedInstancesCountSpecialCaseWith2Args(CallNodeCR node)
        {
        JoinClassInfo info;
        if (SUCCESS != ParseJoinClassInfo(*node.GetArguments(), info))
            return false;

        // call to remove call node prefix from m_ecsql
        GetCallNodePrefix(node);

        static Utf8CP s_fmt = "(SELECT COUNT(1) FROM [%s].[%s] [%s] WHERE %s)";
        m_ecsql.append(Utf8PrintfString(s_fmt, info.RelatedClassName.SchemaName.c_str(), info.RelatedClassName.ClassName.c_str(),
            info.RelatedClassAlias.c_str(), info.WhereClause.GetClause().c_str()));
        ContainerHelpers::Push(m_bindings, info.WhereClause.GetBindings());
        m_ignoreNextArguments = true;
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleGetRelatedInstancesCountSpecialCase(CallNodeCR node)
        {
        ArgumentTreeNodeCP args = node.GetArguments();
        if (3 == args->GetArgumentCount())
            return HandleGetRelatedInstancesCountSpecialCaseWith3Args(node);
        if (2 == args->GetArgumentCount())
            return HandleGetRelatedInstancesCountSpecialCaseWith2Args(node);
        DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid number of arguments. Expecting 2 or 3, got: %" PRIu64, args->GetArgumentCount()));
        return false;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleValueListMethodSpecialCase(CallNodeCR node)
        {
        m_currentValueListMethodNode = &node;
        m_ignoreNextArguments = true;
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleGetVariableIntValuesSpecialCase(CallNodeCR node)
        {
        ArgumentTreeNodeCP args = node.GetArguments();
        if (nullptr == args)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, "Missing arguments");
            return false;
            }
        if (1 != args->GetArgumentCount())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid number of arguments. Expecting 1, got: %" PRIu64, args->GetArgumentCount()));
            return false;
            }

        m_currentValueListMethodNode = &node;
        m_ignoreNextArguments = true;
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleAnyMatchSpecialCase(CallNodeCR node)
        {
        if (m_currentValueListMethodNode.IsNull() && m_currentValueListIdentNode.IsNull())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, "`AnyMatch` may only be used on value lists");
            return false;
            }

        m_skipNextArgumentsOpeningBrace = true;
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    static CallNodePtr CreateCallNode(Utf8CP functionName, NodeCR arg)
        {
        ArgumentTreeNodePtr args = new ArgumentTreeNode();
        args->PushArgument(const_cast<NodeR>(arg));
        return CallNode::Create(*args, functionName, false);
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleCompareDateTimesSpecialCase(CallNodeCR node)
        {
        ArgumentTreeNodeCP args = node.GetArguments();
        if (nullptr == args)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, "Missing arguments");
            return false;
            }
        if (2 != args->GetArgumentCount())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid number of arguments. Expecting 2, got: %" PRIu64, args->GetArgumentCount()));
            return false;
            }

        Append("(");
        NodePtr lhs = CreateCallNode("JULIANDAY", *args->GetArgument(0));
        NodePtr rhs = CreateCallNode("JULIANDAY", *args->GetArgument(1));
        Node::CreateArithmetic(ExpressionToken::TOKEN_Minus, *lhs, *rhs)->Traverse(*this);
        Append(")");

        m_ignoreNextArguments = true;
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleGetFormattedValueSpecialCase(CallNodeCR node)
        {
        ArgumentTreeNodeCP args = node.GetArguments();
        if (nullptr == args)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, "Missing arguments");
            return false;
            }
        if (1 != args->GetArgumentCount() && 2 != args->GetArgumentCount())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid number of arguments. Expecting 1 or 2, got: %" PRIu64, args->GetArgumentCount()));
            return false;
            }
        PrimaryListNodeCP listNode = dynamic_cast<PrimaryListNodeCP>(args->GetArgument(0));
        if (nullptr == listNode || 2 != listNode->GetNumberOfOperators())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("First argument should be property accessor, e.g. 'this.MyProperty'. Got: '%s'", args->GetArgument(0)->ToExpressionString().c_str()));
            return false;
            }
        IdentNodeCP ident = dynamic_cast<IdentNodeCP>(listNode->GetOperatorNode(0));
        DotNodeCP propertyAccessor = dynamic_cast<DotNodeCP>(listNode->GetOperatorNode(1));
        if (nullptr == ident || nullptr == propertyAccessor)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_WARNING, LOG_ERROR, Utf8PrintfString("First argument should be property accessor, e.g. 'this.MyProperty'. Got: '%s'", args->GetArgument(0)->ToExpressionString().c_str()));
            return false;
            }

        Append(FUNCTION_NAME_GetFormattedValue);
        StartArguments(node);
        m_skipNextArgumentsOpeningBrace = true;

        // append class id
        Append(Utf8PrintfString("[%s]", ident->GetName()));
        Append(".[ECClassId]");
        Comma();
        // append property name
        Append(Utf8PrintfString("'%s'", propertyAccessor->GetName()));
        Comma(); // TODO: is this valid? won't there be an excessive comma if there's only one argument?

        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleCallNode(CallNodeCR node)
        {
        static std::map<Utf8String, Utf8String> renames = {
            {"GetSettingValue", FUNCTION_NAME_GetVariableStringValue},
            {"GetSettingIntValue", FUNCTION_NAME_GetVariableIntValue},
            {"GetSettingIntValues", "GetVariableIntValues"},
            {"GetSettingBoolValue", FUNCTION_NAME_GetVariableBoolValue},
            {"HasSetting", FUNCTION_NAME_HasVariable},
        };

        // apply renames
        Utf8CP methodName = node.GetMethodName();
        auto renameIter = renames.find(methodName);
        if (renames.end() != renameIter)
            methodName = renameIter->second.c_str();

        CallNodePtr nodeCopy = CallNode::Create(const_cast<ArgumentTreeNodeR>(*node.GetArguments()), methodName, node.ToString().StartsWith("."));

        // handle special cases
        if (0 == strcmp("IsOfClass", nodeCopy->GetMethodName()) && HandleIsOfClassSpecialCase(*nodeCopy))
            return;
        if (0 == strcmp("GetECClassId", nodeCopy->GetMethodName()) && HandleGetECClassIdSpecialCase(*nodeCopy))
            return;
        if (0 == strcmp("HasRelatedInstance", nodeCopy->GetMethodName()) && HandleHasRelatedInstanceSpecialCase(*nodeCopy))
            return;
        if (0 == strcmp("GetRelatedInstancesCount", nodeCopy->GetMethodName()) && HandleGetRelatedInstancesCountSpecialCase(*nodeCopy))
            return;
        if (0 == strcmp("GetRelatedValue", nodeCopy->GetMethodName()) && HandleGetRelatedValueSpecialCase(*nodeCopy))
            return;
        if (0 == strcmp("GetVariableIntValues", nodeCopy->GetMethodName()) && HandleGetVariableIntValuesSpecialCase(*nodeCopy))
            return;
        if (0 == strcmp("Set", nodeCopy->GetMethodName()) && HandleValueListMethodSpecialCase(*nodeCopy))
            return;
        if ((0 == strcmp("AnyMatch", nodeCopy->GetMethodName()) || 0 == strcmp("AnyMatches", nodeCopy->GetMethodName())) && HandleAnyMatchSpecialCase(*nodeCopy))
            return;
        if (0 == strcmp("CompareDateTimes", nodeCopy->GetMethodName()) && HandleCompareDateTimesSpecialCase(*nodeCopy))
            return;
        if (0 == strcmp("GetFormattedValue", nodeCopy->GetMethodName()) && HandleGetFormattedValueSpecialCase(*nodeCopy))
            return;

        Append(nodeCopy->ToString());
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bvector<ECValue> EvaluateValueListSymbols(PrimaryListNodeR valueListNode, PrimaryListNodeR symbolValueNode)
        {
        bvector<ECValue> result;
        if (!m_shouldEvaluateValueLists)
            return result;
        ValueResultPtr valueListResult;
        if (ExpressionStatus::Success == valueListNode.GetValue(valueListResult, *m_expressionContext) && valueListResult->GetResult().GetValueList())
            {
            EvaluationResult itemValue;
            for (uint32_t i = 0; i < valueListResult->GetResult().GetValueList()->GetCount(); ++i)
                {
                valueListResult->GetResult().GetValueList()->GetValueAt(itemValue, i);
                if (itemValue.GetContext())
                    {
                    EvaluationResult temp;
                    if (ExpressionStatus::Success == itemValue.GetContext()->GetValue(temp, symbolValueNode, {}, 1))
                        itemValue = temp;
                    }
                if (itemValue.GetECValue())
                    {
                    result.push_back(*itemValue.GetECValue());
                    }
                else
                    {
                    DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_WARNING, LOG_ERROR, Utf8PrintfString("Unexpected element type when evaluating a value list: '%s'", valueListNode.ToExpressionString().c_str()));
                    }
                }
            }
        else
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_WARNING, LOG_ERROR, Utf8PrintfString("Failed to evaluate a value list: '%s'", valueListNode.ToExpressionString().c_str()));
            }
        return result;
        }
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bvector<ECValue> EvaluateValueListSymbols(IdentNodeCR valueListNode, PrimaryListNodeCR symbolValueNode)
        {
        auto primaryList = PrimaryListNode::Create();
        primaryList->AppendNameNode(const_cast<IdentNodeR>(valueListNode));
        return EvaluateValueListSymbols(*primaryList, const_cast<PrimaryListNodeR>(symbolValueNode));
        }
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bvector<ECValue> EvaluateValueListSymbols(CallNodeCR valueListNode, PrimaryListNodeCR symbolValueNode)
        {
        auto primaryList = PrimaryListNode::Create();
        primaryList->AppendCallNode(const_cast<CallNodeR>(valueListNode));
        return EvaluateValueListSymbols(*primaryList, const_cast<PrimaryListNodeR>(symbolValueNode));
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleLambdaEqualityExpression(ComparisonNodeCR expression, Utf8CP symbolName)
        {
        if (!m_expressionContext && m_shouldEvaluateValueLists)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Failed to evaluate lambda equality expression - expression context is not set up");

        PrimaryListNodeCP propertyAccessNode = nullptr;
        PrimaryListNodeCP symbolValueNode = nullptr;
        Utf8String leftExpression = expression.GetLeftCP()->ToExpressionString();
        Utf8String rightExpression = expression.GetRightCP()->ToExpressionString();
        if (leftExpression.Equals(symbolName) || leftExpression.StartsWith(Utf8PrintfString("%s.", symbolName).c_str()))
            {
            symbolValueNode = dynamic_cast<PrimaryListNodeCP>(expression.GetLeftCP());
            propertyAccessNode = dynamic_cast<PrimaryListNodeCP>(expression.GetRightCP());
            }
        else if (rightExpression.Equals(symbolName) || rightExpression.StartsWith(Utf8PrintfString("%s.", symbolName).c_str()))
            {
            symbolValueNode = dynamic_cast<PrimaryListNodeCP>(expression.GetRightCP());
            propertyAccessNode = dynamic_cast<PrimaryListNodeCP>(expression.GetLeftCP());
            }

        if (!symbolValueNode || !propertyAccessNode)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_WARNING, LOG_ERROR, Utf8PrintfString("Failed to evaluate lambda equality expression - expecting either "
                "LHS or RHS to start with lambda symbol.Got LHS: '%s', RHS: '%s'", leftExpression.c_str(), rightExpression.c_str()));
            return;
            }

        bvector<ECValue> values;
        if (m_currentValueListMethodNode.IsValid())
            values = EvaluateValueListSymbols(*m_currentValueListMethodNode, *symbolValueNode);
        else if (m_currentValueListIdentNode.IsValid())
            values = EvaluateValueListSymbols(*m_currentValueListIdentNode, *symbolValueNode);
        else
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Lambda method should only be on value lists");

        if (values.empty())
            {
            Append("FALSE");
            }
        else
            {
            auto propertyAccessNodeECSql = ECExpressionToECSqlConverter(m_fieldTypes, m_expressionContext, m_shouldEvaluateValueLists).GetECSql(*propertyAccessNode);
            DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, propertyAccessNodeECSql.GetBindings().empty(),
                Utf8PrintfString("Expected 0 bindings for property access node, got: %" PRIu64 ". Node: `%s`, clause: `%s`.",
                    propertyAccessNode->ToExpressionString().c_str(), propertyAccessNodeECSql.GetClause().c_str()));

            ValuesFilteringHelper valuesHelper(values);
            auto clauseAndBindings = valuesHelper.Create(propertyAccessNodeECSql.GetClause().c_str(), false);
            Append(clauseAndBindings.GetClause());
            ContainerHelpers::MovePush(m_bindings, clauseAndBindings.GetBindings());
            }

        m_currentValueListMethodNode = nullptr;
        m_currentValueListIdentNode = nullptr;
        m_inArguments++;
        m_ignoreArguments++;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleLambdaFunctionCallExpression(NodeCP instanceNode, CallNodeCR callNode, Utf8CP symbolName)
        {
        NodeCP symbolArgumentNode = nullptr;
        size_t symbolArgumentIndex = 0;
        for (size_t i = 0; i < callNode.GetArguments()->GetArgumentCount(); ++i)
            {
            auto const& argumentNode = callNode.GetArguments()->GetArgument(i);
            auto argumentExpression = argumentNode->ToExpressionString();
            if (argumentExpression.Equals(symbolName) || argumentExpression.StartsWith(Utf8PrintfString("%s.", symbolName).c_str()))
                {
                symbolArgumentNode = argumentNode;
                symbolArgumentIndex = i;
                break;
                }
            }

        if (!symbolArgumentNode || !dynamic_cast<PrimaryListNodeCP>(symbolArgumentNode))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Lambda expression function doesn't use lambda symbol");

        bvector<ECValue> values;
        if (m_currentValueListMethodNode.IsValid())
            values = EvaluateValueListSymbols(*m_currentValueListMethodNode, *static_cast<PrimaryListNodeCP>(symbolArgumentNode));
        else if (m_currentValueListIdentNode.IsValid())
            values = EvaluateValueListSymbols(*m_currentValueListIdentNode, *static_cast<PrimaryListNodeCP>(symbolArgumentNode));
        else
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Lambda method should only be on value lists");

        if (values.empty())
            {
            Append("FALSE");
            return;
            }

        bool first = true;
        Append("(");
        for (ECValueCR value : values)
            {
            if (!first)
                Append(" OR", false);
            first = false;

            Append(callNode.GetMethodName());
            Append("(", false);
            if (instanceNode)
                Append(GetInstanceCallArgument(instanceNode->ToExpressionString(), callNode));
            for (size_t i = 0; i < callNode.GetArguments()->GetArgumentCount(); ++i)
                {
                if (i > 0 || instanceNode)
                    Append(",", false);

                if (i == symbolArgumentIndex)
                    {
                    Append("?");
                    continue;
                    }

                auto const& argumentNode = callNode.GetArguments()->GetArgument(i);
                argumentNode->Traverse(*this);
                HandleScopeEnd();
                }
            Append(")", false);
            m_bindings.push_back(std::make_unique<BoundQueryECValue>(value));
            }
        Append(")", false);

        m_currentValueListMethodNode = nullptr;
        m_currentValueListIdentNode = nullptr;
        m_inArguments++;
        m_ignoreArguments++;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleLambdaNode(LambdaNodeCR node)
        {
        if (m_currentValueListMethodNode.IsNull() && m_currentValueListIdentNode.IsNull())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Lambda method should only be on value lists");

        Utf8CP symbolName = node.GetSymbolName();

        auto expressionNode = dynamic_cast<ComparisonNodeCP>(&node.GetExpression());
        if (nullptr != expressionNode && TOKEN_Equal == expressionNode->GetOperation())
            {
            HandleLambdaEqualityExpression(*expressionNode, symbolName);
            return;
            }

        auto primaryListNode = dynamic_cast<PrimaryListNodeCP>(&node.GetExpression());
        if (primaryListNode && primaryListNode->GetNumberOfOperators() == 1)
            {
            auto primaryListOperator = primaryListNode->GetOperatorNode(0);
            if (primaryListOperator && TOKEN_LParen == primaryListOperator->GetOperation() && dynamic_cast<CallNodeCP>(primaryListOperator))
                {
                HandleLambdaFunctionCallExpression(nullptr, *static_cast<CallNodeCP>(primaryListOperator), symbolName);
                return;
                }
            }
        else if (primaryListNode && primaryListNode->GetNumberOfOperators() == 2)
            {
            auto firstOperator = primaryListNode->GetOperatorNode(0);
            auto firstOperatorExpr = firstOperator->ToExpressionString();
            auto secondOperator = primaryListNode->GetOperatorNode(1);
            if ((firstOperatorExpr.Equals("ThisNode") || firstOperatorExpr.Equals("this")) && TOKEN_LParen == secondOperator->GetOperation() && dynamic_cast<CallNodeCP>(secondOperator))
                {
                HandleLambdaFunctionCallExpression(firstOperator, *static_cast<CallNodeCP>(secondOperator), symbolName);
                return;
                }
            }

        DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, "Only an EQUALS operation or function calls are allowed in lambda expression");
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleDotNode(DotNodeCR node)
        {
        m_inStructProperty++;
        Append(".");
        Append("[");
        Append(node.GetName());
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandlePrimaryListNode(PrimaryListNodeCR node)
        {
        if (!m_expressionContext)
            return;

        EvaluationResult evalResult;
        if (ExpressionStatus::Success != const_cast<PrimaryListNodeR>(node).GetValue(evalResult, *m_expressionContext))
            return;

        if (!evalResult.GetECValue())
            return;

        if (VALUEKIND_Primitive != evalResult.GetECValue()->GetKind())
            return;

        // successfuly evaluated node, ignore it's operators
        for (size_t i = 0; i < node.GetNumberOfOperators(); ++i)
            m_ignoredNodes.insert(node.GetOperatorNode(i));

        if (evalResult.GetECValue()->IsBoolean())
            {
            Append(evalResult.GetECValue()->GetBoolean() ? "TRUE" : "FALSE");
            return;
            }

        Append("?");
        m_bindings.push_back(std::make_unique<BoundQueryECValue>(*evalResult.GetECValue()));
        return;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleConditionNode(IIfNodeCR node)
        {
        Append(node.ToString());
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleEqualtyNode(ComparisonNodeCR node)
        {
        Append(node.ToString());

        Utf8String left = node.GetLeftCP()->ToExpressionString();
        if (left.EndsWith(".ClassName"))
            m_usedClasses.push_back(node.GetRightCP()->ToString().Trim("\""));
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleStringConstNode(LiteralNode const& node)
        {
        Utf8String value = node.GetInternalValue().GetUtf8CP();
        if (value.Equals(ContentDescriptor::DisplayLabelField::NAME))
            {
            // note: name of display label field contains invalid characters, so we wrap it
            // with quotes to make it look like a string - see Preprocess method
            Append(QueryHelpers::Wrap(value));
            m_previousToken = TOKEN_Ident;
            HandleScopeEnd();
            return;
            }

        value.ReplaceAll("'", "''");
        Append(Utf8PrintfString("'%s'", value.c_str()));

        if (m_nodesStack.size() > 1 && std::string::npos != (m_nodesStack.end() - 2)->find("LIKE"))
            Append("ESCAPE \'\\\'");
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool IsNodeIgnored(NodeCR node)
        {
        auto iter = m_ignoredNodes.find(&node);
        if (iter != m_ignoredNodes.end())
            {
            if (node.GetOperation() == TOKEN_LParen)
                m_ignoreNextArguments = true;
            m_ignoredNodes.erase(iter);
            return true;
            }
        return false;
        }

public:
    bool StartArrayIndex(NodeCR) override {DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_WARNING, LOG_ERROR, "Array indexes not supported"); return false;}
    bool EndArrayIndex(NodeCR) override {DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_WARNING, LOG_ERROR, "Array indexes not supported"); return false;}
    bool StartArguments(NodeCR) override
        {
        if (m_ignoreNextArguments)
            {
            m_ignoreArguments++;
            m_ignoreNextArguments = false;
            }
        else if (m_ignoreArguments)
            {
            m_ignoreArguments++;
            }
        else
            {
            if (!m_skipNextArgumentsOpeningBrace)
                Append("(", false);
            else
                m_skipNextArgumentsOpeningBrace = false;
            }
        m_inArguments++;
        return true;
        }
    bool EndArguments(NodeCR) override
        {
        if (m_ignoreArguments)
            {
            m_ignoreArguments--;
            }
        else
            {
            HandleScopeEnd();
            if (m_inArguments)
                Append(")");
            }
        if (m_inArguments)
            m_inArguments--;

        // create an aggregated arguments' string and append it to the
        // call node so it represents the whole call node
        Utf8String argsString;
        int pos = (int)m_nodesStack.size() - 1;
        bool collectedArgs = false;
        while (pos >= 0)
            {
            if (!collectedArgs)
                {
                argsString.insert(0, m_nodesStack[pos]);
                if (m_nodesStack[pos] == "(")
                    collectedArgs = true;
                }
            else
                {
                m_nodesStack[pos].append(argsString);
                m_nodesStack.erase(m_nodesStack.begin() + pos + 1, m_nodesStack.end());
                break;
                }
            --pos;
            }

        return true;
        }
    bool Comma() override
        {
        ARGUMENTS_PRECONDITION();
        if (m_inArguments)
            HandleScopeEnd();
        Append(",");
        return true;
        }
    bool OpenParens() override {ARGUMENTS_PRECONDITION(); Append("("); return true;}
    bool CloseParens() override {ARGUMENTS_PRECONDITION(); HandleScopeEnd(); Append(")"); return true;}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool ProcessNode(NodeCR node) override
        {
        ARGUMENTS_PRECONDITION();

        if (IsNodeIgnored(node))
            return true;

        HandleScopeEnd();
        switch (node.GetOperation())
            {
            case TOKEN_Null:
                HandleNullNode();
                break;
            case TOKEN_And:
            case TOKEN_AndAlso:
                Append("AND");
                break;
            case TOKEN_Or:
            case TOKEN_OrElse:
                Append("OR");
                break;
            case TOKEN_Not:
                Append("NOT");
                break;
            case TOKEN_False:
                Append("FALSE");
                break;
            case TOKEN_True:
                Append("TRUE");
                break;
            case TOKEN_Like:
                HandleLikeToken(node);
                break;
            case TOKEN_Concatenate:
                Append("||");
                break;
            case TOKEN_IntegerConstant:
            case TOKEN_FloatConst:
            case TOKEN_PointConst:
            case TOKEN_Greater:
            case TOKEN_GreaterEqual:
            case TOKEN_Less:
            case TOKEN_LessEqual:
            case TOKEN_Minus:
            case TOKEN_Plus:
            case TOKEN_Star:
            case TOKEN_Slash:
                Append(node.ToString());
                break;
            case TOKEN_Equal:
            case TOKEN_NotEqual:
                if (nullptr == dynamic_cast<ComparisonNodeCP>(&node))
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Got TOKEN_Equal or TOKEN_NotEqual, but Node is not a ComparisonNode");
                HandleEqualtyNode(static_cast<ComparisonNodeCR>(node));
                break;
            case TOKEN_StringConst:
                if (nullptr == dynamic_cast<LiteralNode const*>(&node))
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Got TOKEN_StringConst, but Node is not a LiteralNode");
                HandleStringConstNode(static_cast<LiteralNode const&>(node));
                break;
            case TOKEN_DateTimeConst:
                Append(Utf8PrintfString("DATE '%s'", node.ToString().c_str()));
                break;
            case TOKEN_Ident:
                if (nullptr == dynamic_cast<IdentNodeCP>(&node))
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Got TOKEN_Ident, but Node is not a IdentNode");
                HandleIdentNode(static_cast<IdentNodeCR>(node));
                break;
            case TOKEN_LParen:
                if (nullptr == dynamic_cast<CallNodeCP>(&node))
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Got TOKEN_LParen, but Node is not a CallNode");
                HandleCallNode(static_cast<CallNodeCR>(node));
                break;
            case TOKEN_Dot:
                if (nullptr == dynamic_cast<DotNodeCP>(&node))
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Got TOKEN_Dot, but Node is not a DotNode");
                HandleDotNode(static_cast<DotNodeCR>(node));
                break;
            case TOKEN_IIf:
                if (nullptr == dynamic_cast<IIfNodeCP>(&node))
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Got TOKEN_IIf, but Node is not a IIfNode");
                HandleConditionNode(static_cast<IIfNodeCR>(node));
                break;
            case TOKEN_Lambda:
                if (nullptr == dynamic_cast<LambdaNodeCP>(&node))
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Got TOKEN_Lambda, but Node is not a LambdaNode");
                HandleLambdaNode(static_cast<LambdaNodeCR>(node));
                break;
            case TOKEN_PrimaryList:
                if (nullptr == dynamic_cast<PrimaryListNodeCP>(&node))
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, "Got TOKEN_PrimaryList, but Node is not a PrimaryListNode");
                HandlePrimaryListNode(static_cast<PrimaryListNodeCR>(node));
                break;
            default:
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::ECExpressions, Utf8PrintfString("Unhandled ECExpression token: %d", (int)node.GetOperation()));
            }

        m_previousToken = node.GetOperation();
        return true;
        }

    void Reset()
        {
        m_ecsql.clear();
        m_bindings.clear();
        m_usedClasses.clear();
        }

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    ECExpressionToECSqlConverter(IPresentationQueryFieldTypesProvider const* fieldTypes, ExpressionContext* expressionContext, bool evaluateValueLists = true)
        : m_inArguments(0), m_inStructProperty(0), m_ignoreArguments(0), m_previousToken(TOKEN_Unrecognized), m_shouldEvaluateValueLists(evaluateValueLists),
        m_skipNextArgumentsOpeningBrace(false), m_ignoreNextArguments(false), m_fieldTypes(fieldTypes), m_expressionContext(expressionContext)
        {}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    static void Preprocess(Utf8StringR expr)
        {
        expr.ReplaceAll(ContentDescriptor::DisplayLabelField::NAME, Utf8String(ContentDescriptor::DisplayLabelField::NAME).AddQuotes().c_str());
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    QueryClauseAndBindings GetECSql(NodeCR node)
        {
        Reset();
        node.Traverse(*this);
        HandleScopeEnd();
        return QueryClauseAndBindings(m_ecsql, m_bindings);
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+--*/
    bvector<Utf8String> GetUsedClasses(NodeCR node)
        {
        Reset();
        node.Traverse(*this);
        HandleScopeEnd();
        return m_usedClasses;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QueryClauseAndBindings ECExpressionsHelper::ConvertToECSql(Utf8StringCR expression, IPresentationQueryFieldTypesProvider const* fieldTypes, ExpressionContext* expressionContext)
    {
    if (expression.empty())
        return QueryClauseAndBindings();

    Utf8String copy(expression);
    ECExpressionToECSqlConverter::Preprocess(copy);

    NodePtr node = GetNodeFromExpression(copy.c_str());
    if (node.IsNull())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Failed to parse ECExpression: '%s'", expression.c_str()));
        return QueryClauseAndBindings();
        }

    ECExpressionToECSqlConverter converter(fieldTypes, expressionContext);
    return converter.GetECSql(*node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> ECExpressionsHelper::GetUsedClasses(Utf8StringCR expression)
    {
    static const bvector<Utf8String> empty;
    if (expression.empty())
        return empty;

    bvector<Utf8String> const* cachedClasses = m_cache.GetUsedClasses(expression.c_str());
    if (nullptr != cachedClasses)
        return *cachedClasses;

    NodePtr node = GetNodeFromExpression(expression.c_str());
    if (node.IsNull())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_INFO, LOG_ERROR, Utf8PrintfString("Failed to parse ECExpression: '%s'", expression.c_str()));
        return empty;
        }

    ECExpressionToECSqlConverter converter(nullptr, nullptr, false);
    bvector<Utf8String> classes = converter.GetUsedClasses(*node);
    m_cache.Add(expression.c_str(), classes);
    return *m_cache.GetUsedClasses(expression.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsNextNonWhiteSpaceAlphaNum(Utf8StringCR expression, size_t currentIndex)
    {
    for (size_t i = currentIndex + 1; i < expression.size(); ++i)
        {
        if (std::isspace(expression[i]))
            continue;

        return std::isalnum(expression[i]);
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void RemoveExpressionWhitespaces(Utf8StringR expression)
    {
    bool inQuatation = false; // To ignore whitespaces between quatation marks
    bool lastAlphaNum = false;
    bool nextAlphaNum = false;
    bool lastWSpace = false;
    size_t currentIndex = 0;
    auto removedItems = std::remove_if
    (
        expression.begin(),
        expression.end(),
        [&inQuatation, &lastAlphaNum, &nextAlphaNum, &expression, &currentIndex, &lastWSpace](Utf8Char c)
            {
            if ('\"' == c)
                inQuatation = !inQuatation;

            bool isWSpace = std::isspace(c);
            bool isAlphaNum = std::isalnum(c);

            if (isWSpace != lastWSpace) // Recheck next alpha num only if last one has changed for optimization
                nextAlphaNum = IsNextNonWhiteSpaceAlphaNum(expression, currentIndex);
            bool remove = isWSpace && !inQuatation && !(lastAlphaNum && nextAlphaNum); // Remove whitespaces that are not between alphanum symbols

            lastAlphaNum = isAlphaNum ? true : (isWSpace ? lastAlphaNum : false);
            lastWSpace = isWSpace;

            ++currentIndex;
            return remove;
            }
    );

    expression.erase(removedItems, expression.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr ECExpressionsHelper::GetNodeFromExpression(Utf8CP expr)
    {
    NodePtr node;
    if (SUCCESS == m_cache.Get(node, expr))
        return node;

    Utf8String expression = expr;

    // Remove excessive whitespaces for optimizations below to work
    RemoveExpressionWhitespaces(expression);

    // check node's class instead of instance's class - this allows us to avoid retrieving the ECInstance
    // if we're only checking for class.
    expression.ReplaceAll(".ECInstance.IsOfClass(", ".IsOfClass(");
    expression.ReplaceAll("this.IsOfClass(", "ThisNode.IsOfClass(");

    // LIKE operator special case
    expression.ReplaceAll("~", " LIKE ");

    node = ECEvaluator::ParseValueExpressionAndCreateTree(expression.c_str());
    m_cache.Add(expr, node);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECExpressionsCache::Get(NodePtr& node, Utf8CP expression) const
    {
    BeMutexHolder lock(m_mutex);
    auto iter = m_cache.find(expression);
    if (m_cache.end() == iter)
        return ERROR;
    node = iter->second;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECExpressionsCache::Get(OptimizedExpressionPtr& optimizedExpr, Utf8CP expression) const
    {
    BeMutexHolder lock(m_mutex);
    auto iter = m_optimizedCache.find(expression);
    if (m_optimizedCache.end() == iter)
        return ERROR;
    optimizedExpr = iter->second;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> const* ECExpressionsCache::GetUsedClasses(Utf8CP expression) const
    {
    BeMutexHolder lock(m_mutex);
    auto iter = m_usedClasses.find(expression);
    if (m_usedClasses.end() == iter)
        return nullptr;
    return &iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECExpressionsCache::HasOptimizedExpression(Utf8CP expression) const
    {
    BeMutexHolder lock(m_mutex);
    return m_optimizedCache.end() != m_optimizedCache.find(expression);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECExpressionsCache::Add(Utf8CP expression, NodePtr node)
    {
    BeMutexHolder lock(m_mutex);
    m_cache.Insert(expression, node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECExpressionsCache::Add(Utf8CP expression, OptimizedExpressionPtr node)
    {
    BeMutexHolder lock(m_mutex);
    m_optimizedCache.Insert(expression, node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> const& ECExpressionsCache::Add(Utf8CP expression, bvector<Utf8String>& classes)
    {
    BeMutexHolder lock(m_mutex);
    return m_usedClasses.Insert(expression, classes).first->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECExpressionsCache::Clear()
    {
    BeMutexHolder lock(m_mutex);
    m_cache.clear();
    m_optimizedCache.clear();
    }
