/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/MapStrategy.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//_BENTLEY_INTERNAL_ONLY_
#include "ECDbInternalTypes.h"

#define USERMAPSTRATEGY_OPTIONS_SHAREDCOLUMNS "SharedColumns"
#define USERMAPSTRATEGY_OPTIONS_SHAREDCOLUMNSFORSUBCLASSES "SharedColumnsForSubclasses"
#define USERMAPSTRATEGY_OPTIONS_DISABLESHAREDCOLUMNS "DisableSharedColumns"
#define USERMAPSTRATEGY_OPTIONS_JOINEDTABLEPERDIRECTSUBCLASS "JoinedTablePerDirectSubclass"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
// @bsiclass                                 Krischan.Eberle               07/2015
//+===============+===============+===============+===============+===============+=====
struct UserECDbMapStrategy
    {
public:
    //---------------------------------------------------------------------------------
    // @bsienum                                 Krischan.Eberle                06/2015
    //+---------------+---------------+---------------+---------------+---------------+------
    enum class Strategy
        {
        None,
        NotMapped,
        OwnTable,
        TablePerHierarchy,
        ExistingTable,
        SharedTable
        };

    //---------------------------------------------------------------------------------
    // @bsienum                                 Krischan.Eberle                06/2015
    //+---------------+---------------+---------------+---------------+---------------+------
    enum class Options
        {
        None = 0,
        SharedColumns = 1,
        SharedColumnsForSubclasses = 2,
        DisableSharedColumns = 4,
        JoinedTablePerDirectSubclass = 8
        };

private:
    Strategy m_strategy;
    Options m_options;
    int m_minimumSharedColumnCount;

    BentleyStatus Assign(Strategy strategy, Options, int minimumSharedColumnCount);
    static BentleyStatus TryParse(Strategy&, Utf8CP str);
    static BentleyStatus TryParse(Options& option, Utf8CP str);

public:
    UserECDbMapStrategy() : m_strategy(Strategy::None), m_options(Options::None), m_minimumSharedColumnCount(ECN::ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT) {}
    ~UserECDbMapStrategy() {}

    bool IsValid() const;

    Strategy GetStrategy() const { return m_strategy; }
    Options GetOptions() const { return m_options; }
    int GetMinimumSharedColumnCount() const { return m_minimumSharedColumnCount; }

    //! Indicates whether this strategy represents the 'unset' strategy
    bool IsUnset() const { return m_strategy == Strategy::None && m_options == Options::None && m_minimumSharedColumnCount == ECN::ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT; }

    Utf8String ToString() const;
    static Utf8String ToString(Options);
    static BentleyStatus TryParse(UserECDbMapStrategy&, ECN::ECDbClassMap::MapStrategy const& mapStrategyCustomAttribute);
    };

ENUM_IS_FLAGS(UserECDbMapStrategy::Options);

//======================================================================================
// @bsiclass                                 Affan.Khan                02/2015
//+===============+===============+===============+===============+===============+=====
struct ECDbMapStrategy
    {
public:
    //---------------------------------------------------------------------------------
    // @bsienum                                 Krischan.Eberle                06/2015
    //+---------------+---------------+---------------+---------------+---------------+------
    enum class Strategy
        {
        NotMapped = 0,
        OwnTable = 1,
        TablePerHierarchy = 2,
        ExistingTable = 3,
        SharedTable = 4,

        ForeignKeyRelationshipInTargetTable = 100,
        ForeignKeyRelationshipInSourceTable = 101
        };

    //---------------------------------------------------------------------------------
    // @bsienum                                 Krischan.Eberle                06/2015
    //+---------------+---------------+---------------+---------------+---------------+------
    enum class Options
        {
        None = 0,
        SharedColumns = 1,
        ParentOfJoinedTable = 2,
        JoinedTable = 4,
        };

private:
    Strategy m_strategy;
    Options m_options;
    int m_minimumSharedColumnCount;
    bool m_isResolved;

public:
    ECDbMapStrategy() : m_strategy(Strategy::OwnTable), m_options(Options::None), m_minimumSharedColumnCount(ECN::ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT), m_isResolved(false) {}

    //operators
    bool operator== (ECDbMapStrategy const& rhs) const { return m_strategy == rhs.m_strategy && m_options == rhs.m_options && m_isResolved == rhs.m_isResolved; }
    bool operator!= (ECDbMapStrategy const& rhs) const { return !(*this == rhs); }

    BentleyStatus Assign(UserECDbMapStrategy const&);
    BentleyStatus Assign(Strategy, Options, int minimumSharedColumnCount);
    BentleyStatus Assign(Strategy strategy) { return Assign(strategy, Options::None, ECN::ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT); }

    bool IsValid() const;

    //Getters
    Strategy GetStrategy() const { return m_strategy; }
    Options GetOptions() const { return m_options; }
    //!@returns Minimum shared column count or ECN::ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT if unset
    int GetMinimumSharedColumnCount() const { return m_minimumSharedColumnCount; }

    bool IsResolved() const { return m_isResolved; }
    //Helper
    bool IsNotMapped() const { return m_strategy == Strategy::NotMapped; }
    bool IsForeignKeyMapping() const { return m_strategy == Strategy::ForeignKeyRelationshipInSourceTable || m_strategy == Strategy::ForeignKeyRelationshipInTargetTable; }
 
    Utf8String ToString() const;
    static Utf8CP ToString(Strategy);
    };

ENUM_IS_FLAGS(ECDbMapStrategy::Options);

END_BENTLEY_SQLITE_EC_NAMESPACE