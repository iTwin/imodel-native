/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/SHA1.h>
#include <ECDb/ECSqlStatement.h>
#include "PragmaECSqlPreparedStatement.h"


BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaExplainQuery : PragmaManager::GlobalHandler {
    PragmaExplainQuery():GlobalHandler("explain_query","explain query plan"){}
    ~PragmaExplainQuery(){}
    DbResult ToResultSet(Statement& from, StaticPragmaResult& to);
    virtual DbResult Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& val) override;
    virtual DbResult Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaExplainQuery>(); }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct DisqualifyTypeIndex : PragmaManager::ClassHandler {
    std::set<ECClassId> m_disqualifiedClassSet;
    DisqualifyTypeIndex():ClassHandler("disqualify_type_index","set/get disqualify_type_index flag for a given ECClass"){}
    ~DisqualifyTypeIndex(){}
    virtual DbResult Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&, ECClassCR cls) override;
    virtual DbResult Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& val, ECClassCR cls) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<DisqualifyTypeIndex>(); }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaECDbVersion : PragmaManager::GlobalHandler {
    PragmaECDbVersion():GlobalHandler("ecdb_ver","return current and file profile versions"){}
    ~PragmaECDbVersion(){}
    virtual DbResult Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) override;
    virtual DbResult Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaECDbVersion>(); }
};

//=======================================================================================
// @bsiclass PragmaChecksum
//+===============+===============+===============+===============+===============+======
struct PragmaChecksum : PragmaManager::GlobalHandler {
    PragmaChecksum():GlobalHandler("checksum", "checksum([ec_schema|ec_map|db_schema]) return sha1 checksum for data."){}
    ~PragmaChecksum(){}
    virtual DbResult Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) override;
    virtual DbResult Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaChecksum>(); }
};
//================================================================================
// @bsiclass PragmaECDbValidation
//================================================================================
struct PragmaECDbValidation : PragmaManager::GlobalHandler {
    PragmaECDbValidation():GlobalHandler("validate","performs validation checks on ECDb"){}
    virtual DbResult Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) override;
    virtual DbResult Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaECDbValidation>(); }
};

//================================================================================
// @bsiclass PragmaECDbClassIdValidation
//================================================================================
struct PragmaECDbClassIdValidation : PragmaManager::GlobalHandler {
    PragmaECDbClassIdValidation():GlobalHandler("class_id_check","checks if classIds are valid") {}
    virtual DbResult Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) override;
    virtual DbResult Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaECDbClassIdValidation>(); }
};

//================================================================================
// @bsiclass PragmaECDbNavPropIdValidation
//================================================================================
struct PragmaECDbNavPropIdValidation : PragmaManager::GlobalHandler {
    PragmaECDbNavPropIdValidation():GlobalHandler("nav_prop_id_check","checks if classIds are valid") {}
    virtual DbResult Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) override;
    virtual DbResult Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaECDbNavPropIdValidation>(); }
};


//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct DataSHA1 final {
    private:
        static void AppendRow(SHA1& hash, Statement& stmt, bool includeNulls);
        static DbResult AppendRows(SHA1& hash, Statement& stmt, bool includeNulls);
        static bool TableExists(DbCR db, Utf8CP tableName, Utf8CP dbAlias = "main");
        static DbResult ComputeTableHash(SHA1& hash, DbCR db, Utf8CP tableName, Utf8CP dbAlias = "main", bool includeNulls = false, bool sortColumns = false);
    public:
        static DbResult ComputeSchemaHash(SHA1& hash, DbCR db, Utf8CP dbAlias = "main");
        static DbResult ComputeMapHash(SHA1& hash, DbCR db, Utf8CP dbAlias = "main");
        static DbResult ComputeDbSchemaHash(SHA1& hash, DbCR db, Utf8CP dbAlias = "main");
};

END_BENTLEY_SQLITE_EC_NAMESPACE