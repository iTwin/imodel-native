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
    DbResult ToResultSet(Statement&, StaticPragmaResult&);
    virtual DbResult Read(PragmaManager::RowSet&, ECDbCR, PragmaVal const&,  PragmaManager::OptionsMap const&) override;
    virtual DbResult Write(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaExplainQuery>(); }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaDbList : PragmaManager::GlobalHandler {
    PragmaDbList():GlobalHandler("db_list","List all attach dbs"){}
    ~PragmaDbList(){}
    virtual DbResult Read(PragmaManager::RowSet&, ECDbCR, PragmaVal const&,  PragmaManager::OptionsMap const&) override;
    virtual DbResult Write(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaDbList>(); }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct DisqualifyTypeIndex : PragmaManager::ClassHandler {
    std::set<ECClassId> m_disqualifiedClassSet;
    DisqualifyTypeIndex():ClassHandler("disqualify_type_index","set/get disqualify_type_index flag for a given ECClass"){}
    ~DisqualifyTypeIndex(){}
    virtual DbResult Read(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, ECClassCR, PragmaManager::OptionsMap const&) override;
    virtual DbResult Write(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, ECClassCR, PragmaManager::OptionsMap const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<DisqualifyTypeIndex>(); }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaECDbVersion : PragmaManager::GlobalHandler {
    PragmaECDbVersion():GlobalHandler("ecdb_ver","return current and file profile versions"){}
    ~PragmaECDbVersion(){}
    virtual DbResult Read(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    virtual DbResult Write(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaECDbVersion>(); }
};

//=======================================================================================
// @bsiclass PragmaChecksum
//+===============+===============+===============+===============+===============+======
struct PragmaChecksum : PragmaManager::GlobalHandler {
    PragmaChecksum():GlobalHandler("checksum", "checksum([ec_schema|ec_map|db_schema]) return sha1 checksum for data."){}
    ~PragmaChecksum(){}
    virtual DbResult Read(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    virtual DbResult Write(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaChecksum>(); }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaExperimentalFeatures : PragmaManager::GlobalHandler {
    PragmaExperimentalFeatures():GlobalHandler("experimental_features_enabled","enable/disable experimental features"){}
    ~PragmaExperimentalFeatures(){}
    virtual DbResult Read(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    virtual DbResult Write(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaExperimentalFeatures>(); }
};

//=======================================================================================
// @bsiclass PragmaParseTree
//+===============+===============+===============+===============+===============+======
struct PragmaParseTree : PragmaManager::GlobalHandler {
    PragmaParseTree():GlobalHandler("parse_tree", "parse_tree(ecsql) return parse tree of ecsql."){}
    ~PragmaParseTree(){}
    virtual DbResult Read(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    virtual DbResult Write(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaParseTree>(); }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaIntegrityCheck : PragmaManager::GlobalHandler {
    PragmaIntegrityCheck():GlobalHandler("integrity_check","performs integrity checks on ECDb"){}
    virtual DbResult Read(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    DbResult CheckAll(IntegrityChecker&, StaticPragmaResult&, ECDbCR);
    DbResult CheckSchemaLoad(IntegrityChecker&, StaticPragmaResult&, ECDbCR);
    DbResult CheckEcProfile(IntegrityChecker&, StaticPragmaResult&, ECDbCR);
    DbResult CheckDataSchema(IntegrityChecker&, StaticPragmaResult&, ECDbCR);
    DbResult CheckDataColumns(IntegrityChecker&, StaticPragmaResult&, ECDbCR);
    DbResult CheckNavClassIds(IntegrityChecker&, StaticPragmaResult&, ECDbCR);
    DbResult CheckNavIds(IntegrityChecker&, StaticPragmaResult&, ECDbCR);
    DbResult CheckLinkTableFkClassIds(IntegrityChecker&, StaticPragmaResult&, ECDbCR);
    DbResult CheckLinkTableFkIds(IntegrityChecker&, StaticPragmaResult&, ECDbCR);
    DbResult CheckClassIds(IntegrityChecker&, StaticPragmaResult&, ECDbCR);
    DbResult CheckMissingChildRows(IntegrityChecker&, StaticPragmaResult&, ECDbCR);
    virtual DbResult Write(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaIntegrityCheck>(); }

};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaPurgeOrphanRelationships : PragmaManager::GlobalHandler {
    PragmaPurgeOrphanRelationships():GlobalHandler("purge_orphan_relationships","removes orphaned link-table relationships from ECDb"){}
    ~PragmaPurgeOrphanRelationships(){}
    virtual DbResult Read(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    virtual DbResult Write(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaPurgeOrphanRelationships>(); }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaCheckECSqlInsertValues : PragmaManager::GlobalHandler {
    PragmaCheckECSqlInsertValues():GlobalHandler("validate_ecsql_inserts", "validate values in ECSql insert statements."){}
    ~PragmaCheckECSqlInsertValues(){}
    virtual DbResult Read(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    virtual DbResult Write(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaCheckECSqlInsertValues>(); }
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
    static bool TableExists(DbCR, Utf8CP, Utf8CP);
    static DbResult ComputeHash(Utf8String&, DbCR, std::vector<std::string> const &, Utf8CP, HashSize, bool);
    static DbResult ComputeSQLiteSchemaHash(Utf8String&, DbCR, Utf8CP, HashSize);
public:
    static DbResult ComputeHash(Utf8StringR, DbCR, SourceType, Utf8CP dbAlias = "main", HashSize hashSize = HashSize::SHA3_256);
};

END_BENTLEY_SQLITE_EC_NAMESPACE