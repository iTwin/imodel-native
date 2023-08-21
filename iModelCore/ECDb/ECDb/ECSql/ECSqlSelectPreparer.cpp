/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlSelectPreparer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlSelectPreparer::Prepare(ECSqlPrepareContext& ctx, SelectStatementExp const& exp)
    {
    return Prepare(ctx, exp, nullptr);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlSelectPreparer::Prepare(ECSqlPrepareContext& ctx, CommonTableExp const& exp) {
    auto &builder = ctx.GetSqlBuilder();
    builder.Append("WITH ");
    if (exp.Recursive()) {
        builder.Append("RECURSIVE ");
    }
    ctx.PushScope(exp);
    auto blocks = exp.GetCteList();
    for (size_t i = 0; i < blocks.size(); ++i){
        if (i>0) {
            builder.AppendComma();
        }
        // render block
        auto blockExp = blocks[i];
        builder.Append(blockExp->GetName());
        builder.AppendParenLeft();
        auto &cols = blockExp->GetColumns();
        for(size_t j =0; j < cols.size(); ++j){
            if (j>0) {
                builder.AppendComma();
            }
            builder.Append(cols[j]);
        }
        builder.AppendParenRight();
        builder.Append(" AS ");
        builder.AppendParenLeft();
        auto rc = Prepare(ctx, *blockExp->GetQuery(), nullptr);
        if (rc != ECSqlStatus::Success)
            return rc;
        builder.AppendParenRight();
    }
    ctx.PopScope();
    // make it a bit readable
    builder.Append("\n");
    return Prepare(ctx, *exp.GetQuery(), nullptr);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlSelectPreparer::PreparePartial(NativeSqlBuilder& nativeSqlSnippet, ECSqlPrepareContext& ctx, SelectStatementExp const& exp)
    {
    NativeSqlBuilder::ListOfLists selectClauseSqlSnippetList;
    SingleSelectStatementExp const& firstStmt = exp.GetFirstStatement();
    ctx.PushScope(firstStmt, firstStmt.GetOptions());
    ECSqlStatus status;
    if (firstStmt.IsRowConstructor())
        {
        nativeSqlSnippet.Append("SELECT").AppendSpace();
        status = PrepareSelectClauseExp(selectClauseSqlSnippetList, ctx, *firstStmt.GetSelection(), nullptr);
        if (!status.IsSuccess())
            return status;

        bool isFirstItem = true;
        for (NativeSqlBuilder::List const& list : selectClauseSqlSnippetList)
            {
            if (!isFirstItem)
                nativeSqlSnippet.AppendComma();

            nativeSqlSnippet.Append(list);
            isFirstItem = false;
            }
        }
    else
        {
        nativeSqlSnippet.Append("SELECT").AppendSpace();

        if (firstStmt.GetSelectionType() != SqlSetQuantifier::NotSpecified)
            nativeSqlSnippet.Append(ExpHelper::ToSql(firstStmt.GetSelectionType())).AppendSpace();

        // Append selection.
        status = PrepareSelectClauseExp(selectClauseSqlSnippetList, ctx, *firstStmt.GetSelection(), nullptr);
        if (!status.IsSuccess())
            return status;

        bool isFirstItem = true;
        for (NativeSqlBuilder::List const& list : selectClauseSqlSnippetList)
            {
            if (!isFirstItem)
                nativeSqlSnippet.AppendComma();

            nativeSqlSnippet.Append(list);
            isFirstItem = false;
            }
        }
    ctx.PopScope();
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlSelectPreparer::Prepare(ECSqlPrepareContext& ctx, SelectStatementExp const& exp, std::vector<size_t> const* referenceSelectClauseSqlSnippetCounts)
    {
    NativeSqlBuilder::ListOfLists selectClauseSqlSnippetList;
    ECSqlStatus st = Prepare(ctx, selectClauseSqlSnippetList, exp.GetFirstStatement(), referenceSelectClauseSqlSnippetCounts);
    if (st != ECSqlStatus::Success || !exp.IsCompound())
        return st;

    ctx.PushScope(exp);
    switch (exp.GetOperator())
        {
            case SelectStatementExp::CompoundOperator::Except:
                ctx.GetSqlBuilder().Append(" EXCEPT "); break;
            case SelectStatementExp::CompoundOperator::Intersect:
                ctx.GetSqlBuilder().Append(" INTERSECT "); break;
            case SelectStatementExp::CompoundOperator::Union:
                ctx.GetSqlBuilder().Append(" UNION "); break;
            default:
                BeAssert(false && "Error");
                return ECSqlStatus::InvalidECSql;
        }

    if (exp.IsAll())
        ctx.GetSqlBuilder().Append("ALL ");

    SelectClauseExp const* lhs = exp.GetSelection();
    SelectStatementExp const& rhsStatement = *exp.GetRhsStatement();
    SelectClauseExp const* rhs = rhsStatement.GetSelection();
    if (SUCCESS != ValidateSelectClauseItems(ctx, *lhs, *rhs))
        return ECSqlStatus::InvalidECSql;

    //generate list of sql snippet counts per select clause item as this is needed to prepare
    //the RHS select clause (in case of NULL literals)
    std::vector<size_t> selectClauseSqlSnippetCounts;
    for (NativeSqlBuilder::List const& selectClauseItemSqlSnippets : selectClauseSqlSnippetList)
        {
        selectClauseSqlSnippetCounts.push_back(selectClauseItemSqlSnippets.size());
        }

    st = Prepare(ctx, rhsStatement, &selectClauseSqlSnippetCounts);

    ctx.PopScope();
    return st;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlSelectPreparer::Prepare(ECSqlPrepareContext& ctx, NativeSqlBuilder::ListOfLists& selectClauseSqlSnippetList, SingleSelectStatementExp const& exp, std::vector<size_t> const* referenceSelectClauseSqlSnippetCounts)
    {
    BeAssert(exp.IsComplete());

    ctx.PushScope(exp, exp.GetOptions());
    NativeSqlBuilder& sqlGenerator = ctx.GetSqlBuilder();
    ECSqlStatus status;
    if (exp.IsRowConstructor())
        {
        sqlGenerator.Append("SELECT ");
        status = PrepareSelectClauseExp(selectClauseSqlSnippetList, ctx, *exp.GetSelection(), referenceSelectClauseSqlSnippetCounts);
        if (!status.IsSuccess())
            return status;

        bool isFirstItem = true;
        for (NativeSqlBuilder::List const& list : selectClauseSqlSnippetList)
            {
            if (!isFirstItem)
                sqlGenerator.AppendComma();

            sqlGenerator.Append(list);
            isFirstItem = false;
            }

        }
    else
        {
        sqlGenerator.Append("SELECT ");

        if (exp.GetSelectionType() != SqlSetQuantifier::NotSpecified)
            sqlGenerator.Append(ExpHelper::ToSql(exp.GetSelectionType())).AppendSpace();

        // Append selection.
        status = PrepareSelectClauseExp(selectClauseSqlSnippetList, ctx, *exp.GetSelection(), referenceSelectClauseSqlSnippetCounts);
        if (!status.IsSuccess())
            return status;

        bool isFirstItem = true;
        for (NativeSqlBuilder::List const& list : selectClauseSqlSnippetList)
            {
            if (!isFirstItem)
                sqlGenerator.AppendComma();

            sqlGenerator.Append(list);
            isFirstItem = false;
            }

        ExtractPropertyRefs(ctx, &exp);

        sqlGenerator.AppendSpace();
        // Append FROM
        status = ECSqlExpPreparer::PrepareFromExp(ctx, *exp.GetFrom());
        if (!status.IsSuccess())
            return status;

        // Append WHERE
        if (WhereExp const* e = exp.GetWhere())
            {
            sqlGenerator.AppendSpace();
            status = ECSqlExpPreparer::PrepareWhereExp(sqlGenerator, ctx, *e);
            if (!status.IsSuccess())
                return status;
            }
        // Append GROUP BY
        if (GroupByExp const* e = exp.GetGroupBy())
            {
            sqlGenerator.AppendSpace();
            status = ECSqlExpPreparer::PrepareGroupByExp(ctx, *e);
            if (!status.IsSuccess())
                return status;
            }

        // Append HAVING
        if (HavingExp const* e = exp.GetHaving())
            {
            sqlGenerator.AppendSpace();
            status = ECSqlExpPreparer::PrepareHavingExp(ctx, *e);
            if (!status.IsSuccess())
                return status;
            }

        // Append ORDER BY
        if (OrderByExp const* e = exp.GetOrderBy())
            {
            sqlGenerator.AppendSpace();
            status = ECSqlExpPreparer::PrepareOrderByExp(ctx, *e);
            if (!status.IsSuccess())
                return status;
            }

        // Append LIMIT
        if (LimitOffsetExp const* e = exp.GetLimitOffset())
            {
            sqlGenerator.AppendSpace();
            status = ECSqlExpPreparer::PrepareLimitOffsetExp(ctx, *e);
            if (!status.IsSuccess())
                return status;
            }
        }

    ctx.PopScope();
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlSelectPreparer::PrepareSelectClauseExp(NativeSqlBuilder::ListOfLists& selectClauseSnippetsList, ECSqlPrepareContext& ctx, SelectClauseExp const& selectClauseExp, std::vector<size_t> const* referenceSelectClauseSqlSnippetCounts)
    {
    Exp::Collection const& selectClauseItemExpList = selectClauseExp.GetChildren();
    if (referenceSelectClauseSqlSnippetCounts != nullptr && selectClauseItemExpList.size() != referenceSelectClauseSqlSnippetCounts->size())
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }

    for (size_t i = 0; i < selectClauseItemExpList.size(); i++)
        {
        NativeSqlBuilder::List selectClauseItemNativeSqlSnippets;
        DerivedPropertyExp const* selectClauseItemExp = selectClauseItemExpList.Get<DerivedPropertyExp>(i);
        //if no reference select clause was passed, use 1 as default count (i.e. in case of NULL literals one column is assumed)
        const size_t referenceSelectClauseItemSqlSnippetCount = referenceSelectClauseSqlSnippetCounts == nullptr ? 1 : referenceSelectClauseSqlSnippetCounts->at(i);
        ECSqlStatus status = PrepareDerivedPropertyExp(selectClauseItemNativeSqlSnippets, ctx, *selectClauseItemExp, referenceSelectClauseItemSqlSnippetCount);
        if (!status.IsSuccess())
            return status;

        selectClauseSnippetsList.push_back(selectClauseItemNativeSqlSnippets);
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlSelectPreparer::PrepareDerivedPropertyExp(NativeSqlBuilder::List& nativeSqlSnippets, ECSqlPrepareContext& ctx, DerivedPropertyExp const& exp, size_t referenceSqliteSnippetCount)
    {
    ValueExp const* innerExp = exp.GetExpression();
    if (innerExp == nullptr)
        {
        BeAssert(false && "DerivedPropertyExp::GetExpression is not expected to return null during preparation.");
        return ECSqlStatus::Error;
        }

    const int startColumnIndex = ctx.GetCurrentScope().GetNativeSqlSelectClauseColumnCount();

    size_t snippetCountBefore = nativeSqlSnippets.size();
    if (!innerExp->IsParameterExp() && innerExp->GetTypeInfo().IsNull())
        { 
        ECSqlStatus status = ECSqlExpPreparer::PrepareNullExp(nativeSqlSnippets, ctx, *innerExp, referenceSqliteSnippetCount);
        if (!status.IsSuccess())
            return status;
        }
    else
        {
        ECSqlStatus status = ECSqlExpPreparer::PrepareValueExp(nativeSqlSnippets, ctx, *innerExp);
        if (!status.IsSuccess())
            return status;
        }

    std::vector<Utf8String> & resultSet = const_cast<DerivedPropertyExp&>(exp).SqlResultSetR();
    Utf8String alias = exp.GetColumnAlias();
    if (alias.empty() || exp.FindParent(Exp::Type::Subquery) != nullptr)
        alias = exp.GetNestedAlias();
    if (innerExp->GetType() == Exp::Type::CommonTablePropertyName) {
            resultSet.push_back(alias);
    } else if (!alias.empty())
        {
        if (nativeSqlSnippets.size() == 1)
            {
            nativeSqlSnippets.front().AppendSpace().AppendEscaped(alias.c_str());
            resultSet.push_back(alias);
            }
        else
            {
            int idx = 0;
            Utf8String postfix;
            for (NativeSqlBuilder& snippet : nativeSqlSnippets)
                {
                postfix.clear();
                postfix.Sprintf("%s_%d", alias.c_str(), idx);
                idx++;
                resultSet.push_back(postfix);
                snippet.AppendSpace().AppendEscaped(postfix.c_str());
                }
            }
        }
    else
        {
        for (NativeSqlBuilder& snippet : nativeSqlSnippets)
            {
            resultSet.push_back(snippet.GetSql());
            }
        }
    if (ctx.GetCurrentScope().IsRootScope())
        {
        ctx.GetCurrentScopeR().IncrementNativeSqlSelectClauseColumnCount(nativeSqlSnippets.size() - snippetCountBefore);
        ECSqlStatus status = ECSqlFieldFactory::CreateField(ctx, &exp, startColumnIndex);
        if (!status.IsSuccess())
            return status;
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus ECSqlSelectPreparer::ValidateSelectClauseItems(ECSqlPrepareContext& ctx, SelectClauseExp const& lhs, SelectClauseExp const& rhs)
    {
    const size_t count = lhs.GetChildrenCount();
    if (count != rhs.GetChildrenCount())
        {
        ctx.Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Number of properties in all the select clauses of UNION/EXCEPT/INTERSECT must be same in number and type.");
        return ERROR;
        }

    for (size_t i = 0; i < count; i++)
        {
        DerivedPropertyExp const* lhsDerivedPropExp = lhs.GetChildren().Get<DerivedPropertyExp>(i);
        DerivedPropertyExp const* rhsDerivedPropExp = rhs.GetChildren().Get<DerivedPropertyExp>(i);
        ECSqlTypeInfo const& lhsTypeInfo = lhsDerivedPropExp->GetExpression()->GetTypeInfo();
        ECSqlTypeInfo const& rhsTypeInfo = rhsDerivedPropExp->GetExpression()->GetTypeInfo();
        if (!lhsTypeInfo.CanCompare(rhsTypeInfo))
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Type of expression %s in LHS of UNION/EXCEPT/INTERSECT is not same as respective expression %s in RHS.", lhsDerivedPropExp->ToECSql().c_str(), rhsDerivedPropExp->ToECSql().c_str());
            return ERROR;
            }

        if (lhsTypeInfo.IsNull() && (rhsTypeInfo.IsPoint() || !rhsTypeInfo.IsPrimitive()))
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "NULL in LHS of UNION/EXCEPT/INTERSECT is ambiguous if its RHS counterpart is not of a primitive type (excluding Point2d and Point3d).");
            return ERROR;
            }
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ECSqlSelectPreparer::ExtractPropertyRefs(ECSqlPrepareContext& ctx, Exp const* exp)
    {
    if (exp == nullptr)
        return;

    if (exp->GetType() == Exp::Type::PropertyName && !static_cast<PropertyNameExp const*>(exp)->IsPropertyRef())
        {
        PropertyNameExp const* propertyName = static_cast<PropertyNameExp const*>(exp);
        if (propertyName->IsVirtualProperty())
            return;
        
        ctx.GetSelectionOptionsR().AddProperty(propertyName->GetPropertyMap());
        }

    for (Exp const* child : exp->GetChildren())
        {
        ExtractPropertyRefs(ctx, child);
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
