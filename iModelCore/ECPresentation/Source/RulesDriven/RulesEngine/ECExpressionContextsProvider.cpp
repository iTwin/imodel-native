/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ECExpressionContextsProvider.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
#include <regex>
#include "ECSchemaHelper.h"
#include "CustomFunctions.h"

//=======================================================================================
// @bsiclass                                                Grigas.Petraitis    01/2017
//=======================================================================================
struct ProviderContext
    {
    virtual ~ProviderContext() {}
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
    ECDbCR m_db;
    ECInstanceKey m_key;
    ECInstanceContextEvaluator(ECDbCR db, ECInstanceKey key) : m_db(db), m_key(key) {}
public:
    static RefCountedPtr<ECInstanceContextEvaluator> Create(ECDbCR db, ECInstanceKey key) {return new ECInstanceContextEvaluator(db, key);}
    ExpressionContextPtr _GetContext() override
        {
        IECInstancePtr instance;
        InstanceExpressionContextPtr instanceContext = InstanceExpressionContext::Create(nullptr);
        if (m_key.IsValid())
            {
            // load the instance 
            instance = ECInstancesHelper::LoadInstance(m_db, m_key);
            }
        else if (m_key.GetClassId().IsValid())
            {
            // if the instance id is not valid, create an empty instance - this 
            // makes sure we can further successfully use this instance in ECExpressions
            // and all its properties are NULL
            ECClassCP ecClass = m_db.Schemas().GetClass(m_key.GetClassId());
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
    NavNodeCPtr m_node;
    NodeECInstanceContextEvaluator(NavNodeCR node) : m_node(&node) {}
public:
    static RefCountedPtr<NodeECInstanceContextEvaluator> Create(NavNodeCR node) {return new NodeECInstanceContextEvaluator(node);}
    ExpressionContextPtr _GetContext() override
        {
        InstanceExpressionContextPtr instanceContext = InstanceExpressionContext::Create(nullptr);
        RefCountedPtr<IECInstance const> instance = m_node->GetInstance();
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
        ECDbCR m_connection;
        NavNodeCPtr m_node;
        Context(ECDbCR connection, NavNodeCP node) : m_connection(connection), m_node(node) {}
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
        if (ctx.m_node.IsNull() || !ctx.m_connection.IsDbOpen())
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
        
        ECClassCP nodeClass = ctx.m_connection.Schemas().GetClass(extendedData.GetECClassId());
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
                nodeClass = m_context.m_connection.Schemas().GetClass(nodeExtendedData.GetECClassId());

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
            context.AddSymbol(*ValueSymbol::Create("GroupedInstancesCount", ECValue((uint64_t)nodeExtendedData.GetGroupedInstanceKeys().size())));

            if (node.GetType().Equals(NAVNODE_TYPE_ECRelationshipGroupingNode))
                {
                BeAssert(nodeExtendedData.HasParentECClassId() && nodeExtendedData.HasRelationshipDirection());

                Utf8String relationshipDirection;
                switch (nodeExtendedData.GetRelationshipDirection())
                    {
                    case ECRelatedInstanceDirection::Backward: relationshipDirection = "Backward"; break;
                    case ECRelatedInstanceDirection::Forward:  relationshipDirection = "Forward"; break;
                    }
                
                ECClassCP parentClass = m_context.m_connection.Schemas().GetClass(nodeExtendedData.GetParentECClassId());
                context.AddSymbol(*ValueSymbol::Create("ParentClassName", ECValue(parentClass->GetName().c_str(), false)));
                context.AddSymbol(*ValueSymbol::Create("ParentSchemaName", ECValue(parentClass->GetSchema().GetName().c_str(), false)));
                context.AddSymbol(*ValueSymbol::Create("RelationshipDirection", ECValue(relationshipDirection.c_str(), true)));
                }

            if (node.GetType().Equals(NAVNODE_TYPE_ECInstanceNode))
                {
                BeAssert(nullptr != node.GetKey().AsECInstanceNodeKey());
                context.AddSymbol(*ValueSymbol::Create("InstanceId", ECValue(node.GetKey().AsECInstanceNodeKey()->GetInstanceId().ToString().c_str())));
                context.AddSymbol(*ValueSymbol::Create("IsInstanceNode", ECValue(true)));
                context.AddSymbol(*PropertySymbol::Create("ECInstance", *NodeECInstanceContextEvaluator::Create(node)));
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
    ECDbCR m_connection;
    INavNodeLocaterCR m_locater;
    NavNodeKeyCP m_key;
    SymbolExpressionContextPtr m_context;
    NodeContextEvaluator(RulesEngineRootSymbolsContext& rootContext, ECDbCR connection, INavNodeLocaterCR locater, NavNodeKeyCP key)
        : m_rootContext(rootContext), m_connection(connection), m_locater(locater), m_key(key), m_context(nullptr)
        {}
public:
    static RefCountedPtr<NodeContextEvaluator> Create(RulesEngineRootSymbolsContext& rootContext, ECDbCR connection, INavNodeLocaterCR locater, NavNodeKeyCP key)
        {
        return new NodeContextEvaluator(rootContext, connection, locater, key);
        }
    virtual ExpressionContextPtr _GetContext() override
        {
        if (m_context.IsValid())
            return m_context;

        NavNodeCPtr node = (nullptr != m_key) ? m_locater.LocateNode(*m_key) : nullptr;
        if (node.IsNull() && nullptr != m_key && nullptr != m_key->AsECInstanceNodeKey())
            {
            ECInstanceNodeKey const* instanceKey = m_key->AsECInstanceNodeKey();
            node = JsonNavNodesFactory().CreateECInstanceNode(m_connection, instanceKey->GetECClassId(), instanceKey->GetInstanceId(), "");
            }

        NodeSymbolsProvider nodeSymbols(m_rootContext.AddContext(*new NodeSymbolsProvider::Context(m_connection, node.get())));
        m_context = SymbolExpressionContext::Create(nullptr);
        nodeSymbols.PublishSymbols(*m_context, bvector<Utf8String>());
        return m_context;
        }
};

//=======================================================================================
//! Provides functions to access user settings
// @bsiclass                                                Grigas.Petraitis    03/2015
//=======================================================================================
struct UserSettingsSymbolsProvider : IECSymbolProvider
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
    static void OnSettingUsed(Context const& context, Utf8CP settingId)
        {
        if (nullptr != context.m_usedSettingsListener)
            context.m_usedSettingsListener->_OnUserSettingUsed(settingId);
        }
    static ExpressionStatus GetSettingValue(EvaluationResult& evalResult, void* methodContext, EvaluationResultVector& arguments)
        {
        if (1 != arguments.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("UserSettingsSymbolsProvider::GetSettingValue: WrongNumberOfArguments. Expected 1, actually: %" PRIu64, (uint64_t)arguments.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (!arguments[0].IsECValue() || !arguments[0].GetECValue()->IsString())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "UserSettingsSymbolsProvider::GetSettingValue: Wrong argument type (first argument is not ECValue, or is not a string)");
            return ExpressionStatus::WrongType;
            }

        Utf8CP settingId = arguments[0].GetECValue()->GetUtf8CP();
        Context const& context = *static_cast<Context*>(methodContext);
        evalResult.InitECValue().SetUtf8CP(context.m_settings.GetSettingValue(settingId).c_str());

        OnSettingUsed(context, settingId);

        return ExpressionStatus::Success;
        }
    static ExpressionStatus GetSettingIntValue(EvaluationResult& evalResult, void* methodContext, EvaluationResultVector& arguments)
        {
        if (1 != arguments.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("UserSettingsSymbolsProvider::GetSettingIntValue: WrongNumberOfArguments. Expected 1, actually: %" PRIu64, (uint64_t)arguments.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (!arguments[0].IsECValue() || !arguments[0].GetECValue()->IsString())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "UserSettingsSymbolsProvider::GetSettingIntValue: Wrong argument type (first argument is not ECValue, or is not a string)");
            return ExpressionStatus::WrongType;
            }
        
        Utf8CP settingId = arguments[0].GetECValue()->GetUtf8CP();
        Context const& context = *static_cast<Context*>(methodContext);
        evalResult.InitECValue().SetLong(context.m_settings.GetSettingIntValue(settingId));
        
        OnSettingUsed(context, settingId);

        return ExpressionStatus::Success;
        }
    static ExpressionStatus GetSettingIntValues(EvaluationResult& evalResult, void* methodContext, EvaluationResultVector& arguments)
        {
        if (1 != arguments.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("UserSettingsSymbolsProvider::GetSettingIntValues: WrongNumberOfArguments. Expected 1, actually: %" PRIu64, (uint64_t)arguments.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (!arguments[0].IsECValue() || !arguments[0].GetECValue()->IsString())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "UserSettingsSymbolsProvider::GetSettingIntValues: Wrong argument type (first argument is not ECValue, or is not a string)");
            return ExpressionStatus::WrongType;
            }

        if (nullptr == methodContext)
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "UserSettingsSymbolsProvider::GetSettingIntValues: UnknownError (nullptr == methodContext)");
            return ExpressionStatus::UnknownError;
            }

        Utf8CP settingId = arguments[0].GetECValue()->GetUtf8CP();
        Context const& context = *static_cast<Context*>(methodContext);
        bvector<int64_t> values = context.m_settings.GetSettingIntValues(settingId);
        bvector<EvaluationResult> resultValues;
        for (int64_t value : values)
            {
            EvaluationResult r;
            r.InitECValue().SetLong(value);
            resultValues.push_back(r);
            }
        evalResult.SetValueList(*IValueListResult::Create(resultValues));
        
        OnSettingUsed(context, settingId);

        return ExpressionStatus::Success;
        }
    static ExpressionStatus GetSettingBoolValue(EvaluationResult& evalResult, void* methodContext, EvaluationResultVector& arguments)
        {
        if (1 != arguments.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("UserSettingsSymbolsProvider::GetSettingIntValues: WrongNumberOfArguments. Expected 1, actually: %" PRIu64, (uint64_t)arguments.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (!arguments[0].IsECValue() || !arguments[0].GetECValue()->IsString())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "UserSettingsSymbolsProvider::GetSettingBoolValue: Wrong argument type (first argument is not ECValue, or is not a string)");
            return ExpressionStatus::WrongType;
            }

        Utf8CP settingId = arguments[0].GetECValue()->GetUtf8CP();
        Context const& context = *static_cast<Context*>(methodContext);
        evalResult.InitECValue().SetBoolean(context.m_settings.GetSettingBoolValue(settingId));

        OnSettingUsed(context, settingId);
        return ExpressionStatus::Success;
        }
    static ExpressionStatus HasSetting(EvaluationResult& evalResult, void* methodContext, EvaluationResultVector& arguments)
        {
        if (1 != arguments.size())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, Utf8PrintfString("UserSettingsSymbolsProvider::HasSetting: WrongNumberOfArguments. Expected 1, actually: %" PRIu64, (uint64_t)arguments.size()).c_str());
            return ExpressionStatus::WrongNumberOfArguments;
            }

        if (!arguments[0].IsECValue() || !arguments[0].GetECValue()->IsString())
            {
            ECEXPRESSIONS_EVALUATE_LOG(NativeLogging::LOG_ERROR, "UserSettingsSymbolsProvider::HasSetting: Wrong argument type (first argument is not ECValue, or is not a string)");
            return ExpressionStatus::WrongType;
            }

        Utf8CP settingId = arguments[0].GetECValue()->GetUtf8CP();
        Context const& context = *static_cast<Context*>(methodContext);
        evalResult.InitECValue().SetBoolean(context.m_settings.GetSettingBoolValue(settingId));

        OnSettingUsed(context, settingId);
        return ExpressionStatus::Success;
        }
protected:
    Utf8CP _GetName() const override {return "UserSettings";}
    void _PublishSymbols(SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const override
        {
        void* methodContext = const_cast<Context*>(&m_context);
        context.AddSymbol(*MethodSymbol::Create("GetSettingValue", &UserSettingsSymbolsProvider::GetSettingValue, nullptr, methodContext));
        context.AddSymbol(*MethodSymbol::Create("GetSettingIntValue", &UserSettingsSymbolsProvider::GetSettingIntValue, nullptr, methodContext));
        context.AddSymbol(*MethodSymbol::Create("GetSettingIntValues", &UserSettingsSymbolsProvider::GetSettingIntValues, nullptr, methodContext));
        context.AddSymbol(*MethodSymbol::Create("GetSettingBoolValue", &UserSettingsSymbolsProvider::GetSettingBoolValue, nullptr, methodContext));
        context.AddSymbol(*MethodSymbol::Create("HasSetting", &UserSettingsSymbolsProvider::HasSetting, nullptr, methodContext));
        }
public:
    UserSettingsSymbolsProvider(Context const& context) : m_context(context) {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionContextPtr ECExpressionContextsProvider::GetNodeRulesContext(NodeRulesContextParameters const& params)
    {
    RulesEngineRootSymbolsContextPtr rootCtx = RulesEngineRootSymbolsContext::Create();

    // UserSettings
    UserSettingsSymbolsProvider userSettingsSymbols(rootCtx->AddContext(*new UserSettingsSymbolsProvider::Context(params.GetUserSettings(), params.GetUsedSettingsListener())));
    userSettingsSymbols.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());

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

    // UserSettings
    UserSettingsSymbolsProvider userSettingsSymbols(rootCtx->AddContext(*new UserSettingsSymbolsProvider::Context(params.GetUserSettings(), params.GetUsedSettingsListener())));
    userSettingsSymbols.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());
    
    // SelectedNode
    rootCtx->GetSymbolsContext().AddSymbol(*PropertySymbol::Create("SelectedNode", *NodeContextEvaluator::Create(*rootCtx, params.GetConnection(), params.GetNodeLocater(), params.GetSelectedNodeKey())));

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
    
    // UserSettings
    UserSettingsSymbolsProvider userSettingsSymbols(rootCtx->AddContext(*new UserSettingsSymbolsProvider::Context(params.GetUserSettings(), params.GetUsedSettingsListener())));
    userSettingsSymbols.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());
    
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
        rootCtx->GetSymbolsContext().AddSymbol(*PropertySymbol::Create("this", *NodeECInstanceContextEvaluator::Create(params.GetNode())));
    else
        rootCtx->GetSymbolsContext().AddSymbol(*ValueSymbol::Create("this", ECValue()));

    // related instance contexts
    NavNodeExtendedData extendedData(params.GetNode());
    bvector<NavNodeExtendedData::RelatedInstanceKey> relatedInstanceKeys = extendedData.GetRelatedInstanceKeys();
    for (NavNodeExtendedData::RelatedInstanceKey const& key : relatedInstanceKeys)
        {
        rootCtx->GetSymbolsContext().AddSymbol(*PropertySymbol::Create(key.GetAlias(), *ECInstanceContextEvaluator::Create(params.GetConnection(), key.GetInstanceKey())));
        }

    // ECInstance methods
    ECInstanceMethodSymbolsProvider ecInstanceMethods;
    ecInstanceMethods.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());

    return rootCtx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Tautvydas.Zinys                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ExpressionContextPtr ECExpressionContextsProvider::GetCalculatedPropertyContext(NavNodePtr const& thisNode, IUserSettings const& userSettings)
    {
    RulesEngineRootSymbolsContextPtr rootCtx = RulesEngineRootSymbolsContext::Create();

    // UserSettings
    UserSettingsSymbolsProvider userSettingsSymbols(rootCtx->AddContext(*new UserSettingsSymbolsProvider::Context(userSettings, nullptr)));
    userSettingsSymbols.PublishSymbols(rootCtx->GetSymbolsContext(), bvector<Utf8String>());

    // this
    rootCtx->GetSymbolsContext().AddSymbol(*PropertySymbol::Create("this", *NodeECInstanceContextEvaluator::Create(*thisNode)));

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
struct ECExpressionToECSqlConverter : NodeVisitor
{
#define ARGUMENTS_PRECONDITION() if (m_inArguments && m_ignoreArguments) return true;

private:
    bvector<Utf8String> m_usedClasses;
    Utf8String m_ecsql;
    CallNodeCP m_currentValueListMethodNode;
    bool m_inArguments;
    bool m_ignoreArguments;
    bool m_inStructProperty;
    ExpressionToken m_previousToken;
    Utf8String m_previousNode;

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
            m_previousNode.append(" ");
            }

        if (m_inStructProperty)
            m_previousNode.append(value);
        else
            m_previousNode = value;
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
            m_previousNode.append(" ");
            }

        if (m_inStructProperty)
            m_previousNode.append(value);
        else
            m_previousNode = value;
        m_ecsql.append(value);
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Aidas.Vaiksnoras            08/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleLikeToken(NodeCR node)
        {
        Utf8String::size_type previousNode= m_ecsql.rfind(m_previousNode);
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
    bool HandleHasRelatedInstanceSpecialCase(CallNodeCR node)
        {
        ArgumentTreeNodeCP args = node.GetArguments();
        if (nullptr == args || 3 != args->GetArgumentCount())
            {
            BeAssert(false);
            return false;
            }

        Utf8String direction = args->GetArgument(1)->ToString().Trim("\"");
        Utf8CP thisInstanceIdColumnName,
            relatedInstanceIdColumnName, relatedClassIdColumnName;
        if (direction.EqualsI("Forward"))
            {
            thisInstanceIdColumnName = "SourceECInstanceId";
            relatedInstanceIdColumnName = "TargetECInstanceId";
            relatedClassIdColumnName = "TargetECClassId";
            }
        else if (direction.EqualsI("Backward"))
            {
            thisInstanceIdColumnName = "TargetECInstanceId";
            relatedInstanceIdColumnName = "SourceECInstanceId";
            relatedClassIdColumnName = "SourceECClassId";
            }
        else
            {
            BeAssert(false);
            return false;
            }

        Utf8String relationshipSchemaName, relationshipClassName;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(relationshipSchemaName, relationshipClassName, args->GetArgument(0)->ToString().Trim("\"")))
            {
            BeAssert(false);
            return false;
            }
        
        Utf8String relatedSchemaAndClassName = args->GetArgument(2)->ToString().Trim("\"");
        Utf8String relatedClassSchemaName, relatedClassName;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(relatedClassSchemaName, relatedClassName, relatedSchemaAndClassName))
            {
            BeAssert(false);
            return false;
            }
        m_usedClasses.push_back(relatedSchemaAndClassName);

        static Utf8CP s_fmt = "%s.[ECInstanceId] IN ("
            "SELECT [relationship].[%s] "
            "FROM [%s].[%s] relationship, [%s].[%s] related "
            "WHERE [relationship].[%s] = [related].[ECClassId] AND [relationship].[%s] = [related].[ECInstanceId]"
            ")";
        Utf8String prefix = GetCallNodePrefix(node);
        Utf8PrintfString ecsql(s_fmt, 
            prefix.c_str(), thisInstanceIdColumnName,
            relationshipSchemaName.c_str(), relationshipClassName.c_str(),
            relatedClassSchemaName.c_str(), relatedClassName.c_str(),
            relatedClassIdColumnName, relatedInstanceIdColumnName);
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

        Utf8String direction = args->GetArgument(1)->ToString().Trim("\"");
        Utf8CP thisInstanceIdColumnName, thisClassIdColumnName,
            relatedInstanceIdColumnName, relatedClassIdColumnName;
        if (direction.EqualsI("Forward"))
            {
            thisInstanceIdColumnName = "SourceECInstanceId";
            thisClassIdColumnName = "SourceECClassId";
            relatedInstanceIdColumnName = "TargetECInstanceId";
            relatedClassIdColumnName = "TargetECClassId";
            }
        else if (direction.EqualsI("Backward"))
            {
            thisInstanceIdColumnName = "TargetECInstanceId";
            thisClassIdColumnName = "TargetECClassId"; 
            relatedInstanceIdColumnName = "SourceECInstanceId";
            relatedClassIdColumnName = "SourceECClassId";
            }
        else
            {
            BeAssert(false);
            return false;
            }

        Utf8String relationshipSchemaAndClassName = args->GetArgument(0)->ToString().Trim("\"");
        Utf8String relationshipSchemaName, relationshipClassName;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(relationshipSchemaName, relationshipClassName, relationshipSchemaAndClassName))
            {
            BeAssert(false);
            return false;
            }
        m_usedClasses.push_back(relationshipSchemaAndClassName);
        
        Utf8String relatedSchemaAndClassName = args->GetArgument(2)->ToString().Trim("\"");
        Utf8String relatedClassSchemaName, relatedClassName;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(relatedClassSchemaName, relatedClassName, relatedSchemaAndClassName))
            {
            BeAssert(false);
            return false;
            }
        m_usedClasses.push_back(relatedSchemaAndClassName);

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
            relationshipSchemaName.c_str(), relationshipClassName.c_str(),
            relatedClassSchemaName.c_str(), relatedClassName.c_str(),
            thisClassIdColumnName, prefix.c_str(), thisInstanceIdColumnName, prefix.c_str(),
            relatedClassIdColumnName, relatedInstanceIdColumnName);
        m_ecsql.append(ecsql);
        m_ignoreArguments = true;
        return true;
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            03/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    bool HandleGetUserSettingIntValuesSpecialCase(CallNodeCR node)
        {
        ArgumentTreeNodeCP args = node.GetArguments();
        if (nullptr == args || 1 != args->GetArgumentCount())
            {
            BeAssert(false && "Only a single argument (setting ID) is allowed for GetUserSettingIntValues function");
            return false;
            }

        m_currentValueListMethodNode = &node;

        Append(FUNCTION_NAME_InSettingIntValues);
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
        if (nullptr == m_currentValueListMethodNode)
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

        m_inArguments = true;
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            05/2016
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleCallNode(CallNodeCR node)
        {
        // handle special cases
        if (0 == strcmp("IsOfClass", node.GetMethodName()) && HandleIsOfClassSpecialCase(node))
            return;
        if (0 == strcmp("GetECClassId", node.GetMethodName()) && HandleGetECClassIdSpecialCase(node))
            return;
        if (0 == strcmp("HasRelatedInstance", node.GetMethodName()) && HandleHasRelatedInstanceSpecialCase(node))
            return;
        if (0 == strcmp("GetRelatedValue", node.GetMethodName()) && HandleGetRelatedValueSpecialCase(node))
            return;
        if (0 == strcmp("GetUserSettingIntValues", node.GetMethodName()) && HandleGetUserSettingIntValuesSpecialCase(node))
            return;
        if (0 == strcmp("AnyMatch", node.GetMethodName()) && HandleAnyMatchSpecialCase(node))
            return;

        Append(node.ToString());
        m_inArguments = false;
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            03/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    void HandleLambdaNode(LambdaNodeCR node)
        {
        Utf8CP symbolName = node.GetSymbolName();
        ComparisonNodeCP expressionNode = dynamic_cast<ComparisonNodeCP>(&node.GetExpression());
        if (nullptr == expressionNode || TOKEN_Equal != expressionNode->GetOperation())
            {
            BeAssert(false && "Only an EQUALS operation is allowed in lambda expression");
            return;
            }

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

        Comma();
        propertyAccessNode->Traverse(*this);
        EndArguments(*m_currentValueListMethodNode->GetArguments());

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
            case TOKEN_Equal:
            case TOKEN_NotEqual:
            case TOKEN_Minus:
            case TOKEN_Plus:
            case TOKEN_Star:
            case TOKEN_Slash:
                Append(node.ToString());
                break;
            case TOKEN_StringConst:
                BeAssert(nullptr != dynamic_cast<LiteralNode const*>(&node) 
                    && dynamic_cast<LiteralNode const*>(&node)->GetInternalValue().IsUtf8());
                Append(Utf8PrintfString("'%s'", static_cast<LiteralNode const*>(&node)->GetInternalValue().GetUtf8CP()));
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

    NodePtr node = GetNodeFromExpression(expression.c_str());
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
bvector<Utf8String> ECExpressionsHelper::GetUsedClasses(Utf8StringCR expression)
    {
    if (expression.empty())
        return bvector<Utf8String>();

    NodePtr node = GetNodeFromExpression(expression.c_str());
    if (node.IsNull())
        {
        BeAssert(false);
        return bvector<Utf8String>();
        }

    ECExpressionToECSqlConverter converter;
    return converter.GetUsedClasses(*node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr ECExpressionsHelper::GetNodeFromExpression(Utf8CP expr)
    {
    NodePtr node = m_cache.Get(expr);
    if (node.IsValid())
        return node;

    Utf8String expression = expr;

    // check node's class instead of instance's class - this allows us to avoid retrieving the ECInstance
    // if we're only checking for class.
    expression.ReplaceAll(".ECInstance.IsOfClass(", ".IsOfClass(");
    expression.ReplaceAll("this.IsOfClass(", "ThisNode.IsOfClass(");
    
    // LIKE operator special case
    expression.ReplaceAll("~", " LIKE ");

    node = ECEvaluator::ParseValueExpressionAndCreateTree(expression.c_str());
    m_cache.Add(expr, *node);
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NodePtr ECExpressionsCache::Get(Utf8CP expression) const
    {
    auto iter = m_cache.find(expression);
    if (m_cache.end() == iter)
        return nullptr;
    return iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OptimizedExpressionPtr ECExpressionsCache::GetOptimized(Utf8CP expression) const
    {
    auto iter = m_optimizedCache.find(expression);
    if (m_optimizedCache.end() == iter)
        return nullptr;
    return iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECExpressionsCache::HasOptimizedExpression(Utf8CP expression) const {return m_optimizedCache.end() != m_optimizedCache.find(expression);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ECExpressionsCache::Add(Utf8CP expression, Node& node) {m_cache.Insert(expression, &node);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ECExpressionsCache::Add(Utf8CP expression, OptimizedExpression& node) { m_optimizedCache.Insert(expression, &node); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ECExpressionsCache::Clear() {m_cache.clear();}
