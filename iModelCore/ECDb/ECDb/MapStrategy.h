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
// @bsimethod                                 Krischan.Eberle               07/2015
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
    enum class Option
        {
        None = 0,
        Readonly = 1,
        SharedColumns = 2,
        SharedColumnsForSubclasses = 4,
        DisableSharedColumns = 8
        };

private:
    Strategy m_strategy;
    Option m_option;
    bool m_isPolymorphic;
    UserECDbMapStrategy const* m_root;

    BentleyStatus Assign(Strategy strategy, Option option, bool isPolymorphic);
    static BentleyStatus TryParse(Strategy&, Utf8CP str);
    static BentleyStatus TryParse(Option& option, Utf8CP str);

public:
    UserECDbMapStrategy() : m_strategy(Strategy::None), m_option(Option::None), m_isPolymorphic(false), m_root(nullptr) {}
    ~UserECDbMapStrategy() {}

    UserECDbMapStrategy const& AssignRoot(UserECDbMapStrategy const& parent);

    bool IsValid() const;

    Strategy GetStrategy() const { return m_strategy; }
    Option GetOption() const { return m_option; }
    bool IsPolymorphic() const { return m_isPolymorphic; }

    //! Indicates whether this strategy represents the 'unset' strategy
    bool IsUnset() const { return m_strategy == Strategy::None && m_option == Option::None && !m_isPolymorphic; }

    Utf8String ToString() const;

    static BentleyStatus TryParse(UserECDbMapStrategy&, ECN::ECDbClassMap::MapStrategy const& mapStrategyCustomAttribute);
    };

//======================================================================================
// @bsimethod                                 Affan.Khan                02/2015
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
    enum class Option
        {
        None = 0,
        Readonly = 1,
        SharedColumns = 2
        };

private:
    Strategy m_strategy;
    Option m_option;
    bool m_isPolymorphic;
    bool m_isResolved;

public:
    ECDbMapStrategy() : m_strategy(Strategy::OwnTable), m_option(Option::None), m_isPolymorphic(false), m_isResolved(false) {}

    //operators
    bool operator== (ECDbMapStrategy const& rhs) const { return m_strategy == rhs.m_strategy && m_option == rhs.m_option && m_isPolymorphic == rhs.m_isPolymorphic && m_isResolved == rhs.m_isResolved; }
    bool operator!= (ECDbMapStrategy const& rhs) const { return !(*this == rhs); }

    BentleyStatus Assign(UserECDbMapStrategy const&);
    BentleyStatus Assign(Strategy strategy, Option option, bool isPolymorphic);
    BentleyStatus Assign(Strategy strategy, bool isPolymorphic) { return Assign(strategy, Option::None, isPolymorphic); }

    bool IsValid() const;

    //Getters
    Strategy GetStrategy() const { return m_strategy; }
    Option GetOption() const { return m_option; }
    bool IsPolymorphic() const { return m_isPolymorphic; }

    bool IsResolved() const { return m_isResolved; }
    //Helper
    bool IsNotMapped() const { return m_strategy == Strategy::NotMapped; }
    bool IsPolymorphicSharedTable() const { return m_strategy == Strategy::SharedTable && m_isPolymorphic; }
    bool IsForeignKeyMapping() const { return m_strategy == Strategy::ForeignKeyRelationshipInSourceTable || m_strategy == Strategy::ForeignKeyRelationshipInTargetTable; }

    Utf8String ToString() const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE