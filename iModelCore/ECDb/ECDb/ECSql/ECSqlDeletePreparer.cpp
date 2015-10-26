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

    if (auto whereClauseExp = exp.GetOptWhereClauseExp ())
        {
        status = ECSqlExpPreparer::PrepareWhereExp (deleteSqlSnippets.m_whereClauseNativeSqlSnippet, ctx, whereClauseExp);
        if (!status.IsSuccess())
            return status;
        }

    IClassMap const& classMap = classNameExp.GetInfo().GetMap();

    ECDbSqlColumn const* classIdColumn = nullptr;
    if (!classMap.GetTable().TryGetECClassIdColumn(classIdColumn) || classIdColumn->GetPersistenceType() != PersistenceType::Persisted)
        return ECSqlStatus::Success; //no class id column exists -> no system where clause
    
    return classMap.GetStorageDescription().GenerateECClassIdFilter(deleteSqlSnippets.m_systemWhereClauseNativeSqlSnippet, 
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
