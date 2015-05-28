/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ComputedExp.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "Exp.h"
#include "ECSqlTypeInfo.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//forward declarations of referenced Exp classes
struct ParameterExp;
struct ValueExp;

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct ComputedExp : Exp
    {
    DEFINE_EXPR_TYPE(Computed)

private:
    friend struct ECSqlParser;

    bool m_hasParentheses;
    ECSqlTypeInfo m_typeInfo;

    static void FindHasTargetExpExpressions (std::vector<ParameterExp const*>& parameterExpList, ComputedExp const* expr);
protected:
    explicit ComputedExp() : Exp(), m_hasParentheses(false) {}

    ECSqlStatus DetermineOperandsTargetTypes (ECSqlParseContext& ctx, ComputedExp const* lhs, ComputedExp const* rhs) const;

    void SetTypeInfo (ECSqlTypeInfo const& typeInfo) {m_typeInfo = typeInfo;}
    void SetHasParentheses() { m_hasParentheses = true; }

public:
    virtual ~ComputedExp () {}

    bool HasParentheses() const { return m_hasParentheses; }
    ECSqlTypeInfo const& GetTypeInfo () const {return m_typeInfo;}
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct BooleanExp : ComputedExp
    {
    DEFINE_EXPR_TYPE(Boolean)

protected:
    BooleanExp() : ComputedExp() { SetTypeInfo(ECSqlTypeInfo(ECN::PRIMITIVETYPE_Boolean)); }

public:
    virtual ~BooleanExp () {}
    };



//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct BinaryBooleanExp : BooleanExp
    {
    DEFINE_EXPR_TYPE(BinaryBoolean)

private:
    BooleanSqlOperator m_op;
    size_t m_leftOperandExpIndex;
    size_t m_rightOperandExpIndex;

    FinalizeParseStatus CanCompareTypes (ECSqlParseContext& ctx, ComputedExp const& lhs, ComputedExp const& rhs) const;

    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    virtual Utf8String _ToString () const override;

public:
    BinaryBooleanExp(std::unique_ptr<ComputedExp> left, BooleanSqlOperator op, std::unique_ptr<ComputedExp> right);

    ComputedExp const* GetLeftOperand() const {return GetChild<ComputedExp> (m_leftOperandExpIndex);}
    ComputedExp const* GetRightOperand() const {return GetChild<ComputedExp> (m_rightOperandExpIndex);}
    BooleanSqlOperator GetOperator() const {return m_op;}

    virtual Utf8String ToECSql() const override;
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct BooleanFactorExp : BooleanExp
    {
    DEFINE_EXPR_TYPE(BooleanFactor)

private:
    bool m_notOperator;
    size_t m_operandExpIndex;

    virtual Utf8String _ToString() const override;

public:
    BooleanFactorExp(std::unique_ptr<BooleanExp> operand, bool notOperator = false);

    BooleanExp const* GetOperand() const { return GetChild<BooleanExp>(m_operandExpIndex); }
    bool HasNotOperator() const { return m_notOperator; }

    virtual Utf8String ToECSql() const override;
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      04/2015
//+===============+===============+===============+===============+===============+======
struct UnaryPredicateExp : BooleanExp
    {
    DEFINE_EXPR_TYPE(UnaryPredicate)
private:
    size_t m_booleanValueExpIndex;

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    virtual Utf8String _ToString() const override { return "UnaryPredicate"; }

public:
    explicit UnaryPredicateExp(std::unique_ptr<ValueExp> predicateExp);

    ValueExp const* GetValueExp() const { return GetChild<ValueExp>(m_booleanValueExpIndex); }

    virtual Utf8String ToECSql() const override;
    };



END_BENTLEY_SQLITE_EC_NAMESPACE
