/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once


#include "ECInstance.h"

#include <Bentley/BeThread.h>

#define LOGGER_NAMESPACE_ECOBJECTS_ECEXPRESSIONS               "ECObjects.ECExpressions"
#define LOGGER_NAMESPACE_ECOBJECTS_ECEXPRESSIONS_PARSE         LOGGER_NAMESPACE_ECOBJECTS_ECEXPRESSIONS ".Parse"
#define LOGGER_NAMESPACE_ECOBJECTS_ECEXPRESSIONS_EVALUATE      LOGGER_NAMESPACE_ECOBJECTS_ECEXPRESSIONS ".Evaluate"

#ifdef ENABLE_ECEXPRESSIONS_LOGGING
#define ECEXPRESSIONS_PARSE_LOG(sev, msg) \
    if (NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE_ECOBJECTS_ECEXPRESSIONS_PARSE)->isSeverityEnabled(sev)) \
        NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE_ECOBJECTS_ECEXPRESSIONS_PARSE)->message(sev, msg);
#define ECEXPRESSIONS_EVALUATE_LOG(sev, msg) \
    if (NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE_ECOBJECTS_ECEXPRESSIONS_EVALUATE)->isSeverityEnabled(sev)) \
        NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE_ECOBJECTS_ECEXPRESSIONS_EVALUATE)->message(sev, msg);
#else
#define ECEXPRESSIONS_PARSE_LOG(sev, msg) { UNUSED_VARIABLE(sev); UNUSED_VARIABLE(msg); }
#define ECEXPRESSIONS_EVALUATE_LOG(sev, msg) { UNUSED_VARIABLE(sev); UNUSED_VARIABLE(msg); }
#endif

#define EXPR_TYPEDEFS(_name_)  \
        BEGIN_BENTLEY_ECOBJECT_NAMESPACE      \
            struct _name_;      \
            typedef _name_ *         _name_##P;  \
            typedef _name_ &         _name_##R;  \
            typedef _name_ const*    _name_##CP; \
            typedef _name_ const&    _name_##CR; \
        END_BENTLEY_ECOBJECT_NAMESPACE

EXPR_TYPEDEFS(ArgumentTreeNode)
EXPR_TYPEDEFS(ArithmeticNode)
EXPR_TYPEDEFS(BinaryNode)
EXPR_TYPEDEFS(CallNode)
EXPR_TYPEDEFS(LambdaNode)
EXPR_TYPEDEFS(ComparisonNode)
EXPR_TYPEDEFS(ConcatenateNode)
EXPR_TYPEDEFS(ContextSymbol)
EXPR_TYPEDEFS(CustomSymbol)
EXPR_TYPEDEFS(DivideNode)
EXPR_TYPEDEFS(DotNode)
EXPR_TYPEDEFS(ErrorNode)
EXPR_TYPEDEFS(EvaluationResult)
EXPR_TYPEDEFS(ExpressionContext)
EXPR_TYPEDEFS(ExpressionResolver)
EXPR_TYPEDEFS(ExpressionType)
EXPR_TYPEDEFS(IValueListResult)
EXPR_TYPEDEFS(LambdaValue)
EXPR_TYPEDEFS(IdentNode)
EXPR_TYPEDEFS(IIfNode)
EXPR_TYPEDEFS(InstanceListExpressionContext)
EXPR_TYPEDEFS(InstanceExpressionContext)
EXPR_TYPEDEFS(LBracketNode)
EXPR_TYPEDEFS(Lexer)
EXPR_TYPEDEFS(LogicalNode)
EXPR_TYPEDEFS(MethodReference)
EXPR_TYPEDEFS(MethodSymbol)
EXPR_TYPEDEFS(MultiplyNode)
EXPR_TYPEDEFS(Node)
EXPR_TYPEDEFS(NodeVisitor)
EXPR_TYPEDEFS(PlusMinusNode)
EXPR_TYPEDEFS(PrimaryListNode)
EXPR_TYPEDEFS(ReferenceResult)
EXPR_TYPEDEFS(ResolvedTypeNode)
EXPR_TYPEDEFS(ShiftNode)
EXPR_TYPEDEFS(Symbol)
EXPR_TYPEDEFS(SymbolExpressionContext)
EXPR_TYPEDEFS(UnaryArithmeticNode)
EXPR_TYPEDEFS(ValueResult)
EXPR_TYPEDEFS(ValueSymbol)
EXPR_TYPEDEFS(IECSymbolProvider)

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef RefCountedPtr<ArgumentTreeNode>             ArgumentTreeNodePtr;
typedef RefCountedPtr<CallNode>                     CallNodePtr;
typedef RefCountedPtr<LambdaNode>                   LambdaNodePtr;
typedef RefCountedPtr<ContextSymbol>                ContextSymbolPtr;
typedef RefCountedPtr<DotNode>                      DotNodePtr;
typedef RefCountedPtr<ErrorNode>                    ErrorNodePtr;
typedef RefCountedPtr<ExpressionType>               ExpressionTypePtr;
typedef RefCountedPtr<IValueListResult>             IValueListResultPtr;
typedef RefCountedPtr<LambdaValue>                  LambdaValuePtr;
typedef RefCountedPtr<ExpressionContext>            ExpressionContextPtr;
typedef RefCountedPtr<IdentNode>                    IdentNodePtr;
typedef RefCountedPtr<InstanceListExpressionContext> InstanceListExpressionContextPtr;
typedef RefCountedPtr<InstanceExpressionContext>    InstanceExpressionContextPtr;
typedef RefCountedPtr<LBracketNode>                 LBracketNodePtr;
typedef RefCountedPtr<Lexer>                        LexerPtr;
typedef RefCountedPtr<MethodReference>              MethodReferencePtr;
typedef RefCountedPtr<MethodSymbol>                 MethodSymbolPtr;
typedef RefCountedPtr<Node>                         NodePtr;
typedef RefCountedPtr<PrimaryListNode>              PrimaryListNodePtr;
typedef RefCountedPtr<ResolvedTypeNode>             ResolvedTypeNodePtr;
typedef RefCountedPtr<Symbol>                       SymbolPtr;
typedef RefCountedPtr<SymbolExpressionContext>      SymbolExpressionContextPtr;
typedef RefCountedPtr<ValueResult>                  ValueResultPtr;
typedef RefCountedPtr<ValueSymbol>                  ValueSymbolPtr;

typedef bvector<NodeP>                              NodeVector;
typedef bvector<NodeCP>                             NodeCPVector;
typedef NodeVector::iterator                        NodeVectorIterator;

typedef bvector<NodePtr>                            NodePtrVector;
typedef NodePtrVector::iterator                     NodePtrVectorIterator;

typedef bvector<EvaluationResult>                   EvaluationResultVector;
typedef EvaluationResultVector::iterator            EvaluationResultVectorIterator;

enum ValueType
    {
    ValType_None            =  0,
    ValType_ECValue         =  1,
    ValType_Custom          =  2,
    ValType_InstanceList    =  3,
    ValType_ValueList       =  4,
    ValType_Lambda          =  5,
    ValType_Context         =  6,
    };

typedef ExpressionStatus (*ExpressionStaticMethod_t)(EvaluationResult& evalResult, void* context, EvaluationResultVector& arguments);
typedef ExpressionStatus (*ExpressionInstanceMethod_t)(EvaluationResult& evalResult, void* context, ECInstanceListCR instanceList, EvaluationResultVector& arguments);
// NB: We could generalize Instance methods to take any EvaluationResult as the calling object, but we'd have to refactor all our existing instance method implementations to
// check that the caller is an ECInstanceList...rather keep value list methods separate from instance list methods...
typedef ExpressionStatus (*ExpressionValueListMethod_t)(EvaluationResult& evalResult, void* context, IValueListResultCR valueList, EvaluationResultVector& arguments);

#ifndef DOCUMENTATION_GENERATOR

/*=================================================================================**//**
*
* This is result of _GetMethodReference. It is used to invoke a method. If the method
* is an instance method, the MethodReference holds a reference to the instance used to
* invoke the method. That is not supplied on a subsequent call to invoke.
*
+===============+===============+===============+===============+===============+======*/
struct          MethodReference : RefCountedBase
{
protected:
                                MethodReference() {}
    virtual bool                _CanReuseResult ()               { return false; }
    virtual ExpressionStatus    _InvokeStaticMethod (EvaluationResultR evalResult, EvaluationResultVector& arguments) { return ExpressionStatus::NotImpl; }
    virtual ExpressionStatus    _InvokeInstanceMethod (EvaluationResultR evalResult, ECInstanceListCR instanceData, EvaluationResultVector& arguments) { return ExpressionStatus::NotImpl; }
    virtual ExpressionStatus    _InvokeValueListMethod (EvaluationResultR evalResult, IValueListResultCR valueList, EvaluationResultVector& arguments) { return ExpressionStatus::NotImpl; }
    virtual bool                _SupportsStaticMethodCall () const = 0;
    virtual bool                _SupportsInstanceMethodCall () const = 0;
    virtual bool                _SupportsValueListMethodCall () const = 0;

public:
    bool                        SupportsStaticMethodCall () const { return _SupportsStaticMethodCall(); }
    bool                        SupportsInstanceMethodCall () const { return _SupportsInstanceMethodCall(); }
    bool                        SupportsValueListMethodCall () const { return _SupportsValueListMethodCall(); }

    bool                        CanReuseResult()                { return _CanReuseResult(); }
    ExpressionStatus            InvokeStaticMethod (EvaluationResult& evalResult, EvaluationResultVector& arguments)
                                            { return _InvokeStaticMethod(evalResult, arguments); }
    ExpressionStatus            InvokeInstanceMethod (EvaluationResult& evalResult, ECInstanceListCR instanceData, EvaluationResultVector& arguments)
                                            { return _InvokeInstanceMethod(evalResult, instanceData, arguments); }
    ExpressionStatus            InvokeValueListMethod (EvaluationResult& evalResult, IValueListResultCR valueList, EvaluationResultVector& arguments)
                                            { return _InvokeValueListMethod (evalResult, valueList, arguments); }
}; // MethodReference

/*=================================================================================**//**
*
* This is result of _GetMethodReference. It is used to invoke a method.
*
+===============+===============+===============+===============+===============+======*/
struct          MethodReferenceStandard : MethodReference
{
private:
    ExpressionStaticMethod_t    m_staticMethod;
    ExpressionInstanceMethod_t  m_instanceMethod;
    ExpressionValueListMethod_t m_valueListMethod;
    void* m_context;

protected:
    MethodReferenceStandard(ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod, void* context);
    MethodReferenceStandard(ExpressionValueListMethod_t valueListMethod, void* context);

    bool _CanReuseResult() override {return true;}
    bool _SupportsStaticMethodCall() const override {return NULL != m_staticMethod;}
    bool _SupportsInstanceMethodCall() const override {return NULL != m_instanceMethod;}
    bool _SupportsValueListMethodCall() const override {return NULL != m_valueListMethod;}

    //  The vector of arguments does not include the object used to invoke the method. It is
    //  up to the specific implementation of MethodReference to hold onto the instance and to use
    //  that to invoke the method.
    ExpressionStatus    _InvokeStaticMethod (EvaluationResultR evalResult, EvaluationResultVector& arguments) override;
    ExpressionStatus    _InvokeInstanceMethod (EvaluationResultR evalResult, ECInstanceListCR instanceData, EvaluationResultVector& arguments) override;
    ExpressionStatus    _InvokeValueListMethod (EvaluationResultR evalResult, IValueListResultCR valueList, EvaluationResultVector& arguments) override;
public:

    static MethodReferencePtr   Create(ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod, void* context = nullptr);
    static MethodReferencePtr   Create(ExpressionValueListMethod_t valueListMethod, void* context = nullptr);
};

#endif

//! @addtogroup ECObjectsGroup
//! @beginGroup

/*=================================================================================**//**
* Type for a Method
+===============+===============+===============+===============+===============+======*/
enum class ExpressionMethodType
    {
    Static,
    Instance,
    ValueList,
    };

/*=================================================================================**//**
* The context in which an expression is evaluated.
+===============+===============+===============+===============+===============+======*/
struct          ExpressionContext : RefCountedBase
{
private:
    ExpressionContextPtr                m_outer;

#ifndef DOCUMENTATION_GENERATOR
protected:
    virtual                     ~ExpressionContext () {}
                                ExpressionContext(ExpressionContextP outer) : m_outer(outer) { }
    virtual ExpressionStatus    _ResolveMethod(MethodReferencePtr& result, Utf8CP ident, bool useOuterIfNecessary, ExpressionMethodType methodType) { return ExpressionStatus::UnknownSymbol; }
    virtual bool                _IsNamespace() const { return false; }
    //  If we provide this it must be implemented in every class that implements the _GetReference that uses more arguments.
    //  virtual ExpressionStatus    _GetReference(PrimaryListNodeR primaryList, bool useOuterIfNecessary) const { return ExpressionStatus::NotImpl; }
    //  The globalContext may be used to find instance methods
    virtual ExpressionStatus    _GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) = 0;
    virtual ExpressionStatus    _GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) { return ExpressionStatus::NotImpl; }
#endif
public:
#ifndef DOCUMENTATION_GENERATOR
    bool                        IsNamespace () const  { return _IsNamespace(); }
    ExpressionContextP          GetOuterP () const   { return m_outer.get(); }
    ExpressionStatus            ResolveMethod(MethodReferencePtr& result, Utf8CP ident, bool useOuterIfNecessary, ExpressionMethodType methodType)
                                    { return _ResolveMethod(result, ident, useOuterIfNecessary, methodType); }

    ExpressionStatus            GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex = 0)
                                    { return _GetValue(evalResult, primaryList, contextsStack, startIndex); }

    ExpressionStatus            GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex = 0)

                                    { return _GetReference(evalResult, refResult, primaryList, contextsStack, startIndex); }
#endif
}; // End of class ExpressionContext

//=================================================================================//
//! A context in which multiple IECInstances provide the context for expression evaluation.
//+===============+===============+===============+===============+===============+======//
struct          InstanceListExpressionContext : ExpressionContext
    {
private:
    ECInstanceList              m_instances;
    bool                        m_initialized;

    InstanceListExpressionContext (ECInstanceListCR instances, ExpressionContextP outer = NULL);

    void Initialize();

    ECOBJECTS_EXPORT ExpressionStatus   _GetValue (EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) override;
    ECOBJECTS_EXPORT ExpressionStatus   _GetReference (EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) override;

    ExpressionStatus   GetReference (EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex, IECInstanceCR instance);
#ifndef DOCUMENTATION_GENERATOR
protected:
    // The following protected methods are only relevant to derived classes, which may want to:
    //  -lazily load the instance list, and/or
    //  -Reuse the context for differing lists of instances
    ECOBJECTS_EXPORT InstanceListExpressionContext (ExpressionContextP outer = NULL);

    virtual void                                _GetInstances (ECInstanceListR instances) { }

    bool                                        IsInitialized() const { return m_initialized; }
    void                                        Reset() { m_instances.clear(); m_initialized = false; }
    void                                        SetInstanceList (ECInstanceListCR instances) { m_instances = instances; m_initialized = true; }
#endif
public:
    //!<@private
    void                                        Clear() { m_instances.clear(); }

    //! Creates a new InstanceListExpressionContext from the list of IECInstances
    //! @param[in]      instances A list of IECInstances to be associated with the context
    //! @param[in]      outer     An optional ExpressionContext to contain the created context
    //! @return     A new InstanceListExpressionContext
    ECOBJECTS_EXPORT static InstanceListExpressionContextPtr    Create (bvector<IECInstancePtr> const& instances, ExpressionContextP outer = NULL);
    };

#ifndef DOCUMENTATION_GENERATOR
/*---------------------------------------------------------------------------------**//**
* Manager that holds symbol providers. For use only with internal Symbol Providers.
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct InternalECSymbolProviderManager
    {
private:
    BeMutex m_mutex;
    bvector<IECSymbolProviderCP>         m_symbolProviders;

    InternalECSymbolProviderManager();

public:
    // This method is injected into ECObjects to provide symbols.
    void PublishSymbols (SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets);

    // Register an IECSymbolProvider.
    ECOBJECTS_EXPORT void RegisterSymbolProvider (IECSymbolProviderCR provider);

    // Unregister an IECSymbolProvider.
    ECOBJECTS_EXPORT void UnregisterSymbolProvider (IECSymbolProviderCR provider);

    ECOBJECTS_EXPORT static InternalECSymbolProviderManager& GetManager ();
    };
#endif

/*---------------------------------------------------------------------------------**//**
* An InstanceListExpressionContext which simply wraps one or more IECInstances.
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceExpressionContext : InstanceListExpressionContext
    {
private:
    InstanceExpressionContext (ExpressionContextP outer) : InstanceListExpressionContext (outer) { }

public:
    //! Creates an InstanceExpressionContext
    //! @param[in]      outer An optional ExpressionContext which will contain the created InstanceExpressionContext
    //! @return     A new InstanceExpressionContext
    ECOBJECTS_EXPORT static InstanceExpressionContextPtr        Create (ExpressionContextP outer = NULL);

    //! Sets the IECInstance associated with this context
    //! @param[in]      instance The IECInstance to associate with this context
    ECOBJECTS_EXPORT void           SetInstance (IECInstanceCR instance);

    //! Sets the list of IECInstances associated with this context
    //! @param[in]      instances The list of IECInstances to associate with this context
    ECOBJECTS_EXPORT void           SetInstances (ECInstanceListCR instances);
    };

/*=================================================================================**//**
* A context which provides a set of symbols for expression evaluation.
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE SymbolExpressionContext : ExpressionContext
{
private:
    bvector<SymbolPtr>          m_symbols;

#ifndef DOCUMENTATION_GENERATOR
protected:
    ECOBJECTS_EXPORT ExpressionStatus _ResolveMethod(MethodReferencePtr& result, Utf8CP ident, bool useOuterIfNecessary, ExpressionMethodType methodType) override;
    ECOBJECTS_EXPORT ExpressionStatus _GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) override;
    ECOBJECTS_EXPORT ExpressionStatus _GetReference(EvaluationResultR evalResult, ReferenceResultR refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) override;

    ECOBJECTS_EXPORT bool _IsNamespace() const override {return true;}
    
    SymbolExpressionContext(ExpressionContextP outer) : ExpressionContext(outer) {}
#endif
public:
#ifndef DOCUMENTATION_GENERATOR
    ECOBJECTS_EXPORT SymbolCP       FindCP (Utf8CP ident);
    ECOBJECTS_EXPORT BentleyStatus  RemoveSymbol (SymbolR symbol);
    ECOBJECTS_EXPORT BentleyStatus  RemoveSymbol (Utf8CP ident);

    //! Creates a new SymbolExpressionContext using requested symbol sets and if an ECInstance is passed in it will publish it for "this" access
    ECOBJECTS_EXPORT static SymbolExpressionContextPtr   CreateWithThis (bvector<Utf8String> const& requestedSymbolSets, IECInstanceP instance = nullptr);

#endif
    //! Adds a symbol to the context to be used in expression evaluation
    ECOBJECTS_EXPORT BentleyStatus  AddSymbol (SymbolR symbol);
    //! Creates a new SymbolExpressionContext from the given ExpressionContext
    ECOBJECTS_EXPORT static SymbolExpressionContextPtr Create(ExpressionContextP outer);
    //! Creates a new SymbolExpressionContext from the given ExpressionContext and using sybmols from all the specified symbol sets. If the vector 
    //! is empty all available symbol are made available.
    ECOBJECTS_EXPORT static SymbolExpressionContextPtr Create(bvector<Utf8String> const& requestedSymbolSets, ExpressionContextP outer = NULL);
}; // End of class SymbolExpressionContext


/*=================================================================================**//**
*
* Base class for all symbol types
*
+===============+===============+===============+===============+===============+======*/
struct          Symbol : RefCountedBase
{
#ifndef DOCUMENTATION_GENERATOR
private:
    Utf8String     m_name;

protected:
    Symbol (Utf8CP name) : m_name (name) { }

    virtual ExpressionStatus _CreateMethodResult (MethodReferencePtr& result, ExpressionMethodType methodType) const {return ExpressionStatus::MethodRequired;}
    virtual ExpressionStatus _GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) = 0;
    virtual ExpressionStatus _GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) = 0;

    uint32_t _GetExcessiveRefCountThreshold() const override {return 10000;}

public:
    Utf8CP GetName() const {return m_name.c_str();}

    ExpressionStatus CreateMethodResult (MethodReferencePtr& result, ExpressionMethodType methodType) const {return _CreateMethodResult(result, methodType);}

    ExpressionStatus GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex)
                        {return _GetValue(evalResult, primaryList, contextsStack, startIndex);}

    ExpressionStatus GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex)
                        {return _GetReference(evalResult, refResult, primaryList, contextsStack, startIndex);}
#endif

};  // End of class Symbol

/*=================================================================================**//**
* Used to give a name to an instance.
+===============+===============+===============+===============+===============+======*/
struct          ContextSymbol : Symbol
{
#ifndef DOCUMENTATION_GENERATOR
protected:
    ExpressionContextPtr        m_context;

    ContextSymbol (Utf8CP name, ExpressionContextR context) : Symbol(name), m_context(&context) {}

    ExpressionStatus _GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) override;
    ExpressionStatus _GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) override;
#endif

public:
    //! Creates a new ContextSymbol
    //! @param[in] name     The name to be used for this context symbol
    //! @param[in] context  The expression context to be used for this context
    //! @returns A new ContextSymbolPtr
    ECOBJECTS_EXPORT static ContextSymbolPtr        CreateContextSymbol(Utf8CP name, ExpressionContextR context);
};

/*=================================================================================**//**
* Used to introduce a named method into the context.

+===============+===============+===============+===============+===============+======*/
struct          MethodSymbol : Symbol
{
private:
    MethodReferencePtr  m_methodReference;

#ifndef DOCUMENTATION_GENERATOR
protected:
    ExpressionStatus _GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) override;
    ExpressionStatus _GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) override {return ExpressionStatus::NeedsLValue;}
    ExpressionStatus _CreateMethodResult(MethodReferencePtr& result, ExpressionMethodType methodType) const override;

    MethodSymbol(Utf8CP name, MethodReferenceR methodReference);
    MethodSymbol(Utf8CP name, ExpressionValueListMethod_t valueListMethod);
#endif
public:
    //!<@private
    ECOBJECTS_EXPORT static MethodSymbolPtr     Create (Utf8CP name, ExpressionValueListMethod_t valueListMethod);

    //! Creates a new method symbol context, using the supplied methods
    ECOBJECTS_EXPORT static MethodSymbolPtr    Create(Utf8CP name, ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod, void* context = nullptr);
};

/*=================================================================================**//**
* Used to introduce a named property into the context.
+===============+===============+===============+===============+===============+======*/
struct PropertySymbol : Symbol
{
    /*=============================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+==*/
    struct ValueEvaluator : RefCountedBase
        {
        virtual ~ValueEvaluator() {}
        virtual ECValue _EvaluateValue() = 0;
        };

    /*=============================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+==*/
    struct ValueListEvaluator : RefCountedBase
        {
        virtual ~ValueListEvaluator() {}
        virtual IValueListResultPtr _EvaluateValueList() = 0;
        };

    /*=============================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+==*/
    struct ContextEvaluator : RefCountedBase
        {
        virtual ~ContextEvaluator() {}
        virtual ExpressionContextPtr _GetContext() = 0;
        };

private:
    /*=============================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+==*/
    template<class InstanceType, class ReturnValueType>
    struct TemplatedValueEvaluator : ValueEvaluator
        {
        typedef ReturnValueType(InstanceType::*GetValueMethod)() const;
        private:
            InstanceType const& m_instance;
            GetValueMethod m_method;
            TemplatedValueEvaluator(InstanceType const& instance, GetValueMethod method)
                : m_instance(instance), m_method(method)
                {}
        public:
            ECValue _EvaluateValue() override {return ECValue((m_instance.*m_method)());}
            static RefCountedPtr<ValueEvaluator> Create(InstanceType const& instance, GetValueMethod method) {return new TemplatedValueEvaluator(instance, method);}
        };

    /*=============================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+==*/
    template<class InstanceType>
    struct TemplatedValueListEvaluator : ValueListEvaluator
        {
        typedef IValueListResultPtr(InstanceType::*GetValueListMethod)() const;
        private:
            InstanceType const& m_instance;
            GetValueListMethod m_method;
            TemplatedValueListEvaluator(InstanceType const& instance, GetValueListMethod method)
                : m_instance(instance), m_method(method)
                {}
        public:
            IValueListResultPtr _EvaluateValueList() override {return (m_instance.*m_method)();}
            static RefCountedPtr<ValueListEvaluator> Create(InstanceType const& instance, GetValueListMethod method) {return new TemplatedValueListEvaluator(instance, method);}
        };

    /*=============================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+==*/
    template<class InstanceType>
    struct TemplatedContextEvaluator : ContextEvaluator
        {
        typedef ExpressionContextPtr(InstanceType::*GetContextMethod)() const;
        private:
            InstanceType const& m_instance;
            GetContextMethod    m_method;
            TemplatedContextEvaluator(InstanceType const& instance, GetContextMethod method)
                : m_instance(instance), m_method(method)
                {}
        public:
            ExpressionContextPtr _GetContext() override {return (m_instance.*m_method)();}
            static RefCountedPtr<ContextEvaluator> Create(InstanceType const& instance, GetContextMethod method) {return new TemplatedContextEvaluator(instance, method);}
        };
    

    RefCountedPtr<ValueEvaluator> m_valueEvaluator;
    RefCountedPtr<ValueListEvaluator> m_valueListEvaluator;
    RefCountedPtr<ContextEvaluator> m_contextEvaluator;

#ifndef DOCUMENTATION_GENERATOR
protected:
    PropertySymbol(Utf8CP name, ValueEvaluator& evaluator);
    PropertySymbol(Utf8CP name, ValueListEvaluator& evaluator);
    PropertySymbol(Utf8CP name, ContextEvaluator& evaluator);
    ECOBJECTS_EXPORT ExpressionStatus _GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) override;
    ExpressionStatus _GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) override {return ExpressionStatus::NeedsLValue;}

#endif
public:
    ECOBJECTS_EXPORT static RefCountedPtr<PropertySymbol> Create(Utf8CP name, ValueEvaluator& evaluator);
    ECOBJECTS_EXPORT static RefCountedPtr<PropertySymbol> Create(Utf8CP name, ValueListEvaluator& evaluator);
    ECOBJECTS_EXPORT static RefCountedPtr<PropertySymbol> Create(Utf8CP name, ContextEvaluator& evaluator);

    //! Creates a new PropertySymbol with the given name, instance and method reference to get the property value
    //! @param name     Name of the symbol.
    //! @param instance Instance of type InstanceType that will be used to invoke the provided method.
    //! @param method   The method that provides value for this property.
    template<class InstanceType, class ReturnValueType>
    static RefCountedPtr<PropertySymbol> Create(Utf8CP name, InstanceType const& instance, ReturnValueType(InstanceType::*method)() const) 
        {
        return Create(name, *TemplatedValueEvaluator<InstanceType, ReturnValueType>::Create(instance, method));
        }

    //! Creates a new PropertySymbol with the given name, instance and method reference to get the value list
    //! @param name     Name of the symbol.
    //! @param instance Instance of type InstanceType that will be used to invoke the provided method.
    //! @param method   The method that provides the value list.
    template<class InstanceType>
    static RefCountedPtr<PropertySymbol> Create(Utf8CP name, InstanceType const& instance, IValueListResultPtr(InstanceType::*method)() const)
        {
        return Create(name, *TemplatedValueListEvaluator<InstanceType>::Create(instance, method));
        }

    //! Creates a new PropertySymbol with the given name, instance and method reference to get the property value
    //! @param name     Name of the symbol.
    //! @param instance Instance of type InstanceType that will be used to invoke the provided method.
    //! @param method   The method that provides the expression context for this property.
    template<class InstanceType>
    static RefCountedPtr<PropertySymbol> Create(Utf8CP name, InstanceType const& instance, ExpressionContextPtr(InstanceType::*method)() const) 
        {
        return Create(name, *TemplatedContextEvaluator<InstanceType>::Create(instance, method));
        }
};


/*=================================================================================**//**
* Provides a set of Symbols
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE IECSymbolProvider
    {
    //!<@private
    typedef void(*ExternalSymbolPublisher)(SymbolExpressionContextR, bvector<Utf8String> const&);

protected:
    virtual Utf8CP                  _GetName() const = 0;
    virtual void                    _PublishSymbols (SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const = 0;
public:
    ECOBJECTS_EXPORT virtual ~IECSymbolProvider() { /* required due to other virtual functions */ }
    ECOBJECTS_EXPORT Utf8CP         GetName() const
                                        { return _GetName(); }
    ECOBJECTS_EXPORT void           PublishSymbols (SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const
                                        { return _PublishSymbols (context, requestedSymbolSets); }
    //!<@private
    ECOBJECTS_EXPORT static void    RegisterExternalSymbolPublisher (ExternalSymbolPublisher externalPublisher);
    //!<@private
    ECOBJECTS_EXPORT static void    UnRegisterExternalSymbolPublisher ();
    };


enum            ExpressionToken
    {
    TOKEN_None                = 0,
    TOKEN_Null                = 16,               // Null
    TOKEN_True                = 17,
    TOKEN_False               = 18,
    TOKEN_Like                = 19,
    TOKEN_Is                  = 20,
    TOKEN_Star                = 40,
    TOKEN_Plus                = 41,
    TOKEN_Minus               = 42,
    TOKEN_Slash               = 43,               //  /
    TOKEN_Comma               = 44,               //  ,
    TOKEN_IntegerDivide       = 45,               //  \ -- backslash
    TOKEN_LParen              = 46,               //  (
    TOKEN_RParen              = 47,               //  )
    TOKEN_Exponentiation      = 48,               //  ^
    TOKEN_And                 = 50,               //  And
    TOKEN_AndAlso             = 51,               //  AndAlso -- short circuiting And
    TOKEN_Or                  = 52,               //  Or
    TOKEN_OrElse              = 53,               //  OrElse -- short circuiting Or
    TOKEN_Concatenate         = 54,
    TOKEN_Mod                 = 55,               //  Mod
    TOKEN_ShiftLeft           = 56,               //  <<
    TOKEN_ShiftRight          = 57,               //  >>
    TOKEN_Colon               = 58,               //  : decorates a sub-expression with a unit specification ("expr:UnitName[::Factor[::Offset]]")
    TOKEN_LessEqual           = 59,
    TOKEN_GreaterEqual        = 60,
    TOKEN_Less                = 61,
    TOKEN_Greater             = 62,
    TOKEN_Equal               = 63,
    TOKEN_NotEqual            = 64,               //      <>
    TOKEN_Not                 = 65,               //      "Not"
    TOKEN_Xor                 = 66,               //      "Xor"
    TOKEN_UnsignedShiftRight  = 68,               //  >>>
    TOKEN_LeftBracket         = 69,               //      [
    TOKEN_RightBracket        = 70,               //      ]
    TOKEN_Dot                 = 71,               //      .
    TOKEN_IIf                 = 72,               //      IFF
    TOKEN_LCurly              = 73,               //      {
    TOKEN_RCurly              = 74,               //      }

    TOKEN_If                  = 75,
    TOKEN_Else                = 76,
    TOKEN_ElseIf              = 77,
    TOKEN_Select              = 78,
    TOKEN_End                 = 79,
    TOKEN_EndIf               = 80,

    TOKEN_Lambda              = 85,               // => e.g. "SomeArray.Select (X => X.Name)", "SomeArray.AnyMatches (val => val > 10)"

    TOKEN_DoubleColon         = 100,

    TOKEN_Error               = 181,
    TOKEN_Ident               = 182,
    TOKEN_StringConst         = 183,
    TOKEN_PointConst          = 184,
    TOKEN_DateTimeConst       = 185,               //      @
    TOKEN_IntegerConstant     = 187,
    TOKEN_HexConstant         = 188,
    TOKEN_FloatConst          = 189,
    TOKEN_UnitsConst          = 190,               // what is this used for...anything...?
    TOKEN_Unrecognized        = 200,
    TOKEN_BadNumber           = 201,
    TOKEN_BadOctalNumber      = 202,
    TOKEN_BadHexNumber        = 203,
    TOKEN_BadFloatingPointNumber = 204,
    TOKEN_UnterminatedString   = 205,
    TOKEN_PrimaryList         = 206,
    };

#ifndef DOCUMENTATION_GENERATOR

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct          ExpressionType : RefCountedBase
{
private:
    int                 m_unitsPower;   // 0 -- unknown, 1 -- linear, 2 -- squared, 3 -- cubed
    ECN::IECInstancePtr  m_extendedType;
    ECN::ValueKind       m_valueKind;
    ECN::PrimitiveType   m_primitiveType;
    ECN::ArrayKind       m_arrayKind;    //  Relevant only if m_valueKind == VALUEKIND_Array
    ECN::ECClassCP       m_structClass;  //  Relevant if m_valueKind == VALUEKIND_Struct or
                                        //  m_valueKind == VALUEKIND_Array and m_arrayKind == ARRAYKIND_Struct
private:
    void                Init();

protected:

public:
    ExpressionType (ECN::PrimitiveECPropertyR primitiveProp);

}; // ExpressionType

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct          ReferenceResult
{
    ECN::ECPropertyCP   m_property;
    Utf8String          m_accessString;
    ::uint32_t          m_arrayIndex;
    int                 m_memberSelector;   // 1 for x, 2, for y, 3 for z
};

/*=================================================================================**//**
* Result of an ECExpression that contains a list of values, e.g., an array.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IValueListResult : RefCountedBase
    {
protected:
    virtual uint32_t            _GetCount() const = 0;
    virtual ExpressionStatus    _GetValueAt (EvaluationResultR result, uint32_t index) const = 0;

    IValueListResult() { }
public:
    ECOBJECTS_EXPORT uint32_t                   GetCount() const;
    ECOBJECTS_EXPORT ExpressionStatus           GetValueAt (EvaluationResultR result, uint32_t index) const;
    ECOBJECTS_EXPORT Utf8String                 ToString() const;
    ECOBJECTS_EXPORT static IValueListResultPtr Create (IECInstanceR owningInstance, uint32_t arrayPropertyIndex);
    ECOBJECTS_EXPORT static IValueListResultPtr Create (EvaluationResultVector const& values);
    };

#endif
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct          EvaluationResult
{
private:
//  Provides a list of conditions for which the shortcuts or bindings are valid
    ECN::ECValue        m_ecValue;
    union
        {
        IValueListResultP           m_valueList;
        ECInstanceListCP            m_instanceList;
        LambdaValueP                m_lambda;
        ExpressionContextPtr        m_context;
        };
    bool                m_ownsInstanceList;
    ValueType           m_valueType;

public:
#ifndef DOCUMENTATION_GENERATOR
    ValueType           GetValueType() const    { return m_valueType; }
    bool                IsInstanceList() const  { return ValType_InstanceList == m_valueType; }
    bool                IsECValue() const       { return ValType_ECValue == m_valueType; }
    bool                IsValueList() const     { return ValType_ValueList == m_valueType; }
    bool                IsLambda() const        { return ValType_Lambda == m_valueType; }
    bool                IsContext() const       { return ValType_Context == m_valueType; }

    ExpressionStatus    GetInteger(int32_t& result);
    ExpressionStatus    GetBoolean(bool& result, bool requireBoolean = true);

#endif
    //  Constructors and destructors
    ECOBJECTS_EXPORT EvaluationResult ();
    ECOBJECTS_EXPORT ~EvaluationResult ();
    ECOBJECTS_EXPORT EvaluationResult(EvaluationResultCR rhs);

    ECOBJECTS_EXPORT void               Clear();
    ECOBJECTS_EXPORT EvaluationResultR  operator=(EvaluationResultCR rhs);
    ECOBJECTS_EXPORT EvaluationResultR  operator= (ECN::ECValueCR rhs);

    ECOBJECTS_EXPORT ExpressionStatus       GetECValue(ECN::ECValueR result);
    ECOBJECTS_EXPORT ECValueP               GetECValue();
    ECOBJECTS_EXPORT ECValueCP              GetECValue() const;
    ECOBJECTS_EXPORT ECValueR               InitECValue();

    ECOBJECTS_EXPORT ECInstanceListCP       GetInstanceList() const;
    ECOBJECTS_EXPORT void                   SetInstanceList (ECInstanceListCR instanceList, bool makeACopy);
    ECOBJECTS_EXPORT void                   SetInstance (IECInstanceCR instance);

    ECOBJECTS_EXPORT IValueListResultCP     GetValueList() const;
    ECOBJECTS_EXPORT IValueListResultP      GetValueList();
    ECOBJECTS_EXPORT void                   SetValueList (IValueListResultR valueList);

    ECOBJECTS_EXPORT LambdaValueCP          GetLambda() const;
    ECOBJECTS_EXPORT void                   SetLambda (LambdaValueR value);

    ECOBJECTS_EXPORT ExpressionContextP     GetContext() const;
    ECOBJECTS_EXPORT void                   SetContext(ExpressionContextR context);

    ECOBJECTS_EXPORT Utf8String             ToString() const;
    };

/*=================================================================================**//**
*
* Used to introduce a named value into the context.
+===============+===============+===============+===============+===============+======*/
struct          ValueSymbol : Symbol
{
private:
    EvaluationResult                m_expressionValue;

#ifndef DOCUMENTATION_GENERATOR
protected:
    ValueSymbol (Utf8CP name, EvaluationResultCR exprValue);

    virtual                         ~ValueSymbol();
    ExpressionStatus _GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) override;
    ExpressionStatus _GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, bvector<ExpressionContextP> const& contextsStack, ::uint32_t startIndex) override
                                {return ExpressionStatus::NeedsLValue;}
#endif
public:
#ifndef DOCUMENTATION_GENERATOR
    ECOBJECTS_EXPORT static ValueSymbolPtr  Create (Utf8CP name, EvaluationResultCR value);

    ECOBJECTS_EXPORT EvaluationResultCR     GetValue() const;
    ECOBJECTS_EXPORT void                   SetValue (EvaluationResultCR value);
    ECOBJECTS_EXPORT void                   SetValue (ECValueCR value);

#endif
    //! Creates a new ValueSymbol with the given name and given ECValue
    ECOBJECTS_EXPORT static ValueSymbolPtr    Create(Utf8CP name, ECN::ECValueCR exprValue);

};  //  End of ValueSymbol



/*=================================================================================**//**
* Visitor interface for an in-order traversal of the Nodes of an ECExpression tree. Each
* method can return true to continue the traversal, or false to terminate it.
+===============+===============+===============+===============+===============+======*/
struct          NodeVisitor
{
    virtual     ~NodeVisitor() {}
    //! Invoked when an open parenthesis is encountered
    virtual bool OpenParens() = 0;
    //! Invoked when a closing parenthesis is encountered
    virtual bool CloseParens() = 0;
    //! Invoked when an array index is encountered
    virtual bool StartArrayIndex(NodeCR node) = 0;
    //! Invoked after an array index is processed
    virtual bool EndArrayIndex(NodeCR node) = 0;
    //! Invoked when the beginning of an argument list is encountered
    virtual bool StartArguments(NodeCR node) = 0;
    //! Invoked after processing an argument list
    virtual bool EndArguments(NodeCR node) = 0;
    //! Invoked when a comma is encountered
    virtual bool Comma() = 0;
    //! Invoked to process a node
    virtual bool ProcessNode(NodeCR node) = 0;
};

/*=================================================================================**//**
* Parses an EC expression string to produce an expression tree which can be used to
* evaluate the expression.
+===============+===============+===============+===============+===============+======*/
struct          ECEvaluator
    {
private:
    LexerPtr            m_lexer;
    ExpressionContextPtr m_thisContext;

    NodePtr         GetErrorNode(Utf8CP errorMessage, Utf8CP detail1 = NULL, Utf8CP detail2 = NULL);
    NodePtr         Must (ExpressionToken s, NodeR inputNode);
    NodePtr         ParseArguments ();

    NodePtr         ParsePrimary();
    NodePtr         ParseUnaryArith();
    NodePtr         ParseExponentiation();
    NodePtr         ParseMultiplicative();
    NodePtr         ParseIntegerDivision();
    NodePtr         ParseMod ();
    NodePtr         ParseAdditive();
    NodePtr         ParseConcatenation();
    NodePtr         ParseShift();
    NodePtr         ParseComparison();
    NodePtr         ParseNot();
    NodePtr         ParseConjunction();
    NodePtr         ParseValueExpression();
    NodePtr         ParseExpression(Utf8CP expression, bool tryAssignment);
    NodePtr         ParseAssignment();

    bool            CheckComplete ();
                    ECEvaluator(ExpressionContextP thisContext);

public:
    //! Parses a value expression and returns the root node of the expression tree.
    ECOBJECTS_EXPORT static NodePtr  ParseValueExpressionAndCreateTree(Utf8CP expression);
    //! Parses an assignment expression and returns the root node of the expression tree.
    ECOBJECTS_EXPORT static NodePtr  ParseAssignmentExpressionAndCreateTree(Utf8CP expression);

    // Parses and evaluate a value expression and returns the result
    ECOBJECTS_EXPORT static ExpressionStatus  EvaluateExpression(EvaluationResult& result, WCharCP expr, ExpressionContextR context);
    
    // Parses and evaluate a value expression and returns the result
    ECOBJECTS_EXPORT static ExpressionStatus  EvaluateExpression(EvaluationResult& result, Utf8CP expr, ExpressionContextR context);
};  // End of ECEvaluator class

/*=================================================================================**//**
* Holds the result of evaluating an EC expression.
+===============+===============+===============+===============+===============+======*/
struct          ValueResult : RefCountedBase
{
private:
    EvaluationResult    m_evalResult;

    ValueResult (EvaluationResultR result);
    ~ValueResult();

public:
#ifndef DOCUMENTATION_GENERATOR
    static ValueResultPtr      Create(EvaluationResultR result);

    EvaluationResultR           GetResult()         { return m_evalResult; }
    EvaluationResultCR          GetResult() const   { return m_evalResult; }
#endif
    //! Gets the result of the evalution
    ECOBJECTS_EXPORT ExpressionStatus  GetECValue (ECN::ECValueR ecValue);
};

/*=================================================================================**//**
* An object which can optimize an ECExpression tree by resolving constant sub-expressions
* to literal values, or perform other optimizations.
+===============+===============+===============+===============+===============+======*/
struct          ExpressionResolver : RefCountedBase
{
private:
    ExpressionContextPtr m_context;

protected:
    //! Attempts to promote nodes for an arithmetic operation.
    ExpressionStatus PerformArithmeticPromotion(ECN::PrimitiveType&targetType, ResolvedTypeNodePtr& left, ResolvedTypeNodePtr& right);
    //! Attempts to promote nodes for a junction operation.
    ExpressionStatus PerformJunctionPromotion(ECN::PrimitiveType&targetType, ResolvedTypeNodePtr& left, ResolvedTypeNodePtr& right);
    //! Attempts to promote the node to the specified primitive type.
    ExpressionStatus PromoteToType(ResolvedTypeNodePtr& node, ECN::PrimitiveType targetType);
    //! Attempts to promote the node to a string value.
    ExpressionStatus PromoteToString(ResolvedTypeNodePtr& node);
    //! Constructs ExpressionResolver from the specified ExpressionContext.
    ExpressionResolver(ExpressionContextR context) { m_context = &context; }

public:
    //! Returns the ExpressionContext associated with this ExpressionResolver
    ExpressionContextCR GetExpressionContext() const { return *m_context; }
    //! Returns the ExpressionContext associated with this ExpressionResolver
    ExpressionContextR GetExpressionContextR() const { return *m_context; }

    //! Attempts to resolve a PrimaryListNode
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolvePrimaryList (PrimaryListNodeR primaryList);
    //! Attempts to resolve a unary arithmetic node
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolveUnaryArithmeticNode (UnaryArithmeticNodeCR node);
    //! Attempts to resolve a multiplication node
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolveMultiplyNode (MultiplyNodeCR node);
    //! Attempts to resolve a division node
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolveDivideNode (DivideNodeCR node);
    //! Attempts to resolve a subtraction node
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolvePlusMinusNode (PlusMinusNodeCR node);
    //! Attempts to resolve a string concatenation node
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolveConcatenateNode (ConcatenateNodeCR node);
    //! Attempts to resolve a comparison node
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolveComparisonNode (ComparisonNodeCR node);
    //! Attempts to resolve a logical operation node
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolveLogicalNode (LogicalNodeCR node);
    //! Attempts to resolve a shift operation node
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolveShiftNode (ShiftNodeCR node);
    //! Attempts to resolve a resolve a conditional node
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolveIIfNode (IIfNodeCR node);
};

/*=================================================================================**//**
* Defines an expression tree for a parsed EC expression.
+===============+===============+===============+===============+===============+======*/
struct          Node : RefCountedBase
{
private:
    bool                m_inParens;  //  Only used for ToString
protected:
                        Node () { m_inParens = false; }
#ifndef DOCUMENTATION_GENERATOR
    virtual bool _Traverse(NodeVisitorR visitor) const {return visitor.ProcessNode(*this);}
    virtual Utf8String _ToString() const = 0;

    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context)
        { return ExpressionStatus::NotImpl; }

    virtual ExpressionToken _GetOperation () const { return TOKEN_Unrecognized; }

    virtual bool            _SetOperation (ExpressionToken token) { return false; }
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _GetResolvedTree(ExpressionResolverR context);
    virtual ResolvedTypeNodeP _GetAsResolvedTypeNodeP () { return NULL; }

    virtual bool            _IsAdditive () const { return false; }
    virtual bool            _IsUnary () const  { return false; }
    virtual bool            _IsBinary () const  { return false; }
    virtual bool            _IsConstant () const  { return false; }
    virtual bool            _HasError () { return false; }
    virtual NodeP           _GetLeftP () const { return NULL; }
    virtual NodeP           _GetRightP () const { return NULL; }
    virtual bool            _SetLeft (NodeR node) { return false; }
    virtual bool            _SetRight (NodeR node) { return false; }
#endif
public:
#ifndef DOCUMENTATION_GENERATOR
    bool                    GetHasParens() const { return m_inParens; }
    void                    SetHasParens(bool hasParens) { m_inParens = hasParens; }
    bool                    IsAdditive ()   const  { return _IsAdditive(); }
    bool                    IsUnary ()      const  { return _IsUnary(); }
    bool                    IsBinary ()     const  { return _IsBinary(); }
    bool                    IsConstant ()   const  { return _IsConstant (); }

    ExpressionToken         GetOperation () const { return _GetOperation(); }
    bool                    SetOperation (ExpressionToken token) { return _SetOperation (token); }

    NodeP                   GetLeftP () const { return _GetLeftP(); }
    NodeP                   GetRightP () const { return _GetRightP(); }

    NodeCP                  GetLeftCP () const { return _GetLeftP(); }
    NodeCP                  GetRightCP () const { return _GetRightP(); }
    bool                    SetLeft (NodeR node) { return _SetLeft (node); }
    bool                    SetRight (NodeR node) { return _SetRight (node); }

    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreateBooleanLiteral(bool literalValue);
    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreateStringLiteral (Utf8CP value, bool quoted);
    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreateIntegerLiteral (int value);
    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreateInt64Literal(int64_t value);
    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreateFloatLiteral(double value);
    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreateNullLiteral();
    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreateDateTimeLiteral (int64_t ticks);
    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreatePoint2dLiteral (DPoint2dCR value);
    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreatePoint3dLiteral (DPoint3dCR value);
    ECOBJECTS_EXPORT static NodePtr          CreateUnaryArithmetic(ExpressionToken tokenId, NodeR left);
    ECOBJECTS_EXPORT static NodePtr          CreateArithmetic(ExpressionToken  tokenId, NodeR left, NodeR right);
    ECOBJECTS_EXPORT static NodePtr          CreateShift (ExpressionToken tokenId, NodeR left, NodeR right);
    ECOBJECTS_EXPORT static NodePtr          CreateComparison(ExpressionToken   tokenId, NodeR left, NodeR right);
    ECOBJECTS_EXPORT static NodePtr          CreateBitWise(ExpressionToken   op, NodeR left, NodeR right);
    ECOBJECTS_EXPORT static NodePtr          CreateLogical(ExpressionToken   op, NodeR left, NodeR right);
    ECOBJECTS_EXPORT static NodePtr          CreateAssignment(NodeR left, NodeR rightSide, ExpressionToken   assignmentSubtype);
    ECOBJECTS_EXPORT static ArgumentTreeNodePtr CreateArgumentTree();
    ECOBJECTS_EXPORT static NodePtr          CreateIIf(NodeR conditional, NodeR trueNode, NodeR falseNode);

    //  Add nodes for Where, Property, Relationship, ConstantSets, Filters

    ECOBJECTS_EXPORT ExpressionStatus GetValue(EvaluationResult& evalResult, ExpressionContextR context);

    // Remaps access strings within this ECExpression according to the supplied remapper.
    // Returns true if any remapping was actually performed.
    ECOBJECTS_EXPORT bool   Remap (ECSchemaCR oldSchema, ECSchemaCR newSchema, IECSchemaRemapperCR remapper);
#endif
    //! Tries to generate a resolved tree.
    //! @returns a pointer to the root of the resolved tree, or NULL if unable to resolve any of the nodes in the subtree.
    //! @remarks A resolved tree can be executed much more efficiently that a tree that has not been resolved.
    ECOBJECTS_EXPORT ResolvedTypeNodePtr  GetResolvedTree(ExpressionResolverR context);

    //! Returns the value of this expression node using the supplied context
    ECOBJECTS_EXPORT ExpressionStatus GetValue(ValueResultPtr& valueResult, ExpressionContextR context);

    //!  Traverses in parse order
    ECOBJECTS_EXPORT bool  Traverse(NodeVisitorR visitor) const;

    //! Returns a string representation of the Node expression
    ECOBJECTS_EXPORT Utf8String  ToString() const;

    //! Converts the Node expression into an expression string
    ECOBJECTS_EXPORT Utf8String  ToExpressionString() const;
};  //  End of struct Node

/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE

