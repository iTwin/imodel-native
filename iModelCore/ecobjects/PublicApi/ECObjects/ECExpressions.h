/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECExpressions.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

//#include "ECInstanceIterable.h"

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
EXPR_TYPEDEFS(IdentNode)
EXPR_TYPEDEFS(IIfNode)
EXPR_TYPEDEFS(InstanceExpressionContext)
EXPR_TYPEDEFS(InstanceListExpressionContext)
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
EXPR_TYPEDEFS(UnitsType)
EXPR_TYPEDEFS(ValueResult)
EXPR_TYPEDEFS(ValueSymbol)
EXPR_TYPEDEFS(IECSymbolProvider)

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef RefCountedPtr<ArgumentTreeNode>             ArgumentTreeNodePtr;
typedef RefCountedPtr<CallNode>                     CallNodePtr;
typedef RefCountedPtr<ContextSymbol>                ContextSymbolPtr;
typedef RefCountedPtr<DotNode>                      DotNodePtr;
typedef RefCountedPtr<ErrorNode>                    ErrorNodePtr;
typedef RefCountedPtr<ExpressionType>               ExpressionTypePtr;
typedef RefCountedPtr<ExpressionContext>            ExpressionContextPtr;
typedef RefCountedPtr<IdentNode>                    IdentNodePtr;
typedef RefCountedPtr<InstanceExpressionContext>    InstanceExpressionContextPtr;
typedef RefCountedPtr<InstanceListExpressionContext> InstanceListExpressionContextPtr;
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

//! @ingroup ECObjectsGroup
//! Enumerates the possible return values for evaluating an expression or its value
enum ExpressionStatus
    {
    ExprStatus_Success              =   0, //!< Success
    ExprStatus_UnknownError         =   1, //!< There as an unknown error in evaluation
    ExprStatus_UnknownMember        =   2, //!< Returned if a property name in the expression cannot be found in the containing class
    ExprStatus_PrimitiveRequired    =   3, //!< Returned when a primitive is expected and not found
    ExprStatus_StructRequired       =   4, //!< Returned when a struct is expected and not found
    ExprStatus_ArrayRequired        =   5, //!< Returned when an array is expected and not found
    ExprStatus_UnknownSymbol        =   6, //!< Returned when the symbol in the expression cannot be resolved
    ExprStatus_DotNotSupported      =   7,

    //  Returning ExprStatus_NotImpl in base methods is the lazy approach for methods that should be
    //  pure virtual.  Should be eliminated after prototyping phase is done
    ExprStatus_NotImpl              =   8,

    ExprStatus_NeedsLValue          =   9, //!< Returned when the symbol needs to be an lvalue
    ExprStatus_WrongType            =  10, //!< Returned when the symbol type is of the wrong type for the expression
    ExprStatus_IncompatibleTypes    =  11, //!< Returned when expression uses incompatible types (ie, trying to perform arithmetic on two strings)
    ExprStatus_MethodRequired       =  12, //!< Returned when a method token is expected and not found
    ExprStatus_InstanceMethodRequired =  13, //!< Returned when an instance method is called, but has not been defined
    ExprStatus_StaticMethodRequired =  14, //!< Returned when a static method is called, but has not been defined
    ExprStatus_InvalidTypesForDivision =  15, //!< Returned when the expression tries to perform a division operation on types that cannot be divided
    ExprStatus_DivideByZero             =  16, //!< Returned when the division operation tries to divide by zero
    ExprStatus_WrongNumberOfArguments   =  17, //!< Returned when the number of arguments to a method in an expression do not match the number of arguments actually expected
    };

/*__PUBLISH_SECTION_END__*/

enum ValueType
    {
    ValType_None        =  0,
    ValType_ECValue     =  1,
    ValType_Custom      =  2,
    };

enum UnitsOrder
    {
    UO_Unknown      = 0,
    UO_Count        = 1,
    UO_Linear       = 2,
    UO_Area         = 4,
    UO_Volume       = 8
    };

/*=================================================================================**//**
*
*
+===============+===============+===============+===============+===============+======*/
enum            ResultCategory
    {
    Result_Unknown,
    Result_Error,                               //  Should insert some error detail
    Result_UnknownSymbol,
    Result_UnknownMember,
    Result_Value,
    Result_Null,
    Result_PrimitiveProperty,
    Result_ArrayProperty,
    Result_StructProperty,
    Result_InstanceSymbol,
    Result_ClassBinder,                         //  Should only be found in bind phase
    Result_PointMember,
    Result_StaticMethod,
    Result_InstanceMethod,
    Result_CustomSymbol,                        //  Is this really different from Result_Namespace
    };

/*__PUBLISH_SECTION_START__*/

typedef ExpressionStatus (*ExpressionStaticMethod_t)(EvaluationResult& evalResult, EvaluationResultVector& arguments);
typedef ExpressionStatus (*ExpressionInstanceMethod_t)(EvaluationResult& evalResult, EvaluationResultCR instanceData, EvaluationResultVector& arguments);

/*__PUBLISH_SECTION_END__*/

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
    virtual ExpressionStatus    _InvokeStaticMethod (EvaluationResultR evalResult, EvaluationResultVector& arguments) { return ExprStatus_NotImpl; }
    virtual ExpressionStatus    _InvokeInstanceMethod (EvaluationResultR evalResult, EvaluationResultCR instanceData, EvaluationResultVector& arguments) { return ExprStatus_NotImpl; }
    virtual bool                _SupportsStaticMethodCall () const = 0;
    virtual bool                _SupportsInstanceMethodCall () const = 0;

public:
    bool                        SupportsStaticMethodCall () const { return _SupportsStaticMethodCall(); }
    bool                        SupportsInstanceMethodCall () const { return _SupportsInstanceMethodCall(); }

    bool                        CanReuseResult()                { return _CanReuseResult(); }
    ExpressionStatus            InvokeStaticMethod (EvaluationResult& evalResult, EvaluationResultVector& arguments)
                                            { return _InvokeStaticMethod(evalResult, arguments); }
    ExpressionStatus            InvokeInstanceMethod (EvaluationResult& evalResult, EvaluationResultCR instanceData, EvaluationResultVector& arguments)
                                            { return _InvokeInstanceMethod(evalResult, instanceData, arguments); }
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
protected:
                                MethodReferenceStandard(ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod);
    virtual bool                _CanReuseResult ()               { return true; }
    virtual bool                _SupportsStaticMethodCall () const override { return NULL != m_staticMethod; }
    virtual bool                _SupportsInstanceMethodCall () const override { return NULL != m_instanceMethod; }

    //  The vector of arguments does not include the object used to invoke the method. It is
    //  up to the specific implementation of MethodReference to hold onto the instance and to use
    //  that to invoke the method.
    virtual ExpressionStatus    _InvokeStaticMethod (EvaluationResultR evalResult, EvaluationResultVector& arguments) override;
    virtual ExpressionStatus    _InvokeInstanceMethod (EvaluationResultR evalResult, EvaluationResultCR instanceData, EvaluationResultVector& arguments) override;
public:

    static MethodReferencePtr   Create(ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod);
}; // MethodReference

/*__PUBLISH_SECTION_START__*/

/*=================================================================================**//**
* The context in which an expression is evaluated.
* @ingroup ECObjectsGroup
+===============+===============+===============+===============+===============+======*/
struct          ExpressionContext : RefCountedBase
{

/*__PUBLISH_SECTION_END__*/
private:
    ExpressionContextPtr                m_outer;
    bool                                m_allowsTypeConversion;

protected:

    virtual                     ~ExpressionContext () {}
                                ExpressionContext(ExpressionContextP outer) : m_outer(outer), m_allowsTypeConversion (true) { }
    virtual ExpressionStatus    _ResolveMethod(MethodReferencePtr& result, wchar_t const* ident, bool useOuterIfNecessary) { return ExprStatus_UnknownSymbol; }
    virtual bool                _IsNamespace() const { return false; }
    //  If we provide this it must be implemented in every class that implements the _GetReference that uses more arguments.
    //  virtual ExpressionStatus    _GetReference(PrimaryListNodeR primaryList, bool useOuterIfNecessary) const { return ExprStatus_NotImpl; }
    //  The globalContext may be used to find instance methods
    virtual ExpressionStatus    _GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex) = 0;
    virtual ExpressionStatus    _GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex) { return ExprStatus_NotImpl; }

public:

    bool                        IsNamespace () const  { return _IsNamespace(); }
    ExpressionContextP          GetOuterP () const   { return m_outer.get(); }
    ExpressionStatus            ResolveMethod(MethodReferencePtr& result, wchar_t const* ident, bool useOuterIfNecessary)
                                    { return _ResolveMethod(result, ident, useOuterIfNecessary); }

    ExpressionStatus            GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex = 0)
                                    { return _GetValue(evalResult, primaryList, globalContext, startIndex); }

    ExpressionStatus            GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex = 0)

                                    { return _GetReference(evalResult, refResult, primaryList, globalContext, startIndex); }

// constructors are hidden from published API -> make it abstract in the published API
//__PUBLISH_CLASS_VIRTUAL__
/*__PUBLISH_SECTION_START__*/
    //! By default, property values obtained from IECInstances are subject to type conversion. The ConvertToExpressionType() method of
    //! the IECTypeAdapter associated with the ECProperty will be called to perform conversion.
    //! If this ExpressionContext has an outer context, it inherits the value of the outermost context.
    ECOBJECTS_EXPORT bool       AllowsTypeConversion() const;

    //! Enable or disable type conversion of ECProperty values.
    //! Has no effect if this context has an outer context.
    //! If this context serves as an outer context and has no outer context of its own, all of its inner contexts inherit the value.
    ECOBJECTS_EXPORT void       SetAllowsTypeConversion (bool allows);
}; // End of class ExpressionContext

/*=================================================================================**//**
* A context in which an IECInstance provides the context for expression evaluation.
* @ingroup ECObjectsGroup
+===============+===============+===============+===============+===============+======*/
struct          InstanceExpressionContext : ExpressionContext
{
/*__PUBLISH_SECTION_END__*/

private:
    ECN::IECInstancePtr          m_instance;

protected:

                                InstanceExpressionContext(ExpressionContextP outer) : ExpressionContext(outer) {}
    virtual ExpressionStatus    _GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex) override;
    virtual ExpressionStatus    _GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex) override;

public:
    ECN::ECEnablerCR             GetEnabler() { return m_instance->GetEnabler(); }
    ECN::IECInstanceP            GetInstanceP() { return m_instance.get(); }
/*__PUBLISH_SECTION_START__*/
public:
    ECOBJECTS_EXPORT ECN::IECInstanceCP          GetInstanceCP() const; //!< Returns the IECInstance that is the context of this expression
    ECOBJECTS_EXPORT void                        SetInstance(ECN::IECInstanceCR instance); //!< Sets the instance that is used as the context for this expression
    ECOBJECTS_EXPORT static InstanceExpressionContextPtr Create(ExpressionContextP outer); //!< Creates a new InstanceExpressionContext from the supplied ExpressionContext
}; // End of class InstanceExpressionContext

/*=================================================================================**//**
* A context in which multiple IECInstances provide the context for expression evaluation
* @ingroup ECObjectsGroup
+===============+===============+===============+===============+===============+======*/
struct          InstanceListExpressionContext : ExpressionContext
    {
/*__PUBLISH_SECTION_END__*/
private:
    bvector<InstanceExpressionContextPtr>                   m_instances;
    bool                                                    m_initialized;

    InstanceListExpressionContext (bvector<IECInstancePtr> const& instances);

    void                                        Initialize();
    void                                        Initialize (bvector<IECInstancePtr> const& instances);

                     virtual bool               _IsNamespace() const override { return true; }
    ECOBJECTS_EXPORT virtual ExpressionStatus   _GetValue (EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex) override;
    ECOBJECTS_EXPORT virtual ExpressionStatus   _GetReference (EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex) override;
    ECOBJECTS_EXPORT virtual ExpressionStatus   _ResolveMethod(MethodReferencePtr& result, wchar_t const* ident, bool useOuterIfNecessary) override;
protected:
    // The following protected methods are only relevant to derived classes, which may want to:
    //  -lazily load the instance list, and/or
    //  -Reuse the context for differing lists of instances
    ECOBJECTS_EXPORT InstanceListExpressionContext();

    virtual void                                _GetInstances (bvector<IECInstancePtr>& instances) { }

    bool                                        IsInitialized() const { return m_initialized; }
    void                                        Reset() { m_instances.clear(); m_initialized = false; }
/*__PUBLISH_SECTION_START__*/
public:
    //! Creates a new InstanceListExpressionContext from the list of IECInstances
    ECOBJECTS_EXPORT static InstanceListExpressionContextPtr    Create (bvector<IECInstancePtr> const& instances);
    };

/*=================================================================================**//**
* A context which provides a set of symbols for expression evaluation.
* @ingroup ECObjectsGroup
+===============+===============+===============+===============+===============+======*/
struct          SymbolExpressionContext : ExpressionContext
{
/*__PUBLISH_SECTION_END__*/
private:
    bvector<SymbolPtr>          m_symbols;

protected:

    ECOBJECTS_EXPORT virtual ExpressionStatus    _ResolveMethod(MethodReferencePtr& result, wchar_t const* ident, bool useOuterIfNecessary) override;
    ECOBJECTS_EXPORT virtual ExpressionStatus    _GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex) override;
    ECOBJECTS_EXPORT virtual ExpressionStatus    _GetReference(EvaluationResultR evalResult, ReferenceResultR refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex) override;

    ECOBJECTS_EXPORT virtual bool                _IsNamespace() const { return true; }
                                SymbolExpressionContext(ExpressionContextP outer) : ExpressionContext(outer) {}

public:

    SymbolCP                    FindCP (wchar_t const* ident);
    BentleyStatus               RemoveSymbol (SymbolR symbol);
    BentleyStatus               RemoveSymbol (wchar_t const* ident);

    ECOBJECTS_EXPORT static SymbolExpressionContextPtr   Create (bvector<WString> const& requestedSymbolSets);

/*__PUBLISH_SECTION_START__*/
public:
    //! Adds a symbol to the context to be used in expression evaluation
    ECOBJECTS_EXPORT BentleyStatus  AddSymbol (SymbolR symbol);
    //! Creates a new SymbolExpressionContext from the given ExpressionContext
    ECOBJECTS_EXPORT static SymbolExpressionContextPtr Create(ExpressionContextP outer);
}; // End of class SymbolExpressionContext


/*=================================================================================**//**
*
* Base class for all symbol types
*
* @ingroup ECObjectsGroup
+===============+===============+===============+===============+===============+======*/
struct          Symbol : RefCountedBase
{
/*__PUBLISH_SECTION_END__*/
private:
    WString     m_name;

protected:
                Symbol(wchar_t const* name)
    {
    m_name = name;
    }

    virtual ExpressionStatus         _CreateMethodResult (MethodReferencePtr& result) const     { return ExprStatus_MethodRequired; };
    virtual ExpressionStatus         _GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex) = 0;
    virtual ExpressionStatus         _GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex) = 0;

public:
    wchar_t const*                       GetName() const { return m_name.c_str(); }

    ExpressionStatus                CreateMethodResult (MethodReferencePtr& result) const { return _CreateMethodResult (result); }

    ExpressionStatus                GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex)
                                                { return _GetValue(evalResult, primaryList, globalContext, startIndex); }

    ExpressionStatus                GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex)
                                                { return _GetReference(evalResult, refResult, primaryList, globalContext, startIndex); }
/*__PUBLISH_SECTION_START__*/

};  // End of class Symbol

/*=================================================================================**//**
*
* Used to give a name to an instance.
* @ingroup ECObjectsGroup
+===============+===============+===============+===============+===============+======*/
struct          ContextSymbol : Symbol
{
/*__PUBLISH_SECTION_END__*/
protected:
    ExpressionContextPtr        m_context;

                ContextSymbol (wchar_t const* name, ExpressionContextR context)
                                    : Symbol(name), m_context(&context) {}

    virtual ExpressionStatus        _GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex) override;
    virtual ExpressionStatus        _GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex) override;

/*__PUBLISH_SECTION_START__*/

public:
    //! Creates a new ContextSymbol
    //! @param[in] name     The name to be used for this context symbol
    //! @param[in] context  The expression context to be used for this context
    //! @returns A new ContextSymbolPtr
    ECOBJECTS_EXPORT static ContextSymbolPtr        CreateContextSymbol(wchar_t const* name, ExpressionContextR context);
};

/*=================================================================================**//**
* Used to introduce a named method into the context.
* @ingroup ECObjectsGroup
+===============+===============+===============+===============+===============+======*/
struct          MethodSymbol : Symbol
{
/*__PUBLISH_SECTION_END__*/
private:
    MethodReferencePtr  m_methodReference;

protected:
    virtual ExpressionStatus        _GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex) override;
    virtual ExpressionStatus        _GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex) override
                                                { return ExprStatus_NeedsLValue; }
    virtual ExpressionStatus         _CreateMethodResult (MethodReferencePtr& result) const
        {
        result = m_methodReference.get();
        return ExprStatus_Success;
        }

                MethodSymbol(wchar_t const* name, ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod);

/*__PUBLISH_SECTION_START__*/
public:
    //! Creates a new method symbol context, using the supplied methods
    ECOBJECTS_EXPORT static MethodSymbolPtr    Create(wchar_t const* name, ExpressionStaticMethod_t staticMethod, ExpressionInstanceMethod_t instanceMethod);
};

/*=================================================================================**//**
*
* Used to introduce a named value into the context.
* @ingroup ECObjectsGroup
+===============+===============+===============+===============+===============+======*/
struct          ValueSymbol : Symbol
{
/*__PUBLISH_SECTION_END__*/
private:
    ECN::ECValue     m_expressionValue;

protected:
    virtual                         ~ValueSymbol();
    virtual ExpressionStatus        _GetValue(EvaluationResultR evalResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex) override;

    virtual ExpressionStatus        _GetReference(EvaluationResultR evalResult, ReferenceResult& refResult, PrimaryListNodeR primaryList, ExpressionContextR globalContext, ::UInt32 startIndex) override
                                                { return ExprStatus_NeedsLValue; }

public:
                ValueSymbol (wchar_t const* name, ECN::ECValueCR exprValue);

/*__PUBLISH_SECTION_START__*/
    //! Gets the value of this symbol
    ECOBJECTS_EXPORT void       GetValue(ECN::ECValueR exprValue);
    //! Sets the value of this symbol to a new ECValue
    ECOBJECTS_EXPORT void       SetValue(ECN::ECValueCR exprValue);
    //! Creates a new ValueSymbol with the given name and given ECValue
    ECOBJECTS_EXPORT static ValueSymbolPtr    Create(wchar_t const* name, ECN::ECValueCR exprValue);

};  //  End of ValueSymbol

/*__PUBLISH_SECTION_END__*/

/*=================================================================================**//**
* Provides a set of Symbols
+===============+===============+===============+===============+===============+======*/
struct      IECSymbolProvider : RefCountedBase
    {
    typedef void (* ExternalSymbolPublisher)(SymbolExpressionContextR, bvector<WString> const&);
protected:
    virtual WCharCP                 _GetName() const = 0;
    virtual void                    _PublishSymbols (SymbolExpressionContextR context, bvector<WString> const& requestedSymbolSets) = 0;
public:
    ECOBJECTS_EXPORT WCharCP        GetName() const
                                        { return _GetName(); }
    ECOBJECTS_EXPORT void           PublishSymbols (SymbolExpressionContextR context, bvector<WString> const& requestedSymbolSets)
                                        { return _PublishSymbols (context, requestedSymbolSets); }

    ECOBJECTS_EXPORT static void    RegisterExternalSymbolPublisher (ExternalSymbolPublisher externalPublisher);
    };

typedef RefCountedPtr<IECSymbolProvider> IECSymbolProviderPtr;

/*__PUBLISH_SECTION_START__*/

//! @ingroup ECObjectsGroup
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
    TOKEN_Colon               = 58,               //   :
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

    TOKEN_DoubleColon         = 100,

    TOKEN_Error               = 181,
    TOKEN_Ident               = 182,
    TOKEN_StringConst         = 183,
    TOKEN_PointConst          = 184,
    TOKEN_DateTimeConst       = 185,               //      @
    TOKEN_IntegerConstant     = 187,
    TOKEN_HexConstant         = 188,
    TOKEN_FloatConst          = 189,
    TOKEN_UnitsConst          = 190,
    TOKEN_Unrecognized        = 200,
    TOKEN_BadNumber           = 201,
    TOKEN_BadOctalNumber      = 202,
    TOKEN_BadHexNumber        = 203,
    TOKEN_BadFloatingPointNumber = 204,
    TOKEN_UnterminatedString   = 205,
    TOKEN_PrimaryList         = 206,
    };

/*__PUBLISH_SECTION_END__*/
/*=================================================================================**//**
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          UnitsType : RefCountedBase
{
    UnitsOrder  m_unitsOrder;
    ECN::IECInstancePtr  m_extendedType;
    bool        m_powerCanDecrease;
    bool        m_powerCanIncrease;
};

/*=================================================================================**//**
* @bsiclass                                                     John.Gooding    02/2011
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
    ECN::ECPropertyCP    m_property;
    WString             m_accessString;
    ::UInt32            m_arrayIndex;
    int                 m_memberSelector;   // 1 for x, 2, for y, 3 for z
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct          EvaluationResult
{
private:
//  Provides a list of conditions for which the shortcuts or bindings are valid
    ValueType   m_valueType;
    UnitsOrder  m_unitsOrder;
    ECN::ECValue m_ecValue;

public:

    ExpressionStatus        GetInteger(Int32& result);
    ExpressionStatus        GetBoolean(bool& result, bool requireBoolean = true);

    //  Constructors and destructors
    ECOBJECTS_EXPORT EvaluationResult ();
    ECOBJECTS_EXPORT ~EvaluationResult ();
    ECOBJECTS_EXPORT EvaluationResult(EvaluationResultCR rhs);

    ECOBJECTS_EXPORT EvaluationResultR operator=(EvaluationResultCR rhs);
    void                    Clear();

    ECOBJECTS_EXPORT ECN::ECValueR            GetECValueR();
    ECOBJECTS_EXPORT ExpressionStatus        GetECValue(ECN::ECValueR result);
    ECOBJECTS_EXPORT ECN::ECValueCR           GetECValue() const;
    ECOBJECTS_EXPORT EvaluationResultR       operator= (ECN::ECValueCR rhs);
    };

/*__PUBLISH_SECTION_START__*/

/*=================================================================================**//**
*
+===============+===============+===============+===============+===============+======*/
struct          NodeVisitor
{
    virtual     ~NodeVisitor() {}
    virtual bool OpenParens() = 0;
    virtual bool CloseParens() = 0;
    virtual bool StartArrayIndex(NodeCR node) = 0;
    virtual bool EndArrayIndex(NodeCR node) = 0;
    virtual bool StartArguments(NodeCR node) = 0;
    virtual bool EndArguments(NodeCR node) = 0;
    virtual bool Comma() = 0;
    virtual bool ProcessNode(NodeCR node) = 0;
};

/*=================================================================================**//**
* Parses an EC expression string to produce an expression tree which can be used to
* evaluate the expression.
* @ingroup ECObjectsGroup
+===============+===============+===============+===============+===============+======*/
struct          ECEvaluator
    {
/*__PUBLISH_SECTION_END__*/
private:
    LexerPtr            m_lexer;
    ExpressionContextPtr m_thisContext;

    NodePtr         GetErrorNode(wchar_t const*  errorMessage, wchar_t const* detail1 = NULL, wchar_t const* detail2 = NULL);
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
    NodePtr         ParseExpression(wchar_t const* expression, bool tryAssignment);
    NodePtr         ParseAssignment();

    bool            CheckComplete ();
                    ECEvaluator(ExpressionContextP thisContext);

/*__PUBLISH_SECTION_START__*/
public:
    //! Parses a value expression and returns the root node of the expression tree.
    ECOBJECTS_EXPORT static NodePtr  ParseValueExpressionAndCreateTree(wchar_t const* expression);
    //! Parses an assignment expression and returns the root node of the expression tree.
    ECOBJECTS_EXPORT static NodePtr  ParseAssignmentExpressionAndCreateTree(wchar_t const* expression);

};  // End of ECEvaluator class

/*=================================================================================**//**
* Holds the result of evaluating an EC expression.
* @ingroup ECObjectsGroup
+===============+===============+===============+===============+===============+======*/
struct          ValueResult : RefCountedBase
{
/*__PUBLISH_SECTION_END__*/
private:
    EvaluationResult    m_evalResult;

    ValueResult (EvaluationResultR result);
    ~ValueResult();

public:
    static ValueResultPtr      Create(EvaluationResultR result);

/*__PUBLISH_SECTION_START__*/
    //! Gets the result of the evalution
    ECOBJECTS_EXPORT ExpressionStatus  GetECValue (ECN::ECValueR ecValue);
};

/*=================================================================================**//**
*
* @ingroup ECObjectsGroup
+===============+===============+===============+===============+===============+======*/
struct          ExpressionResolver : RefCountedBase
{
private:
    ExpressionContextPtr m_context;

protected:
    ExpressionStatus PerformArithmeticPromotion(ECN::PrimitiveType&targetType, ResolvedTypeNodePtr& left, ResolvedTypeNodePtr& right);
    ExpressionStatus PerformJunctionPromotion(ECN::PrimitiveType&targetType, ResolvedTypeNodePtr& left, ResolvedTypeNodePtr& right);
    ExpressionStatus PromoteToType(ResolvedTypeNodePtr& node, ECN::PrimitiveType targetType);
    ExpressionStatus PromoteToString(ResolvedTypeNodePtr& node);
    ExpressionResolver(ExpressionContextR context) { m_context = &context; }

public:
    ExpressionContextCR GetExpressionContext() const { return *m_context; }
    ExpressionContextR GetExpressionContextR() const { return *m_context; }

    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolvePrimaryList (PrimaryListNodeR primaryList);
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolveUnaryArithmeticNode (UnaryArithmeticNodeCR node);
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolveMultiplyNode (MultiplyNodeCR node);
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolveDivideNode (DivideNodeCR node);
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolvePlusMinusNode (PlusMinusNodeCR node);
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolveConcatenateNode (ConcatenateNodeCR node);
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolveComparisonNode (ComparisonNodeCR node);
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolveLogicalNode (LogicalNodeCR node);
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolveShiftNode (ShiftNodeCR node);
    ECOBJECTS_EXPORT virtual ResolvedTypeNodePtr _ResolveIIfNode (IIfNodeCR node);
};

/*=================================================================================**//**
* Defines an expression tree for a parsed EC expression.
* @ingroup ECObjectsGroup
+===============+===============+===============+===============+===============+======*/
struct          Node : RefCountedBase
{
/*__PUBLISH_SECTION_END__*/
private:
    bool                m_inParens;  //  Only used for ToString
protected:
                        Node () { m_inParens = false; }
    virtual bool        _Traverse(NodeVisitorR visitor) const { return visitor.ProcessNode(*this); }
    virtual WString     _ToString() const = 0;

    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context,
                                        bool allowUnknown, bool allowOverrides)
        { return ExprStatus_NotImpl; }

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
    virtual void            _DetermineKnownUnits(UnitsTypeR unitsType) const { }
    virtual void            _ForceUnitsOrder(UnitsTypeCR  knownType)  {}

public:
    bool                    GetHasParens() const { return m_inParens; }
    void                    SetHasParens(bool hasParens) { m_inParens = hasParens; }
    bool                    IsAdditive ()   const  { return _IsAdditive(); }
    bool                    IsUnary ()      const  { return _IsUnary(); }
    bool                    IsBinary ()     const  { return _IsBinary(); }
    bool                    IsConstant ()   const  { return _IsConstant (); }

    void                    ForceUnitsOrder(UnitsTypeCR  knownType)  { _ForceUnitsOrder(knownType); }
    void                    DetermineKnownUnits(UnitsTypeR unitsType) const { _DetermineKnownUnits(unitsType);  }
    ExpressionToken         GetOperation () const { return _GetOperation(); }
    bool                    SetOperation (ExpressionToken token) { return _SetOperation (token); }

    NodeP                   GetLeftP () const { return _GetLeftP(); }
    NodeP                   GetRightP () const { return _GetRightP(); }

    NodeCP                  GetLeftCP () const { return _GetLeftP(); }
    NodeCP                  GetRightCP () const { return _GetRightP(); }
    bool                    SetLeft (NodeR node) { return _SetLeft (node); }
    bool                    SetRight (NodeR node) { return _SetRight (node); }

    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreateBooleanLiteral(bool literalValue);
    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreateStringLiteral (wchar_t const* value, bool quoted);
    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreateIntegerLiteral (int value);
    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreateInt64Literal(Int64 value);
    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreateFloatLiteral(double value);
    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreateNullLiteral();
    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreateDateTimeLiteral (Int64 ticks);
    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreatePoint2DLiteral (DPoint2dCR value);
    ECOBJECTS_EXPORT static ResolvedTypeNodePtr CreatePoint3DLiteral (DPoint3dCR value);
    static NodePtr          CreateUnaryArithmetic(ExpressionToken tokenId, NodeR left);
    static NodePtr          CreateArithmetic(ExpressionToken  tokenID, NodeR left, NodeR right);
    static NodePtr          CreateShift (ExpressionToken tokenID, NodeR left, NodeR right);
    static NodePtr          CreateComparison(ExpressionToken   tokenID, NodeR left, NodeR right);
    static NodePtr          CreateBitWise(ExpressionToken   op, NodeR left, NodeR right);
    static NodePtr          CreateLogical(ExpressionToken   op, NodeR left, NodeR right);
    static NodePtr          CreateAssignment(NodeR left, NodeR rightSide, ExpressionToken   assignmentSubtype);
    static ArgumentTreeNodePtr CreateArgumentTree();
    static NodePtr          CreateIIf(NodeR conditional, NodeR trueNode, NodeR falseNode);

    //  Add nodes for Where, Property, Relationship, ConstantSets, Filters

    ECOBJECTS_EXPORT ExpressionStatus GetValue(EvaluationResult& evalResult, ExpressionContextR context,
                                        bool allowUnknown, bool allowOverrides);

// constructors are hidden from published API -> make it abstract in the published API
//__PUBLISH_CLASS_VIRTUAL__
/*__PUBLISH_SECTION_START__*/
public:
    //! Tries to generate a resolved tree.
    //! @Returns a pointer to the root of the resolved tree, or NULL if unable to resolve any of the nodes in the subtree.
    //! @remarks A resolved tree can be executed much more efficiently that a tree that has not been resolved.
    ECOBJECTS_EXPORT ResolvedTypeNodePtr  GetResolvedTree(ExpressionResolverR context);

    //! Returns the value of this expression node using the supplied context
    ECOBJECTS_EXPORT ExpressionStatus GetValue(ValueResultPtr& valueResult, ExpressionContextR context,
                                        bool allowUnknown, bool allowOverrides);

    //!  Traverses in parse order
    ECOBJECTS_EXPORT bool  Traverse(NodeVisitorR visitor) const;

    //! Returns a string representation of the Node expression
    ECOBJECTS_EXPORT WString  ToString() const;

    //! Converts the Node expression into an expression string
    ECOBJECTS_EXPORT WString  ToExpressionString() const;
};  //  End of struct Node

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
