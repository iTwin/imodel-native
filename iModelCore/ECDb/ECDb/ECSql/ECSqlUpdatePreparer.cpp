/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlUpdatePreparer.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlUpdatePreparer.h"
#include "ECSqlPropertyNameExpPreparer.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlUpdatePreparer::Prepare(ECSqlPrepareContext& ctx, UpdateStatementExp const& exp)
    {
    BeAssert(exp.IsComplete());

    ctx.PushScope(exp, exp.GetOptionsClauseExp());

#ifndef ECSQLPREPAREDSTATEMENT_REFACTOR
    ECSqlStatus stat = CheckForReadonlyProperties(ctx, exp);
    if (stat != ECSqlStatus::Success)
        return stat;
#endif

    ClassNameExp const* classNameExp = exp.GetClassNameExp();

#ifndef ECSQLPREPAREDSTATEMENT_REFACTOR
    ClassMap const& classMap = classNameExp->GetInfo().GetMap();
    SystemPropertyExpIndexMap const& specialTokenExpIndexMap = exp.GetAssignmentListExp()->GetSpecialTokenExpIndexMap();
    if (specialTokenExpIndexMap.Contains(ECSqlSystemPropertyInfo::ECInstanceId()))
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDBSYS_PROP_ECInstanceId " is not allowed in SET clause of ECSQL UPDATE statement. ECDb does not support to modify auto-generated " ECDBSYS_PROP_ECInstanceId "s.");
        return ECSqlStatus::InvalidECSql;
        }

    if (auto info = ctx.GetJoinedTableInfo())
        {
        if (info->HasParentOfJoinedTableECSql() && info->HasJoinedTableECSql())
            {
            ParentOfJoinedTableECSqlStatement* parentOfJoinedTableStmt = ctx.GetECSqlStatementR().GetPreparedStatementP()->CreateParentOfJoinedTableECSqlStatement(classMap.GetClass().GetId());
            ECSqlStatus status = parentOfJoinedTableStmt->Prepare(ctx.GetECDb(), info->GetParentOfJoinedTableECSql(), ctx.GetWriteToken());
            if (status != ECSqlStatus::Success)
                {
                ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report("Failed to prepare ECSQL '%s'. Could not prepare the subset of the ECSQL that targets the primary table: %s.", exp.ToECSql().c_str(), info->GetParentOfJoinedTableECSql());
                return ECSqlStatus::InvalidECSql;
                }
            }
        }
    if (classMap.IsRelationshipClassMap())
        {
        if (specialTokenExpIndexMap.Contains(ECSqlSystemPropertyInfo::SourceECInstanceId()) ||
            specialTokenExpIndexMap.Contains(ECSqlSystemPropertyInfo::SourceECClassId())||
            specialTokenExpIndexMap.Contains(ECSqlSystemPropertyInfo::TargetECInstanceId()) ||
            specialTokenExpIndexMap.Contains(ECSqlSystemPropertyInfo::TargetECClassId()))
            {
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDBSYS_PROP_SourceECInstanceId ", " ECDBSYS_PROP_SourceECClassId ", " ECDBSYS_PROP_TargetECInstanceId ", or " ECDBSYS_PROP_TargetECClassId " are not allowed in the SET clause of ECSQL UPDATE statement. ECDb does not support to modify those as they are keys of the relationship. Instead delete the relationship and insert the desired new one.");
            return ECSqlStatus::InvalidECSql;
            }
        }
#endif

    NativeSqlBuilder& nativeSqlBuilder = ctx.GetSqlBuilderR();

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

        nativeSqlBuilder.Append(propertyListSnippets[i]).Append(BooleanSqlOperator::EqualTo).Append(valueListSnippets[i]);
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

#ifdef ECSQLPREPAREDSTATEMENT_REFACTOR
        topLevelWhereClause = whereClause;
#else
        //WHERE [%s] IN (SELECT [%s].[%s] FROM [%s] INNER JOIN [%s] ON [%s].[%s] = [%s].[%s] WHERE (%s))
        auto const & currentClassMap = classMap;
        if (!currentClassMap.IsMappedToSingleTable())
            {
            auto& primaryTable = currentClassMap.GetPrimaryTable();
            auto& secondaryTable = currentClassMap.GetJoinedTable();

            auto const tableBeenAccessed = whereClauseExp->GetReferencedTables();
            bool referencedRootOfJoinedTable = (tableBeenAccessed.find(&primaryTable) != tableBeenAccessed.end());
            bool referencedJoinedTable = (tableBeenAccessed.find(&secondaryTable) != tableBeenAccessed.end());
            if (!referencedRootOfJoinedTable && referencedJoinedTable)
                {
                //do not modifiy where
                topLevelWhereClause = whereClause;
                }
            else
                {
                auto joinedTableId = secondaryTable.GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
                auto parentOfjoinedTableId = primaryTable.GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
                NativeSqlBuilder snippet;
                snippet.AppendFormatted(
                    "WHERE [%s] IN (SELECT [%s].[%s] FROM [%s] INNER JOIN [%s] ON [%s].[%s]=[%s].[%s] %s)",
                    joinedTableId->GetName().c_str(),
                    primaryTable.GetName().c_str(),
                    parentOfjoinedTableId->GetName().c_str(),
                    primaryTable.GetName().c_str(),
                    secondaryTable.GetName().c_str(),
                    secondaryTable.GetName().c_str(),
                    joinedTableId->GetName().c_str(),
                    primaryTable.GetName().c_str(),
                    parentOfjoinedTableId->GetName().c_str(),
                    whereClause.ToString()
                    );
                
                //topLevelWhereClause.GetParameterIndexMappings()
                topLevelWhereClause = snippet;
                }
            }
        else
            topLevelWhereClause = whereClause;
#endif
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

#ifndef ECSQLPREPAREDSTATEMENT_REFACTOR

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                         05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlStatus ECSqlUpdatePreparer::CheckForReadonlyProperties(ECSqlPrepareContext& ctx, UpdateStatementExp const& exp)
    {
    OptionsExp const* optionsExp = ctx.GetCurrentScope().GetOptions();
    if (optionsExp != nullptr && optionsExp->HasOption(OptionsExp::READONLYPROPERTIESAREUPDATABLE_OPTION))
        return ECSqlStatus::Success;

    for (Exp const* expr : exp.GetAssignmentListExp()->GetChildren())
        {
        AssignmentExp const* assignmentExp = static_cast<AssignmentExp const*>(expr);        
        if (!assignmentExp->GetPropertyNameExp()->IsPropertyRef())
            {
            ECPropertyCR prop = assignmentExp->GetPropertyNameExp()->GetPropertyMap().GetProperty();

            if (prop.IsReadOnlyFlagSet() && prop.GetIsReadOnly() && !prop.IsCalculated())
                {
                ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report("The ECProperty '%s' is read-only. Read-only ECProperties cannot be modified by an ECSQL UPDATE statement. %s",
                                                                        prop.GetName().c_str(), exp.ToECSql().c_str());
                return ECSqlStatus::InvalidECSql;
                }
            }
        }

    return ECSqlStatus::Success;
    }
#endif
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlUpdatePreparer::PrepareAssignmentListExp(NativeSqlSnippets& snippets, ECSqlPrepareContext& ctx, AssignmentListExp const* assignmentListExp)
    {
    //! Like SELECT we like to prepare leave nodes
    ctx.PushScope(*assignmentListExp);
    BeAssert(snippets.m_propertyNamesNativeSqlSnippets.empty());
    BeAssert(snippets.m_valuesNativeSqlSnippets.empty());

    size_t index = 0;
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
            if (ECSqlExpPreparer::IsNullExp(*valueExp))
                stat = ECSqlExpPreparer::PrepareNullLiteralValueExp(rhsNativeSqlSnippets, ctx, valueExp->GetAs<LiteralValueExp>(), sqlSnippetCount);
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

        index++;
        }

    ctx.PopScope();
    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
