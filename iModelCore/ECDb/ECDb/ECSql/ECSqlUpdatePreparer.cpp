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
    
    ECSqlStatus stat = CheckForReadonlyProperties(ctx, exp);
    if (stat != ECSqlStatus::Success)
        return stat;

    ClassNameExp const* classNameExp = exp.GetClassNameExp();

    SystemPropertyExpIndexMap const& specialTokenExpIndexMap = exp.GetAssignmentListExp()->GetSpecialTokenExpIndexMap();
    if (specialTokenExpIndexMap.Contains(ECSqlSystemPropertyInfo::Class::ECInstanceId))
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
            ECSqlStatus status = parentOfJoinedTableStmt->Prepare(ctx.GetECDb(), info->GetParentOfJoinedTableECSql(), ctx.GetWriteToken());
            if (status != ECSqlStatus::Success)
                {
                ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to prepare ECSQL '%s'. Could not prepare the subset of the ECSQL that targets the primary table: %s.", exp.ToECSql().c_str(), info->GetParentOfJoinedTableECSql());
                return ECSqlStatus::InvalidECSql;
                }
            }
        }

    if (classMap.IsRelationshipClassMap())
        {
        if (specialTokenExpIndexMap.Contains(ECSqlSystemPropertyInfo::Relationship::SourceECInstanceId) ||
            specialTokenExpIndexMap.Contains(ECSqlSystemPropertyInfo::Relationship::SourceECClassId)||
            specialTokenExpIndexMap.Contains(ECSqlSystemPropertyInfo::Relationship::TargetECInstanceId) ||
            specialTokenExpIndexMap.Contains(ECSqlSystemPropertyInfo::Relationship::TargetECClassId))
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
    NativeSqlSnippets snippets;
    status = PrepareAssignmentListExp(snippets, ctx, exp.GetAssignmentListExp());
    if (!status.IsSuccess())
        return status;

    //Skip overflow properties
    auto propertyListSnippets = NativeSqlBuilder::FlattenJaggedList(snippets.m_propertyNamesNativeSqlSnippets, snippets.m_overflowPropertyComponentIndexes);
    auto valueListSnippets = NativeSqlBuilder::FlattenJaggedList(snippets.m_valuesNativeSqlSnippets, snippets.m_overflowPropertyComponentIndexes);
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

    if (!snippets.m_overflowPropertyComponentIndexes.empty())
        {
        if (!propertyListSnippets.empty())
            nativeSqlBuilder.AppendComma();

        DbColumn const* overflowColumn = exp.GetClassNameExp()->GetInfo().GetMap().GetJoinedTable().GetPhysicalOverflowColumn();
        nativeSqlBuilder.AppendEscaped(overflowColumn->GetName().c_str()).Append(BooleanSqlOperator::EqualTo);
        nativeSqlBuilder.Append("json_set(ifnull(").AppendEscaped(overflowColumn->GetName().c_str()).AppendComma().Append("'{}')");

        std::vector<size_t> overflowIndexes;
        std::transform(std::begin(snippets.m_overflowPropertyComponentIndexes), std::end(snippets.m_overflowPropertyComponentIndexes), std::back_inserter(overflowIndexes),
                  [] (decltype(snippets.m_overflowPropertyComponentIndexes)::value_type const& pair)
            {
            return pair.first;
            });

        std::sort(begin(overflowIndexes), end(overflowIndexes));
        for (size_t i : overflowIndexes)
            {
            NativeSqlBuilder::List const& propertyNamesNativeSqlSnippets = snippets.m_propertyNamesNativeSqlSnippets[i];
            NativeSqlBuilder::List const& valuesNativeSqlSnippets = snippets.m_valuesNativeSqlSnippets[i];
            std::vector<size_t> const& overflowComponentIndexes = snippets.m_overflowPropertyComponentIndexes[i];
            std::vector<SingleColumnDataPropertyMap const*> const& overflowPropertyMaps = snippets.m_overflowPropertyMaps[i];
            for (size_t j = 0; j < overflowComponentIndexes.size(); j++)
                {
                const size_t k = overflowComponentIndexes[j];

                nativeSqlBuilder.Append(",'$.").Append(propertyNamesNativeSqlSnippets[k]).Append("',");

                const bool addBlobToBase64Func = overflowPropertyMaps[j]->GetPersistenceDataType() == DbColumn::Type::Blob;
                if (addBlobToBase64Func)
                    nativeSqlBuilder.Append(SQLFUNC_BlobToBase64 "(");

                nativeSqlBuilder.Append(valuesNativeSqlSnippets[k]);

                if (addBlobToBase64Func)
                    nativeSqlBuilder.AppendParenRight();
                }
            }

        nativeSqlBuilder.Append(")");
        }

    if (snippets.m_propertyNamesNativeSqlSnippets.empty())
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
                
                //topLevelWhereClause.GetParameterIndexMappings()
                topLevelWhereClause = snippet;
                topLevelWhereClause.CopyIndexMappingFrom(whereClause);
                }
            }
        else
            topLevelWhereClause = whereClause;
        }

    nativeSqlBuilder.Append(topLevelWhereClause);
    OptionsExp const* optionsExp = exp.GetOptionsClauseExp();
    if (optionsExp == nullptr || !optionsExp->HasOption(OptionsExp::NOECCLASSIDFILTER_OPTION))
        {
        // WHERE clause
        Utf8String systemWhereClause;
        DbTable const* table = &classMap.GetPrimaryTable();
        DbColumn const& classIdColumn = table->GetECClassIdColumn();

        if (classIdColumn.GetPersistenceType() == PersistenceType::Physical)
            {
            if (ctx.IsParentOfJoinedTable())
                {
                auto joinedTableClass = ctx.GetECDb().Schemas().GetECClass(ctx.GetJoinedTableClassId());
                auto joinedTableMap = ctx.GetECDb().Schemas().GetDbMap().GetClassMap(*joinedTableClass);
                if (SUCCESS != joinedTableMap->GetStorageDescription().GenerateECClassIdFilter(systemWhereClause, *table,
                    classIdColumn,
                    exp.GetClassNameExp()->IsPolymorphic()))
                    return ECSqlStatus::Error;
                }
            else if (ctx.GetJoinedTableInfo() != nullptr  && !ctx.GetJoinedTableInfo()->HasJoinedTableECSql())
                {
                auto joinedTableMap = ctx.GetECDb().Schemas().GetDbMap().GetClassMap(ctx.GetJoinedTableInfo()->GetClass());
                if (SUCCESS != joinedTableMap->GetStorageDescription().GenerateECClassIdFilter(systemWhereClause, *table,
                    classIdColumn,
                    exp.GetClassNameExp()->IsPolymorphic()))
                    return ECSqlStatus::Error;
                }
            else
                {
                if (SUCCESS != classMap.GetStorageDescription().GenerateECClassIdFilter(systemWhereClause, *table,
                    classIdColumn,
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
                ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "The ECProperty '%s' is read-only. Read-only ECProperties cannot be modified by an ECSQL UPDATE statement. %s",
                                                                        prop.GetName().c_str(), exp.ToECSql().c_str());
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
ECSqlStatus ECSqlUpdatePreparer::PrepareAssignmentListExp(NativeSqlSnippets& snippets, ECSqlPrepareContext& ctx, AssignmentListExp const* assignmentListExp)
    {
    //! Like SELECT we like to prepare leave nodes
    ctx.PushScope(*assignmentListExp);
    BeAssert(snippets.m_propertyNamesNativeSqlSnippets.empty());
    BeAssert(snippets.m_valuesNativeSqlSnippets.empty());

    size_t index = 0;
    for (auto childExp : assignmentListExp->GetChildren())
        {
        BeAssert(childExp != nullptr);
        AssignmentExp const* assignmentExp = static_cast<AssignmentExp const*> (childExp);
        if (assignmentExp->GetPropertyNameExp()->GetPropertyMap().IsData())
            {
            size_t component = 0;
            SearchPropertyMapVisitor visitor(PropertyMap::Type::SingleColumnData);
            assignmentExp->GetPropertyNameExp()->GetPropertyMap().AcceptVisitor(visitor);
            for (PropertyMap const* childPropertyMap : visitor.Results())
                {               
                SingleColumnDataPropertyMap const* childDataPropertyMap = childPropertyMap->GetAs<SingleColumnDataPropertyMap>();
                if (childDataPropertyMap->GetColumn().IsOverflowSlave())
                    {
                    snippets.m_overflowPropertyComponentIndexes[index].push_back(component);
                    snippets.m_overflowPropertyMaps[index].push_back(childDataPropertyMap);
                    }

                component++;
                }
            }

        
        NativeSqlBuilder::List propertyNamesNativeSqlSnippets;
        auto stat = ECSqlPropertyNameExpPreparer::Prepare(propertyNamesNativeSqlSnippets, ctx, assignmentExp->GetPropertyNameExp());
        if (!stat.IsSuccess())
            {
            ctx.PopScope();
            return stat;
            }

        const auto sqlSnippetCount = propertyNamesNativeSqlSnippets.size();
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

            snippets.m_propertyNamesNativeSqlSnippets.push_back(propertyNamesNativeSqlSnippets);
            snippets.m_valuesNativeSqlSnippets.push_back(rhsNativeSqlSnippets);
            }

        index++;
        }

    ctx.PopScope();
    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
