/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/MapStrategy.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

enum class Strategy : uint32_t //0xffffffff 
    {
    //Public Strategies that user can specify in ECDbClassHint::MapStrategy
    //===========================================================================================
    DoNotMap = 0x1,
    DoNotMapHierarchy = 0x2,

    TablePerHierarchy = 0x4, // Map entire hierarchy to a single table.
    TablePerClass = 0x8,     // Map class to its own table.
    SharedTableForThisClass = 0x10,  //<TableName> must be provided
    MapToExistingTable = 0x20, //<TableName> must be provided
    TableForThisClass = 40, //This is not inhertied to children. 
    //Options that can also be specified along with some map strategies
    WithReuseColumns = 0x100,             //Applied to : TablePerClass, TablePerHierarchy, SharedTableForThisClass
    WithExclusivelyStoredInThisTable = 0x200, //Applied to : TablePerClass, TablePerHierarchy, SharedTableForThisClass, MapToExistingTable, TablePerClass
    WithReadonly = 0x400, //Applied to : MapToExistingTable

    //For Relationship valid values are TablePerHierarchy, TableForThisClass, SharedTableForThisClass, DoNotMap
    //Private Strategies used by ECDb Internally
    //===========================================================================================
    NoHint = 0x0,
    Default = TableForThisClass,
    InParentTable = 0x1000,
    RelationshipTargetTable = 0x2000,
    RelationshipSourceTable = 0x4000,
    Options = WithReuseColumns | WithExclusivelyStoredInThisTable | WithReadonly
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
#define STRATEGY_OPTION_WITH_REUSE_COLUMNS                      "WithReuseColumns"
#define STRATEGY_OPTION_WITH_READONLY                           "WithReadonly"
#define STRATEGY_OPTION_WITH_EXCLUSIVELY_STORED_IN_THIS_TABLE   "WithExclusivelyStoredInThisTable"
#define STRATEGY_DEIMITER                                       "|"
#define DEIMITER                                                " " STRATEGY_DEIMITER " "

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
                    { Strategy::TablePerClass | Strategy::WithExclusivelyStoredInThisTable, 
                        STRATEGY_TABLE_PER_CLASS DEIMITER STRATEGY_OPTION_WITH_EXCLUSIVELY_STORED_IN_THIS_TABLE },
                    { Strategy::TablePerHierarchy, 
                        STRATEGY_TABLE_PER_HIERARCHY },
                    { Strategy::TablePerHierarchy | Strategy::WithExclusivelyStoredInThisTable, 
                        STRATEGY_TABLE_PER_HIERARCHY DEIMITER STRATEGY_OPTION_WITH_EXCLUSIVELY_STORED_IN_THIS_TABLE },
                    { Strategy::TablePerHierarchy | Strategy::WithReuseColumns, 
                        STRATEGY_TABLE_PER_HIERARCHY DEIMITER STRATEGY_OPTION_WITH_REUSE_COLUMNS },
                    { Strategy::TablePerHierarchy | Strategy::WithExclusivelyStoredInThisTable | Strategy::WithReuseColumns, 
                        STRATEGY_TABLE_PER_HIERARCHY DEIMITER STRATEGY_OPTION_WITH_EXCLUSIVELY_STORED_IN_THIS_TABLE DEIMITER STRATEGY_OPTION_WITH_REUSE_COLUMNS },
                    { Strategy::MapToExistingTable, 
                        STRATEGY_MAP_TO_EXISTING_TABLE },
                    { Strategy::MapToExistingTable | Strategy::WithReadonly, 
                        STRATEGY_MAP_TO_EXISTING_TABLE DEIMITER STRATEGY_OPTION_WITH_READONLY },
                    { Strategy::InParentTable, 
                        STRATEGY_IN_PARENT_TABLE },
                    { Strategy::InParentTable | Strategy::WithExclusivelyStoredInThisTable, 
                        STRATEGY_IN_PARENT_TABLE DEIMITER STRATEGY_OPTION_WITH_EXCLUSIVELY_STORED_IN_THIS_TABLE },
                    { Strategy::InParentTable | Strategy::WithReuseColumns, 
                        STRATEGY_IN_PARENT_TABLE DEIMITER STRATEGY_OPTION_WITH_REUSE_COLUMNS },
                    { Strategy::InParentTable | Strategy::WithExclusivelyStoredInThisTable | Strategy::WithReuseColumns, 
                        STRATEGY_IN_PARENT_TABLE DEIMITER STRATEGY_OPTION_WITH_EXCLUSIVELY_STORED_IN_THIS_TABLE DEIMITER STRATEGY_OPTION_WITH_REUSE_COLUMNS },
                    { Strategy::SharedTableForThisClass, 
                        STRATEGY_SHARED_TABLE_FOR_THIS_CLASS },
                    { Strategy::SharedTableForThisClass | Strategy::WithExclusivelyStoredInThisTable, 
                        STRATEGY_SHARED_TABLE_FOR_THIS_CLASS DEIMITER STRATEGY_OPTION_WITH_EXCLUSIVELY_STORED_IN_THIS_TABLE },
                    { Strategy::SharedTableForThisClass | Strategy::WithReuseColumns, 
                        STRATEGY_SHARED_TABLE_FOR_THIS_CLASS DEIMITER STRATEGY_OPTION_WITH_REUSE_COLUMNS },
                    { Strategy::SharedTableForThisClass | Strategy::WithExclusivelyStoredInThisTable | Strategy::WithReuseColumns, 
                        STRATEGY_SHARED_TABLE_FOR_THIS_CLASS DEIMITER STRATEGY_OPTION_WITH_EXCLUSIVELY_STORED_IN_THIS_TABLE DEIMITER STRATEGY_OPTION_WITH_REUSE_COLUMNS },
            };

            auto itor = s_validStratgies.find (strategy);
            if (itor == s_validStratgies.end ())
                return nullptr;

            return itor->second;
            }
        static bool IsValid (Strategy strategy)
            {
            auto isValid = ConvertToString (strategy) != nullptr;
            if (isValid == false)
                {
                BeAssert (false && "Invalid Strategy specified. See documentation for correct permutation of strategy flags.");
                }

            return isValid;
            }
        static BentleyStatus Parse (Strategy& out, Utf8CP mapStrategyHint)
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
                    { STRATEGY_OPTION_WITH_REUSE_COLUMNS, Strategy::WithReuseColumns },
                    { STRATEGY_OPTION_WITH_EXCLUSIVELY_STORED_IN_THIS_TABLE, Strategy::WithExclusivelyStoredInThisTable },
                    { STRATEGY_OPTION_WITH_READONLY, Strategy::WithReadonly }

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
        BentleyStatus AddOption (Strategy withOption)
            {
            if (!IsValid (withOption | m_strategy))
                return BentleyStatus::ERROR;

            m_strategy = m_strategy | withOption;
            return BentleyStatus::ERROR;
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
        void SetTablePerHierarchy (bool withReuseColumns, bool withExclusivelyStoreInThisTable)
            {
            Assign (Strategy::TablePerHierarchy);
            if (withReuseColumns)
                AddOption (Strategy::WithReuseColumns);

            if (withExclusivelyStoreInThisTable)
                AddOption (Strategy::WithExclusivelyStoredInThisTable);
            }
        void SetTablePerClass (bool withExclusivelyStoreInThisTable)
            {
            Assign (Strategy::TablePerClass);
            if (withExclusivelyStoreInThisTable)
                AddOption (Strategy::WithExclusivelyStoredInThisTable);
            }
        void SetSharedTableForThisClass (bool withReuseColumns, bool withExclusivelyStoreInThisTable)
            {
            Assign (Strategy::SharedTableForThisClass);
            if (withReuseColumns)
                AddOption (Strategy::WithReuseColumns);

            if (withExclusivelyStoreInThisTable)
                AddOption (Strategy::WithExclusivelyStoredInThisTable);
            }
        void SetMapToExistingTable (bool withReadOnly)
            {
            Assign (Strategy::SharedTableForThisClass);
            if (withReadOnly)
                AddOption (Strategy::WithReadonly);
            }
        void SetInParentTable (bool withReuseColumns)
            {
            Assign (Strategy::InParentTable);
            if (withReuseColumns)
                AddOption (Strategy::WithReuseColumns);
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
        Strategy GetStrategy (bool withoutOptions = false) const 
            { 
            if (!withoutOptions)
                return m_strategy;

            return static_cast<Strategy>(ToUInt32 () & ~(static_cast<uint32_t>(Strategy::Options)));
            }

        bool IsReuseColumns () const { return (GetStrategy () & Strategy::WithReuseColumns) == Strategy::WithReuseColumns; }
        bool IsExclusiveyStoreInThisTable () const { return (GetStrategy () & Strategy::WithExclusivelyStoredInThisTable) == Strategy::WithExclusivelyStoredInThisTable; }
        bool IsReadonly () const { return (GetStrategy () & Strategy::WithReadonly) == Strategy::WithReadonly; }
        bool IsDoNotMap () const { return GetStrategy (true) == Strategy::DoNotMap; }
        bool IsNoHint () const { return GetStrategy (true) == Strategy::NoHint; }

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

        Utf8CP ToString (bool withoutOptions = false) const
            {
            return ConvertToString (GetStrategy(withoutOptions));
            }
        uint32_t ToUInt32 () const { return static_cast<uint32_t>(GetStrategy ()); }
        int32_t ToInt32 () const { return static_cast<int32_t>(GetStrategy ()); }

        void Reset ()
            {
            Assign (Strategy::NoHint);
            }
        static BentleyStatus Parse (ECDbMapStrategy& out, Utf8CP mapStrategyHint)
            {
            Strategy strategy;
            if (Parse (strategy, mapStrategyHint) != BentleyStatus::SUCCESS)
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