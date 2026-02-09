/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlUpdatePreparer.h"
#include "ECSqlPropertyNameExpPreparer.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlUpdatePreparer::Prepare(ECSqlPrepareContext& ctx, UpdateStatementExp const& exp)
    {
    BeAssert(exp.IsComplete());
    ClassMap const& classMap = exp.GetClassNameExp()->GetInfo().GetMap();
    if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
        {
        BeAssert(false && "Should have been caught before");
        return ECSqlStatus::InvalidECSql;
        }

    ctx.PushScope(exp, exp.GetOptionsClauseExp());
    NativeSqlBuilder& nativeSqlBuilder = ctx.GetSqlBuilder();

    ClassNameExp const* classNameExp = exp.GetClassNameExp();

    // UPDATE clause
    nativeSqlBuilder.Append("UPDATE ");
    ECSqlStatus status = ECSqlExpPreparer::PrepareClassRefExp(nativeSqlBuilder, ctx, *classNameExp);
    if (!status.IsSuccess())
        return status;

    // SET clause
    NativeSqlSnippets sqlSnippets;
    status = PrepareAssignmentListExp(sqlSnippets, ctx, exp.GetAssignmentListExp());
    if (!status.IsSuccess())
        return status;

    const std::vector<size_t> emptyIndexSkipList;
    NativeSqlBuilder::List propertyListSnippets = NativeSqlBuilder::FlattenJaggedList(sqlSnippets.m_propertyNamesNativeSqlSnippets, emptyIndexSkipList);
    NativeSqlBuilder::List valueListSnippets = NativeSqlBuilder::FlattenJaggedList(sqlSnippets.m_valuesNativeSqlSnippets, emptyIndexSkipList);
    if (propertyListSnippets.size() != valueListSnippets.size())
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }

    nativeSqlBuilder.Append(" SET ");
    for (size_t i = 0; i < propertyListSnippets.size(); i++)
        {
        if (i != 0)
            nativeSqlBuilder.AppendComma();

        nativeSqlBuilder.Append(propertyListSnippets[i]).Append(ExpHelper::ToSql(BooleanSqlOperator::EqualTo)).Append(valueListSnippets[i]);
        }

    if (sqlSnippets.m_propertyNamesNativeSqlSnippets.empty())
        ctx.SetNativeStatementIsNoop(true);

    NativeSqlBuilder topLevelWhereClause;
    if (auto whereClauseExp = exp.GetWhereClauseExp())
        {
        NativeSqlBuilder whereClause;
        status = ECSqlExpPreparer::PrepareWhereExp(whereClause, ctx, *whereClauseExp);
        if (!status.IsSuccess())
            return status;

        topLevelWhereClause = whereClause;
        }

    if (!topLevelWhereClause.IsEmpty())
        nativeSqlBuilder.AppendSpace().Append(topLevelWhereClause);

    OptionsExp const* optionsExp = exp.GetOptionsClauseExp();
    if (optionsExp == nullptr || !optionsExp->HasOption(OptionsExp::NOECCLASSIDFILTER_OPTION))
        {
        // WHERE clause
        Utf8String systemWhereClause;
        if (ECSqlStatus::Success != ECSqlExpPreparer::GenerateECClassIdFilter(systemWhereClause, *classNameExp))
            return ECSqlStatus::Error;

        if (!systemWhereClause.empty())
            {
            if (topLevelWhereClause.IsEmpty())
                nativeSqlBuilder.Append(" WHERE ");
            else
                nativeSqlBuilder.Append(" AND ");

            nativeSqlBuilder.AppendParenLeft().Append(systemWhereClause.c_str()).AppendParenRight();
            }
        }

    ctx.PopScope();
    return status;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlUpdatePreparer::PrepareAssignmentListExp(NativeSqlSnippets& snippets, ECSqlPrepareContext& ctx, AssignmentListExp const* assignmentListExp)
    {
    //! Like SELECT we like to prepare leave nodes
    ctx.PushScope(*assignmentListExp);
    BeAssert(snippets.m_propertyNamesNativeSqlSnippets.empty());
    BeAssert(snippets.m_valuesNativeSqlSnippets.empty());

    for (Exp const* childExp : assignmentListExp->GetChildren())
        {
        BeAssert(childExp != nullptr);
        AssignmentExp const& assignmentExp = childExp->GetAs<AssignmentExp>();
        NativeSqlBuilder::List propertyNamesNativeSqlSnippets;
        ECSqlStatus stat = ECSqlPropertyNameExpPreparer::Prepare(propertyNamesNativeSqlSnippets, ctx, *assignmentExp.GetPropertyNameExp());
        if (!stat.IsSuccess())
            {
            ctx.PopScope();
            return stat;
            }

        const size_t sqlSnippetCount = propertyNamesNativeSqlSnippets.size();
        //If target expression does not have any SQL snippets, it means the expression is not necessary in SQLite SQL (e.g. for source/target class id props)
        //In that case the respective value exp does not need to be prepared either.
        if (sqlSnippetCount > 0)
            {
            NativeSqlBuilder::List rhsNativeSqlSnippets;
            ValueExp const* valueExp = assignmentExp.GetValueExp();
            //if value is null exp, we need to pass target operand snippets
            if (!valueExp->IsParameterExp() && valueExp->GetTypeInfo().IsNull())
                stat = ECSqlExpPreparer::PrepareNullExp(rhsNativeSqlSnippets, ctx, *valueExp, sqlSnippetCount);
            else
                stat = ECSqlExpPreparer::PrepareValueExp(rhsNativeSqlSnippets, ctx, *valueExp);

            if (!stat.IsSuccess())
                {
                ctx.PopScope();
                return stat;
                }

            if (sqlSnippetCount != rhsNativeSqlSnippets.size())
                {
                BeAssert(false && "LHS and RHS SQLite SQL snippet count differs for the ECSQL UPDATE assignment");
                return ECSqlStatus::Error;
                }

            snippets.m_propertyNamesNativeSqlSnippets.push_back(propertyNamesNativeSqlSnippets);
            snippets.m_valuesNativeSqlSnippets.push_back(rhsNativeSqlSnippets);
            }
        }

    ctx.PopScope();
    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
