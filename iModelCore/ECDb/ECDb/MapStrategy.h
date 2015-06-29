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

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
enum class MapStrategy : uint32_t //0xffffffff 
    {
    NoHint = 0x0,

    //Public Strategies that user can specify in custom attribute ClassMap::MapStrategy
    //===========================================================================================
    DoNotMap = 0x1,
    DoNotMapHierarchy = 0x2,

    TablePerHierarchy = 0x4, // Map entire hierarchy to a single table.
    TablePerClass = 0x8,     // Map class to its own table.
    SharedTableForThisClass = 0x10,  //<TableName> must be provided
    MapToExistingTable = 0x20, //<TableName> must be provided
    TableForThisClass = 40, //This is not inhertied to children. 
    //Options that can also be specified along  some map strategies
    SharedColumns = 0x100,             //Applied to : TablePerClass, TablePerHierarchy, SharedTableForThisClass
    
    DisableSharedColumnsForThisClass = 0x200,
    ExclusivelyStoredInThisTable = 0x400, //Applied to : TablePerClass, TablePerHierarchy, SharedTableForThisClass, MapToExistingTable, TablePerClass
    Readonly = 0x800, //Applied to : MapToExistingTable
    //For Relationship valid values are TablePerHierarchy, TableForThisClass, SharedTableForThisClass, DoNotMap

    //Private Strategies used by ECDb Internally
    //===========================================================================================
    InParentTable = 0x1000,
    RelationshipTargetTable = 0x2000,
    RelationshipSourceTable = 0x4000,

    Default = TableForThisClass,

    Options = SharedColumns | ExclusivelyStoredInThisTable | Readonly | DisableSharedColumnsForThisClass
    };

#define STRATEGY_DO_NOT_MAP                                     "DoNotMap"
#define STRATEGY_DO_NOT_MAP_HIERARCHY                           "DoNotMapHierarchy"
#define STRATEGY_NO_HINT                                        "NoHint"
#define STRATEGY_TABLE_FOR_THIS_CLASS                           "TableForThisClass"
#define STRATEGY_TABLE_PER_CLASS                                "TablePerClass"
#define STRATEGY_TABLE_PER_HIERARCHY                            "TablePerHierarchy"
#define STRATEGY_MAP_TO_EXISTING_TABLE                          "MapToExistingTable"
#define STRATEGY_IN_PARENT_TABLE                                "InParentTable"
#define STRATEGY_SHARED_TABLE_FOR_THIS_CLASS                    "SharedTableForThisClass"
#define STRATEGY_OPTION_SHARED_COLUMNS                          "SharedColumns"
#define STRATEGY_OPTION_DISABLE_SHARED_COLUMS_FOR_THIS_CLASS    "DisableSharedColumnsForThisClass"
#define STRATEGY_OPTION_READONLY                                "Readonly"
#define STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE        "ExclusivelyStoredInThisTable"
#define STRATEGY_DELIMITER                                      "|"
#define DELIMITER                                               " " STRATEGY_DELIMITER " "

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
static inline MapStrategy operator | (MapStrategy a, MapStrategy b)
    {
    return static_cast<MapStrategy>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
static inline MapStrategy operator & (MapStrategy a, MapStrategy b)
    {
    return static_cast<MapStrategy>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }


//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
struct ECDbMapStrategy
    {
    private:
        MapStrategy m_strategy;
        Utf8String m_strategyStr;

        bool IsValid (MapStrategy strategy);
        BentleyStatus Parse (MapStrategy& out, Utf8CP mapStrategyHint);

        static Utf8CP ConvertToString(MapStrategy strategy);

    public:
        explicit ECDbMapStrategy(MapStrategy strategy) { Assign(strategy); }
        ~ECDbMapStrategy() {}

        ECDbMapStrategy(ECDbMapStrategy const& rhs) : m_strategy(rhs.m_strategy), m_strategyStr(rhs.m_strategyStr) {}
        ECDbMapStrategy(ECDbMapStrategy&& rhs) : m_strategy(std::move(rhs.m_strategy)), m_strategyStr(std::move(rhs.m_strategyStr)) {}
        ECDbMapStrategy& operator=(ECDbMapStrategy const&);
        ECDbMapStrategy& operator=(ECDbMapStrategy&&);

        //operators
        bool operator== (ECDbMapStrategy const& rhs) const { return m_strategy == rhs.m_strategy; }
        bool operator!= (ECDbMapStrategy const& rhs) const { return !(*this == rhs); }
        //bool operator== (MapStrategy const& mapStrategy) const { return m_strategy == mapStrategy; }

        bool operator< (ECDbMapStrategy const& rhs) const { return m_strategy < rhs.m_strategy; }
        bool operator>(ECDbMapStrategy const& rhs) const { return m_strategy > rhs.m_strategy; }

        //Setters

        BentleyStatus Assign(MapStrategy strategy);
        BentleyStatus AddOption(MapStrategy Option);

        void SetDoNotMap () { Assign (MapStrategy::DoNotMap); }
        void SetDoNotMapHierarchy (){ Assign (MapStrategy::DoNotMapHierarchy); }
        void SetTablePerHierarchy (bool sharedColumnsOption, bool ExclusivelyStoreInThisTable)
            {
            Assign (MapStrategy::TablePerHierarchy);
            if (sharedColumnsOption)
                AddOption (MapStrategy::SharedColumns);

            if (ExclusivelyStoreInThisTable)
                AddOption (MapStrategy::ExclusivelyStoredInThisTable);
            }
        void SetTablePerClass (bool exclusivelyStoreInThisTable)
            {
            Assign (MapStrategy::TablePerClass);
            if (exclusivelyStoreInThisTable)
                AddOption (MapStrategy::ExclusivelyStoredInThisTable);
            }
        void SetSharedTableForThisClass(bool sharedColumnsOption, bool exclusivelyStoreInThisTable)
            {
            Assign (MapStrategy::SharedTableForThisClass);
            if (sharedColumnsOption)
                AddOption (MapStrategy::SharedColumns);

            if (exclusivelyStoreInThisTable)
                AddOption (MapStrategy::ExclusivelyStoredInThisTable);
            }
        void SetMapToExistingTable (bool ReadOnly)
            {
            Assign (MapStrategy::SharedTableForThisClass);
            if (ReadOnly)
                AddOption (MapStrategy::Readonly);
            }
        void SetInParentTable(bool sharedColumnsOption)
            {
            Assign (MapStrategy::InParentTable);
            if (sharedColumnsOption)
                AddOption (MapStrategy::SharedColumns);
            }

        //Getters
        MapStrategy GetStrategy (bool outOptions = false) const 
            { 
            if (!outOptions)
                return m_strategy;

            return (MapStrategy) (((uint32_t) m_strategy) & ~((uint32_t) MapStrategy::Options));
            }

        bool IsSharedColumns () const { return (GetStrategy () & MapStrategy::SharedColumns) == MapStrategy::SharedColumns; }
        bool IsExclusiveyStoreInThisTable () const { return (GetStrategy () & MapStrategy::ExclusivelyStoredInThisTable) == MapStrategy::ExclusivelyStoredInThisTable; }
        bool IsReadonly () const { return (GetStrategy () & MapStrategy::Readonly) == MapStrategy::Readonly; }
        bool IsDoNotMap () const { return GetStrategy (true) == MapStrategy::DoNotMap; }
        bool IsNoHint () const { return GetStrategy (false) == MapStrategy::NoHint; }

        bool IsTableForThisClass () const { return GetStrategy (true) == MapStrategy::TableForThisClass; }

        bool IsDoNotMapHierarchy () const { return GetStrategy (true) == MapStrategy::DoNotMapHierarchy; }
        bool IsTablePerHierarchy () const { return GetStrategy (true) == MapStrategy::TablePerHierarchy; }
        bool IsTablePerClass () const { return GetStrategy (true) == MapStrategy::TablePerClass; }
        bool IsSharedTableForThisClass () const { return GetStrategy (true) == MapStrategy::SharedTableForThisClass; }
        bool IsMapToExistingTable () const { return GetStrategy (true) == MapStrategy::MapToExistingTable; }
        bool IsInParentTable () const { return GetStrategy (true) == MapStrategy::InParentTable; }
        bool IsRelationshipTargetTable () const { return GetStrategy (true) == MapStrategy::RelationshipTargetTable; }
        bool IsRelationshipSourceTable () const { return GetStrategy(true) == MapStrategy::RelationshipSourceTable; }
        //Helper
        bool IsMapped () const { return !(IsDoNotMap () || IsDoNotMapHierarchy ()); }
        bool IsUnmapped () const { return IsDoNotMap () || IsDoNotMapHierarchy (); }
        bool IsEndTableMapping () const{ return IsRelationshipSourceTable () || IsRelationshipTargetTable (); }
        bool IsDisableSharedColumnsForThisClass()const { return (GetStrategy() & MapStrategy::DisableSharedColumnsForThisClass) == MapStrategy::DisableSharedColumnsForThisClass; }
        bool IsLinkTableStrategy () const;

        Utf8CP ToString (bool outOptions = false) const { return ConvertToString (GetStrategy(outOptions)); }
        void Reset () { Assign (MapStrategy::NoHint); }
        BentleyStatus Parse(ECDbMapStrategy& out, Utf8CP mapStrategyHint, Utf8CP mapStrategyHintOption);     


        static ECDbMapStrategy GetDefaultMapStrategy () { return ECDbMapStrategy (MapStrategy::Default); }
    };

    END_BENTLEY_SQLITE_EC_NAMESPACE