/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/SHA1.h>
#include <ECDb/ECSqlStatement.h>
#include "PragmaECSqlPreparedStatement.h"
#include "PragmaVirtualTab.h"


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

    struct Module : BeSQLite::DbModule {
        std::set<ECClassId>& m_set;
        struct VTab : DbVirtualTable {
            struct Cursor : PragmaVirtualTabCursor {
                Cursor(VTab& vt) : PragmaVirtualTabCursor(vt) {}
                DbResult Filter(int, const char*, int argc, BeSQLite::DbValue* argv) override;
                DbResult Next() override { m_eof = true; return BE_SQLITE_OK; }
            };
            VTab(Module& m) : DbVirtualTable(m) {}
            DbResult Open(DbCursor*& cur) override { cur = new Cursor(*this); return BE_SQLITE_OK; }
            DbResult BestIndex(IndexInfo& info) override;
        };
        Module(ECDbR ecdb, std::set<ECClassId>& set)
            : BeSQLite::DbModule(ecdb, "pragma_disqualify_type_index", "CREATE TABLE x(disqualify_type_index,class_id hidden,val hidden)")
            , m_set(set) {}
        DbResult Connect(DbVirtualTable*& out, Config& conf, int, const char* const*) override {
            out = new VTab(*this); conf.SetTag(Config::Tags::Innocuous); return BE_SQLITE_OK;
        }
    };
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
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaECSqlVersion : PragmaManager::GlobalHandler {
    PragmaECSqlVersion() : GlobalHandler("ecsql_ver", "return current ECSQL version") {}
    ~PragmaECSqlVersion() {}
    virtual DbResult Read(PragmaManager::RowSet&, ECDbCR, const PragmaVal&, const PragmaManager::OptionsMap&) override;
    virtual DbResult Write(PragmaManager::RowSet&, ECDbCR, const PragmaVal&, const PragmaManager::OptionsMap&) override;
    static std::unique_ptr<PragmaManager::Handler> Create() { return std::make_unique<PragmaECSqlVersion>(); }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaSqliteSql : PragmaManager::GlobalHandler {
    PragmaSqliteSql() : GlobalHandler("sqlite_sql", "return underlying SQLite SQL") {}
    ~PragmaSqliteSql() {}
    virtual DbResult Read(PragmaManager::RowSet&, ECDbCR, const PragmaVal&, const PragmaManager::OptionsMap&) override;
    virtual DbResult Write(PragmaManager::RowSet&, ECDbCR, const PragmaVal&, const PragmaManager::OptionsMap&) override;
    static std::unique_ptr<PragmaManager::Handler> Create() { return std::make_unique<PragmaSqliteSql>(); }
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
struct PragmaCheckECSqlWriteValues : PragmaManager::GlobalHandler {
    PragmaCheckECSqlWriteValues():GlobalHandler("validate_ecsql_writes", "validate values in ECSql insert statements."){}
    ~PragmaCheckECSqlWriteValues(){}
    virtual DbResult Read(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    virtual DbResult Write(PragmaManager::RowSet&, ECDbCR, PragmaVal const&, PragmaManager::OptionsMap const&) override;
    static std::unique_ptr<PragmaManager::Handler> Create () { return std::make_unique<PragmaCheckECSqlWriteValues>(); }
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

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaECDbVersionModule : BeSQLite::DbModule {
    struct VTab : DbVirtualTable {
        struct Cursor : PragmaVirtualTabCursor {
            Cursor(VTab& vt) : PragmaVirtualTabCursor(vt) {}
            DbResult Filter(int, const char*, int, BeSQLite::DbValue*) override;
            DbResult Next() override { m_eof = true; return BE_SQLITE_OK; }
        };
        VTab(PragmaECDbVersionModule& m) : DbVirtualTable(m) {}
        DbResult Open(DbCursor*& cur) override { cur = new Cursor(*this); return BE_SQLITE_OK; }
        DbResult BestIndex(IndexInfo&) override { return BE_SQLITE_OK; }
    };
    PragmaECDbVersionModule(ECDbR ecdb) : BeSQLite::DbModule(ecdb, "pragma_ecdb_ver", "CREATE TABLE x(current,file)") {}
    DbResult Connect(DbVirtualTable*& out, Config& conf, int, const char* const*) override {
        out = new VTab(*this); conf.SetTag(Config::Tags::Innocuous); return BE_SQLITE_OK;
    }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaECSqlVersionModule : BeSQLite::DbModule {
    struct VTab : DbVirtualTable {
        struct Cursor : PragmaVirtualTabCursor {
            Cursor(VTab& vt) : PragmaVirtualTabCursor(vt) {}
            DbResult Filter(int, const char*, int, BeSQLite::DbValue*) override;
            DbResult Next() override { m_eof = true; return BE_SQLITE_OK; }
        };
        VTab(PragmaECSqlVersionModule& m) : DbVirtualTable(m) {}
        DbResult Open(DbCursor*& cur) override { cur = new Cursor(*this); return BE_SQLITE_OK; }
        DbResult BestIndex(IndexInfo&) override { return BE_SQLITE_OK; }
    };
    PragmaECSqlVersionModule(ECDbR ecdb) : BeSQLite::DbModule(ecdb, "pragma_ecsql_ver", "CREATE TABLE x(ecsql_ver)") {}
    DbResult Connect(DbVirtualTable*& out, Config& conf, int, const char* const*) override {
        out = new VTab(*this); conf.SetTag(Config::Tags::Innocuous); return BE_SQLITE_OK;
    }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaExperimentalFeaturesModule : BeSQLite::DbModule {
    struct VTab : DbVirtualTable {
        struct Cursor : PragmaVirtualTabCursor {
            Cursor(VTab& vt) : PragmaVirtualTabCursor(vt) {}
            DbResult Filter(int, const char*, int argc, BeSQLite::DbValue* argv) override;
            DbResult Next() override { m_eof = true; return BE_SQLITE_OK; }
        };
        VTab(PragmaExperimentalFeaturesModule& m) : DbVirtualTable(m) {}
        DbResult Open(DbCursor*& cur) override { cur = new Cursor(*this); return BE_SQLITE_OK; }
        DbResult BestIndex(IndexInfo& info) override;
    };
    PragmaExperimentalFeaturesModule(ECDbR ecdb) : BeSQLite::DbModule(ecdb, "pragma_experimental_features_enabled", "CREATE TABLE x(experimental_features_enabled,val hidden)") {}
    DbResult Connect(DbVirtualTable*& out, Config& conf, int, const char* const*) override {
        out = new VTab(*this); conf.SetTag(Config::Tags::Innocuous); return BE_SQLITE_OK;
    }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaCheckECSqlWriteValuesModule : BeSQLite::DbModule {
    struct VTab : DbVirtualTable {
        struct Cursor : PragmaVirtualTabCursor {
            Cursor(VTab& vt) : PragmaVirtualTabCursor(vt) {}
            DbResult Filter(int, const char*, int argc, BeSQLite::DbValue* argv) override;
            DbResult Next() override { m_eof = true; return BE_SQLITE_OK; }
        };
        VTab(PragmaCheckECSqlWriteValuesModule& m) : DbVirtualTable(m) {}
        DbResult Open(DbCursor*& cur) override { cur = new Cursor(*this); return BE_SQLITE_OK; }
        DbResult BestIndex(IndexInfo& info) override;
    };
    PragmaCheckECSqlWriteValuesModule(ECDbR ecdb) : BeSQLite::DbModule(ecdb, "pragma_validate_ecsql_writes", "CREATE TABLE x(validate_ecsql_writes,val hidden)") {}
    DbResult Connect(DbVirtualTable*& out, Config& conf, int, const char* const*) override {
        out = new VTab(*this); conf.SetTag(Config::Tags::Innocuous); return BE_SQLITE_OK;
    }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaChecksumModule : BeSQLite::DbModule {
    struct VTab : DbVirtualTable {
        struct Cursor : PragmaVirtualTabCursor {
            Cursor(VTab& vt) : PragmaVirtualTabCursor(vt) {}
            DbResult Filter(int, const char*, int argc, BeSQLite::DbValue* argv) override;
            DbResult Next() override { m_eof = true; return BE_SQLITE_OK; }
        };
        VTab(PragmaChecksumModule& m) : DbVirtualTable(m) {}
        DbResult Open(DbCursor*& cur) override { cur = new Cursor(*this); return BE_SQLITE_OK; }
        DbResult BestIndex(IndexInfo& info) override;
    };
    PragmaChecksumModule(ECDbR ecdb) : BeSQLite::DbModule(ecdb, "pragma_checksum", "CREATE TABLE x(sha3_256,val hidden)") {}
    DbResult Connect(DbVirtualTable*& out, Config& conf, int, const char* const*) override {
        out = new VTab(*this); conf.SetTag(Config::Tags::Innocuous); return BE_SQLITE_OK;
    }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaParseTreeModule : BeSQLite::DbModule {
    struct VTab : DbVirtualTable {
        struct Cursor : PragmaVirtualTabCursor {
            Cursor(VTab& vt) : PragmaVirtualTabCursor(vt) {}
            DbResult Filter(int, const char*, int argc, BeSQLite::DbValue* argv) override;
            DbResult Next() override { m_eof = true; return BE_SQLITE_OK; }
        };
        VTab(PragmaParseTreeModule& m) : DbVirtualTable(m) {}
        DbResult Open(DbCursor*& cur) override { cur = new Cursor(*this); return BE_SQLITE_OK; }
        DbResult BestIndex(IndexInfo& info) override;
    };
    PragmaParseTreeModule(ECDbR ecdb) : BeSQLite::DbModule(ecdb, "pragma_parse_tree", "CREATE TABLE x(val,ecsql hidden)") {}
    DbResult Connect(DbVirtualTable*& out, Config& conf, int, const char* const*) override {
        out = new VTab(*this); conf.SetTag(Config::Tags::Innocuous); return BE_SQLITE_OK;
    }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaSqliteSqlModule : BeSQLite::DbModule {
    struct VTab : DbVirtualTable {
        struct Cursor : PragmaVirtualTabCursor {
            Cursor(VTab& vt) : PragmaVirtualTabCursor(vt) {}
            DbResult Filter(int, const char*, int argc, BeSQLite::DbValue* argv) override;
            DbResult Next() override { m_eof = true; return BE_SQLITE_OK; }
        };
        VTab(PragmaSqliteSqlModule& m) : DbVirtualTable(m) {}
        DbResult Open(DbCursor*& cur) override { cur = new Cursor(*this); return BE_SQLITE_OK; }
        DbResult BestIndex(IndexInfo& info) override;
    };
    PragmaSqliteSqlModule(ECDbR ecdb) : BeSQLite::DbModule(ecdb, "pragma_sqlite_sql", "CREATE TABLE x(sqlite_sql,ecsql hidden)") {}
    DbResult Connect(DbVirtualTable*& out, Config& conf, int, const char* const*) override {
        out = new VTab(*this); conf.SetTag(Config::Tags::Innocuous); return BE_SQLITE_OK;
    }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaExplainQueryModule : BeSQLite::DbModule {
    struct VTab : DbVirtualTable {
        struct Cursor : PragmaVirtualTabCursor {
            std::vector<std::vector<PragmaColumnValue>> m_rows;
            size_t m_rowIdx = 0;
            Cursor(VTab& vt) : PragmaVirtualTabCursor(vt) {}
            DbResult Filter(int, const char*, int argc, BeSQLite::DbValue* argv) override;
            DbResult Next() override;
        };
        VTab(PragmaExplainQueryModule& m) : DbVirtualTable(m) {}
        DbResult Open(DbCursor*& cur) override { cur = new Cursor(*this); return BE_SQLITE_OK; }
        DbResult BestIndex(IndexInfo& info) override;
    };
    PragmaExplainQueryModule(ECDbR ecdb) : BeSQLite::DbModule(ecdb, "pragma_explain_query", "CREATE TABLE x(id,parent,notused,detail,ecsql hidden)") {}
    DbResult Connect(DbVirtualTable*& out, Config& conf, int, const char* const*) override {
        out = new VTab(*this); conf.SetTag(Config::Tags::Innocuous); return BE_SQLITE_OK;
    }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaDbListModule : BeSQLite::DbModule {
    struct VTab : DbVirtualTable {
        struct Cursor : PragmaVirtualTabCursor {
            std::vector<std::vector<PragmaColumnValue>> m_rows;
            size_t m_rowIdx = 0;
            Cursor(VTab& vt) : PragmaVirtualTabCursor(vt) {}
            DbResult Filter(int, const char*, int, BeSQLite::DbValue*) override;
            DbResult Next() override;
        };
        VTab(PragmaDbListModule& m) : DbVirtualTable(m) {}
        DbResult Open(DbCursor*& cur) override { cur = new Cursor(*this); return BE_SQLITE_OK; }
        DbResult BestIndex(IndexInfo&) override { return BE_SQLITE_OK; }
    };
    PragmaDbListModule(ECDbR ecdb) : BeSQLite::DbModule(ecdb, "pragma_db_list", "CREATE TABLE x(sno,alias,fileName,profile)") {}
    DbResult Connect(DbVirtualTable*& out, Config& conf, int, const char* const*) override {
        out = new VTab(*this); conf.SetTag(Config::Tags::Innocuous); return BE_SQLITE_OK;
    }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaPurgeOrphanRelationshipsModule : BeSQLite::DbModule {
    struct VTab : DbVirtualTable {
        struct Cursor : PragmaVirtualTabCursor {
            Cursor(VTab& vt) : PragmaVirtualTabCursor(vt) {}
            DbResult Filter(int, const char*, int, BeSQLite::DbValue*) override;
            DbResult Next() override { m_eof = true; return BE_SQLITE_OK; }
        };
        VTab(PragmaPurgeOrphanRelationshipsModule& m) : DbVirtualTable(m) {}
        DbResult Open(DbCursor*& cur) override { cur = new Cursor(*this); return BE_SQLITE_OK; }
        DbResult BestIndex(IndexInfo&) override { return BE_SQLITE_OK; }
    };
    PragmaPurgeOrphanRelationshipsModule(ECDbR ecdb) : BeSQLite::DbModule(ecdb, "pragma_purge_orphan_relationships", "CREATE TABLE x(status)") {}
    DbResult Connect(DbVirtualTable*& out, Config& conf, int, const char* const*) override {
        out = new VTab(*this); conf.SetTag(Config::Tags::Innocuous); return BE_SQLITE_OK;
    }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaIntegrityCheckModule : BeSQLite::DbModule {
    struct VTab : DbVirtualTable {
        struct Cursor : PragmaVirtualTabCursor {
            std::vector<std::vector<PragmaColumnValue>> m_rows;
            size_t m_rowIdx = 0;
            Cursor(VTab& vt) : PragmaVirtualTabCursor(vt) {}
            DbResult Filter(int, const char*, int argc, BeSQLite::DbValue* argv) override;
            DbResult Next() override;
        };
        VTab(PragmaIntegrityCheckModule& m) : DbVirtualTable(m) {}
        DbResult Open(DbCursor*& cur) override { cur = new Cursor(*this); return BE_SQLITE_OK; }
        DbResult BestIndex(IndexInfo& info) override;
    };
    // Superset schema: 20 result columns + 1 hidden arg
    // sno(0), check_name(1), result(2), elapsed_sec(3), schema(4), type(5), name(6),
    // issue(7), table_name(8), col(9), id(10), class(11), property(12), nav_id(13),
    // nav_classId(14), primary_class(15), key_id(16), key_classId(17), class_id(18),
    // MissingRowInTables(19), check_arg(20, hidden)
    PragmaIntegrityCheckModule(ECDbR ecdb) : BeSQLite::DbModule(ecdb, "pragma_integrity_check",
        "CREATE TABLE x(sno,check_name,result,elapsed_sec,schema,type,name,issue,table_name,col,id,class,property,nav_id,nav_classId,primary_class,key_id,key_classId,class_id,MissingRowInTables,check_arg hidden)") {}
    DbResult Connect(DbVirtualTable*& out, Config& conf, int, const char* const*) override {
        out = new VTab(*this); conf.SetTag(Config::Tags::Innocuous); return BE_SQLITE_OK;
    }
};

END_BENTLEY_SQLITE_EC_NAMESPACE