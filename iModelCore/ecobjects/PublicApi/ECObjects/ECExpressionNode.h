/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECExpressionNode.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
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
    wchar_t         m_tokenBuilder [1024];
    size_t          m_inputIndex;
    size_t          m_outputIndex;
    size_t          m_maxInputIndex;
    size_t          m_maxOutputIndex;
    size_t          m_startPosition;

    wchar_t const*  m_inputCP;              //  Points into contents of m_inputString
    Bentley::WString   m_inputString;

    wchar_t const*          getTokenStringCP ();
    wchar_t                 GetCurrentChar ();
    wchar_t                 GetNextChar  ();
    void                    PushBack ();
    wchar_t                 PeekChar  ();
    wchar_t                 GetDigits (ExpressionToken&  tokenType, bool isOctal);
    bool                    IsHexDigit (wchar_t ch);
    wchar_t                 GetHexConstant (ExpressionToken&  tokenType);
    ExpressionToken         GetNumericConstant ();
    ExpressionToken         GetString ();
    ExpressionToken         GetIdentifier ();
    ExpressionToken         ScanToken ();
    void                    MarkOffset();
                            Lexer (wchar_t const* inputString);
public:
    static LexerPtr         Create(wchar_t const* inputString);
    ECOBJECTS_EXPORT static WString     GetString(ExpressionToken tokenName);
    void                    Advance ();
    ExpressionToken         GetTokenType ();
    ExpressionToken         GetTokenModifier ();
    wchar_t const*          GetTokenStringCP ();
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
public:

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
    static ExpressionStatus PerformMultiplication(EvaluationResultR resultOut, EvaluationResultR left, EvaluationResultR right);
    static ExpressionStatus PerformExponentiation(EvaluationResultR resultOut, EvaluationResultR left, EvaluationResultR right);
    static ExpressionStatus PerformIntegerDivision(EvaluationResultR resultOut, EvaluationResultR left, EvaluationResultR right);
    static ExpressionStatus PerformDivision(EvaluationResultR resultOut, EvaluationResultR left, EvaluationResultR right);
    static ExpressionStatus PerformMod(EvaluationResultR resultOut, EvaluationResultR left, EvaluationResultR right);
    static ExpressionStatus PerformLikeTest(EvaluationResultR resultOut, EvaluationResultR left, EvaluationResultR right) { return ExprStatus_NotImpl; }
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
    static  void        DetermineKnownUnitsSame (UnitsTypeR units, NodeCPVector& nodes);
    static  void        DetermineKnownUnitsSame (UnitsTypeR units, NodeCR rightMost);

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
    WString     m_summary;
    WString     m_detail1;
    WString     m_detail2;

protected:
    virtual bool            _HasError () override { return true; }

                ErrorNode (wchar_t const* summary, wchar_t const* detail1, wchar_t const* detail12) 
                    : m_summary(summary), m_detail1(detail1), m_detail2(detail12) {}
    virtual WString     _ToString() const override { return L"ERROR"; }
    virtual ExpressionToken _GetOperation () const override { return TOKEN_Error; }

public:
    static ErrorNodePtr Create(wchar_t const* summary, wchar_t const* detail1, wchar_t const* detail12)
        {
        if (NULL == detail1)
            detail1 = L"";
        return new ErrorNode (summary, NULL == detail1 ? L"" : detail1, NULL== detail12 ? L"" : detail12);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct LiteralNode : ResolvedTypeNode
    {
private:
    virtual WString             _ToString() const override      { return _GetECValue().ToString(); }
    virtual ExpressionStatus    _GetValue (EvaluationResult& evalResult, ExpressionContextR, bool, bool) override
        {
        evalResult = _GetECValue();
        return ExprStatus_Success;
        }
protected:
    virtual ECValue             _GetECValue() const = 0;
    virtual bool                _IsConstant () const override { return true; }
    LiteralNode(ECN::PrimitiveType primtiveType) : ResolvedTypeNode(primtiveType) {}
public:
    ECValue                     GetECValue() const { return _GetECValue(); }
    };

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          StringLiteralNode : LiteralNode
{
private:
    WString     m_value;

protected:
    virtual WString     _ToString() const override 
        { 
        WString retval = L"\"";
        retval.append (m_value);
        retval.append (L"\"");
        return retval;
        }

    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides) override
        {
        evalResult = ECN::ECValue(m_value.c_str(), false);

        return ExprStatus_Success;
        }

    virtual ExpressionToken _GetOperation () const override { return TOKEN_StringConst; }
    virtual bool            _IsConstant () const override { return true; }
    virtual ECValue         _GetECValue() const override { return ECValue (m_value.c_str()); }
public:
    ExpressionStatus _GetStringValue(ECValueR result, ExpressionContextR context) override
        {
        result.SetString(m_value.c_str(), false);
        return ExprStatus_Success;
        }

    wchar_t const*   GetInternalValue () { return m_value.c_str(); }

                StringLiteralNode (wchar_t const* literalValue) : LiteralNode(PRIMITIVETYPE_String)
        {
        m_value = literalValue;
        }

}; // End of struct StringLiteralNode

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T, ExpressionToken TOKEN, ECN::PrimitiveType typeID>
struct          BaseLiteralNode : LiteralNode
    {
protected:
    T           m_value;


    virtual ExpressionToken     _GetOperation() const override  { return TOKEN; }
    virtual ECValue             _GetECValue() const { return ECValue (m_value); }
    virtual bool                _IsConstant () const override { return true; }
    virtual ResolvedTypeNodePtr _GetResolvedTree(ExpressionResolverR context) override { return this; }

public:
    BaseLiteralNode (T const& value) : LiteralNode(typeID), m_value(value) { }
    
    
    T           GetInternalValue() const { return m_value; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct IntegerLiteralNode : BaseLiteralNode<::Int32, TOKEN_IntegerConstant, PRIMITIVETYPE_Integer>
    {
protected:
    virtual bool _SupportsGetBooleanValue() override { return true; }
    virtual bool _SupportsGetDoubleValue() override { return true; }
    virtual bool _SupportsGetLongValue() override { return true; }
public:
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override { return m_value != 0; }
    ::Int32 _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { return m_value; }
    ::Int64 _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { return m_value; }
    double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return m_value; }
    IntegerLiteralNode(::Int32 value) : BaseLiteralNode(value) {}
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct Int64LiteralNode : BaseLiteralNode<Int64, TOKEN_IntegerConstant, PRIMITIVETYPE_Long>
    {
protected:
    virtual bool _SupportsGetBooleanValue() override { return true; }
    virtual bool _SupportsGetDoubleValue() override { return true; }
    virtual bool _SupportsGetIntegerValue() override { return true; }
public:
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override { return m_value != 0; }
    ::Int32 _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { return (::Int32)m_value; }
    ::Int64 _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { return m_value; }
    double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return m_value; }
    Int64LiteralNode(::Int64 value) : BaseLiteralNode(value) {}
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct DoubleLiteralNode : BaseLiteralNode<double, TOKEN_FloatConst, PRIMITIVETYPE_Double>
    {
public:
    double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return m_value; }
    DoubleLiteralNode(double value) : BaseLiteralNode(value) {}
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct BooleanLiteralNode : BaseLiteralNode<bool, TOKEN_True, PRIMITIVETYPE_Boolean>
    {
protected:
    virtual bool _SupportsGetIntegerValue() override { return true; }
public:
    ::Int32 _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { return (::Int32)m_value; }
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override { return m_value; }
    BooleanLiteralNode(bool value) : BaseLiteralNode(value) {}
    };

typedef BaseLiteralNode<DPoint2d, TOKEN_PointConst, PRIMITIVETYPE_Point2D>     Point2DLiteralNode;
typedef BaseLiteralNode<DPoint3d, TOKEN_PointConst, PRIMITIVETYPE_Point3D>     Point3DLiteralNode;

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct DateTimeLiteralNode : BaseLiteralNode<Int64, TOKEN_DateTimeConst, PRIMITIVETYPE_DateTime>
    {
protected:
    virtual ECValue         _GetECValue() const override { ECValue v; v.SetDateTimeTicks (GetInternalValue()); return v; }
public:
    DateTimeLiteralNode (Int64 ticks) : BaseLiteralNode<Int64, TOKEN_DateTimeConst, PRIMITIVETYPE_DateTime>(ticks) { }
    ::Int64 _GetDateTimeValue(ExpressionStatus& status, ExpressionContextR context) override { return m_value; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct          NullLiteralNode : LiteralNode
    {
protected:
    virtual WString             _ToString() const override { return L"Null"; }
    virtual ExpressionStatus    _GetValue (EvaluationResult& evalResult, ExpressionContextR, bool, bool)
        {
        evalResult = ECN::ECValue (/*null*/);
        return ExprStatus_Success;
        }
    virtual ExpressionToken     _GetOperation () const override { return TOKEN_Null; }
    virtual ECValue             _GetECValue() const override { return ECValue (/*null*/); }
public:
    //  If we don't provide some type code it can't be a ResolvedTypeNode preventing the entire
    //  tree above it from being ResolvedTypeNode.  Nothing should match PRIMITIVETYPE_Binary.
    NullLiteralNode() : LiteralNode(PRIMITIVETYPE_Binary) { }
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
    virtual WString     _ToString() const override 
        {
        return L"";
        }

    virtual ExpressionToken _GetOperation () const override { return TOKEN_PrimaryList; }
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides) override;

    virtual bool            _HasError () override { return false; }
    virtual void            _DetermineKnownUnits(UnitsTypeR unitsType) const override { }
    virtual void            _ForceUnitsOrder(UnitsTypeCR  knownType) override {}
    virtual ResolvedTypeNodePtr _GetResolvedTree(ExpressionResolverR context) override { return context._ResolvePrimaryList(*this); }

public:
    ECOBJECTS_EXPORT wchar_t const*     GetName(size_t index) const;
    ECOBJECTS_EXPORT size_t             GetNumberOfOperators() const;
    ECOBJECTS_EXPORT NodeP              GetOperatorNode(size_t index) const;    // NEEDSWORK: const method returning member as non-const pointer...propagates to CallNode::InvokeXXXMethod()...
    ECOBJECTS_EXPORT ExpressionToken    GetOperation(size_t index) const;

    static PrimaryListNodePtr Create() { return new PrimaryListNode(); }
    void                    AppendCallNode(CallNodeR  callNode);
    void                    AppendNameNode(IdentNodeR  nameNode);
    void                    AppendArrayNode(LBracketNodeR  lbracketNode);
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
    WString     m_value;
    bvector<WString> m_qualifiers;

                IdentNode(wchar_t const* name) : m_value(name) {}
    friend      struct DotNode;
protected:
    virtual WString     _ToString() const override { return m_value; }

    //  May want to distinguish between compiled category and resolved category
    virtual ExpressionToken _GetOperation () const override { return TOKEN_Ident; }

    virtual bool            _HasError () override { return false; }
    virtual void            _DetermineKnownUnits(UnitsTypeR unitsType) const override { }
    virtual void            _ForceUnitsOrder(UnitsTypeCR  knownType) override {}

public:
    void                    PushQualifier(WCharCP rightName);
    wchar_t const*          GetName() const { return m_value.c_str(); }
    static IdentNodePtr     Create(wchar_t const*name) { return new IdentNode(name); }

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
    WString                 m_memberName;

    virtual bool        _Traverse(NodeVisitorR visitor) const
        {
        return visitor.ProcessNode(*this); 
        }

protected:
    virtual WString     _ToString() const override 
        {
        return L"." + m_memberName;
        }

    //  May want to distinguish between compiled category and resolved category
    virtual ExpressionToken _GetOperation () const override { return TOKEN_Dot; }

    virtual bool            _HasError () override { return false; }
    virtual void            _DetermineKnownUnits(UnitsTypeR unitsType) const override { }
    virtual void            _ForceUnitsOrder(UnitsTypeCR  knownType) override {}

public:

    wchar_t const*   GetMemberName() { return m_memberName.c_str(); }

                DotNode (wchar_t const* memberName) : IdentNode(memberName), m_memberName(memberName) {}
    static DotNodePtr Create(wchar_t const* memberName) { return new DotNode(memberName); }

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
    virtual WString     _ToString() const override { return L"["; }
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
    virtual void            _DetermineKnownUnits(UnitsTypeR unitsType) const override { }
    virtual void            _ForceUnitsOrder(UnitsTypeCR  knownType) override {}
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
    virtual WString     _ToString() const override { return Lexer::GetString(m_operator); }

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
    virtual void            _DetermineKnownUnits(UnitsTypeR unitsType) const override { }
    virtual void            _ForceUnitsOrder(UnitsTypeCR  knownType) override {}

public:
                UnaryNode (ExpressionToken tokenId, NodeR left) : 
                                    m_operator(tokenId), m_left(&left) {}

};  //  End of struct Unary

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
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides) override;

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

    virtual WString     _ToString() const override { return Lexer::GetString(m_operatorCode); }

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
    virtual void            _DetermineKnownUnits(UnitsTypeR unitsType) const override { }
    virtual void            _ForceUnitsOrder(UnitsTypeCR  knownType) override {}

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
    virtual void            _DetermineKnownUnits(UnitsTypeR unitsType) const override;
    virtual void            _ForceUnitsOrder(UnitsTypeCR  knownType) override;

    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides);


    virtual ExpressionStatus _Promote(EvaluationResult& leftResult, EvaluationResult& rightResult, 
                                        ExpressionContextR context) { return ExprStatus_NotImpl; }

    //  Undecided if this is pure virtual or a default implementation.
    virtual ExpressionStatus _PerformOperation(EvaluationResultR result,
                                        EvaluationResultCR leftResult, EvaluationResultCR rightResult, 
                                        ExpressionContextR context)  { return ExprStatus_NotImpl; }

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
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides) override;

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
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides) override;
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
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides) override;

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
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides) override;

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
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides) override;
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
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides) override;

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
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides) override;
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
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides) override;

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
    virtual WString     _ToString() const override { return L""; }

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
    ExpressionStatus EvaluateArguments(EvaluationResultVector& results, ExpressionContextR context);
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
    WString             m_methodName;
    bool                m_dotted;

    virtual bool        _Traverse(NodeVisitorR visitor) const
        {
        bool    retval = visitor.ProcessNode(*this);;
        if (!retval)
            return false;

        return m_arguments->Traverse(visitor);
        }

    //  Constructor
                CallNode(ArgumentTreeNodeR arguments, wchar_t const* methodName, bool dotted) : 
                                    m_arguments (&arguments), m_methodName(methodName), m_dotted(dotted) {}

protected:
    virtual WString     _ToString() const override 
        {
        WString     retval;
        if (m_dotted)
            retval.append(L".");
        retval.append(m_methodName);
        return retval; 
        }

    virtual ExpressionToken _GetOperation () const override { return TOKEN_LParen; }


public:
    wchar_t const*          GetMethodName() const { return m_methodName.c_str(); }
    ArgumentTreeNode const* GetArguments() const { return m_arguments.get(); }
    static CallNodePtr      Create(ArgumentTreeNodeR arguments, wchar_t const*methodName, bool dotted)
                                    { return new CallNode(arguments, methodName, dotted); }

    ExpressionStatus    InvokeInstanceMethod(EvaluationResult& evalResult, EvaluationResultCR instanceData, ExpressionContextR context);
    ExpressionStatus    InvokeStaticMethod(EvaluationResult& evalResult, MethodReferenceR  methodReference, ExpressionContextR context);
    ExpressionStatus    InvokeStaticMethod(EvaluationResult& evalResult, ExpressionContextR context);

};  //  End of struct CallNode

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
    virtual WString     _ToString() const override { return Lexer::GetString(TOKEN_IIf); }
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
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides);
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
    ::Int32 _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { CheckInteger(); return -m_operand->_GetIntegerValue(status, context); }
    ::Int64 _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { CheckLong(); return -m_operand->_GetLongValue(status, context); }
    double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { CheckDouble(); return -m_operand->_GetDoubleValue(status, context); }
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
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override { return !m_operand->_GetBooleanValue(status, context); }
    ::Int32 _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { return ~m_operand->_GetIntegerValue(status, context); }
    ::Int64 _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { return ~m_operand->_GetLongValue(status, context); }
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
    ::Int32 _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetIntegerValue(status, context) * m_right->_GetIntegerValue(status, context); }
    ::Int64 _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetLongValue(status, context) * m_right->_GetLongValue(status, context); }
    double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetDoubleValue(status, context) * m_right->_GetDoubleValue(status, context); }

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
        ::Int32 m_i;
        ::Int64 m_i64;
        double  m_d;
        } m_right;

    ResolvedMultiplyConstantNode(ResolvedTypeNodeR left, ::Int32 value) : ResolvedTypeNode(PRIMITIVETYPE_Integer), m_left(&left) { m_right.m_i = value; }
    ResolvedMultiplyConstantNode(ResolvedTypeNodeR left, ::Int64 value) : ResolvedTypeNode(PRIMITIVETYPE_Long), m_left(&left) { m_right.m_i64 = value; }
    ResolvedMultiplyConstantNode(ResolvedTypeNodeR left, double value) : ResolvedTypeNode(PRIMITIVETYPE_Double), m_left(&left) { m_right.m_d = value; }
public:
    ::Int32 _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { CheckInteger(); return m_left->_GetIntegerValue(status, context) * m_right.m_i; }
    ::Int64 _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { CheckLong(); return m_left->_GetLongValue(status, context) * m_right.m_i64; }
    double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { CheckDouble(); return m_left->_GetDoubleValue(status, context) * m_right.m_d; }

    static ResolvedTypeNodePtr CreateInteger(ResolvedTypeNodeR left, ::Int32 value) { return new ResolvedMultiplyConstantNode(left, value); }
    static ResolvedTypeNodePtr CreateInt64(ResolvedTypeNodeR left, ::Int64 value) { return new ResolvedMultiplyConstantNode(left, value); }
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
    ::Int32 _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override 
        {
        ::Int32 divisor = m_right->_GetIntegerValue(status, context);
        if (0 == divisor)
            {
            if (ExprStatus_Success == status)
                status = ExprStatus_DivideByZero;
            return 0;
            }

        return m_left->_GetIntegerValue(status, context) % divisor; 
        }

    ::Int64 _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override
        {
        ::Int64 divisor = m_right->_GetLongValue(status, context);
        if (0 == divisor)
            {
            if (ExprStatus_Success == status)
                status = ExprStatus_DivideByZero;
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
    ::Int32 _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override 
        {
        ::Int32 divisor = m_right->_GetIntegerValue(status, context);
        if (0 == divisor)
            {
            if (ExprStatus_Success == status)
                status = ExprStatus_DivideByZero;
            return 0;
            }

        return m_left->_GetIntegerValue(status, context) / divisor; 
        }

    ::Int64 _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override
        {
        ::Int64 divisor = m_right->_GetLongValue(status, context);
        if (0 == divisor)
            {
            if (ExprStatus_Success == status)
                status = ExprStatus_DivideByZero;
            return 0;
            }

        return m_left->_GetLongValue(status, context) / divisor; 
        }

    double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override
        {
        double divisor = m_right->_GetDoubleValue(status, context);
        if (0.0 == divisor)
            {
            if (ExprStatus_Success == status)
                status = ExprStatus_DivideByZero;
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
    double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override
        {
        double divisor = m_right->_GetDoubleValue(status, context);
        if (0.0 == divisor)
            {
            if (ExprStatus_Success == status)
                status = ExprStatus_DivideByZero;
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
    double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetDoubleValue(status, context) / m_divisor; }
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
    double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override
        {
        double divisor = m_divisor->_GetDoubleValue(status, context);
        if (0.0 == divisor)
            {
            if (ExprStatus_Success == status)
                status = ExprStatus_DivideByZero;
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
    ::Int32 _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetIntegerValue(status, context) + m_right->_GetIntegerValue(status, context); }
    ::Int64 _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetLongValue(status, context) + m_right->_GetLongValue(status, context); }
    double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetDoubleValue(status, context) + m_right->_GetDoubleValue(status, context); }

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
        ::Int32     m_i;
        ::Int64     m_i64;
        double      m_d;
        } m_right;
    ResolvedTypeNodePtr m_left;
    ResolvedAddConstantNode(ECN::PrimitiveType resultType, ResolvedTypeNodeR left, ECValueCR right);
public:
    ::Int32 _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetIntegerValue(status, context) + m_right.m_i; }
    ::Int64 _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetLongValue(status, context) + m_right.m_i64; }
    double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetDoubleValue(status, context) + m_right.m_d; }

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
    ::Int32 _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetIntegerValue(status, context) - m_right->_GetIntegerValue(status, context); }
    ::Int64 _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetLongValue(status, context) - m_right->_GetLongValue(status, context); }
    double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetDoubleValue(status, context) - m_right->_GetDoubleValue(status, context); }

    static ResolvedTypeNodePtr Create(ECN::PrimitiveType primitiveType, ResolvedTypeNodeR left, ResolvedTypeNodeR right) { return new ResolvedSubtractNode(primitiveType, left, right); }
    };

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedConcatenateNode : ResolvedBinaryNode
    {
    ResolvedConcatenateNode(ResolvedTypeNodeR left, ResolvedTypeNodeR right) : ResolvedBinaryNode(PRIMITIVETYPE_String, left, right) {}
public:
    ExpressionStatus _GetStringValue(ECValueR result, ExpressionContextR context) override;
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
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
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
struct          ResolvedCompareIntegerToConstantNode : ResolvedCompareToConstantNode<::Int32>
{
private:
    ResolvedCompareIntegerToConstantNode(ExpressionToken operatorCode, ResolvedTypeNodeR left, ::Int32 right) : ResolvedCompareToConstantNode<::Int32>(operatorCode, left, right) {}
public:
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ExpressionToken operatorCode, ResolvedTypeNodeR left, ::Int32 right) { return new ResolvedCompareIntegerToConstantNode(operatorCode, left, right); }
};

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedCompareLongToConstantNode : ResolvedCompareToConstantNode<::Int64>
{
private:
    ResolvedCompareLongToConstantNode(ExpressionToken operatorCode, ResolvedTypeNodeR left, ::Int64 right) : ResolvedCompareToConstantNode<::Int64>(operatorCode, left, right) {}
public:
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ExpressionToken operatorCode, ResolvedTypeNodeR left, ::Int64 right) { return new ResolvedCompareLongToConstantNode(operatorCode, left, right); }
};

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedCompareDoubleToConstantNode : ResolvedCompareToConstantNode<double>
{
private:
    ResolvedCompareDoubleToConstantNode(ExpressionToken operatorCode, ResolvedTypeNodeR left, double right) : ResolvedCompareToConstantNode<double>(operatorCode, left, right) {}
public:
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
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
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
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
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
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
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
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
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ExpressionToken operatorCode, ResolvedTypeNodeR left, ResolvedTypeNodeR right) { return new ResolvedCompareBooleanNode(operatorCode, left, right); }
};

/*=================================================================================**//**
* @bsiclass                                                      John.Gooding    09/2013
+===============+===============+===============+===============+===============+======*/
struct          ResolvedCompareStringNode : ResolvedCompareNode
{
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
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
    ::Int32 _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override;
    ::Int64 _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override;
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
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
    ::Int32 _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override;
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
    ::Int64 _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override;
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
    ::Int32 _GetIntegerValue(ExpressionStatus& status, ExpressionContextR context) override;
    ::Int64 _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override;
    double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override;
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override;
    ExpressionStatus _GetStringValue(ECValueR result, ExpressionContextR context) override;
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
    ::Int64 _GetLongValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetIntegerValue(status, context); }
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
    double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetIntegerValue(status, context); }
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
    double _GetDoubleValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetLongValue(status, context); }
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
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetLongValue(status, context) != 0; }
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
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetIntegerValue(status, context) != 0; }
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
    bool _GetBooleanValue(ExpressionStatus& status, ExpressionContextR context) override { return m_left->_GetDoubleValue(status, context) != 0; }
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
    ExpressionStatus _GetStringValue(ECValueR result, ExpressionContextR context) override;
    static ResolvedTypeNodePtr Create(ResolvedTypeNodeR left) { return new ResolvedConvertToString(left); }
    };

END_BENTLEY_ECOBJECT_NAMESPACE

