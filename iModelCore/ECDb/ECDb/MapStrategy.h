/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/MapStrategy.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//_BENTLEY_INTERNAL_ONLY_
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

enum class Strategy : uint32_t //0xffffffff 
    {
    //Public Strategies that user can specify in custom attribute ClassMap::MapStrategy
    //===========================================================================================
    DoNotMap = 0x1,
    DoNotMapHierarchy = 0x2,

    TablePerHierarchy = 0x4, // Map entire hierarchy to a single table.
    TablePerClass = 0x8,     // Map class to its own table.
    SharedTableForThisClass = 0x10,  //<TableName> must be provided
    MapToExistingTable = 0x20, //<TableName> must be provided
    TableForThisClass = 0x40, //This is not inhertied to children. 
    //Options that can also be specified along  some map strategies
    ReuseColumns = 0x100,             //Applied to : TablePerClass, TablePerHierarchy, SharedTableForThisClass
    ExclusivelyStoredInThisTable = 0x200, //Applied to : TablePerClass, TablePerHierarchy, SharedTableForThisClass, MapToExistingTable, TablePerClass
    Readonly = 0x400, //Applied to : MapToExistingTable
    DisableReuseColumnsForThisClass = 0x800,
    //For Relationship valid values are TablePerHierarchy, TableForThisClass, SharedTableForThisClass, DoNotMap
    //Private Strategies used by ECDb Internally
    //===========================================================================================
    NoHint = 0x0,
    Default = TableForThisClass,
    InParentTable = 0x1000,
    RelationshipTargetTable = 0x2000,
    RelationshipSourceTable = 0x4000,
    Options = ReuseColumns | ExclusivelyStoredInThisTable | Readonly | DisableReuseColumnsForThisClass
    };

#define STRATEGY_DO_NOT_MAP                                     "DoNotMap"
#define STRATEGY_DO_NOT_MAP_HIERARCHY                           "DoNotMapHierarchy"
#define STRATEGY_NO_HINT                                        "NoHint"
#define STRATEGY_RELATIONSHIP_SOURCE_TABLE                      "RelationshipSourceTable"
#define STRATEGY_RELATIONSHIP_TARGET_TABLE                      "RelationshipTargetTable"
#define STRATEGY_TABLE_FOR_THIS_CLASS                           "TableForThisClass"
#define STRATEGY_TABLE_PER_CLASS                                "TablePerClass"
#define STRATEGY_TABLE_PER_HIERARCHY                            "TablePerHierarchy"
#define STRATEGY_MAP_TO_EXISTING_TABLE                          "MapToExistingTable"
#define STRATEGY_IN_PARENT_TABLE                                "InParentTable"
#define STRATEGY_SHARED_TABLE_FOR_THIS_CLASS                    "SharedTableForThisClass"
#define STRATEGY_OPTION_REUSE_COLUMNS                      "ReuseColumns"
#define STRATEGY_OPTION_READONLY                           "Readonly"
#define STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE   "ExclusivelyStoredInThisTable"
#define STRATEGY_DEIMITER                                       "|"
#define DEIMITER                                                " " STRATEGY_DEIMITER " "
#define STRATEGY_OPTION_DISABLE_REUSE_COLUMS_FOR_THIS_CLASS "DisableReuseColumnsForThisClass"
static inline Strategy operator | (Strategy a, Strategy b)
    {
    return static_cast<Strategy>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

static inline Strategy operator & (Strategy a, Strategy b)
    {
    return static_cast<Strategy>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }


struct ECDbMapStrategy
    {
    private:
        Strategy m_strategy;
        Utf8String m_strategyStr;
        static Utf8CP ConvertToString (Strategy strategy)
            {
            static std::map<Strategy, Utf8CP> s_validStratgies
                {
                    { Strategy::DoNotMap, 
                        STRATEGY_DO_NOT_MAP },
                    { Strategy::DoNotMapHierarchy, 
                        STRATEGY_DO_NOT_MAP_HIERARCHY },
                    { Strategy::NoHint, 
                        STRATEGY_NO_HINT },
                    { Strategy::RelationshipSourceTable, 
                        STRATEGY_RELATIONSHIP_SOURCE_TABLE },
                    { Strategy::RelationshipTargetTable, 
                        STRATEGY_RELATIONSHIP_TARGET_TABLE },
                    { Strategy::TableForThisClass, 
                        STRATEGY_TABLE_FOR_THIS_CLASS },
                    { Strategy::TablePerClass, 
                        STRATEGY_TABLE_PER_CLASS },
                    { Strategy::TablePerClass | Strategy::ExclusivelyStoredInThisTable, 
                        STRATEGY_TABLE_PER_CLASS DEIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE },
                    { Strategy::TablePerHierarchy, 
                        STRATEGY_TABLE_PER_HIERARCHY },
                    { Strategy::TablePerHierarchy | Strategy::ExclusivelyStoredInThisTable, 
                        STRATEGY_TABLE_PER_HIERARCHY DEIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE },
                    { Strategy::TablePerHierarchy | Strategy::ReuseColumns, 
                        STRATEGY_TABLE_PER_HIERARCHY DEIMITER STRATEGY_OPTION_REUSE_COLUMNS },
                    { Strategy::TablePerHierarchy | Strategy::ExclusivelyStoredInThisTable | Strategy::ReuseColumns, 
                        STRATEGY_TABLE_PER_HIERARCHY DEIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE DEIMITER STRATEGY_OPTION_REUSE_COLUMNS },
                    { Strategy::MapToExistingTable, 
                        STRATEGY_MAP_TO_EXISTING_TABLE },
                    { Strategy::MapToExistingTable | Strategy::Readonly, 
                        STRATEGY_MAP_TO_EXISTING_TABLE DEIMITER STRATEGY_OPTION_READONLY },
                    { Strategy::InParentTable, 
                        STRATEGY_IN_PARENT_TABLE },
                    { Strategy::InParentTable | Strategy::ExclusivelyStoredInThisTable, 
                        STRATEGY_IN_PARENT_TABLE DEIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE },
                    { Strategy::InParentTable | Strategy::ReuseColumns, 
                        STRATEGY_IN_PARENT_TABLE DEIMITER STRATEGY_OPTION_REUSE_COLUMNS },
                        { Strategy::InParentTable | Strategy::ReuseColumns | Strategy::DisableReuseColumnsForThisClass,
                        STRATEGY_IN_PARENT_TABLE DEIMITER STRATEGY_OPTION_REUSE_COLUMNS DEIMITER STRATEGY_OPTION_DISABLE_REUSE_COLUMS_FOR_THIS_CLASS },

                    { Strategy::InParentTable | Strategy::ExclusivelyStoredInThisTable | Strategy::ReuseColumns, 
                        STRATEGY_IN_PARENT_TABLE DEIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE DEIMITER STRATEGY_OPTION_REUSE_COLUMNS },
                    { Strategy::SharedTableForThisClass, 
                        STRATEGY_SHARED_TABLE_FOR_THIS_CLASS },
                    { Strategy::SharedTableForThisClass | Strategy::ExclusivelyStoredInThisTable, 
                        STRATEGY_SHARED_TABLE_FOR_THIS_CLASS DEIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE },
                    { Strategy::SharedTableForThisClass | Strategy::ReuseColumns, 
                        STRATEGY_SHARED_TABLE_FOR_THIS_CLASS DEIMITER STRATEGY_OPTION_REUSE_COLUMNS },
                    { Strategy::SharedTableForThisClass | Strategy::ExclusivelyStoredInThisTable | Strategy::ReuseColumns, 
                        STRATEGY_SHARED_TABLE_FOR_THIS_CLASS DEIMITER STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE DEIMITER STRATEGY_OPTION_REUSE_COLUMNS },
                    { Strategy::DisableReuseColumnsForThisClass ,
                    STRATEGY_OPTION_DISABLE_REUSE_COLUMS_FOR_THIS_CLASS },

            };

            auto itor = s_validStratgies.find (strategy);
            if (itor == s_validStratgies.end ())
                return nullptr;

            return itor->second;
            }
         bool IsValid (Strategy strategy)
            {
            auto isValid = ConvertToString (strategy) != nullptr;
            if (isValid == false)
                {
                BeAssert (false && "Invalid Strategy specified. See documentation for correct permutation of strategy flags.");
                }
            m_strategyStr = ConvertToString(strategy);
            return isValid;
            }
        BentleyStatus Parse (Strategy& out, Utf8CP mapStrategyHint)
            {
            static std::map<Utf8CP, Strategy, CompareIUtf8> s_type
                {
                    { STRATEGY_DO_NOT_MAP, Strategy::DoNotMap },
                    { STRATEGY_DO_NOT_MAP_HIERARCHY, Strategy::DoNotMapHierarchy },
                    { STRATEGY_TABLE_PER_HIERARCHY, Strategy::TablePerHierarchy },
                    { STRATEGY_TABLE_PER_CLASS, Strategy::TablePerClass },
                    { STRATEGY_TABLE_FOR_THIS_CLASS, Strategy::TableForThisClass },
                    { STRATEGY_SHARED_TABLE_FOR_THIS_CLASS, Strategy::SharedTableForThisClass },
                    { STRATEGY_MAP_TO_EXISTING_TABLE, Strategy::MapToExistingTable },
                    { STRATEGY_OPTION_REUSE_COLUMNS, Strategy::ReuseColumns },
                    { STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE, Strategy::ExclusivelyStoredInThisTable },
                    { STRATEGY_OPTION_READONLY, Strategy::Readonly },
                    { STRATEGY_OPTION_DISABLE_REUSE_COLUMS_FOR_THIS_CLASS, Strategy::DisableReuseColumnsForThisClass }

            };

     
            Utf8String hint = mapStrategyHint;
            hint.Trim ();
            out = Strategy::NoHint;
            size_t s = 0;
            auto n = hint.find (STRATEGY_DEIMITER, s);
            if (n == Utf8String::npos)
                {
                auto itor = s_type.find (hint.c_str ());
                if (itor == s_type.end ())
                    return BentleyStatus::ERROR;

                out = itor->second;
                }
            else
                {
                do
                    {
                    Utf8String part = hint.substr (s, n - s);
                    part.Trim ();
                    if (!part.empty ())
                        {
                        auto itor = s_type.find (part.c_str ());
                        if (itor == s_type.end ())
                            return BentleyStatus::ERROR;

                        out = out | itor->second;
                        }

                    s = n + 1;
                    } while ((n = hint.find (STRATEGY_DEIMITER, s)) != Utf8String::npos);
                }

            n = hint.size ();
            if ((n - s) > 0)
                {
                Utf8String part = hint.substr (s, n - s);
                part.Trim ();
                if (!part.empty ())
                    {
                    auto itor = s_type.find (part.c_str ());
                    if (itor == s_type.end ())
                        return BentleyStatus::ERROR;

                    out = out | itor->second;
                    }
                }

            if (IsValid (out))
                {
                return BentleyStatus::SUCCESS;
                }

            out = Strategy::NoHint;
            return BentleyStatus::ERROR;
            }
        void MoveTo (ECDbMapStrategy& strategy)
            {
            strategy.Assign (GetStrategy ());
            Reset ();
            }
    public:
        BentleyStatus Assign (Strategy strategy)
            {
            if (!IsValid (strategy))
                return BentleyStatus::ERROR;

            m_strategy = strategy;
            return BentleyStatus::SUCCESS;
            }
        BentleyStatus AddOption (Strategy Option)
            {
            if (!IsValid (Option | m_strategy))
                return BentleyStatus::ERROR;

            m_strategy = m_strategy | Option;
            return BentleyStatus::SUCCESS;
            }

        ECDbMapStrategy ()
            {
            Reset ();
            }
        ~ECDbMapStrategy (){}
        ECDbMapStrategy (ECDbMapStrategy const& strategy)
            {
            Assign (strategy.GetStrategy());
            }
        ECDbMapStrategy (ECDbMapStrategy && strategy)
            {
            strategy.MoveTo (*this);
            }
        explicit ECDbMapStrategy (Strategy strategy)
            {
            Assign (strategy);
            }
        explicit ECDbMapStrategy (uint32_t strategy)
            {
            Assign (static_cast<Strategy>(strategy));
            }
        explicit ECDbMapStrategy (int32_t strategy)
            {
            Assign (static_cast<Strategy>(strategy));
            }

        //operators
        bool operator == (ECDbMapStrategy const& mapStrategy) const { return m_strategy == mapStrategy.m_strategy; }
        bool operator == (Strategy const& mapStrategy) const { return m_strategy == mapStrategy; }

        bool operator < (ECDbMapStrategy const& mapStrategy) const { return m_strategy < mapStrategy.m_strategy; }
        bool operator > (ECDbMapStrategy const& mapStrategy) const { return m_strategy > mapStrategy.m_strategy; }
        ECDbMapStrategy& operator = (ECDbMapStrategy const& mapStrategy) { Assign (mapStrategy.m_strategy); return *this; }
        ECDbMapStrategy& operator = (Strategy mapStrategy) { Assign (mapStrategy); return *this; }
        ECDbMapStrategy& operator = (ECDbMapStrategy && mapStrategy) { mapStrategy.MoveTo (*this); return *this; }
        //Setters

        void SetDoNotMap () { Assign (Strategy::DoNotMap); }
        void SetTableForThisClass () { Assign (Strategy::TableForThisClass); }
        void SetDoNotMapHierarchy (){ Assign (Strategy::DoNotMapHierarchy); }
        void SetTablePerHierarchy (bool ReuseColumns, bool ExclusivelyStoreInThisTable)
            {
            Assign (Strategy::TablePerHierarchy);
            if (ReuseColumns)
                AddOption (Strategy::ReuseColumns);

            if (ExclusivelyStoreInThisTable)
                AddOption (Strategy::ExclusivelyStoredInThisTable);
            }
        void SetTablePerClass (bool ExclusivelyStoreInThisTable)
            {
            Assign (Strategy::TablePerClass);
            if (ExclusivelyStoreInThisTable)
                AddOption (Strategy::ExclusivelyStoredInThisTable);
            }
        void SetSharedTableForThisClass (bool ReuseColumns, bool ExclusivelyStoreInThisTable)
            {
            Assign (Strategy::SharedTableForThisClass);
            if (ReuseColumns)
                AddOption (Strategy::ReuseColumns);

            if (ExclusivelyStoreInThisTable)
                AddOption (Strategy::ExclusivelyStoredInThisTable);
            }
        void SetMapToExistingTable (bool ReadOnly)
            {
            Assign (Strategy::SharedTableForThisClass);
            if (ReadOnly)
                AddOption (Strategy::Readonly);
            }
        void SetInParentTable (bool ReuseColumns)
            {
            Assign (Strategy::InParentTable);
            if (ReuseColumns)
                AddOption (Strategy::ReuseColumns);
            }
        void SetRelationshipTargetTable ()
            {
            Assign (Strategy::RelationshipTargetTable);
            }
        void SetRelationshipSourceTable ()
            {
            Assign (Strategy::RelationshipSourceTable);
            }
        void SetToDefaultMapStrategy () { Assign (Strategy::Default); }
        //Getters
        Strategy GetStrategy (bool outOptions = false) const 
            { 
            if (!outOptions)
                return m_strategy;

            return static_cast<Strategy>(ToUInt32 () & ~(static_cast<uint32_t>(Strategy::Options)));
            }

        bool IsReuseColumns () const { return (GetStrategy () & Strategy::ReuseColumns) == Strategy::ReuseColumns; }
        bool IsExclusiveyStoreInThisTable () const { return (GetStrategy () & Strategy::ExclusivelyStoredInThisTable) == Strategy::ExclusivelyStoredInThisTable; }
        bool IsReadonly () const { return (GetStrategy () & Strategy::Readonly) == Strategy::Readonly; }
        bool IsDoNotMap () const { return GetStrategy (true) == Strategy::DoNotMap; }
        bool IsNoHint () const { return GetStrategy (false) == Strategy::NoHint; }

        bool IsTableForThisClass () const { return GetStrategy (true) == Strategy::TableForThisClass; }

        bool IsDoNotMapHierarchy () const { return GetStrategy (true) == Strategy::DoNotMapHierarchy; }
        bool IsTablePerHierarchy () const { return GetStrategy (true) == Strategy::TablePerHierarchy; }
        bool IsTablePerClass () const { return GetStrategy (true) == Strategy::TablePerClass; }
        bool IsSharedTableForThisClass () const { return GetStrategy (true) == Strategy::SharedTableForThisClass; }
        bool IsMapToExistingTable () const { return GetStrategy (true) == Strategy::MapToExistingTable; }
        bool IsInParentTable () const { return GetStrategy (true) == Strategy::InParentTable; }
        bool IsRelationshipTargetTable () const { return GetStrategy (true) == Strategy::RelationshipTargetTable; }
        bool IsRelationshipSourceTable () const { return GetStrategy(true) == Strategy::RelationshipSourceTable; }
        //Helper
        bool IsMapped () const { return !(IsDoNotMap () || IsDoNotMapHierarchy ()); }
        bool IsUnmapped () const { return IsDoNotMap () || IsDoNotMapHierarchy (); }
        bool IsEndTableMapping () const{ return IsRelationshipSourceTable () || IsRelationshipTargetTable (); }
        bool IsDisableReuseColumnsForThisClass()const { return (GetStrategy() & Strategy::DisableReuseColumnsForThisClass) == Strategy::DisableReuseColumnsForThisClass; }
        bool IsLinkTableStrategy () const
            {
            auto mapStrategy = GetStrategy (true);
            // RelationshipClassMappingRule: not sure why all of these are mapping to link tables
            return (mapStrategy == Strategy::TableForThisClass ||
                mapStrategy == Strategy::TablePerHierarchy ||
                mapStrategy == Strategy::InParentTable ||
                mapStrategy == Strategy::TablePerClass ||
                mapStrategy == Strategy::SharedTableForThisClass);
            }

        Utf8CP ToString (bool outOptions = false) const
            {
            return ConvertToString (GetStrategy(outOptions));
            }
        uint32_t ToUInt32 () const { return static_cast<uint32_t>(GetStrategy ()); }
        int32_t ToInt32 () const { return static_cast<int32_t>(GetStrategy ()); }

        void Reset ()
            {
            Assign (Strategy::NoHint);
            }
        BentleyStatus Parse(ECDbMapStrategy& out, Utf8CP mapStrategyHint, Utf8CP mapStrategyHintOption)
            {
            Utf8String mapStrategy;
            if (mapStrategyHint == nullptr)
                {
                mapStrategy = mapStrategyHintOption;
                }
            else if (mapStrategyHintOption==nullptr)
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

            return out.Assign (strategy);
            }     
        static ECDbMapStrategy GetDefaultMapStrategy () 
            {
            ECDbMapStrategy defaultStrategy;
            defaultStrategy.SetToDefaultMapStrategy ();
            return defaultStrategy;
            }
    };

    END_BENTLEY_SQLITE_EC_NAMESPACE