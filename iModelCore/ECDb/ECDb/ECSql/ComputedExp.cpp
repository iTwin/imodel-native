/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ComputedExp.h"
#include "ValueExp.h"
#include "ExpHelper.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//NOTE: The code in this file is sorted by the class names. This should make maintenance easier as there are too many types in this single file


//*************************** BooleanBinaryExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BinaryBooleanExp::BinaryBooleanExp(std::unique_ptr<ComputedExp> left, BooleanSqlOperator op, std::unique_ptr<ComputedExp> right)
    : BooleanExp(Type::BinaryBoolean), m_op(op)
    {
    m_leftOperandExpIndex = AddChild(std::move(left));
    m_rightOperandExpIndex = AddChild(std::move(right));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
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
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Only one operand of the expression '%s' can be an expression list.", ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }

            expWithVaryingTypeInfo = rhs;
            expWithRegularTypeInfo = lhs;
            }

        _Analysis_assume_(expWithRegularTypeInfo != nullptr);
        _Analysis_assume_(expWithVaryingTypeInfo != nullptr);

        if (expWithVaryingTypeInfo != nullptr)
            {
            for (Exp const* child : expWithVaryingTypeInfo->GetChildren())
                {
                ComputedExp const& childComputedExp = child->GetAs<ComputedExp>();

                Exp::FinalizeParseStatus stat = CanCompareTypes(ctx, *expWithRegularTypeInfo, childComputedExp);
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
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool BinaryBooleanExp::_TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const
    {
    ComputedExp const* lhs = GetLeftOperand();
    ComputedExp const* rhs = GetRightOperand();
    ComputedExp const* targetExp = nullptr;
    if (IsInHierarchy(*lhs, parameterExp))
        targetExp = rhs;
    else if (IsInHierarchy(*rhs, parameterExp))
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus BinaryBooleanExp::CanCompareTypes(ECSqlParseContext& ctx, ComputedExp const& lhs, ComputedExp const& rhs) const
    {
    //The parser imposes as little comparability restrictions on top of the ECSQL grammar as possible.
    //It is the aim of the parser to control comparability either by the grammar or by SQLite

    ECSqlTypeInfo const& lhsTypeInfo = lhs.GetTypeInfo();
    ECSqlTypeInfo const& rhsTypeInfo = rhs.GetTypeInfo();
    const ECSqlTypeInfo::Kind lhsTypeKind = lhsTypeInfo.GetKind();
    const ECSqlTypeInfo::Kind rhsTypeKind = rhsTypeInfo.GetKind();

    //first check whether types on both sides match generally for comparisons
    Utf8String canCompareErrorMessage;
    //parameter types are determined later, so exclude them from type checking
    if (!lhs.Contains(Exp::Type::Parameter) && !rhs.Contains(Exp::Type::Parameter) && !lhsTypeInfo.CanCompare(rhsTypeInfo, &canCompareErrorMessage))
        {
        if (canCompareErrorMessage.empty())
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Type mismatch in expression '%s'.", ToECSql().c_str());
        else
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Type mismatch in expression '%s': %s", ToECSql().c_str(), canCompareErrorMessage.c_str());

        return FinalizeParseStatus::Error;
        }

    const bool lhsIsStructWithStructArray = (lhsTypeKind == ECSqlTypeInfo::Kind::Struct && ContainsStructArrayProperty(lhsTypeInfo.GetStructType()));
    const bool rhsIsStructWithStructArray = (rhsTypeKind == ECSqlTypeInfo::Kind::Struct && ContainsStructArrayProperty(rhsTypeInfo.GetStructType()));

    //Limit operators for non-primitive types
    //cannot assume both sides have same types as one can still represent the SQL NULL or a parameter
    if (lhsTypeInfo.IsPoint() || rhsTypeInfo.IsPoint() ||
        lhsTypeInfo.IsGeometry() || rhsTypeInfo.IsGeometry() ||
        (lhsTypeKind == ECSqlTypeInfo::Kind::Struct && !lhsIsStructWithStructArray) ||
        (rhsTypeKind == ECSqlTypeInfo::Kind::Struct && !rhsIsStructWithStructArray) ||
        lhsTypeKind == ECSqlTypeInfo::Kind::PrimitiveArray ||
        rhsTypeKind == ECSqlTypeInfo::Kind::PrimitiveArray ||
        lhsTypeKind == ECSqlTypeInfo::Kind::Navigation ||
        rhsTypeKind == ECSqlTypeInfo::Kind::Navigation)
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
                    ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Type mismatch in expression '%s'. Operator not supported with point, geometry, navigation properties, struct or primitive array operands.", ToECSql().c_str());
                    return FinalizeParseStatus::Error;
            }
        }

    // For structs and structs array no operator supported for now
    //cannot assume both sides have same types as one can still represent the SQL NULL
    if (lhsTypeKind == ECSqlTypeInfo::Kind::StructArray || lhsIsStructWithStructArray ||
        rhsTypeKind == ECSqlTypeInfo::Kind::StructArray || rhsIsStructWithStructArray)
        {
        //structs and arrays not supported in where expressions for now
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Type mismatch in expression '%s'. Operator not supported with struct arrays or structs that contain struct arrays.", ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool BinaryBooleanExp::ContainsStructArrayProperty(ECClassCR ecclass)
    {
    for (ECPropertyCP prop : ecclass.GetProperties(true))
        {
        StructArrayECPropertyCP arrayProp = prop->GetAsStructArrayProperty();
        if (arrayProp != nullptr)
            return true;

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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void BinaryBooleanExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (HasParentheses())
        ctx.AppendToECSql("(");

    ctx.AppendToECSql(*GetLeftOperand()).AppendToECSql(" ").AppendToECSql(ExpHelper::ToSql(m_op)).AppendToECSql(" ").AppendToECSql(*GetRightOperand());

    if (HasParentheses())
        ctx.AppendToECSql(")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String BinaryBooleanExp::_ToString() const
    {
    Utf8String str("BinaryBoolean [Operator: ");
    str.append(ExpHelper::ToSql(m_op)).append("]");
    return str;
    }

//*************************** BooleanFactorExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BooleanFactorExp::BooleanFactorExp(std::unique_ptr<BooleanExp> operand, bool notOperator)
    : BooleanExp(Type::BooleanFactor), m_notOperator(notOperator)
    {
    m_operandExpIndex = AddChild(std::move(operand));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void BooleanFactorExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (HasParentheses())
        ctx.AppendToECSql("(");

    if (m_notOperator)
        ctx.AppendToECSql("NOT ");

    ctx.AppendToECSql(*GetOperand());

    if (HasParentheses())
        ctx.AppendToECSql(")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String BooleanFactorExp::_ToString() const
    {
    Utf8String str("BooleanFactor [Operator: ");
    if (m_notOperator)
        str.append("NOT");
    else
        str.append("-");

    str.append("]");
    return str;
    }

//*************************** UnaryPredicateExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
UnaryPredicateExp::UnaryPredicateExp(std::unique_ptr<ValueExp> booleanValueExp) : BooleanExp(Type::UnaryPredicate)
    {
    m_booleanValueExpIndex = AddChild(std::move(booleanValueExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus UnaryPredicateExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    ValueExp const* valueExp = GetValueExp();
    if (valueExp->IsParameterExp())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Type mismatch in expression '%s'. Unary predicates cannot be parametrized.", ToECSql().c_str());
        return FinalizeParseStatus::Error;
        }


    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void UnaryPredicateExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (HasParentheses())
        ctx.AppendToECSql("(");

    ctx.AppendToECSql(*GetValueExp());

    if (HasParentheses())
        ctx.AppendToECSql(")");
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
