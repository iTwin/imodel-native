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

    if (exp->GetType () == Exp::Type::PropertyName)
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
ECSqlStatus ECSqlSelectPreparer::Prepare (ECSqlPrepareContext& ctx, SelectStatementExp const& exp)
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
ECSqlStatus ECSqlSelectPreparer::Prepare(ECSqlPrepareContext& ctx, UnionStatementExp const& exp)
    {
    BeAssert(exp.IsComplete() && exp.GetLhs() != nullptr && exp.GetRhs() != nullptr);

    ExpCP lhsExp = exp.GetLhs();
    ECSqlStatus stat;
    if (lhsExp->IsSelectStatement())
        stat = Prepare(ctx, *static_cast<SelectStatementExp const*> (lhsExp));
    else
        {
        BeAssert(lhsExp->GetType() == Exp::Type::Union);
        stat = Prepare(ctx, *static_cast<UnionStatementExp const*> (lhsExp));
        }

    if (stat != ECSqlStatus::Success)
        return stat;

    NativeSqlBuilder& sqlBuilder = ctx.GetSqlBuilderR();
    sqlBuilder.Append(" UNION ");
    if (exp.IsUnionAll())
        sqlBuilder.Append(" ALL ");

    return Prepare(ctx, *exp.GetRhs());
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
