/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"
#include "ECDbPublishedTests.h"
#include <numeric>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
BEGIN_ECDBUNITTESTS_NAMESPACE


struct SchemaSyncTestFixture : public ECDbTestFixture {
	static std::unique_ptr<ECDb> CreateECDb(Utf8CP asFileName) {
        auto fileName = BuildECDbPath(asFileName);
        if (fileName.DoesPathExist()) {
			if (fileName.BeDeleteFile() != BeFileNameStatus::Success) {
                throw std::runtime_error("unable to delete file");
            }
        }
        auto ecdb = std::make_unique<ECDb>();
		if (BE_SQLITE_OK != ecdb->CreateNewDb(fileName)) {
            return nullptr;
        }
        ecdb->SaveChanges();
        return std::move(ecdb);
    }

	static std::unique_ptr<ECDb> CopyAs(ECDbR fromDb, Utf8CP asFileName) {
        fromDb.SaveChanges();
        auto fileName = BuildECDbPath(asFileName);
        if (fileName.DoesPathExist()) {
			if (fileName.BeDeleteFile() != BeFileNameStatus::Success) {
                throw std::runtime_error("unable to delete file");
            }
        }
        BeFileName::BeCopyFile( BeFileName(fromDb.GetDbFileName(), true), fileName);
        auto ecdb = std::make_unique<ECDb>();
        if (BE_SQLITE_OK != ecdb->OpenBeSQLiteDb(fileName, Db::OpenParams(Db::OpenMode::ReadWrite))) {
            return nullptr;
        }
        ecdb->SaveChanges();
        return std::move(ecdb);
    }
    static SchemaImportResult ImportSchemas(ECDbR ecdb, std::vector<SchemaItem> items, SchemaManager::SchemaImportOptions opts = SchemaManager::SchemaImportOptions::None) {
        auto schemaReadContext = ECSchemaReadContext::CreateContext();
		schemaReadContext->AddSchemaLocater(ecdb.GetSchemaLocater());
		bvector<ECSchemaCP> importSchemas;
		for(auto& item: items) {
			ECSchemaPtr schema;
			ECSchema::ReadFromXmlString(schema, item.GetXmlString().c_str(), *schemaReadContext);
			if (!schema.IsValid()) {
                return SchemaImportResult::ERROR;
            }
            importSchemas.push_back(schema.get());
        }
        return ecdb.Schemas().ImportSchemas(importSchemas, opts);
    }
	static SchemaImportResult ImportSchema(ECDbR ecdb, SchemaItem item, SchemaManager::SchemaImportOptions opts = SchemaManager::SchemaImportOptions::None) {
        return ImportSchemas(ecdb, std::vector<SchemaItem>{item}, opts);
    }
	static std::unique_ptr<ECDb> OpenECDb(Utf8CP asFileNam) {
        auto ecdb = std::make_unique<ECDb>();
        if (BE_SQLITE_OK != ecdb->OpenBeSQLiteDb(asFileNam, Db::OpenParams(Db::OpenMode::ReadWrite))) {
            return nullptr;
        }
        return std::move(ecdb);
    }
};


TEST_F(SchemaSyncTestFixture, test) {
    auto syncDb = CreateECDb("sync.db");
    const auto synDbFile = std::string{syncDb->GetDbFileName()};
    syncDb = nullptr;

	auto b1 = CreateECDb("b1.db");
    auto b2 = CreateECDb("b2.db");

    auto pushChangesToSyncDb = [&]( ECDbR ecdb) {
        ASSERT_EQ(SchemaImportResult::OK, ecdb.Schemas().SyncSchemas(synDbFile, SchemaManager::SyncAction::Push));
    };
    auto pullChangesFromSyncDb = [&](ECDbR ecdb) {
        ASSERT_EQ(SchemaImportResult::OK, ecdb.Schemas().SyncSchemas(synDbFile, SchemaManager::SyncAction::Pull));
        ecdb.SaveChanges();
    };

    pushChangesToSyncDb(*b1);

    auto schema1 = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
		<ECSchema schemaName="TestSchema1" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
			<ECEntityClass typeName="Pipe1">
				<ECProperty propertyName="p1" typeName="int" />
				<ECProperty propertyName="p2" typeName="int" />
			</ECEntityClass>
		</ECSchema>)xml");


    ASSERT_EQ (SchemaImportResult::OK, ImportSchema(*b1, schema1));
    b1->SaveChanges();
	ASSERT_TRUE(b1->TableExists("ts_Pipe1"));
    pushChangesToSyncDb(*b1);

	// check if sync db has schema changes
	syncDb = OpenECDb(synDbFile.c_str());
    auto pipe1 = syncDb->Schemas().GetClass("TestSchema1", "Pipe1");
    ASSERT_NE(pipe1, nullptr);
	ASSERT_EQ(pipe1->GetPropertyCount(), 2);
	ASSERT_FALSE(syncDb->TableExists("ts_Pipe1"));
    syncDb = nullptr;

	// pull changes from sync db into client db and check if schema changes was there and valid
    pullChangesFromSyncDb(*b2);
    pipe1 = b2->Schemas().GetClass("TestSchema1", "Pipe1");
    ASSERT_NE(pipe1, nullptr);
	ASSERT_EQ(pipe1->GetPropertyCount(), 2);
	ASSERT_TRUE(b2->TableExists("ts_Pipe1"));

	// add two more properties from client db and push it to sync db.
    auto schema2 = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
		<ECSchema schemaName="TestSchema1" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
			<ECEntityClass typeName="Pipe1">
				<ECProperty propertyName="p1" typeName="int" />
				<ECProperty propertyName="p2" typeName="int" />
				<ECProperty propertyName="p3" typeName="int" />
				<ECProperty propertyName="p4" typeName="int" />
			</ECEntityClass>
		</ECSchema>)xml");
    ASSERT_EQ (SchemaImportResult::OK, ImportSchema(*b2, schema2));
    b2->SaveChanges();
	pushChangesToSyncDb(*b2);

	// check if sync db has schema changes
	syncDb = OpenECDb(synDbFile.c_str());
    pipe1 = syncDb->Schemas().GetClass("TestSchema1", "Pipe1");
    ASSERT_NE(pipe1, nullptr);
	ASSERT_EQ(pipe1->GetPropertyCount(), 4);
    syncDb = nullptr;


	// pull changes from sync db into master db and check if schema changes was there and valid
    pullChangesFromSyncDb(*b1);
    pipe1 = b1->Schemas().GetClass("TestSchema1", "Pipe1");
    ASSERT_NE(pipe1, nullptr);
	ASSERT_EQ(pipe1->GetPropertyCount(), 4);
    ASSERT_TRUE(b1->TableExists("ts_Pipe1"));
}


END_ECDBUNITTESTS_NAMESPACE
