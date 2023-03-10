/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"
#include "ECDbPublishedTests.h"
#include <numeric>
#include "MockHubApi.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
BEGIN_ECDBUNITTESTS_NAMESPACE
/**
 * A IModel that use shared schema channel can only update/import schema via shared schema channel.
 * IModel will not allow schema import unless shared schema channel is setup up.
 * IModel will contain be_prop that will let ECDb know to not allow direct schema import call instead
 * it will require to call ImportSchemasViaSharedChannel()
*/
const char* DEFAULT_SHA3_256_ECDB_SCHEMA = "44c5d675cdab562b732a90b8c0128149daaa7a2beefbcbddb576f7bf059cec33";
const char* DEFAULT_SHA3_256_ECDB_MAP = "9c7834d13177336f0fa57105b9c1175b912b2e12e62ca2224482c0ffd9dfd337";
const char* DEFAULT_SHA3_256_SQLITE_SCHEMA = "c4ca1cdd07de041e71f3e8d4b1942d29da89653c85276025d786688b6f576443";

TEST_F(SchemaSyncTestFixture, checksum_pragma) {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();

    // PrintHash(*b1, "checksum for seed file");

    Test("schema checksum", [&]() {
        ECSqlStatement stmt;
        EXPECT_EQ(stmt.Prepare(*b1, "PRAGMA checksum(ecdb_schema)"), ECSqlStatus::Success);
        EXPECT_EQ(stmt.Step(), BE_SQLITE_ROW);
        EXPECT_STREQ(stmt.GetValueText(0), DEFAULT_SHA3_256_ECDB_SCHEMA);
    });

    Test("map checksum", [&]() {
        ECSqlStatement stmt;
        EXPECT_EQ(stmt.Prepare(*b1, "PRAGMA checksum(ecdb_map)"), ECSqlStatus::Success);
        EXPECT_EQ(stmt.Step(), BE_SQLITE_ROW);
        EXPECT_STREQ(stmt.GetValueText(0), DEFAULT_SHA3_256_ECDB_MAP);
    });

    Test("sqlite schema checksum", [&]() {
        ECSqlStatement stmt;
        EXPECT_EQ(stmt.Prepare(*b1, "PRAGMA checksum(sqlite_schema)"), ECSqlStatus::Success);
        EXPECT_EQ(stmt.Step(), BE_SQLITE_ROW);
        EXPECT_STREQ(stmt.GetValueText(0), DEFAULT_SHA3_256_SQLITE_SCHEMA);
    });
}


TEST_F(SchemaSyncTestFixture, Test) {
    ECDbHub hub;
    SharedSchemaDb schemaChannel("sync-db");
    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SharedSchemaChannel::Status::SUCCESS,
        b1->Schemas().GetSharedChannel().Init(schemaChannel.GetChannelUri()));

    b1->PullMergePush("init");
    b1->SaveChanges();

    auto b2 = hub.CreateBriefcase();
    auto b3 = hub.CreateBriefcase();


    ASSERT_EQ(b1->GetBriefcaseId().GetValue(), 11);
    ASSERT_EQ(b2->GetBriefcaseId().GetValue(), 12);
    ASSERT_EQ(b3->GetBriefcaseId().GetValue(), 13);


    if ("check syn db hash") {
        schemaChannel.WithReadOnly([&](ECDbR syncDb) {
            EXPECT_STREQ(DEFAULT_SHA3_256_ECDB_SCHEMA, GetSchemaHash(syncDb).c_str());
            EXPECT_STREQ(DEFAULT_SHA3_256_ECDB_MAP, GetMapHash(syncDb).c_str());
            EXPECT_STREQ(DEFAULT_SHA3_256_SQLITE_SCHEMA, GetDbSchemaHash(syncDb).c_str());
        });
    }

    EXPECT_STREQ(DEFAULT_SHA3_256_ECDB_SCHEMA, GetSchemaHash(*b1).c_str());
    EXPECT_STREQ(DEFAULT_SHA3_256_ECDB_MAP, GetMapHash(*b1).c_str());
    EXPECT_STREQ(DEFAULT_SHA3_256_SQLITE_SCHEMA, GetDbSchemaHash(*b1).c_str());

    EXPECT_STREQ(DEFAULT_SHA3_256_ECDB_SCHEMA, GetSchemaHash(*b2).c_str());
    EXPECT_STREQ(DEFAULT_SHA3_256_ECDB_MAP, GetMapHash(*b2).c_str());
    EXPECT_STREQ(DEFAULT_SHA3_256_SQLITE_SCHEMA, GetDbSchemaHash(*b2).c_str());

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "57df4675ccbce3493d2bb882ad3bb28f3266425c2f22fd55e57e187808b3add3";
    const auto SCHEMA1_HASH_ECDB_MAP = "8b1c6d8fa5b29e085bf94fae710527f56fa1c1792bd7404ff5775ed07f86f21f";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "8608aab5fa8a874b3f9140451ab8410c785483a878c8d915f48a26ef20e8241c";
    Test("import schema into b1", [&]() {
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
        ASSERT_EQ (SchemaImportResult::OK, ImportSchema(*b1, schema1, SchemaManager::SchemaImportOptions::None, schemaChannel.GetChannelUri()));
        ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

        ASSERT_TRUE(b1->TableExists("ts_Pipe1"));
        ASSERT_STRCASEEQ(GetIndexDDL(*b1, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1])");
        EXPECT_STREQ(SCHEMA1_HASH_ECDB_SCHEMA, GetSchemaHash(*b1).c_str());
        EXPECT_STREQ(SCHEMA1_HASH_ECDB_MAP, GetMapHash(*b1).c_str());
        EXPECT_STREQ(SCHEMA1_HASH_SQLITE_SCHEMA, GetDbSchemaHash(*b1).c_str());
    });

    Test("check if sync-db has changes but not tables and index", [&]() {
        schemaChannel.WithReadOnly([&](ECDbR syncDb) {
            auto pipe1 = syncDb.Schemas().GetClass("TestSchema1", "Pipe1");
            ASSERT_NE(pipe1, nullptr);
            ASSERT_EQ(pipe1->GetPropertyCount(), 2);
            ASSERT_FALSE(syncDb.TableExists("ts_Pipe1"));
            ASSERT_STRCASEEQ(GetIndexDDL(syncDb, "idx_pipe1_p1").c_str(), "");
            EXPECT_STREQ(SCHEMA1_HASH_ECDB_SCHEMA, GetSchemaHash(syncDb).c_str());
            EXPECT_STREQ(SCHEMA1_HASH_ECDB_MAP, GetMapHash(syncDb).c_str());
            EXPECT_STREQ(DEFAULT_SHA3_256_SQLITE_SCHEMA, GetDbSchemaHash(syncDb).c_str()); // It should not change
            ASSERT_TRUE(ForeignkeyCheck(syncDb));
        });
    });

    Test("pull changes from sync-db into b2 and verify class, table and index exists", [&]() {
        ASSERT_EQ(SharedSchemaChannel::Status::SUCCESS, schemaChannel.Pull(*b2,[&](){
            auto pipe1 = b2->Schemas().GetClass("TestSchema1", "Pipe1");
            ASSERT_NE(pipe1, nullptr);
            ASSERT_EQ(pipe1->GetPropertyCount(), 2);
            ASSERT_TRUE(b2->TableExists("ts_Pipe1"));
            ASSERT_STRCASEEQ(GetIndexDDL(*b2, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1])");
            EXPECT_STREQ(SCHEMA1_HASH_ECDB_SCHEMA, GetSchemaHash(*b2).c_str());
            EXPECT_STREQ(SCHEMA1_HASH_ECDB_MAP, GetMapHash(*b2).c_str());
            EXPECT_STREQ(SCHEMA1_HASH_SQLITE_SCHEMA, GetDbSchemaHash(*b2).c_str());
            ASSERT_TRUE(ForeignkeyCheck(*b2));
        })) << "Pull changes from schemaChannel into b2";
    });

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "4a674e4d762c6960fa5276c7395d813c786dc4023843674e93548e9f9ad8cd62";
    const auto SCHEMA2_HASH_ECDB_MAP = "d9bb9b10a0b3745b1878eff131e692e7930d34883ae52506b5be23bd4e8d2b5f";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "a5903dad8066700b537ea5f939043e8d8cbfe1297ea6fa0c3e20d7c00e5e3d44";
    Test("update schema by adding more properties and expand index in b2", [&]() {
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
        ASSERT_EQ (SchemaImportResult::OK, ImportSchema(*b2, schema2, SchemaManager::SchemaImportOptions::None, schemaChannel.GetChannelUri()));
        ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

        ASSERT_TRUE(b2->TableExists("ts_Pipe1"));
        ASSERT_STRCASEEQ(GetIndexDDL(*b2, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1], [p2])");
        EXPECT_STREQ(SCHEMA2_HASH_ECDB_SCHEMA, GetSchemaHash(*b2).c_str());
        EXPECT_STREQ(SCHEMA2_HASH_ECDB_MAP, GetMapHash(*b2).c_str());
        EXPECT_STREQ(SCHEMA2_HASH_SQLITE_SCHEMA, GetDbSchemaHash(*b2).c_str());
    });

    Test("check if sync-db has changes but not tables and index", [&]() {
        schemaChannel.WithReadOnly([&](ECDbR syncDb) {
            auto pipe1 = syncDb.Schemas().GetClass("TestSchema1", "Pipe1");
            ASSERT_NE(pipe1, nullptr);
            ASSERT_EQ(pipe1->GetPropertyCount(), 4);
            ASSERT_STRCASEEQ(GetIndexDDL(syncDb, "idx_pipe1_p1").c_str(), "");
            EXPECT_STREQ(SCHEMA2_HASH_ECDB_SCHEMA, GetSchemaHash(syncDb).c_str());
            EXPECT_STREQ(SCHEMA2_HASH_ECDB_MAP, GetMapHash(syncDb).c_str());
            EXPECT_STREQ(DEFAULT_SHA3_256_SQLITE_SCHEMA, GetDbSchemaHash(syncDb).c_str());
            ASSERT_TRUE(ForeignkeyCheck(syncDb));
        });
    });

    Test("pull changes from sync db into master db and check if schema changes was there and valid", [&]() {
        ASSERT_EQ(SharedSchemaChannel::Status::SUCCESS, schemaChannel.Pull(*b1, [&]() {
            auto pipe1 = b1->Schemas().GetClass("TestSchema1", "Pipe1");
            ASSERT_NE(pipe1, nullptr);
            ASSERT_EQ(pipe1->GetPropertyCount(), 4);

            ASSERT_TRUE(b1->TableExists("ts_Pipe1"));
            ASSERT_STRCASEEQ(GetIndexDDL(*b1, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1], [p2])");

            EXPECT_STREQ(SCHEMA2_HASH_ECDB_SCHEMA, GetSchemaHash(*b1).c_str());
            EXPECT_STREQ(SCHEMA2_HASH_ECDB_MAP, GetMapHash(*b1).c_str());
            EXPECT_STREQ(SCHEMA2_HASH_SQLITE_SCHEMA, GetDbSchemaHash(*b1).c_str());
            ASSERT_TRUE(ForeignkeyCheck(*b1));
        })) << "Pull changes from schemaChannel into b1";
    });

    Test("PullMergePush for b1", [&]() {
        ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("b1 import schema"))  << "b1->PullMergePush()";
    });

    Test("PullMergePush for b2", [&]() {
        ASSERT_EQ(BE_SQLITE_OK, b2->PullMergePush("b2 import schema")) << "b2->PullMergePush()";
        b2->SaveChanges();
    });

    Test("b3 pull changes from hub", [&]() {
        ASSERT_EQ(BE_SQLITE_OK, b3->PullMergePush("add new schema"))  << "b3->PullMergePush()";
        auto pipe1 = b3->Schemas().GetClass("TestSchema1", "Pipe1");
        ASSERT_NE(pipe1, nullptr);
        ASSERT_EQ(pipe1->GetPropertyCount(), 4);
        ASSERT_TRUE(b3->TableExists("ts_Pipe1"));
        ASSERT_STRCASEEQ(GetIndexDDL(*b3, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1], [p2])");
        EXPECT_STREQ(SCHEMA2_HASH_ECDB_SCHEMA, GetSchemaHash(*b3).c_str());
        EXPECT_STREQ(SCHEMA2_HASH_ECDB_MAP, GetMapHash(*b3).c_str());
        EXPECT_STREQ(SCHEMA2_HASH_SQLITE_SCHEMA, GetDbSchemaHash(*b3).c_str());
    });
}

END_ECDBUNITTESTS_NAMESPACE
