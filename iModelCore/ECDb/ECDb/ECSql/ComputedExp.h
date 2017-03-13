/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ComputedExp.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
private:
    friend struct ECSqlParser;

    bool m_hasParentheses;
    ECSqlTypeInfo m_typeInfo;

protected:
    explicit ComputedExp(Type type) : Exp(type), m_hasParentheses(false) {}

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
protected:
    explicit BooleanExp(Type type) : ComputedExp(type) { SetTypeInfo(ECSqlTypeInfo(ECN::PRIMITIVETYPE_Boolean)); }

public:
    virtual ~BooleanExp () {}
    };



//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct BinaryBooleanExp final : BooleanExp
    {
private:
    BooleanSqlOperator m_op;
    size_t m_leftOperandExpIndex;
    size_t m_rightOperandExpIndex;

    FinalizeParseStatus _FinalizeParsing (ECSqlParseContext&, FinalizeParseMode mode) override;
    bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;
    void _ToECSql(ECSqlRenderContext&) const override;
    Utf8String _ToString() const override;

    FinalizeParseStatus CanCompareTypes(ECSqlParseContext&, ComputedExp const& lhs, ComputedExp const& rhs) const;
    static bool ContainsStructArrayProperty(ECN::ECClassCR);

public:
    BinaryBooleanExp(std::unique_ptr<ComputedExp> left, BooleanSqlOperator op, std::unique_ptr<ComputedExp> right);

    ComputedExp const* GetLeftOperand() const {return GetChild<ComputedExp> (m_leftOperandExpIndex);}
    ComputedExp const* GetRightOperand() const {return GetChild<ComputedExp> (m_rightOperandExpIndex);}
    BooleanSqlOperator GetOperator() const {return m_op;}
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct BooleanFactorExp final : BooleanExp
    {
private:
    bool m_notOperator;
    size_t m_operandExpIndex;

    void _ToECSql(ECSqlRenderContext&) const override;
    Utf8String _ToString() const override;

public:
    BooleanFactorExp(std::unique_ptr<BooleanExp> operand, bool notOperator = false);

    BooleanExp const* GetOperand() const { return GetChild<BooleanExp>(m_operandExpIndex); }
    bool HasNotOperator() const { return m_notOperator; }
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      04/2015
//+===============+===============+===============+===============+===============+======
struct UnaryPredicateExp final : BooleanExp
    {
private:
    size_t m_booleanValueExpIndex;

    FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
    void _ToECSql(ECSqlRenderContext& ctx) const override;
    Utf8String _ToString() const override { return "UnaryPredicate"; }

public:
    explicit UnaryPredicateExp(std::unique_ptr<ValueExp> predicateExp);

    ValueExp const* GetValueExp() const { return GetChild<ValueExp>(m_booleanValueExpIndex); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
