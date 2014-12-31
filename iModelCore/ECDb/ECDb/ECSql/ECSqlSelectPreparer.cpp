/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlSelectPreparer.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlSelectPreparer.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

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
    //1. Append DISTINCT | ALL if any
    sqlGenerator.Append (exp.GetSelectionType ());

    //2. Append selection.
    auto status = ECSqlExpPreparer::PrepareSelectClauseExp (ctx, exp.GetSelection ());
    if (status != ECSqlStatus::Success)
        return status;

    sqlGenerator.AppendSpace ();
    //3. Append FROM
    status = ECSqlExpPreparer::PrepareFromExp (ctx, exp.GetFrom ());
    if (status != ECSqlStatus::Success)
        return status;

    //4. Append WHERE
    if (auto e = exp.GetOptWhere ())
        {
        sqlGenerator.AppendSpace ();
        status = ECSqlExpPreparer::PrepareWhereExp (sqlGenerator, ctx, e);
        if (status != ECSqlStatus::Success)
            return status;
        }
    //5. Append GROUP BY
    if (auto e = exp.GetOptGroupBy ())
        {
        sqlGenerator.AppendSpace ();
        status = ECSqlExpPreparer::PrepareGroupByExp (ctx, e);
        if (status != ECSqlStatus::Success)
            return status;
        }

    //6. Append ORDER BY
    if (auto e = exp.GetOptOrderBy ())
        {
        sqlGenerator.AppendSpace ();
        status = ECSqlExpPreparer::PrepareOrderByExp (ctx, e);
        if (status != ECSqlStatus::Success)
            return status;
        }

    //7. Append HAVING
    if (auto e = exp.GetOptHaving ())
        {
        sqlGenerator.AppendSpace ();
        status = ECSqlExpPreparer::PrepareHavingExp (ctx, e);
        if (status != ECSqlStatus::Success)
            return status;
        }

    //8. Append LIMIT
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

END_BENTLEY_SQLITE_EC_NAMESPACE
