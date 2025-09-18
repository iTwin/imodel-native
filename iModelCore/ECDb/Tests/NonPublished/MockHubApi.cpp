#include "MockHubApi.h"
#include <numeric>
#include <iostream>

USING_NAMESPACE_BENTLEY_SQLITE_EC;
//***************************************************************************************
// SchemaSyncDb
//***************************************************************************************
// Default SHA  hashes
const char* SchemaSyncTestFixture::DEFAULT_SHA3_256_ECDB_SCHEMA = "44c5d675cdab562b732a90b8c0128149daaa7a2beefbcbddb576f7bf059cec33";
const char* SchemaSyncTestFixture::DEFAULT_SHA3_256_ECDB_MAP = "9c7834d13177336f0fa57105b9c1175b912b2e12e62ca2224482c0ffd9dfd337";
const char* SchemaSyncTestFixture::DEFAULT_SHA3_256_SQLITE_SCHEMA = "c4ca1cdd07de041e71f3e8d4b1942d29da89653c85276025d786688b6f576443";
const char* SchemaSyncTestFixture::DEFAULT_SHA3_256_CHANNEL_SQLITE_SCHEMA = "c4ca1cdd07de041e71f3e8d4b1942d29da89653c85276025d786688b6f576443";
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::Test(Utf8CP name, std::function<void()> test){
    std::cout << "  o " << " " << name << std::endl;
    test();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaImportResult SchemaSyncTestFixture::ImportSchemas(ECDbR ecdb, std::vector<SchemaItem> items, SchemaManager::SchemaImportOptions opts, SchemaSync::SyncDbUri uri) {
    auto schemaReadContext = ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(ecdb.GetSchemaLocater());
    bvector<ECSchemaCP> importSchemas;
    for(auto& item: items) {
        ECSchemaPtr schema;
        if (item.GetType() == SchemaItem::Type::File)
            {
            // Construct the path to the sample schema
            BeFileName ecSchemaFilePath;
            BeTest::GetHost().GetDocumentsRoot(ecSchemaFilePath);
            ecSchemaFilePath.AppendToPath(L"ECDb");
            ecSchemaFilePath.AppendToPath(L"Schemas");
            ecSchemaFilePath.AppendToPath(item.GetFileName());

            if (!ecSchemaFilePath.DoesPathExist())
                return SchemaImportResult::ERROR;

            schemaReadContext->AddSchemaPath(ecSchemaFilePath.GetName());
            ECSchema::ReadFromXmlFile(schema, ecSchemaFilePath, *schemaReadContext);
            }
        else
            {
            BeAssert(item.GetType() == SchemaItem::Type::String);
            ECSchema::ReadFromXmlString(schema, item.GetXmlString().c_str(), *schemaReadContext);
            }
        if (!schema.IsValid()) {
            return SchemaImportResult::ERROR;
        }
        importSchemas.push_back(schema.get());
    }
    return ecdb.Schemas().ImportSchemas(importSchemas, opts,nullptr, uri);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaImportResult SchemaSyncTestFixture::ImportSchema(ECDbR ecdb, SchemaItem item, SchemaManager::SchemaImportOptions opts, SchemaSync::SyncDbUri uri) {
    return ImportSchemas(ecdb, std::vector<SchemaItem>{item}, opts, uri);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaImportResult SchemaSyncTestFixture::ImportSchema(SchemaItem item, SchemaManager::SchemaImportOptions opts)
    {
    return ImportSchemas(*m_briefcase, std::vector<SchemaItem> {item}, opts, GetSyncDbUri());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<TrackedECDb> SchemaSyncTestFixture::OpenECDb(Utf8CP asFileNam) {
    auto ecdb = std::make_unique<TrackedECDb>();
    if (BE_SQLITE_OK != ecdb->OpenBeSQLiteDb(asFileNam, Db::OpenParams(Db::OpenMode::ReadWrite))) {
        return nullptr;
    }
    return std::move(ecdb);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult SchemaSyncTestFixture::ReopenECDb()
    {
    if (!m_briefcase->IsDbOpen())
        return BE_SQLITE_ERROR;

    auto saveStatus = m_briefcase->SaveChanges();
    if (saveStatus != BE_SQLITE_OK)
        {
        printf("Failed to save changes");
        return saveStatus;
        }

    Utf8String filename = m_briefcase->GetDbFileName();

    m_briefcase->CloseDb();
    m_briefcase = OpenECDb(filename.c_str());
    if (m_briefcase == nullptr)
        return BE_SQLITE_ERROR;

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::CloseECDb()
    {
    if (m_briefcase->IsDbOpen())
        m_briefcase->SaveChanges();
    m_briefcase->CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaImportResult SchemaSyncTestFixture::SetupECDb(Utf8CP ecdbName)
    {
    m_hub = std::make_unique<ECDbHub>(ECDbHub());
    m_briefcase = m_hub->CreateBriefcase();
    m_schemaChannel = std::make_unique<SchemaSyncDb>(SchemaSyncDb(ecdbName));
    if (SchemaSync::Status::OK != m_briefcase->Schemas().GetSchemaSync().Init(GetSyncDbUri(), BeGuid(true).ToString(), false))
        return SchemaImportResult::ERROR;

    EXPECT_EQ(BE_SQLITE_OK, m_briefcase->PullMergePush("init"));
    EXPECT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

    m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckSyncHashes(syncDb); });
    CheckHashes(*m_briefcase);

    return SchemaImportResult::OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaImportResult SchemaSyncTestFixture::SetupECDb(Utf8CP ecdbName, SchemaItem const& schema, SchemaManager::SchemaImportOptions opts)
    {
    m_hub = std::make_unique<ECDbHub>(ECDbHub());
    m_briefcase = m_hub->CreateBriefcase();
    m_schemaChannel = std::make_unique<SchemaSyncDb>(SchemaSyncDb(ecdbName));
    if (SchemaSync::Status::OK != m_briefcase->Schemas().GetSchemaSync().Init(GetSyncDbUri(), BeGuid(true).ToString(), false))
        return SchemaImportResult::ERROR;

    EXPECT_EQ(BE_SQLITE_OK, m_briefcase->PullMergePush("init"));
    EXPECT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

    m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckSyncHashes(syncDb); });
    CheckHashes(*m_briefcase);

    if (SchemaImportResult::OK != SchemaSyncTestFixture::ImportSchema(*m_briefcase, schema, opts, GetSyncDbUri()))
        {
        EXPECT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
        return SchemaImportResult::ERROR;
        }

    EXPECT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

    return SchemaImportResult::OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DropSchemaResult SchemaSyncTestFixture::DropSchema(Utf8CP schemaName)
    {
    auto dropSuccess = m_briefcase->Schemas().DropSchema(schemaName);

    if (dropSuccess.IsSuccess())
        EXPECT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
    else
        EXPECT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());

    return dropSuccess;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SchemaSyncTestFixture::GetSchemaHash(ECDbCR db) {
    ECSqlStatement stmt;
    if (stmt.Prepare(db, "PRAGMA checksum(ecdb_schema)") != ECSqlStatus::Success) {
        return "";
    }
    if (stmt.Step() == BE_SQLITE_ROW) {
        return stmt.GetValueText(0);
    }
    return "";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SchemaSyncTestFixture::GetMapHash(ECDbCR db) {
    ECSqlStatement stmt;
    if (stmt.Prepare(db, "PRAGMA checksum(ecdb_map)") != ECSqlStatus::Success) {
        return "";
    }
    if (stmt.Step() == BE_SQLITE_ROW) {
        return stmt.GetValueText(0);
    }
    return "";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SchemaSyncTestFixture::GetDbSchemaHash(ECDbCR db) {
    ECSqlStatement stmt;
    if (stmt.Prepare(db, "PRAGMA checksum(sqlite_schema)") != ECSqlStatus::Success) {
        return "";
    }
    if (stmt.Step() == BE_SQLITE_ROW) {
        return stmt.GetValueText(0);
    }
    return "";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaSyncTestFixture::ForeignkeyCheck(ECDbCR db) {
    Statement stmt;
    EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "PRAGMA foreign_key_check"));
    auto rc = stmt.Step();
    if (rc == BE_SQLITE_DONE) {
        return true;
    }
    while(rc == BE_SQLITE_ROW) {
        printf("%s\n",
                SqlPrintfString("[table=%s], [rowid=%lld], [parent=%s], [fkid=%d]",
                                stmt.GetValueText(0),
                                stmt.GetValueInt64(1),
                                stmt.GetValueText(2),
                                stmt.GetValueInt(3))
                    .GetUtf8CP());

        rc = stmt.Step();
    }
    return false;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::PrintHash(ECDbR ecdb, Utf8CP desc) {
    printf("=====%s======\n", desc);
    printf("\tSchema: SHA3-%s\n", GetSchemaHash(ecdb).c_str());
    printf("\t   Map: SHA3-%s\n", GetMapHash(ecdb).c_str());
    printf("\t    Db: SHA3-%s\n", GetDbSchemaHash(ecdb).c_str());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncTestFixture::CheckHashes(ECDbR ecdb, Utf8CP schemaHash, Utf8CP mapHash, Utf8CP dbSchemaHash, bool strictCheck, int lineNo)
    {
    if (strictCheck)
        {
        ASSERT_STREQ(schemaHash, GetSchemaHash(ecdb).c_str())       << "File: " << ecdb.GetDbFileName() << " Line: " << lineNo;
        ASSERT_STREQ(mapHash, GetMapHash(ecdb).c_str())             << "File: " << ecdb.GetDbFileName() << " Line: " << lineNo;;
        ASSERT_STREQ(dbSchemaHash, GetDbSchemaHash(ecdb).c_str())   << "File: " << ecdb.GetDbFileName() << " Line: " << lineNo;;
        }
    else
        {
        EXPECT_STREQ(schemaHash, GetSchemaHash(ecdb).c_str())       << "File: " << ecdb.GetDbFileName() << " Line: " << lineNo;;
        EXPECT_STREQ(mapHash, GetMapHash(ecdb).c_str())             << "File: " << ecdb.GetDbFileName() << " Line: " << lineNo;;
        EXPECT_STREQ(dbSchemaHash, GetDbSchemaHash(ecdb).c_str())   << "File: " << ecdb.GetDbFileName() << " Line: " << lineNo;;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string SchemaSyncTestFixture::GetIndexDDL(ECDbCR ecdb, Utf8CP indexName) {
    Statement stmt;
    EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "select sql from sqlite_master where name=?"));
    stmt.BindText(1, indexName, Statement::MakeCopy::Yes);
    if (stmt.Step() == BE_SQLITE_ROW) {
        return stmt.GetValueText(0);
    }
    return "";
};

//***************************************************************************************
// SchemaSyncDb
//***************************************************************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSyncDb::SchemaSyncDb(Utf8CP name){
    BeFileName outPath;
    BeTest::GetHost().GetOutputRoot(outPath);
    Utf8String fileName = name;
    fileName.append(".ecdb");
    outPath.AppendToPath(WString(fileName.c_str(), true).GetWCharCP());
    if (outPath.DoesPathExist()) {
        if (outPath.BeDeleteFile() != BeFileNameStatus::Success) {
            throw std::runtime_error("unable to delete file");
        }
    }
    m_fileName = outPath;
    auto ecdb = std::make_unique<ECDb>();
    if (BE_SQLITE_OK != ecdb->CreateNewDb(m_fileName)) {
        throw std::runtime_error("unable to create file");
    }
    ecdb->SaveChanges();
    ecdb->CloseDb();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName SchemaSyncDb::GetFileName() const { return m_fileName;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<ECDb> SchemaSyncDb::OpenReadOnly(DefaultTxn mode) {
    auto ecdb = std::make_unique<ECDb>();
    if (ecdb->OpenBeSQLiteDb(m_fileName, Db::OpenParams(Db::OpenMode::Readonly, mode)) != BE_SQLITE_OK) {
        return nullptr;
    }
    return std::move(ecdb);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<ECDb> SchemaSyncDb::OpenReadWrite(DefaultTxn mode) {
    auto ecdb = std::make_unique<ECDb>();
    if (ecdb->OpenBeSQLiteDb(m_fileName, Db::OpenParams(Db::OpenMode::ReadWrite, mode)) != BE_SQLITE_OK) {
        return nullptr;
    }
    return std::move(ecdb);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncDb::WithReadOnly(std::function<void(ECDbR)> cb, DefaultTxn mode) {
    auto ecdb = OpenReadOnly(mode);
    if (ecdb == nullptr) {
        throw std::runtime_error("unable to open file");
    }
    cb(*ecdb);
    ecdb->CloseDb();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaSyncDb::WithReadWrite(std::function<void(ECDbR)> cb, DefaultTxn mode) {
    auto ecdb = OpenReadWrite(mode);
    if (ecdb == nullptr) {
        throw std::runtime_error("unable to open file");
    }
    cb(*ecdb);
    ecdb->CloseDb();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// DbResult SchemaSyncDb::Push(ECDbR ecdb, std::function<void()> cb) {
//     auto rc = ecdb.Schemas().SyncSchemas(GetFileName().GetNameUtf8(), SchemaManager::SyncAction::Push);
//     if (rc == BE_SQLITE_OK && cb != nullptr) {
//         cb();
//     }
//     return rc;
// }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSync::Status SchemaSyncDb::Pull(ECDbR ecdb, std::function<void()> cb) {
    auto rc = ecdb.Schemas().GetSchemaSync().Pull(GetSyncDbUri());
    if (rc == SchemaSync::Status::OK && cb != nullptr) {
        cb();
    }
    return rc;
}

//***************************************************************************************
// InMemoryECDb
//***************************************************************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InMemoryECDb::WriteToDisk(Utf8CP fileName, const char *zSchema, bool overrideFile) const {
    BeFileName filePath(fileName);
    if (filePath.DoesPathExist()) {
        if (overrideFile) {
            if (filePath.BeDeleteFile() != BeFileNameStatus::Success) {
                return false;
            }
        } else {
            return false;
        }
    }
    DbBuffer buf = Serialize(zSchema);
    if (buf.Empty()) {
        return false;
    }
    BeFile outFile;
    if (BeFileStatus::Success != outFile.Create(filePath, true)) {
        return false;
    }
    if (BeFileStatus::Success != outFile.Write(nullptr, buf.Data(), (uint32_t)buf.Size())) {
        return false;
    }
    if (BeFileStatus::Success != outFile.Flush() ){
        return false;
    }
    return BeFileStatus::Success == outFile.Close();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InMemoryECDb::Ptr InMemoryECDb::CreateSnapshot(DbResult* outRc) {
    DbResult ALLOW_NULL_OUTPUT(rc, outRc);
    //SaveChanges("create snapshot");
    auto buff = Serialize();
    auto dbPtr = InMemoryECDb::Create();
    dbPtr->CloseDb();
    rc = Db::Deserialize(buff, *dbPtr, DbDeserializeOptions::FreeOnClose | DbDeserializeOptions::Resizable, nullptr, [&](DbR db) {
        db.ResetBriefcaseId(BeBriefcaseId(0));
    });
    if (rc == BE_SQLITE_OK) {

        dbPtr->ChangeDbGuid(GetDbGuid());
        return std::move(dbPtr);
    }
    return nullptr;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InMemoryECDb::Ptr InMemoryECDb::Create() {
	return Ptr(new InMemoryECDb());

}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaImportResult InMemoryECDb::ImportSchema(SchemaItem const& si) {
    auto ctx = ECSchemaReadContextPtr();
    if (ECDbTestFixture::ReadECSchema(ctx, *this, si) != SUCCESS)
        return SchemaImportResult::ERROR;

    bvector<ECN::ECSchemaP> schemas;
    ctx->GetCache().GetSchemas(schemas);
    bvector<ECN::ECSchemaCP> schemasIn(schemas.begin(), schemas.end());
    return Schemas().ImportSchemas(schemasIn, nullptr);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InMemoryECDb::InMemoryECDb() {
    if (CreateNewDb(BEDB_MemoryDb) != BE_SQLITE_OK) {
        throw std::runtime_error("unable to created in memory ecdb");
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InMemoryECDb::~InMemoryECDb() {
    if (IsDbOpen())
        CloseDb();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InMemoryECDb::_OnDbClose() {
    SaveChanges();
    ECDb::_OnDbClose();
}

//***************************************************************************************
// TrackedECDb
//***************************************************************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TrackedECDb::_OnDbCreated(CreateParams const& params) {
	auto rc = ECDb::_OnDbCreated(params);
	SetupTracker();
	return rc;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TrackedECDb::_OnDbOpening() {
	auto rc = ECDb::_OnDbOpening();
	if (!IsReadonly()) {
		SetupTracker();

	}
	return rc;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TrackedECDb::SetupTracker(std::unique_ptr<ECDbChangeTracker> tracker) {
    if (m_tracker != nullptr) {
        m_tracker->EndTracking();
        this->SetChangeTracker(nullptr);
        m_tracker = nullptr;
    }
    m_tracker = tracker != nullptr ? std::move(tracker) : std::make_unique<ECDbChangeTracker>(*this);
    this->SetChangeTracker(m_tracker.get());
    m_tracker->EnableTracking(true);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TrackedECDb::~TrackedECDb() {
    if (IsDbOpen())
        CloseDb();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TrackedECDb::_OnDbClose() {
    SaveChanges();
    this->SetChangeTracker(nullptr);
    if (m_tracker != nullptr) {
        m_tracker->EndTracking();
        m_tracker = nullptr;
    }
    ECDb::_OnDbClose();
}

//***************************************************************************************
// ECDbChangeTracker
//***************************************************************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<ECDbChangeSet const*> ECDbChangeTracker::GetLocalChangesets() const {
    std::vector<ECDbChangeSet const*>  list;
    for(auto& r : m_localChangesets) {
        list.push_back(r.get());
    }
    return list;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbChangeSet::Ptr ECDbChangeTracker::MakeChangeset(bool deleteLocalChangesets, Utf8CP op) {
    bvector<Utf8String> ddlChanges;
    ChangeGroup group;
    bool hasSchemaChanges = false;

    m_mdb.SaveChanges(op);

    for (auto& changeset : m_localChangesets) {
        const auto rc = changeset->AddToChangeGroup(group);
        if (rc != BE_SQLITE_OK) {
            return nullptr;
        }
        for (auto& ddl : changeset->GetDDLs()) {
            ddlChanges.push_back(ddl);
        }
        if (!hasSchemaChanges) {
            hasSchemaChanges = changeset->HasSchemaChanges();
        }
    }

    auto changeset = ECDbChangeSet::Create((int)(m_localChangesets.size() + 1), op, BeStringUtilities::Join(ddlChanges, ";").c_str(), hasSchemaChanges);
    if (BE_SQLITE_OK != changeset->FromChangeGroup(group)) {
        return nullptr;
    }
    if (deleteLocalChangesets) {
        m_localChangesets.clear();
    }
    return std::move(changeset);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeTracker::OnCommitStatus ECDbChangeTracker::_OnCommit(bool isCommit, Utf8CP operation) {
    if (isCommit) {
        auto changeset = ECDbChangeSet::From(*this, operation);
        if (changeset != nullptr) {
            m_localChangesets.push_back(std::move(changeset));
        }
    }
    EndTracking();
    CreateSession();
    return OnCommitStatus::Commit;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbChangeTracker::Ptr ECDbChangeTracker::Clone(ECDb& db) const {
    auto tracker= Create(db);
    for (auto& changeset : m_localChangesets) {
        tracker->m_localChangesets.push_back(changeset->Clone());
    }
    return std::move(tracker);

}

//***************************************************************************************
// ECDbChangeSet
//***************************************************************************************

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbChangeSet::ToSQL(DbCR db, std::function<void(bool isDDL, std::string const&)> cb) const{
	for (auto& ddl : GetDDLs()) {
		cb(true,  ddl + ";");
	}
	bmap<Utf8String,bvector<Utf8String>> tableColMap;
	auto toString = [](DbValue const& val) -> std::string {
		if (!val.IsValid()) {
			return "NULL";
		}
		switch (val.GetValueType()) {
		case DbValueType::IntegerVal:
			return Utf8PrintfString("%" PRId64, val.GetValueInt64());
		case DbValueType::FloatVal:
			return Utf8PrintfString("%0.17lf", val.GetValueDouble());
		case DbValueType::TextVal:
			return Utf8PrintfString("'%s'", val.GetValueText());
		case DbValueType::BlobVal: {
			Utf8String hexStr;
			std::string valStr = "X'";
			const auto ptr = (uint8_t const*)(val.GetValueBlob());
			const auto len = val.GetValueBytes();
			for (auto i = 0; i < len; ++i) {
				hexStr.clear();
				hexStr.Sprintf("%02X", ptr[i]);
				valStr.append(hexStr);
			}
			valStr.append("'");
			return std::move(valStr);
		}
		case DbValueType::NullVal:
			return "NULL";
		}
		return "<unknown>";
	};

	std::string sql;
	for (auto const& change : const_cast<ECDbChangeSet*>(this)->GetChanges()) {
		Byte* pkCols;
		int nPkCols;
		Utf8CP tableName;
		DbOpcode opCode;
		int indirect;
		int nCols;
		change.GetOperation(&tableName, &nCols, &opCode, &indirect);
		change.GetPrimaryKeyColumns(&pkCols, &nPkCols);
		auto it = tableColMap.find(tableName);
		if (it == tableColMap.end()) {
			auto newIt = tableColMap.emplace(tableName, bvector<Utf8String>());
			it = newIt.first;
			db.GetColumns(it->second, tableName);
		}
		auto const& columns = it->second;
		if (opCode == DbOpcode::Delete) {
			sql = "DELETE FROM ";
			sql.append(tableName).append(" WHERE ");
			bool first = true;
			for (int i = 0; i < nCols; ++i) {
				if (pkCols[i]) {
					auto& columnName = columns[i];
					auto dbVal = change.GetOldValue(i);
					if (first) {
						first = false;
					} else {
						sql.append(" AND ");
					}
					sql.append("(").append(columnName).append(" IS ").append(toString(dbVal).append(")"));
				}
			}
			sql.append(";");
			cb(false, sql);
		} else if (opCode == DbOpcode::Insert) {
			sql = "INSERT INTO ";
			sql.append(tableName).append("(");
			std::string val = " VALUES(";
			bool first = true;
			for (int i = 0; i < nCols; ++i) {
				auto& columnName = columns[i];
				auto dbVal = change.GetNewValue(i);
				if (dbVal.IsNull())
					continue;

				if (first) {
					first = false;
				} else {
					sql.append(",");
					val.append(",");
				}
				sql.append(columnName);
				val.append(toString(dbVal));
			}
			sql.append(")").append(val).append(");");
			cb(false, sql);
		} else if (opCode == DbOpcode::Update) {
			sql = "UPDATE ";
			sql.append(tableName).append(" SET ");
			std::string where = " WHERE ";
			bool firstPk = true;
			bool firstData = true;
			for (int i = 0; i < nCols; ++i) {
				auto& columnName = columns[i];
				if (pkCols[i]) {
					auto dbVal = change.GetOldValue(i);
					if (firstPk) {
						firstPk = false;
					} else {
						sql.append(" AND ");
					}
					where.append("(").append(columnName).append(" IS ").append(toString(dbVal).append(")"));
				} else {
					auto oldVal = change.GetNewValue(i);
					if (firstData) {
						firstData = false;
					} else {
						sql.append(",");
					}
					sql.append(columnName).append("=").append(toString(oldVal));
				}
			}
			sql.append(where).append(";");
			cb(false, sql);
		}
	}
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> ECDbChangeSet::GetDDLs() const {
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(m_ddl.c_str(), ";", tokens);
    return tokens;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbChangeSet::Ptr ECDbChangeSet::Clone() const {
    auto changeset = std::make_unique<ECDbChangeSet>(0, m_operation.c_str(), m_ddl.c_str(), m_hasSchemaChanges);
    for (auto& chunk : this->m_data.m_chunks) {
        changeset->m_data.Append((Byte const*)&chunk[0], (int)chunk.size());
    }
    return std::move(changeset);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbChangeSet::Ptr ECDbChangeSet::From(ECDbChangeTracker& tracker, Utf8CP comment) {
    if (!tracker.HasChanges() && !tracker.HasDdlChanges()) {
        return nullptr;
    }
    auto changeset = std::make_unique<ECDbChangeSet>( (int)(tracker.GetLocalChangesets().size() + 1), comment, tracker.GetDDL().c_str(), false);
    if (tracker.HasChanges()) {
        auto rc = changeset->FromChangeTrack(tracker);
        if (rc != BE_SQLITE_OK) {
            return nullptr;
        }
        bool hasECChanges = false;
        for (auto& change : changeset->GetChanges()) {
            if (change.GetTableName().StartsWithIAscii("ec_")) {
                hasECChanges = true;
                break;
            }
        }
        changeset->m_hasSchemaChanges = hasECChanges;
    }
    return std::move(changeset);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbChangeSet::Ptr ECDbChangeSet::Create(int index, Utf8CP op, Utf8CP ddl, bool isSchemaChangeset) {
    return std::make_unique<ECDbChangeSet>(index, op, ddl, isSchemaChangeset);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeStream::ConflictResolution ECDbChangeSet::_OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) {

    Utf8CP tableName = nullptr;
    int nCols, indirect;
    DbOpcode opcode;
    DbResult result = iter.GetOperation(&tableName, &nCols, &opcode, &indirect);
    BeAssert(result == BE_SQLITE_OK);

    if (cause == ChangeSet::ConflictCause::Conflict) {
        return ChangeSet::ConflictResolution::Replace;
    }
    if (cause == ChangeSet::ConflictCause::ForeignKey) {
        // Note: No current or conflicting row information is provided if it's a FKey conflict
        // Since we abort on FKey conflicts, always try and provide details about the error
        int nConflicts = 0;
        result = iter.GetFKeyConflicts(&nConflicts);
        BeAssert(result == BE_SQLITE_OK);
        LOG.errorv("Detected %d foreign key conflicts in ChangeSet. Aborting merge.", nConflicts);
        return ChangeSet::ConflictResolution::Abort ;
    }
    if(cause == ChangeSet::ConflictCause::NotFound) {
        if (opcode == DbOpcode::Delete) {
            // Caused by CASCADE DELETE on a foreign key, and is usually not a problem.
            return ChangeSet::ConflictResolution::Skip;
        }
  if (opcode == DbOpcode::Update && 0 == ::strncmp(tableName, "ec_", 3)) {
            // Caused by a ON DELETE SET NULL constraint on a foreign key - this is known to happen with "ec_" tables, but needs investigation if it happens otherwise
            return ChangeSet::ConflictResolution::Skip;
        }
        // Refer to comment below
        return opcode == DbOpcode::Update ? ChangeSet::ConflictResolution::Skip : ChangeSet::ConflictResolution::Replace;
    }
    if (ChangeSet::ConflictCause::Constraint == cause) {
        return ChangeSet::ConflictResolution::Skip;
    }
    return ConflictResolution::Replace;
}

//***************************************************************************************
// ECDbHub
//***************************************************************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName ECDbHub::BuildECDbPath(Utf8CP name) const {
    BeFileName outPath = m_basePath;
    Utf8String fileName = name;
    fileName.append(".ecdb");
    outPath.AppendToPath(WString(fileName.c_str(), true).c_str());
    return outPath;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbHub::CreateSeedFile() {
    m_seedFile = BuildECDbPath("seed");
    if (m_seedFile.DoesPathExist()) {
        if (m_seedFile.BeDeleteFile() != BeFileNameStatus::Success) {
            throw std::runtime_error("unable to delete file");
        }
    }
/* Use this instead of the block below to create a new DB. Currently this will change the Checksums used in all tests...
    auto ecdb = std::make_unique<TrackedECDb>();
    if (BE_SQLITE_OK != ecdb->CreateNewDb(m_seedFile)) {
        return BE_SQLITE_ERROR;
    }
    ecdb->SaveChanges();
    ecdb->CloseDb();*/

    BeFileName seed4003FileName;
    BeTest::GetHost().GetDocumentsRoot(seed4003FileName);
    seed4003FileName.AppendToPath(L"ECDb").AppendToPath(L"profileseeds").AppendToPath(L"4003-sync-seed.ecdb");
    ECDb ecdb;
    if (ECDbTestFixture::CloneECDb(ecdb, m_seedFile, seed4003FileName) != DbResult::BE_SQLITE_OK)
        return BE_SQLITE_ERROR;

    ecdb.SaveChanges();
    ecdb.CloseDb();
    return BE_SQLITE_OK;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbHub::ECDbHub():m_id(true), m_briefcaseid(10) {
    BeFileName outPath;
    BeTest::GetHost().GetOutputRoot(outPath);
    outPath.AppendToPath(WString(m_id.ToString().c_str(), true).c_str());
    if (!outPath.DoesPathExist()) {
        BeFileName::CreateNewDirectory(outPath.GetName());
    }
    m_basePath = outPath;
    CreateSeedFile();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<ECDbChangeSet*> ECDbHub::Query(int afterChangesetId) {
	std::vector<ECDbChangeSet*> results;
	if (afterChangesetId < 0 || afterChangesetId >= (int)m_changesets.size()) {
		return results;
	}
	for (auto i = afterChangesetId; i < m_changesets.size(); ++i) {
		results.push_back(m_changesets[i].get());
	}
	return results;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int ECDbHub::PushNewChangeset(ECDbChangeSet::Ptr changeset) {
	m_changesets.push_back(std::move(changeset));
	return (int)(m_changesets.size()) - 1;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<TrackedECDb> ECDbHub::CreateBriefcase() {
    const auto briefcaseId = GetNextBriefcaseId();
    const auto fileName = BuildECDbPath(SqlPrintfString("%d", briefcaseId.GetValue()).GetUtf8CP());
    BeFileName::BeCopyFile(m_seedFile, fileName);
    auto ecdb = std::make_unique<TrackedECDb>();
    if (BE_SQLITE_OK != ecdb->OpenBeSQLiteDb(fileName, Db::OpenParams(Db::OpenMode::ReadWrite))) {
        fileName.BeDeleteFile();
        return nullptr;
    }
    ecdb->ResetBriefcaseId(briefcaseId);
    ecdb->SaveChanges();
    ecdb->SetHub(*this);
    ecdb->PullMergePush("");
    ecdb->SaveChanges();
    return std::move(ecdb);
}

struct PrintChangeSet {
    public:
        static void Print(ECDbCR conn, ECDbChangeSet const& cs) {
            std::vector<std::string> ddlList;
            std::vector<std::string> dmlList;
            cs.ToSQL(conn, [&](bool isDDL, const std::string& sql) {
                if (isDDL) {
                    ddlList.push_back(sql);
                } else {
                    dmlList.push_back(sql);
                }
            });
            int i = 0;
            std::sort(dmlList.begin(), dmlList.end());
            printf("====================================================\n");
            for(auto& v: ddlList) {
                printf("%2d. %s\n", ++i, v.c_str());
            }
            for(auto& v: dmlList) {
                printf("%2d. %s\n", ++i, v.c_str());
            }
            printf("====================================================\n");
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult TrackedECDb::PullMergePush(Utf8CP comment) {
    if (m_hub == nullptr || m_tracker == nullptr) {
        return BE_SQLITE_ERROR;
    }
    auto changesetsToApply = m_hub->Query(m_changesetId + 1);

#ifdef TRACE_CS
    auto cancelTrace = GetTraceStmtEvent().AddListener([](TraceContext const& ctx, Utf8CP sql) {
        printf("[STMT] %s\n", ctx.GetExpandedSql().c_str());

    });
#endif
    m_tracker->EnableTracking(false);
    for (auto& changesetToApply : changesetsToApply) {
        changesetToApply->SetECDb(*this);
#ifdef TRACE_CS
        PrintChangeSet::Print(*this, *changesetToApply);
#endif
//         for (auto& ddl : changesetToApply->GetDDLs()) {
//             auto rc = TryExecuteSql(ddl.c_str());
//             if (rc != BE_SQLITE_OK && false) {
//                 m_tracker->EnableTracking(true);
//                 LOG.errorv("PullAndMergeChangesFrom(): %s", GetLastError().c_str());
// #ifdef TRACE_CS
//                 cancelTrace();
// #endif
//                 return rc;
//             }
//         }
        auto rc = changesetToApply->ApplyChanges(*this, false, true);
        if (rc != BE_SQLITE_OK) {
            LOG.errorv("PullAndMergeChangesFrom(): %s", GetLastError().c_str());
#ifdef TRACE_CS
            cancelTrace();
#endif
            return rc;
        }
    }
    if (!changesetsToApply.empty()) {
        auto rc = AfterSchemaChangeSetApplied();
        if (rc != BE_SQLITE_OK) {
#ifdef TRACE_CS
            cancelTrace();
#endif
            return rc;
        }
    }
    m_tracker->EnableTracking(true);
    if (!m_tracker->GetLocalChangesets().empty()){
        auto changeset = m_tracker->MakeChangeset(true, comment);
        if (changeset == nullptr) {
            m_tracker->EnableTracking(true);
#ifdef TRACE_CS
            cancelTrace();
#endif
            return BE_SQLITE_ERROR;
        }
        m_changesetId = m_hub->PushNewChangeset(std::move(changeset));
    }
#ifdef TRACE_CS
    cancelTrace();
#endif
    return BE_SQLITE_OK;

}