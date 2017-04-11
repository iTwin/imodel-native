/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/MapStrategy.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//_BENTLEY_INTERNAL_ONLY_
#include "ECDbInternalTypes.h"
#include "ECDbMapSchemaHelper.h"

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
    ForeignKeyRelationshipInTargetTable = 10,
    ForeignKeyRelationshipInSourceTable = 11
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
struct TablePerHierarchyInfo final
    {
    public:
        enum class ShareColumnsMode
            {
            No = 0,
            Yes = 1,
            ApplyToSubclassesOnly = 2
            };
    private:
        bool m_isValid = false;
        ShareColumnsMode m_shareColumnsMode = ShareColumnsMode::No;
        int m_sharedColumnCount = -1;
        JoinedTableInfo m_joinedTableInfo = JoinedTableInfo::None;

        BentleyStatus DetermineSharedColumnsInfo(ShareColumns const&, MapStrategyExtendedInfo const* baseMapStrategy, ShareColumns const* baseClassShareColumnsCA, ECN::ECClassCR, IssueReporter const&);
        BentleyStatus DetermineJoinedTableInfo(bool hasJoinedTablePerDirectSubclassOption, MapStrategyExtendedInfo const* baseMapStrategy, ECN::ECClassCR, IssueReporter const&);

    public:
        TablePerHierarchyInfo() : TablePerHierarchyInfo(false) {}
        explicit TablePerHierarchyInfo(bool isValid) : m_isValid(isValid) {}
        TablePerHierarchyInfo(ShareColumnsMode shareColumnsMode, int sharedColumnCount, JoinedTableInfo joinedTableInfo)
            : m_isValid(true), m_shareColumnsMode(shareColumnsMode), m_sharedColumnCount(sharedColumnCount), m_joinedTableInfo(joinedTableInfo)
            {}

        BentleyStatus Initialize(ShareColumns const&, MapStrategyExtendedInfo const* baseMapStrategy, ShareColumns const* baseClassShareColumnsCA, bool hasJoinedTablePerDirectSubclassOption, ECN::ECClassCR, IssueReporter const&);

        //!@return true if the respective MapStrategy is TablePerHierarchy. false if MapStrategy is not TablePerHierarchy
        bool IsValid() const { return m_isValid; }
        ShareColumnsMode GetShareColumnsMode() const { return m_shareColumnsMode; }
        int GetSharedColumnCount() const { return m_sharedColumnCount; }
        JoinedTableInfo GetJoinedTableInfo() const { return m_joinedTableInfo; }
    };


//======================================================================================
// @bsiclass                                Krischan.Eberle                08/2016
//+===============+===============+===============+===============+===============+=====
struct MapStrategyExtendedInfo final
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
    bool IsTablePerHierarchy() const { return m_strategy == MapStrategy::TablePerHierarchy; }
    TablePerHierarchyInfo const& GetTphInfo() const { BeAssert(IsTablePerHierarchy() == m_tphInfo.IsValid()); return m_tphInfo; }

    bool IsValid() const { return m_isValid; }
    static bool IsForeignKeyMapping(MapStrategyExtendedInfo const& strat) { return strat.GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable || strat.GetStrategy() == MapStrategy::ForeignKeyRelationshipInTargetTable; }
    static Utf8CP ToString(MapStrategy);
    };


//======================================================================================
// @bsiclass                                 Krischan.Eberle               08/2016
//+===============+===============+===============+===============+===============+=====
struct ClassMappingCACache final
    {
private:
    ECDbClassMap m_classMapCA;
    bool m_hasMapStrategy;
    MapStrategy m_mapStrategy;
    ShareColumns m_shareColumnsCA;
    bool m_hasJoinedTablePerDirectSubclassOption;
    DbIndexList m_dbIndexListCA;

    static BentleyStatus TryParse(MapStrategy&, Utf8CP str, ECN::ECClassCR);

public:
    ClassMappingCACache() : m_hasMapStrategy(false), m_mapStrategy(MapStrategy::NotMapped), m_hasJoinedTablePerDirectSubclassOption(false) {}
    BentleyStatus Initialize(ECN::ECClassCR);

    ~ClassMappingCACache() {}

    bool HasMapStrategy() const { return m_hasMapStrategy; }
    ECDbClassMap const& GetClassMap() const { return m_classMapCA; }
    MapStrategy GetStrategy() const { return m_mapStrategy; }
    ShareColumns const& GetShareColumnsCA() const { return m_shareColumnsCA; }
    bool HasJoinedTablePerDirectSubclassOption() const { return m_hasJoinedTablePerDirectSubclassOption; }
    DbIndexList const& GetDbIndexListCA() const { return m_dbIndexListCA; }
    };



END_BENTLEY_SQLITE_EC_NAMESPACE