/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlSelectPreparer.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlSelectPreparer.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    01/2016
//+---------------+---------------+---------------+---------------+---------------+--------
void ExtractPropertyRefs(ECSqlPrepareContext& ctx, Exp const* exp)
    {
    if (exp == nullptr)
        return;

    if (exp->GetType() == Exp::Type::PropertyName && !static_cast<PropertyNameExp const*>(exp)->IsPropertyRef())
        {
        auto propertyName = static_cast<PropertyNameExp const*>(exp);
        ctx.GetSelectionOptionsR().AddProperty(propertyName->GetPropertyMap().GetPropertyAccessString());
        }

    for (auto child : exp->GetChildren())
        {
        ExtractPropertyRefs(ctx, child);
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlSelectPreparer::Prepare(ECSqlPrepareContext& ctx, SingleSelectStatementExp const& exp)
    {
    BeAssert(exp.IsComplete());

    ctx.PushScope(exp, exp.GetOptions());

    NativeSqlBuilder& sqlGenerator = ctx.GetSqlBuilderR();
    sqlGenerator.Append("SELECT ");
    
    if (exp.GetSelectionType() != SqlSetQuantifier::NotSpecified)
        sqlGenerator.Append(exp.GetSelectionType()).AppendSpace();

    // Append selection.
    ECSqlStatus status = ECSqlExpPreparer::PrepareSelectClauseExp(ctx, exp.GetSelection());
    if (!status.IsSuccess())
        return status;

    ExtractPropertyRefs(ctx, &exp);

    sqlGenerator.AppendSpace();
    // Append FROM
    status = ECSqlExpPreparer::PrepareFromExp(ctx, exp.GetFrom());
    if (!status.IsSuccess())
        return status;

    // Append WHERE
    if (WhereExp const* e = exp.GetWhere())
        {
        sqlGenerator.AppendSpace();
        status = ECSqlExpPreparer::PrepareWhereExp(sqlGenerator, ctx, e);
        if (!status.IsSuccess())
            return status;
        }
    // Append GROUP BY
    if (GroupByExp const* e = exp.GetGroupBy())
        {
        sqlGenerator.AppendSpace();
        status = ECSqlExpPreparer::PrepareGroupByExp(ctx, e);
        if (!status.IsSuccess())
            return status;
        }

    // Append HAVING
    if (HavingExp const* e = exp.GetHaving())
        {
        sqlGenerator.AppendSpace();
        status = ECSqlExpPreparer::PrepareHavingExp(ctx, e);
        if (!status.IsSuccess())
            return status;
        }

    // Append ORDER BY
    if (OrderByExp const* e = exp.GetOrderBy())
        {
        sqlGenerator.AppendSpace();
        status = ECSqlExpPreparer::PrepareOrderByExp(ctx, e);
        if (!status.IsSuccess())
            return status;
        }

    // Append LIMIT
    if (LimitOffsetExp const* e = exp.GetLimitOffset())
        {
        sqlGenerator.AppendSpace();
        status = ECSqlExpPreparer::PrepareLimitOffsetExp(ctx, e);
        if (!status.IsSuccess())
            return status;
        }

    ctx.PopScope();
    return ECSqlStatus::Success;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlSelectPreparer::Prepare(ECSqlPrepareContext& ctx, SelectStatementExp const& exp)
    {
    ECSqlStatus st = Prepare(ctx, exp.GetCurrent());
    if (st != ECSqlStatus::Success || !exp.IsCompound())
        return st;

    SelectClauseExp const* lhs = exp.GetSelection();
    ctx.PushScope(exp);
    switch (exp.GetOperator())
        {
            case SelectStatementExp::Operator::Except:
                ctx.GetSqlBuilderR().Append(" EXCEPT "); break;
            case SelectStatementExp::Operator::Intersect:
                ctx.GetSqlBuilderR().Append(" INTERSECT "); break;
            case SelectStatementExp::Operator::Union:
                ctx.GetSqlBuilderR().Append(" UNION "); break;
            default:
                BeAssert(false && "Error");
                return ECSqlStatus::Error;
        }

    if (exp.IsAll())
        ctx.GetSqlBuilderR().Append("ALL ");

    st = Prepare(ctx, *exp.GetNext());

    SelectClauseExp const* rhs = exp.GetNext()->GetSelection();
    if (rhs->GetChildrenCount() != lhs->GetChildrenCount())
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Number of properties in all the select clauses of UNION/EXCEPT/INTERSECT must be same in number and type");
        return ECSqlStatus::Error;
        }

    for (size_t i = 0; i < rhs->GetChildrenCount(); i++)
        {
        DerivedPropertyExp const* rhsDerivedPropExp = rhs->GetChildren().Get<DerivedPropertyExp>(i);
        DerivedPropertyExp const* lhsDerivedPropExp = lhs->GetChildren().Get<DerivedPropertyExp>(i);

        if (!rhsDerivedPropExp->GetExpression()->GetTypeInfo().CanCompare(lhsDerivedPropExp->GetExpression()->GetTypeInfo()))
            {
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type of expression %s in LHS of UNION/EXCEPT/INTERSECT is not same as respective expression %s in RHS.", lhsDerivedPropExp->ToECSql().c_str(), rhsDerivedPropExp->ToECSql().c_str());
            return ECSqlStatus::Error;
            }
        }

    ctx.PopScope();
    return st;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
