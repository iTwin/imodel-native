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
// @bsimethod                                    Krischan.Eberle       05/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ComputedExp::_ToECSql() const
    {
    Utf8String ecsql;
    if (HasParentheses())
        ecsql.append("(");

    _DoToECSql(ecsql);

    if (HasParentheses())
        ecsql.append(")");

    return std::move(ecsql);
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
Exp::FinalizeParseStatus BinaryBooleanExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    if (mode == Exp::FinalizeParseMode::AfterFinalizingChildren)
        {
        auto lhs = GetLeftOperand();
        auto rhs = GetRightOperand();

        ComputedExp const* expWithVaryingTypeInfo = nullptr;
        ComputedExp const* expWithRegularTypeInfo = nullptr;
        if (lhs->GetTypeInfo().GetKind() == ECSqlTypeInfo::Kind::Varies)
            {
            expWithVaryingTypeInfo = lhs;
            expWithRegularTypeInfo = rhs;
            }

        if (rhs->GetTypeInfo().GetKind() == ECSqlTypeInfo::Kind::Varies)
            {
            //only one side can be of Kind::Varies. If lhs is Varies, expWithVaryingTypeInfo was already set and no longer is null
            if (expWithVaryingTypeInfo != nullptr)
                {
                ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Only one operand of the expression '%s' can be an expression list.", ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }

            expWithVaryingTypeInfo = rhs;
            expWithRegularTypeInfo = lhs;
            }

        if (expWithVaryingTypeInfo != nullptr)
            {
            for (auto child : expWithVaryingTypeInfo->GetChildren())
                {
                BeAssert(dynamic_cast<ComputedExp const*> (child) != nullptr);
                auto childComputedExp = static_cast<ComputedExp const*> (child);

                auto stat = CanCompareTypes(ctx, *expWithRegularTypeInfo, *childComputedExp);
                if (stat != Exp::FinalizeParseStatus::Completed)
                    return stat;
                }

            return Exp::FinalizeParseStatus::Completed;
            }
        else
            return CanCompareTypes(ctx, *lhs, *rhs);
        }

    BeAssert(false);
    return FinalizeParseStatus::Error;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       06/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool IsInHierarchy(Exp const& rootExp, Exp const& candidateExp)
    {
    if (&rootExp == &candidateExp)
        return true;

    for (Exp const* childExp : rootExp.GetChildren())
        {
        if (IsInHierarchy(*childExp, candidateExp))
            return true;
        }

    return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       06/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool BinaryBooleanExp::_TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const
    {
    ComputedExp const* lhs = GetLeftOperand();
    ComputedExp const* rhs = GetRightOperand();
    ComputedExp const* targetExp = nullptr;
    if (IsInHierarchy(*lhs,parameterExp))
        targetExp = rhs;
    else if (IsInHierarchy(*rhs,parameterExp))
        targetExp = lhs;
    else
        {
        BeAssert(false && "Expected to find parameter exp in either LHS or RHS exp hierarchy of BinaryBooleanExp");
        return false;
        }

    parameterExp.SetTargetExpInfo(*targetExp);
    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus BinaryBooleanExp::CanCompareTypes(ECSqlParseContext& ctx, ComputedExp const& lhs, ComputedExp const& rhs) const
    {
    //parameter types are determined later, so exclude them from type checking
    const bool lhsIsParameter = lhs.IsParameterExp();
    const bool rhsIsParameter = rhs.IsParameterExp();
    ECSqlTypeInfo const& lhsTypeInfo = lhs.GetTypeInfo();
    ECSqlTypeInfo const& rhsTypeInfo = rhs.GetTypeInfo();
    const ECSqlTypeInfo::Kind lhsTypeKind = lhsTypeInfo.GetKind();
    const ECSqlTypeInfo::Kind rhsTypeKind = rhsTypeInfo.GetKind();
    const bool lhsIsNull = lhsTypeKind == ECSqlTypeInfo::Kind::Null;
    const bool rhsIsNull = rhsTypeKind == ECSqlTypeInfo::Kind::Null;

    if (lhsIsNull || rhsIsNull)
        {
        if (m_op != BooleanSqlOperator::Is && m_op != BooleanSqlOperator::IsNot &&
            m_op != BooleanSqlOperator::EqualTo && m_op != BooleanSqlOperator::NotEqualTo)
            {
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch in expression '%s'. NULL can only be used with operators IS, IS NOT, = or <>.", ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }
        }

    if (m_op == BooleanSqlOperator::Is || m_op == BooleanSqlOperator::IsNot)
        {
        if (!lhsIsNull && !rhsIsNull)
            {
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch in expression '%s'. Operators IS or IS NOT can only be used with NULL.", ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }
        }

    //first check whether types on both sides match generally for comparisons
    Utf8String canCompareErrorMessage;
    if (!lhsIsParameter && !rhsIsParameter && !lhsTypeInfo.Matches(rhsTypeInfo, &canCompareErrorMessage))
        {
        if (canCompareErrorMessage.empty())
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch in expression '%s'.", ToECSql().c_str());
        else
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch in expression '%s': %s", ToECSql().c_str(), canCompareErrorMessage.c_str());

        return FinalizeParseStatus::Error;
        }

    if (m_op == BooleanSqlOperator::Like || m_op == BooleanSqlOperator::NotLike)
        {
        if ((!lhsIsParameter && (!lhsTypeInfo.IsPrimitive() || lhsTypeInfo.GetPrimitiveType() != PRIMITIVETYPE_String)) ||
            (!rhsIsParameter && (!rhsTypeInfo.IsPrimitive() || rhsTypeInfo.GetPrimitiveType() != PRIMITIVETYPE_String)))
            {
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch in expression '%s'. LIKE operator only supported with string operands.", ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }
        }

    const bool lhsIsStructWithStructArray = (lhsTypeKind == ECSqlTypeInfo::Kind::Struct && ContainsStructArrayProperty(lhsTypeInfo.GetStructType()));
    const bool rhsIsStructWithStructArray = (rhsTypeKind == ECSqlTypeInfo::Kind::Struct && ContainsStructArrayProperty(rhsTypeInfo.GetStructType()));

    //Limit operators for point expressions
    //cannot assume both sides have same types as one can still represent the SQL NULL or a parameter
    if (lhsTypeInfo.IsPoint() || rhsTypeInfo.IsPoint() ||
        lhsTypeInfo.IsGeometry() || rhsTypeInfo.IsGeometry() ||
        (lhsTypeKind == ECSqlTypeInfo::Kind::Struct && !lhsIsStructWithStructArray) ||
        (rhsTypeKind == ECSqlTypeInfo::Kind::Struct && !rhsIsStructWithStructArray) ||
        lhsTypeKind == ECSqlTypeInfo::Kind::PrimitiveArray ||
        rhsTypeKind == ECSqlTypeInfo::Kind::PrimitiveArray)
        {
        switch (m_op)
            {
                case BooleanSqlOperator::EqualTo:
                case BooleanSqlOperator::NotEqualTo:
                case BooleanSqlOperator::In:
                case BooleanSqlOperator::NotIn:
                case BooleanSqlOperator::Is:
                case BooleanSqlOperator::IsNot:
                    return FinalizeParseStatus::Completed;

                default:
                    ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch in expression '%s'. Operator not supported with point, geometry, struct or primitive array operands.", ToECSql().c_str());
                    return FinalizeParseStatus::Error;
            }
        }

    // For structs and structs array no operator supported for now
    //cannot assume both sides have same types as one can still represent the SQL NULL
    if (lhsTypeKind == ECSqlTypeInfo::Kind::StructArray || lhsIsStructWithStructArray ||
        rhsTypeKind == ECSqlTypeInfo::Kind::StructArray || rhsIsStructWithStructArray)
        {
        //structs and arrays not supported in where expressions for now
        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch in expression '%s'. Operator not supported with struct arrays or structs that contain struct array´s.", ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       10/2015
//+---------------+---------------+---------------+---------------+---------------+--------
bool BinaryBooleanExp::ContainsStructArrayProperty(ECClassCR ecclass)
    {
    for (ECPropertyCP prop : ecclass.GetProperties())
        {
        ArrayECPropertyCP arrayProp = prop->GetAsArrayProperty();
        if (arrayProp != nullptr)
            { 
            if (ARRAYKIND_Struct == arrayProp->GetKind())
                return true;

            continue;
            }
       
        StructECPropertyCP structProp = prop->GetAsStructProperty();
        if (structProp != nullptr)
            {
            if (ContainsStructArrayProperty(structProp->GetType()))
                return true;
            }
        }

    return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void BinaryBooleanExp::_DoToECSql(Utf8StringR ecsql) const
    {
    ecsql.append(GetLeftOperand()->ToECSql()).append(" ").append(ExpHelper::ToString(m_op)).append(" ");

    ComputedExp const* rhs = GetRightOperand();
    const bool rhsNeedsParens = m_op == BooleanSqlOperator::NotIn || m_op == BooleanSqlOperator::In;

    if (rhsNeedsParens)
        ecsql.append("(");

    ecsql.append(rhs->ToECSql());

    if (rhsNeedsParens)
        ecsql.append(")");
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
void BooleanFactorExp::_DoToECSql(Utf8StringR ecsql) const
    {
    if (m_notOperator)
        ecsql.append("NOT ");

    ecsql.append(GetOperand()->ToECSql());
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
    if (valueExp->IsParameterExp () || (!valueExpTypeInfo.IsBoolean() && !valueExpTypeInfo.IsExactNumeric()))
        {
        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch in expression '%s'. Unary predicates can only have expressions of boolean or integral type and cannot be parametrized.", ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }

    return FinalizeParseStatus::Completed;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+--------
void UnaryPredicateExp::_DoToECSql(Utf8StringR ecsql) const
    {
    ecsql.append (GetValueExp()->ToECSql());
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
