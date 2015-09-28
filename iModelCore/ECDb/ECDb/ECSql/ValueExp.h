/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ValueExp.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ComputedExp.h"
#include "ClassRefExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct ParameterExp;

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct ValueExp : ComputedExp
    {
    DEFINE_EXPR_TYPE(Value)
protected:
    bool m_isConstant;

    ValueExp() : ComputedExp(), m_isConstant(false) {}
    ValueExp (bool isConstant) : ComputedExp (), m_isConstant (isConstant) {}

    void SetIsConstant(bool isConstant) { m_isConstant = isConstant; }
public:
    virtual ~ValueExp () {}

    bool IsConstant() const { return m_isConstant; }
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct BetweenRangeValueExp : ValueExp
    {
DEFINE_EXPR_TYPE(BetweenRangeValue) 

private:
    size_t m_lowerBoundOperandExpIndex;
    size_t m_upperBoundOperandExpIndex;

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;

    virtual void _DoToECSql(Utf8StringR ecsql) const override { ecsql.append(GetLowerBoundOperand()->ToECSql()).append(" AND ").append(GetUpperBoundOperand()->ToECSql()); }
    virtual Utf8String _ToString() const override { return "BetweenRangeValue"; }

public:
    BetweenRangeValueExp(std::unique_ptr<ValueExp> lowerBound, std::unique_ptr<ValueExp> upperBound)
        : ValueExp ()
        {
        m_lowerBoundOperandExpIndex = AddChild (std::move(lowerBound));
        m_upperBoundOperandExpIndex = AddChild (std::move(upperBound));
        }

    ValueExp const* GetLowerBoundOperand () const {return GetChild<ValueExp> (m_lowerBoundOperandExpIndex);}
    ValueExp const* GetUpperBoundOperand() const {return GetChild<ValueExp> (m_upperBoundOperandExpIndex);}

    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct BinaryValueExp : ValueExp
    {
    DEFINE_EXPR_TYPE(BinaryValue) 

private:
    size_t m_leftOperandExpIndex;
    size_t m_rightOperandExpIndex;
    BinarySqlOperator m_op;

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
    virtual bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;

    virtual void _DoToECSql(Utf8StringR ecsql) const override;
    virtual Utf8String _ToString() const override;

public:
    BinaryValueExp(std::unique_ptr<ValueExp> lhs, BinarySqlOperator op ,std::unique_ptr<ValueExp> rhs)
        : ValueExp (lhs->IsConstant() && rhs->IsConstant()), m_op(op)
        {
        m_leftOperandExpIndex = AddChild (std::move(lhs));
        m_rightOperandExpIndex = AddChild (std::move(rhs));
        }

    ValueExp const* GetLeftOperand() const {return GetChild<ValueExp> (m_leftOperandExpIndex);}
    ValueExp const* GetRightOperand() const {return GetChild<ValueExp> (m_rightOperandExpIndex);}
    BinarySqlOperator GetOperator() const {return m_op;}
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct CastExp : ValueExp
    {
    DEFINE_EXPR_TYPE(Cast) 
private:
    Utf8String m_castTargetType;
    size_t m_castOperandIndex;
  
    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;

    virtual void _DoToECSql(Utf8StringR ecsql) const override;
    virtual Utf8String _ToString() const override;

public:
    CastExp (std::unique_ptr<ValueExp> castOperand, Utf8CP castTargetType)
        : ValueExp (castOperand->IsConstant()), m_castTargetType (castTargetType)
        {
        m_castOperandIndex = AddChild (std::move (castOperand));
        }

    ValueExp const* GetCastOperand() const { return GetChild<ValueExp> (m_castOperandIndex);}
    Utf8String GetCastTargetType() const {return m_castTargetType;}

    bool NeedsCasting () const;
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct LiteralValueExp : ValueExp
    {
    DEFINE_EXPR_TYPE(LiteralValue)
public:
    static Utf8CP const CURRENT_DATE;
    static Utf8CP const CURRENT_TIMESTAMP;

private:
    Utf8String m_value;

    LiteralValueExp (Utf8CP value, ECSqlTypeInfo type);

    BentleyStatus ResolveDataType (ECSqlParseContext&);

    virtual void _DoToECSql(Utf8StringR ecsql) const override;
    virtual Utf8String _ToString () const override;

public:
    static BentleyStatus Create (std::unique_ptr<ValueExp>&, ECSqlParseContext&, Utf8CP value, ECSqlTypeInfo type);

    Utf8StringCR GetValue () const;
    int64_t GetValueAsInt64() const;
    bool GetValueAsBoolean() const;

    static Utf8String EscapeStringLiteral (Utf8StringCR constantStringLiteral);
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      10/2013
//+===============+===============+===============+===============+===============+======
struct ECClassIdFunctionExp : ValueExp
    {
DEFINE_EXPR_TYPE(ECClassIdFunction)

private:
    static Utf8CP const NAME;

    Utf8String m_classAlias;
    RangeClassRefExp const* m_classRefExp;

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
    virtual void _DoToECSql(Utf8StringR ecsql) const override;
    virtual Utf8String _ToString() const override;

    bool HasClassAlias () const { return !m_classAlias.empty ();}

public:
    explicit ECClassIdFunctionExp(Utf8CP classAlias)
        : ValueExp(), m_classAlias(classAlias), m_classRefExp(nullptr) {}

    ~ECClassIdFunctionExp () {}

    Utf8CP GetClassAlias () const { return m_classAlias.c_str ();}

    RangeClassRefExp const* GetClassRefExp () const { return m_classRefExp;}
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct FunctionCallExp : ValueExp
    {
    DEFINE_EXPR_TYPE(FunctionCall)
private:
    Utf8String m_functionName;

    static bmap<Utf8CP, ECN::PrimitiveType, CompareIUtf8> s_builtinFunctionNonDefaultReturnTypes;

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
    virtual bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;
    virtual void _DoToECSql(Utf8StringR ecsql) const override;
    virtual Utf8String _ToString() const override;

public:
    explicit FunctionCallExp (Utf8CP functionName) : ValueExp (), m_functionName(functionName) {}
    virtual ~FunctionCallExp () {}

    Utf8CP GetFunctionName() const { return m_functionName.c_str();}

    void AddArgument (std::unique_ptr<ValueExp> argument);

    static ECN::PrimitiveType DetermineReturnType(ECDbCR, Utf8CP functionName, int argCount);
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct LikeRhsValueExp : ValueExp
    {
    DEFINE_EXPR_TYPE(LikeRhsValue) 
private:
    size_t m_rhsExpIndex;
    int m_escapeExpIndex;

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
    virtual bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;

    virtual void _DoToECSql(Utf8StringR ecsql) const override;
    virtual Utf8String _ToString() const override { return "LikeRhsValue"; }

public:
    explicit LikeRhsValueExp (std::unique_ptr<ValueExp> rhsExp, std::unique_ptr<ValueExp> escapeExp = nullptr)
        : ValueExp (rhsExp->IsConstant()), m_escapeExpIndex (UNSET_CHILDINDEX)
        {
        m_rhsExpIndex = AddChild (std::move(rhsExp));
        if (escapeExp != nullptr)
            m_escapeExpIndex = static_cast<int> (AddChild (std::move(escapeExp)));
        }

    ValueExp const* GetRhsExp () const {return GetChild<ValueExp> (m_rhsExpIndex);}
    bool HasEscapeExp () const { return m_escapeExpIndex >= 0;}
    ValueExp const* GetEscapeExp () const;
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct ParameterExp : ValueExp
    {
    DEFINE_EXPR_TYPE(Parameter)
private:
    int m_parameterIndex;
    Utf8String m_parameterName;
    ComputedExp const* m_targetExp;

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
    virtual void _DoToECSql(Utf8StringR ecsql) const override;
    virtual Utf8String _ToString() const override;

public:
    explicit ParameterExp (Utf8CP parameterName)
        : ValueExp (), m_parameterName (parameterName), m_parameterIndex (-1), m_targetExp (nullptr) {}

    void SetDefaultTargetExpInfo();
    void SetTargetExpInfo(ComputedExp const& exp);
    void SetTargetExpInfo(ECSqlTypeInfo const& targetTypeInfo);
    ComputedExp const* GetTargetExp() const { return m_targetExp; }
    bool IsNamedParameter () const { return !m_parameterName.empty (); }
    Utf8CP GetParameterName () const { return m_parameterName.c_str (); }
    int GetParameterIndex () const { return m_parameterIndex; }
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct SetFunctionCallExp : FunctionCallExp
    {
    DEFINE_EXPR_TYPE(SetFunctionCall)

    enum class Function
        {
        Any,
        Avg,
        Count,
        Every,
        Min,
        Max,
        Some,
        Sum
        };

private:
    Function m_function;
    SqlSetQuantifier m_setQuantifier;
    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;

    virtual void _DoToECSql(Utf8StringR ecsql) const override;
    virtual Utf8String _ToString() const override;

    static Utf8CP ToString(Function function);
public:
    SetFunctionCallExp(Function function, SqlSetQuantifier setQuantifier)
        : FunctionCallExp(ToString(function)), m_function(function), m_setQuantifier(setQuantifier) {}

    Function GetFunction() const { return m_function; }
    SqlSetQuantifier GetSetQuantifier() const { return m_setQuantifier; }
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      01/2014
//+===============+===============+===============+===============+===============+======
struct FoldFunctionCallExp : FunctionCallExp
    {
DEFINE_EXPR_TYPE (FoldFunctionCall)
public:
    enum class FoldFunction
        {
        Lower,
        Upper
        };

private:
    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;

    virtual Utf8String _ToString () const override;

    static Utf8CP ToString(FoldFunction);

public:
    explicit FoldFunctionCallExp(FoldFunction foldFunction) : FunctionCallExp(ToString(foldFunction)) {}
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct UnaryValueExp : ValueExp
    {
    DEFINE_EXPR_TYPE(UnaryValue)

private:
    size_t m_operandExpIndex;
    UnarySqlOperator m_op;

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
    virtual bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;
    virtual void _DoToECSql(Utf8StringR ecsql) const override;
    virtual Utf8String _ToString () const override;

public:
    UnaryValueExp(ValueExp* operand, UnarySqlOperator op)
        : ValueExp (operand->IsConstant()), m_op(op)
        {
        m_operandExpIndex = AddChild (std::unique_ptr<Exp> (operand));
        }

    ValueExp const* GetOperand() const {return GetChild<ValueExp> (m_operandExpIndex);}
    UnarySqlOperator GetOperator() const {return m_op;}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
