/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlDeletePreparer.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlDeletePreparer::Prepare(ECSqlPrepareContext& ctx, DeleteStatementExp const& exp)
    {
    BeAssert(exp.IsComplete());
    ctx.PushScope(exp, exp.GetOptionsClauseExp());

    ClassNameExp const* classNameExp = exp.GetClassNameExp();
    ClassMap const& classMap = classNameExp->GetInfo().GetMap();
    if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
        {
        BeAssert(false && "Should have been caught before");
        return ECSqlStatus::InvalidECSql;
        }

    NativeSqlSnippets deleteNativeSqlSnippets;
    ECSqlStatus stat = GenerateNativeSqlSnippets(deleteNativeSqlSnippets, ctx, exp, *classNameExp);
    if (!stat.IsSuccess())
        return stat;

    BuildNativeSqlDeleteStatement(ctx.GetSqlBuilder(), deleteNativeSqlSnippets);
    ctx.PopScope();
    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlDeletePreparer::GenerateNativeSqlSnippets(NativeSqlSnippets& deleteSqlSnippets, ECSqlPrepareContext& ctx, DeleteStatementExp const& exp, ClassNameExp const& classNameExp)
    {
    ECSqlStatus status = ECSqlExpPreparer::PrepareClassRefExp(deleteSqlSnippets.m_classNameNativeSqlSnippet, ctx, classNameExp);
    if (!status.IsSuccess())
        return status;

    WhereExp const* whereExp = exp.GetWhereClauseExp();
    if (whereExp != nullptr)
        {
        NativeSqlBuilder whereClause;
        ClassMap const& currentClassMap = classNameExp.GetInfo().GetMap();
        if (currentClassMap.IsMappedToSingleTable())
            {
            status = ECSqlExpPreparer::PrepareWhereExp(whereClause, ctx, *whereExp);
            if (!status.IsSuccess())
                return status;

            deleteSqlSnippets.m_whereClauseNativeSqlSnippet = whereClause;
            }
        else
            {
            ctx.GetCurrentScopeR().SetExtendedOption(ECSqlPrepareContext::ExpScope::ExtendedOptions::UsePrimaryTableForSystemPropertyResolution);
            std::set<DbTable const*> tableReferencedByDataProperties;
            //First Pass
            const std::vector<Exp const*> propertyNameExps = whereExp->Find(Exp::Type::PropertyName, true);
            const DbTable* primaryTable = &currentClassMap.GetPrimaryTable();
            for (Exp const* exp : propertyNameExps)
                {
                PropertyNameExp const& propertyNameExp = exp->GetAs<PropertyNameExp>();
                if (propertyNameExp.IsPropertyRef())
                    continue;

                PropertyMap const* propertyMap = propertyNameExp.GetTypeInfo().GetPropertyMap();
                if (propertyMap->IsData())
                    tableReferencedByDataProperties.insert(&propertyMap->GetAs<DataPropertyMap>().GetTable());
                }

             
            if (tableReferencedByDataProperties.empty() ||
                (tableReferencedByDataProperties.size() == 1 && tableReferencedByDataProperties.find(primaryTable) != tableReferencedByDataProperties.end())
                )
                {
                status = ECSqlExpPreparer::PrepareWhereExp(whereClause, ctx, *whereExp);
                if (!status.IsSuccess())
                    return status;

                deleteSqlSnippets.m_whereClauseNativeSqlSnippet = whereClause;
                }
            else
                {
                NativeSqlBuilder sql;
                DbTable const* last = nullptr;
                auto const start = tableReferencedByDataProperties.begin();
                auto const end = tableReferencedByDataProperties.end();
                sql.AppendFormatted("WHERE [%s] IN (", primaryTable->FindFirst(DbColumn::Kind::ECInstanceId)->GetName().c_str());
                for (auto itor = start; itor != end; ++itor)
                    {
                    DbTable const* current = (*itor);
                    Utf8CP currentTable = current->GetName().c_str();
                    Utf8CP currentPK = current->FindFirst(DbColumn::Kind::ECInstanceId)->GetName().c_str();
                    if (itor == start)
                        {
                        sql.AppendFormatted("SELECT [%s].[%s] FROM [%s]", currentTable, currentPK, currentTable);
                        }
                    else
                        {
                        Utf8CP lastTable = last->GetName().c_str();
                        Utf8CP lastPK = last->FindFirst(DbColumn::Kind::ECInstanceId)->GetName().c_str();
                        sql.AppendFormatted(" INNER JOIN [%s] ON [%s].[%s]=[%s].[%s]", currentTable, currentTable, currentPK, lastTable, lastPK);
                        }

                    last = current;
                    }

                status = ECSqlExpPreparer::PrepareWhereExp(whereClause, ctx, *whereExp);
                if (!status.IsSuccess())
                    return status;

                sql.AppendSpace().Append(whereClause).Append(")");
                deleteSqlSnippets.m_whereClauseNativeSqlSnippet = sql;
                }
            }
        }

    //System WHERE clause
    //if option to disable class id filter is set, nothing more to do
    OptionsExp const* optionsExp = exp.GetOptionsClauseExp();
    if (optionsExp != nullptr && optionsExp->HasOption(OptionsExp::NOECCLASSIDFILTER_OPTION))
        return ECSqlStatus::Success;

    Utf8String classIdFilter;
    if (ECSqlStatus::Success != ECSqlExpPreparer::GenerateECClassIdFilter(classIdFilter, classNameExp))
        return ECSqlStatus::Error;

    deleteSqlSnippets.m_systemWhereClauseNativeSqlSnippet.Append(classIdFilter.c_str());
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ECSqlDeletePreparer::BuildNativeSqlDeleteStatement(NativeSqlBuilder& deleteBuilder, NativeSqlSnippets const& deleteNativeSqlSnippets)
    {
    deleteBuilder.Append("DELETE FROM ").Append(deleteNativeSqlSnippets.m_classNameNativeSqlSnippet);

    bool whereAlreadyAppended = false;
    if (!deleteNativeSqlSnippets.m_whereClauseNativeSqlSnippet.IsEmpty())
        {
        deleteBuilder.AppendSpace().Append(deleteNativeSqlSnippets.m_whereClauseNativeSqlSnippet);
        whereAlreadyAppended = true;
        }

    if (!deleteNativeSqlSnippets.m_systemWhereClauseNativeSqlSnippet.IsEmpty())
        {
        if (whereAlreadyAppended)
            deleteBuilder.Append(" AND ").AppendParenLeft();
        else
            deleteBuilder.Append(" WHERE ");

        deleteBuilder.Append(deleteNativeSqlSnippets.m_systemWhereClauseNativeSqlSnippet);

        if (whereAlreadyAppended)
            deleteBuilder.AppendParenRight();
        }
    }



END_BENTLEY_SQLITE_EC_NAMESPACE
