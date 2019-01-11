/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ECExpressionContextsProvider.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "ECExpressionContextsProvider.h"
#include "ExtendedData.h"
#include <ECPresentation/ECPresentation.h>
#include <ECObjects/ECExpressions.h>
#include <ECObjects/ECExpressionNode.h>
#include <ECObjects/SystemSymbolProvider.h>
#include "ECSchemaHelper.h"
#include "CustomFunctions.h"
#include "NavNodesCache.h"

//=======================================================================================
// @bsiclass                                                Grigas.Petraitis    01/2017
//=======================================================================================
struct ProviderContext
    {
    virtual ~ProviderContext() {}
    };

//=======================================================================================
// @bsiclass                                                Grigas.Petraitis    02/2018
//=======================================================================================
struct CommonRulesEngineSymbolsProvider : IECSymbolProvider
{
private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                02/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ExpressionStatus CreateSet(EvaluationResult& evalResult, void*, EvaluationResultVector& args)
        {
        bvector<EvaluationResult> values;
        for (EvaluationResultCR arg : args)
            {
            if (!arg.IsECValue())
                {
                BeAssert(false);
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
// @bsiclass                                                Grigas.Petraitis    01/2017
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
    ExpressionStatus _ResolveMethod(MethodReferencePtr& result, Utf8CP ident, bool useOuterIfNecessary) override {return m_internalContext->ResolveMethod(result, ident, useOuterIfNecessary);}
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
// @bsiclass                                                Grigas.Petraitis    11/2016
//=======================================================================================
struct ECInstanceContextEvaluator : PropertySymbol::ContextEvaluator
{
private:
    IConnectionCR m_connection;
    ECInstanceKey m_key;
    ECInstanceContextEvaluator(IConnectionCR connection, ECInstanceKey key) : m_connection(connection), m_key(key) {}
public:
    static RefCountedPtr<ECInstanceContextEvaluator> Create(IConnectionCR connection, ECInstanceKey key) {return new ECInstanceContextEvaluator(connection, key);}
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
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ECInstanceContextEvaluator: Invalid key");
            BeAssert(false);
            }
        if (instance.IsValid())
            instanceContext->SetInstance(*instance);
        return instanceContext;
        }
};

//=======================================================================================
// @bsiclass                                                Grigas.Petraitis    12/2015
//=======================================================================================
struct NodeECInstanceContextEvaluator : PropertySymbol::ContextEvaluator
{
private:
    IConnectionCR m_connection;
    NavNodeCPtr m_node;
    NodeECInstanceContextEvaluator(IConnectionCR connection, NavNodeCR node) : m_connection(connection), m_node(&node) {}
public:
    static RefCountedPtr<NodeECInstanceContextEvaluator> Create(IConnectionCR connection, NavNodeCR node) {return new NodeECInstanceContextEvaluator(connection, node);}
    ExpressionContextPtr _GetContext() override
        {
        ECInstanceNodeKey const* key = m_node->GetKey()->AsECInstanceNodeKey();
        if (nullptr == key)
            return nullptr;

        InstanceExpressionContextPtr instanceContext = InstanceExpressionContext::Create(nullptr);
        IECInstancePtr instance;
        ECInstancesHelper::LoadInstance(instance, m_connection, key->GetInstanceKey());
        if (instance.IsValid())
            instanceContext->SetInstance(*instance);
        return instanceContext;
        }
};

//=======================================================================================
// @bsiclass                                                Grigas.Petraitis    12/2015
//=======================================================================================
struct ECInstanceMethodSymbolsProvider : IECSymbolProvider
{
private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Paul.Connelly   08/12
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ExpressionStatus IsOfClass (EvaluationResult& evalResult, void*, ECInstanceListCR instanceData, EvaluationResultVector& args)
        {
        if (2 != args.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("ECInstanceMethodSymbolsProvider::IsOfClass: WrongNumberOfArguments. Expected 2, actually: %" PRIu64, (uint64_t)args.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }          
        
        IECInstancePtr instance;
        Utf8CP schemaname, classname;
        if (!SystemSymbolProvider::ExtractArg(classname, args[0])|| !SystemSymbolProvider::ExtractArg(schemaname, args[1]))
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ECInstanceMethodSymbolsProvider::IsOfClass class name or schema name is not a string");
            return ExpressionStatus::UnknownError;
            }          

        bool found = false;
        for (IECInstancePtr const& instance: instanceData)
            {
            found = instance->GetClass().Is(schemaname, classname);
            if (found)
                break;
            }

        evalResult.InitECValue().SetBoolean(found);
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("ECInstanceMethodSymbolsProvider::IsOfClass: Result: %s", evalResult.ToString().c_str()).c_str());
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
// @bsiclass                                                Grigas.Petraitis    03/2015
//=======================================================================================
struct NodeSymbolsProvider : IECSymbolProvider
{
    struct Context : ProviderContext
        {
        IConnectionCR m_connection;
        NavNodeCPtr m_node;
        Context(IConnectionCR connection, NavNodeCP node) : m_connection(connection), m_node(node) {}
        };

private:
    Context const& m_context;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                01/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ExpressionStatus IsOfClass(EvaluationResult& evalResult, void* context, EvaluationResultVector& args)
        {
        if (2 != args.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("NodeSymbolsProvider::IsOfClass: WrongNumberOfArguments. Expected 2, actually: %" PRIu64, (uint64_t)args.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (nullptr == context)
            {
            evalResult.InitECValue().SetBoolean(false);
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, "NodeSymbolsProvider::IsOfClass: Result: false (context == nullptr)" );
            return ExpressionStatus::Success;
            }

        Context const& ctx = *(Context*)context;
        if (ctx.m_node.IsNull() || !ctx.m_connection.IsOpen())
            {
            evalResult.InitECValue().SetBoolean(false);
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, "NodeSymbolsProvider::IsOfClass: Result: false (node is null or connection is closed)");
            return ExpressionStatus::Success;
            }

        Utf8CP schemaname, classname;
        if (!SystemSymbolProvider::ExtractArg(classname, args[0]) || !SystemSymbolProvider::ExtractArg(schemaname, args[1]))
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "NodeSymbolsProvider::IsOfClass: UnknownError. Invalid class name or schema name");
            return ExpressionStatus::UnknownError;
            }

        NavNodeExtendedData extendedData(*ctx.m_node);
        if (!extendedData.HasECClassId())
            {
            evalResult.InitECValue().SetBoolean(false);
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, "NodeSymbolsProvider::IsOfClass: Result: false (node does not have class id)");
            return ExpressionStatus::Success;
            }
        
        ECClassCP nodeClass = ctx.m_connection.GetECDb().Schemas().GetClass(extendedData.GetECClassId());
        if (nullptr == nodeClass)
            {
            BeAssert(false);
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "NodeSymbolsProvider::IsOfClass: UnkonwnError (node class not found)");
            return ExpressionStatus::UnknownError;
            }
        
        evalResult.InitECValue().SetBoolean(nodeClass->Is(schemaname, classname));
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("NodeSymbolsProvider::IsOfClass: (%s is of class %s:%s) = %s", nodeClass->GetFullName(), schemaname, classname, evalResult.ToString().c_str()).c_str());
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
            context.AddSymbol(*ValueSymbol::Create("SchemaName", ECValue("", false)));
            context.AddSymbol(*ValueSymbol::Create("SchemaMajorVersion", ECValue(0)));
            context.AddSymbol(*ValueSymbol::Create("SchemaMinorVersion", ECValue(0)));
            context.AddSymbol(*ValueSymbol::Create("InstanceId", ECValue("", false)));
            context.AddSymbol(*ValueSymbol::Create("IsInstanceNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsClassNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsRelationshipClassNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("ParentClassName", ECValue("", false)));
            context.AddSymbol(*ValueSymbol::Create("ParentSchemaName", ECValue("", false)));
            context.AddSymbol(*ValueSymbol::Create("RelationshipDirection", ECValue("", false)));
            context.AddSymbol(*ValueSymbol::Create("IsSchemaNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsSearchNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsClassGroupingNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsPropertyGroupingNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("GroupedInstancesCount", ECValue(0)));
            context.AddSymbol(*ValueSymbol::Create("ECInstance", ECValue()));
            }
        else
            {
            NavNodeCR node = *m_context.m_node;
            NavNodeExtendedData nodeExtendedData(node);

            ECClassCP nodeClass = nullptr;
            if (nodeExtendedData.HasECClassId())
                nodeClass = m_context.m_connection.GetECDb().Schemas().GetClass(nodeExtendedData.GetECClassId());

            context.AddSymbol(*ValueSymbol::Create("IsNull", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("Type", ECValue(node.GetType().c_str(), false)));
            context.AddSymbol(*ValueSymbol::Create("Label", ECValue(node.GetLabel().c_str(), false)));
            context.AddSymbol(*ValueSymbol::Create("Description", ECValue(node.GetDescription().c_str(), false)));
            context.AddSymbol(*ValueSymbol::Create("ClassName", nullptr != nodeClass ? ECValue(nodeClass->GetName().c_str(), false) : ECValue()));
            context.AddSymbol(*ValueSymbol::Create("SchemaName", nullptr != nodeClass ? ECValue(nodeClass->GetSchema().GetName().c_str(), true) : ECValue()));
            context.AddSymbol(*ValueSymbol::Create("SchemaMajorVersion", nullptr != nodeClass ? ECValue((int)nodeClass->GetSchema().GetVersionRead()) : ECValue(0)));
            context.AddSymbol(*ValueSymbol::Create("SchemaMinorVersion", nullptr != nodeClass ? ECValue((int)nodeClass->GetSchema().GetVersionMinor()) : ECValue(0)));
            context.AddSymbol(*ValueSymbol::Create("IsClassNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsRelationshipClassNode", ECValue(node.GetType().Equals(NAVNODE_TYPE_ECRelationshipGroupingNode))));
            context.AddSymbol(*ValueSymbol::Create("IsSchemaNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsSearchNode", ECValue(false)));
            context.AddSymbol(*ValueSymbol::Create("IsClassGroupingNode", ECValue(node.GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode))));
            context.AddSymbol(*ValueSymbol::Create("IsPropertyGroupingNode", ECValue(node.GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode) || node.GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode))));
            context.AddSymbol(*ValueSymbol::Create("GroupedInstancesCount", ECValue((uint64_t)nodeExtendedData.GetGroupedInstanceKeysCount())));

            if (node.GetType().Equals(NAVNODE_TYPE_ECRelationshipGroupingNode))
                {
                BeAssert(nodeExtendedData.HasParentECClassId() && nodeExtendedData.HasRelationshipDirection());

                Utf8String relationshipDirection;
                switch (nodeExtendedData.GetRelationshipDirection())
                    {
                    case ECRelatedInstanceDirection::Backward: relationshipDirection = "Backward"; break;
                    case ECRelatedInstanceDirection::Forward:  relationshipDirection = "Forward"; break;
                    }
                
                ECClassCP parentClass = m_context.m_connection.GetECDb().Schemas().GetClass(nodeExtendedData.GetParentECClassId());
                context.AddSymbol(*ValueSymbol::Create("ParentClassName", ECValue(parentClass->GetName().c_str(), false)));
                context.AddSymbol(*ValueSymbol::Create("ParentSchemaName", ECValue(parentClass->GetSchema().GetName().c_str(), false)));
                context.AddSymbol(*ValueSymbol::Create("RelationshipDirection", ECValue(relationshipDirection.c_str(), true)));
                }

            if (node.GetType().Equals(NAVNODE_TYPE_ECInstanceNode))
                {
                BeAssert(nullptr != node.GetKey()->AsECInstanceNodeKey());
                context.AddSymbol(*ValueSymbol::Create("InstanceId", ECValue(node.GetKey()->AsECInstanceNodeKey()->GetInstanceId().ToString().c_str())));
                context.AddSymbol(*ValueSymbol::Create("IsInstanceNode", ECValue(true)));
                context.AddSymbol(*PropertySymbol::Create("ECInstance", *NodeECInstanceContextEvaluator::Create(m_context.m_connection, node)));
                }
            else
                {
                context.AddSymbol(*ValueSymbol::Create("InstanceId", ECValue()));
                context.AddSymbol(*ValueSymbol::Create("IsInstanceNode", ECValue(false)));
                context.AddSymbol(*ValueSymbol::Create("ECInstance", ECValue()));
                }
            }
        context.AddSymbol(*MethodSymbol::Create("IsOfClass", &NodeSymbolsProvider::IsOfClass, 
            nullptr, const_cast<Context*>(&m_context)));
        }
public:
    NodeSymbolsProvider(Context const& context) 
        : m_context(context)
        {}
};

//=======================================================================================
// @bsiclass                                                Grigas.Petraitis    12/2015
//=======================================================================================
struct NodeContextEvaluator : PropertySymbol::ContextEvaluator
{
private:
    RulesEngineRootSymbolsContext& m_rootContext;
    IConnectionCR m_connection;
    Utf8String m_locale;
    INavNodeLocaterCP m_locater;
    NavNodeKeyCP m_key;
    SymbolExpressionContextPtr m_context;
    NodeContextEvaluator(RulesEngineRootSymbolsContext& rootContext, IConnectionCR connection, Utf8String locale, INavNodeLocaterCP locater, NavNodeKeyCP key)
        : m_rootContext(rootContext), m_connection(connection), m_locale(locale), m_locater(locater), m_key(key), m_context(nullptr)
        {}
public:
    static RefCountedPtr<NodeContextEvaluator> Create(RulesEngineRootSymbolsContext& rootContext, IConnectionCR connection, 
        Utf8String locale, INavNodeLocaterCP locater, NavNodeKeyCP key)
        {
        return new NodeContextEvaluator(rootContext, connection, locale, locater, key);
        }
    virtual ExpressionContextPtr _GetContext() override
        {
        if (m_context.IsValid())
            return m_context;

        JsonNavNodeCPtr node = (nullptr != m_key && nullptr != m_locater) ? m_locater->LocateNode(m_connection, m_locale, *m_key) : nullptr;
        if (node.IsNull() && nullptr != m_key && nullptr != m_key->AsECInstanceNodeKey())
            {
            ECInstanceNodeKey const* instanceKey = m_key->AsECInstanceNodeKey();
            JsonNavNodePtr temp = JsonNavNodesFactory().CreateECInstanceNode(m_connection, m_locale, instanceKey->GetECClassId(), instanceKey->GetInstanceId(), "");
            temp->SetNodeKey(*NavNodesHelper::CreateNodeKey(m_connection, *temp, bvector<Utf8String>()));
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
// @bsiclass                                                Grigas.Petraitis    03/2015
//=======================================================================================
struct RulesetVariablesSymbolsProvider : IECSymbolProvider
{
    struct Context : ProviderContext
        {
        IUserSettings const& m_settings;
        IUsedUserSettingsListener* m_usedSettingsListener;
        Context(IUserSettings const& settings, IUsedUserSettingsListener* usedSettingsListener) 
            : m_settings(settings), m_usedSettingsListener(usedSettingsListener)
            {}
        };
private:
    Context const& m_context;

private:
    static void OnVariableUsed(Context const& context, Utf8CP variableId)
        {
        if (nullptr != context.m_usedSettingsListener)
            context.m_usedSettingsListener->_OnUserSettingUsed(variableId);
        }
    static ExpressionStatus GetStringVariableValue(EvaluationResult& evalResult, void* methodContext, EvaluationResultVector& arguments)
        {
        if (1 != arguments.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("RulesetVariablesSymbolsProvider::GetStringVariableValue: WrongNumberOfArguments. Expected 1, actually: %" PRIu64, (uint64_t)arguments.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (!arguments[0].IsECValue() || !arguments[0].GetECValue()->IsString())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "RulesetVariablesSymbolsProvider::GetStringVariableValue: Wrong argument type (first argument is not ECValue, or is not a string)");
            return ExpressionStatus::WrongType;
            }

        Utf8CP variableId = arguments[0].GetECValue()->GetUtf8CP();
        Context const& context = *static_cast<Context*>(methodContext);
        evalResult.InitECValue().SetUtf8CP(context.m_settings.GetSettingValue(variableId).c_str());

        OnVariableUsed(context, variableId);

        return ExpressionStatus::Success;
        }
    static ExpressionStatus GetIntVariableValue(EvaluationResult& evalResult, void* methodContext, EvaluationResultVector& arguments)
        {
        if (1 != arguments.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("RulesetVariablesSymbolsProvider::GetIntVariableValue: WrongNumberOfArguments. Expected 1, actually: %" PRIu64, (uint64_t)arguments.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (!arguments[0].IsECValue() || !arguments[0].GetECValue()->IsString())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "RulesetVariablesSymbolsProvider::GetIntVariableValue: Wrong argument type (first argument is not ECValue, or is not a string)");
            return ExpressionStatus::WrongType;
            }
        
        Utf8CP variableId = arguments[0].GetECValue()->GetUtf8CP();
        Context const& context = *static_cast<Context*>(methodContext);
        evalResult.InitECValue().SetLong(context.m_settings.GetSettingIntValue(variableId));
        
        OnVariableUsed(context, variableId);

        return ExpressionStatus::Success;
        }
    static ExpressionStatus GetIntArrayVariableValue(EvaluationResult& evalResult, void* methodContext, EvaluationResultVector& arguments)
        {
        if (1 != arguments.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("RulesetVariablesSymbolsProvider::GetIntArrayVariableValue: WrongNumberOfArguments. Expected 1, actually: %" PRIu64, (uint64_t)arguments.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (!arguments[0].IsECValue() || !arguments[0].GetECValue()->IsString())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "RulesetVariablesSymbolsProvider::GetIntArrayVariableValue: Wrong argument type (first argument is not ECValue, or is not a string)");
            return ExpressionStatus::WrongType;
            }

        if (nullptr == methodContext)
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "RulesetVariablesSymbolsProvider::GetIntArrayVariableValue: UnknownError (nullptr == methodContext)");
            return ExpressionStatus::UnknownError;
            }

        Utf8CP variableId = arguments[0].GetECValue()->GetUtf8CP();
        Context const& context = *static_cast<Context*>(methodContext);
        bvector<int64_t> values = context.m_settings.GetSettingIntValues(variableId);
        bvector<EvaluationResult> resultValues;
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
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("RulesetVariablesSymbolsProvider::GetBoolVariableValue: WrongNumberOfArguments. Expected 1, actually: %" PRIu64, (uint64_t)arguments.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (!arguments[0].IsECValue() || !arguments[0].GetECValue()->IsString())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "RulesetVariablesSymbolsProvider::GetBoolVariableValue: Wrong argument type (first argument is not ECValue, or is not a string)");
            return ExpressionStatus::WrongType;
            }

        Utf8CP variableId = arguments[0].GetECValue()->GetUtf8CP();
        Context const& context = *static_cast<Context*>(methodContext);
        evalResult.InitECValue().SetBoolean(context.m_settings.GetSettingBoolValue(variableId));

        OnVariableUsed(context, variableId);
        return ExpressionStatus::Success;
        }
    static ExpressionStatus HasVariable(EvaluationResult& evalResult, void* methodContext, EvaluationResultVector& arguments)
        {
        if (1 != arguments.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("RulesetVariablesSymbolsProvider::HasVariable: WrongNumberOfArguments. Expected 1, actually: %" PRIu64, (uint64_t)arguments.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (!arguments[0].IsECValue() || !arguments[0].GetECValue()->IsString())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "RulesetVariablesSymbolsProvider::HasVariable: Wrong argument type (first argument is not ECValue, or is not a string)");
            return ExpressionStatus::WrongType;
            }

        Utf8CP variableId = arguments[0].GetECValue()->GetUtf8CP();
        Context const& context = *static_cast<Context*>(methodContext);
        evalResult.InitECValue().SetBoolean(context.m_settings.GetSettingBoolValue(variableId));

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
// @bsiclass                                                Mantas.Kontrimas    05/2018
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
            BeAssert(false);
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("LabelSymbolsProvider::GetRelatedDisplayLabelQuery: UnknownError. Invalid direction (%s)", direction).c_str());
            return ExpressionStatus::UnknownError;
            }

        Utf8String relationshipSchemaName, relationshipClassName;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(relationshipSchemaName, relationshipClassName, relationshipName))
            {
            BeAssert(false);
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("LabelSymbolsProvider::GetRelatedDisplayLabelQuery: UnknownError. Could not parse relationship name: %s", relationshipName).c_str());
            return ExpressionStatus::UnknownError;
            }

        Utf8String relatedClassSchemaName, relatedClassName;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(relatedClassSchemaName, relatedClassName, className))
            {
            BeAssert(false);
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("LabelSymbolsProvider::GetRelatedDisplayLabelQuery: UnknownError. Could not parse class name: %s", className).c_str());
            return ExpressionStatus::UnknownError;
            }

        query = Utf8PrintfString("SELECT " FUNCTION_NAME_GetRelatedDisplayLabel "([related].[ECClassId], [related].[ECInstanceId]) FROM %%s this, [%s].[%s] relationship, [%s].[%s] related "
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
    * @bsimethod                                    Mantas.Kontrimas                05/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ExpressionStatus GetRelatedDisplayLabel (EvaluationResult& evalResult, void* context, ECInstanceListCR instanceData, EvaluationResultVector& args)
        {
        if (instanceData.empty())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "LabelSymbolsProvider::GetRelatedDisplayLabel: WrongType. ECInstanceList is empty");
            return ExpressionStatus::WrongType;
            }

        if (3 != args.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("LabelSymbolsProvider::GetRelatedDisplayLabel: WrongNumberOfArguments. Expected 3, actually: %" PRIu64, (uint64_t)args.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        for (size_t i = 0; i < 3; ++i)
            {
            if (!args[i].IsECValue() || !args[i].GetECValue()->IsString())
                {
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "LabelSymbolsProvider::GetRelatedDisplayLabel: WrongType. Invalid arguments (expecting strings)");
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
                {
                BeAssert(false);
                continue;
                }

            ECInstanceId id;
            ECInstanceId::FromString(id, instance->GetInstanceId().c_str());
            status = stmt.BindId(1, id);
            if (!status.IsSuccess())
                {
                BeAssert(false);
                continue;
                }

            if (DbResult::BE_SQLITE_ROW == stmt.Step())
                {
                evalResult.InitECValue() = ECValue(stmt.GetValueText(0));
                ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("LabelOverrideSymbolsProvider::GetRelatedDisplayLabel: Result: %s", evalResult.ToString().c_str()).c_str());
                return ExpressionStatus::Success;
                }
            }
    
        evalResult.InitECValue().SetToNull();
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_TRACE, Utf8PrintfString("LabelOverrideSymbolsProvider::GetRelatedDisplayLabel: Result: %s", evalResult.ToString().c_str()).c_str());
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionContextPtr ECExpressionContextsProvider::GetNodeRulesContext(NodeRulesContextParameters const& params)
    {
    RulesEngineRootSymbolsContextPtr rootCtx = RulesEngineRootSymbolsContext::Create();

    // Ruleset variables
    RulesetVariablesSymbolsProvider rulesetVariablesSymbols(rootCtx->AddContext(*new RulesetVariablesSymbolsProvider::Context(params.GetUserSettings(), params.GetUsedSettingsListener())));
    rulesetVariablesSymbols.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());

    // ParentNode
    NodeSymbolsProvider nodeSymbols(rootCtx->AddContext(*new NodeSymbolsProvider::Context(params.GetConnection(), params.GetParentNode())));
    SymbolExpressionContextPtr parentNodeCtx = SymbolExpressionContext::Create(nullptr);
    nodeSymbols.PublishSymbols(*parentNodeCtx, bvector<Utf8String>());
    rootCtx->GetSymbolsContext().AddSymbol(*ContextSymbol::CreateContextSymbol("ParentNode", *parentNodeCtx));

    // ECInstance methods
    ECInstanceMethodSymbolsProvider ecInstanceMethods;
    ecInstanceMethods.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());

    return rootCtx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionContextPtr ECExpressionContextsProvider::GetContentRulesContext(ContentRulesContextParameters const& params)
    {
    RulesEngineRootSymbolsContextPtr rootCtx = RulesEngineRootSymbolsContext::Create();

    // Ruleset variables
    RulesetVariablesSymbolsProvider rulesetVariablesSymbols(rootCtx->AddContext(*new RulesetVariablesSymbolsProvider::Context(params.GetUserSettings(), params.GetUsedSettingsListener())));
    rulesetVariablesSymbols.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());
    
    // SelectedNode
    rootCtx->GetSymbolsContext().AddSymbol(*PropertySymbol::Create("SelectedNode", *NodeContextEvaluator::Create(*rootCtx, params.GetConnection(), params.GetLocale(), params.GetNodeLocater(), params.GetSelectedNodeKey())));

    // Content-specific
    rootCtx->GetSymbolsContext().AddSymbol(*ValueSymbol::Create("ContentDisplayType", ECValue(params.GetContentDisplayType().c_str())));
    rootCtx->GetSymbolsContext().AddSymbol(*ValueSymbol::Create("SelectionProviderName", ECValue(params.GetSelectionProviderName().c_str())));
    rootCtx->GetSymbolsContext().AddSymbol(*ValueSymbol::Create("IsSubSelection", ECValue(params.IsSubSelection())));

    // ECInstance methods
    ECInstanceMethodSymbolsProvider ecInstanceMethods;
    ecInstanceMethods.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());

    return rootCtx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionContextPtr ECExpressionContextsProvider::GetCustomizationRulesContext(CustomizationRulesContextParameters const& params)
    {
    RulesEngineRootSymbolsContextPtr rootCtx = RulesEngineRootSymbolsContext::Create();
    
    // Ruleset variables
    RulesetVariablesSymbolsProvider rulesetVariablesSymbols(rootCtx->AddContext(*new RulesetVariablesSymbolsProvider::Context(params.GetUserSettings(), params.GetUsedSettingsListener())));
    rulesetVariablesSymbols.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());
    
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
    if (params.GetNode().GetType().Equals(NAVNODE_TYPE_ECInstanceNode))
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

    // ECInstance methods
    ECInstanceMethodSymbolsProvider ecInstanceMethods;
    ecInstanceMethods.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());

    // Label related methods
    LabelSymbolsProvider labelOverrrideMethods(rootCtx->AddContext(*new LabelSymbolsProvider::Context(params.GetConnection())));
    labelOverrrideMethods.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());

    return rootCtx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Tautvydas.Zinys                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionContextPtr ECExpressionContextsProvider::GetCalculatedPropertyContext(CalculatedPropertyContextParameters const& params)
    {
    RulesEngineRootSymbolsContextPtr rootCtx = RulesEngineRootSymbolsContext::Create();

    // Ruleset variables
    RulesetVariablesSymbolsProvider rulesetVariablesSymbols(rootCtx->AddContext(*new RulesetVariablesSymbolsProvider::Context(params.GetUserSettings(), params.GetUsedSettingsListener())));
    rulesetVariablesSymbols.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());

    // this
    rootCtx->GetSymbolsContext().AddSymbol(*PropertySymbol::Create("this", *NodeECInstanceContextEvaluator::Create(params.GetConnection(), params.GetNode())));

    // ECInstance methods
    ECInstanceMethodSymbolsProvider ecInstanceMethods;
    ecInstanceMethods.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());

    return rootCtx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECExpressionsHelper::EvaluateECExpression(ECValueR result, Utf8StringCR expression, ExpressionContextR context)
    {
    NodePtr node = GetNodeFromExpression(expression.c_str());

    ValueResultPtr valueResult;
    if (ExpressionStatus::Success != node->GetValue(valueResult, context))
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ECExpressionsHelper::EvaluateECExpression: Could not get node value");
        return false;
        }

    if (ExpressionStatus::Success != valueResult->GetECValue(result))
        {
        ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "ECExpressionsHelper::EvaluateECExpression: Could not get ECValue from value result");
        return false;
        }

    return true;
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                05/2016
+===============+===============+===============+===============+===============+======*/
struct SelectColumnsInfo
    {
    Utf8String ClassIdColumnName;
    Utf8String InstanceIdColumnName;
    };
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                05/2016
+===============+===============+===============+===============+===============+======*/
struct SelectClassNames
    {
    Utf8String SchemaName;
    Utf8String ClassName;
    };
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                05/2016
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
* @bsiclass                                     Grigas.Petraitis                05/2016
+===============+===============+===============+===============+===============+======*/
struct ECExpressionToECSqlConverter : NodeVisitor
{
#define ARGUMENTS_PRECONDITION() if (m_inArguments && m_ignoreArguments) return true;

private:
    bvector<Utf8String> m_usedClasses;
    Utf8String m_ecsql;
    RefCountedPtr<CallNode const> m_currentValueListMethodNode;
    bool m_inArguments;
    bool m_ignoreArguments;
    bool m_inStructProperty;
    ExpressionToken m_previousToken;
    bvector<Utf8String> m_nodesStack;

private:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            05/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    static bool NeedsSpaceAfter(Utf8StringCR str)
        {
        BeAssert(!str.empty());
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
    * @bsimethod                                    Grigas.Petraitis            06/2015
    +---------------+---------------+---------------+---------------+---------------+--*/
    static bool NeedsSpaceBefore(Utf8StringCR str)
        {
        BeAssert(!str.empty());
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
    * @bsimethod                                    Grigas.Petraitis            05/2016
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
    * @bsimethod                                    Grigas.Petraitis            05/2016
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
    * @bsimethod                                    Aidas.Vaiksnoras            08/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleLikeToken(NodeCR node)
        {
        Utf8String::size_type previousNode = m_ecsql.rfind(m_nodesStack.back());
        if (previousNode == Utf8String::npos)
            return;

        m_ecsql.insert(previousNode, "CAST(");
        Append("AS TEXT)");
        Append("LIKE");
        }
   
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            05/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleIdentNode(IdentNodeCR node)
        {
        Append(Utf8PrintfString("[%s]", node.GetName()));
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            09/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleNullNode()
        {
        if (TOKEN_Equal == m_previousToken)
            {
            BeAssert(m_ecsql.EndsWith("="));
            m_ecsql.replace(m_ecsql.size() - 1, 1, "IS");
            }
        if (TOKEN_NotEqual == m_previousToken)
            {
            BeAssert(m_ecsql.EndsWith("<>"));
            m_ecsql.replace(m_ecsql.size() - 2, 2, "IS NOT");
            }
        Append("NULL");
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            09/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleScopeEnd()
        {
        if (m_inStructProperty)
            Append("]");

        m_inStructProperty = false;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            05/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    Utf8String GetCallNodePrefix(CallNodeCR node)
        {
        Utf8String prefix;
        if (node.ToString().StartsWith("."))
            {
            size_t lastSpaceIndex = m_ecsql.find_last_of(' ');
            if (Utf8String::npos != lastSpaceIndex)
                {
                prefix = m_ecsql.substr(lastSpaceIndex + 1);
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
    * @bsimethod                                    Grigas.Petraitis            05/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleIsOfClassSpecialCase(CallNodeCR node)
        {
        Utf8String prefix = GetCallNodePrefix(node);
        if (prefix.Equals("[ThisNode]") || prefix.Equals("ThisNode"))
            prefix = "[this]";
        Append(FUNCTION_NAME_IsOfClass);
        StartArguments(node);
        if (!prefix.empty())
            m_ecsql.append(prefix).append(".");
        Append("[ECClassId]");
        Comma();

        if (nullptr != node.GetArguments() && 2 == node.GetArguments()->GetArgumentCount())
            {
            NodeCP classNameArg = node.GetArguments()->GetArgument(0);
            NodeCP schemaNameArg = node.GetArguments()->GetArgument(1);
            Utf8String qualifiedClassName = schemaNameArg->ToString().Trim("\"");
            qualifiedClassName.append(":").append(classNameArg->ToString().Trim("\""));
            m_usedClasses.push_back(qualifiedClassName);
            }

        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            05/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleGetECClassIdSpecialCase(CallNodeCR node)
        {
        Append(FUNCTION_NAME_GetECClassId);
        return true;
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            05/2016
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
            return ERROR;
            }

        Utf8String relationshipSchemaAndClassName = args.GetArgument(0)->ToString().Trim("\"");
        if (ECObjectsStatus::Success != ECClass::ParseClassName(info.RelationshipNames.SchemaName, info.RelationshipNames.ClassName, relationshipSchemaAndClassName))
            return ERROR;
                
        Utf8String relatedSchemaAndClassName = args.GetArgument(2)->ToString().Trim("\"");
        if (ECObjectsStatus::Success != ECClass::ParseClassName(info.RelatedClassNames.SchemaName, info.RelatedClassNames.ClassName, relatedSchemaAndClassName))
            return ERROR;

        
        m_usedClasses.push_back(relationshipSchemaAndClassName);
        m_usedClasses.push_back(relatedSchemaAndClassName);
        return SUCCESS;
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            05/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleHasRelatedInstanceSpecialCase(CallNodeCR node)
        {
        ArgumentTreeNodeCP args = node.GetArguments();
        if (nullptr == args || 3 != args->GetArgumentCount())
            {
            BeAssert(false);
            return false;
            }

        RelatedClassInfo info;
        if (SUCCESS != ParseRelatedClassInfo(*args, info))
            {
            BeAssert(false);
            return false;
            }

        static Utf8CP s_fmt = "%s.[ECInstanceId] IN ("
            "SELECT [relationship].[%s] "
            "FROM [%s].[%s] relationship, [%s].[%s] related "
            "WHERE [relationship].[%s] = [related].[ECClassId] AND [relationship].[%s] = [related].[ECInstanceId]"
            ")";
        Utf8String prefix = GetCallNodePrefix(node);
        Utf8PrintfString ecsql(s_fmt, 
            prefix.c_str(), info.ThisColumn.InstanceIdColumnName.c_str(),
            info.RelationshipNames.SchemaName.c_str(), info.RelationshipNames.ClassName.c_str(),
            info.RelatedClassNames.SchemaName.c_str(), info.RelatedClassNames.ClassName.c_str(),
            info.RelatedColumn.ClassIdColumnName.c_str(), info.RelatedColumn.InstanceIdColumnName.c_str());
        m_ecsql.append(ecsql);
        m_ignoreArguments = true;
        return true;
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            05/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleGetRelatedValueSpecialCase(CallNodeCR node)
        {
        ArgumentTreeNodeCP args = node.GetArguments();
        if (nullptr == args || 4 != args->GetArgumentCount())
            {
            BeAssert(false);
            return false;
            }

        RelatedClassInfo info;
        if (SUCCESS != ParseRelatedClassInfo(*args, info))
            {
            BeAssert(false);
            return false;
            }

        Utf8String propertyName = args->GetArgument(3)->ToString().Trim("\"");
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
        m_ignoreArguments = true;
        return true;
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            12/2018
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleGetRelatedInstancesCountSpecialCase(CallNodeCR node)
        {
        ArgumentTreeNodeCP args = node.GetArguments();
        if (nullptr == args || 3 != args->GetArgumentCount())
            {
            BeAssert(false);
            return false;
            }

        RelatedClassInfo info;
        if (SUCCESS != ParseRelatedClassInfo(*args, info))
            {
            BeAssert(false);
            return false;
            }

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
        m_ignoreArguments = true;
        return true;
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            02/2018
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleValueListMethodSpecialCase(CallNodeCR node)
        {
        m_currentValueListMethodNode = &node;
        m_ignoreArguments = true;
        return true;
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            03/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleGetVariableIntValuesSpecialCase(CallNodeCR node)
        {
        ArgumentTreeNodeCP args = node.GetArguments();
        if (nullptr == args || 1 != args->GetArgumentCount())
            {
            BeAssert(false && "Only a single argument (setting ID) is allowed for GetVariableIntValues function");
            return false;
            }

        m_currentValueListMethodNode = &node;

        Append(FUNCTION_NAME_InVariableIntValues);
        StartArguments(*args);
        args->GetArgument(0)->Traverse(*this);
        m_ignoreArguments = true;
        return true;
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            03/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleAnyMatchSpecialCase(CallNodeCR node)
        {
        if (m_currentValueListMethodNode.IsNull())
            {
            BeAssert(false);
            return false;
            }
        
        ArgumentTreeNodeCP args = node.GetArguments();
        if (nullptr == args || 1 != args->GetArgumentCount() || nullptr == dynamic_cast<LambdaNodeCP>(args->GetArgument(0)))
            {
            BeAssert(false && "Only a single lambda function is supported as an argument for AnyMatch function");
            return false;
            }
        
        bool isGetUserSettingIntValuesSpecialCase = (0 == strcmp("GetVariableIntValues", m_currentValueListMethodNode->GetMethodName()));
        if (!isGetUserSettingIntValuesSpecialCase)
            {
            m_ignoreArguments = true;
            args->Traverse(*this);
            }

        m_inArguments = true;
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            05/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleCallNode(CallNodeCR node)
        {
        static bmap<Utf8String, Utf8String> renames;
        if (renames.empty())
            {
            renames.Insert("GetSettingValue", FUNCTION_NAME_GetVariableStringValue);
            renames.Insert("GetSettingIntValue", FUNCTION_NAME_GetVariableIntValue);
            renames.Insert("GetSettingIntValues", "GetVariableIntValues");
            renames.Insert("GetSettingBoolValue", FUNCTION_NAME_GetVariableBoolValue);
            renames.Insert("HasSetting", FUNCTION_NAME_HasVariable);
            }

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
        if (0 == strcmp("AnyMatch", nodeCopy->GetMethodName()) && HandleAnyMatchSpecialCase(*nodeCopy))
            return;

        Append(nodeCopy->ToString());
        m_inArguments = false;
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            03/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleLambdaNode(LambdaNodeCR node)
        {
        if (m_currentValueListMethodNode.IsNull())
            {
            BeAssert(false && "Lamba node is only expected after a value list node");
            return;
            }

        Utf8CP symbolName = node.GetSymbolName();
        ComparisonNodeCP expressionNode = dynamic_cast<ComparisonNodeCP>(&node.GetExpression());
        if (nullptr == expressionNode || TOKEN_Equal != expressionNode->GetOperation())
            {
            BeAssert(false && "Only an EQUALS operation is allowed in lambda expression");
            return;
            }

        bool isGetUserSettingIntValuesSpecialCase = (0 == strcmp("GetVariableIntValues", m_currentValueListMethodNode->GetMethodName()));

        NodeCP propertyAccessNode;
        if (expressionNode->GetLeftCP()->ToExpressionString().Equals(symbolName))
            propertyAccessNode = expressionNode->GetRightCP();
        else if (expressionNode->GetRightCP()->ToExpressionString().Equals(symbolName))
            propertyAccessNode = expressionNode->GetLeftCP();
        else
            {
            BeAssert(false);
            return;
            }

        if (isGetUserSettingIntValuesSpecialCase)
            Comma();

        propertyAccessNode->Traverse(*this);
        HandleScopeEnd();

        if (isGetUserSettingIntValuesSpecialCase)
            {
            EndArguments(*m_currentValueListMethodNode->GetArguments());
            }
        else
            {
            Append("IN ");
            m_inArguments = false;
            StartArguments(*m_currentValueListMethodNode);
            m_currentValueListMethodNode->GetArguments()->Traverse(*this);
            }

        m_currentValueListMethodNode = nullptr;
        m_inArguments = true;
        m_ignoreArguments = true;
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            05/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleDotNode(DotNodeCR node)
        {
        Append(node.GetName());
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            05/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleConditionNode(IIfNodeCR node)
        {
        Append(node.ToString());
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            10/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleEqualtyNode(ComparisonNodeCR node)
        {
        Append(node.ToString());

        Utf8String left = node.GetLeftCP()->ToExpressionString();
        if (left.EndsWith(".ClassName"))
            m_usedClasses.push_back(node.GetRightCP()->ToString().Trim("\""));
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            08/2018
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleStringConstNode(LiteralNode const& node)
        {
        Utf8CP value = node.GetInternalValue().GetUtf8CP();
        if (0 == strcmp(value, ContentDescriptor::DisplayLabelField::NAME))
            {
            // note: name of display label field contains invalid characters, so we wrap it
            // with quotes to make it look like a string - see Preprocess method
            Append(QueryHelpers::Wrap(value));
            return;
            }
        Append(Utf8PrintfString("'%s'", value));

        if (m_nodesStack.size() > 1 && std::string::npos != (m_nodesStack.end() - 2)->find("LIKE"))
            Append("ESCAPE \'\\\'");
        }

public:
    bool StartArrayIndex(NodeCR) override {BeAssert(false); return false;}
    bool EndArrayIndex(NodeCR) override {BeAssert(false); return false;}
    bool ProcessUnits(UnitSpecCR) override {BeAssert(false); return false;}
    bool StartArguments(NodeCR) override
        {
        if (!m_inArguments && !m_ignoreArguments)
            Append("(", false);
        m_inArguments = true;
        return true;
        }
    bool EndArguments(NodeCR) override
        {
        if (!m_ignoreArguments)
            {
            HandleScopeEnd();
            Append(")");
            }
        m_ignoreArguments = false;
        m_inArguments = false;

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
    bool Comma() override {ARGUMENTS_PRECONDITION(); Append(","); return true;}
    bool OpenParens() override {ARGUMENTS_PRECONDITION(); Append("("); return true;}
    bool CloseParens() override {ARGUMENTS_PRECONDITION(); Append(")"); return true;}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            05/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool ProcessNode(NodeCR node) override
        {
        ARGUMENTS_PRECONDITION(); 

        HandleScopeEnd();

        m_inStructProperty = (TOKEN_Dot == node.GetOperation());
        if (m_inStructProperty)
            {
            Append(".");
            Append("[");
            }

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
                BeAssert(nullptr != dynamic_cast<ComparisonNodeCP>(&node));
                HandleEqualtyNode(static_cast<ComparisonNodeCR>(node));
                break;
            case TOKEN_StringConst:
                BeAssert(nullptr != dynamic_cast<LiteralNode const*>(&node) 
                    && dynamic_cast<LiteralNode const*>(&node)->GetInternalValue().IsUtf8());
                HandleStringConstNode(static_cast<LiteralNode const&>(node));
                break;
            case TOKEN_DateTimeConst:
                Append(Utf8PrintfString("DATE '%s'", node.ToString().c_str()));
                break;
            case TOKEN_Ident:
                BeAssert(nullptr != dynamic_cast<IdentNodeCP>(&node));
                HandleIdentNode(static_cast<IdentNodeCR>(node));
                break;
            case TOKEN_LParen:
                BeAssert(nullptr != dynamic_cast<CallNodeCP>(&node));
                HandleCallNode(static_cast<CallNodeCR>(node));
                break;
            case TOKEN_Dot:
                BeAssert(nullptr != dynamic_cast<DotNodeCP>(&node));
                HandleDotNode(static_cast<DotNodeCR>(node));
                break;
            case TOKEN_IIf:
                BeAssert(nullptr != dynamic_cast<IIfNodeCP>(&node));
                HandleConditionNode(static_cast<IIfNodeCR>(node));
                break;
            case TOKEN_Lambda:
                BeAssert(nullptr != dynamic_cast<LambdaNodeCP>(&node));
                HandleLambdaNode(static_cast<LambdaNodeCR>(node));
                break;
            default:
                BeAssert(false);
            }

        m_previousToken = node.GetOperation();
        return true;
        }

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            05/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    ECExpressionToECSqlConverter() 
        : m_inArguments(false), m_inStructProperty(false), m_ignoreArguments(false), m_previousToken(TOKEN_Unrecognized)
        {}
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            08/2018
    +---------------+---------------+---------------+---------------+---------------+--*/
    static void Preprocess(Utf8StringR expr)
        {
        expr.ReplaceAll(ContentDescriptor::DisplayLabelField::NAME, Utf8String(ContentDescriptor::DisplayLabelField::NAME).AddQuotes().c_str());
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            05/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    Utf8StringCR GetECSql(NodeCR node) 
        {
        m_ecsql.clear();
        node.Traverse(*this);
        HandleScopeEnd();
        m_ecsql.ReplaceAll("parent].[parent", "parent.parent");
        return m_ecsql;
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            07/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    bvector<Utf8String> const& GetUsedClasses(NodeCR node)
        {
        m_usedClasses.clear();
        node.Traverse(*this);
        HandleScopeEnd();
        return m_usedClasses;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECExpressionsHelper::ConvertToECSql(Utf8StringCR expression)
    {
    if (expression.empty())
        return "";

    Utf8String copy(expression);
    ECExpressionToECSqlConverter::Preprocess(copy);

    NodePtr node = GetNodeFromExpression(copy.c_str());
    if (node.IsNull())
        {
        BeAssert(false);
        return "";
        }

    ECExpressionToECSqlConverter converter;
    return converter.GetECSql(*node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> const& ECExpressionsHelper::GetUsedClasses(Utf8StringCR expression)
    {
    static bvector<Utf8String> empty;
    if (expression.empty())
        return empty;

    bvector<Utf8String> const* cachedClasses = m_cache.GetUsedClasses(expression.c_str());
    if (nullptr != cachedClasses)
        return *cachedClasses;

    NodePtr node = GetNodeFromExpression(expression.c_str());
    if (node.IsNull())
        {
        BeAssert(false);
        return empty;
        }

    ECExpressionToECSqlConverter converter;
    bvector<Utf8String> classes = converter.GetUsedClasses(*node);
    m_cache.Add(expression.c_str(), classes);
    return *m_cache.GetUsedClasses(expression.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr ECExpressionsHelper::GetNodeFromExpression(Utf8CP expr)
    {
    NodePtr node;
    if (SUCCESS == m_cache.Get(node, expr))
        return node;

    Utf8String expression = expr;

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
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECExpressionsCache::Get(NodePtr& node, Utf8CP expression) const
    {
    auto iter = m_cache.find(expression);
    if (m_cache.end() == iter)
        return ERROR;
    node = iter->second;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECExpressionsCache::Get(OptimizedExpressionPtr& optimizedExpr, Utf8CP expression) const
    {
    auto iter = m_optimizedCache.find(expression);
    if (m_optimizedCache.end() == iter)
        return ERROR;
    optimizedExpr = iter->second;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> const* ECExpressionsCache::GetUsedClasses(Utf8CP expression) const
    {
    auto iter = m_usedClasses.find(expression);
    if (m_usedClasses.end() == iter)
        return nullptr;
    return &iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECExpressionsCache::HasOptimizedExpression(Utf8CP expression) const {return m_optimizedCache.end() != m_optimizedCache.find(expression);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ECExpressionsCache::Add(Utf8CP expression, NodePtr node) {m_cache.Insert(expression, node);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ECExpressionsCache::Add(Utf8CP expression, OptimizedExpressionPtr node) {m_optimizedCache.Insert(expression, node);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> const& ECExpressionsCache::Add(Utf8CP expression, bvector<Utf8String>& classes) {return m_usedClasses.Insert(expression, classes).first->second;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ECExpressionsCache::Clear() {m_cache.clear(); m_optimizedCache.clear();}
