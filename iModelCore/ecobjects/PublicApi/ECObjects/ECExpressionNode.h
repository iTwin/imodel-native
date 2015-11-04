/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECExpressionNode.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*=================================================================================**//**
*
+===============+===============+===============+===============+===============+======*/
struct          Lexer : RefCountedBase
{
private:
    ExpressionToken   m_currentToken;
    ExpressionToken   m_tokenModifier;   //  Only used for Assign??
    Utf8Char        m_tokenBuilder [1024];
    size_t          m_inputIndex;
    size_t          m_outputIndex;
    size_t          m_maxInputIndex;
    size_t          m_maxOutputIndex;
    size_t          m_startPosition;

    Utf8CP             m_inputCP;              //  Points into contents of m_inputString
    Utf8String         m_inputString;

    Utf8CP                  getTokenStringCP ();
    Utf8Char                GetCurrentChar ();
    Utf8Char                GetNextChar  ();
    void                    PushBack ();
    Utf8Char                PeekChar  ();
    Utf8Char                GetDigits (ExpressionToken&  tokenType, bool isOctal);
    bool                    IsHexDigit (Utf8Char ch);
    Utf8Char                GetHexConstant (ExpressionToken&  tokenType);
    ExpressionToken         GetNumericConstant ();
    ExpressionToken         GetString ();
    ExpressionToken         GetIdentifier ();
    ExpressionToken         ScanToken ();
    void                    MarkOffset();
                            Lexer (Utf8CP inputString);
public:
    static LexerPtr         Create(Utf8CP inputString);
    ECOBJECTS_EXPORT static Utf8String     GetString(ExpressionToken tokenName);
    void                    Advance ();
    ExpressionToken         GetTokenType ();
    ExpressionToken         GetTokenModifier ();
    Utf8CP                  GetTokenStringCP ();
    size_t                  GetTokenStartOffset();

    };  //  End of class Lexer

/*=================================================================================**//**
*
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          Operations 
{
private:
    static void        ReportInvalidDivisionOperands() {}
    static void        ReportInvalidArithmeticOperands(EvaluationResultR left, EvaluationResultR right) {}
    static void        ReportInvalidLikeOperands(EvaluationResultR left, EvaluationResultR right) {}
    static void        ReportInvalidJunctionOperands(EvaluationResultR left, EvaluationResultR right) {}
    static void        ReportInvalidComparisonOperands(EvaluationResultR left, EvaluationResultR right) {}
    static void        ReportInvalidUnaryArithmeticOperand(ExpressionToken operationCode, EvaluationResultR left) {}

    static ExpressionStatus EnforceMultiplicativeUnits (UnitSpecR units, EvaluationResultR left, EvaluationResultR right);
public:
    // PRECONDITION: PromoteCommon was called, left and right are non-null and have same type and type is int, long, or double
    static ExpressionStatus EnforceLikeUnits (EvaluationResultR left, EvaluationResultR right);

    static ExpressionStatus ConvertToInt32(EvaluationResultR evalResult);
    static ExpressionStatus ConvertToString(EvaluationResultR evalResult);
    static ExpressionStatus ConvertToString(ECN::ECValueR evalResult);
    static ExpressionStatus ConvertToInt64(EvaluationResultR evalResult);
    static ExpressionStatus ConvertToDouble(EvaluationResultR evalResult);
    static ExpressionStatus ConvertToDateTime(EvaluationResultR evalResult);
    static ExpressionStatus ConvertToArithmeticOrBooleanOperand (EvaluationResultR evalResult);
    static ExpressionStatus ConvertToBooleanOperand (EvaluationResultR ecValue);
    static ExpressionStatus ConvertStringToArithmeticOperand (EvaluationResultR ecValue);
    static ExpressionStatus ConvertStringToArithmeticOrBooleanOperand (EvaluationResultR ecValue);
    static ExpressionStatus PerformArithmeticPromotion(EvaluationResult& leftResult, EvaluationResult& rightResult);
    static ExpressionStatus PerformJunctionPromotion(EvaluationResultR left, EvaluationResultR right);
    static ExpressionStatus PerformUnaryMinus(EvaluationResultR resultOut, EvaluationResultR left);
    static ExpressionStatus PerformUnaryNot(EvaluationResultR resultOut, EvaluationResultR left);
    static ExpressionStatus PerformShift(EvaluationResultR resultOut, ExpressionToken shiftOp, EvaluationResultR left, EvaluationResultR right);
    static ExpressionStatus PerformMultiplication(EvaluationResultR resultOut, EvaluationResultR left, EvaluationResultR right, bool enforceUnits);
    static ExpressionStatus PerformExponentiation(EvaluationResultR resultOut, EvaluationResultR left, EvaluationResultR right);
    static ExpressionStatus PerformIntegerDivision(EvaluationResultR resultOut, EvaluationResultR left, EvaluationResultR right, bool enforceUnits);
    static ExpressionStatus PerformDivision(EvaluationResultR resultOut, EvaluationResultR left, EvaluationResultR right, bool enforceUnits);
    static ExpressionStatus PerformMod(EvaluationResultR resultOut, EvaluationResultR left, EvaluationResultR right);
    static ExpressionStatus PerformLikeTest(EvaluationResultR resultOut, EvaluationResultR left, EvaluationResultR right) { return ExpressionStatus::NotImpl; }
    static ExpressionStatus PerformJunctionOperator(EvaluationResultR resultOut, ExpressionToken junctionOperator, EvaluationResultR left, EvaluationResultR right);
    static ExpressionStatus PerformLogicalOr(EvaluationResultR resultOut, EvaluationResultR leftValue, EvaluationResultR rightValue);
    static ExpressionStatus PerformLogicalAnd(EvaluationResultR resultOut, EvaluationResultR leftValue, EvaluationResultR rightValue);
};  // End of class Operations

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                      John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct           NodeHelpers
{
private:

protected:

public:
    static  void        GetAdditiveNodes(NodeCPVector& nodes, NodeCR rightMost);

}; // NodeHelpers

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          ErrorNode : Node
    {
private:
    Utf8String     m_summary;
    Utf8String     m_detail1;
    Utf8String     m_detail2;

protected:
    virtual bool            _HasError () override { return true; }

                ErrorNode (Utf8CP summary, Utf8CP detail1, Utf8CP detail12) 
                    : m_summary(summary), m_detail1(detail1), m_detail2(detail12) {}
    virtual Utf8String     _ToString() const override { return "ERROR"; }
    virtual ExpressionToken _GetOperation () const override { return TOKEN_Error; }

public:
    static ErrorNodePtr Create(Utf8CP summary, Utf8CP detail1, Utf8CP detail12)
        {
        if (NULL == detail1)
            detail1 = "";
        return new ErrorNode (summary, NULL == detail1 ? "" : detail1, NULL== detail12 ? "" : detail12);
        }
    };

//=======================================================================================
//! A node with the data type fully determined.  A ResolvedTypeNode can be the child of a 
//! Node, but a Node that is not a ResolvedTypeNode cannot be the child of a ResolvedTypeNode. 
// @bsiclass                                                    John.Gooding    09/2013
//=======================================================================================
struct ResolvedTypeNode : Node
    {
private:
    ECN::PrimitiveType       m_primitiveType;

protected:
    ECOBJECTS_EXPORT virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context) override;
    ResolvedTypeNode(ECN::PrimitiveType primitiveType) : m_primitiveType(primitiveType) {}
    //!  Optimization provided as alternative to dynamic_cast
    virtual ResolvedTypeNodeP _GetAsResolvedTypeNodeP () override { return this; }
    //!  For classes that are not subclasses of ResolvedTypeNode try to create an instance of a subclass of ResolvedTypeNode.
    virtual ResolvedTypeNodePtr _GetResolvedTree(ExpressionResolverR context) override { return this; }
    virtual Utf8String     _ToString() const { return "RESOLVED"; }

    void CheckBoolean() {BeAssert(PRIMITIVETYPE_Boolean == m_primitiveType); }
    void CheckDateTime() {BeAssert(PRIMITIVETYPE_DateTime == m_primitiveType); }
    void CheckDouble() {BeAssert(PRIMITIVETYPE_Double == m_primitiveType); }
    void CheckInteger() {BeAssert(PRIMITIVETYPE_Integer == m_primitiveType); }
    void CheckLong() {BeAssert(PRIMITIVETYPE_Long == m_primitiveType); }
    void CheckString() {BeAssert(PRIMITIVETYPE_String == m_primitiveType); }

    void SetPrimitiveType (PrimitiveType type) { m_primitiveType = type; }
public:
    enum SupportedGetTypes
        {
        GetBoolean      =  1,
        GetDateTime     =  2,
        GetInteger      =  4,
        GetLong         =  8,
        GetDouble       = 16,
        GetString       = 32
        };

    virtual bool _SupportsGetBooleanValue() {return PRIMITIVETYPE_Boolean == m_primitiveType; }
    virtual bool _SupportsGetDateTimeValue() {return PRIMITIVETYPE_DateTime == m_primitiveType; }
    virtual bool _SupportsGetDoubleValue() {return PRIMITIVETYPE_Double == m_primitiveType; }
    virtual bool _SupportsGetIntegerValue() {return PRIMITIVETYPE_Integer == m_primitiveType; }
    virtual bool _SupportsGetLongValue() {return PRIMITIVETYPE_Long == m_primitiveType; }
    virtual bool _SupportsGetStringValue() {return PRIMITIVETYPE_String == m_primitiveType; }

    ECN::PrimitiveType GetResolvedPrimitiveType() const { return m_primitiveType; }
    virtual bool _GetBooleanValue(ExpressionStatus& expressionStatus, ExpressionContextR context) { expressionStatus = ExpressionStatus::WrongType; return false; }
    virtual ::int64_t _GetDateTimeValue(ExpressionStatus& expressionStatus, ExpressionContextR context) { expressionStatus = ExpressionStatus::WrongType; return 0; }
    virtual double _GetDoubleValue(ExpressionStatus& expressionStatus, ExpressionContextR context) { expressionStatus = ExpressionStatus::WrongType; return 0; }
    virtual ::int32_t _GetIntegerValue(ExpressionStatus& expressionStatus, ExpressionContextR context) { expressionStatus = ExpressionStatus::WrongType; return 0; }
    virtual ::int64_t _GetLongValue(ExpressionStatus& expressionStatus, ExpressionContextR context) { expressionStatus = ExpressionStatus::WrongType; return 0; }
    virtual ExpressionStatus _GetStringValue(ECValueR expressionStatus, ExpressionContextR context) { return ExpressionStatus::WrongType; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct LiteralNode : ResolvedTypeNode
    {
private:
    ECValue         m_value;

    LiteralNode (ECValueCR v) : ResolvedTypeNode (v.GetPrimitiveType()), m_value (v) { }

    //  John: If we don't provide some type code it can't be a ResolvedTypeNode preventing the entire
    //  tree above it from being ResolvedTypeNode.  Nothing should match PRIMITIVETYPE_Binary.
    LiteralNode() : ResolvedTypeNode (PRIMITIVETYPE_Binary), m_value (PRIMITIVETYPE_Binary) { }

    virtual Utf8String             _ToString() const override
        {
        Utf8String str;
        m_value.ConvertPrimitiveToECExpressionLiteral (str);
        return str;
        }

    virtual ExpressionStatus    _GetValue (EvaluationResult& result, ExpressionContextR) override
        {
        result = m_value;
        return ExpressionStatus::Success;
        }

    virtual bool                _IsConstant() const override { return true; }
    virtual ExpressionToken     _GetOperation() const override
        {
        if (m_value.IsNull())
            return TOKEN_Null;

        switch (m_value.GetPrimitiveType())
            {
            case PRIMITIVETYPE_String:      return TOKEN_StringConst;
            case PRIMITIVETYPE_Integer:     return TOKEN_IntegerConstant;
            case PRIMITIVETYPE_Long:        return TOKEN_IntegerConstant;
            case PRIMITIVETYPE_Double:      return TOKEN_FloatConst;
            case PRIMITIVETYPE_Boolean:     return TOKEN_True;
            case PRIMITIVETYPE_DateTime:    return TOKEN_DateTimeConst;
            default:                        BeAssert (false); return TOKEN_Null;
            }
        }

    virtual bool    _SupportsGetBooleanValue() override { return !m_value.IsNull() && m_value.CanConvertToPrimitiveType (PRIMITIVETYPE_Boolean); }
    virtual bool    _SupportsGetDoubleValue() override  { return !m_value.IsNull() && m_value.CanConvertToPrimitiveType (PRIMITIVETYPE_Double); }
    virtual bool    _SupportsGetIntegerValue() override { return !m_value.IsNull() && m_value.CanConvertToPrimitiveType (PRIMITIVETYPE_Integer); }
    virtual bool    _SupportsGetLongValue() override    { return !m_value.IsNull() && m_value.CanConvertToPrimitiveType (PRIMITIVETYPE_Long); }
    virtual bool    _SupportsGetStringValue() override  { return !m_value.IsNull() && m_value.CanConvertToPrimitiveType (PRIMITIVETYPE_String); }
    virtual bool    _SupportsGetDateTimeValue() override{ return !m_value.IsNull() && m_value.CanConvertToPrimitiveType (PRIMITIVETYPE_DateTime); }

    virtual bool    _GetBooleanValue (ExpressionStatus& status, ExpressionContextR context) override
        {
        ECValue v (m_value);
        status = v.ConvertToPrimitiveType (PRIMITIVETYPE_Boolean) ? ExpressionStatus::Success : ExpressionStatus::WrongType;
        return ExpressionStatus::Success == status ? v.GetBoolean() : false;
        }
    virtual ::int32_t _GetIntegerValue (ExpressionStatus& status, ExpressionContextR) override
        {
        ECValue v (m_value);
        status = v.ConvertToPrimitiveType (PRIMITIVETYPE_Integer) ? ExpressionStatus::Success : ExpressionStatus::WrongType;
        return ExpressionStatus::Success == status ? v.GetInteger() : 0;
        }
    virtual ::int64_t _GetLongValue (ExpressionStatus& status, ExpressionContextR) override
        {
        ECValue v (m_value);
        status = v.ConvertToPrimitiveType (PRIMITIVETYPE_Long) ? ExpressionStatus::Success : ExpressionStatus::WrongType;
        return ExpressionStatus::Success == status ? v.GetLong() : 0;
        }
    virtual double  _GetDoubleValue (ExpressionStatus& status, ExpressionContextR) override
        {
        ECValue v (m_value);
        status = v.ConvertToPrimitiveType (PRIMITIVETYPE_Double) ? ExpressionStatus::Success : ExpressionStatus::WrongType;
        return ExpressionStatus::Success == status ? v.GetDouble() : 0.0;
        }
    virtual ::int64_t _GetDateTimeValue (ExpressionStatus& status, ExpressionContextR) override
        {
        ECValue v (m_value);
        status = v.ConvertToPrimitiveType (PRIMITIVETYPE_DateTime) ? ExpressionStatus::Success : ExpressionStatus::WrongType;
        return ExpressionStatus::Success == status ? (::int64_t) v.GetDouble() : 0;
        }
    virtual ExpressionStatus _GetStringValue(ECValueR v, ExpressionContextR context)
        {
        if (m_value.GetPrimitiveType() == PRIMITIVETYPE_String)
            {
            v = m_value;
            return ExpressionStatus::Success;
            }
        else
            return ExpressionStatus::WrongType;
        }
public:
    ECValueCR   GetInternalValue() const { return m_value; }
    bool        SetInternalValue (ECValueCR v)
        {
        if (v.IsNull())
            m_value = ECValue (PRIMITIVETYPE_Binary);
        else if (v.IsPrimitive())
            m_value = v;
        else
            return false;

        SetPrimitiveType (m_value.GetPrimitiveType());
        return true;
        }

    static ResolvedTypeNodePtr      CreateString (Utf8CP v)          { return new LiteralNode (ECValue (v, false)); }
    static ResolvedTypeNodePtr      CreateInteger (::int32_t v)     { return new LiteralNode (ECValue (v)); }
    static ResolvedTypeNodePtr      CreateDouble (double v)         { return new LiteralNode (ECValue (v)); }
    static ResolvedTypeNodePtr      CreateLong (::int64_t v)        { return new LiteralNode (ECValue (v)); }
    static ResolvedTypeNodePtr      CreateBoolean (bool v)          { return new LiteralNode (ECValue (v)); }
    static ResolvedTypeNodePtr      CreatePoint3D (DPoint3dCR v)    { return new LiteralNode (ECValue (v)); }
    static ResolvedTypeNodePtr      CreatePoint2D (DPoint2dCR v)    { return new LiteralNode (ECValue (v)); }
    static ResolvedTypeNodePtr      CreateNull()                    { return new LiteralNode(); }
    static ResolvedTypeNodePtr      CreateDateTime (::int64_t ticks)
        {
        ECValue v;
        v.SetDateTimeTicks (ticks);
        return new LiteralNode (v);
        }
    };

/*=================================================================================**//**
*
* Holds a list of nodes forming a primary.  Every dotted expression, method call
* or array access expression must start with an identifier.  That identifier is represented
* by the member m_identName.  All subsequent operations are represented by entries in 
* m_operators.
*
* The entire primary list is fed into the context.  The context that resolves m_identName
* is responsible for handling all of the operators in the list.
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          PrimaryListNode : Node
{
private:
    bvector<NodePtr>        m_operators;

    virtual bool        _Traverse(NodeVisitorR visitor) const
        {
        for (size_t i = 0; i < m_operators.size(); i++)
            {
            bool retval = m_operators[i]->Traverse(visitor);
            if (!retval)
                return retval;
            }
        
        return true;
        }

protected:
    virtual Utf8String     _ToString() const override 
        {
        return "";
        }

    virtual ExpressionToken _GetOperation () const override { return TOKEN_PrimaryList; }
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context) override;

    virtual bool            _HasError () override { return false; }
    virtual ResolvedTypeNodePtr _GetResolvedTree(ExpressionResolverR context) override { return context._ResolvePrimaryList(*this); }

public:
    ECOBJECTS_EXPORT Utf8CP             GetName(size_t index) const;
    ECOBJECTS_EXPORT size_t             GetNumberOfOperators() const;
    ECOBJECTS_EXPORT NodeP              GetOperatorNode(size_t index) const;    // NEEDSWORK: const method returning member as non-const pointer...propagates to CallNode::InvokeXXXMethod()...
    ECOBJECTS_EXPORT ExpressionToken    GetOperation(size_t index) const;

    static PrimaryListNodePtr Create() { return new PrimaryListNode(); }
    void                    AppendCallNode(CallNodeR  callNode);
    void                    AppendNameNode(IdentNodeR  nameNode);
    void                    AppendArrayNode(LBracketNodeR  lbracketNode);
    void                    AppendLambdaNode (LambdaNodeR lambdaNode);
}; //  End of PrimaryListNode

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          IdentNode : Node
{
private:
    Utf8String      m_value;
    bvector<Utf8String> m_qualifiers;

                IdentNode(Utf8CP name) : m_value(name) {}
    friend      struct DotNode;
protected:
    virtual Utf8String     _ToString() const override { return m_value; }

    //  May want to distinguish between compiled category and resolved category
    virtual ExpressionToken _GetOperation () const override { return TOKEN_Ident; }

    virtual bool            _HasError () override { return false; }

public:
    void                    PushQualifier(Utf8CP rightName);
    bvector<Utf8String> const& GetQualifiers() const { return m_qualifiers; }
    Utf8CP                  GetName() const { return m_value.c_str(); }
    void                    SetName (Utf8CP name) { m_value = name; }
    static IdentNodePtr     Create(Utf8CP name) { return new IdentNode(name); }

}; // End of struct IdentNode

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          DotNode : IdentNode
{
private:
    Utf8String                 m_memberName;

    virtual bool        _Traverse(NodeVisitorR visitor) const
        {
        return visitor.ProcessNode(*this); 
        }

protected:
    virtual Utf8String     _ToString() const override 
        {
        return "." + m_memberName;
        }

    //  May want to distinguish between compiled category and resolved category
    virtual ExpressionToken _GetOperation () const override { return TOKEN_Dot; }

    virtual bool            _HasError () override { return false; }

public:

    Utf8CP GetMemberName() const { return m_memberName.c_str(); }

    DotNode (Utf8CP memberName) : IdentNode(memberName), m_memberName(memberName) {}
    static DotNodePtr Create(Utf8CP memberName) { return new DotNode(memberName); }

}; //  End of DotNode

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          LBracketNode : Node
{
private:
    //  May want to cache a reference object for cases where this is working with a constant reference
    NodePtr   m_container;
    NodePtr   m_index;

                LBracketNode(NodeR index) : m_index(&index) {}

protected:
    virtual Utf8String     _ToString() const override { return "["; }
    virtual bool        _Traverse(NodeVisitorR visitor) const
        {
        bool retval = visitor.StartArrayIndex(*this);
        if (retval)
            retval = m_index->Traverse(visitor);
        if (retval)
            retval = visitor.EndArrayIndex(*this);

        return retval;
        }

    //  May want to distinguish between compiled category and resolved category
    virtual ExpressionToken _GetOperation () const override { return TOKEN_LeftBracket; }

    virtual bool            _HasError () override { return false; }
public:
    NodePtr                 GetIndexNode() const { return m_index.get(); }
    static LBracketNodePtr  Create(NodeR index) { return new LBracketNode(index); }
}; //  End of LBracketNode

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          UnaryNode : Node
{
private:
    ExpressionToken   m_operator;
    NodePtr   m_left;

protected:
    virtual Utf8String     _ToString() const override { return Lexer::GetString(m_operator); }

    virtual bool        _Traverse(NodeVisitorR visitor) const
        {
        bool    retval = visitor.ProcessNode(*this);
        if (retval)
            retval = m_left->Traverse(visitor);

        return retval;
        }

    virtual ExpressionToken _GetOperation () const override { return m_operator; }

    virtual bool            _IsUnary () const override { return true; }
    virtual NodeP           _GetLeftP () const override { return m_left.get(); }
    virtual bool            _SetLeft (NodeR node) override { m_left = &node; return true; }

public:
                UnaryNode (ExpressionToken tokenId, NodeR left) : 
                                    m_operator(tokenId), m_left(&left) {}

};  //  End of struct Unary

/*---------------------------------------------------------------------------------**//**
* Currently this only supports specifying a BASE unit, plus optionally a factor and offset.
* Later we may enhance it so that it accepts any unit available in the Units_Schema...right
* now we don't want to have to look up units by name in the schema, and we don't have
* a pressing use case for arbitrary unit declarations.
* @bsistruct                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnitSpecNode : UnaryNode
    {
private:
    UnitSpec            m_units;

    UnitSpecNode (NodeR left, Utf8CP unitName) : UnaryNode (TOKEN_Colon, left), m_units (unitName, UnitConverter (false)) { }

    virtual ExpressionStatus    _GetValue (EvaluationResult& evalResult, ExpressionContextR context) override;
    virtual bool                _Traverse (NodeVisitorR visitor) const override
        {
        bool ret = visitor.ProcessNode (*this);
        if (ret)
            ret = visitor.ProcessUnits (m_units);

        return ret;
        }
public:
    static UnitSpecNodePtr      Create (NodeR left, Utf8CP baseUnitName);

    void                        SetFactor (double factor);
    void                        SetFactorAndOffset (double factor, double offset);
    UnitSpecCR                  GetUnits() const { return m_units; }
    };

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          UnaryArithmeticNode : UnaryNode
{
protected:
    virtual ResolvedTypeNodePtr _GetResolvedTree(ExpressionResolverR context) override { return context._ResolveUnaryArithmeticNode(*this); }
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context) override;

public:
                UnaryArithmeticNode (ExpressionToken tokenId, NodeR left) : UnaryNode (tokenId, left) {}

};  //  End of struct UnaryArithmeticNode

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          BinaryNode : Node
{
private:
    NodePtr     m_left;
    NodePtr     m_right;

protected:
    ExpressionToken  m_operatorCode;

    virtual Utf8String     _ToString() const override { return Lexer::GetString(m_operatorCode); }

    virtual bool        _Traverse(NodeVisitorR visitor) const
        {
        bool    retval = m_left->Traverse(visitor);
        if (retval)
            retval = visitor.ProcessNode(*this);
        if (retval)
            retval = m_right->Traverse(visitor);

        return retval;
        }

    virtual ExpressionToken _GetOperation () const override { return m_operatorCode; }

    virtual bool            _IsAdditive () const override
                                { return TOKEN_Plus == m_operatorCode || TOKEN_Minus == m_operatorCode; }
    virtual bool            _IsBinary () const override { return true; }
    //  may be true for the context
    //  virtual bool            _IsConstant () const  override { return true; }

    virtual NodeP           _GetLeftP () const override { return m_left.get(); }
    virtual NodeP           _GetRightP () const override { return m_right.get(); }
    virtual bool            _SetLeft (NodeR node) override { m_left = &node; return true; }
    virtual bool            _SetRight (NodeR node) override { m_right = &node; return true; }

    ExpressionStatus        PromoteCommon(EvaluationResult& leftResult, EvaluationResult& rightResult, ExpressionContextR context, bool allowStrings);
    ExpressionStatus        GetOperandValues(EvaluationResult& leftResult, EvaluationResult& rightResult, ExpressionContextR context);

public:
                    BinaryNode (ExpressionToken operatorCode, NodeR left, NodeR right) : m_operatorCode(operatorCode), m_left(&left), m_right(&right) {}

    //  virtual abstract ECEvaluationResult ApplyOperator (ECEvaluationResult leftValue, ECEvaluationResult rightValue);

};  //  End of struct Binary

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          ArithmeticNode : BinaryNode  //  No modifiers -- see ModifierNode
{
protected:

    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context) override;


    virtual ExpressionStatus _Promote(EvaluationResult& leftResult, EvaluationResult& rightResult, 
                                        ExpressionContextR context) { return ExpressionStatus::NotImpl; }

    //  Undecided if this is pure virtual or a default implementation.
    virtual ExpressionStatus _PerformOperation(EvaluationResultR result,
                                        EvaluationResultCR leftResult, EvaluationResultCR rightResult, 
                                        ExpressionContextR context)  { return ExpressionStatus::NotImpl; }

    ExpressionStatus        PerformDefaultPromotion(EvaluationResult& leftResult, EvaluationResult& rightResult);
    virtual ResolvedTypeNodePtr _GetResolvedTree(ExpressionResolverR context) override
                                //  Every subclass of ArithmeticNode should implement _GetResolvedTree
                                { BeAssert(false && L"_ResolveArithmeticNode not implemented"); return NULL; }

public:
                ArithmeticNode(ExpressionToken operatorCode, NodeR left, NodeR right) 
                                : BinaryNode (operatorCode, left, right) {}

};  //  End of struct ArithmeticNode

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          ExponentNode : ArithmeticNode
{
protected:
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context) override;

public:
                ExponentNode(NodeR left, NodeR right) 
                                : ArithmeticNode (TOKEN_Exponentiation, left, right) {}

};  //  End of struct ExponentNode

/*=================================================================================**//**
*
* Implements multiply and mod
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          MultiplyNode : ArithmeticNode  //  No modifiers -- see ModifierNode
{
protected:
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context) override;
    virtual ResolvedTypeNodePtr _GetResolvedTree(ExpressionResolverR context) override { return context._ResolveMultiplyNode(*this); }

public:
                MultiplyNode(ExpressionToken operatorCode, NodeR left, NodeR right) 
                                : ArithmeticNode (operatorCode, left, right) {}
};  //  End of struct MultiplyNode

/*=================================================================================**//**
*
* Implements division and integer division
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          DivideNode : ArithmeticNode  //  No modifiers -- see ModifierNode
{
protected:
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context) override;

    virtual ResolvedTypeNodePtr _GetResolvedTree(ExpressionResolverR context) override { return context._ResolveDivideNode(*this); }
public:
                DivideNode(ExpressionToken operatorCode, NodeR left, NodeR right) 
                                : ArithmeticNode (operatorCode, left, right) {}

};  //  End of struct DivideNode

/*=================================================================================**//**
*
* Implements plus and minus
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          PlusMinusNode : ArithmeticNode 
{
protected:
    //  Undecided if this is pure virtual or a default implementation.
    virtual ExpressionStatus _Promote(EvaluationResult& leftResult, EvaluationResult& rightResult, 
                                        ExpressionContextR context);

    //  Undecided if this is pure virtual or a default implementation.
    virtual ExpressionStatus _PerformOperation(EvaluationResultR result,
                                        EvaluationResultCR leftResult, EvaluationResultCR rightResult, 
                                        ExpressionContextR context);
    virtual ResolvedTypeNodePtr _GetResolvedTree(ExpressionResolverR context) override { return context._ResolvePlusMinusNode(*this); }
public:
                PlusMinusNode(ExpressionToken operatorCode, NodeR left, NodeR right) 
                                : ArithmeticNode (operatorCode, left, right) {}

};  //  End of struct PlusMinusNode

/*=================================================================================**//**
*
* Implements string concatenation operator
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          ConcatenateNode : ArithmeticNode 
{
protected:
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context) override;

    virtual ResolvedTypeNodePtr _GetResolvedTree(ExpressionResolverR context) override { return context._ResolveConcatenateNode(*this); }
public:
                ConcatenateNode(NodeR left, NodeR right) 
                                : ArithmeticNode (TOKEN_Concatenate, left, right) {}

};  //  End of struct ConcatenateNode

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          ShiftNode : BinaryNode
{
protected:
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context) override;
    virtual ResolvedTypeNodePtr _GetResolvedTree(ExpressionResolverR context) override { return context._ResolveShiftNode(*this); }

public:
                ShiftNode(ExpressionToken operatorCode, NodeR left, NodeR right) 
                                : BinaryNode (operatorCode, left, right) {}

};  //  End of struct ShiftNode

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          ComparisonNode : BinaryNode
{
protected:
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context) override;

    virtual ResolvedTypeNodePtr _GetResolvedTree(ExpressionResolverR context) override { return context._ResolveComparisonNode(*this); }

public:
                ComparisonNode(ExpressionToken operatorCode, NodeR left, NodeR right) 
                                : BinaryNode (operatorCode, left, right) {}

};  //  End of struct ComparisonNode

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          LogicalNode : BinaryNode
{
protected:
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context) override;
    virtual ResolvedTypeNodePtr _GetResolvedTree(ExpressionResolverR context) override { return context._ResolveLogicalNode(*this); }
    virtual bool            _SetOperation (ExpressionToken operatorCode) override
        {
        switch (operatorCode)
            {
        case TOKEN_And: case TOKEN_AndAlso:
        case TOKEN_Or: case TOKEN_OrElse: case TOKEN_Xor:
            m_operatorCode = operatorCode; return true;
        default:
            return false;
            }
        }
public:
                LogicalNode(ExpressionToken operatorCode, NodeR left, NodeR right) 
                                : BinaryNode (operatorCode, left, right) {}

};  //  End of struct LogicalNode

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          AssignmentNode : BinaryNode
{
private:
    //  Result is stored in the right result
    static ExpressionStatus PerformModifier (ExpressionToken  modifier, EvaluationResultR left, EvaluationResultR right);

public:
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context) override;

                AssignmentNode(NodeR left, NodeR right, ExpressionToken assignmentSubtype) 
                                : BinaryNode (assignmentSubtype, left, right) {}

};  //  End of struct AssignmentNode

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          ArgumentTreeNode : Node
{
private:
    NodePtrVector    m_arguments;

protected:
    virtual Utf8String     _ToString() const override { return ""; }

    virtual bool        _Traverse(NodeVisitorR visitor) const
        {
        bool    announceComma = false;
        visitor.StartArguments(*this);

        for (NodePtrVector::const_iterator curr = m_arguments.begin(); curr != m_arguments.end(); ++curr)
            {
            if (announceComma && !visitor.Comma())
                return false;

            NodeR   nodeRef = *(*curr).get();
            if (!nodeRef.Traverse(visitor))
                return false;

            announceComma = true;
            }

        return visitor.EndArguments(*this);
        }

    virtual ExpressionToken _GetOperation () const { return TOKEN_Comma; }

public:
                ArgumentTreeNode() {}
    size_t      GetArgumentCount() const { return m_arguments.size(); }
    NodeCP      GetArgument (size_t i) const { return i < m_arguments.size() ? m_arguments[i].get() : NULL; }
    void        PushArgument(NodeR node) { m_arguments.push_back(&node); }
    ECOBJECTS_EXPORT ExpressionStatus EvaluateArguments(EvaluationResultVector& results, ExpressionContextR context) const;
};  //  End of struct ArgumentTreeNode

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          CallNode : Node
{
private:
    ArgumentTreeNodePtr m_arguments;
    Utf8String             m_methodName;
    bool                m_dotted;

    virtual bool        _Traverse(NodeVisitorR visitor) const
        {
        bool    retval = visitor.ProcessNode(*this);;
        if (!retval)
            return false;

        return m_arguments->Traverse(visitor);
        }

    //  Constructor
                CallNode(ArgumentTreeNodeR arguments, Utf8CP methodName, bool dotted) : 
                                    m_arguments (&arguments), m_methodName(methodName), m_dotted(dotted) {}

protected:
    virtual Utf8String     _ToString() const override 
        {
        Utf8String     retval;
        if (m_dotted)
            retval.append(".");
        retval.append(m_methodName);
        return retval; 
        }

    virtual ExpressionToken _GetOperation () const override { return TOKEN_LParen; }


public:
    Utf8CP                  GetMethodName() const { return m_methodName.c_str(); }
    ArgumentTreeNode const* GetArguments() const { return m_arguments.get(); }
    static CallNodePtr      Create(ArgumentTreeNodeR arguments, Utf8CP methodName, bool dotted)
                                    { return new CallNode(arguments, methodName, dotted); }

    ExpressionStatus    InvokeInstanceMethod(EvaluationResult& evalResult, ECInstanceListCR instanceData, ExpressionContextR context);
    ExpressionStatus    InvokeStaticMethod(EvaluationResult& evalResult, MethodReferenceR  methodReference, ExpressionContextR context);
    ExpressionStatus    InvokeStaticMethod(EvaluationResult& evalResult, ExpressionContextR context);
    ExpressionStatus    InvokeValueListMethod (EvaluationResultR evalResult, IValueListResultCR valueList, ExpressionContextR context);

};  //  End of struct CallNode

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct LambdaNode : Node
    {
private:
    Utf8String      m_symbolName;
    NodePtr         m_lambdaExpression;

    LambdaNode (Utf8CP name, NodeR lambdaExpr) : m_symbolName (name), m_lambdaExpression (&lambdaExpr) { }
protected:
    virtual bool    _Traverse (NodeVisitorR visitor) const
        {
        bool retval = visitor.ProcessNode (*this);
        if (retval)
            retval = m_lambdaExpression->Traverse (visitor);

        return retval;
        }
    
    virtual Utf8String _ToString() const override
        {
        Utf8String ret (m_symbolName);
        ret.append ("=>");
        return ret;
        }

    virtual ExpressionToken     _GetOperation() const override { return TOKEN_Lambda; }
    virtual ExpressionStatus    _GetValue(EvaluationResult& evalResult, ExpressionContextR context) override;
public:
    Utf8CP              GetSymbolName() const { return m_symbolName.c_str(); }
    NodeR               GetExpression() const { return *m_lambdaExpression; }

    static LambdaNodePtr    Create (Utf8CP name, NodeR expr) { return new LambdaNode (name, expr); }
    };

/*---------------------------------------------------------------------------------**//**
* A result which needs to be evaluated in context.
* @bsistruct                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct LambdaValue : RefCountedBase
    {
private:
    LambdaNodePtr           m_node;
    ExpressionContextPtr    m_context;
    
    LambdaValue (LambdaNodeR node, ExpressionContextR context) : m_node(&node), m_context(&context) { }
public:
    LambdaNodeR             GetNode() const { return *m_node; }
    ExpressionContextCR     GetContext() const { return *m_context; }

    static LambdaValuePtr Create (LambdaNodeR node, ExpressionContextR context) { return new LambdaValue (node, context); }

    struct IProcessor
        {
        // Called for each application of the lambda to a target. Return false if no further evaluation desired.
        virtual bool        ProcessResult (ExpressionStatus status, EvaluationResultCR member, EvaluationResultCR result) = 0;
        };

    ECOBJECTS_EXPORT ExpressionStatus   Evaluate (IValueListResultCR valueList, IProcessor& processor) const;
    };

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          IIfNode : Node
{
private:
    NodePtr     m_condition;
    NodePtr     m_true;
    NodePtr     m_false;

protected:
    virtual Utf8String  _ToString() const override { return Lexer::GetString(TOKEN_IIf); }
    virtual bool        _Traverse(NodeVisitorR visitor) const
        {
        if (!visitor.ProcessNode(*this))
            return false;

        if (!visitor.OpenParens())
            return false;

        if (!m_condition->Traverse(visitor))
            return false;

        if (!visitor.Comma())
            return false;

        if (!m_true->Traverse(visitor))
            return false;

        if (!visitor.Comma())
            return false;

        if (!m_false->Traverse(visitor))
            return false;

        return visitor.CloseParens();
        }

    virtual ExpressionToken _GetOperation () const { return TOKEN_IIf; }
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context) override;
    virtual ResolvedTypeNodePtr _GetResolvedTree(ExpressionResolverR context) override { return context._ResolveIIfNode(*this); }

public:
    NodeP GetConditionP() const { return m_condition.get(); }
    NodeP GetTrueP() const { return m_true.get(); }
    NodeP GetFalseP() const { return m_false.get(); }

                IIfNode(NodeR condition, NodeR trueNode, NodeR falseNode)
                                : m_condition(&condition), m_true(&trueNode), m_false(&falseNode) {}
};  //  End of struct IIfNode

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedUnaryNode : ResolvedTypeNode
    {
protected:
    ResolvedTypeNodePtr   m_operand;
    ResolvedUnaryNode(ECN::PrimitiveType primitiveType, ResolvedTypeNodeR operand) : ResolvedTypeNode(primitiveType), m_operand(&operand) {}
    ResolvedTypeNodeR GetOperand() { return *m_operand; }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedUnaryMinusNode : ResolvedUnaryNode
{
private:
    ResolvedUnaryMinusNode(ECN::PrimitiveType primitiveType, ResolvedTypeNodeR operand) : ResolvedUnaryNode(primitiveType, operand) {}

public:
    virtual ::int32_t _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { CheckInteger(); return -m_operand->_GetIntegerValue(status, context); }
    virtual ::int64_t _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { CheckLong(); return -m_operand->_GetLongValue(status, context); }
    virtual double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { CheckDouble(); return -m_operand->_GetDoubleValue(status, context); }
    static ResolvedTypeNodePtr Create(ECN::PrimitiveType primitiveType, ResolvedTypeNodeR operand)  { return new ResolvedUnaryMinusNode(primitiveType, operand); }

};  //  End of struct ResolvedUnaryMinusNode

//  This is just a no-op. We want to eliminate it from tree.
//  struct          UnaryPlusNode : UnaryNode

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedUnaryNotNode : ResolvedUnaryNode
{
private:
    ResolvedUnaryNotNode(ECN::PrimitiveType primitiveType, ResolvedTypeNodeR operand) : ResolvedUnaryNode(primitiveType, operand) {}
public:
    virtual bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override { return !m_operand->_GetBooleanValue(status, context); }
    virtual ::int32_t _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { return ~m_operand->_GetIntegerValue(status, context); }
    virtual ::int64_t _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { return ~m_operand->_GetLongValue(status, context); }
    static ResolvedTypeNodePtr Create(ECN::PrimitiveType primitiveType, ResolvedTypeNodeR operand) { return new ResolvedUnaryNotNode(primitiveType, operand); }
};  //  End of struct UnaryNotNode

//  ResolvedExponentNode deferred; 

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedBinaryNode : ResolvedTypeNode
    {
protected:
    ResolvedTypeNodePtr   m_left;
    ResolvedTypeNodePtr   m_right;
    ResolvedBinaryNode(ECN::PrimitiveType primitiveType, ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedTypeNode(primitiveType), m_left(&left), m_right(&right) {}
    ResolvedTypeNodeR GetLeft() { return *m_left; }
    ResolvedTypeNodeR GetRight() { return *m_right; }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedMultiplyNode : ResolvedBinaryNode
    {
private:
    ResolvedMultiplyNode(ECN::PrimitiveType primitiveType, ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedBinaryNode(primitiveType, left, right) {}
public:
    virtual ::int32_t _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetIntegerValue(status, context) * m_right->_GetIntegerValue(status, context); }
    virtual ::int64_t _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetLongValue(status, context) * m_right->_GetLongValue(status, context); }
    virtual double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetDoubleValue(status, context) * m_right->_GetDoubleValue(status, context); }

    static ResolvedTypeNodePtr Create(ECN::PrimitiveType primitiveType, ResolvedTypeNodeR left, ResolvedTypeNodeR right) { return new ResolvedMultiplyNode(primitiveType, left, right); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedMultiplyConstantNode : ResolvedTypeNode
    {
private:
    ResolvedTypeNodePtr m_left;
    union
        {
        ::int32_t m_i;
        ::int64_t m_i64;
        double  m_d;
        } m_right;

    ResolvedMultiplyConstantNode(ResolvedTypeNodeR left, ::int32_t value) : ResolvedTypeNode(PRIMITIVETYPE_Integer), m_left(&left) { m_right.m_i = value; }
    ResolvedMultiplyConstantNode(ResolvedTypeNodeR left, ::int64_t value) : ResolvedTypeNode(PRIMITIVETYPE_Long), m_left(&left) { m_right.m_i64 = value; }
    ResolvedMultiplyConstantNode(ResolvedTypeNodeR left, double value) : ResolvedTypeNode(PRIMITIVETYPE_Double), m_left(&left) { m_right.m_d = value; }
public:
    virtual ::int32_t _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { CheckInteger(); return m_left->_GetIntegerValue(status, context) * m_right.m_i; }
    virtual ::int64_t _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { CheckLong(); return m_left->_GetLongValue(status, context) * m_right.m_i64; }
    virtual double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { CheckDouble(); return m_left->_GetDoubleValue(status, context) * m_right.m_d; }

    static ResolvedTypeNodePtr CreateInteger(ResolvedTypeNodeR left, ::int32_t value) { return new ResolvedMultiplyConstantNode(left, value); }
    static ResolvedTypeNodePtr CreateInt64(ResolvedTypeNodeR left, ::int64_t value) { return new ResolvedMultiplyConstantNode(left, value); }
    static ResolvedTypeNodePtr CreateDouble(ResolvedTypeNodeR left, double value) { return new ResolvedMultiplyConstantNode(left, value); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedModNode : ResolvedBinaryNode
    {
private:
    ResolvedModNode(ECN::PrimitiveType primitiveType, ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedBinaryNode(primitiveType, left, right) {}
public:
    virtual ::int32_t _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override 
        {
        ::int32_t divisor = m_right->_GetIntegerValue(status, context);
        if (0 == divisor)
            {
            if (ExpressionStatus::Success == status)
                status = ExpressionStatus::DivideByZero;
            return 0;
            }

        return m_left->_GetIntegerValue(status, context) % divisor; 
        }

    virtual ::int64_t _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override
        {
        ::int64_t divisor = m_right->_GetLongValue(status, context);
        if (0 == divisor)
            {
            if (ExpressionStatus::Success == status)
                status = ExpressionStatus::DivideByZero;
            return 0;
            }

        return m_left->_GetLongValue(status, context) % divisor; 
        }

    static ResolvedTypeNodePtr Create(ECN::PrimitiveType primitiveType, ResolvedTypeNodeR left, ResolvedTypeNodeR right) { return new ResolvedModNode(primitiveType, left, right); }
    };

//  Exponent Node -- currently not implemented, defer for now.

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
//  Division is always double precision. There is a separate integer division operator.
struct          ResolvedIntegerDivideNode : ResolvedBinaryNode
    {
private:
    ResolvedIntegerDivideNode(ECN::PrimitiveType primType, ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedBinaryNode(primType, left, right) {}
public:
    virtual ::int32_t _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override 
        {
        ::int32_t divisor = m_right->_GetIntegerValue(status, context);
        if (0 == divisor)
            {
            if (ExpressionStatus::Success == status)
                status = ExpressionStatus::DivideByZero;
            return 0;
            }

        return m_left->_GetIntegerValue(status, context) / divisor; 
        }

    virtual ::int64_t _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override
        {
        ::int64_t divisor = m_right->_GetLongValue(status, context);
        if (0 == divisor)
            {
            if (ExpressionStatus::Success == status)
                status = ExpressionStatus::DivideByZero;
            return 0;
            }

        return m_left->_GetLongValue(status, context) / divisor; 
        }

    virtual double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override
        {
        double divisor = m_right->_GetDoubleValue(status, context);
        if (0.0 == divisor)
            {
            if (ExpressionStatus::Success == status)
                status = ExpressionStatus::DivideByZero;
            return 0;
            }

        return floor(m_left->_GetDoubleValue(status, context) / divisor); 
        }

    static ResolvedTypeNodePtr Create(ECN::PrimitiveType primType, ResolvedTypeNodeR left, ResolvedTypeNodeR right) { return new ResolvedIntegerDivideNode(primType, left, right); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
//  Division is always double precision. There is a separate integer division operator.
struct          ResolvedDivideNode : ResolvedBinaryNode
    {
private:
    ResolvedDivideNode(ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedBinaryNode(ECN::PRIMITIVETYPE_Double, left, right) {}
public:
    virtual double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override
        {
        double divisor = m_right->_GetDoubleValue(status, context);
        if (0.0 == divisor)
            {
            if (ExpressionStatus::Success == status)
                status = ExpressionStatus::DivideByZero;
            return 0;
            }

        return m_left->_GetDoubleValue(status, context) / divisor; 
        }

    static ResolvedTypeNodePtr Create(ResolvedTypeNodeR left, ResolvedTypeNodeR right) { return new ResolvedDivideNode(left, right); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedDivideByConstantNode : ResolvedTypeNode
    {
private:
    double m_divisor;
    ResolvedTypeNodePtr m_left;
    ResolvedDivideByConstantNode(ResolvedTypeNodeR left, double divisor) : ResolvedTypeNode(ECN::PRIMITIVETYPE_Double), m_left(&left), m_divisor(divisor) {}

public:
    virtual double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetDoubleValue(status, context) / m_divisor; }
    static ResolvedTypeNodePtr Create(ResolvedTypeNodeR left, double divisor) { return new ResolvedDivideByConstantNode(left, divisor); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedDivideConstantNode : ResolvedTypeNode
    {
private:
    double m_left;
    ResolvedTypeNodePtr m_divisor;
    ResolvedDivideConstantNode(double left, ResolvedTypeNodeR divisor) : ResolvedTypeNode(ECN::PRIMITIVETYPE_Double), m_left(left), m_divisor(&divisor) {}

public:
    virtual double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override
        {
        double divisor = m_divisor->_GetDoubleValue(status, context);
        if (0.0 == divisor)
            {
            if (ExpressionStatus::Success == status)
                status = ExpressionStatus::DivideByZero;
            return 0;
            }

        return m_left / divisor; 
        }

    static ResolvedTypeNodePtr Create(double left, ResolvedTypeNodeR divisor) { return new ResolvedDivideConstantNode(left, divisor); }
    };


/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedAddNode : ResolvedBinaryNode
    {
private:
    ResolvedAddNode(ECN::PrimitiveType primitiveType, ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedBinaryNode(primitiveType, left, right) {}
public:
    virtual ::int32_t _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetIntegerValue(status, context) + m_right->_GetIntegerValue(status, context); }
    virtual ::int64_t _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetLongValue(status, context) + m_right->_GetLongValue(status, context); }
    virtual double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetDoubleValue(status, context) + m_right->_GetDoubleValue(status, context); }

    static ResolvedTypeNodePtr Create(ECN::PrimitiveType primitiveType, ResolvedTypeNodeR left, ResolvedTypeNodeR right) { return new ResolvedAddNode(primitiveType, left, right); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedAddConstantNode : ResolvedTypeNode
    {
private:
    union
        {
        ::int32_t   m_i;
        ::int64_t   m_i64;
        double      m_d;
        } m_right;
    ResolvedTypeNodePtr m_left;
    ResolvedAddConstantNode(ECN::PrimitiveType resultType, ResolvedTypeNodeR left, ECValueCR right);
public:
    virtual ::int32_t _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetIntegerValue(status, context) + m_right.m_i; }
    virtual ::int64_t _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetLongValue(status, context) + m_right.m_i64; }
    virtual double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetDoubleValue(status, context) + m_right.m_d; }

    static ResolvedTypeNodePtr Create(ECN::PrimitiveType resultType, ResolvedTypeNodeR left, ECValueCR right) { return new ResolvedAddConstantNode(resultType, left, right); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedSubtractNode : ResolvedBinaryNode
    {
private:
    ResolvedSubtractNode(ECN::PrimitiveType primitiveType, ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedBinaryNode(primitiveType, left, right) {}
public:
    virtual ::int32_t _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetIntegerValue(status, context) - m_right->_GetIntegerValue(status, context); }
    virtual ::int64_t _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetLongValue(status, context) - m_right->_GetLongValue(status, context); }
    virtual double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetDoubleValue(status, context) - m_right->_GetDoubleValue(status, context); }

    static ResolvedTypeNodePtr Create(ECN::PrimitiveType primitiveType, ResolvedTypeNodeR left, ResolvedTypeNodeR right) { return new ResolvedSubtractNode(primitiveType, left, right); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedConcatenateNode : ResolvedBinaryNode
    {
    ResolvedConcatenateNode(ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedBinaryNode(PRIMITIVETYPE_String, left, right) {}
public:
    virtual ExpressionStatus _GetStringValue(ECValueR result, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ResolvedTypeNodeR left, ResolvedTypeNodeR right) { return new ResolvedConcatenateNode(left, right); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedCompareNode : ResolvedBinaryNode
{
protected:
    ExpressionToken m_operatorCode;

public:
    ResolvedCompareNode(ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedBinaryNode(ECN::PRIMITIVETYPE_Boolean, left, right), m_operatorCode(operatorCode) {}
};

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedCompareIntegerNode : ResolvedCompareNode
{
private:
    ResolvedCompareIntegerNode(ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedCompareNode(operatorCode, left, right){}
public:
    virtual bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) { return new ResolvedCompareIntegerNode(operatorCode, left, right); }
};

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
template <typename T> struct          ResolvedCompareToConstantNode : ResolvedTypeNode
{
protected:
    ExpressionToken m_operatorCode;
    ResolvedTypeNodePtr m_left;
    T m_right;
public:
    ResolvedCompareToConstantNode(ExpressionToken operatorCode, ResolvedTypeNodeR left, T right) :
                                            ResolvedTypeNode(PRIMITIVETYPE_Boolean), m_operatorCode(operatorCode), m_left(&left), m_right(right) {}
};

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedCompareIntegerToConstantNode : ResolvedCompareToConstantNode< ::int32_t>
{
private:
    ResolvedCompareIntegerToConstantNode(ExpressionToken operatorCode, ResolvedTypeNodeR left, ::int32_t right) : ResolvedCompareToConstantNode< ::int32_t>(operatorCode, left, right) {}
public:
    virtual bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ExpressionToken operatorCode, ResolvedTypeNodeR left, ::int32_t right) { return new ResolvedCompareIntegerToConstantNode(operatorCode, left, right); }
};

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedCompareLongToConstantNode : ResolvedCompareToConstantNode< ::int64_t>
{
private:
    ResolvedCompareLongToConstantNode(ExpressionToken operatorCode, ResolvedTypeNodeR left, ::int64_t right) : ResolvedCompareToConstantNode< ::int64_t>(operatorCode, left, right) {}
public:
    virtual bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ExpressionToken operatorCode, ResolvedTypeNodeR left, ::int64_t right) { return new ResolvedCompareLongToConstantNode(operatorCode, left, right); }
};

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedCompareDoubleToConstantNode : ResolvedCompareToConstantNode<double>
{
private:
    ResolvedCompareDoubleToConstantNode(ExpressionToken operatorCode, ResolvedTypeNodeR left, double right) : ResolvedCompareToConstantNode<double>(operatorCode, left, right) {}
public:
    virtual bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ExpressionToken operatorCode, ResolvedTypeNodeR left, double right) { return new ResolvedCompareDoubleToConstantNode(operatorCode, left, right); }
};

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedCompareBooleanToConstantNode : ResolvedCompareToConstantNode<bool>
{
private:
    ResolvedCompareBooleanToConstantNode(ExpressionToken operatorCode, ResolvedTypeNodeR left, bool right) : ResolvedCompareToConstantNode<bool>(operatorCode, left, right) {}
public:
    virtual bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ExpressionToken operatorCode, ResolvedTypeNodeR left, bool right) { return new ResolvedCompareBooleanToConstantNode(operatorCode, left, right); }
};

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedCompareLongNode : ResolvedCompareNode
{
private:
    ResolvedCompareLongNode(ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedCompareNode(operatorCode, left, right){}
public:
    virtual bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) { return new ResolvedCompareLongNode(operatorCode, left, right); }
};

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedCompareDoubleNode : ResolvedCompareNode
{
private:
    ResolvedCompareDoubleNode(ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedCompareNode(operatorCode, left, right){}
public:
    virtual bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) { return new ResolvedCompareDoubleNode(operatorCode, left, right); }
};

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedCompareBooleanNode : ResolvedCompareNode
{
private:
    ResolvedCompareBooleanNode(ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedCompareNode(operatorCode, left, right){}
public:
    virtual bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) { return new ResolvedCompareBooleanNode(operatorCode, left, right); }
};

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedCompareStringNode : ResolvedCompareNode
{
    virtual bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
    ResolvedCompareStringNode(ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedCompareNode(operatorCode, left, right){}
};

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedLogicalBitNode : ResolvedBinaryNode
    {
    ExpressionToken m_operatorCode;
    ResolvedLogicalBitNode(ECN::PrimitiveType primitiveType, ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedBinaryNode(primitiveType, left, right), m_operatorCode(operatorCode) {}

public:
    virtual ::int32_t _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override;
    virtual ::int64_t _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override;
    virtual bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ECN::PrimitiveType primitiveType, ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) { return new ResolvedLogicalBitNode(primitiveType, operatorCode, left, right); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedShiftInteger : ResolvedBinaryNode
    {
private:
    ExpressionToken m_operatorCode;
    ResolvedShiftInteger(ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedBinaryNode(PRIMITIVETYPE_Integer, left, right), m_operatorCode(operatorCode) {}

public:
    virtual ::int32_t _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) { return new ResolvedShiftInteger(operatorCode, left, right); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedShiftLong : ResolvedBinaryNode
    {
private:
    ExpressionToken m_operatorCode;
    ResolvedShiftLong(ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedBinaryNode(PRIMITIVETYPE_Long, left, right), m_operatorCode(operatorCode) {}

public:
    virtual ::int64_t _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) { return new ResolvedShiftLong(operatorCode, left, right); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedIIfNode : ResolvedTypeNode
    {
    ResolvedTypeNodePtr m_condition;
    ResolvedTypeNodePtr m_true;
    ResolvedTypeNodePtr m_false;

    ExpressionToken m_operatorCode;
    ResolvedIIfNode(ECN::PrimitiveType primType, ResolvedTypeNodeR condition, ResolvedTypeNodeR ifTrue, ResolvedTypeNodeR ifFalse) : 
                                ResolvedTypeNode(primType), m_condition(&condition), m_true(&ifTrue), m_false(&ifFalse) {}

public:
    virtual ::int32_t _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override;
    virtual ::int64_t _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override;
    virtual double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override;
    virtual bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
    virtual ExpressionStatus _GetStringValue(ECValueR result, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ECN::PrimitiveType primType, ResolvedTypeNodeR condition, ResolvedTypeNodeR ifTrue, ResolvedTypeNodeR ifFalse) 
                    { return new ResolvedIIfNode(primType, condition, ifTrue, ifFalse); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedConvertIntegerToLong : ResolvedTypeNode
    {
private:
    ResolvedTypeNodePtr m_left;
    ResolvedConvertIntegerToLong(ResolvedTypeNodeR left) : ResolvedTypeNode(PRIMITIVETYPE_Long) { m_left = &left; }
protected:
    virtual bool                _IsConstant () const override { return m_left->IsConstant(); }
public:
    virtual ::int64_t _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetIntegerValue(status, context); }
    static ResolvedTypeNodePtr Create(ResolvedTypeNodeR left) { return new ResolvedConvertIntegerToLong(left); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedConvertIntegerToDouble : ResolvedTypeNode
    {
private:
    ResolvedTypeNodePtr m_left;
    ResolvedConvertIntegerToDouble(ResolvedTypeNodeR left) : ResolvedTypeNode(PRIMITIVETYPE_Double) { m_left = &left; }
protected:
    virtual bool                _IsConstant () const override { return m_left->IsConstant(); }
public:
    virtual double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetIntegerValue(status, context); }
    static ResolvedTypeNodePtr Create(ResolvedTypeNodeR left) { return new ResolvedConvertIntegerToDouble(left); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedConvertLongToDouble : ResolvedTypeNode
    {
private:
    ResolvedTypeNodePtr m_left;
    ResolvedConvertLongToDouble(ResolvedTypeNodeR left) : ResolvedTypeNode(PRIMITIVETYPE_Double) { m_left = &left; }
protected:
    virtual bool                _IsConstant () const override { return m_left->IsConstant(); }
public:
    virtual double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return (double)m_left->_GetLongValue(status, context); }
    static ResolvedTypeNodePtr Create(ResolvedTypeNodeR left) { return new ResolvedConvertLongToDouble(left); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedConvertLongToBoolean : ResolvedTypeNode
    {
private:
    ResolvedTypeNodePtr m_left;
    ResolvedConvertLongToBoolean(ResolvedTypeNodeR left) : ResolvedTypeNode(PRIMITIVETYPE_Boolean) { m_left = &left; }
protected:
    virtual bool                _IsConstant () const override { return m_left->IsConstant(); }
public:
    virtual bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetLongValue(status, context) != 0; }
    static ResolvedTypeNodePtr Create(ResolvedTypeNodeR left) { return new ResolvedConvertLongToBoolean(left); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedConvertIntegerToBoolean : ResolvedTypeNode
    {
private:
    ResolvedTypeNodePtr m_left;
    ResolvedConvertIntegerToBoolean(ResolvedTypeNodeR left) : ResolvedTypeNode(PRIMITIVETYPE_Boolean) { m_left = &left; }
protected:
    virtual bool                _IsConstant () const override { return m_left->IsConstant(); }
public:
    virtual bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetIntegerValue(status, context) != 0; }
    static ResolvedTypeNodePtr Create(ResolvedTypeNodeR left) { return new ResolvedConvertIntegerToBoolean(left); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedConvertDoubleToBoolean : ResolvedTypeNode
    {
private:
    ResolvedTypeNodePtr m_left;
    ResolvedConvertDoubleToBoolean(ResolvedTypeNodeR left) : ResolvedTypeNode(PRIMITIVETYPE_Boolean) { m_left = &left; }
protected:
    virtual bool                _IsConstant () const override { return m_left->IsConstant(); }
public:
    virtual bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetDoubleValue(status, context) != 0; }
    static ResolvedTypeNodePtr Create(ResolvedTypeNodeR left) { return new ResolvedConvertDoubleToBoolean(left); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedConvertToString : ResolvedTypeNode
    {
private:
    ResolvedTypeNodePtr m_left;
    ResolvedConvertToString(ResolvedTypeNodeR left) : ResolvedTypeNode(PRIMITIVETYPE_String) { m_left = &left; }
protected:
    virtual bool                _IsConstant () const override { return m_left->IsConstant(); }
public:
    virtual ExpressionStatus _GetStringValue(ECValueR result, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ResolvedTypeNodeR left) { return new ResolvedConvertToString(left); }
    };

END_BENTLEY_ECOBJECT_NAMESPACE

