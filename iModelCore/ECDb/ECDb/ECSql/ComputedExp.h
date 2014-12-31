/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ComputedExp.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "Exp.h"
#include "ECSqlTypeInfo.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//forward declarations of referenced Exp classes
struct ConstantValueExp;
struct ParameterExp;
struct ValueExp;

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct ComputedExp : Exp
    {
    DEFINE_EXPR_TYPE(Computed)

private:
    ECSqlTypeInfo m_typeInfo;

    static void FindHasTargetExpExpressions (std::vector<ParameterExp const*>& parameterExpList, ComputedExp const* expr);
protected:
    ComputedExp ()
        : Exp ()
        {}

    ECSqlStatus DetermineOperandsTargetTypes (ECSqlParseContext& ctx, ComputedExp const* lhs, ComputedExp const* rhs) const;

    void SetTypeInfo (ECSqlTypeInfo const& typeInfo) {m_typeInfo = typeInfo;}

public:

    virtual ~ComputedExp () {}

    ECSqlTypeInfo const& GetTypeInfo () const {return m_typeInfo;}
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct BooleanExp : ComputedExp
    {
    DEFINE_EXPR_TYPE(Boolean)

protected:
    BooleanExp ()
        : ComputedExp ()
        {}

    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override
        {
        if (mode == FinalizeParseMode::BeforeFinalizingChildren)
            {
            SetTypeInfo (ECSqlTypeInfo (ECN::PRIMITIVETYPE_Boolean));
            }

        return FinalizeParseStatus::Completed;
        }

public:
    virtual ~BooleanExp () {}
    };



//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct BooleanBinaryExp : BooleanExp
    {
    DEFINE_EXPR_TYPE(BooleanBinary)

private:
    SqlBooleanOperator m_op;
    size_t m_leftOperandExpIndex;
    size_t m_rightOperandExpIndex;

    FinalizeParseStatus CanCompareTypes (ECSqlParseContext& ctx, ComputedExp const& lhs, ComputedExp const& rhs) const;

    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    virtual Utf8String _ToString () const override;

public:
    BooleanBinaryExp(std::unique_ptr<ComputedExp> left, SqlBooleanOperator op, std::unique_ptr<ComputedExp> right);

    ComputedExp const* GetLeftOperand() const {return GetChild<ComputedExp> (m_leftOperandExpIndex);}
    ComputedExp const* GetRightOperand() const {return GetChild<ComputedExp> (m_rightOperandExpIndex);}
    SqlBooleanOperator GetOperator() const {return m_op;}

    virtual Utf8String ToECSql() const override;
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct BooleanUnaryExp : BooleanExp
    {
    DEFINE_EXPR_TYPE(BooleanUnary)
private:
    SqlBooleanUnaryOperator m_op;
    size_t m_operandExpIndex;

    virtual Utf8String _ToString () const override;

public:
    BooleanUnaryExp (std::unique_ptr<BooleanExp> operand, SqlBooleanUnaryOperator op);

    BooleanExp const* GetOperand () const {return GetChild<BooleanExp> (m_operandExpIndex);}
    SqlBooleanUnaryOperator GetOperator () const {return m_op;}

    virtual Utf8String ToECSql() const override;
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
