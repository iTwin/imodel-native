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
enum class ECDbMapStrategy
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
struct ECDbMapStrategyHelper
    {
    public:
        static const ECDbMapStrategy DEFAULT = ECDbMapStrategy::OwnTable;

    private:
        ECDbMapStrategyHelper();
        ~ECDbMapStrategyHelper();

    public:
        static bool IsForeignKeyMapping(ECDbMapStrategy strat) { return strat == ECDbMapStrategy::ForeignKeyRelationshipInSourceTable || strat == ECDbMapStrategy::ForeignKeyRelationshipInTargetTable; }
        static Utf8CP ToString(ECDbMapStrategy);
    };

//======================================================================================
// @bsiclass                                 Krischan.Eberle               08/2016
//+===============+===============+===============+===============+===============+=====
struct ClassMappingCACache
    {
private:
    ECN::ECDbClassMap m_classMapCA;
    ECDbMapStrategy m_mapStrategy;
    ECN::ShareColumns m_shareColumnsCA;
    bool m_hasJoinedTablePerDirectSubclassOption;

    static BentleyStatus TryParse(ECDbMapStrategy&, Utf8CP str, ECN::ECClassCR);

public:
    ClassMappingCACache() : m_mapStrategy(ECDbMapStrategy::NotMapped), m_hasJoinedTablePerDirectSubclassOption(false) {}
    BentleyStatus Initialize(ECN::ECClassCR);

    ~ClassMappingCACache() {}

    bool HasClassMapCA() const { return m_classMapCA.IsValid(); }
    ECN::ECDbClassMap const& GetClassMap() const { return m_classMapCA; }
    ECDbMapStrategy GetMapStrategy() const { return m_mapStrategy; }
    ECN::ShareColumns const& GetShareColumnsCA() const { return m_shareColumnsCA; }
    bool HasJoinedTablePerDirectSubclassOption() const { return m_hasJoinedTablePerDirectSubclassOption; }
    };


//======================================================================================
// @bsiclass                                Krischan.Eberle                08/2016
//+===============+===============+===============+===============+===============+=====
struct TablePerHierarchyInfo
    {
    public:
        enum class JoinedTableInfo
            {
            None = 0,
            JoinedTable = 1,
            ParentOfJoinedTable = 2
            };

    private:
        bool m_isSharedColumns;
        int m_sharedColumnCount;
        Utf8String m_excessColumnName;
        JoinedTableInfo m_joinedTableInfo;

        BentleyStatus DetermineSharedColumnsInfo(ECN::ShareColumns const&, ECN::ShareColumns const* baseClassShareColumnsCA, ECN::ECClassCR, IssueReporter const&);
        BentleyStatus DetermineJoinedTableInfo(bool hasJoinedTablePerDirectSubclassOption, JoinedTableInfo const* baseClassJoinedTableInfo, ECN::ECClassCR, IssueReporter const&);

    public:
        TablePerHierarchyInfo() : m_isSharedColumns(false), m_sharedColumnCount(-1), m_joinedTableInfo(JoinedTableInfo::None) {}
        TablePerHierarchyInfo(bool isSharedColumns, int sharedColumnCount, Utf8CP excessColName, JoinedTableInfo joinedTableInfo) 
            : m_isSharedColumns(isSharedColumns), m_sharedColumnCount(sharedColumnCount), m_excessColumnName(excessColName), m_joinedTableInfo(joinedTableInfo) 
            {}

        BentleyStatus Initialize(ECN::ShareColumns const&, ECN::ShareColumns const* baseClassShareColumnsCA, bool hasJoinedTablePerDirectSubclassOption, JoinedTableInfo const* baseClassJoinedTableInfo, ECN::ECClassCR, IssueReporter const&);

        bool IsSharedColumns() const { return m_isSharedColumns; }
        int GetSharedColumnCount() const { return m_sharedColumnCount; }
        Utf8StringCR GetExcessColumnName() const { return m_excessColumnName; }
        JoinedTableInfo GetJoinedTableInfo() const { return m_joinedTableInfo; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE