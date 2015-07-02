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
// @bsienum                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
enum class MapStrategy
    {
    None = 0,

    //Public Strategies that user can specify in custom attribute ClassMap::MapStrategy
    //===========================================================================================
    NotMapped,
    OwnTable,
    SharedTable,
    ExistingTable,
    
    //Private strategies used by ECDb internally
    Default = OwnTable,

    ForeignKeyRelationshipInTargetTable = 100,
    ForeignKeyRelationshipInSourceTable = 101
    };

//---------------------------------------------------------------------------------
// @bsienum                                 Krischan.Eberle                06/2015
//+---------------+---------------+---------------+---------------+---------------+------
enum class MapStrategyOptions
    {
    None = 0,
    Readonly = 1,
    SharedColumns = 2,
    SharedColumnsForSubclasses = 4,
    DisableSharedColumns = 8
    };


//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2015
//+---------------+---------------+---------------+---------------+---------------+------
struct ECDbMapStrategy
    {
private:
    MapStrategy m_strategy;
    int m_options;
    bool m_isPolymorphic;

    static BentleyStatus TryParse(MapStrategy&, Utf8CP str);
    static BentleyStatus TryParse(int& options, Utf8CP str);

public:
    ECDbMapStrategy() : m_strategy(MapStrategy::None), m_options((int) MapStrategyOptions::None), m_isPolymorphic(false) {}
    ECDbMapStrategy(MapStrategy strategy, int options, bool isPolymorphic) : m_strategy(strategy), m_options(options), m_isPolymorphic(isPolymorphic) {}
    ~ECDbMapStrategy() {}

    ECDbMapStrategy(ECDbMapStrategy const& rhs) : m_strategy(rhs.m_strategy), m_options(rhs.m_options), m_isPolymorphic(rhs.m_isPolymorphic) {}
    ECDbMapStrategy(ECDbMapStrategy&& rhs) : m_strategy(std::move(rhs.m_strategy)), m_options(std::move(rhs.m_options)), m_isPolymorphic(std::move(rhs.m_isPolymorphic)) {}
    ECDbMapStrategy& operator=(ECDbMapStrategy const&);
    ECDbMapStrategy& operator=(ECDbMapStrategy&&);

    //operators
    bool operator== (ECDbMapStrategy const& rhs) const { return m_strategy == rhs.m_strategy && m_options == rhs.m_options && m_isPolymorphic == rhs.m_isPolymorphic; }
    bool operator!= (ECDbMapStrategy const& rhs) const { return !(*this == rhs); }

    BentleyStatus Assign(MapStrategy strategy, MapStrategyOptions options, bool isPolymorphic);
    BentleyStatus Assign(MapStrategy strategy, bool isPolymorphic) { return Assign(strategy, MapStrategyOptions::None, isPolymorphic); }

    bool IsValid(bool isResolved = true) const;

    //Getters
    MapStrategy GetStrategy() const { return m_strategy; }
    int GetOptions() const { return m_options; }
    bool HasOptions() const { return m_options != (int) MapStrategyOptions::None; }
    bool HasOption(MapStrategyOptions option) const { return (m_options & (int) option) == (int) option; }
    bool IsPolymorphic() const { return m_isPolymorphic; }

    //Helper
    bool IsNotMapped() const { return m_strategy == MapStrategy::NotMapped; }
    bool IsPolymorphicSharedTable() const { return m_strategy == MapStrategy::SharedTable && m_isPolymorphic; }
    bool IsForeignKeyMapping() const { return m_strategy == MapStrategy::ForeignKeyRelationshipInSourceTable || m_strategy == MapStrategy::ForeignKeyRelationshipInTargetTable; }

    Utf8String ToString() const;

    static BentleyStatus TryParse(ECDbMapStrategy&, ECN::ECDbClassMap::MapStrategy const& mapStrategyCustomAttribute);
    };

    END_BENTLEY_SQLITE_EC_NAMESPACE