/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlUpdatePreparer.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlUpdatePreparer.h"
#include "ECSqlPropertyNameExpPreparer.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlUpdatePreparer::Prepare(ECSqlPrepareContext& ctx, UpdateStatementExp const& exp)
    {
    BeAssert(exp.IsComplete());
    ctx.PushScope(exp, exp.GetOptionsClauseExp());
    
    ECSqlStatus stat = CheckForReadonlyProperties(ctx, exp);
    if (stat != ECSqlStatus::Success)
        return stat;

    ClassNameExp const* classNameExp = exp.GetClassNameExp();

    Exp::SystemPropertyExpIndexMap const& specialTokenExpIndexMap = exp.GetAssignmentListExp()->GetSpecialTokenExpIndexMap();
    if (!specialTokenExpIndexMap.IsUnset(ECSqlSystemProperty::ECInstanceId))
        {
        ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECInstanceId is not allowed in SET clause of ECSQL UPDATE statement. ECDb does not support to modify auto-generated ECInstanceIds.");
        return ECSqlStatus::InvalidECSql;
        }

    ClassMap const& classMap = classNameExp->GetInfo().GetMap();
    if (auto info = ctx.GetJoinedTableInfo())
        {
        if (info->HasParentOfJoinedTableECSql() && info->HasJoinedTableECSql())
            {
            ParentOfJoinedTableECSqlStatement* parentOfJoinedTableStmt = ctx.GetECSqlStatementR().GetPreparedStatementP()->CreateParentOfJoinedTableECSqlStatement(classMap.GetClass().GetId());
            ECSqlStatus status = parentOfJoinedTableStmt->Prepare(ctx.GetECDb(), info->GetParentOfJoinedTableECSql());
            if (status != ECSqlStatus::Success)
                {
                ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Preparing the ECSQL '%s' failed. Preparing the primary table ECSQL '%s' failed", exp.ToECSql().c_str(), info->GetParentOfJoinedTableECSql());
                return ECSqlStatus::InvalidECSql;
                }
            }
        }

    if (classMap.IsRelationshipClassMap())
        {
        if (!specialTokenExpIndexMap.IsUnset(ECSqlSystemProperty::SourceECInstanceId) ||
            !specialTokenExpIndexMap.IsUnset(ECSqlSystemProperty::SourceECClassId) ||
            !specialTokenExpIndexMap.IsUnset(ECSqlSystemProperty::TargetECInstanceId) ||
            !specialTokenExpIndexMap.IsUnset(ECSqlSystemProperty::TargetECClassId))
            {
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "SourceECInstanceId, TargetECInstanceId, SourceECClassId, or TargetECClassId are not allowed in the SET clause of ECSQL UPDATE statement. ECDb does not support to modify those as they are keys of the relationship. Instead delete the relationship and insert the desired new one.");
            return ECSqlStatus::InvalidECSql;
            }
        }

    NativeSqlBuilder& nativeSqlBuilder = ctx.GetSqlBuilderR();

    // UPDATE clause
    nativeSqlBuilder.Append("UPDATE ");
    auto status = ECSqlExpPreparer::PrepareClassRefExp(nativeSqlBuilder, ctx, *classNameExp);
    if (!status.IsSuccess())
        return status;

    // SET clause
    NativeSqlBuilder::ListOfLists assignmentListSnippetLists;
    status = PrepareAssignmentListExp(assignmentListSnippetLists, ctx, exp.GetAssignmentListExp());
    if (!status.IsSuccess())
        return status;

    const std::vector<size_t> emptyIndexSkipList;
    auto assignmentListSnippets = NativeSqlBuilder::FlattenJaggedList(assignmentListSnippetLists, emptyIndexSkipList);
    nativeSqlBuilder.Append(" SET ").Append(assignmentListSnippets);
    if (assignmentListSnippets.size() == 0)
        ctx.SetNativeStatementIsNoop(true);

    //WHERE [%s] IN (SELECT [%s].[%s] FROM [%s] INNER JOIN [%s] ON [%s].[%s] = [%s].[%s] WHERE (%s))
    NativeSqlBuilder topLevelWhereClause;
    if (auto whereClauseExp = exp.GetWhereClauseExp())
        {
        NativeSqlBuilder whereClause;
        status = ECSqlExpPreparer::PrepareWhereExp(whereClause, ctx, whereClauseExp);
        if (!status.IsSuccess())
            return status;

        //Following generate optimized WHERE depending on what was accessed in WHERE class of delete.
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
                    " WHERE [%s] IN (SELECT [%s].[%s] FROM [%s] INNER JOIN [%s] ON [%s].[%s] = [%s].[%s] %s) ",
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

                topLevelWhereClause = snippet;
                }
            }
        else
            topLevelWhereClause = whereClause;
        }

    nativeSqlBuilder.Append(topLevelWhereClause);
    if (exp.GetOptionsClauseExp() == nullptr || !exp.GetOptionsClauseExp()->HasOption(OptionsExp::NOECCLASSIDFILTER_OPTION))
        {
        // WHERE clause
        Utf8String systemWhereClause;
        DbColumn const* classIdColumn = nullptr;
        DbTable const* table = &classMap.GetPrimaryTable();

        if (table->TryGetECClassIdColumn(classIdColumn) &&
            classIdColumn->GetPersistenceType() == PersistenceType::Persisted)
            {
            if (ctx.IsParentOfJoinedTable())
                {
                auto joinedTableClass = ctx.GetECDb().Schemas().GetECClass(ctx.GetJoinedTableClassId());
                auto joinedTableMap = ctx.GetECDb().GetECDbImplR().GetECDbMap().GetClassMap(*joinedTableClass);
                if (SUCCESS != joinedTableMap->GetStorageDescription().GenerateECClassIdFilter(systemWhereClause, *table,
                    *classIdColumn,
                    exp.GetClassNameExp()->IsPolymorphic()))
                    return ECSqlStatus::Error;
                }
            else if (ctx.GetJoinedTableInfo() != nullptr  && !ctx.GetJoinedTableInfo()->HasJoinedTableECSql())
                {
                auto joinedTableMap = ctx.GetECDb().GetECDbImplR().GetECDbMap().GetClassMap(ctx.GetJoinedTableInfo()->GetClass());
                if (SUCCESS != joinedTableMap->GetStorageDescription().GenerateECClassIdFilter(systemWhereClause, *table,
                    *classIdColumn,
                    exp.GetClassNameExp()->IsPolymorphic()))
                    return ECSqlStatus::Error;
                }
            else
                {
                if (SUCCESS != classMap.GetStorageDescription().GenerateECClassIdFilter(systemWhereClause, *table,
                    *classIdColumn,
                    exp.GetClassNameExp()->IsPolymorphic()))
                    return ECSqlStatus::Error;
                }
            }

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
// @bsimethod                                    Affan.Khan                         05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlStatus ECSqlUpdatePreparer::CheckForReadonlyProperties(ECSqlPrepareContext& ctx, UpdateStatementExp const& exp)
    {
    for (Exp const* expr : exp.GetAssignmentListExp()->GetChildren())
        {
        AssignmentExp const* assignmentExp = static_cast<AssignmentExp const*>(expr);        
        if (!assignmentExp->GetPropertyNameExp()->IsPropertyRef())
            {
            ECPropertyCR prop = assignmentExp->GetPropertyNameExp()->GetPropertyMap().GetProperty();

            if (prop.IsReadOnlyFlagSet() && prop.GetIsReadOnly() && !prop.IsCalculated())
                {
                ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Preparing the ECSQL '%s' failed. Cannot insert data into ECProperty '%s' which is marked as 'IsReadonly'",
                                                                        exp.ToECSql().c_str(), prop.GetName().c_str());
                return ECSqlStatus::InvalidECSql;
                }
            }
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlUpdatePreparer::PrepareAssignmentListExp(NativeSqlBuilder::ListOfLists& nativeSqlSnippetLists, ECSqlPrepareContext& ctx, AssignmentListExp const* assignmentListExp)
    {
    ctx.PushScope(*assignmentListExp);
    BeAssert(nativeSqlSnippetLists.empty());
    for (auto childExp : assignmentListExp->GetChildren())
        {
        BeAssert(childExp != nullptr);

        auto assignmentExp = static_cast<AssignmentExp const*> (childExp);
        NativeSqlBuilder::List nativeSqlSnippets;
        auto stat = ECSqlPropertyNameExpPreparer::Prepare(nativeSqlSnippets, ctx, assignmentExp->GetPropertyNameExp());
        if (!stat.IsSuccess())
            {
            ctx.PopScope();
            return stat;
            }

        const auto sqlSnippetCount = nativeSqlSnippets.size();
        //If target expression does not have any SQL snippets, it means the expression is not necessary in SQLite SQL (e.g. for source/target class id props)
        //In that case the respective value exp does not need to be prepared either.
        if (sqlSnippetCount > 0)
            {
            NativeSqlBuilder::List rhsNativeSqlSnippets;
            auto valueExp = assignmentExp->GetValueExp();
            //if value is null exp, we need to pass target operand snippets
            if (ECSqlExpPreparer::IsNullExp(*valueExp))
                {
                BeAssert(dynamic_cast<LiteralValueExp const*> (valueExp) != nullptr);
                stat = ECSqlExpPreparer::PrepareNullLiteralValueExp(rhsNativeSqlSnippets, ctx, static_cast<LiteralValueExp const*> (valueExp), sqlSnippetCount);
                }
            else
                stat = ECSqlExpPreparer::PrepareValueExp(rhsNativeSqlSnippets, ctx, valueExp);

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

            for (size_t i = 0; i < sqlSnippetCount; i++)
                {
                nativeSqlSnippets[i].Append(BooleanSqlOperator::EqualTo).Append(rhsNativeSqlSnippets[i]);
                }
            }
        else
            {
            if (assignmentExp->GetPropertyNameExp()->GetTypeInfo().GetKind() == ECSqlTypeInfo::Kind::StructArray)
                {
                auto valueExp = assignmentExp->GetValueExp();
                if (!ECSqlExpPreparer::IsNullExp(*valueExp))
                    {
                    NativeSqlBuilder::List rhsNativeSqlSnippets;
                    auto valueExp = assignmentExp->GetValueExp();
                    stat = ECSqlExpPreparer::PrepareValueExp(rhsNativeSqlSnippets, ctx, valueExp);
                    }
                }

            }

        nativeSqlSnippetLists.push_back(move(nativeSqlSnippets));
        }

    ctx.PopScope();
    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
