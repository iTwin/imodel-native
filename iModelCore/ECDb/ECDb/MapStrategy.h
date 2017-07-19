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
        Nullable<uint32_t> m_maxSharedColumnsBeforeOverflow;
        JoinedTableInfo m_joinedTableInfo = JoinedTableInfo::None;

        BentleyStatus DetermineSharedColumnsInfo(ShareColumnsCustomAttribute const&, MapStrategyExtendedInfo const* baseMapStrategy, ShareColumnsCustomAttribute const* baseClassShareColumnsCA, ECN::ECClassCR, IssueReporter const&);
        BentleyStatus DetermineJoinedTableInfo(bool hasJoinedTablePerDirectSubclassOption, MapStrategyExtendedInfo const* baseMapStrategy, ECN::ECClassCR, IssueReporter const&);

    public:
        TablePerHierarchyInfo() : TablePerHierarchyInfo(false) {}
        explicit TablePerHierarchyInfo(bool isValid) : m_isValid(isValid) {}
        TablePerHierarchyInfo(ShareColumnsMode shareColumnsMode, Nullable<uint32_t> maxSharedColumnsBeforeOverflow, JoinedTableInfo joinedTableInfo)
            : m_isValid(true), m_shareColumnsMode(shareColumnsMode), m_maxSharedColumnsBeforeOverflow(maxSharedColumnsBeforeOverflow), m_joinedTableInfo(joinedTableInfo)
            {}

        BentleyStatus Initialize(ShareColumnsCustomAttribute const&, MapStrategyExtendedInfo const* baseMapStrategy, ShareColumnsCustomAttribute const* baseClassShareColumnsCA, bool hasJoinedTablePerDirectSubclassOption, ECN::ECClassCR, IssueReporter const&);

        //!@return true if the respective MapStrategy is TablePerHierarchy. false if MapStrategy is not TablePerHierarchy
        bool IsValid() const { return m_isValid; }
        ShareColumnsMode GetShareColumnsMode() const { return m_shareColumnsMode; }
        Nullable<uint32_t> GetMaxSharedColumnsBeforeOverflow() const { return m_maxSharedColumnsBeforeOverflow; }
        JoinedTableInfo GetJoinedTableInfo() const { return m_joinedTableInfo; }
    };


//======================================================================================
// @bsiclass                                Krischan.Eberle                08/2016
//+===============+===============+===============+===============+===============+=====
struct MapStrategyExtendedInfo final
    {
private:
    MapStrategy m_strategy = MapStrategy::NotMapped;
    TablePerHierarchyInfo m_tphInfo;
    bool m_isValid = false;

public:
    MapStrategyExtendedInfo() {}
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
    ClassMapCustomAttribute m_classMapCA;
    Nullable<MapStrategy> m_mapStrategy;
    ShareColumnsCustomAttribute m_shareColumnsCA;
    bool m_hasJoinedTablePerDirectSubclassOption = false;

    static BentleyStatus TryParse(MapStrategy&, Utf8CP str, ECN::ECClassCR);

public:
    ClassMappingCACache() {}
    BentleyStatus Initialize(ECN::ECClassCR);

    ~ClassMappingCACache() {}

    ClassMapCustomAttribute const& GetClassMap() const { return m_classMapCA; }
    Nullable<MapStrategy> const& GetStrategy() const { return m_mapStrategy; }
    ShareColumnsCustomAttribute const& GetShareColumnsCA() const { return m_shareColumnsCA; }
    bool HasJoinedTablePerDirectSubclassOption() const { return m_hasJoinedTablePerDirectSubclassOption; }
    };



END_BENTLEY_SQLITE_EC_NAMESPACE