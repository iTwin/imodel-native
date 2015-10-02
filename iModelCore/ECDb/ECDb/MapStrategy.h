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
        SharedTable,
        ExistingTable
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
        JoinedTableForSubclasses = 8,
        };

private:
    Strategy m_strategy;
    Options m_options;
    bool m_appliesToSubclasses;
    UserECDbMapStrategy const* m_root;

    BentleyStatus Assign(Strategy strategy, Options, bool appliesToSubclasses);
    static BentleyStatus TryParse(Strategy&, Utf8CP str);
    static BentleyStatus TryParse(Options& option, Utf8CP str);

public:
    UserECDbMapStrategy() : m_strategy(Strategy::None), m_options(Options::None), m_appliesToSubclasses(false), m_root(nullptr) {}
    ~UserECDbMapStrategy() {}

    UserECDbMapStrategy const& AssignRoot(UserECDbMapStrategy const& parent);

    bool IsValid() const;

    Strategy GetStrategy() const { return m_strategy; }
    Options GetOptions() const { return m_options; }
    bool AppliesToSubclasses() const { return m_appliesToSubclasses; }

    //! Indicates whether this strategy represents the 'unset' strategy
    bool IsUnset() const { return m_strategy == Strategy::None && m_options == Options::None && !m_appliesToSubclasses; }

    Utf8String ToString() const;

    static BentleyStatus TryParse(UserECDbMapStrategy&, ECN::ECDbClassMap::MapStrategy const& mapStrategyCustomAttribute);
    };

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
        NotMapped,
        OwnTable,
        SharedTable,
        ExistingTable,

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
        JoinedTable = 4
        };

private:
    Strategy m_strategy;
    Options m_options;
    bool m_appliesToSubclasses;
    bool m_isResolved;

public:
    ECDbMapStrategy() : m_strategy(Strategy::OwnTable), m_options(Options::None), m_appliesToSubclasses(false), m_isResolved(false) {}

    //operators
    bool operator== (ECDbMapStrategy const& rhs) const { return m_strategy == rhs.m_strategy && m_options == rhs.m_options && m_appliesToSubclasses == rhs.m_appliesToSubclasses && m_isResolved == rhs.m_isResolved; }
    bool operator!= (ECDbMapStrategy const& rhs) const { return !(*this == rhs); }

    BentleyStatus Assign(UserECDbMapStrategy const&);
    BentleyStatus Assign(Strategy, Options, bool isPolymorphic);
    BentleyStatus Assign(Strategy strategy, bool isPolymorphic) { return Assign(strategy, Options::None, isPolymorphic); }

    bool IsValid() const;

    //Getters
    Strategy GetStrategy() const { return m_strategy; }
    Options GetOptions() const { return m_options; }
    bool AppliesToSubclasses() const { return m_appliesToSubclasses; }

    bool IsResolved() const { return m_isResolved; }
    //Helper
    bool IsNotMapped() const { return m_strategy == Strategy::NotMapped; }
    bool IsForeignKeyMapping() const { return m_strategy == Strategy::ForeignKeyRelationshipInSourceTable || m_strategy == Strategy::ForeignKeyRelationshipInTargetTable; }

    Utf8String ToString() const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE