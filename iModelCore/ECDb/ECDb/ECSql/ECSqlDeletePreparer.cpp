/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlDeletePreparer.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlDeletePreparer::Prepare (ECSqlPrepareContext& ctx, DeleteStatementExp const& exp)
    {
    BeAssert (exp.IsComplete ());
    ctx.PushScope (exp, exp.GetOptionsClauseExp());

    auto classNameExp = exp.GetClassNameExp ();
    auto const& classMap = classNameExp->GetInfo ().GetMap ();
 
    NativeSqlSnippets deleteNativeSqlSnippets;
    auto stat = GenerateNativeSqlSnippets (deleteNativeSqlSnippets, ctx, exp, *classNameExp);
    if (!stat.IsSuccess())
        return stat;

    if (classMap.GetClassMapType () == ClassMap::Type::RelationshipEndTable)
        stat = PrepareForEndTableRelationship (ctx, deleteNativeSqlSnippets, static_cast<RelationshipClassEndTableMapCR> (classMap));
    else
        stat = PrepareForClass (ctx, deleteNativeSqlSnippets);

    ctx.PopScope ();
    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlDeletePreparer::PrepareForClass 
(
ECSqlPrepareContext& ctx, 
NativeSqlSnippets& nativeSqlSnippets
)
    {
    BuildNativeSqlDeleteStatement (ctx.GetSqlBuilderR (), nativeSqlSnippets); 
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlStatus ECSqlDeletePreparer::PrepareForEndTableRelationship 
(
ECSqlPrepareContext& ctx, 
NativeSqlSnippets& nativeSqlSnippets, 
RelationshipClassEndTableMapCR classMap
)
    {
    auto otherEndECInstanceIdPropMap = classMap.GetOtherEndECInstanceIdPropMap ();
    auto otherEndECClassIdPropMap = classMap.GetOtherEndECInstanceIdPropMap ();

    NativeSqlBuilder::List propertyNamesToUnsetSqlSnippets;
    classMap.GetPropertyMaps ().Traverse (
        [&propertyNamesToUnsetSqlSnippets, &otherEndECInstanceIdPropMap, &otherEndECClassIdPropMap] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        //virtual prop maps map to non-existing columns. So they don't need to be considered in the list of columns to be nulled out
        if (!propMap->IsVirtual () && (!propMap->IsSystemPropertyMap () || propMap == otherEndECInstanceIdPropMap || propMap == otherEndECClassIdPropMap))
            {
            auto sqlSnippets = propMap->ToNativeSql (nullptr, ECSqlType::Delete, false);
            propertyNamesToUnsetSqlSnippets.insert (propertyNamesToUnsetSqlSnippets.end (), sqlSnippets.begin (), sqlSnippets.end ());
            }
        }, 
        false); //don't recurse, i.e. only top-level prop maps are interesting here


    BuildNativeSqlUpdateStatement (ctx.GetSqlBuilderR (), nativeSqlSnippets, propertyNamesToUnsetSqlSnippets);
    return ECSqlStatus::Success;
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
        //WHERE [%s] IN (SELECT [%s].[%s] FROM [%s] INNER JOIN [%s] ON [%s].[%s] = [%s].[%s] WHERE (%s))
        NativeSqlBuilder whereClause;
        //Following generate optimized WHERE depending on what was accessed in WHERE class of delete. It will avoid uncessary
        IClassMap const& currentClassMap = classNameExp.GetInfo().GetMap();
        if (currentClassMap.IsMappedToSingleTable())
            {
            status = ECSqlExpPreparer::PrepareWhereExp(whereClause, ctx, whereExp);
            if (!status.IsSuccess())
                return status;

            deleteSqlSnippets.m_whereClauseNativeSqlSnippet = whereClause;
            }
        else
            {
            std::vector<Exp const*> propertyExpsInWhereClause =  whereExp->Find(Exp::Type::PropertyName, true);
            ECDbSqlTable& primaryTable = currentClassMap.GetPrimaryTable();
            ECDbSqlTable& joinedTable = currentClassMap.GetJoinedTable();

            /* WIP Needs fixes as the prepare picks the joined table when it should actually pick the primary table
            std::set<ECDbSqlTable const*> tablesReferencedByWhereClause = whereExp->GetReferencedTables();
            const bool primaryTableIsReferencedByWhereClause = (tablesReferencedByWhereClause.find(&primaryTable) != tablesReferencedByWhereClause.end());
            const bool joinedTableIsReferencedByWhereClause = (tablesReferencedByWhereClause.find(&joinedTable) != tablesReferencedByWhereClause.end());
            ECSqlSystemProperty systemPropExp = ECSqlSystemProperty::ECInstanceId;
            
            if (propertyExpsInWhereClause.size() == 1 && 
                static_cast<PropertyNameExp const*>(propertyExpsInWhereClause[0])->TryGetSystemProperty(systemPropExp) && systemPropExp == ECSqlSystemProperty::ECInstanceId)
                {
                //WhereClause only consists of ECInstanceId exp
                ctx.GetCurrentScopeR().SetExtendedOption(ECSqlPrepareContext::ExpScope::ExtendedOptions::SkipTableAliasWhenPreparingDeleteWhereClause);
                status = ECSqlExpPreparer::PrepareWhereExp(whereClause, ctx, whereExp);
                if (!status.IsSuccess())
                    return status;

                deleteSqlSnippets.m_whereClauseNativeSqlSnippet = whereClause;
                }
            else if (primaryTableIsReferencedByWhereClause && !joinedTableIsReferencedByWhereClause)
                {
                //only primary table is involved in where clause -> do not modify where
                status = ECSqlExpPreparer::PrepareWhereExp(whereClause, ctx, whereExp);
                if (!status.IsSuccess())
                    return status;

                deleteSqlSnippets.m_whereClauseNativeSqlSnippet = whereClause;
                }
            else*/
                {
                status = ECSqlExpPreparer::PrepareWhereExp(whereClause, ctx, whereExp);
                if (!status.IsSuccess())
                    return status;

                ECDbSqlColumn const* joinedTableIdCol = joinedTable.GetFilteredColumnFirst(ColumnKind::ECInstanceId);
                ECDbSqlColumn const* primaryTableIdCol = primaryTable.GetFilteredColumnFirst(ColumnKind::ECInstanceId);
                NativeSqlBuilder snippet;
                snippet.AppendFormatted(
                    " WHERE [%s] IN (SELECT [%s].[%s] FROM [%s] INNER JOIN [%s] ON [%s].[%s] = [%s].[%s] %s) ",
                    primaryTableIdCol->GetName().c_str(),
                    primaryTable.GetName().c_str(),
                    primaryTableIdCol->GetName().c_str(),
                    primaryTable.GetName().c_str(),
                    joinedTable.GetName().c_str(),
                    joinedTable.GetName().c_str(),
                    joinedTableIdCol->GetName().c_str(),
                    primaryTable.GetName().c_str(),
                    primaryTableIdCol->GetName().c_str(),
                    whereClause.ToString()
                    );

                deleteSqlSnippets.m_whereClauseNativeSqlSnippet = snippet;
                deleteSqlSnippets.m_whereClauseNativeSqlSnippet.ImportParameters(whereClause);
                }
            }
        }


    //System WHERE clause
    //if option to disable class id filter is set, nothing more to do
    if (exp.GetOptionsClauseExp() != nullptr && exp.GetOptionsClauseExp()->HasOption(OptionsExp::NOECCLASSIDFILTER_OPTION))
        return ECSqlStatus::Success;


    IClassMap const& classMap = classNameExp.GetInfo().GetMap();
    ECDbSqlTable const* table = &classMap.GetPrimaryTable();
    ECDbSqlColumn const* classIdColumn = nullptr;
    if (!table->TryGetECClassIdColumn(classIdColumn) || classIdColumn->GetPersistenceType() != PersistenceType::Persisted)
        return ECSqlStatus::Success; //no class id column exists -> no system where clause
    
    return classMap.GetStorageDescription().GenerateECClassIdFilter(deleteSqlSnippets.m_systemWhereClauseNativeSqlSnippet,
        *table,
        *classIdColumn,
        exp.GetClassNameExp()->IsPolymorphic()) == SUCCESS ? ECSqlStatus::Success : ECSqlStatus::Error;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ECSqlDeletePreparer::BuildNativeSqlDeleteStatement
(
NativeSqlBuilder& deleteBuilder,
NativeSqlSnippets const& deleteNativeSqlSnippets
)
    {
    deleteBuilder.Append ("DELETE FROM ").Append (deleteNativeSqlSnippets.m_classNameNativeSqlSnippet);

    bool whereAlreadyAppended = false;
    if (!deleteNativeSqlSnippets.m_whereClauseNativeSqlSnippet.IsEmpty ())
        {
        deleteBuilder.AppendSpace ().Append (deleteNativeSqlSnippets.m_whereClauseNativeSqlSnippet);
        whereAlreadyAppended = true;
        }
    
    if (!deleteNativeSqlSnippets.m_systemWhereClauseNativeSqlSnippet.IsEmpty ())
        {
        if (whereAlreadyAppended)
            deleteBuilder.Append (" AND ").AppendParenLeft();
        else
            deleteBuilder.Append (" WHERE ");

        deleteBuilder.Append (deleteNativeSqlSnippets.m_systemWhereClauseNativeSqlSnippet);
        
        if (whereAlreadyAppended)
            deleteBuilder.AppendParenRight();
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ECSqlDeletePreparer::BuildNativeSqlUpdateStatement
(
NativeSqlBuilder& updateBuilder,
NativeSqlSnippets const& deleteNativeSqlSnippets,
NativeSqlBuilder::List const& propNamesToUnsetNativeSqlSnippets
)
    {
    updateBuilder.Append ("UPDATE ").Append (deleteNativeSqlSnippets.m_classNameNativeSqlSnippet);

    //Columns of properties of the relationship need to be nulled out when "deleting" the relationship
    BeAssert (!propNamesToUnsetNativeSqlSnippets.empty ());
    updateBuilder.Append (" SET ");
    
    bool isFirstItem = true;
    for (auto const& sqlSnippet : propNamesToUnsetNativeSqlSnippets)
        {
        if (!isFirstItem)
            updateBuilder.AppendComma ();

        updateBuilder.Append (sqlSnippet).Append (" = NULL");
        isFirstItem = false;
        }


    bool whereAlreadyAppended = false;
    if (!deleteNativeSqlSnippets.m_whereClauseNativeSqlSnippet.IsEmpty ())
        {
        updateBuilder.AppendSpace ().Append (deleteNativeSqlSnippets.m_whereClauseNativeSqlSnippet);
        whereAlreadyAppended = true;
        }

    if (!deleteNativeSqlSnippets.m_systemWhereClauseNativeSqlSnippet.IsEmpty ())
        {
        if (whereAlreadyAppended)
            updateBuilder.Append (" AND ");
        else
            updateBuilder.Append (" WHERE ");

        updateBuilder.Append (deleteNativeSqlSnippets.m_systemWhereClauseNativeSqlSnippet);
        }
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
