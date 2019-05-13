/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

        BentleyStatus DetermineSharedColumnsInfo(ShareColumnsCustomAttribute const&, MapStrategyExtendedInfo const* baseMapStrategy, ECN::ECClassCR, IIssueReporter const&);
        BentleyStatus DetermineJoinedTableInfo(bool hasJoinedTablePerDirectSubclassOption, MapStrategyExtendedInfo const* baseMapStrategy, ECN::ECClassCR, IIssueReporter const&);

    public:
        TablePerHierarchyInfo() {}
        TablePerHierarchyInfo(ShareColumnsMode shareColumnsMode, Nullable<uint32_t> maxSharedColumnsBeforeOverflow, JoinedTableInfo joinedTableInfo)
            : m_isValid(true), m_shareColumnsMode(shareColumnsMode), m_maxSharedColumnsBeforeOverflow(maxSharedColumnsBeforeOverflow), m_joinedTableInfo(joinedTableInfo)
            {}

        BentleyStatus Initialize(ShareColumnsCustomAttribute const&, MapStrategyExtendedInfo const* baseMapStrategy, bool hasJoinedTablePerDirectSubclassOption, ECN::ECClassCR, IIssueReporter const&);

        //!@return true if the respective MapStrategy is TablePerHierarchy
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
    explicit MapStrategyExtendedInfo(MapStrategy strat) : m_strategy(strat), m_isValid(true) { BeAssert(strat != MapStrategy::TablePerHierarchy); }
    MapStrategyExtendedInfo(MapStrategy strat, TablePerHierarchyInfo const& tphInfo) : m_strategy(strat), m_tphInfo(tphInfo), m_isValid(true) { BeAssert(strat == MapStrategy::TablePerHierarchy); BeAssert(tphInfo.IsValid()); }

    MapStrategy GetStrategy() const { return m_strategy; }
    bool IsTablePerHierarchy() const { return m_strategy == MapStrategy::TablePerHierarchy; }
    TablePerHierarchyInfo const& GetTphInfo() const { BeAssert(IsTablePerHierarchy() == m_tphInfo.IsValid()); return m_tphInfo; }

    bool IsValid() const { return m_isValid; }
    static bool IsForeignKeyMapping(MapStrategyExtendedInfo const& strat) { return strat.GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable || strat.GetStrategy() == MapStrategy::ForeignKeyRelationshipInTargetTable; }
    static Utf8CP ToString(MapStrategy);

    static BentleyStatus ParseMapStrategy(MapStrategy& strategy, Utf8StringCR str)
        {
        if (str.EqualsIAscii("OwnTable"))
            strategy = MapStrategy::OwnTable;
        else if (str.EqualsIAscii("NotMapped"))
            strategy = MapStrategy::NotMapped;
        else if (str.EqualsIAscii("TablePerHierarchy"))
            strategy = MapStrategy::TablePerHierarchy;
        else if (str.EqualsIAscii("ExistingTable"))
            strategy = MapStrategy::ExistingTable;
        else
            return ERROR;

        return SUCCESS;
        }

    };


//======================================================================================
// @bsiclass                                Krischan.Eberle                11/2017
//+===============+===============+===============+===============+===============+=====
struct MapStrategyValidator final
    {
    private:
        MapStrategyValidator() = delete;
        ~MapStrategyValidator() = delete;

    public:
        static bool Validate(MapStrategy strat) { return strat == MapStrategy::ExistingTable || strat == MapStrategy::ForeignKeyRelationshipInSourceTable || strat == MapStrategy::ForeignKeyRelationshipInTargetTable || strat == MapStrategy::NotMapped || strat == MapStrategy::OwnTable || strat == MapStrategy::TablePerHierarchy; }
        static bool Validate(TablePerHierarchyInfo::ShareColumnsMode mode) { return mode == TablePerHierarchyInfo::ShareColumnsMode::ApplyToSubclassesOnly || mode == TablePerHierarchyInfo::ShareColumnsMode::No || mode == TablePerHierarchyInfo::ShareColumnsMode::Yes; }
        static bool Validate(JoinedTableInfo info) { return info == JoinedTableInfo::JoinedTable || info == JoinedTableInfo::None || info == JoinedTableInfo::ParentOfJoinedTable; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE