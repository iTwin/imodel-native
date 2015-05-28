/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ComputedExp.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ComputedExp.h"
#include "ValueExp.h"
#include "ExpHelper.h"

using namespace std;
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//NOTE: The code in this file is sorted by the class names. This should make maintenance easier as there are too many types in this single file

//*************************** ComputedExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       09/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ComputedExp::FindHasTargetExpExpressions (vector<ParameterExp const*>& parameterExpList, ComputedExp const* expr)
    {
    const auto type = expr->GetType ();
    if (type == Exp::Type::Parameter)
        {
        parameterExpList.push_back (static_cast<ParameterExp const*>(expr));
        }

    vector<ComputedExp const*> computedChildren;
    for(auto child : expr->GetChildren())
        {
        auto computedChild = dynamic_cast<ComputedExp const*>(child);
        if (computedChild != nullptr)
            {
            FindHasTargetExpExpressions (parameterExpList, computedChild);
            }
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlStatus ComputedExp::DetermineOperandsTargetTypes (ECSqlParseContext& ctx, ComputedExp const* lhs, ComputedExp const* rhs) const
    {
    if (lhs == nullptr || rhs == nullptr)
        {
        BeAssert (false);
        return ctx.SetError (ECSqlStatus::ProgrammerError, nullptr);
        }

    vector<ParameterExp const*> lhsParameterExpList;
    FindHasTargetExpExpressions (lhsParameterExpList, lhs);

    vector<ParameterExp const*> rhsParameterExpList;
    FindHasTargetExpExpressions (rhsParameterExpList, rhs);

    for (auto parameterExp : lhsParameterExpList)
        {
        const_cast<ParameterExp*> (parameterExp)->SetTargetExp (*rhs);
        }

    for (auto parameterExp : rhsParameterExpList)
        {
        const_cast<ParameterExp*> (parameterExp)->SetTargetExp (*lhs);
        }

    return ECSqlStatus::Success;
    }

//*************************** BooleanBinaryExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BinaryBooleanExp::BinaryBooleanExp (std::unique_ptr<ComputedExp> left, BooleanSqlOperator op, std::unique_ptr<ComputedExp> right) 
    : BooleanExp (), m_op(op)
    {
    m_leftOperandExpIndex = AddChild (std::move (left));
    m_rightOperandExpIndex = AddChild (std::move (right));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus BinaryBooleanExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    auto lhs = GetLeftOperand ();
    auto rhs = GetRightOperand ();

    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        //if operands are parameters set the target exp in those expressions
        if (m_op != BooleanSqlOperator::And && m_op != BooleanSqlOperator::Or)
            {
            if (DetermineOperandsTargetTypes (ctx, lhs, rhs) != ECSqlStatus::Success)
                return FinalizeParseStatus::Error;
            }

        return FinalizeParseStatus::NotCompleted;
        }

    ComputedExp const* expWithVaryingTypeInfo = nullptr;
    ComputedExp const* expWithRegularTypeInfo = nullptr;
    if (lhs->GetTypeInfo ().GetKind () == ECSqlTypeInfo::Kind::Varies)
        {
        expWithVaryingTypeInfo = lhs;
        expWithRegularTypeInfo = rhs;
        }

    if (rhs->GetTypeInfo ().GetKind () == ECSqlTypeInfo::Kind::Varies)
        {
        //only one side can be of Kind::Varies. If lhs is Varies, expWithVaryingTypeInfo was already set and no longer is null
        if (expWithVaryingTypeInfo != nullptr)
            {
            ctx.SetError (ECSqlStatus::InvalidECSql, "Only one operand of the expression '%s' can be an expression list.", ToECSql ().c_str ());
            return FinalizeParseStatus::Error;
            }

        expWithVaryingTypeInfo = rhs;
        expWithRegularTypeInfo = lhs;
        }

    if (expWithVaryingTypeInfo != nullptr)
        {
        for (auto child : expWithVaryingTypeInfo->GetChildren ())
            {
            BeAssert (dynamic_cast<ComputedExp const*> (child) != nullptr);
            auto childComputedExp = static_cast<ComputedExp const*> (child);

            auto stat = CanCompareTypes (ctx, *expWithRegularTypeInfo, *childComputedExp);
            if (stat != Exp::FinalizeParseStatus::Completed)
                return stat;
            }

        return Exp::FinalizeParseStatus::Completed;
        }
    else
        return CanCompareTypes (ctx, *lhs, *rhs);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus BinaryBooleanExp::CanCompareTypes (ECSqlParseContext& ctx, ComputedExp const& lhs, ComputedExp const& rhs) const
    {
    auto const& lhsTypeInfo = lhs.GetTypeInfo ();
    auto const& rhsTypeInfo = rhs.GetTypeInfo ();

    //first check whether types on both sides match generally for comparisons
    Utf8String canCompareErrorMessage;
    if (!lhsTypeInfo.Matches (rhsTypeInfo, &canCompareErrorMessage))
        {
        if (canCompareErrorMessage.empty ())
            ctx.SetError (ECSqlStatus::InvalidECSql, "Type mismatch in expression '%s'.", ToECSql ().c_str ());
        else
            ctx.SetError (ECSqlStatus::InvalidECSql, "Type mismatch in expression '%s': %s", ToECSql ().c_str (), canCompareErrorMessage.c_str ());

        return FinalizeParseStatus::Error;
        }

    if ((m_op == BooleanSqlOperator::Like || m_op == BooleanSqlOperator::NotLike) &&
        (!lhsTypeInfo.IsPrimitive () || !rhsTypeInfo.IsPrimitive () ||
        lhsTypeInfo.GetPrimitiveType () != PRIMITIVETYPE_String || rhsTypeInfo.GetPrimitiveType () != PRIMITIVETYPE_String))
        {
        ctx.SetError (ECSqlStatus::InvalidECSql, "Type mismatch in expression '%s'. LIKE operator only supported with string operands.", ToECSql ().c_str ());
        return FinalizeParseStatus::Error;
        }

    //Limit operators for point expressions
    //cannot assume both sides have same types as one can still represent the SQL NULL
    if (lhsTypeInfo.IsPoint () || rhsTypeInfo.IsPoint ())
        {
        switch (m_op)
            {
            case BooleanSqlOperator::EqualTo:
            case BooleanSqlOperator::NotEqualTo:
            case BooleanSqlOperator::In:
            case BooleanSqlOperator::NotIn:
            case BooleanSqlOperator::Is:
            case BooleanSqlOperator::IsNot:
                break;

            default:
                ctx.SetError (ECSqlStatus::InvalidECSql, "Type mismatch in expression '%s'. Operator not supported with point operands.", ToECSql ().c_str ());
                return FinalizeParseStatus::Error;
            }
        }

    const auto lhsTypeKind = lhsTypeInfo.GetKind ();
    const auto rhsTypeKind = rhsTypeInfo.GetKind ();

    //For geometry props only IS NULL / IS NOT NULL is supported
    //cannot assume both sides have same types as one can still represent the SQL NULL
    if (lhsTypeInfo.IsGeometry () || rhsTypeInfo.IsGeometry ())
        {
        if ((m_op != BooleanSqlOperator::Is && m_op != BooleanSqlOperator::IsNot) ||
            ((lhsTypeInfo.IsGeometry () && rhsTypeKind != ECSqlTypeInfo::Kind::Null) ||
            (lhsTypeKind != ECSqlTypeInfo::Kind::Null && rhsTypeInfo.IsGeometry ())))
            {
            //for prim arrays only is null and arrays not supported in where expressions for now
            ctx.SetError (ECSqlStatus::InvalidECSql, "Type mismatch in expression '%s'. For geometry properties only IS NULL and IS NOT NULL is supported.", ToECSql ().c_str ());
            return FinalizeParseStatus::Error;
            }
        }

    // For primitive arrays only IS NULL / IS NOT NULL is supported
    //cannot assume both sides have same types as one can still represent the SQL NULL
    if (lhsTypeKind == ECSqlTypeInfo::Kind::PrimitiveArray ||
        rhsTypeKind == ECSqlTypeInfo::Kind::PrimitiveArray)
        {
        if ((m_op != BooleanSqlOperator::Is && m_op != BooleanSqlOperator::IsNot) ||
            ((lhsTypeKind == ECSqlTypeInfo::Kind::PrimitiveArray && rhsTypeKind != ECSqlTypeInfo::Kind::Null) ||
             (lhsTypeKind != ECSqlTypeInfo::Kind::Null && rhsTypeKind == ECSqlTypeInfo::Kind::PrimitiveArray)))
            {
            //for prim arrays only is null and arrays not supported in where expressions for now
            ctx.SetError (ECSqlStatus::InvalidECSql, "Type mismatch in expression '%s'. For primitive arrays only IS NULL and IS NOT NULL is supported.", ToECSql ().c_str ());
            return FinalizeParseStatus::Error;
            }
        }

    // For structs and structs array no operator supported for now
    //cannot assume both sides have same types as one can still represent the SQL NULL
    if (lhsTypeKind == ECSqlTypeInfo::Kind::Struct || lhsTypeKind == ECSqlTypeInfo::Kind::StructArray ||
        rhsTypeKind == ECSqlTypeInfo::Kind::Struct || rhsTypeKind == ECSqlTypeInfo::Kind::StructArray)
        {
        //structs and arrays not supported in where expressions for now
        ctx.SetError (ECSqlStatus::InvalidECSql, "Type mismatch in expression '%s'. Operator not supported with structs and struct arrays.", ToECSql ().c_str ());
        return FinalizeParseStatus::Error;
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String BinaryBooleanExp::ToECSql() const 
    {
    Utf8String ecsql;
    if (HasParentheses())
        ecsql.append("(");
    ecsql.append(GetLeftOperand()->ToECSql()).append(" ").append(ExpHelper::ToString(m_op)).append(" ");

    ComputedExp const* rhs = GetRightOperand();
    const bool rhsNeedsParens = m_op == BooleanSqlOperator::NotIn || m_op == BooleanSqlOperator::In;

    if (rhsNeedsParens)
        ecsql.append("(");

    ecsql.append(rhs->ToECSql());

    if (rhsNeedsParens)
        ecsql.append(")");

    if (HasParentheses())
        ecsql.append(")");
    return std::move(ecsql);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String BinaryBooleanExp::_ToString() const 
    {
    Utf8String str ("BinaryBoolean [Operator: ");
    str.append (ExpHelper::ToString (m_op)).append ("]");
    return str;
    }


//*************************** BooleanFactorExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BooleanFactorExp::BooleanFactorExp (unique_ptr<BooleanExp> operand, bool notOperator) 
    : BooleanExp(), m_notOperator(notOperator)
    {
    m_operandExpIndex = AddChild (std::move(operand));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String BooleanFactorExp::ToECSql() const 
    {
    Utf8String ecsql;
    if (m_notOperator)
        ecsql.append("NOT ");

    if (HasParentheses())
        ecsql.append("(");

    ecsql.append(GetOperand()->ToECSql());
    
    if (HasParentheses())
        ecsql.append(")");

    return ecsql;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String BooleanFactorExp::_ToString() const 
    {
    Utf8String str ("BooleanFactor [Operator: ");
    if (m_notOperator)
        str.append("NOT");
    else
        str.append("-");

    str.append ("]");
    return str;
    }

//*************************** UnaryPredicateExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+--------
UnaryPredicateExp::UnaryPredicateExp(unique_ptr<ValueExp> booleanValueExp)
    : BooleanExp()
    {
    m_booleanValueExpIndex = AddChild(std::move(booleanValueExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus UnaryPredicateExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    ValueExp const* valueExp = GetValueExp();
    ECSqlTypeInfo const& valueExpTypeInfo = valueExp->GetTypeInfo();
    if (valueExp->GetType () == Exp::Type::Parameter || (!valueExpTypeInfo.IsBoolean() && !valueExpTypeInfo.IsExactNumeric()))
        {
        ctx.SetError(ECSqlStatus::InvalidECSql, "Type mismatch in expression '%s'. Unary predicates can only have expressions of boolean or integral type and cannot be parametrized.", ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }

    return FinalizeParseStatus::Completed;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String UnaryPredicateExp::ToECSql() const
    {
    return GetValueExp()->ToECSql();
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
