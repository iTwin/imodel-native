/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/SystemColumnPreparer.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SystemColumnPreparer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//****************************************************************
// SystemColumnPreparer
//****************************************************************
//static
std::map<IClassMap::Type, std::unique_ptr<SystemColumnPreparer>> SystemColumnPreparer::s_flyweights;

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2015
//---------------------------------------------------------------------------------------
ECSqlStatus SystemColumnPreparer::GetWhereClause(ECSqlPrepareContext& ctx, NativeSqlBuilder& whereClauseBuilder, IClassMap const& classMap, ECSqlType ecsqlType, bool isPolymorphicClassExp, Utf8CP tableAlias) const
    {
    return _GetWhereClause(ctx, whereClauseBuilder, classMap, ecsqlType, isPolymorphicClassExp, tableAlias);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2015
//---------------------------------------------------------------------------------------
//static
SystemColumnPreparer const& SystemColumnPreparer::GetFor(IClassMap const& classMap)
    {
    return GetFor(classMap.GetClassMapType());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2015
//---------------------------------------------------------------------------------------
//static
SystemColumnPreparer const& SystemColumnPreparer::GetDefault()
    {
    return GetFor(IClassMap::Type::Class);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2015
//---------------------------------------------------------------------------------------
//static
SystemColumnPreparer const& SystemColumnPreparer::GetFor(IClassMap::Type classMapType)
    {
    auto it = s_flyweights.find(classMapType);
    if (it == s_flyweights.end())
        {
        std::unique_ptr<SystemColumnPreparer> preparer = nullptr;
        switch (classMapType)
            {
                case IClassMap::Type::RelationshipEndTable:
                    preparer = std::unique_ptr<SystemColumnPreparer>(new EndTableSystemColumnPreparer());
                    break;
                case IClassMap::Type::SecondaryTable:
                    preparer = std::unique_ptr<SystemColumnPreparer>(new SecondaryTableSystemColumnPreparer());
                    break;
                default:
                    preparer = std::unique_ptr<SystemColumnPreparer>(new RegularClassSystemColumnPreparer());
                    break;
            }

        SystemColumnPreparer const& preparerR = *preparer;
        s_flyweights[classMapType] = std::move(preparer);

        return preparerR;
        }

    return *it->second;
    }


//****************************************************************
// RegularClassSystemColumnPreparer
//****************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2015
//---------------------------------------------------------------------------------------
ECSqlStatus RegularClassSystemColumnPreparer::_GetWhereClause(ECSqlPrepareContext& ctx, NativeSqlBuilder& whereClauseBuilder, IClassMap const& classMap, ECSqlType ecsqlType, bool isPolymorphicClassExp, Utf8CP tableAlias) const
    {
    BeAssert(!classMap.GetMapStrategy().IsNotMapped() && "ClassMap::NativeSqlConverterImpl::GetWhereClause not expected to be called by unmapped class map.");

    HorizontalPartition const* horizPartition = nullptr;
    std::vector<size_t> nonVirtualPartitionIndices = classMap.GetStorageDescription().GetNonVirtualHorizontalPartitionIndices();
    if (!isPolymorphicClassExp || nonVirtualPartitionIndices.empty())
        horizPartition = &classMap.GetStorageDescription().GetRootHorizontalPartition();
    else
        {                                                                                                                                                                                           
        BeAssert(nonVirtualPartitionIndices.size() == 1 && "Check that class only maps to a single table should have been done during class name preparation");
        horizPartition = classMap.GetStorageDescription().GetHorizontalPartition(nonVirtualPartitionIndices[0]);
        }

    BeAssert(horizPartition != nullptr);
    ECDbSqlTable const& table = horizPartition->GetTable();
    if (table.GetPersistenceType() == PersistenceType::Virtual)
        return ECSqlStatus::Success; //table is virtual-> noop

    ECDbSqlColumn const* classIdCol = nullptr;
    table.TryGetECClassIdColumn(classIdCol);
    BeAssert(classIdCol != nullptr || horizPartition->GetClassIds().size() == 1 && "If table doesn't have class id column, only one class must map to it");
    if (classIdCol == nullptr || !horizPartition->NeedsClassIdFilter ())
        return ECSqlStatus::Success; //table doesn't have class id or all class ids need to be considered -> no filter needed

    if (!whereClauseBuilder.IsEmpty())
        whereClauseBuilder.Append(BooleanSqlOperator::And);

    whereClauseBuilder.Append(classIdCol->GetName().c_str(), true);
    horizPartition->AppendECClassIdFilterSql(whereClauseBuilder);
    return ECSqlStatus::Success;
    }


//****************************************************************
// EndTableSystemColumnPreparer
//****************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2015
//---------------------------------------------------------------------------------------
ECSqlStatus EndTableSystemColumnPreparer::_GetWhereClause(ECSqlPrepareContext& ctx, NativeSqlBuilder& whereClauseBuilder, IClassMap const& classMap, ECSqlType ecsqlType, bool isPolymorphicClassExp, Utf8CP tableAlias) const
    {
    RelationshipClassEndTableMap const& relClassMap = static_cast<RelationshipClassEndTableMap const&> (classMap);
    if (!whereClauseBuilder.IsEmpty())
        whereClauseBuilder.Append(" AND ");

    auto otherEndECInstanceIdColSqlSnippets = relClassMap.GetOtherEndECInstanceIdPropMap()->ToNativeSql(tableAlias, ecsqlType, false);
    BeAssert(!otherEndECInstanceIdColSqlSnippets.empty());
    bool isFirstItem = true;
    for (auto const& sqlSnippet : otherEndECInstanceIdColSqlSnippets)
        {
        if (!isFirstItem)
            whereClauseBuilder.Append(" AND ");

        whereClauseBuilder.Append(sqlSnippet).Append(" IS NOT NULL");

        isFirstItem = false;
        }

    return ECSqlStatus::Success;
    }


//****************************************************************
// SecondaryTableSystemColumnPreparer
//****************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2015
//---------------------------------------------------------------------------------------
ECSqlStatus SecondaryTableSystemColumnPreparer::_GetWhereClause(ECSqlPrepareContext& ctx, NativeSqlBuilder& whereClauseBuilder, IClassMap const& classMap, ECSqlType ecsqlType, bool isPolymorphicClassExp, Utf8CP tableAlias) const
    {
    // ECSQL_TODO  Remove this funtion. Each child ECSql statement must create ECSQL that add ECPropertyId and 
    // ECArrayIndex as they are now properties and have property may.
    // Preparing via type flag just add complication to code when the actully child statement 
    // what to use them in different way then this where statement - Affan(Added to do)

    BeAssert(whereClauseBuilder.IsEmpty());
    whereClauseBuilder.AppendParenLeft().Append(tableAlias, ECDB_COL_ECPropertyPathId).Append(" IS NULL AND ");
    whereClauseBuilder.Append(tableAlias, ECDB_COL_ECArrayIndex).Append(" IS NULL)");

    return GetDefault().GetWhereClause(ctx, whereClauseBuilder, classMap, ecsqlType, isPolymorphicClassExp, tableAlias);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

