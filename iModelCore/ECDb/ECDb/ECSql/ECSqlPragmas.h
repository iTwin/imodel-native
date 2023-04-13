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

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaExperimentalFeatures : PragmaManager::GlobalHandler {
    PragmaExperimentalFeatures():GlobalHandler("experimental_features_enabled","enable/disable experimental features"){}
    ~PragmaExperimentalFeatures(){}
    virtual DbResult Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) override;
    virtual DbResult Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& val) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaExperimentalFeatures>(); }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaIntegrityCheck : PragmaManager::GlobalHandler {
    PragmaIntegrityCheck():GlobalHandler("integrity_check","performs integrity checks on ECDb"){}
    virtual DbResult Read(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const& v) override;
    DbResult CheckAll(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb);
    DbResult CheckSchemaLoad(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb);
    DbResult CheckEcProfile(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb);
    DbResult CheckDataSchema(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb);
    DbResult CheckDataColumns(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb);
    DbResult CheckNavClassIds(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb);
    DbResult CheckNavIds(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb);
    DbResult CheckLinkTableFkClassIds(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb);
    DbResult CheckLinkTableFkIds(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb);
    DbResult CheckClassIds(IntegrityChecker& checker, StaticPragmaResult& result, ECDbCR ecdb);
    virtual DbResult Write(PragmaManager::RowSet& rowSet, ECDbCR ecdb, PragmaVal const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaIntegrityCheck>(); }

};
//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SHA3Helper final {
    enum class HashSize {
        SHA3_224 = 224,
        SHA3_256 = 256,
        SHA3_384 = 384,
        SHA3_512 = 512,
    };
    enum class SourceType {
        ECDB_SCHEMA,
        ECDB_MAP,
        SQLITE_SCHEMA,
    };


private:
    static bool TableExists(DbCR db, Utf8CP tableName, Utf8CP dbAlias);
    static DbResult ComputeHash(Utf8String& hash, DbCR db, std::vector<std::string> const & tables, Utf8CP dbAlias, HashSize hashSize, bool skipTableThatDoesNotExists);
    static DbResult ComputeSQLiteSchemaHash(Utf8String& hash, DbCR db, Utf8CP dbAlias, HashSize hashSize);
public:
    static DbResult ComputeHash(Utf8StringR hash, DbCR db, SourceType type, Utf8CP dbAlias = "main", HashSize hashSize = HashSize::SHA3_256);
};

END_BENTLEY_SQLITE_EC_NAMESPACE