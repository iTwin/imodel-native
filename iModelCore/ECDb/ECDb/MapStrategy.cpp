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
BentleyStatus ECDbMapStrategy::Assign(Strategy strategy)
    {
    if (!IsValid(strategy))
        return BentleyStatus::ERROR;

    m_strategy = strategy;
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMapStrategy::MoveTo(ECDbMapStrategy& strategy)
    {
    strategy.Assign(GetStrategy());
    Reset();
    }


//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMapStrategy::AddOption(Strategy Option)
    {
    if (!IsValid(Option | m_strategy))
        return BentleyStatus::ERROR;

    m_strategy = m_strategy | Option;
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbMapStrategy::IsValid(Strategy strategy)
    {
    auto isValid = ConvertToString(strategy) != nullptr;
    if (isValid == false)
        {
            BeAssert(false && "Invalid Strategy specified. See documentation for correct permutation of strategy flags.");
        }
    m_strategyStr = ConvertToString(strategy);
    return isValid;
    }


//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbMapStrategy::Parse(Strategy& out, Utf8CP mapStrategyHint)
    {
    static std::map<Utf8CP, Strategy, CompareIUtf8> s_type
        {
                {STRATEGY_DO_NOT_MAP, Strategy::DoNotMap},
                {STRATEGY_DO_NOT_MAP_HIERARCHY, Strategy::DoNotMapHierarchy},
                {STRATEGY_TABLE_PER_HIERARCHY, Strategy::TablePerHierarchy},
                {STRATEGY_TABLE_PER_CLASS, Strategy::TablePerClass},
                {STRATEGY_TABLE_FOR_THIS_CLASS, Strategy::TableForThisClass},
                {STRATEGY_SHARED_TABLE_FOR_THIS_CLASS, Strategy::SharedTableForThisClass},
                {STRATEGY_MAP_TO_EXISTING_TABLE, Strategy::MapToExistingTable},
                {STRATEGY_OPTION_SHARED_COLUMNS, Strategy::SharedColumns},
                {STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE, Strategy::ExclusivelyStoredInThisTable},
                {STRATEGY_OPTION_READONLY, Strategy::Readonly},
                {STRATEGY_OPTION_DISABLE_SHARED_COLUMS_FOR_THIS_CLASS, Strategy::DisableSharedColumnsForThisClass}

        };


    Utf8String hint = mapStrategyHint;
    hint.Trim();
    out = Strategy::NoHint;
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

    out = Strategy::NoHint;
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
    Strategy strategy;
    if (Parse(strategy, mapStrategy.c_str()) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    return out.Assign(strategy);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP ECDbMapStrategy::ConvertToString(Strategy strategy)
    {
    static std::map<Strategy, Utf8CP> s_validStratgies
        {
                {Strategy::DoNotMap,
                STRATEGY_DO_NOT_MAP},
                {Strategy::DoNotMapHierarchy,
                STRATEGY_DO_NOT_MAP_HIERARCHY},
                {Strategy::NoHint,
                STRATEGY_NO_HINT},
                {Strategy::RelationshipSourceTable,
                STRATEGY_RELATIONSHIP_SOURCE_TABLE},
                {Strategy::RelationshipTargetTable,
                STRATEGY_RELATIONSHIP_TARGET_TABLE},
                {Strategy::TableForThisClass,
                STRATEGY_TABLE_FOR_THIS_CLASS},
                {Strategy::TablePerClass,
                STRATEGY_TABLE_PER_CLASS},
                {Strategy::TablePerClass | Strategy::ExclusivelyStoredInThisTable,
                STRATEGY_TABLE_PER_CLASS DELIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE},
                {Strategy::TablePerHierarchy,
                STRATEGY_TABLE_PER_HIERARCHY},
                {Strategy::TablePerHierarchy | Strategy::ExclusivelyStoredInThisTable,
                STRATEGY_TABLE_PER_HIERARCHY DELIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE},
                {Strategy::TablePerHierarchy | Strategy::SharedColumns,
                STRATEGY_TABLE_PER_HIERARCHY DELIMITER STRATEGY_OPTION_SHARED_COLUMNS},
                {Strategy::TablePerHierarchy | Strategy::ExclusivelyStoredInThisTable | Strategy::SharedColumns,
                STRATEGY_TABLE_PER_HIERARCHY DELIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE DELIMITER STRATEGY_OPTION_SHARED_COLUMNS},
                {Strategy::MapToExistingTable,
                STRATEGY_MAP_TO_EXISTING_TABLE},
                {Strategy::MapToExistingTable | Strategy::Readonly,
                STRATEGY_MAP_TO_EXISTING_TABLE DELIMITER STRATEGY_OPTION_READONLY},
                {Strategy::InParentTable,
                STRATEGY_IN_PARENT_TABLE},
                {Strategy::InParentTable | Strategy::ExclusivelyStoredInThisTable,
                STRATEGY_IN_PARENT_TABLE DELIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE},
                {Strategy::InParentTable | Strategy::SharedColumns,
                STRATEGY_IN_PARENT_TABLE DELIMITER STRATEGY_OPTION_SHARED_COLUMNS},
                {Strategy::InParentTable | Strategy::SharedColumns | Strategy::DisableSharedColumnsForThisClass,
                STRATEGY_IN_PARENT_TABLE DELIMITER STRATEGY_OPTION_SHARED_COLUMNS DELIMITER STRATEGY_OPTION_DISABLE_SHARED_COLUMS_FOR_THIS_CLASS},

                {Strategy::InParentTable | Strategy::ExclusivelyStoredInThisTable | Strategy::SharedColumns,
                STRATEGY_IN_PARENT_TABLE DELIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE DELIMITER STRATEGY_OPTION_SHARED_COLUMNS},
                {Strategy::SharedTableForThisClass,
                STRATEGY_SHARED_TABLE_FOR_THIS_CLASS},
                {Strategy::SharedTableForThisClass | Strategy::ExclusivelyStoredInThisTable,
                STRATEGY_SHARED_TABLE_FOR_THIS_CLASS DELIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE},
                {Strategy::SharedTableForThisClass | Strategy::SharedColumns,
                STRATEGY_SHARED_TABLE_FOR_THIS_CLASS DELIMITER STRATEGY_OPTION_SHARED_COLUMNS},
                {Strategy::SharedTableForThisClass | Strategy::ExclusivelyStoredInThisTable | Strategy::SharedColumns,
                STRATEGY_SHARED_TABLE_FOR_THIS_CLASS DELIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE DELIMITER STRATEGY_OPTION_SHARED_COLUMNS},
                {Strategy::DisableSharedColumnsForThisClass,
                STRATEGY_OPTION_DISABLE_SHARED_COLUMS_FOR_THIS_CLASS},

        };

    auto itor = s_validStratgies.find(strategy);
    if (itor == s_validStratgies.end())
        return nullptr;

    return itor->second;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE


