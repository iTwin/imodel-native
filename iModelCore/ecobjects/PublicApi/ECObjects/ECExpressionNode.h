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
    static ExpressionStatus ConvertToInt64(EvaluationResultR evalResult);
    static ExpressionStatus ConvertToDouble(EvaluationResultR evalResult);
    static ExpressionStatus ConvertToDateTime(EvaluationResultR evalResult);
    static ExpressionStatus ConvertToArithmeticOrBooleanOperand (EvaluationResultR evalResult);
    static ExpressionStatus ConvertToBooleanOperand (EvaluationResultR ecValue);
    static ExpressionStatus ConvertStringToArithmeticOperand (EvaluationResultR ecValue);
    static ExpressionStatus ConvertStringToArithmeticOrBooleanOperand (EvaluationResultR ecValue);
    static ExpressionStatus PerformArithmeticPromotion(EvaluationResult& leftResult, EvaluationResult& rightResult, bool allowStrings = false);
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

//=======================================================================================
//! A node with the data type fully determined.  A ResolvedTypeNode can be the child of a 
//! Node, but a Node that is not a ResolvedTypeNode cannot be the child of a ResolvedTypeNode. 
//! If anything in a tree's subtrees has a type that is not resolved, then the type itself
//! is not resolved.
// @bsiclass                                                    John.Gooding    09/2013
//=======================================================================================
struct ResolvedTypeNode : Node
    {
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
    virtual ECValue         _GetECValue() const override { return ECValue (m_value.c_str()); }
public:
    wchar_t const*   GetInternalValue () const { return m_value.c_str(); }

                StringLiteralNode (wchar_t const* literalValue)
        {
        m_value = literalValue;
        }

}; // End of struct StringLiteralNode

/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                                     John.Gooding    02/2011
+===============+===============+===============+===============+===============+======*/
struct          UnitsConstantNode : Node
{
private:
    WString      m_value;

protected:
    virtual WString     _ToString() const override { return m_value; }

    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides) override
        {
        //  Store the string in the ECValue and store a type in there
        //  May want to add the string to a string pool, and have this node save an index

        return ExprStatus_Success;
        }

    virtual ExpressionToken _GetOperation () const override { return TOKEN_UnitsConst; }

public:
                UnitsConstantNode (wchar_t const* value) : m_value (value) {}

}; // End of struct IntegerLiteralNode

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T, ExpressionToken TOKEN>
struct          BaseLiteralNode : LiteralNode
    {
private:
    T           m_value;

    virtual ExpressionToken     _GetOperation() const override  { return TOKEN; }
protected:
    virtual ECValue             _GetECValue() const { return ECValue (m_value); }
public:
    BaseLiteralNode (T const& value) : m_value(value) { }
    
    T           GetInternalValue() const { return m_value; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
typedef BaseLiteralNode<Int32, TOKEN_IntegerConstant>   IntegerLiteralNode;
typedef BaseLiteralNode<Int64, TOKEN_IntegerConstant>   Int64LiteralNode;
typedef BaseLiteralNode<double, TOKEN_FloatConst>       DoubleLiteralNode;
typedef BaseLiteralNode<bool, TOKEN_True>               BooleanLiteralNode;
typedef BaseLiteralNode<DPoint2d, TOKEN_PointConst>     Point2DLiteralNode;
typedef BaseLiteralNode<DPoint3d, TOKEN_PointConst>     Point3DLiteralNode;

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct DateTimeLiteralNode : BaseLiteralNode<Int64, TOKEN_DateTimeConst>
    {
protected:
    virtual ECValue         _GetECValue() const override { ECValue v; v.SetDateTimeTicks (GetInternalValue()); return v; }
public:
    DateTimeLiteralNode (Int64 ticks) : BaseLiteralNode<Int64, TOKEN_DateTimeConst>(ticks) { }
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
    NullLiteralNode() { }
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
private:
protected:
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
struct          BitWiseNode : BinaryNode
{
protected:
    virtual ExpressionStatus _GetValue(EvaluationResult& evalResult, ExpressionContextR context, 
                                        bool allowUnknown, bool allowOverrides) override;

public:
                BitWiseNode(ExpressionToken operatorCode, NodeR left, NodeR right) 
                                : BinaryNode (operatorCode, left, right) {}
};  //  End of struct BitWiseNode

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

public:
                IIfNode(NodeR condition, NodeR trueNode, NodeR falseNode)
                                : m_condition(&condition), m_true(&trueNode), m_false(&falseNode) {}
};  //  End of struct IIfNode

END_BENTLEY_ECOBJECT_NAMESPACE

