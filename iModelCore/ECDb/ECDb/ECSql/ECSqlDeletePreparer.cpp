/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlDeletePreparer.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    ctx.PushScope (exp);

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
ECSqlStatus ECSqlDeletePreparer::GenerateNativeSqlSnippets 
(
NativeSqlSnippets& deleteSqlSnippets, 
ECSqlPrepareContext& ctx, 
DeleteStatementExp const& exp,
ClassNameExp const& classNameExp
)
    {
    auto status = ECSqlExpPreparer::PrepareClassRefExp (deleteSqlSnippets.m_classNameNativeSqlSnippet, ctx, classNameExp);
    if (!status.IsSuccess())
        return status;

   
    //WHERE [%s] IN (SELECT [%s].[%s] FROM [%s] INNER JOIN [%s] ON [%s].[%s] = [%s].[%s] WHERE (%s))
    if (auto whereClauseExp = exp.GetOptWhereClauseExp())
        {
        NativeSqlBuilder whereClause;
        status = ECSqlExpPreparer::PrepareWhereExp(whereClause, ctx, whereClauseExp);
        if (!status.IsSuccess())
            return status;

        //Following generate optimized WHERE depending on what was accessed in WHERE class of delete. It will avoid uncessary
        auto const & currentClassMap = classNameExp.GetInfo().GetMap();
        if (auto rootOfJoinedTable = currentClassMap.FindRootOfJoinedTable())
            {
            auto const tableBeenAccessed = whereClauseExp->GetReferencedTables();
            bool referencedRootOfJoinedTable = (tableBeenAccessed.find(&rootOfJoinedTable->GetTable()) != tableBeenAccessed.end());
            bool referencedJoinedTable = (tableBeenAccessed.find(&currentClassMap.GetTable()) != tableBeenAccessed.end());
            if (referencedRootOfJoinedTable && !referencedJoinedTable)
                {
                //do not modifiy where
                deleteSqlSnippets.m_whereClauseNativeSqlSnippet = whereClause;
                }
            else
                {
                auto joinedTableId = currentClassMap.GetTable().GetFilteredColumnFirst(ECDbKnownColumns::ECInstanceId);
                auto parentOfjoinedTableId = rootOfJoinedTable->GetTable().GetFilteredColumnFirst(ECDbKnownColumns::ECInstanceId);
                NativeSqlBuilder snippet;
                snippet.AppendFormatted(
                    " WHERE [%s] IN (SELECT [%s].[%s] FROM [%s] INNER JOIN [%s] ON [%s].[%s] = [%s].[%s] %s) ",
                    parentOfjoinedTableId->GetName().c_str(),
                    rootOfJoinedTable->GetTable().GetName().c_str(),
                    parentOfjoinedTableId->GetName().c_str(),
                    rootOfJoinedTable->GetTable().GetName().c_str(),
                    currentClassMap.GetTable().GetName().c_str(),
                    currentClassMap.GetTable().GetName().c_str(),
                    joinedTableId->GetName().c_str(),
                    rootOfJoinedTable->GetTable().GetName().c_str(),
                    parentOfjoinedTableId->GetName().c_str(),
                    whereClause.ToString()
                    );

                deleteSqlSnippets.m_whereClauseNativeSqlSnippet = snippet;
                }
            }
        else
            deleteSqlSnippets.m_whereClauseNativeSqlSnippet = whereClause;
        }
    
    IClassMap const& classMap = classNameExp.GetInfo().GetMap();
    auto& storageDesc = classMap.GetStorageDescription ();
    if (storageDesc.GetNonVirtualHorizontalPartitionIndices ().empty() || !exp.GetClassNameExp ()->IsPolymorphic ())
        {
        if (auto classIdColumn = classMap.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECClassId))
            {
            if (classIdColumn->GetPersistenceType() == PersistenceType::Persisted)
                {
                deleteSqlSnippets.m_systemWhereClauseNativeSqlSnippet.AppendEscaped (classIdColumn->GetName().c_str()).Append(" = ").Append (classMap.GetClass ().GetId ());
                }
            }
        }
    else if (storageDesc.GetNonVirtualHorizontalPartitionIndices ().size() == 1 && exp.GetClassNameExp ()->IsPolymorphic ())
        {
        if (auto classIdColumn = classMap.GetTable ().GetFilteredColumnFirst (ECDbKnownColumns::ECClassId))
            {
            if (classIdColumn->GetPersistenceType () == PersistenceType::Persisted)
                {
                auto& partition = storageDesc.GetHorizontalPartitions ().at (storageDesc.GetNonVirtualHorizontalPartitionIndices ().at (0));               
                if (partition.NeedsClassIdFilter()) 
                    deleteSqlSnippets.m_systemWhereClauseNativeSqlSnippet.AppendEscaped(classIdColumn->GetName().c_str());
                }
            }
        }
    return ECSqlStatus::Success;
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
            deleteBuilder.Append (" AND ");
        else
            deleteBuilder.Append (" WHERE ");

        deleteBuilder.Append (deleteNativeSqlSnippets.m_systemWhereClauseNativeSqlSnippet);
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
