/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"
#include "ECDbPublishedTests.h"
#include <numeric>
#include <Bentley/SHA1.h>

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
    static Utf8String GetSchemaHash(ECDbCR db) {
        ECSqlStatement stmt;
        if (stmt.Prepare(db, "PRAGMA checksum(ec_schema)") != ECSqlStatus::Success) {
            return "";
        }
        if (stmt.Step() == BE_SQLITE_ROW) {
            return stmt.GetValueText(0);
        }
        return "";
    }
    static Utf8String GetMapHash(ECDbCR db) {
        ECSqlStatement stmt;
        if (stmt.Prepare(db, "PRAGMA checksum(ec_map)") != ECSqlStatus::Success) {
            return "";
        }
        if (stmt.Step() == BE_SQLITE_ROW) {
            return stmt.GetValueText(0);
        }
        return "";
    }
    static Utf8String GetDbSchemaHash(ECDbCR db) {
        ECSqlStatement stmt;
        if (stmt.Prepare(db, "PRAGMA checksum(db_schema)") != ECSqlStatus::Success) {
            return "";
        }
        if (stmt.Step() == BE_SQLITE_ROW) {
            return stmt.GetValueText(0);
        }
        return "";
    }
};
/**
 * =====b1-import======
        Schema: SHA1-58378b2ff3c9c4ad9664395274173d2c9dee1970
           Map: SHA1-fe4dfa5552f7a28b49e003ba2d2989ef3380538b
            Db: SHA1-c7c7a104e2197c9aeb95e76534318a1c6ecc68d7
=====syncDb======
        Schema: SHA1-58378b2ff3c9c4ad9664395274173d2c9dee1970
           Map: SHA1-fe4dfa5552f7a28b49e003ba2d2989ef3380538b
            Db: SHA1-c6f37c87a652dbeb8b0413ad3dca36b59e8b273b
=====b2-pull======
        Schema: SHA1-58378b2ff3c9c4ad9664395274173d2c9dee1970
           Map: SHA1-64cbec645bf8fdea4800cd3e3508165ddd43c2a1
            Db: SHA1-c7c7a104e2197c9aeb95e76534318a1c6ecc68d7
=====b2-import======
        Schema: SHA1-5cbd24c81c6f5bd432a3c4829b4691c53977e490
           Map: SHA1-5de69370fba557c2efdd877241b936ab2b8dbd46
            Db: SHA1-a0ecfd5f3845cc756be16b9649b1a3d5d8be3325
=====syncDb======
        Schema: SHA1-5cbd24c81c6f5bd432a3c4829b4691c53977e490
           Map: SHA1-2c27ac8d1c59937eaa95f769956cde7e8b2bb662
            Db: SHA1-c6f37c87a652dbeb8b0413ad3dca36b59e8b273b
 *
 *
*/

TEST_F(SchemaSyncTestFixture, Test) {
    auto printHash = [&](ECDbR ecdb, Utf8CP desc) {
        printf("=====%s======\n", desc);
        printf("\tSchema: SHA1-%s\n", GetSchemaHash(ecdb).c_str());
        printf("\t   Map: SHA1-%s\n", GetMapHash(ecdb).c_str());
        printf("\t    Db: SHA1-%s\n", GetDbSchemaHash(ecdb).c_str());
    };

    std::string synDbFile;
    if ("create syncdb") {
        auto syncDb = CreateECDb("sync.db");
        synDbFile = std::string{syncDb->GetDbFileName()};
    }

    auto b1 = CreateECDb("b1.db");
    auto b2 = CopyAs(*b1, "b2.db");
    ASSERT_EQ(BE_SQLITE_OK, b1->Schemas().InitSyncDb(synDbFile));

    if ("check syn db hash") {
        auto syncDb = OpenECDb(synDbFile.c_str());
        EXPECT_STREQ("9a0c2041a498d674fc0045cb0209e3dd8951b904", GetSchemaHash(*syncDb).c_str());
        EXPECT_STREQ("49675fee5c0f1264853df55142364516b5ef5202", GetMapHash(*syncDb).c_str());
        EXPECT_STREQ("c6f37c87a652dbeb8b0413ad3dca36b59e8b273b", GetDbSchemaHash(*syncDb).c_str());
    }

    EXPECT_STREQ("9a0c2041a498d674fc0045cb0209e3dd8951b904", GetSchemaHash(*b1).c_str());
    EXPECT_STREQ("49675fee5c0f1264853df55142364516b5ef5202", GetMapHash(*b1).c_str());
    EXPECT_STREQ("c6f37c87a652dbeb8b0413ad3dca36b59e8b273b", GetDbSchemaHash(*b1).c_str());


    EXPECT_STREQ("9a0c2041a498d674fc0045cb0209e3dd8951b904", GetSchemaHash(*b2).c_str());
    EXPECT_STREQ("49675fee5c0f1264853df55142364516b5ef5202", GetMapHash(*b2).c_str());
    EXPECT_STREQ("c6f37c87a652dbeb8b0413ad3dca36b59e8b273b", GetDbSchemaHash(*b2).c_str());


    auto pushChangesToSyncDb = [&](ECDbR ecdb) {
        ASSERT_EQ(SchemaImportResult::OK, ecdb.Schemas().SyncSchemas(synDbFile, SchemaManager::SyncAction::Push));
    };
    auto pullChangesFromSyncDb = [&](ECDbR ecdb) {
        ASSERT_EQ(SchemaImportResult::OK, ecdb.Schemas().SyncSchemas(synDbFile, SchemaManager::SyncAction::Pull));
        ecdb.SaveChanges();
    };

    auto getIndexDDL = [&](ECDbCR ecdb, Utf8CP indexName) -> std::string {
        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "select sql from sqlite_master where name=?"));
        stmt.BindText(1, indexName, Statement::MakeCopy::Yes);
        if (stmt.Step() == BE_SQLITE_ROW) {
            return stmt.GetValueText(0);
        }
        return "";
    };

    pushChangesToSyncDb(*b1);

    if ("import schema into b1") {
        auto schema1 = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                <ECEntityClass typeName="Pipe1">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <DbIndexList xmlns="ECDbMap.02.00.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>idx_pipe1_p1</Name>
                                    <IsUnique>False</IsUnique>
                                    <Properties>
                                        <string>p1</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="p1" typeName="int" />
                    <ECProperty propertyName="p2" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml");
        ASSERT_EQ (SchemaImportResult::OK, ImportSchema(*b1, schema1));
        b1->SaveChanges();
        ASSERT_TRUE(b1->TableExists("ts_Pipe1"));
        ASSERT_STRCASEEQ(getIndexDDL(*b1, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1])");
        pushChangesToSyncDb(*b1);
        EXPECT_STREQ("58378b2ff3c9c4ad9664395274173d2c9dee1970", GetSchemaHash(*b1).c_str());
        EXPECT_STREQ("fe4dfa5552f7a28b49e003ba2d2989ef3380538b", GetMapHash(*b1).c_str());
        EXPECT_STREQ("c7c7a104e2197c9aeb95e76534318a1c6ecc68d7", GetDbSchemaHash(*b1).c_str());

    }

    if("check if sync-db has changes but not tables and index") {
        auto syncDb = OpenECDb(synDbFile.c_str());
        auto pipe1 = syncDb->Schemas().GetClass("TestSchema1", "Pipe1");
        ASSERT_NE(pipe1, nullptr);
        ASSERT_EQ(pipe1->GetPropertyCount(), 2);
        ASSERT_FALSE(syncDb->TableExists("ts_Pipe1"));
        ASSERT_STRCASEEQ(getIndexDDL(*syncDb, "idx_pipe1_p1").c_str(), "");
        EXPECT_STREQ("58378b2ff3c9c4ad9664395274173d2c9dee1970", GetSchemaHash(*syncDb).c_str());
        EXPECT_STREQ("fe4dfa5552f7a28b49e003ba2d2989ef3380538b", GetMapHash(*syncDb).c_str());
        EXPECT_STREQ("c6f37c87a652dbeb8b0413ad3dca36b59e8b273b", GetDbSchemaHash(*syncDb).c_str()); // It should not change
    }

	if("pull changes from sync-db into b2 and verify class, table and index exists") {
        pullChangesFromSyncDb(*b2);
        auto pipe1 = b2->Schemas().GetClass("TestSchema1", "Pipe1");
        ASSERT_NE(pipe1, nullptr);
        ASSERT_EQ(pipe1->GetPropertyCount(), 2);
        ASSERT_TRUE(b2->TableExists("ts_Pipe1"));
        ASSERT_STRCASEEQ(getIndexDDL(*b2, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1])");
        EXPECT_STREQ("58378b2ff3c9c4ad9664395274173d2c9dee1970", GetSchemaHash(*b2).c_str());
        EXPECT_STREQ("fe4dfa5552f7a28b49e003ba2d2989ef3380538b", GetMapHash(*b2).c_str());
        EXPECT_STREQ("c7c7a104e2197c9aeb95e76534318a1c6ecc68d7", GetDbSchemaHash(*b2).c_str());
    }

    if ("update schema by adding more properties and expand index in b2") {
        auto schema2 = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                <ECEntityClass typeName="Pipe1">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <DbIndexList xmlns="ECDbMap.02.00.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>idx_pipe1_p1</Name>
                                    <IsUnique>False</IsUnique>
                                    <Properties>
                                        <string>p1</string>
                                        <string>p2</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="p1" typeName="int" />
                    <ECProperty propertyName="p2" typeName="int" />
                    <ECProperty propertyName="p3" typeName="int" />
                    <ECProperty propertyName="p4" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml");
        ASSERT_EQ (SchemaImportResult::OK, ImportSchema(*b2, schema2));
        b2->SaveChanges();
        ASSERT_STRCASEEQ(getIndexDDL(*b2, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1], [p2])");
        pushChangesToSyncDb(*b2);
        EXPECT_STREQ("5cbd24c81c6f5bd432a3c4829b4691c53977e490", GetSchemaHash(*b2).c_str());   // CORRECT
        EXPECT_STREQ("5775dbd6f8956d6cb25e5f55c57948eaeefd7a42", GetMapHash(*b2).c_str());      // CORRECT
        EXPECT_STREQ("a0ecfd5f3845cc756be16b9649b1a3d5d8be3325", GetDbSchemaHash(*b2).c_str()); // CORRECT

    }

	if("check if sync-db has changes but not tables and index") {
        auto syncDb = OpenECDb(synDbFile.c_str());
        auto pipe1 = syncDb->Schemas().GetClass("TestSchema1", "Pipe1");
        ASSERT_NE(pipe1, nullptr);
        ASSERT_EQ(pipe1->GetPropertyCount(), 4);
        ASSERT_STRCASEEQ(getIndexDDL(*syncDb, "idx_pipe1_p1").c_str(), "");
        EXPECT_STREQ("5cbd24c81c6f5bd432a3c4829b4691c53977e490", GetSchemaHash(*syncDb).c_str());   // CORRECT
        EXPECT_STREQ("48c61859858c52efb43c1c407cf39051dd2a983d", GetMapHash(*syncDb).c_str());      // WRONG
        EXPECT_STREQ("c6f37c87a652dbeb8b0413ad3dca36b59e8b273b", GetDbSchemaHash(*syncDb).c_str()); // CORRECT It should not change
    }

	if("pull changes from sync db into master db and check if schema changes was there and valid") {
        pullChangesFromSyncDb(*b1);
        auto pipe1 = b1->Schemas().GetClass("TestSchema1", "Pipe1");
        ASSERT_NE(pipe1, nullptr);
        ASSERT_EQ(pipe1->GetPropertyCount(), 4);
        ASSERT_TRUE(b1->TableExists("ts_Pipe1"));
        ASSERT_STRCASEEQ(getIndexDDL(*b1, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1], [p2])");
        EXPECT_STREQ("5cbd24c81c6f5bd432a3c4829b4691c53977e490", GetSchemaHash(*b1).c_str());   // CORRECT
        EXPECT_STREQ("48c61859858c52efb43c1c407cf39051dd2a983d", GetMapHash(*b1).c_str());      // WRONG
        EXPECT_STREQ("a0ecfd5f3845cc756be16b9649b1a3d5d8be3325", GetDbSchemaHash(*b1).c_str()); // CORRECT
    }
}


END_ECDBUNITTESTS_NAMESPACE
