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
    BeAssert(!classMap.GetMapStrategy().IsUnmapped() && "ClassMap::NativeSqlConverterImpl::GetWhereClause not expected to be called by unmapped class map.");

    StorageDescription const& storageInfo = classMap.GetStorageDescription();
    std::vector<HorizontalPartition> const& horizPartitions = storageInfo.GetHorizontalPartitions();
    if (horizPartitions.empty())
        {
        BeAssert(false && "No horizontal partitions for class map.");
        return ECSqlStatus::Success;
        }

    if (isPolymorphicClassExp && horizPartitions.size() > 1)
            return ctx.SetError(ECSqlStatus::InvalidECSql, "Polymorphic ECSQL %s is only supported if the ECClass and all its subclasses are mapped to the same table.",
            ExpHelper::ToString(ecsqlType));
        
    HorizontalPartition const& horizPartition = horizPartitions[0];
    if (horizPartition.HasFilter())
        {
        if (!whereClauseBuilder.IsEmpty())
            whereClauseBuilder.Append(BooleanSqlOperator::AND);

        whereClauseBuilder.AppendParenLeft();
        horizPartition.AppendECClassIdFilterSql(whereClauseBuilder);
        whereClauseBuilder.AppendParenRight();
        }

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

    auto otherEndECInstanceIdColSqlSnippets = relClassMap.GetOtherEndECInstanceIdPropMap()->ToNativeSql(tableAlias, ecsqlType);
    BeAssert(!otherEndECInstanceIdColSqlSnippets.empty());
    whereClauseBuilder.AppendParenLeft();
    bool isFirstItem = true;
    for (auto const& sqlSnippet : otherEndECInstanceIdColSqlSnippets)
        {
        if (!isFirstItem)
            whereClauseBuilder.Append(" AND ");

        whereClauseBuilder.Append(sqlSnippet).Append(" IS NOT NULL");

        isFirstItem = false;
        }

    whereClauseBuilder.AppendParenRight();

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

