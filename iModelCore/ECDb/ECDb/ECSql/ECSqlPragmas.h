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