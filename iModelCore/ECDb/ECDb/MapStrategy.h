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
    TableForThisClass = 40, //This is not inhertied to children. 
    //Options that can also be specified along  some map strategies
    SharedColumns = 0x100,             //Applied to : TablePerClass, TablePerHierarchy, SharedTableForThisClass
    ExclusivelyStoredInThisTable = 0x200, //Applied to : TablePerClass, TablePerHierarchy, SharedTableForThisClass, MapToExistingTable, TablePerClass
    Readonly = 0x400, //Applied to : MapToExistingTable
    DisableSharedColumnsForThisClass = 0x800,
    //For Relationship valid values are TablePerHierarchy, TableForThisClass, SharedTableForThisClass, DoNotMap
    //Private Strategies used by ECDb Internally
    //===========================================================================================
    NoHint = 0x0,
    Default = TableForThisClass,
    InParentTable = 0x1000,
    RelationshipTargetTable = 0x2000,
    RelationshipSourceTable = 0x4000,
    Options = SharedColumns | ExclusivelyStoredInThisTable | Readonly | DisableSharedColumnsForThisClass
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
#define STRATEGY_OPTION_SHARED_COLUMNS                      "ReuseColumns"
#define STRATEGY_OPTION_READONLY                           "Readonly"
#define STRATEGY_OPTION_EXCLUSIVELY_STORED_IN_THIS_TABLE   "ExclusivelyStoredInThisTable"
#define STRATEGY_DELIMITER                                       "|"
#define DELIMITER                                                " " STRATEGY_DELIMITER " "
#define STRATEGY_OPTION_DISABLE_SHARED_COLUMS_FOR_THIS_CLASS "DisableReuseColumnsForThisClass"

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
static inline Strategy operator | (Strategy a, Strategy b)
    {
    return static_cast<Strategy>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
static inline Strategy operator & (Strategy a, Strategy b)
    {
    return static_cast<Strategy>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }


//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
struct ECDbMapStrategy
    {
    private:
        Strategy m_strategy;
        Utf8String m_strategyStr;
        bool IsValid (Strategy strategy);
        BentleyStatus Parse (Strategy& out, Utf8CP mapStrategyHint);
        void MoveTo (ECDbMapStrategy& strategy);

        static Utf8CP ConvertToString(Strategy strategy);

    public:
        ECDbMapStrategy () { Reset (); }
        ECDbMapStrategy (ECDbMapStrategy const& strategy)  { Assign (strategy.GetStrategy());  }
        ECDbMapStrategy (ECDbMapStrategy && strategy) { strategy.MoveTo (*this); }
        explicit ECDbMapStrategy (Strategy strategy) { Assign (strategy); }
        explicit ECDbMapStrategy (uint32_t strategy) { Assign (static_cast<Strategy>(strategy)); }
        explicit ECDbMapStrategy (int32_t strategy) { Assign (static_cast<Strategy>(strategy)); }
        ~ECDbMapStrategy() {}

        //operators
        bool operator == (ECDbMapStrategy const& mapStrategy) const { return m_strategy == mapStrategy.m_strategy; }
        bool operator == (Strategy const& mapStrategy) const { return m_strategy == mapStrategy; }

        bool operator < (ECDbMapStrategy const& mapStrategy) const { return m_strategy < mapStrategy.m_strategy; }
        bool operator > (ECDbMapStrategy const& mapStrategy) const { return m_strategy > mapStrategy.m_strategy; }
        ECDbMapStrategy& operator = (ECDbMapStrategy const& mapStrategy) { Assign (mapStrategy.m_strategy); return *this; }
        ECDbMapStrategy& operator = (Strategy mapStrategy) { Assign (mapStrategy); return *this; }
        ECDbMapStrategy& operator = (ECDbMapStrategy && mapStrategy) { mapStrategy.MoveTo (*this); return *this; }

        //Setters

        BentleyStatus Assign(Strategy strategy);
        BentleyStatus AddOption(Strategy Option);

        void SetDoNotMap () { Assign (Strategy::DoNotMap); }
        void SetTableForThisClass () { Assign (Strategy::TableForThisClass); }
        void SetDoNotMapHierarchy (){ Assign (Strategy::DoNotMapHierarchy); }
        void SetTablePerHierarchy (bool ReuseColumns, bool ExclusivelyStoreInThisTable)
            {
            Assign (Strategy::TablePerHierarchy);
            if (ReuseColumns)
                AddOption (Strategy::SharedColumns);

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
                AddOption (Strategy::SharedColumns);

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
                AddOption (Strategy::SharedColumns);
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

        bool IsSharedColumns () const { return (GetStrategy () & Strategy::SharedColumns) == Strategy::SharedColumns; }
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
        bool IsDisableSharedColumnsForThisClass()const { return (GetStrategy() & Strategy::DisableSharedColumnsForThisClass) == Strategy::DisableSharedColumnsForThisClass; }
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

        Utf8CP ToString (bool outOptions = false) const { return ConvertToString (GetStrategy(outOptions)); }
        uint32_t ToUInt32 () const { return static_cast<uint32_t>(GetStrategy ()); }
        int32_t ToInt32 () const { return static_cast<int32_t>(GetStrategy ()); }

        void Reset () { Assign (Strategy::NoHint); }
        BentleyStatus Parse(ECDbMapStrategy& out, Utf8CP mapStrategyHint, Utf8CP mapStrategyHintOption);     
        static ECDbMapStrategy GetDefaultMapStrategy () 
            {
            ECDbMapStrategy defaultStrategy;
            defaultStrategy.SetToDefaultMapStrategy ();
            return defaultStrategy;
            }
    };

    END_BENTLEY_SQLITE_EC_NAMESPACE