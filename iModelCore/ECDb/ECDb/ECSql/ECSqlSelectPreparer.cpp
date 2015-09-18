/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlSelectPreparer.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlSelectPreparer.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

void ExtractPropertyRefs (ECSqlPrepareContext& ctx, Exp const* exp)
    {
    if (exp == nullptr)
        return;

    if (exp->GetType () == Exp::Type::PropertyName && !static_cast<PropertyNameExp const*>(exp)-> IsPropertyRef())
        {
        auto propertyName = static_cast<PropertyNameExp const*>(exp);
        ctx.GetSelectionOptionsR ().AddProperty (propertyName->GetPropertyMap ().GetPropertyAccessString());
        }

    for (auto child : exp->GetChildren ())
        {
        ExtractPropertyRefs (ctx, child);
        }
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlSelectPreparer::Prepare (ECSqlPrepareContext& ctx, SingleSelectStatementExp const& exp)
    {
    BeAssert (exp.IsComplete ());

    ctx.PushScope (exp);

    auto& sqlGenerator = ctx.GetSqlBuilderR ();
    sqlGenerator.Append ("SELECT ");
    // Append DISTINCT | ALL if any
    sqlGenerator.Append (exp.GetSelectionType ());

    // Append selection.
    auto status = ECSqlExpPreparer::PrepareSelectClauseExp (ctx, exp.GetSelection ());
    if (status != ECSqlStatus::Success)
        return status;

    ExtractPropertyRefs (ctx, &exp);

    sqlGenerator.AppendSpace ();
    // Append FROM
    status = ECSqlExpPreparer::PrepareFromExp (ctx, exp.GetFrom ());
    if (status != ECSqlStatus::Success)
        return status;

    // Append WHERE
    if (auto e = exp.GetOptWhere ())
        {
        sqlGenerator.AppendSpace ();
        status = ECSqlExpPreparer::PrepareWhereExp(sqlGenerator, ctx, e);
        if (status != ECSqlStatus::Success)
            return status;
        }
    // Append GROUP BY
    if (auto e = exp.GetOptGroupBy ())
        {
        sqlGenerator.AppendSpace ();
        status = ECSqlExpPreparer::PrepareGroupByExp (ctx, e);
        if (status != ECSqlStatus::Success)
            return status;
        }

    // Append HAVING
    if (auto e = exp.GetOptHaving ())
        {
        sqlGenerator.AppendSpace ();
        status = ECSqlExpPreparer::PrepareHavingExp (ctx, e);
        if (status != ECSqlStatus::Success)
            return status;
        }

    // Append ORDER BY
    if (auto e = exp.GetOptOrderBy())
        {
        sqlGenerator.AppendSpace();
        status = ECSqlExpPreparer::PrepareOrderByExp(ctx, e);
        if (status != ECSqlStatus::Success)
            return status;
        }

    // Append LIMIT
    if (auto e = exp.GetLimitOffset ())
        {
        sqlGenerator.AppendSpace ();
        status = ECSqlExpPreparer::PrepareLimitOffsetExp (ctx, e);
        if (status != ECSqlStatus::Success)
            return status;
        }

    ctx.PopScope ();
    return ECSqlStatus::Success;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlSelectPreparer::Prepare(ECSqlPrepareContext& ctx, SelectStatementExp const& exp)
    {
    auto st = Prepare (ctx, exp.GetCurrent ());
    if (st != ECSqlStatus::Success || !exp.IsCompound())
        return st;

    auto lhs = exp.GetSelection ();
    ctx.PushScope (exp);
    switch (exp.GetOP ())
        {
        case SelectStatementExp::Operator::Except:
            ctx.GetSqlBuilderR ().Append (" EXCEPT "); break;
        case SelectStatementExp::Operator::Intersect:
            ctx.GetSqlBuilderR ().Append (" INTERSECT "); break;
        case SelectStatementExp::Operator::Union:
            ctx.GetSqlBuilderR ().Append (" UNION "); break;
        default:
            BeAssert (false && "Error");
            return ECSqlStatus::ProgrammerError;    
        }

    if (exp.IsAll ())
        ctx.GetSqlBuilderR ().Append ("ALL ");

    st = Prepare (ctx, *exp.GetNext ());

    auto rhs = exp.GetNext ()->GetSelection ();
    if (rhs->GetChildrenCount () != lhs->GetChildrenCount ())
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Number of properties in all the select clauses of UNION/EXCEPT/INTERSECT must be same in number and type");
        return ECSqlStatus::UserError;
        }

    for (size_t i = 0; i < rhs->GetChildrenCount (); i++)
        {
        auto rhsP = static_cast <DerivedPropertyExp const*>(rhs->GetChildren ()[i]);
        auto lhsP = static_cast <DerivedPropertyExp const*>(lhs->GetChildren ()[i]);

        if (!rhsP->GetExpression ()->GetTypeInfo ().Equals (lhsP->GetExpression ()->GetTypeInfo ()))
            {
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type of expression %s in UNION/EXCEPT/INTERSECT is not same as respective expression %s", lhsP->ToECSql().c_str(), rhs->ToECSql().c_str());
            return ECSqlStatus::UserError;
            }
        }
    ctx.PopScope ();
    return st;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
