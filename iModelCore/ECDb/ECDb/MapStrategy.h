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

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
// @bsienum                                Krischan.Eberle                08/2016
//+===============+===============+===============+===============+===============+=====
enum class MapStrategy
    {
    NotMapped = 0,
    OwnTable = 1,
    TablePerHierarchy = 2,
    ExistingTable = 3,
    SharedTable = 4,

    ForeignKeyRelationshipInSourceTable = 100,
    ForeignKeyRelationshipInTargetTable = 101
    };

//======================================================================================
// @bsiclass                                Krischan.Eberle                08/2016
//+===============+===============+===============+===============+===============+=====
enum class JoinedTableInfo
    {
    None = 0,
    JoinedTable = 1,
    ParentOfJoinedTable = 2
    };

struct MapStrategyExtendedInfo;

//======================================================================================
// @bsiclass                                Krischan.Eberle                08/2016
//+===============+===============+===============+===============+===============+=====
struct TablePerHierarchyInfo
    {
    private:
        bool m_isValid;
        bool m_useSharedColumns;
        int m_sharedColumnCount;
        Utf8String m_excessColumnName;
        JoinedTableInfo m_joinedTableInfo;

        BentleyStatus DetermineSharedColumnsInfo(ECN::ShareColumns const&, MapStrategyExtendedInfo const* baseMapStrategy, ECN::ShareColumns const* baseClassShareColumnsCA, ECN::ECClassCR, IssueReporter const&);
        BentleyStatus DetermineJoinedTableInfo(bool hasJoinedTablePerDirectSubclassOption, MapStrategyExtendedInfo const* baseMapStrategy, ECN::ECClassCR, IssueReporter const&);

    public:
        TablePerHierarchyInfo() : TablePerHierarchyInfo(false) {}
        explicit TablePerHierarchyInfo(bool isValid) : m_isValid(isValid), m_useSharedColumns(false), m_sharedColumnCount(-1), m_joinedTableInfo(JoinedTableInfo::None) {}
        TablePerHierarchyInfo(bool isSharedColumns, int sharedColumnCount, Utf8CP excessColName, JoinedTableInfo joinedTableInfo)
            : m_isValid(true), m_useSharedColumns(isSharedColumns), m_sharedColumnCount(sharedColumnCount), m_excessColumnName(excessColName), m_joinedTableInfo(joinedTableInfo)
            {}

        BentleyStatus Initialize(ECN::ShareColumns const&, MapStrategyExtendedInfo const* baseMapStrategy, ECN::ShareColumns const* baseClassShareColumnsCA, bool hasJoinedTablePerDirectSubclassOption, ECN::ECClassCR, IssueReporter const&);

        //!@return true if the respective MapStrategy is TablePerHierarchy. false if MapStrategy is not TablePerHierarchy
        bool IsValid() const { return m_isValid; }
        bool UseSharedColumns() const { return m_useSharedColumns; }
        int GetSharedColumnCount() const { return m_sharedColumnCount; }
        Utf8StringCR GetExcessColumnName() const { return m_excessColumnName; }
        JoinedTableInfo GetJoinedTableInfo() const { return m_joinedTableInfo; }
    };


//======================================================================================
// @bsiclass                                Krischan.Eberle                08/2016
//+===============+===============+===============+===============+===============+=====
struct MapStrategyExtendedInfo
    {
public:
    static const MapStrategy DEFAULT = MapStrategy::OwnTable;

private:
    MapStrategy m_strategy;
    TablePerHierarchyInfo m_tphInfo;
    bool m_isValid;

public:
    MapStrategyExtendedInfo() : m_isValid(false) {}
    explicit MapStrategyExtendedInfo(MapStrategy strat) : m_strategy(strat), m_tphInfo(strat == MapStrategy::TablePerHierarchy), m_isValid(true) {}
    explicit MapStrategyExtendedInfo(TablePerHierarchyInfo const& tphInfo) : m_strategy(MapStrategy::TablePerHierarchy), m_tphInfo(tphInfo), m_isValid(true)
        {
        BeAssert(tphInfo.IsValid());
        }

    MapStrategy GetStrategy() const { return m_strategy; }
    TablePerHierarchyInfo const& GetTphInfo() const { return m_tphInfo; }

    bool IsValid() const { return m_isValid; }
    static bool IsForeignKeyMapping(MapStrategyExtendedInfo const& strat) { return strat.GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable || strat.GetStrategy() == MapStrategy::ForeignKeyRelationshipInTargetTable; }
    static Utf8CP ToString(MapStrategy);
    };


//======================================================================================
// @bsiclass                                 Krischan.Eberle               08/2016
//+===============+===============+===============+===============+===============+=====
struct ClassMappingCACache
    {
private:
    ECN::ECDbClassMap m_classMapCA;
    bool m_hasMapStrategy;
    MapStrategy m_mapStrategy;
    ECN::ShareColumns m_shareColumnsCA;
    bool m_hasJoinedTablePerDirectSubclassOption;

    static BentleyStatus TryParse(MapStrategy&, Utf8CP str, ECN::ECClassCR);

public:
    ClassMappingCACache() : m_hasMapStrategy(false), m_mapStrategy(MapStrategy::NotMapped), m_hasJoinedTablePerDirectSubclassOption(false) {}
    BentleyStatus Initialize(ECN::ECClassCR);

    ~ClassMappingCACache() {}

    bool HasMapStrategy() const { return m_hasMapStrategy; }
    ECN::ECDbClassMap const& GetClassMap() const { return m_classMapCA; }
    MapStrategy GetStrategy() const { return m_mapStrategy; }
    ECN::ShareColumns const& GetShareColumnsCA() const { return m_shareColumnsCA; }
    bool HasJoinedTablePerDirectSubclassOption() const { return m_hasJoinedTablePerDirectSubclassOption; }
    };



END_BENTLEY_SQLITE_EC_NAMESPACE