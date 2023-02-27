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


TEST_F(SchemaSyncTestFixture, Test) {
    ECDbHub hub;
    SharedSchemaDb schemaChannel("sync-db");
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    auto b3 = hub.CreateBriefcase();


    ASSERT_EQ(b1->GetBriefcaseId().GetValue(), 11);
    ASSERT_EQ(b2->GetBriefcaseId().GetValue(), 12);
    ASSERT_EQ(b3->GetBriefcaseId().GetValue(), 13);


    ASSERT_EQ(BE_SQLITE_OK, b1->Schemas().InitSharedSchemaDb(
        schemaChannel.GetFileName().GetNameUtf8()));

    if ("check syn db hash") {
        schemaChannel.WithReadOnly([&](ECDbR syncDb) {
            EXPECT_STREQ("9a0c2041a498d674fc0045cb0209e3dd8951b904", GetSchemaHash(syncDb).c_str());
            EXPECT_STREQ("49675fee5c0f1264853df55142364516b5ef5202", GetMapHash(syncDb).c_str());
            EXPECT_STREQ("c6f37c87a652dbeb8b0413ad3dca36b59e8b273b", GetDbSchemaHash(syncDb).c_str());
        });
    }

    EXPECT_STREQ("9a0c2041a498d674fc0045cb0209e3dd8951b904", GetSchemaHash(*b1).c_str());
    EXPECT_STREQ("49675fee5c0f1264853df55142364516b5ef5202", GetMapHash(*b1).c_str());
    EXPECT_STREQ("c6f37c87a652dbeb8b0413ad3dca36b59e8b273b", GetDbSchemaHash(*b1).c_str());

    EXPECT_STREQ("9a0c2041a498d674fc0045cb0209e3dd8951b904", GetSchemaHash(*b2).c_str());
    EXPECT_STREQ("49675fee5c0f1264853df55142364516b5ef5202", GetMapHash(*b2).c_str());
    EXPECT_STREQ("c6f37c87a652dbeb8b0413ad3dca36b59e8b273b", GetDbSchemaHash(*b2).c_str());

    ASSERT_EQ(BE_SQLITE_OK, schemaChannel.Push(*b1)) << "Push changes to from b1 to schemaChannel";
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
        ASSERT_EQ (SchemaImportResult::OK, ImportSchema(*b1, schema1));
        ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
        ASSERT_EQ(BE_SQLITE_OK, schemaChannel.Push(*b1)) << "Push changes to from b1 to schemaChannel";

        ASSERT_TRUE(b1->TableExists("ts_Pipe1"));
        ASSERT_STRCASEEQ(GetIndexDDL(*b1, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1])");

        EXPECT_STREQ("58378b2ff3c9c4ad9664395274173d2c9dee1970", GetSchemaHash(*b1).c_str());
        EXPECT_STREQ("fe4dfa5552f7a28b49e003ba2d2989ef3380538b", GetMapHash(*b1).c_str());
        EXPECT_STREQ("c7c7a104e2197c9aeb95e76534318a1c6ecc68d7", GetDbSchemaHash(*b1).c_str());
    });

    Test("check if sync-db has changes but not tables and index", [&]() {
        schemaChannel.WithReadOnly([&](ECDbR syncDb) {
            auto pipe1 = syncDb.Schemas().GetClass("TestSchema1", "Pipe1");
            ASSERT_NE(pipe1, nullptr);
            ASSERT_EQ(pipe1->GetPropertyCount(), 2);
            ASSERT_FALSE(syncDb.TableExists("ts_Pipe1"));
            ASSERT_STRCASEEQ(GetIndexDDL(syncDb, "idx_pipe1_p1").c_str(), "");
            EXPECT_STREQ("58378b2ff3c9c4ad9664395274173d2c9dee1970", GetSchemaHash(syncDb).c_str());
            EXPECT_STREQ("fe4dfa5552f7a28b49e003ba2d2989ef3380538b", GetMapHash(syncDb).c_str());
            EXPECT_STREQ("c6f37c87a652dbeb8b0413ad3dca36b59e8b273b", GetDbSchemaHash(syncDb).c_str()); // It should not change
            ASSERT_TRUE(ForeignkeyCheck(syncDb));
        });
    });

    Test("pull changes from sync-db into b2 and verify class, table and index exists", [&]() {
        ASSERT_EQ(BE_SQLITE_OK, schemaChannel.Pull(*b2,[&](){
            auto pipe1 = b2->Schemas().GetClass("TestSchema1", "Pipe1");
            ASSERT_NE(pipe1, nullptr);
            ASSERT_EQ(pipe1->GetPropertyCount(), 2);
            ASSERT_TRUE(b2->TableExists("ts_Pipe1"));
            ASSERT_STRCASEEQ(GetIndexDDL(*b2, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1])");
            EXPECT_STREQ("58378b2ff3c9c4ad9664395274173d2c9dee1970", GetSchemaHash(*b2).c_str());
            EXPECT_STREQ("fe4dfa5552f7a28b49e003ba2d2989ef3380538b", GetMapHash(*b2).c_str());
            EXPECT_STREQ("c7c7a104e2197c9aeb95e76534318a1c6ecc68d7", GetDbSchemaHash(*b2).c_str());
            ASSERT_TRUE(ForeignkeyCheck(*b2));
        })) << "Pull changes from schemaChannel into b2";
    });

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
        ASSERT_EQ (SchemaImportResult::OK, ImportSchema(*b2, schema2));
        ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
        ASSERT_EQ(BE_SQLITE_OK, schemaChannel.Push(*b2)) << "Push changes to from b1 to schemaChannel";

        ASSERT_TRUE(b2->TableExists("ts_Pipe1"));
        ASSERT_STRCASEEQ(GetIndexDDL(*b2, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1], [p2])");

        EXPECT_STREQ("5cbd24c81c6f5bd432a3c4829b4691c53977e490", GetSchemaHash(*b2).c_str());
        EXPECT_STREQ("5775dbd6f8956d6cb25e5f55c57948eaeefd7a42", GetMapHash(*b2).c_str());
        EXPECT_STREQ("a0ecfd5f3845cc756be16b9649b1a3d5d8be3325", GetDbSchemaHash(*b2).c_str());
    });

    Test("check if sync-db has changes but not tables and index", [&]() {
        schemaChannel.WithReadOnly([&](ECDbR syncDb) {
            auto pipe1 = syncDb.Schemas().GetClass("TestSchema1", "Pipe1");
            ASSERT_NE(pipe1, nullptr);
            ASSERT_EQ(pipe1->GetPropertyCount(), 4);
            ASSERT_STRCASEEQ(GetIndexDDL(syncDb, "idx_pipe1_p1").c_str(), "");
            EXPECT_STREQ("5cbd24c81c6f5bd432a3c4829b4691c53977e490", GetSchemaHash(syncDb).c_str());
            EXPECT_STREQ("5775dbd6f8956d6cb25e5f55c57948eaeefd7a42", GetMapHash(syncDb).c_str());
            EXPECT_STREQ("c6f37c87a652dbeb8b0413ad3dca36b59e8b273b", GetDbSchemaHash(syncDb).c_str());
            ASSERT_TRUE(ForeignkeyCheck(syncDb));
        });
    });

    Test("pull changes from sync db into master db and check if schema changes was there and valid", [&]() {
        ASSERT_EQ(BE_SQLITE_OK, schemaChannel.Pull(*b1, [&]() {
            auto pipe1 = b1->Schemas().GetClass("TestSchema1", "Pipe1");
            ASSERT_NE(pipe1, nullptr);
            ASSERT_EQ(pipe1->GetPropertyCount(), 4);

            ASSERT_TRUE(b1->TableExists("ts_Pipe1"));
            ASSERT_STRCASEEQ(GetIndexDDL(*b1, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1], [p2])");

            EXPECT_STREQ("5cbd24c81c6f5bd432a3c4829b4691c53977e490", GetSchemaHash(*b1).c_str());
            EXPECT_STREQ("5775dbd6f8956d6cb25e5f55c57948eaeefd7a42", GetMapHash(*b1).c_str());
            EXPECT_STREQ("a0ecfd5f3845cc756be16b9649b1a3d5d8be3325", GetDbSchemaHash(*b1).c_str());
            ASSERT_TRUE(ForeignkeyCheck(*b1));
        })) << "Pull changes from schemaChannel into b1";
    });

    Test("PullMergePush for b1", [&]() {
        ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("b1 import schema"))  << "b1->PullMergePush()";
    });

    Test("b3 pull changes from hub", [&]() {
        ASSERT_EQ(BE_SQLITE_OK, b3->PullMergePush("add new schema"))  << "b3->PullMergePush()";
        auto pipe1 = b3->Schemas().GetClass("TestSchema1", "Pipe1");
        ASSERT_NE(pipe1, nullptr);
        ASSERT_EQ(pipe1->GetPropertyCount(), 4);
        ASSERT_TRUE(b3->TableExists("ts_Pipe1"));
        ASSERT_STRCASEEQ(GetIndexDDL(*b3, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1], [p2])");
        EXPECT_STREQ("5cbd24c81c6f5bd432a3c4829b4691c53977e490", GetSchemaHash(*b3).c_str());
        EXPECT_STREQ("5775dbd6f8956d6cb25e5f55c57948eaeefd7a42", GetMapHash(*b3).c_str());
        EXPECT_STREQ("a0ecfd5f3845cc756be16b9649b1a3d5d8be3325", GetDbSchemaHash(*b3).c_str());
    });


    Test("PullMergePush for b2", [&]() {
        ASSERT_EQ(BE_SQLITE_OK, b2->PullMergePush("b2 import schema")) << "b2->PullMergePush()";
        b2->AbandonChanges();
    });

    
}

END_ECDBUNITTESTS_NAMESPACE
