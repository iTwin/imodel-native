/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/MapStrategy.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbMapStrategy& ECDbMapStrategy::operator=(ECDbMapStrategy const& rhs)
    {
    if (this != &rhs)
        {
        m_strategy = rhs.m_strategy;
        m_strategyStr = rhs.m_strategyStr;
        }

    return *this;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbMapStrategy& ECDbMapStrategy::operator=(ECDbMapStrategy&& rhs)
    {
    if (this != &rhs)
        {
        m_strategy = std::move(rhs.m_strategy);
        m_strategyStr = std::move(rhs.m_strategyStr);
        }

    return *this;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMapStrategy::Assign(MapStrategy strategy)
    {
    if (!IsValid(strategy))
        return BentleyStatus::ERROR;

    m_strategy = strategy;
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMapStrategy::AddOption(MapStrategy Option)
    {
    if (!IsValid(Option | m_strategy))
        return BentleyStatus::ERROR;

    m_strategy = m_strategy | Option;
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbMapStrategy::IsValid(MapStrategy strategy)
    {
    m_strategyStr = ConvertToString(strategy);
    return !m_strategyStr.empty();
    }


//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbMapStrategy::IsLinkTableStrategy() const
    {
    MapStrategy mapStrategy = GetStrategy(true);
    // RelationshipClassMappingRule: not sure why all of these are mapping to link tables
    return (mapStrategy == MapStrategy::TableForThisClass ||
            mapStrategy == MapStrategy::TablePerHierarchy ||
            mapStrategy == MapStrategy::InParentTable ||
            mapStrategy == MapStrategy::TablePerClass ||
            mapStrategy == MapStrategy::SharedTableForThisClass);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMapStrategy::Parse(MapStrategy& out, Utf8CP mapStrategyHint)
    {
    static std::map<Utf8CP, MapStrategy, CompareIUtf8> s_type
        {
                {STRATEGY_DO_NOT_MAP, MapStrategy::DoNotMap},
                {STRATEGY_DO_NOT_MAP_HIERARCHY, MapStrategy::DoNotMapHierarchy},
                {STRATEGY_TABLE_PER_HIERARCHY, MapStrategy::TablePerHierarchy},
                {STRATEGY_TABLE_PER_CLASS, MapStrategy::TablePerClass},
                {STRATEGY_TABLE_FOR_THIS_CLASS, MapStrategy::TableForThisClass},
                {STRATEGY_SHARED_TABLE_FOR_THIS_CLASS, MapStrategy::SharedTableForThisClass},
                {STRATEGY_MAP_TO_EXISTING_TABLE, MapStrategy::MapToExistingTable},
                {STRATEGY_OPTION_SHARED_COLUMNS, MapStrategy::SharedColumns},
                {STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE, MapStrategy::ExclusivelyStoredInThisTable},
                {STRATEGY_OPTION_READONLY, MapStrategy::Readonly},
                {STRATEGY_OPTION_DISABLE_SHARED_COLUMS_FOR_THIS_CLASS, MapStrategy::DisableSharedColumnsForThisClass}

        };


    Utf8String hint = mapStrategyHint;
    hint.Trim();
    out = MapStrategy::NoHint;
    size_t s = 0;
    auto n = hint.find(STRATEGY_DELIMITER, s);
    if (n == Utf8String::npos)
        {
        auto itor = s_type.find(hint.c_str());
        if (itor == s_type.end())
            return BentleyStatus::ERROR;

        out = itor->second;
        }
    else
        {
        do
            {
            Utf8String part = hint.substr(s, n - s);
            part.Trim();
            if (!part.empty())
                {
                auto itor = s_type.find(part.c_str());
                if (itor == s_type.end())
                    return BentleyStatus::ERROR;

                out = out | itor->second;
                }

            s = n + 1;
            } while ((n = hint.find(STRATEGY_DELIMITER, s)) != Utf8String::npos);
        }

    n = hint.size();
    if ((n - s) > 0)
        {
        Utf8String part = hint.substr(s, n - s);
        part.Trim();
        if (!part.empty())
            {
            auto itor = s_type.find(part.c_str());
            if (itor == s_type.end())
                return BentleyStatus::ERROR;

            out = out | itor->second;
            }
        }

    if (IsValid(out))
        {
        return BentleyStatus::SUCCESS;
        }

    out = MapStrategy::NoHint;
    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMapStrategy::Parse(ECDbMapStrategy& out, Utf8CP mapStrategyHint, Utf8CP mapStrategyHintOption)
    {
    Utf8String mapStrategy;
    if (mapStrategyHint == nullptr)
        {
        mapStrategy = mapStrategyHintOption;
        }
    else if (mapStrategyHintOption == nullptr)
        {
        mapStrategy = mapStrategyHint;
        }
    else
        {
        mapStrategy = mapStrategyHint;
        mapStrategy.append(" | ");
        mapStrategy.append(mapStrategyHintOption);
        }
    MapStrategy strategy;
    if (Parse(strategy, mapStrategy.c_str()) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    return out.Assign(strategy);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP ECDbMapStrategy::ConvertToString(MapStrategy strategy)
    {
    static std::map<MapStrategy, Utf8CP> s_validStratgies
        {
                {MapStrategy::DoNotMap,
                STRATEGY_DO_NOT_MAP},
                {MapStrategy::DoNotMapHierarchy,
                STRATEGY_DO_NOT_MAP_HIERARCHY},
                {MapStrategy::NoHint,
                STRATEGY_NO_HINT},
                {MapStrategy::RelationshipSourceTable,
                STRATEGY_RELATIONSHIP_SOURCE_TABLE},
                {MapStrategy::RelationshipTargetTable,
                STRATEGY_RELATIONSHIP_TARGET_TABLE},
                {MapStrategy::TableForThisClass,
                STRATEGY_TABLE_FOR_THIS_CLASS},
                {MapStrategy::TablePerClass,
                STRATEGY_TABLE_PER_CLASS},
                {MapStrategy::TablePerClass | MapStrategy::ExclusivelyStoredInThisTable,
                STRATEGY_TABLE_PER_CLASS DELIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE},
                {MapStrategy::TablePerHierarchy,
                STRATEGY_TABLE_PER_HIERARCHY},
                {MapStrategy::TablePerHierarchy | MapStrategy::ExclusivelyStoredInThisTable,
                STRATEGY_TABLE_PER_HIERARCHY DELIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE},
                {MapStrategy::TablePerHierarchy | MapStrategy::SharedColumns,
                STRATEGY_TABLE_PER_HIERARCHY DELIMITER STRATEGY_OPTION_SHARED_COLUMNS},
                {MapStrategy::TablePerHierarchy | MapStrategy::ExclusivelyStoredInThisTable | MapStrategy::SharedColumns,
                STRATEGY_TABLE_PER_HIERARCHY DELIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE DELIMITER STRATEGY_OPTION_SHARED_COLUMNS},
                {MapStrategy::MapToExistingTable,
                STRATEGY_MAP_TO_EXISTING_TABLE},
                {MapStrategy::MapToExistingTable | MapStrategy::Readonly,
                STRATEGY_MAP_TO_EXISTING_TABLE DELIMITER STRATEGY_OPTION_READONLY},
                {MapStrategy::InParentTable,
                STRATEGY_IN_PARENT_TABLE},
                {MapStrategy::InParentTable | MapStrategy::ExclusivelyStoredInThisTable,
                STRATEGY_IN_PARENT_TABLE DELIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE},
                {MapStrategy::InParentTable | MapStrategy::SharedColumns,
                STRATEGY_IN_PARENT_TABLE DELIMITER STRATEGY_OPTION_SHARED_COLUMNS},
                {MapStrategy::InParentTable | MapStrategy::SharedColumns | MapStrategy::DisableSharedColumnsForThisClass,
                STRATEGY_IN_PARENT_TABLE DELIMITER STRATEGY_OPTION_SHARED_COLUMNS DELIMITER STRATEGY_OPTION_DISABLE_SHARED_COLUMS_FOR_THIS_CLASS},

                {MapStrategy::InParentTable | MapStrategy::ExclusivelyStoredInThisTable | MapStrategy::SharedColumns,
                STRATEGY_IN_PARENT_TABLE DELIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE DELIMITER STRATEGY_OPTION_SHARED_COLUMNS},
                {MapStrategy::SharedTableForThisClass,
                STRATEGY_SHARED_TABLE_FOR_THIS_CLASS},
                {MapStrategy::SharedTableForThisClass | MapStrategy::ExclusivelyStoredInThisTable,
                STRATEGY_SHARED_TABLE_FOR_THIS_CLASS DELIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE},
                {MapStrategy::SharedTableForThisClass | MapStrategy::SharedColumns,
                STRATEGY_SHARED_TABLE_FOR_THIS_CLASS DELIMITER STRATEGY_OPTION_SHARED_COLUMNS},
                {MapStrategy::SharedTableForThisClass | MapStrategy::ExclusivelyStoredInThisTable | MapStrategy::SharedColumns,
                STRATEGY_SHARED_TABLE_FOR_THIS_CLASS DELIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE DELIMITER STRATEGY_OPTION_SHARED_COLUMNS},
                {MapStrategy::DisableSharedColumnsForThisClass,
                STRATEGY_OPTION_DISABLE_SHARED_COLUMS_FOR_THIS_CLASS},

        };

    auto itor = s_validStratgies.find(strategy);
    if (itor == s_validStratgies.end())
        return nullptr;

    return itor->second;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE


