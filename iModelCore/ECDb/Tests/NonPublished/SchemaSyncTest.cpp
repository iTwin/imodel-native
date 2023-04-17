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

auto schemaXMLBuilder = [&](Utf8CP newSchemaVersion = "01.00.00")
    {
    Utf8CP schemaTemplate =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema1" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
        </ECSchema>)xml";
    Utf8String schemaXml;
    schemaXml.Sprintf(schemaTemplate, newSchemaVersion);
    return schemaXml;
    };

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, checksum_pragma)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();

    // PrintHash(*b1, "checksum for seed file");

    Test(
        "schema checksum",
        [&]()
            {
            ECSqlStatement stmt;
            EXPECT_EQ(stmt.Prepare(*b1, "PRAGMA checksum(ecdb_schema)"), ECSqlStatus::Success);
            EXPECT_EQ(stmt.Step(), BE_SQLITE_ROW);
            EXPECT_STREQ(stmt.GetValueText(0), DEFAULT_SHA3_256_ECDB_SCHEMA);
            }
    );

    Test(
        "map checksum",
        [&]()
            {
            ECSqlStatement stmt;
            EXPECT_EQ(stmt.Prepare(*b1, "PRAGMA checksum(ecdb_map)"), ECSqlStatus::Success);
            EXPECT_EQ(stmt.Step(), BE_SQLITE_ROW);
            EXPECT_STREQ(stmt.GetValueText(0), DEFAULT_SHA3_256_ECDB_MAP);
            }
    );

    Test(
        "sqlite schema checksum",
        [&]()
            {
            ECSqlStatement stmt;
            EXPECT_EQ(stmt.Prepare(*b1, "PRAGMA checksum(sqlite_schema)"), ECSqlStatus::Success);
            EXPECT_EQ(stmt.Step(), BE_SQLITE_ROW);
            EXPECT_STREQ(stmt.GetValueText(0), DEFAULT_SHA3_256_SQLITE_SCHEMA);
            }
    );
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, FullSchemaSyncWorkflow)
    {
    ECDbHub hub;
    SharedSchemaDb schemaChannel("sync-db");
    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SharedSchemaChannel::Status::OK, b1->Schemas().GetSharedChannel().Init(schemaChannel.GetChannelUri()));

    b1->PullMergePush("init");
    b1->SaveChanges();

    auto b2 = hub.CreateBriefcase();
    auto b3 = hub.CreateBriefcase();

    Test(
        "Check briefcase ids",
        [&]()
            {
            ASSERT_EQ(b1->GetBriefcaseId().GetValue(), 11);
            ASSERT_EQ(b2->GetBriefcaseId().GetValue(), 12);
            ASSERT_EQ(b3->GetBriefcaseId().GetValue(), 13);
            }
    );

    Test(
        "check syncDb, b1, b2 and b3 hashes",
        [&]()
            {
            schemaChannel.WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb); });
            CheckHashes(*b1);
            CheckHashes(*b2);
            CheckHashes(*b3);
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "57df4675ccbce3493d2bb882ad3bb28f3266425c2f22fd55e57e187808b3add3";
    const auto SCHEMA1_HASH_ECDB_MAP = "8b1c6d8fa5b29e085bf94fae710527f56fa1c1792bd7404ff5775ed07f86f21f";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "8608aab5fa8a874b3f9140451ab8410c785483a878c8d915f48a26ef20e8241c";
    Test(
        "import schema into b1",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, schemaChannel.GetChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

            ASSERT_TRUE(b1->TableExists("ts_Pipe1"));
            ASSERT_STRCASEEQ(GetIndexDDL(*b1, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1])");
            CheckHashes(*b1, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            }
    );

    Test(
        "check if sync-db has changes but not tables and index",
        [&]()
            {
            schemaChannel.WithReadOnly(
                [&](ECDbR syncDb)
                    {
                    auto pipe1 = syncDb.Schemas().GetClass("TestSchema1", "Pipe1");
                    ASSERT_NE(pipe1, nullptr);
                    ASSERT_EQ(pipe1->GetPropertyCount(), 2);
                    ASSERT_FALSE(syncDb.TableExists("ts_Pipe1"));
                    ASSERT_STRCASEEQ(GetIndexDDL(syncDb, "idx_pipe1_p1").c_str(), "");
                    CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); // SQLITE_SCHEMA hash should not change
                    ASSERT_TRUE(ForeignkeyCheck(syncDb));
                    }
            );
            }
    );

    Test(
        "pull changes from sync-db into b2 and verify class, table and index exists",
        [&]()
            {
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                schemaChannel.Pull(
                    *b2,
                    [&]()
                        {
                        auto pipe1 = b2->Schemas().GetClass("TestSchema1", "Pipe1");
                        ASSERT_NE(pipe1, nullptr);
                        ASSERT_EQ(pipe1->GetPropertyCount(), 2);
                        ASSERT_TRUE(b2->TableExists("ts_Pipe1"));
                        ASSERT_STRCASEEQ(GetIndexDDL(*b2, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1])");
                        CheckHashes(*b2, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*b2));
                        }
                )
            ) << "Pull changes from schemaChannel into b2";
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "4a674e4d762c6960fa5276c7395d813c786dc4023843674e93548e9f9ad8cd62";
    const auto SCHEMA2_HASH_ECDB_MAP = "d9bb9b10a0b3745b1878eff131e692e7930d34883ae52506b5be23bd4e8d2b5f";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "a5903dad8066700b537ea5f939043e8d8cbfe1297ea6fa0c3e20d7c00e5e3d44";
    Test(
        "update schema by adding more properties and expand index in b2",
        [&]()
            {
            auto schema2 = SchemaItem(
                R"xml(<?xml version="1.0" encoding="UTF-8"?>
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
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b2, schema2, SchemaManager::SchemaImportOptions::None, schemaChannel.GetChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

            ASSERT_TRUE(b2->TableExists("ts_Pipe1"));
            ASSERT_STRCASEEQ(GetIndexDDL(*b2, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1], [p2])");
            CheckHashes(*b2, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            }
    );

    Test(
        "check if sync-db has changes but not tables and index",
        [&]()
            {
            schemaChannel.WithReadOnly(
                [&](ECDbR syncDb)
                    {
                    auto pipe1 = syncDb.Schemas().GetClass("TestSchema1", "Pipe1");
                    ASSERT_NE(pipe1, nullptr);
                    ASSERT_EQ(pipe1->GetPropertyCount(), 4);
                    ASSERT_STRCASEEQ(GetIndexDDL(syncDb, "idx_pipe1_p1").c_str(), "");
                    CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP);
                    ASSERT_TRUE(ForeignkeyCheck(syncDb));
                    }
            );
            }
    );

    Test(
        "pull changes from sync db into master db and check if schema changes was there and valid",
        [&]()
            {
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                schemaChannel.Pull(
                    *b1,
                    [&]()
                        {
                        auto pipe1 = b1->Schemas().GetClass("TestSchema1", "Pipe1");
                        ASSERT_NE(pipe1, nullptr);
                        ASSERT_EQ(pipe1->GetPropertyCount(), 4);

                        ASSERT_TRUE(b1->TableExists("ts_Pipe1"));
                        ASSERT_STRCASEEQ(GetIndexDDL(*b1, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1], [p2])");

                        CheckHashes(*b1, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*b1));
                        }
                )
            ) << "Pull changes from schemaChannel into b1";
            }
    );

    Test("PullMergePush for b1", [&]() { ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("b1 import schema")) << "b1->PullMergePush()"; });

    Test(
        "PullMergePush for b2",
        [&]()
            {
            ASSERT_EQ(BE_SQLITE_OK, b2->PullMergePush("b2 import schema")) << "b2->PullMergePush()";
            b2->SaveChanges();
            }
    );

    Test(
        "b3 pull changes from hub",
        [&]()
            {
            ASSERT_EQ(BE_SQLITE_OK, b3->PullMergePush("add new schema"))  << "b3->PullMergePush()";
            auto pipe1 = b3->Schemas().GetClass("TestSchema1", "Pipe1");
            ASSERT_NE(pipe1, nullptr);
            ASSERT_EQ(pipe1->GetPropertyCount(), 4);
            ASSERT_TRUE(b3->TableExists("ts_Pipe1"));
            ASSERT_STRCASEEQ(GetIndexDDL(*b3, "idx_pipe1_p1").c_str(), "CREATE INDEX [idx_pipe1_p1] ON [ts_Pipe1]([p1], [p2])");
            EXPECT_STREQ(SCHEMA2_HASH_ECDB_SCHEMA, GetSchemaHash(*b3).c_str());
            EXPECT_STREQ(SCHEMA2_HASH_ECDB_MAP, GetMapHash(*b3).c_str());
            EXPECT_STREQ(SCHEMA2_HASH_SQLITE_SCHEMA, GetDbSchemaHash(*b3).c_str());
            }
    );
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, PushSchemaWithoutInitializingSchemaChannel)
    {
    ECDbHub hub;
    SharedSchemaDb schemaChannel("sync-db");
    auto b1 = hub.CreateBriefcase();

    Test(
        "Create and import schema to schemaChannel without initializing schemaChannel",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*b1, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, schemaChannel.GetChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            CheckHashes(*b1);
            schemaChannel.WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb); });
            }
    );
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// First case is it correct?
// Technically briefcase doesn't have shared schema channel and no Channel Uri is passed, but should it return OK tho, even if it doesn't push to anything?
TEST_F(SchemaSyncTestFixture, InvalidSchemaChannel)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();

    Test(
        "Empty schemaChannel",
        [&]()
            {
            const auto SCHEMA1_HASH_ECDB_SCHEMA = "57df4675ccbce3493d2bb882ad3bb28f3266425c2f22fd55e57e187808b3add3";
            const auto SCHEMA1_HASH_ECDB_MAP = "8b1c6d8fa5b29e085bf94fae710527f56fa1c1792bd7404ff5775ed07f86f21f";
            const auto SCHEMA1_HASH_SQLITE_SCHEMA = "8608aab5fa8a874b3f9140451ab8410c785483a878c8d915f48a26ef20e8241c";

            // Saves changes locally
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, (SharedSchemaChannel::ChannelUri) ""));
            CheckHashes(*b1, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            }
    );

    Test(
        "Semicolon schemaChannel",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*b1, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, (SharedSchemaChannel::ChannelUri) ";"));
            CheckHashes(*b1);
            ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            }
    );

    Test(
        "Space schemaChannel",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*b1, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, (SharedSchemaChannel::ChannelUri) " "));
            ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            CheckHashes(*b1);
            }
    );

    Test(
        "Not a correct file extention schemaChannel",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*b1, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, (SharedSchemaChannel::ChannelUri) "Z:\\Test\\Location\\fake-file.exe"));
            ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            CheckHashes(*b1);
            }
    );

    Test(
        "Not a filename schemaChannel",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*b1, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, (SharedSchemaChannel::ChannelUri) "fkj3lakjflakf90asfbnasghaklg3akglk4j;glkja;lkgj4;3lkgkanbkj4rtjak5;lkgjak4j3lktjalksdjg;lkj;lfj2qkj2qoipoa4tnb;"));
            ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            CheckHashes(*b1);
            }
    );

    Test(
        "All illegal file characters",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*b1, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, (SharedSchemaChannel::ChannelUri) "# % & { } \\ < > * \? / $ ! \' \" : @ + ` | ="));
            ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            CheckHashes(*b1);
            }
    );
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, InvalidSchemaChannelWithInitializedSharedChannel)
    {
    ECDbHub hub;
    auto b1 = hub.CreateBriefcase();
    SharedSchemaDb schemaChannel("sync-db");

    ASSERT_EQ(SharedSchemaChannel::Status::OK, b1->Schemas().GetSharedChannel().Init(schemaChannel.GetChannelUri()));
    b1->PullMergePush("init");
    b1->SaveChanges();

    Test(
        "Initialized schema and empty schemaChannel",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*b1, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, (SharedSchemaChannel::ChannelUri) ""));
            ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            CheckHashes(*b1);
            schemaChannel.WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb); });
            }
    );

    Test(
        "Initialized schema and semicolon schemaChannel",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*b1, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, (SharedSchemaChannel::ChannelUri) ";"));
            ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            CheckHashes(*b1);
            schemaChannel.WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb); });
            }
    );

    Test(
        "Initialized schema and space schemaChannel",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*b1, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, (SharedSchemaChannel::ChannelUri) " "));
            ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            CheckHashes(*b1);
            schemaChannel.WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb); });
            }
    );

    Test(
        "Initialized schema and not a correct file extention schemaChannel",
        [&]()
            {
            ASSERT_EQ(
                SchemaImportResult::ERROR,
                ImportSchema(*b1, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, (SharedSchemaChannel::ChannelUri) "Z:\\Test\\Location\\fake-file.exe")
            );
            ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            CheckHashes(*b1);
            schemaChannel.WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb); });
            }
    );

    Test(
        "Initialized schema and not a filename schemaChannel",
        [&]()
            {
            ASSERT_EQ(
                SchemaImportResult::ERROR,
                ImportSchema(
                    *b1,
                    SchemaItem(schemaXMLBuilder()),
                    SchemaManager::SchemaImportOptions::None,
                    (SharedSchemaChannel::ChannelUri) "fkj3lakjflakf90asfbnasghaklg3akglk4j;glkja;lkgj4;3lkgkanbkj4rtjak5;lkgjak4j3lktjalksdjg;lkj;lfj2qkj2qoipoa4tnb;"
                )
            );
            ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            CheckHashes(*b1);
            schemaChannel.WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb); });
            }
    );

    Test(
        "Initialized schema and all illegal file characters",
        [&]()
            {
            ASSERT_EQ(
                SchemaImportResult::ERROR,
                ImportSchema(
                    *b1,
                    SchemaItem(schemaXMLBuilder()),
                    SchemaManager::SchemaImportOptions::None,
                    (SharedSchemaChannel::ChannelUri) "# % & { } \\ < > * \? / $ ! \' \" : @ + ` | ="
                )
            );
            ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            CheckHashes(*b1);
            schemaChannel.WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb); });
            }
    );
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// Should dis work?
TEST_F(SchemaSyncTestFixture, BriefcasePushesSchemaWithInvalidImportOptions)
    {
    Test(
        "Initialize shared schema channel with no import options",
        [&]() { ASSERT_EQ(SchemaImportResult::OK, SetupECDb("sync-db", SchemaItem(schemaXMLBuilder()))); }
    );

    Test(
        "Import schema with large negative SchemaImportOption",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(SchemaItem(schemaXMLBuilder()), (SchemaManager::SchemaImportOptions) -100000011));
            // ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            }
    );

    Test(
        "Import schema with small negative SchemaImportOption",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(SchemaItem(schemaXMLBuilder()), (SchemaManager::SchemaImportOptions) -1));
            // ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            }
    );

    Test(
        "Import schema with small SchemaImportOption between valid ones",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(SchemaItem(schemaXMLBuilder()), (SchemaManager::SchemaImportOptions) 3));
            // ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            }
    );

    Test(
        "Import schema with bigger positive SchemaImportOption",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(SchemaItem(schemaXMLBuilder()), (SchemaManager::SchemaImportOptions) 100));
            // ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            }
    );

    Test(
        "Import schema with large positive SchemaImportOption",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(SchemaItem(schemaXMLBuilder()), (SchemaManager::SchemaImportOptions) 1000000));
            // ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            }
    );
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, PushSchemaToNewSchemaChannelWhenExistingSchemaChannelIsInitialized)
    {
    ECDbHub hub;
    SharedSchemaDb schemaChannel("sync-db");
    SharedSchemaChannel::ChannelUri emptyUri;
    auto b1 = hub.CreateBriefcase();

    ASSERT_EQ(SharedSchemaChannel::Status::OK, b1->Schemas().GetSharedChannel().Init(schemaChannel.GetChannelUri()));
    b1->PullMergePush("init");
    b1->SaveChanges();

    Test(
        "Create and import schema with no schemaChannel",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*b1, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, emptyUri));
            CheckHashes(*b1);
            schemaChannel.WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb); });
            ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            }
    );
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// TODO: needs fixing at "wut"
TEST_F(SchemaSyncTestFixture, BriefcasePushesSchemaToDifferentSchemaChannels)
    {
    ECDbHub hub1;
    SharedSchemaDb schemaChannel1("sync-db-1");
    auto b1 = hub1.CreateBriefcase();
    ASSERT_EQ(SharedSchemaChannel::Status::OK, b1->Schemas().GetSharedChannel().Init(schemaChannel1.GetChannelUri()));
    b1->PullMergePush("channel1 init");
    b1->SaveChanges();

    ECDbHub hub2;
    SharedSchemaDb schemaChannel2("sync-db-2");
    auto b2 = hub2.CreateBriefcase();
    ASSERT_EQ(SharedSchemaChannel::Status::OK, b2->Schemas().GetSharedChannel().Init(schemaChannel2.GetChannelUri()));
    b2->PullMergePush("channel2 init");
    b2->SaveChanges();

    auto schema2 = SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema2" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Pipe2">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <DbIndexList xmlns="ECDbMap.02.00.00">
                        <Indexes>
                            <DbIndex>
                                <Name>idx_pipe2_p3</Name>
                                <IsUnique>False</IsUnique>
                                <Properties>
                                    <string>p3</string>
                                </Properties>
                            </DbIndex>
                        </Indexes>
                    </DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="p3" typeName="int" />
                <ECProperty propertyName="p4" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml"
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "57df4675ccbce3493d2bb882ad3bb28f3266425c2f22fd55e57e187808b3add3";
    const auto SCHEMA1_HASH_ECDB_MAP = "8b1c6d8fa5b29e085bf94fae710527f56fa1c1792bd7404ff5775ed07f86f21f";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "8608aab5fa8a874b3f9140451ab8410c785483a878c8d915f48a26ef20e8241c";

    Test(
        "Import schema from b1 to schemaChannel1",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, schemaChannel1.GetChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("b1 import schema"));
            ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

            CheckHashes(*b1, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            schemaChannel1.WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "214eb014a70505aadc3885841bb8838f1abed8ecd6dbe9117b70ccbc9d94cebc";
    const auto SCHEMA2_HASH_ECDB_MAP = "927c44b85969b28eb2efef5e5f558a31ca4c0d7788ad43264c8babe68869e1ca";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "5afba22ac8137666912f9746b1011a34093e79bb1e76abddb2aea57a2e7c5541";

    Test(
        "Import schema from b2 to schemaChannel2",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b2, schema2, SchemaManager::SchemaImportOptions::None, schemaChannel2.GetChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, b2->PullMergePush("b2 import schema"));
            ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

            CheckHashes(*b2, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            schemaChannel2.WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Create and import schema from b1 to schemaChannel2",
        [&]()
            {
            ASSERT_EQ(
                SchemaImportResult::ERROR,
                ImportSchema(*b1, schema2, SchemaManager::SchemaImportOptions::None, schemaChannel2.GetChannelUri())
            ) << "should be a miss match of local and shared channel ids";
            ASSERT_EQ(BE_SQLITE_OK, b1->AbandonChanges());
            }
    );

    // TODO: Need fixing - how to reinitialize to a different shared channel
    Test(
        "Delete local changes and switch ECDb hub for b1",
        [&]()
            {
            ASSERT_EQ(BE_SQLITE_DONE, b1->DeleteBriefcaseLocalValue("TestSchema1"));
            b1->SetHub(hub2);

            ASSERT_EQ(SharedSchemaChannel::Status::ERROR_SHARED_CHANNEL_ALREADY_INITIALIZED, b1->Schemas().GetSharedChannel().Init(schemaChannel2.GetChannelUri()));
            // b1->PullMergePush("b1 PullMergePush to schemaChannel2");
            // b1->SaveChanges();
            // PrintHash(*b1, "b1 delete local value");
            // CheckHashes(*b1);
            // schemaChannel1.WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    // Test(
    //     "Import schema from b1 to schemaChannel2",
    //     [&]()
    //         {
    //         ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, schema2, SchemaManager::SchemaImportOptions::None, schemaChannel2.GetChannelUri())) << "should not be a miss match of channel ids";
    //         ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("b1 import schema to second schema channel"));
    //         ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    //         PrintHash(*b1, "b1 import to channel1");
    //         }
    // );
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, BriefcasePushesInvalidEmptyECSchema)
    {
    Test(
        "Create an invalid schema and try to import",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
                                    </DbIndex>
                                </Indexes>
                            </DbIndexList>
                        </ECCustomAttributes>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, SetupECDb("sync-db", schema)) << "Import of invalid schema";
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb); });
            CheckHashes(*m_briefcase);
            }
    );
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, SecondBriefcasePushesSchema)
    {
    ECDbHub hub;
    SharedSchemaDb schemaChannel("sync-db");
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();

    ASSERT_EQ(SharedSchemaChannel::Status::OK, b1->Schemas().GetSharedChannel().Init(schemaChannel.GetChannelUri())) << "Initialize schemaChannel from b1";
    b1->PullMergePush("init");
    b1->SaveChanges();

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "57df4675ccbce3493d2bb882ad3bb28f3266425c2f22fd55e57e187808b3add3";
    const auto SCHEMA1_HASH_ECDB_MAP = "8b1c6d8fa5b29e085bf94fae710527f56fa1c1792bd7404ff5775ed07f86f21f";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "8608aab5fa8a874b3f9140451ab8410c785483a878c8d915f48a26ef20e8241c";

    Test(
        "Import changes from b2 to schemaChannel",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*b2, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, schemaChannel.GetChannelUri()));
            schemaChannel.WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb); });
            CheckHashes(*b1);
            CheckHashes(*b2);
            ASSERT_EQ(BE_SQLITE_OK, b2->AbandonChanges());
            }
    );

    Test(
        "b2->PullMergePush and then import changes",
        [&]()
            {
            b2->PullMergePush("second briefcase comes in");
            b2->SaveChanges();

            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b2, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, schemaChannel.GetChannelUri()));
            schemaChannel.WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            CheckHashes(*b1);
            CheckHashes(*b2, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            }
    );

    Test(
        "Pull from schema Channel to b1",
        [&]()
            {
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                schemaChannel.Pull(
                    *b1,
                    [&]()
                        {
                        CheckHashes(*b1, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        }
                )
            );
            }
    );
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, TrackerWorkflow)
    {
    ECDbHub hub;
    SharedSchemaDb schemaChannel("sync-db");
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();
    auto b1ChangeTracker = b1->GetTracker();
    auto b2ChangeTracker = b2->GetTracker();

    Test(
        "Initial tracker assertion has no changes",
        [&]() 
            {
            ASSERT_FALSE(b1ChangeTracker->HasChanges());
            ASSERT_FALSE(b2ChangeTracker->HasChanges());
            }
    );

    Test(
        "Initialization of schemaChannel",
        [&]()
            {
            ASSERT_EQ(SharedSchemaChannel::Status::OK, b1->Schemas().GetSharedChannel().Init(schemaChannel.GetChannelUri()));

            // Briefcase 1 change tracker assertion
            ASSERT_TRUE(b1ChangeTracker->HasChanges());
            ASSERT_TRUE(b1ChangeTracker->HasDataChanges());
            ASSERT_FALSE(b1ChangeTracker->HasDdlChanges());
            ASSERT_EQ(1, b1ChangeTracker->GetLocalChangesets().size());

            // Briefcase 2 should be unchanged
            ASSERT_FALSE(b2ChangeTracker->HasChanges());
            ASSERT_EQ(0, b2ChangeTracker->GetLocalChangesets().size());

            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "57df4675ccbce3493d2bb882ad3bb28f3266425c2f22fd55e57e187808b3add3";
    const auto SCHEMA1_HASH_ECDB_MAP = "8b1c6d8fa5b29e085bf94fae710527f56fa1c1792bd7404ff5775ed07f86f21f";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "8608aab5fa8a874b3f9140451ab8410c785483a878c8d915f48a26ef20e8241c";
    Test(
        "Create and import ECSchema from b1 to sharedChannel",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, SchemaItem(schemaXMLBuilder()), SchemaManager::SchemaImportOptions::None, schemaChannel.GetChannelUri()));
            ASSERT_TRUE(b1ChangeTracker->HasChanges());
            ASSERT_TRUE(b1ChangeTracker->HasDataChanges());
            ASSERT_FALSE(b1ChangeTracker->HasDdlChanges());
            ASSERT_EQ(2, b1ChangeTracker->GetLocalChangesets().size());
            
            ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
            ASSERT_FALSE(b1ChangeTracker->HasChanges());
            ASSERT_EQ(3, b1ChangeTracker->GetLocalChangesets().size());
            CheckHashes(*b1, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);

            ASSERT_FALSE(b2ChangeTracker->HasChanges());
            ASSERT_EQ(0, b2ChangeTracker->GetLocalChangesets().size());
            CheckHashes(*b2);
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "4788587ee261bd4ebe0da5e9561c1193f79e6a86c0f54d855456402fd97939e3";
    const auto SCHEMA2_HASH_ECDB_MAP = "b4233e89aef79b6614416e5537e4ca6b5ef2d95655df01eba37e6e147662e245";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "524458d6160a9d611440adb2faec0cd37474063d5b3255625c1a8baa3b806abb";
    Test(
        "Schema change from briefcase 1 to schemaChannel",
        [&]()
            {
             auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="TestSchema1" alias="ts" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
                        <ECProperty propertyName="p3" typeName="int" />
                        <ECProperty propertyName="p4" typeName="string" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, schema, SchemaManager::SchemaImportOptions::None, schemaChannel.GetChannelUri()));
            ASSERT_TRUE(b1ChangeTracker->HasChanges());
            ASSERT_TRUE(b1ChangeTracker->HasDataChanges());
            ASSERT_FALSE(b1ChangeTracker->HasDdlChanges());
            ASSERT_EQ(4, b1ChangeTracker->GetLocalChangesets().size());
            
            ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
            ASSERT_FALSE(b1ChangeTracker->HasChanges());
            ASSERT_EQ(5, b1ChangeTracker->GetLocalChangesets().size());
            CheckHashes(*b1, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);

            ASSERT_FALSE(b2ChangeTracker->HasChanges());
            ASSERT_EQ(0, b2ChangeTracker->GetLocalChangesets().size());
            }
    );

    Test(
        "PullMergePush for b1",
        [&]()
            {
            ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("b1 import schema"));
            ASSERT_FALSE(b1ChangeTracker->HasChanges());
            ASSERT_EQ(0, b1ChangeTracker->GetLocalChangesets().size());

            b1->SaveChanges();
            ASSERT_FALSE(b1ChangeTracker->HasChanges());
            ASSERT_EQ(0, b1ChangeTracker->GetLocalChangesets().size());
            CheckHashes(*b1, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);

            ASSERT_FALSE(b2ChangeTracker->HasChanges());
            ASSERT_EQ(0, b2ChangeTracker->GetLocalChangesets().size());
            }
    );

    Test(
        "PullMergePush for b2",
        [&]()
            {
            ASSERT_EQ(BE_SQLITE_OK, b2->PullMergePush("b2 sync schema"));
            ASSERT_FALSE(b2ChangeTracker->HasChanges());
            ASSERT_EQ(0, b2ChangeTracker->GetLocalChangesets().size());

            b2->SaveChanges();
            ASSERT_FALSE(b2ChangeTracker->HasChanges());
            ASSERT_EQ(0, b2ChangeTracker->GetLocalChangesets().size());
            CheckHashes(*b2, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);

            ASSERT_FALSE(b1ChangeTracker->HasChanges());
            ASSERT_EQ(0, b1ChangeTracker->GetLocalChangesets().size());
            }
    );

    const auto SCHEMA3_HASH_ECDB_SCHEMA = "d0d84710fa6441bf327e73cf5c4b3f29585e54219926d5c7f6fd24c1e0573a1c";
    const auto SCHEMA3_HASH_ECDB_MAP = "3408a5b7be261e4d0d7d5413b8cd6df76575c8a00331b5c2e7ad49820ac0024c";
    const auto SCHEMA3_HASH_SQLITE_SCHEMA = "50cdc05b02f5e39921b1a516c1cd284c64d935f6a680ed6f312c4bc21f3899a7";
    Test(
        "Schema change from briefcase 2 to schemaChannel",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="TestSchema1" alias="ts" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
                        <ECProperty propertyName="p3" typeName="int" />
                        <ECProperty propertyName="p4" typeName="string" />
                        <ECProperty propertyName="p5" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b2, schema, SchemaManager::SchemaImportOptions::None, schemaChannel.GetChannelUri()));
            ASSERT_TRUE(b2ChangeTracker->HasChanges());
            ASSERT_TRUE(b2ChangeTracker->HasDataChanges());
            ASSERT_FALSE(b2ChangeTracker->HasDdlChanges());
            ASSERT_EQ(1, b2ChangeTracker->GetLocalChangesets().size());

            ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
            ASSERT_FALSE(b2ChangeTracker->HasChanges());
            ASSERT_EQ(2, b2ChangeTracker->GetLocalChangesets().size());
            CheckHashes(*b2, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);

            ASSERT_FALSE(b1ChangeTracker->HasChanges());
            ASSERT_EQ(0, b1ChangeTracker->GetLocalChangesets().size());
            }
    );

    Test(
        "PullMergePush for b2 after b2 schema changes",
        [&]()
            {
            ASSERT_EQ(BE_SQLITE_OK, b2->PullMergePush("b2 import schema update"));
            ASSERT_FALSE(b2ChangeTracker->HasChanges());
            ASSERT_EQ(0, b2ChangeTracker->GetLocalChangesets().size());

            b2->SaveChanges();
            ASSERT_FALSE(b2ChangeTracker->HasChanges());
            ASSERT_EQ(0, b2ChangeTracker->GetLocalChangesets().size());
            CheckHashes(*b2, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);

            ASSERT_FALSE(b1ChangeTracker->HasChanges());
            ASSERT_EQ(0, b1ChangeTracker->GetLocalChangesets().size());
            }
    );

    Test(
        "Final hash check",
        [&]()
            {
            CheckHashes(*b1, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            CheckHashes(*b2, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);
            schemaChannel.WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP); });
            }
    );
    }


//*************************
// From SchemaUpgradeTests
//*************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, DeleteSchema_VerifyCustomAttributesAreDeletedAsWell)
    {
    //This test simulate a case where there is orphan custom attribute instance and expect schema import to fail;
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "ff4e6e548d1566f81f33b2658c543424f2b04cef747c3bad34930f6c4ff7d29e";
    const auto SCHEMA1_HASH_ECDB_MAP = "187df5f660c246dbe2d3f2c8571b1cc1821a068bc1071d266a40bd9a570901c6";
    Test(
        "Setup ECDb",
        [&]()
            {
            auto testCa = SchemaItem(
                R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="TestCA" alias="tsca" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECCustomAttributeClass typeName="TestCA1" modifier="Sealed" appliesTo="Any"/>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schema_del", testCa));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "fd5b2c6c3aa4441c409469d6c0a61d580d0e09da594099c1d87bf8de6be81b15";
    const auto SCHEMA2_HASH_ECDB_MAP = "d925bbbadc91cccb226554a4b06e1d333aea21bcda00d10060818d19912e1b9b";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "6999f650adc04e6e20df544550cf9ebc590f4336c8283370934d5b96e8acd512";
    Test(
        "Import additional schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="TestCA" version="01.00.00" alias="tsca"/>
                    <ECCustomAttributes>
                        <TestCA1 xmlns="TestCA.01.00"/>
                    </ECCustomAttributes>
                    <ECEntityClass typeName="Pipe">
                        <ECCustomAttributes>
                            <TestCA1 xmlns="TestCA.01.00"/>
                        </ECCustomAttributes>
                        <ECProperty propertyName="p4" typeName="int">
                            <ECCustomAttributes>
                                <TestCA1 xmlns="TestCA.01.00"/>
                            </ECCustomAttributes>
                        </ECProperty>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    auto pipeClass = m_briefcase->Schemas().GetClass("TestSchema","Pipe");
    auto testCA1Class = m_briefcase->Schemas().GetClass("TestCA","TestCA1");
    Test(
        "Check if classes exist",
        [&]()
            {
            ASSERT_NE(nullptr, pipeClass);
            ASSERT_NE(nullptr, testCA1Class);
            }
    );

    auto p4Prop = pipeClass->GetPropertyP("p4");
    Test("Check if property exist", [&]() { ASSERT_NE(nullptr, p4Prop); });

    auto testCA1ClassId = testCA1Class->GetId();
    auto pipeClassId = pipeClass->GetId();
    auto pipePropId = p4Prop->GetId();
    const auto ContainerType_Schema = 1;
    const auto ContainerType_Class = 30;
    const auto ContainerType_Property = 992;
    auto doesCustomAttributeExists = [&](BeInt64Id containerId, ECN::ECClassId caClassId, int containerType)
        {
        auto stmt = m_briefcase->GetCachedStatement("SELECT Id FROM ec_CustomAttribute WHERE ContainerId =? AND ContainerType=? AND ClassId =?");
        stmt->BindId(1, containerId);
        stmt->BindInt(2, containerType);
        stmt->BindId(3, caClassId);
        return stmt->Step() == BE_SQLITE_ROW;
        };

    Test(
        "Check if custom attributes exist",
        [&]()
            {
            ASSERT_TRUE(doesCustomAttributeExists(pipeClassId, testCA1ClassId, ContainerType_Class));
            ASSERT_TRUE(doesCustomAttributeExists(pipePropId, testCA1ClassId, ContainerType_Property));
            }
    );

    Test(
        "Drop TestSchema",
        [&]()
            {
            ASSERT_TRUE(DropSchema("TestSchema", true).IsSuccess());

            // Validation check should fail
            ASSERT_FALSE(doesCustomAttributeExists(pipeClassId, testCA1ClassId, ContainerType_Class)) << "Expect CA instances to be deleted";
            ASSERT_FALSE(doesCustomAttributeExists(pipePropId, testCA1ClassId, ContainerType_Property));

            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Dropping schemas from shared channel is not allowed but returned result is OK",
        [&]()
            {
            Utf8String schemaXml;
            ASSERT_EQ(SchemaWriteStatus::Success, m_briefcase->Schemas().GetSchema("TestCA")->WriteToXmlString(schemaXml, ECVersion::V3_1));
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(SchemaItem(schemaXml)));

            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); }); // It is intended to not be able to drop everything from shared Channel
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// DropSchema doesn't work like in schemaUpgrade, should it work?
// Need to disable tracker to be able to use DropSchema
// Can't push drops to shared channel for some reason
TEST_F(SchemaSyncTestFixture, DeleteSchema_Check_Table_Drop)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "47358aeccfa438db0653fb12e16ffc3cb21716bd89313947e145edb2fa8e4049";
    const auto SCHEMA1_HASH_ECDB_MAP = "4b9b37d0d4e26e210bc2b82fbc50aaaccf31d8f9eab48d67a1fb8afbb5f6907b";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "a799d9fcacf6a62435de7f2596392c7699408bdaa9f909af4596d820d305ba10";
    Test(
        "Init bisCore",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="BisCore" alias="bis" version="01.00.12" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName="Element" modifier="Abstract">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.2.0">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="p1" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement" modifier="Abstract">
                        <BaseClass>Element</BaseClass>
                        <ECCustomAttributes>
                            <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0"/>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement3d" modifier="Abstract">
                        <BaseClass>GeometricElement</BaseClass>
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.2.0">
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName="p2" typeName="int" />
                        <ECProperty propertyName="p3" typeName="int" />
                        <ECProperty propertyName="p4" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schema_del", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "fc42180744d16fa46935591cda2c096fb2f9ab7e0c7230dcaec744e009e6e0a5";//
    const auto SCHEMA2_HASH_ECDB_MAP = "0f8225485fb52c416774152d2e56d502a93835f53d5b804421f87f4d3e61bf54";//
    Test(
        "span join table",
        [&]() 
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="BisCore" version="01.00.12" alias="bis"/>
                    <ECEntityClass typeName="Pipe">
                        <BaseClass>bis:GeometricElement3d</BaseClass>
                        <ECProperty propertyName="p2" typeName="int" />
                        <ECProperty propertyName="p3" typeName="int" />
                        <ECProperty propertyName="p4" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA3_HASH_ECDB_SCHEMA = "d4c7a15cde6dfd5e60b57e0ae386b2169e33f5570f47fe477c8050919fef1f38";
    const auto SCHEMA3_HASH_ECDB_MAP = "463751fe6f014849006724c9cac4bb280ed680df9cee7a104090830157fc65d2";
    const auto SCHEMA3_HASH_SQLITE_SCHEMA = "3f8ad1c7d151794e8d267f22f8ff63acbb10625fed9ef122130e5c2191add5c9";
    Test(
        "span overflow table",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="TestSchema1" alias="ts1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="BisCore" version="01.00.12" alias="bis"/>
                    <ECEntityClass typeName="Pipe1">
                        <BaseClass>bis:GeometricElement3d</BaseClass>
                        <ECProperty propertyName="p2" typeName="int" />
                        <ECProperty propertyName="p3" typeName="int" />
                        <ECProperty propertyName="p4" typeName="int" />
                        <ECProperty propertyName="p5" typeName="int" />
                        <ECProperty propertyName="p6" typeName="int" />
                        <ECProperty propertyName="p7" typeName="int" />
                        <ECProperty propertyName="p8" typeName="int" />
                        <ECProperty propertyName="p9" typeName="int" />
                        <ECProperty propertyName="pa" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check tables",
        [&]()
            {
            ASSERT_TRUE(m_briefcase->TableExists("bis_Element"));
            ASSERT_TRUE(m_briefcase->TableExists("bis_GeometricElement3d"));
            ASSERT_TRUE(m_briefcase->TableExists("bis_GeometricElement3d_Overflow"));
            }
    );

    const auto SCHEMA4_HASH_ECDB_MAP = "5fab223124703ec61be047e2e4154f8bdac711a162e12c8a1b2d8737132b0857";
    Test(
        "Drop TestSchema1",
        [&]()
            {
            ASSERT_TRUE(DropSchema("TestSchema1").IsSuccess());
            ASSERT_TRUE(m_briefcase->TableExists("bis_Element")) << "bis_Element table should not be dropped";
            ASSERT_TRUE(m_briefcase->TableExists("bis_GeometricElement3d")) << "bis_GeometricElement3d table should not be dropped";
            ASSERT_TRUE(m_briefcase->TableExists("bis_GeometricElement3d_Overflow")) << "bis_GeometricElement3d_Overflow table should not be dropped";
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA4_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);
            }
    );

    // TODO: chech if other briefcases dont have dropped schemas

    const auto SCHEMA5_HASH_ECDB_MAP = "bde4d99746cf01acdd04e69c2b39133e92a2ed53858f787450fa7ecdd7cf1675";
    Test(
        "Drop TestSchema",
        [&]()
            {
            ASSERT_TRUE(DropSchema("TestSchema").IsSuccess());
            ASSERT_TRUE(m_briefcase->TableExists("bis_Element")) << "bis_Element table should not be dropped";
            ASSERT_TRUE(m_briefcase->TableExists("bis_GeometricElement3d")) << "bis_GeometricElement3d table should not be dropped";
            ASSERT_TRUE(m_briefcase->TableExists("bis_GeometricElement3d_Overflow")) << "bis_GeometricElement3d_Overflow table should not be dropped";
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA5_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);
            }
    );

    Test(
        "Drop BisCore",
        [&]()
            {
            ASSERT_TRUE(DropSchema("BisCore", true).IsSuccess());
            ASSERT_FALSE(m_briefcase->TableExists("bis_Element")) << "bis_Element table should be dropped";
            ASSERT_FALSE(m_briefcase->TableExists("bis_GeometricElement3d")) << "bis_GeometricElement3d table should be dropped";
            ASSERT_FALSE(m_briefcase->TableExists("bis_GeometricElement3d_Overflow")) << "bis_GeometricElement3d_Overflow table be dropped";
            CheckHashes(*m_briefcase); // default values == empty briefcase
            }
    );

    // PullMergePush briefcase
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// Cant drop white it's tracked
// Briefcase changes don't push to shared channel
TEST_F(SchemaSyncTestFixture, DeleteSchema)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "ea4c2210c4f6f02bf3063126c8fde7a9244a58d4b9b2e4297891a3ee689e9851";
    const auto SCHEMA1_HASH_ECDB_MAP = "33c1134c50fde34896259ee41d842d71b26591cd6f5061cfb91241e1a324e493";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "69d6d5fdf9b5f7d93634514f6e40357fea3cf41f82dbb301e3567ed0facd519a";
    Test(
        "Setup delete schemaDb",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='A'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00.00'/>
                        </ECCustomAttributes>
                        <ECProperty propertyName='PA' typeName='int' />
                    </ECEntityClass>
                    <ECEntityClass typeName='B'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00.00'/>
                            <DbIndexList xmlns='ECDbMap.02.00'>
                                <Indexes>
                                    <DbIndex>
                                        <Name>IDX_Partial</Name>
                                        <IsUnique>False</IsUnique>
                                        <Properties>
                                            <string>AFk.Id</string>
                                        </Properties>
                                    </DbIndex>
                                </Indexes>
                            </DbIndexList>
                        </ECCustomAttributes>
                        <ECProperty propertyName='PB' typeName='int' />
                        <ECNavigationProperty propertyName='AFk' relationshipName='AHasB' direction='Backward' />
                    </ECEntityClass>
                    <ECRelationshipClass typeName='AHasB' strength='Embedding'>
                        <Source cardinality='(0,1)' polymorphic='True'>
                            <Class class ='A' />
                        </Source>
                        <Target cardinality='(0,N)' polymorphic='True'>
                            <Class class ='B' />
                        </Target>
                    </ECRelationshipClass>
                    <ECRelationshipClass typeName='ARefB'  modifier='Sealed' strength='referencing' strengthDirection='forward'>
                        <ECCustomAttributes>
                            <LinkTableRelationshipMap xmlns='ECDbMap.2.0'>
                                <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                            </LinkTableRelationshipMap>
                        </ECCustomAttributes>
                        <Source cardinality='(0,N)' polymorphic='True'>
                            <Class class ='A' />
                        </Source>
                        <Target cardinality='(0,N)' polymorphic='True'>
                            <Class class ='B' />
                        </Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schema_del", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "a6890ae18a24d53cce29ce3e998b357b4e9244b34460fab3d6f44f715e8d4a0c";
    const auto SCHEMA2_HASH_ECDB_MAP = "cf097419aece66e31041acb2e3569a6903cf7d660b3277fc2df058da5dc298d3";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "02076e3d9a50766f055920f9c9f629ea553c778e816c4c5d09121aed0e7e2140";
    Test(
        "Import a schema change",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema1' nameSpacePrefix='ts1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'TestSchema' version='01.00' prefix = 'ts' />
                    <ECEntityClass typeName='C'>
                        <BaseClass>ts:A</BaseClass>
                        <ECProperty propertyName='PC' typeName='int' />
                    </ECEntityClass>
                    <ECEntityClass typeName='D' modifier='Sealed'>
                        <BaseClass>ts:B</BaseClass>
                        <ECProperty propertyName='PD' typeName='int' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    
    ECInstanceKey a0;
    Test(
        "insert instance of A",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.A(PA) VALUES(100)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(a0));
            }
    );

    ECInstanceKey b0;
    Test(
        "insert instance of B",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.B(PB, AFk) VALUES(200, ?)"));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, a0.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(b0));
            }
    );

    ECInstanceKey c0;
    Test(
        "insert instance of C",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts1.C(PA, PC) VALUES(100, 200)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(c0));
            }
    );

    ECInstanceKey d0;
    Test(
        "insert instance of D",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts1.D(PB, PD, AFk) VALUES(200, 300, ?)"));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, c0.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(d0));
            }
    );

    ECInstanceKey d1;
    Test(
        "insert second instance of D",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts1.D(PB, PD, AFk) VALUES(300, 400, ?)"));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, a0.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(d1));
            }
    );

    ECInstanceKey aRefB0;
    Test(
        "insert instance of ARefB",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.ARefB(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?,?,?,?)"));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, a0.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, a0.GetClassId()));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, b0.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, b0.GetClassId()));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(aRefB0));
            }
    );

    ECInstanceKey aRefB1;
    Test(
        "insert second instance of ARefB",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.ARefB(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?,?,?,?)"));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, c0.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, c0.GetClassId()));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, d0.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, d0.GetClassId()));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(aRefB1));
            }
    );

    ECInstanceKey aRefB2;
    Test(
        "insert third instance of ARefB",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.ARefB(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?,?,?,?)"));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, a0.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, a0.GetClassId()));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, d0.GetInstanceId()));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, d0.GetClassId()));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(aRefB2));
            }
    );

    Test(
        "Save all instance changes",
        [&]()
            {
            m_briefcase->SaveChanges();
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            }
    );

    Test(
        "Try to delete TestSchema",
        [&]()
            {
            auto rc = DropSchema("TestSchema");
            ASSERT_FALSE(rc.IsSuccess());
            ASSERT_TRUE(rc.HasDependendSchemas());
            }
    );

    std::unique_ptr<DropSchemaResult> rc1;
    Test(
        "Try to delete TestSchema1",
        [&]()
            {
            rc1 = std::make_unique<DropSchemaResult>(DropSchema("TestSchema1"));
            ASSERT_FALSE(rc1->IsSuccess());
            ASSERT_TRUE(rc1->HasInstances());
            }
    );

    Test(
        "Delete instances from TestSchema1",
        [&]()
            {
            for(auto& kp : rc1->GetInstances().GetEntityKeyMap())
                {
                auto baseClassId = kp.first;
                std::shared_ptr<IdSet<BeInt64Id>> idToDelete = std::make_shared<IdSet<BeInt64Id>>();
                for (auto key: kp.second)
                    idToDelete->insert(key.GetInstanceId());

                Utf8String className = m_briefcase->Schemas().GetClass(baseClassId)->GetFullName();
                ECSqlStatement stmt;
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("DELETE FROM %s WHERE InVirtualSet(?, ECInstanceId)", className.c_str())));
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindVirtualSet(1, idToDelete));
                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
                ASSERT_EQ(m_briefcase->GetModifiedRowCount(), idToDelete->size());
                }
            }
    );

    const auto SCHEMA3_HASH_ECDB_MAP = "58bf6103669910b463f4c230d7eb61d3d9fe996de7fc9b151a81e83e618f888e";
    Test(
        "Successfully delete TestSchema1 from briefcase",
        [&]()
            {
            ASSERT_TRUE(DropSchema("TestSchema1").IsSuccess());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            // How to push dropped schemas to shared channel
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->PullMergePush("Drop TestSchema1"));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            // m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP); });
            ASSERT_TRUE(m_briefcase->GetTracker()->GetLocalChangesets().empty());
            }
    );

    std::unique_ptr<DropSchemaResult> rc0;
    Test(
        "Try to delete TestSchema again, should give different error",
        [&]()
            {
            rc0 = std::make_unique<DropSchemaResult>(DropSchema("TestSchema"));
            ASSERT_FALSE(rc0->IsSuccess());
            ASSERT_TRUE(rc0->HasInstances());
            }
    );

    Test(
        "Delete detected instances from TestSchema",
        [&]()
            {
            for(auto& kp : rc0->GetInstances().GetEntityKeyMap())
                {
                auto baseClassId = kp.first;
                std::shared_ptr<IdSet<BeInt64Id>> idToDelete = std::make_shared<IdSet<BeInt64Id>>();
                for (auto key: kp.second)
                    idToDelete->insert(key.GetInstanceId());

                Utf8String className = m_briefcase->Schemas().GetClass(baseClassId)->GetFullName();
                ECSqlStatement stmt;
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("DELETE FROM %s WHERE InVirtualSet(?, ECInstanceId)", className.c_str())));
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindVirtualSet(1, idToDelete));
                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
                ASSERT_EQ(m_briefcase->GetModifiedRowCount(), idToDelete->size());
                ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
                }
            }
    );

    // Cannot make arbitrary schema changes when changes are being tracked if tracker is not disabled
    Test(
        "Successfully delete TestSchema from briefcase",
        [&]()
            {
            ASSERT_TRUE(DropSchema("TestSchema", true).IsSuccess());
            CheckHashes(*m_briefcase);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); }); // Why it doesn't drop schemas?
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateECSchemaAttributes)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "700742be350b58268aeb3fd138322ffade0ee2d45b632cf9e086446c49c005ec";
    Test(
        "Import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("UpdateECSchemaAttributes", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "4c664dc4c288e9396a965c3a3abff1946a129582e58d812504bbcf4a269aab6d";
    Test(
        "Modifying display label and description is expected to be supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' displayLabel='New Test Schema' description='This is a New Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "After modifying display label and description",
        [&]()
            {
            auto expected = JsonValue(R"json([{"DisplayLabel":"New Test Schema", "Description":"This is a New Test Schema", "Alias":"ts"}])json");
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            }
    );

    Test(
        "Modifying alias is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts2' displayLabel='New Test Schema' description='This is a New Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// Why isn't minor version change saved?
// it says ok but skips?
TEST_F(SchemaSyncTestFixture, ModifySchemaVersion)
    {
    auto schemaXml = [&](Utf8CP newSchemaVersion = "10.10.10")
        {
        Utf8CP schemaTemplate =
            R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName='TestSchema1' alias='ts1' version='%s' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                <ECEntityClass typeName='TestClassA' >
                    <ECProperty propertyName='L1' typeName='double'/>
                </ECEntityClass>
            </ECSchema>)xml";
        Utf8String schemaXml;
        schemaXml.Sprintf(schemaTemplate, newSchemaVersion);
        return schemaXml;
        };
    auto assertVersion = [&](ECSchemaCP schemaPointer, uint32_t readVersion, uint32_t writeVersion, uint32_t minorVersion)
        {
        ASSERT_EQ(readVersion, schemaPointer->GetVersionRead());
        ASSERT_EQ(writeVersion, schemaPointer->GetVersionWrite());
        ASSERT_EQ(minorVersion, schemaPointer->GetVersionMinor());
        };

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "383c0adf0ddf6bfe396c6caf6e8020defe446640a2bf1fedd3ef174a0ca7301b";
    const auto SCHEMA1_HASH_ECDB_MAP = "a3a4f4bd3e7c375e583bb9d5dde2d4ee593dab571dd1ab2bae88d5c531753137";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "2821726bf97bd871e7e7ca78101175541782ba81be1b692eee2a68547f403a48";
    Test(
        "Init ECDb",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("SchemaUpgrade_ModifySchemaVersion", SchemaItem(schemaXml("10.10.10"))));
            assertVersion(m_briefcase->Schemas().GetSchema("TestSchema1"), 10, 10, 10);
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Decreasing minor version when write version was incremented is supported",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "f413afe76940f366025b12179084a67d883a6ab292ca54158575548e67bedc07";
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(SchemaItem(schemaXml("10.11.9"))));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            assertVersion(m_briefcase->Schemas().GetSchema("TestSchema1"), 10, 11, 9);
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Decreasing write version when read version was incremented is supported",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "490264cc2f8bb3a1252cbf9a6878eae6611c45bd7f92e41b37b73fe780ec8a0b";
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(SchemaItem(schemaXml("11.10.9"))));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            assertVersion(m_briefcase->Schemas().GetSchema("TestSchema1"), 11, 10, 9);
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Decreasing minor version when read version was incremented is supported",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "2aee1c0a40c8a3f26fc11de65144dca1cf74935ceec64ff96026cce85769388f";
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(SchemaItem(schemaXml("12.10.8"))));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            assertVersion(m_briefcase->Schemas().GetSchema("TestSchema1"), 12, 10, 8);
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "fd8871094a4cdffcc543aa3193d154a3ce771adddff752657460ca40511a92cc";
    Test(
        "Decreasing minor and write version when read version was incremented is supported",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(SchemaItem(schemaXml("13.1.7"))));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            assertVersion(m_briefcase->Schemas().GetSchema("TestSchema1"), 13, 1, 7);
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Decreasing read version is not supported",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(SchemaItem(schemaXml("12.1.7"))));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            assertVersion(m_briefcase->Schemas().GetSchema("TestSchema1"), 13, 1, 7);
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Decreasing write version is not supported",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(SchemaItem(schemaXml("13.0.7"))));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            assertVersion(m_briefcase->Schemas().GetSchema("TestSchema1"), 13, 1, 7);
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Decreasing minor version is supported",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(SchemaItem(schemaXml("13.1.6"))));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            assertVersion(m_briefcase->Schemas().GetSchema("TestSchema1"), 13, 1, 7); // Why isn't saved?
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, SchemaDowngrade_MoreComplex)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "90d7d4b0e6fa42bbca30909358a5f4453e5fbf23dc04180586b7e708c8046eab";
    const auto SCHEMA1_HASH_ECDB_MAP = "aad4ba89e506aa58452c801120bf1e662ad761a14332932283eab33113e7d419";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "616136b709172fd8b18c5d9213f3c87b5c3d05a852787260324b52e009c45ae4";
    Test(
        "Import initial schema",
        [&]() 
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="2.4.3" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName="Parent" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00.00"/>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Code" typeName="int"/>
                        <ECProperty propertyName="Val" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub" >
                        <BaseClass>Parent</BaseClass>
                        <ECProperty propertyName="SubProp" typeName="string" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("SchemaDowngrade", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Import schema with smaller read version",
        [&]() 
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.9.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName="Parent" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00.00"/>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Code" typeName="int"/>
                        <ECProperty propertyName="Val" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub" >
                        <BaseClass>Parent</BaseClass>
                        <ECProperty propertyName="SubProp" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub2" >
                        <BaseClass>Parent</BaseClass>
                        <ECProperty propertyName="Sub2Prop" typeName="string" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Import schema with smaller write version",
        [&]() 
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="2.2.78" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName="Parent" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00.00"/>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Code" typeName="int"/>
                        <ECProperty propertyName="Val" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub" >
                        <BaseClass>Parent</BaseClass>
                        <ECProperty propertyName="SubProp" typeName="string" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Import schema with smaller minor version",
        [&]() 
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="2.4.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName="Parent" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00.00"/>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Name" typeName="string" />
                        <ECProperty propertyName="Code" typeName="int"/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check schema atributes",
        [&]()
            {
            ASSERT_TRUE(m_briefcase->Schemas().GetClass("TestSchema", "Sub") != nullptr) << "Class Sub is still expected to exist as schema minor version downgrade is skipped.";
            ECClassCP parentClass = m_briefcase->Schemas().GetClass("TestSchema", "Parent");
            ASSERT_TRUE(parentClass != nullptr);
            ASSERT_TRUE(parentClass->GetPropertyP("Val") != nullptr) << "Property Val in class Parent is still expected to exist as schema minor version downgrade is skipped.";
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// Can't upgrade ECXml version, why?
TEST_F(SchemaSyncTestFixture, ECVersions)
    {
    auto verifySchemaVersion = [] (ECDbCR ecdb, Utf8CP schemaName, uint32_t expectedOriginalXmlVersionMajor, uint32_t expectedOriginalXmlVersionMinor)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT OriginalECXmlVersionMajor, OriginalECXmlVersionMinor FROM meta.ECSchemaDef WHERE Name=?"));
        stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(expectedOriginalXmlVersionMajor, (uint32_t) stmt.GetValueInt(0));
        ASSERT_EQ(expectedOriginalXmlVersionMinor, (uint32_t) stmt.GetValueInt(1));
        };

    auto schemaXml = [&](Utf8CP newECXmlVersion = "3.0")
        {
        Utf8CP schemaTemplate =
            R"xml(<?xml version='1.0' encoding='utf-8'?>
            <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.%s'>
            </ECSchema>)xml";
        Utf8String schemaXml;
        schemaXml.Sprintf(schemaTemplate, newECXmlVersion);
        return schemaXml;
        };

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "cb08e969d4e36cbb139608a7ce22c50130f8659f1322ddff4b4b730233e30b65";
    Test(
        "Import initial schema",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("SchemaOriginalECXmlVersion", SchemaItem(schemaXml("3.0"))));
            verifySchemaVersion(*m_briefcase, "TestSchema", 3, 0);
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Upgrade of ECXml version to 3.1",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(SchemaItem(schemaXml("3.1")))); // should be OK
            // verifySchemaVersion(*m_briefcase, "TestSchema", 3, 1);
            // PrintHash(*m_briefcase, "b 2");
            // m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { PrintHash(syncDb, "channel 2"); });
            }
    );

    Test(
        "Upgrade of ECXml version to 3.2",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(SchemaItem(schemaXml("3.2"))));  // should be OK
            // verifySchemaVersion(*m_briefcase, "TestSchema", 3, 2);
            // PrintHash(*m_briefcase, "b 3");
            // m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { PrintHash(syncDb, "channel 3"); });
            }
    );

    Test(
        "Downgrade of ECXml version is not supported",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(SchemaItem(schemaXml("3.1")))); // This one is correct
            // verifySchemaVersion(*m_briefcase, "TestSchema", 3, 2);
            // PrintHash(*m_briefcase, "b 4");
            // m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { PrintHash(syncDb, "channel 4"); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateECClassAttributes)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "9b5ea082cc9f973856cc6f017e5ba8f01c02fc5c9ed4763cf1f04c6570c26d8e";
    const auto SCHEMA1_HASH_ECDB_MAP = "32608ea390282589dafbb36721fb70a9336a359434d056ab67dd245415c0f814";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "533fbcc8fc6a9a9efdb25aff7db7bdb91856955e6b8c236b3f5218b64361d533";
    Test(
        "Import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='Test Class' modifier='None' />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("UpdateECClassAttributes", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "660cefcd3dbf7e7dfac63f22bbacd5ecbb286efe731398fb18c9ece613d8c70d";
    Test(
        "Modifying ECClass display label and description is expected to be supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' displayLabel='New Test Schema' description='This is a New Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECEntityClass typeName='TestClass' displayLabel='My Test Class' description='My Test Class' modifier='None' />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "After modifying display label and description",
        [&]()
            {
            auto expected = JsonValue(R"json([{"DisplayLabel":"My Test Class", "Description":"My Test Class"}])json");
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddingUpdatingAndDeletingMinMax)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "c7ec8926fc236afc54f73bebac83f04c5dbbf7954bbaf5cf584e5424131063d4";
    const auto SCHEMA1_HASH_ECDB_MAP = "d1740d0775bf9c18eeff71fda2a0147774d84b95425b45e940ab1f102c26bdc2";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "011907794910d5af09f0c44b54319b9b10f7bfa329fbe3323873b780d6ab575e";
    Test(
        "Import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECEntityClass typeName='Foo'>
                        <ECProperty propertyName='P1' typeName='long'   minimumValue  = '1'     maximumValue   = '200'    />
                        <ECProperty propertyName='P2' typeName='double' minimumValue  = '1.22'  maximumValue   = '100.22' />
                        <ECProperty propertyName='P3' typeName='string' minimumLength = '1'     maximumLength  = '1000'   />
                        <ECProperty propertyName='P4' typeName='long'   maximumValue  = '1200'                            />
                        <ECProperty propertyName='P5' typeName='double' maximumValue  = '1200.12'                         />
                        <ECProperty propertyName='P6' typeName='string' maximumLength = '1000'                            />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schema_update_minMax", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check min max values of the imported schema",
        [&]()
            {
            ECClassCP foo = m_briefcase->Schemas().GetClass("TestSchema", "Foo");
            ASSERT_NE(nullptr, foo);

            ECValue minV, maxV;
            ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P1")->GetMinimumValue(minV));
            ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P1")->GetMaximumValue(maxV));
            ASSERT_EQ(1, minV.GetLong());
            ASSERT_EQ(200, maxV.GetLong());

            minV = maxV = ECValue();
            ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P2")->GetMinimumValue(minV));
            ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P2")->GetMaximumValue(maxV));
            ASSERT_EQ(1.22, minV.GetDouble());
            ASSERT_EQ(100.22, maxV.GetDouble());

            ASSERT_EQ(1, foo->GetPropertyP("P3")->GetMinimumLength());
            ASSERT_EQ(1000, foo->GetPropertyP("P3")->GetMaximumLength());

            minV = maxV = ECValue();
            ASSERT_EQ(ECObjectsStatus::Error, foo->GetPropertyP("P4")->GetMinimumValue(minV));
            ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P4")->GetMaximumValue(maxV));
            ASSERT_TRUE(minV.IsNull());
            ASSERT_EQ(1200, maxV.GetLong());

            minV = maxV = ECValue();
            ASSERT_EQ(ECObjectsStatus::Error, foo->GetPropertyP("P5")->GetMinimumValue(minV));
            ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P5")->GetMaximumValue(maxV));
            ASSERT_TRUE(minV.IsNull());
            ASSERT_EQ(1200.12, maxV.GetDouble());

            ASSERT_EQ(0, foo->GetPropertyP("P6")->GetMinimumLength());
            ASSERT_EQ(1000, foo->GetPropertyP("P6")->GetMaximumLength());
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "e0d3f984c96d9e974948b12ce77c729566769f5e9f462966871ea6a3b30eeee4";
    Test(
        "Import edited schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECEntityClass typeName='Foo'>
                        <ECProperty propertyName='P1' typeName='long'   />
                        <ECProperty propertyName='P2' typeName='double' />
                        <ECProperty propertyName='P3' typeName='string' />
                        <ECProperty propertyName='P4' typeName='long'   minimumValue  = '12'   maximumValue   = '2200'    />
                        <ECProperty propertyName='P5' typeName='double' minimumValue  = '1.33' maximumValue   = '2200.12' />
                        <ECProperty propertyName='P6' typeName='string' minimumLength = '11'   maximumLength  = '9000'    />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check min max values of the edited schema",
        [&]()
            {
            ECClassCP foo = m_briefcase->Schemas().GetClass("TestSchema", "Foo");
            ASSERT_NE(nullptr, foo);

            ECValue minV, maxV;
            ASSERT_EQ(ECObjectsStatus::Error, foo->GetPropertyP("P1")->GetMinimumValue(minV));
            ASSERT_EQ(ECObjectsStatus::Error, foo->GetPropertyP("P1")->GetMaximumValue(maxV));
            ASSERT_TRUE(minV.IsNull());
            ASSERT_TRUE(maxV.IsNull());

            minV = maxV = ECValue();
            ASSERT_EQ(ECObjectsStatus::Error, foo->GetPropertyP("P2")->GetMinimumValue(minV));
            ASSERT_EQ(ECObjectsStatus::Error, foo->GetPropertyP("P2")->GetMaximumValue(maxV));
            ASSERT_TRUE(minV.IsNull());
            ASSERT_TRUE(maxV.IsNull());

            ASSERT_EQ(0, foo->GetPropertyP("P3")->GetMinimumLength());
            ASSERT_EQ(0, foo->GetPropertyP("P3")->GetMaximumLength());

            minV = maxV = ECValue();
            ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P4")->GetMinimumValue(minV));
            ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P4")->GetMaximumValue(maxV));
            ASSERT_EQ(12, minV.GetLong());
            ASSERT_EQ(2200, maxV.GetLong());

            minV = maxV = ECValue();
            ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P5")->GetMinimumValue(minV));
            ASSERT_EQ(ECObjectsStatus::Success, foo->GetPropertyP("P5")->GetMaximumValue(maxV));
            ASSERT_EQ(1.33, minV.GetDouble());
            ASSERT_EQ(2200.12, maxV.GetDouble());

            ASSERT_EQ(11, foo->GetPropertyP("P6")->GetMinimumLength());
            ASSERT_EQ(9000, foo->GetPropertyP("P6")->GetMaximumLength());
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddingUpdatingAndDeletingPriority)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "a52fd4e987f48355050617030d7e95942d3aaadfc651e51af142c3992a672b4b";
    const auto SCHEMA1_HASH_ECDB_MAP = "eab09cbcdaa4b10e915d33e1c4cb6d389fd0b9ed39a0aa2e5c24357cbbf263eb";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "5fd180ad4deca14bc3188321140c63a077f54c2e61a22facbb4d9bb4d29341aa";
    Test(
        "Import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                   <ECEntityClass typeName='Foo'>
                       <ECProperty propertyName='P1' typeName='string' priority='1010' />
                       <ECProperty propertyName='P2' typeName='string' priority='1020' />
                       <ECProperty propertyName='P3' typeName='string' priority='1030' />
                       <ECProperty propertyName='P4' typeName='string' />
                       <ECProperty propertyName='P5' typeName='string' />
                   </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schema_update_priority", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check property priorities of initial schema",
        [&]()
            {
            ECClassCP foo = m_briefcase->Schemas().GetClass("TestSchema", "Foo");
            ASSERT_NE(nullptr, foo);
            ASSERT_EQ(1010, foo->GetPropertyP("P1")->GetPriority());
            ASSERT_EQ(1020, foo->GetPropertyP("P2")->GetPriority());
            ASSERT_EQ(1030, foo->GetPropertyP("P3")->GetPriority());
            ASSERT_EQ(0, foo->GetPropertyP("P4")->GetPriority());
            ASSERT_EQ(0, foo->GetPropertyP("P5")->GetPriority());
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "6eafc69781e919129c81eda84603e0034bb37ff03f3655447864bb841fb5d6c2";
    Test(
        "Import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                   <ECEntityClass typeName='Foo'>
                       <ECProperty propertyName='P1' typeName='string' priority='2010' />
                       <ECProperty propertyName='P2' typeName='string' priority='2020' />
                       <ECProperty propertyName='P3' typeName='string'                 />
                       <ECProperty propertyName='P4' typeName='string' priority='1040' />
                       <ECProperty propertyName='P5' typeName='string' priority='1050' />
                   </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check property priorities of edited schema",
        [&]()
            {
            ECClassCP foo = m_briefcase->Schemas().GetClass("TestSchema", "Foo");
            ASSERT_NE(nullptr, foo);
            ASSERT_EQ(2010, foo->GetPropertyP("P1")->GetPriority());
            ASSERT_EQ(2020, foo->GetPropertyP("P2")->GetPriority());
            ASSERT_EQ(0, foo->GetPropertyP("P3")->GetPriority());
            ASSERT_EQ(1040, foo->GetPropertyP("P4")->GetPriority());
            ASSERT_EQ(1050, foo->GetPropertyP("P5")->GetPriority());
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateBaseClass_Update_Mixin_AppliesToEntityClass_Generalized)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "81320154f7b3eb5b59c806681d1062d3a096f10e87944e980bed6babf133a418";
    const auto SCHEMA1_HASH_ECDB_MAP = "f2bf4b1c37302fe3ec09e892c2547eb3ad9fb4879eba48e5d8b89ae9097d728f";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "82488d5828102fb18314b74fe31a1e20f8bc3087cc83475974ecf0980abcfed5";
    Test(
        "Import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>"
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>"
                    <ECEntityClass typeName='IOptionA' modifier='Abstract'>"
                        <ECCustomAttributes>"
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                                <AppliesToEntityClass>SubElement</AppliesToEntityClass>"
                            </IsMixin>"
                        </ECCustomAttributes>"
                    </ECEntityClass>"
                    <ECEntityClass typeName='Element'>"
                        <ECProperty propertyName='Code' typeName='string' />"
                    </ECEntityClass>"
                    <ECEntityClass typeName='SubElement'>"
                        <BaseClass>Element</BaseClass>"
                        <ECProperty propertyName='Text' typeName='string' />"
                    </ECEntityClass>"
                    <ECEntityClass typeName='SupportOption' modifier='None' >"
                        <BaseClass>SubElement</BaseClass>"
                        <BaseClass>IOptionA</BaseClass>"
                        <ECProperty propertyName='P1' typeName='string' />"
                    </ECEntityClass>"
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("appliesToEntityClass", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check value of AppliesToEntityClass of initial schema",
        [&]()
            {
            auto mixIn = m_briefcase->Schemas().GetClass("TestSchema", "IOptionA");
            auto ca = mixIn->GetCustomAttributeLocal("CoreCustomAttributes", "IsMixin");
            ECValue appliesToValue;
            ca->GetValue(appliesToValue, "AppliesToEntityClass");
            ASSERT_FALSE(appliesToValue.IsNull() || !appliesToValue.IsString());
            ASSERT_STRCASEEQ(appliesToValue.GetUtf8CP(), "SubElement");
            }
    );

    Test(
        "Import edited schema with some changes",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "6ad6345a02348b9b4bbca7e8eefd6aaccf86b9fa8b7e0788561def84fdbe8af6";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IOptionA' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SubElement'>
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName='Text' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>SubElement</BaseClass>
                        <BaseClass>IOptionA</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check value of AppliesToEntityClass of edited schema",
        [&]()
            {
            auto mixIn = m_briefcase->Schemas().GetClass("TestSchema", "IOptionA");
            auto ca = mixIn->GetCustomAttributeLocal("CoreCustomAttributes", "IsMixin");
            ECValue appliesToValue;
            ca->GetValue(appliesToValue, "AppliesToEntityClass");
            ASSERT_FALSE(appliesToValue.IsNull() || !appliesToValue.IsString());
            ASSERT_STRCASEEQ(appliesToValue.GetUtf8CP(), "Element");
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateBaseClass_Update_Mixin_AppliesToEntityClass_Specialized)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "81320154f7b3eb5b59c806681d1062d3a096f10e87944e980bed6babf133a418";
    const auto SCHEMA1_HASH_ECDB_MAP = "f2bf4b1c37302fe3ec09e892c2547eb3ad9fb4879eba48e5d8b89ae9097d728f";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "82488d5828102fb18314b74fe31a1e20f8bc3087cc83475974ecf0980abcfed5";
    Test(
        "Import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IOptionA' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>SubElement</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SubElement'>
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName='Text' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>SubElement</BaseClass>
                        <BaseClass>IOptionA</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("appliesToEntityClass_props", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check value of initial schema",
        [&]()
            {
            auto mixIn = m_briefcase->Schemas().GetClass("TestSchema", "IOptionA");
            auto ca = mixIn->GetCustomAttributeLocal("CoreCustomAttributes", "IsMixin");
            ECValue appliesToValue;
            ca->GetValue(appliesToValue, "AppliesToEntityClass");
            ASSERT_FALSE(appliesToValue.IsNull() || !appliesToValue.IsString());
            ASSERT_STRCASEEQ(appliesToValue.GetUtf8CP(), "SubElement");
            }
    );

    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IOptionA' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>SupportOption</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SubElement'>
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName='Text' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>SubElement</BaseClass>
                        <BaseClass>IOptionA</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            // ERROR ECDb - ECSchema Upgrade failed. MixIn TestSchema:IOptionA: Modifing 'AppliesToEntityClass' from TestSchema:SubElement to TestSchema:SupportOption is only
            // supported TestSchema:SubElement derived from TestSchema:SupportOption.ERROR ECDb - ECSchema Upgrade failed. MixIn TestSchema:IOptionA: Modifing 'AppliesToEntityClass'
            // from TestSchema:SubElement to TestSchema:SupportOption is only supported TestSchema:SubElement derived from TestSchema:SupportOption.
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateBaseClass_UpdateEmptyMixinBaseClass) //TFS#917566
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "5c08db0df328833f8f0c6d78bf9fc158f2d6070505cba41c94678787fefe883b";
    const auto SCHEMA1_HASH_ECDB_MAP = "0f051fda13a804dd21c322a60eac58c11e8eed22fe708473e0f67e1a0fe72dd3";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "b78e32379a70eae0f45eedbf149e17397796681525ae2aaa61602ea48ee60e60";
    Test(
        "Import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IOptionA' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='IOptionB' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>IOptionA</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check values of initial schema",
        [&]()
            {
            ECClassCP supportOption = m_briefcase->Schemas().GetClass("TestSchema", "SupportOption");
            ASSERT_NE(supportOption, nullptr);
            ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_STREQ(supportOption->GetBaseClasses().at(1)->GetFullName(), "TestSchema:IOptionA");
            ASSERT_EQ(2, supportOption->GetBaseClasses().size());
            }
    );

    Test(
        "import edited schema with some changes",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "759d70b1d3c691b8302d592f45a832f6e5db6a7022973f25da243866ce1b9e53";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IOptionA' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='IOptionB' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>IOptionB</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check values of edited schema",
        [&]()
            {
            ECClassCP supportOption = m_briefcase->Schemas().GetClass("TestSchema", "SupportOption");
            ASSERT_NE(supportOption, nullptr);
            ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_STREQ(supportOption->GetBaseClasses().at(1)->GetFullName(), "TestSchema:IOptionB");
            ASSERT_EQ(2, supportOption->GetBaseClasses().size());
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateBaseClass_UpdateEmptyMixinBaseClassWithNoneEmptyBaseClass) //TFS#917566
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "7ad20eacf7056b74415b761025f995bea76bd02f2d17f832da69180a19580819";
    const auto SCHEMA1_HASH_ECDB_MAP = "a8edf6c2bcf15cf34f9d9d5ca682f8c5b91e5a7c1d5ed2f43b766a46874c048a";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "b78e32379a70eae0f45eedbf149e17397796681525ae2aaa61602ea48ee60e60";
    Test(
        "Import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IOptionA' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='IOptionB' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P2' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>IOptionA</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check values of initial schema",
        [&]()
            {
            ECClassCP supportOption = m_briefcase->Schemas().GetClass("TestSchema", "SupportOption");
            ASSERT_NE(supportOption, nullptr);
            ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_STREQ(supportOption->GetBaseClasses().at(1)->GetFullName(), "TestSchema:IOptionA");
            ASSERT_EQ(2, supportOption->GetBaseClasses().size());
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "ae101d24c863665396cb8c18fd3fd133a2f8c4e476097c373544f4b85a15ad23";
    const auto SCHEMA2_HASH_ECDB_MAP = "ceeeb3903bb98b53a41d7d645e4458e500548906acf45e9703a29f1ee5becf2d";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "d26da3680fbcd83f8503e623223ef61a3158b896b8b951ff49f8e14433392d29";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IOptionA' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='IOptionB' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P2' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>IOptionB</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify we can insert and select",
        [&]()
            {
            auto expected = JsonValue(R"json([{"Code":"code", "P2":"p2", "P1":"p1"}])json");
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.SupportOption (Code,P2,P1) VALUES ('code', 'p2', 'p1')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code, P2, P1 FROM TestSchema.SupportOption WHERE Code='code'")) << "After swapping baseclass to none-empty mixin";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateBaseClass_AddMixinBaseClassWithProperties)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "a6a88b77528fe032a16b14ecce797348bd6780078f19328e87a3ef0574e32b93";
    const auto SCHEMA1_HASH_ECDB_MAP = "99db48b2ff01d60e90d407ce7613814e7f643f45e338b4fddc4631c791216290";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "b78e32379a70eae0f45eedbf149e17397796681525ae2aaa61602ea48ee60e60";
    Test(
        "Import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IMyMixin' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                         <ECProperty propertyName='M1' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check values of initial schema",
        [&]()
            {
            auto supportOption = m_briefcase->Schemas().GetClass("TestSchema", "SupportOption");
            ASSERT_NE(supportOption, nullptr);
            ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_EQ(1, supportOption->GetBaseClasses().size());
            ASSERT_EQ(2, supportOption->GetPropertyCount(true));
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "5f8d9bf6748719824e8759fd9f77c8cecf2f3b8cf67354f09e8b91b7caebbd98";
    const auto SCHEMA2_HASH_ECDB_MAP = "91059936ff9b0929c90d9b3f7fdfb07506a1af00d8aea6c8c986e543be2e75da";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "af059399e4d3f393a1229ae6ce5570ca36e1ba300462ea544fbaad6b7b0fdf99";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IMyMixin' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName='M1' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>IMyMixin</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check values of edited schema",
        [&]()
            {
            auto supportOption = m_briefcase->Schemas().GetClass("TestSchema", "SupportOption");
            ASSERT_NE(supportOption, nullptr);
            ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_STREQ(supportOption->GetBaseClasses().at(1)->GetFullName(), "TestSchema:IMyMixin");
            ASSERT_EQ(2, supportOption->GetBaseClasses().size());
            ASSERT_EQ(3, supportOption->GetPropertyCount(true));
            }
    );

    Test(
        "Verify we can insert and select",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.SupportOption (Code,M1,P1) VALUES ('code1', 'm1', 'p1')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            auto expected = JsonValue(R"json([{"Code":"code1", "M1":"m1", "P1":"p1"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code, M1, P1 FROM TestSchema.SupportOption WHERE Code='code1'")) << "Verify inserted instance";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            expected = JsonValue(R"json([{"Code":"code1"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code FROM TestSchema.Element")) << "Verify polymorphic query by base class";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            expected = JsonValue(R"json([{"M1":"m1"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT M1 FROM TestSchema.IMyMixin")) << "Verify polymorphic query by mixin";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateBaseClass_AddPropertiesToEmptyMixinBaseClass)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "91fd10fedd38dbd45d6960eeff6495477b75e76621ced242d03774f8d32c80d1";
    const auto SCHEMA1_HASH_ECDB_MAP = "c9270d435d7befb4a9d5ff6c7de4bdc04bbcd68c479186516f6b623742f82e3f";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "b78e32379a70eae0f45eedbf149e17397796681525ae2aaa61602ea48ee60e60";
    Test(
        "Import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IMyMixin' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>IMyMixin</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check values of initial schema",
        [&]()
            {
            auto supportOption = m_briefcase->Schemas().GetClass("TestSchema", "SupportOption");
            ASSERT_NE(supportOption, nullptr);
            ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_STREQ(supportOption->GetBaseClasses().at(1)->GetFullName(), "TestSchema:IMyMixin");
            ASSERT_EQ(2, supportOption->GetBaseClasses().size());
            ASSERT_EQ(2, supportOption->GetPropertyCount(true));
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "dbf8d623a723894133b50b3b0dcf5fac6e06904b430e6e0e8471437b8754178b";
    const auto SCHEMA2_HASH_ECDB_MAP = "5008b146ff9854e5920126032c5596befb30290a8e6808633c6f088c5b7daa2d";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "af059399e4d3f393a1229ae6ce5570ca36e1ba300462ea544fbaad6b7b0fdf99";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IMyMixin' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName='M1' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>IMyMixin</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check values of edited schema",
        [&]()
            {
            auto supportOption = m_briefcase->Schemas().GetClass("TestSchema", "SupportOption");
            ASSERT_EQ(2, supportOption->GetBaseClasses().size());
            ASSERT_EQ(3, supportOption->GetPropertyCount(true));
            }
    );

    Test(
        "Verify we can insert and select",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.SupportOption (Code,M1,P1) VALUES ('code2', 'm1', 'p1')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            auto expected = JsonValue(R"json([{"Code":"code2", "M1":"m1", "P1":"p1"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code, M1, P1 FROM TestSchema.SupportOption WHERE Code='code2'")) << "Verify inserted instance";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            // select polymorphically
            expected = JsonValue(R"json([{"Code":"code2"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code FROM TestSchema.Element")) << "Verify polymorphic query by base class";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            expected = JsonValue(R"json([{"M1":"m1"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT M1 FROM TestSchema.IMyMixin")) << "Verify polymorphic query by mixin";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateBaseClass_AddMixinWithPropertiesUsingTablePerHierarchy)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "e9f1488e9107a2bc6380f5ed4bfa6c8b05ce2c4b5168326054eca99eb94b2803";
    const auto SCHEMA1_HASH_ECDB_MAP = "b3c6024a0acb7774cde582406785f87759a9aafb04272b270cc88373fadd7dec";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "f11074eeef9a6137eb320fe95a7ec22c0286a000d8c146bbfd5fdbaa4c4d263c";
    Test(
        "Import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='Element'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.2.0'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName='ElementProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Pipe' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName='PipeProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Valve' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName='ValveProperty' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check values of initial schema",
        [&]()
            {
            auto pipe = m_briefcase->Schemas().GetClass("TestSchema", "Pipe");
            ASSERT_NE(pipe, nullptr);
            ASSERT_STREQ(pipe->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_EQ(1, pipe->GetBaseClasses().size());
            ASSERT_EQ(2, pipe->GetPropertyCount(true));

            auto valve = m_briefcase->Schemas().GetClass("TestSchema", "Valve");
            ASSERT_NE(valve, nullptr);
            ASSERT_STREQ(valve->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_EQ(1, valve->GetBaseClasses().size());
            ASSERT_EQ(2, valve->GetPropertyCount(true));
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "a0297ff0a92740d9fb9ec7d60ee31af2b97d7fce893874b490fdcd257805ae99";
    const auto SCHEMA2_HASH_ECDB_MAP = "933de8c603f3d1ad3fc2681482527febff72b521d5c0be81d77ab0f5cbd3ac86";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "1758836c9d933f4f73d60dfe3c4e42d2a2657a19534c6d0760a492b130bc8c40";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='TaggedPhysicalElement' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName='MixinProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.2.0'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName='ElementProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Pipe' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>TaggedPhysicalElement</BaseClass>
                        <ECProperty propertyName='PipeProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Valve' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>TaggedPhysicalElement</BaseClass>
                        <ECProperty propertyName='ValveProperty' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check values of edited schema",
        [&]()
            {
            auto pipe = m_briefcase->Schemas().GetClass("TestSchema", "Pipe");
            ASSERT_NE(pipe, nullptr);
            ASSERT_EQ(2, pipe->GetBaseClasses().size());
            ASSERT_STREQ(pipe->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_STREQ(pipe->GetBaseClasses().at(1)->GetFullName(), "TestSchema:TaggedPhysicalElement");
            ASSERT_EQ(3, pipe->GetPropertyCount(true));

            auto valve = m_briefcase->Schemas().GetClass("TestSchema", "Valve");
            ASSERT_NE(valve, nullptr);
            ASSERT_EQ(2, valve->GetBaseClasses().size());
            ASSERT_STREQ(valve->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_STREQ(pipe->GetBaseClasses().at(1)->GetFullName(), "TestSchema:TaggedPhysicalElement");
            ASSERT_EQ(3, valve->GetPropertyCount(true));
            }
    );

    Test(
        "Verify we can insert and select",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Pipe (ElementProperty,MixinProperty,PipeProperty) VALUES ('elem', 'mix', 'pipe')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            auto expected = JsonValue(R"json([{"ElementProperty":"elem", "MixinProperty":"mix", "PipeProperty":"pipe"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ElementProperty, MixinProperty, PipeProperty FROM TestSchema.Pipe")) << "Verify inserted pipe";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Valve (ElementProperty,MixinProperty,ValveProperty) VALUES ('elem2', 'mix', 'valve')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            expected = JsonValue(R"json([{"ElementProperty":"elem2", "MixinProperty":"mix", "ValveProperty":"valve"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ElementProperty, MixinProperty, ValveProperty FROM TestSchema.Valve")) << "Verify inserted valve";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            // select polymorphically
            expected = JsonValue(R"json([{"ElementProperty":"elem"}, {"ElementProperty":"elem2"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ElementProperty FROM TestSchema.Element")) << "Verify polymorphic query by base class";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            expected = JsonValue(R"json([{"MixinProperty":"mix"}, {"MixinProperty":"mix"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT MixinProperty FROM TestSchema.TaggedPhysicalElement")) << "Verify polymorphic query by mixin";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateBaseClass_AddMixinWithPropertiesUsingJoinedTablePerDirectSubclass)
    {
    Test(
        "Import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "29e8b92ebabe53a1993728b5e525374efd5b826f22631d0ab9c1753c79faed7f";
            const auto SCHEMA_HASH_ECDB_MAP = "cfb2e86d3499bc9ff634db79f2a28af36fd08edb590f1cb303b5c592406a327e";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "a477bab64de1d64aab3397e547b9bf8a7b54097147cab95ac4ce3f6c630fb109";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='Element'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.2.0'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <JoinedTablePerDirectSubclass xmlns='ECDbMap.2.0'/>
                        </ECCustomAttributes>
                        <ECProperty propertyName='ElementProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Pipe' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName='PipeProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Valve' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName='ValveProperty' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check values of initial schema",
        [&]()
            {
            ECClassCP pipe = m_briefcase->Schemas().GetClass("TestSchema", "Pipe");
            ASSERT_NE(pipe, nullptr);
            ASSERT_STREQ(pipe->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_EQ(1, pipe->GetBaseClasses().size());
            ASSERT_EQ(2, pipe->GetPropertyCount(true));

            ECClassCP valve = m_briefcase->Schemas().GetClass("TestSchema", "Valve");
            ASSERT_NE(valve, nullptr);
            ASSERT_STREQ(valve->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_EQ(1, valve->GetBaseClasses().size());
            ASSERT_EQ(2, valve->GetPropertyCount(true));
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "641c7582e52f303c190d69cedf1a2444a7a46edda8659a631fa3d2a3b6d1f4d8";
    const auto SCHEMA1_HASH_ECDB_MAP = "9562cfcfc61207ce64ab8655aa4ee952a0ba61f03df74d14bb48cf9dd09cd299";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "9a07435a3fd6822c10ccedc5b2153eebb7356a8e2b2d1f7aae58ecd4d3ff8bb3";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='TaggedPhysicalElement' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName='MixinProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.2.0'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <JoinedTablePerDirectSubclass xmlns='ECDbMap.2.0'/>
                        </ECCustomAttributes>
                        <ECProperty propertyName='ElementProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Pipe' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>TaggedPhysicalElement</BaseClass>
                        <ECProperty propertyName='PipeProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Valve' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>TaggedPhysicalElement</BaseClass>
                        <ECProperty propertyName='ValveProperty' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check values of edited schema",
        [&]()
            {
            auto pipe = m_briefcase->Schemas().GetClass("TestSchema", "Pipe");
            ASSERT_NE(pipe, nullptr);
            ASSERT_EQ(2, pipe->GetBaseClasses().size());
            ASSERT_STREQ(pipe->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_STREQ(pipe->GetBaseClasses().at(1)->GetFullName(), "TestSchema:TaggedPhysicalElement");
            ASSERT_EQ(3, pipe->GetPropertyCount(true));

            auto valve = m_briefcase->Schemas().GetClass("TestSchema", "Valve");
            ASSERT_NE(valve, nullptr);
            ASSERT_EQ(2, valve->GetBaseClasses().size());
            ASSERT_STREQ(valve->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_STREQ(pipe->GetBaseClasses().at(1)->GetFullName(), "TestSchema:TaggedPhysicalElement");
            ASSERT_EQ(3, valve->GetPropertyCount(true));
            }
    );

    Test(
        "Verify we can insert and select",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Pipe (ElementProperty,MixinProperty,PipeProperty) VALUES ('elem', 'mix', 'pipe')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            auto expected = JsonValue(R"json([{"ElementProperty":"elem", "MixinProperty":"mix", "PipeProperty":"pipe"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ElementProperty, MixinProperty, PipeProperty FROM TestSchema.Pipe WHERE ElementProperty='elem'")) << "Verify inserted pipe";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Valve (ElementProperty,MixinProperty,ValveProperty) VALUES ('elem2', 'mix', 'valve')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            expected = JsonValue(R"json([{"ElementProperty":"elem2", "MixinProperty":"mix", "ValveProperty":"valve"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ElementProperty, MixinProperty, ValveProperty FROM TestSchema.Valve WHERE ElementProperty='elem2'")) << "Verify inserted valve";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            // select polymorphically
            expected = JsonValue(R"json([{"ElementProperty":"elem"}, {"ElementProperty":"elem2"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ElementProperty FROM TestSchema.Element")) << "Verify polymorphic query by base class";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            expected = JsonValue(R"json([{"MixinProperty":"mix"}, {"MixinProperty":"mix"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT MixinProperty FROM TestSchema.TaggedPhysicalElement")) << "Verify polymorphic query by mixin";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateBaseClass_AddMixinWithPropertiesToMultipleClasses)
    {
    Test(
        "import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "58df4c7928036dee4aa71b4ff6e76e5820fd098833fe06c76502e28d06ef5387";
            const auto SCHEMA_HASH_ECDB_MAP = "bb15a5b43f57f0ccba568db5affb7b2711cccb177821d376b5a39d8ebce7a21f";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "5779abf6d328a4278fa7e618328d36ba97f4d6f12a533f12ac4c46b46df7f1fb";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='ElementProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Pipe' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName='PipeProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Valve' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName='ValveProperty' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check initial properties",
        [&]()
            {
            auto pipe = m_briefcase->Schemas().GetClass("TestSchema", "Pipe");
            ASSERT_NE(pipe, nullptr);
            ASSERT_STREQ(pipe->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_EQ(1, pipe->GetBaseClasses().size());
            ASSERT_EQ(2, pipe->GetPropertyCount(true));

            auto valve = m_briefcase->Schemas().GetClass("TestSchema", "Valve");
            ASSERT_NE(valve, nullptr);
            ASSERT_STREQ(valve->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_EQ(1, valve->GetBaseClasses().size());
            ASSERT_EQ(2, valve->GetPropertyCount(true));
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "fcb155b1eb25aa98560862415c59ed5e224c28ae0aaa7e93891c359d42fbc9ce";
    const auto SCHEMA1_HASH_ECDB_MAP = "8d5816e120c6d474b02db290daa2a68c1de0cc7765c3b926c41b815800a296da";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "8458e9fd519f11eab5bc2aa519df630fdc24d76709350ece4403d2b74af456a4";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='TaggedPhysicalElement' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName='MixinProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='ElementProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Pipe' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>TaggedPhysicalElement</BaseClass>
                        <ECProperty propertyName='PipeProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Valve' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>TaggedPhysicalElement</BaseClass>
                        <ECProperty propertyName='ValveProperty' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check edited properties",
        [&]()
            {
            auto pipe = m_briefcase->Schemas().GetClass("TestSchema", "Pipe");
            ASSERT_NE(pipe, nullptr);
            ASSERT_EQ(2, pipe->GetBaseClasses().size());
            ASSERT_STREQ(pipe->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_STREQ(pipe->GetBaseClasses().at(1)->GetFullName(), "TestSchema:TaggedPhysicalElement");
            ASSERT_EQ(3, pipe->GetPropertyCount(true));

            auto valve = m_briefcase->Schemas().GetClass("TestSchema", "Valve");
            ASSERT_NE(valve, nullptr);
            ASSERT_EQ(2, valve->GetBaseClasses().size());
            ASSERT_STREQ(valve->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_STREQ(pipe->GetBaseClasses().at(1)->GetFullName(), "TestSchema:TaggedPhysicalElement");
            ASSERT_EQ(3, valve->GetPropertyCount(true));
            }
    );

    Test(
        "Verify we can insert and select",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Pipe (ElementProperty,MixinProperty,PipeProperty) VALUES ('elem', 'mix', 'pipe')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            auto expected = JsonValue(R"json([{"ElementProperty":"elem", "MixinProperty":"mix", "PipeProperty":"pipe"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ElementProperty, MixinProperty, PipeProperty FROM TestSchema.Pipe WHERE ElementProperty='elem'")) << "Verify inserted pipe";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Valve (ElementProperty,MixinProperty,ValveProperty) VALUES ('elem2', 'mix', 'valve')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            expected = JsonValue(R"json([{"ElementProperty":"elem2", "MixinProperty":"mix", "ValveProperty":"valve"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ElementProperty, MixinProperty, ValveProperty FROM TestSchema.Valve WHERE ElementProperty='elem2'")) << "Verify inserted valve";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            // select polymorphically
            expected = JsonValue(R"json([{"ElementProperty":"elem"}, {"ElementProperty":"elem2"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ElementProperty FROM TestSchema.Element")) << "Verify polymorphic query by base class";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            expected = JsonValue(R"json([{"MixinProperty":"mix"}, {"MixinProperty":"mix"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT MixinProperty FROM TestSchema.TaggedPhysicalElement")) << "Verify polymorphic query by mixin";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateClass_ChangeAbstractIntoConcreteClass)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "3cbb5c2f7efe8bcf3d2397470e9e6f68616db3858ca44361d3dea94414359eb1";
    const auto SCHEMA1_HASH_ECDB_MAP = "f36e370f003c951538b8aa6e2091d10a8bf0ab9a41c9ff4d0a4635de2e1db79b";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "65cde579ee0a43a9fc97e38e93f8fc99ab5654a66b1f7b9f9a3ce6ba815d1709";
    Test(
        "import initial schema",
        [&]()
            {
            
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='Base' modifier='Abstract'>
                        <ECProperty propertyName='BaseProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='MyClass' modifier='Abstract'>
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName='MyProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Subclass' modifier='None' >
                        <BaseClass>MyClass</BaseClass>
                        <ECProperty propertyName='SubclassProperty' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify initial state of the class is abstract",
        [&]()
            {
            auto subclass = m_briefcase->Schemas().GetClass("TestSchema", "Subclass");
            ASSERT_NE(subclass, nullptr);
            ASSERT_STREQ(subclass->GetBaseClasses().at(0)->GetFullName(), "TestSchema:MyClass");
            ASSERT_EQ(1, subclass->GetBaseClasses().size());
            ASSERT_EQ(3, subclass->GetPropertyCount(true));
            auto myClass = m_briefcase->Schemas().GetClass("TestSchema", "MyClass");
            ASSERT_NE(myClass, nullptr);
            ASSERT_EQ(ECClassModifier::Abstract, myClass->GetClassModifier()) << "Verify initial state of the class is abstract";
            }
    );

    Test(
        "Importing schema should fail because abstract to concrete class requires Table per Hierarchy mapping strategy",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='Base' modifier='Abstract'>
                        <ECProperty propertyName='BaseProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='MyClass' modifier='None'>
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName='MyProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Subclass' modifier='None' >
                        <BaseClass>MyClass</BaseClass>
                        <ECProperty propertyName='SubclassProperty' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateClass_ChangeAbstractIntoConcreteClassUsingTablePerHierarchy)
    {
    const auto SCHEMA1_HASH_ECDB_MAP = "d3f5e8be598cf2bb98baa4e0db59e3f9105a4d5a7acf4d61551c7ed425f1fc23";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "6aa2dbfb84f8ac973166c4b3f1ecd49072539312f46945f8ec895f6472af5dae";
    Test(
        "import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "02ad8f8f5b0afdbb27a0949e18ba3358a7daeba865a1cb7da5621ee13246527d";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Base' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName='BaseProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='MyClass' modifier='Abstract'>
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName='MyProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Subclass' modifier='None' >
                        <BaseClass>MyClass</BaseClass>
                        <ECProperty propertyName='SubclassProperty' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check initial properties",
        [&]()
            {
            auto subclass = m_briefcase->Schemas().GetClass("TestSchema", "Subclass");
            ASSERT_NE(subclass, nullptr);
            ASSERT_STREQ(subclass->GetBaseClasses().at(0)->GetFullName(), "TestSchema:MyClass");
            ASSERT_EQ(1, subclass->GetBaseClasses().size());
            ASSERT_EQ(3, subclass->GetPropertyCount(true));
            auto myClass = m_briefcase->Schemas().GetClass("TestSchema", "MyClass");
            ASSERT_NE(myClass, nullptr);
            ASSERT_EQ(ECClassModifier::Abstract, myClass->GetClassModifier()) << "Verify initial state of the class is abstract";
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "61da328291706941c2411483bd902b7f292a285fb48be8707b2ef15cffc194aa";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Base' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName='BaseProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='MyClass' modifier='None'>
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName='MyProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Subclass' modifier='None' >
                        <BaseClass>MyClass</BaseClass>
                        <ECProperty propertyName='SubclassProperty' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check edited properties",
        [&]()
            {
            auto subclass = m_briefcase->Schemas().GetClass("TestSchema", "Subclass");
            ASSERT_NE(subclass, nullptr);
            ASSERT_STREQ(subclass->GetBaseClasses().at(0)->GetFullName(), "TestSchema:MyClass");
            ASSERT_EQ(1, subclass->GetBaseClasses().size());
            ASSERT_EQ(3, subclass->GetPropertyCount(true));
            auto myClass = m_briefcase->Schemas().GetClass("TestSchema", "MyClass");
            ASSERT_NE(myClass, nullptr);
            ASSERT_EQ(ECClassModifier::None, myClass->GetClassModifier()) << "Verify new state of the class is abstract";
            }
    );

    Test(
        "Verify we can insert and select",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.MyClass (BaseProperty,MyProperty) VALUES ('base', 'value')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            auto expected = JsonValue(R"json([{"BaseProperty":"base", "MyProperty":"value"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT BaseProperty, MyProperty FROM TestSchema.MyClass WHERE MyProperty='value'")) << "Verify inserted instance";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            // select polymorphically
            expected = JsonValue(R"json([{"BaseProperty":"base"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT BaseProperty FROM TestSchema.Base")) << "Verify polymorphic query by base class";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateClass_ChangeAbstractIntoConcreteClassWithAbstractSubclass)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "ef7df83e0166bd2d4c27a1f6be29ad5066c0c379cf04f2c5755eab4948c2d5eb";
    const auto SCHEMA1_HASH_ECDB_MAP = "810cf1c4e8e145a16b78f6397de980046bbebdfb798d929a0fd22de86760badf";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "438204e1fab816e0580192172d6aee534a6d38c387c4ff21cf72899abb81f659";
    Test(
        "import initial schema",
        [&]()
            {
            
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Base' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName='BaseProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='MyClass' modifier='Abstract'>
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName='MyProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Subclass' modifier='None' >
                        <BaseClass>MyClass</BaseClass>
                        <ECProperty propertyName='SubclassProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='AbstractSubclass' modifier='Abstract' >
                        <BaseClass>MyClass</BaseClass>
                        <ECProperty propertyName='AbstractSubclassProperty' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify initial state of the class is abstract",
        [&]()
            {
            auto subclass = m_briefcase->Schemas().GetClass("TestSchema", "Subclass");
            ASSERT_NE(subclass, nullptr);
            ASSERT_STREQ(subclass->GetBaseClasses().at(0)->GetFullName(), "TestSchema:MyClass");
            ASSERT_EQ(1, subclass->GetBaseClasses().size());
            ASSERT_EQ(3, subclass->GetPropertyCount(true));
            auto myClass = m_briefcase->Schemas().GetClass("TestSchema", "MyClass");
            ASSERT_NE(myClass, nullptr);
            ASSERT_EQ(ECClassModifier::Abstract, myClass->GetClassModifier()) << "Verify initial state of the class is abstract";
            }
    );

    Test(
        "Import should fail because class has an abstract subclass",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Base' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName='BaseProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='MyClass' modifier='None'>
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName='MyProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Subclass' modifier='None' >
                        <BaseClass>MyClass</BaseClass>
                        <ECProperty propertyName='SubclassProperty' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='AbstractSubclass' modifier='Abstract' >
                        <BaseClass>MyClass</BaseClass>
                        <ECProperty propertyName='AbstractSubclassProperty' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateBaseClass_AddNewEmptyMixinBaseClasses) //TFS#917566
    {
    const auto SCHEMA1_HASH_ECDB_MAP = "0f051fda13a804dd21c322a60eac58c11e8eed22fe708473e0f67e1a0fe72dd3";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "b78e32379a70eae0f45eedbf149e17397796681525ae2aaa61602ea48ee60e60";
    Test(
        "import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "b80b1572b2d7ee361b0ea3b4c8e391b306c99c871b518fdb572a59bfea0f48f8";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IOptionA' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='IOptionB' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    
    Test(
        "Check initial base classes",
        [&]()
            {
            ECClassCP supportOption = m_briefcase->Schemas().GetClass("TestSchema", "SupportOption");
            ASSERT_NE(supportOption, nullptr);
            ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_EQ(1, supportOption->GetBaseClasses().size());
            }
    );

    Test(
        "import edited schema with some changes",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "bcc0e54e76ae8d56242bce0f21127ad6705c3b56c39cfbd2e5334d24a44cca21";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IOptionA' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='IOptionB' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>IOptionA</BaseClass>
                        <BaseClass>IOptionB</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check changed base classes",
        [&]()
            {
            auto supportOption = m_briefcase->Schemas().GetClass("TestSchema", "SupportOption");
            ASSERT_NE(supportOption, nullptr);
            ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_STREQ(supportOption->GetBaseClasses().at(1)->GetFullName(), "TestSchema:IOptionA");
            ASSERT_STREQ(supportOption->GetBaseClasses().at(2)->GetFullName(), "TestSchema:IOptionB");
            ASSERT_EQ(3, supportOption->GetBaseClasses().size());
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateBaseClass_AddNewNoneEmptyMixinBaseClasses) //TFS#917566
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "12a54c2055905d404d6003aba4fa385f715e958ed7346def372812f12bd452ff";
    const auto SCHEMA1_HASH_ECDB_MAP = "a8edf6c2bcf15cf34f9d9d5ca682f8c5b91e5a7c1d5ed2f43b766a46874c048a";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "b78e32379a70eae0f45eedbf149e17397796681525ae2aaa61602ea48ee60e60";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IOptionA' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='IOptionB' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P2' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check base classes",
        [&]()
            {
            ECClassCP supportOption = m_briefcase->Schemas().GetClass("TestSchema", "SupportOption");
            ASSERT_NE(supportOption, nullptr);
            ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_EQ(1, supportOption->GetBaseClasses().size());
            }
    );

    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IOptionA' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='IOptionB' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>IOptionA</BaseClass>
                        <BaseClass>IOptionB</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateBaseClass_RemoveNoneEmptyMixinBaseClasses) //TFS#917566
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "7a24c6ccf285cbbb05c24ba11bf2cb9dda9ef03d268d8f653afecd00daf855d6";
    const auto SCHEMA1_HASH_ECDB_MAP = "9c38626b24eb982d979cc6fdd6beea9ef3d7cce5f7a4269e293ee65bc70a585f";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "02e3675f2f7dbdce7f0fd584f39452d0befaacda3f39a3cecd454452916118f4";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IOptionA' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='IOptionB' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P2' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>IOptionA</BaseClass>
                        <BaseClass>IOptionB</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check base classes",
        [&]()
            {
            auto supportOption = m_briefcase->Schemas().GetClass("TestSchema", "SupportOption");
            ASSERT_NE(supportOption, nullptr);
            ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_STREQ(supportOption->GetBaseClasses().at(1)->GetFullName(), "TestSchema:IOptionA");
            ASSERT_STREQ(supportOption->GetBaseClasses().at(2)->GetFullName(), "TestSchema:IOptionB");
            ASSERT_EQ(3, supportOption->GetBaseClasses().size());
            }
    );

    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IOptionA' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='IOptionB' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                         <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateBaseClass_RemoveEmptyMixinBaseClasses) //TFS#917566
    {
    const auto SCHEMA1_HASH_ECDB_MAP = "9c38626b24eb982d979cc6fdd6beea9ef3d7cce5f7a4269e293ee65bc70a585f";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "02e3675f2f7dbdce7f0fd584f39452d0befaacda3f39a3cecd454452916118f4";
    Test(
        "import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "7a24c6ccf285cbbb05c24ba11bf2cb9dda9ef03d268d8f653afecd00daf855d6";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IOptionA' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='IOptionB' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P2' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <BaseClass>IOptionA</BaseClass>
                        <BaseClass>IOptionB</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check initial base classes",
        [&]()
            {
            auto supportOption = m_briefcase->Schemas().GetClass("TestSchema", "SupportOption");
            ASSERT_NE(supportOption, nullptr);
            ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_STREQ(supportOption->GetBaseClasses().at(1)->GetFullName(), "TestSchema:IOptionA");
            ASSERT_STREQ(supportOption->GetBaseClasses().at(2)->GetFullName(), "TestSchema:IOptionB");
            ASSERT_EQ(3, supportOption->GetBaseClasses().size());
            }
    );

    Test(
        "import edited schema with some changes",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "1b6881906ec11265fce16e7458b341dfe4e95f05505a9d438f9a0f8093c46d73";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='IOptionA' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='IOptionB' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P2' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Element'>
                        <ECProperty propertyName='Code' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='SupportOption' modifier='None' >
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName='P1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check changed base classes",
        [&]()
            {
            auto supportOption = m_briefcase->Schemas().GetClass("TestSchema", "SupportOption");
            ASSERT_NE(supportOption, nullptr);
            ASSERT_STREQ(supportOption->GetBaseClasses().at(0)->GetFullName(), "TestSchema:Element");
            ASSERT_EQ(1, supportOption->GetBaseClasses().size());
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, TryRemoveMixinCustomAttribute_Simple) //TFS#917566
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "6562bc3c968eab4d84592a41aa2e962efbe8ef1f802e65520357b93626139538";
    const auto SCHEMA1_HASH_ECDB_MAP = "d999bff4980bcf59dde6e6ab7c005fce26e9f73427da1bbed253dd8d804f8b54";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "77af11ada178c9b759bec7717d606ddd7d1ccfae9168df93c3918729958ef815";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='ISourceEnd' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>TestClass</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import not allowed edited schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='ISourceEnd' modifier='Abstract'/>
                    <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, TryAddMixinCustomAttribute_Simple) //TFS#917566
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "a4753f921912d121e4cfa807c7dd52abc20aa36d7bc2c94f42f4db39bb0f4503";
    const auto SCHEMA1_HASH_ECDB_MAP = "b03dde17dc37b789b8ee05ec452e340624a55926e4ff922b407483a08eee60ac";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "c2290bfec905956783ae44b90262c073cb92a3fc20bd4966cdd29cef79242384";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='ISourceEnd' modifier='Abstract'/>
                    <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import not allowed edited schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='ISourceEnd' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>TestClass</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, TryRemoveMixinCustomAttribute_Complex) //TFS#917566
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "24ef7e7c2b9c0242a6c92c50f8d39766be5b56fb8f18122c112456d5a8a57f70";
    const auto SCHEMA1_HASH_ECDB_MAP = "d999bff4980bcf59dde6e6ab7c005fce26e9f73427da1bbed253dd8d804f8b54";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "77af11ada178c9b759bec7717d606ddd7d1ccfae9168df93c3918729958ef815";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='ISourceEnd' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>TestClass</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >
                        <BaseClass>ISourceEnd</BaseClass>
                        <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import not allowed edited schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' prefix='CoreCA'/>
                    <ECEntityClass typeName='ISourceEnd' modifier='Abstract'>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00'>
                                <AppliesToEntityClass>TestClassA</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='TestClassA' displayLabel='Modified Test Class' description='modified test class' modifier='None' >
                        <BaseClass>ISourceEnd</BaseClass>
                        <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateECPropertyAttributes)
    {
    const auto SCHEMA1_HASH_ECDB_MAP = "82a4f93a397725447cf558de379c6f5a4c044c2de789edf25e9f889fbaf52a43";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "77af11ada178c9b759bec7717d606ddd7d1ccfae9168df93c3918729958ef815";
    Test(
        "import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "349db63db10ca6797014ccae53a798eecacf449f4ad6df718b900f9c92e83069";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='Test Property' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("UpdateECClassAttributes", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "955bd52f9972f919d018453691cea75d986f12fb4d7266a34347f200fafbfa74";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' displayLabel='New Test Schema' description='This is a New Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='My Test Property' description='My Test Property' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema)) << "Modifying ECProperty display label and description is expected to be supported";
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify we can insert and select",
        [&]()
            {
            ECSqlStatement stmt;
            auto expected = JsonValue(R"json([{"DisplayLabel":"My Test Property", "Description":"My Test Property"}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='TestProperty'")) << "After modifying display label and description";
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdatingECDbMapCAIsNotSupported)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "562ab49478e43dd3a03b0de8eb098700754fcf4c1978d2d008e5986d1a5ab0cc";
    const auto SCHEMA1_HASH_ECDB_MAP = "3bb311a6b4f0d149920e1f6104bef90ca2da52e91a5942a78b3e537dbc56ba2c";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "1b91d646402a1bb5aeee79121711565bbd6de62b952d7acdb376a77348ec19e1";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' >
                            <ECCustomAttributes>
                                <PropertyMap xmlns='ECDbMap.02.00'>
                                    <IsNullable>false</IsNullable>
                                </PropertyMap>
                            </ECCustomAttributes>
                        </ECProperty>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import not allowed edited schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts_modified' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >
                            <ECCustomAttributes>
                                <PropertyMap xmlns='ECDbMap.02.00'>
                                    <IsNullable>true</IsNullable>
                                </PropertyMap>
                            </ECCustomAttributes>
                        </ECProperty>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// seems to be fixed but it isn't the same as original
TEST_F(SchemaSyncTestFixture, ClassModifier)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "ee7e9dd70d06ee3b800630fdb4ba49dfeb9064aa4450d59a1feac5dec06c8668";
    const auto SCHEMA1_HASH_ECDB_MAP = "2d75103ef619198e771a6760f9b2169630e4746a230746084b742a815914b93a";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "f3b0934f2e24da442a3d609e1760192c8c0609761d01d3f2ecf8584f82db6d78";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />
                    <ECEntityClass typeName='Koo' modifier='Abstract' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='L1' typeName='long' />
                        <ECProperty propertyName='S1' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Foo' modifier='Sealed' >
                        <BaseClass>Koo</BaseClass>
                        <ECProperty propertyName='L2' typeName='long' />
                        <ECProperty propertyName='S2' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Goo' modifier='Abstract' >
                        <ECProperty propertyName='L3' typeName='long' />
                        <ECProperty propertyName='S3' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Boo' modifier='None' >
                        <BaseClass>Goo</BaseClass>
                        <ECProperty propertyName='L4' typeName='long' />
                        <ECProperty propertyName='S4' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Moo' modifier='Sealed' >
                        <BaseClass>Boo</BaseClass>
                        <ECProperty propertyName='L5' typeName='long' />
                        <ECProperty propertyName='S5' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify insertion",
        [&]()
            {
            //! We only like to see if insertion works. If data is left then import will fail for second schema as we do not allow rows
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')")); //Abstract
            ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step()); // Original tests have this set to BE_SQLITE_DONE
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Foo (L2, S2) VALUES (2, 't2')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Goo (L3, S3) VALUES (3, 't3')")); //Abstract
            ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step()); // Original tests have this set to BE_SQLITE_DONE
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Boo (L4, S4) VALUES (4, 't4')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Moo (L5, S5) VALUES (5, 't5')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "2a5c8632f8003ed0b7ba494a20f0ec581c93d69b7d8ac7ebe0d7981934530a8f";
    const auto SCHEMA2_HASH_ECDB_MAP = "46fdfb8422421c527c2f9366140bb1bfc0cae62e1b4f891bfdd17783c4049fcf";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "f8304802c56773492595cb4d133e846f7a917c4ddc38de9697fbadbf429110d1";
    Test(
        "Delete some properties",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Koo' modifier='Abstract' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='L1' typeName='long' />
                        <ECProperty propertyName='S1' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Foo' modifier='None' >
                        <BaseClass>Koo</BaseClass>
                        <ECProperty propertyName='L2' typeName='long' />
                        <ECProperty propertyName='S2' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Voo' modifier='Sealed' >
                        <BaseClass>Koo</BaseClass>
                        <ECProperty propertyName='L6' typeName='long' />
                        <ECProperty propertyName='S6' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Goo' modifier='Abstract' >
                        <ECProperty propertyName='L3' typeName='long' />
                        <ECProperty propertyName='S3' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Boo' modifier='Sealed' >
                        <BaseClass>Goo</BaseClass>
                        <ECProperty propertyName='L4' typeName='long' />
                        <ECProperty propertyName='S4' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            m_briefcase->GetTracker()->EnableTracking(false); // Cannot make arbitrary schema changes when changes are being tracked
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            m_briefcase->GetTracker()->EnableTracking(true);
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify insertion after deletion",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (6, 't6')"));
            ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step()); // Original tests have this set to BE_SQLITE_DONE
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Foo (L2, S2) VALUES (7, 't7')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Boo (L4, S4) VALUES (10, 't10')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Moo (L5, S5) VALUES (11, 't11')"));
            ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step()); // Original tests have this set to BE_SQLITE_DONE
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Voo (L6, S6) VALUES (12, 't12')")); //New class added
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Goo (L3, S3) VALUES (8, 't8')")); //Class is still abstract
            ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step()); // Original tests have this set to BE_SQLITE_DONE
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// seems to be fixed but it isn't the same as original
TEST_F(SchemaSyncTestFixture, UpdateECClassModifierToAbstract)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "c7de1e4aa5931e37184e9a9721d75beba4c9beb69e864e21a19948fe36408a11";
    const auto SCHEMA1_HASH_ECDB_MAP = "e17f55418ebef8c9bef4d3ddad32ede1da12ef4be0c12590b0a7791cf98626a5";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "1841a00c77366e8a564659a76bb6bc962ae8dab25e94ba8e10e72e74163762af";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Koo' modifier='Abstract' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='L1' typeName='long' />
                        <ECProperty propertyName='S1' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Foo' modifier='Sealed' >
                        <BaseClass>Koo</BaseClass>
                        <ECProperty propertyName='L2' typeName='long' />
                        <ECProperty propertyName='S2' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify insertion",
        [&]()
            {
            //! We only like to see if insertion works. If data is left then import will fail for second schema as we do not allow rows
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')")) << "Should be invalidECSql because Koo is abstract"; //Abstract
            ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step()); // Original tests have this set to BE_SQLITE_DONE
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Foo (L2, S2) VALUES (2, 't2')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Delete some properties",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Koo' modifier='Abstract' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='L1' typeName='long' />
                        <ECProperty propertyName='S1' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Foo' modifier='Abstract' >
                        <BaseClass>Koo</BaseClass>
                        <ECProperty propertyName='L2' typeName='long' />
                        <ECProperty propertyName='S2' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// AssertSchemaUpdate like this?
TEST_F(SchemaSyncTestFixture, ModifyECClassModifierFromAbstract)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "bf9aa4ff08cae9b421fabee7d46daf5fab2cd6d16c5f28a39646fe6e506430ce";
    const auto SCHEMA1_HASH_ECDB_MAP = "13b955ae789dfff78ff1960ab1743a130b8fe229c55bb1c26c264ab30e3c20f6";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECEntityClass typeName="Foo" modifier="Abstract">
                       <ECProperty propertyName="L1" typeName="long" />
                       <ECProperty propertyName="S1" typeName="string"/>
                   </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate_modifyclassmodifiertoabstract", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Change Abstract to Sealed is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECEntityClass typeName="Foo" modifier="Sealed">
                       <ECProperty propertyName="L1" typeName="long" />
                       <ECProperty propertyName="S1" typeName="string"/>
                   </ECEntityClass>
                </ECSchema>)xml"
            );
            auto newBriefcase = m_hub->CreateBriefcase();

            // AssertSchemaUpdate like this?
            // Close db?
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->AbandonChanges());
            CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Change Abstract to None is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECEntityClass typeName="Foo" modifier="None">
                       <ECProperty propertyName="L1" typeName="long" />
                       <ECProperty propertyName="S1" typeName="string"/>
                   </ECEntityClass>
                </ECSchema>)xml"
            );
            auto newBriefcase = m_hub->CreateBriefcase();

            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->AbandonChanges());
            CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UnsealingClasses)
    {
    Test(
        "index on sealed class",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "8d813b715570258426e6ef865c334cb85f31d7d6c875937bf476b76d4f20ee1d";
            const auto SCHEMA_HASH_ECDB_MAP = "d9821d2695c24184c473dd50d0ab2bf871e5d75973b4fe8769ddee4c17147f4f";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "c28a6999c16ddb5345755ba262ff87ff25b749904d1172f5eb9cb0fe84436a15";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="Foo" modifier="Sealed">
                        <ECProperty propertyName="Prop" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("UnsealingClasses", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Class modifier changed from Sealed to None",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "996b40e522d4aae62e9bce7327cc0144c48aa08dbaf3ec298d3040a7ba042701";
            const auto SCHEMA_HASH_ECDB_MAP = "2876a1a05f4ce5abfd1aa6e5e120c3875bf3e602c6653ed68dc574ad4c2030ff";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "fe79f0372911254f2d389f7f6cda4f5156323aa27a012dae5dfafead03356e5c";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Foo" modifier="None">
                        <ECProperty propertyName="Prop" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub" modifier="None">
                        <BaseClass>Foo</BaseClass>
                        <ECProperty propertyName="Prop" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA1_HASH_ECDB_MAP = "e6a37ef1d3385534c16c6d0fbe860ee8574573dd4679b4466817045914880795";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "54a752668d77998991b81d3b20d62e736987c417b4700f143a8b8d25e35cffe2";
    Test(
        "sealed sub class (TPH)",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "35423da602d7c562aa0f9d3bab033fce84d446368265e6563309bafed230f77c";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Base" modifier="None">
                            <ECCustomAttributes>
                                <ClassMap xmlns="ECDbMap.02.00">
                                    <MapStrategy>TablePerHierarchy</MapStrategy>
                                </ClassMap>
                            </ECCustomAttributes>
                        <ECProperty propertyName="Prop" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub" modifier="Sealed">
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName="SubProp" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("UnsealingClasses", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Unsealing subclass (TPH)",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "04890ea5d0f5f98ac0259aa6d93eb8e4390a4070f248371f366895130aac3147";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Base" modifier="None">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Prop" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub" modifier="None">
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName="SubProp" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, DeleteProperty_OwnTable)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "fe10b63d32993ad2e31dd837354ece078653038df6f3efaf8eb393054f46d80b";
    const auto SCHEMA1_HASH_ECDB_MAP = "cd50ebc7301ff18e1fde52155610c1b4ae9e499fade8addd2a113894732ed4b5";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "9e2ae803e7d0f7bdc493ad26b31d668d40dee102d5812ce4ec395ae6baf684e9";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Foo' modifier='Abstract' >
                        <ECProperty propertyName='L1' typeName='long' />
                        <ECProperty propertyName='S1' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Goo' modifier='None' >
                        <BaseClass>Foo</BaseClass>
                        <ECProperty propertyName='L2' typeName='long' />
                        <ECProperty propertyName='S2' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Delete some properties",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Foo' modifier='None' >
                        <ECProperty propertyName='L1' typeName='long' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Goo' modifier='None' >
                        <BaseClass>Foo</BaseClass>
                        <ECProperty propertyName='L2' typeName='long' />
                        <ECProperty propertyName='S2' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            auto newBriefcase = m_hub->CreateBriefcase();

            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->AbandonChanges());
            CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// stmt.Step() after invalidECSql returns BE_SQLITE_ERROR, probably should be like that but why is is not the same in upgrade schemas
TEST_F(SchemaSyncTestFixture, DeleteProperties_TPH)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "65ddc9359aa2880b15d9f44ce347e57c8b30d4694321b355e352278fb25a0ad9";
    const auto SCHEMA1_HASH_ECDB_MAP = "ac96fd49a53bf356e7daa58ec225adc4f3b90d7d29102ce344b03afe39a02855";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "1225552890cfc5ee217bed5541d8b8ecd731a7c18b439eeefaa8cc88a23f76b2";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Koo' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='L1' typeName='long' />
                        <ECProperty propertyName='S1' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Foo' modifier='None' >
                        <BaseClass>Koo</BaseClass>
                        <ECProperty propertyName='L2' typeName='long' />
                        <ECProperty propertyName='S2' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Goo' modifier='None' >
                        <BaseClass>Foo</BaseClass>
                        <ECProperty propertyName='L3' typeName='long' />
                        <ECProperty propertyName='S3' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Boo' modifier='None' >
                        <BaseClass>Goo</BaseClass>
                        <ECProperty propertyName='L4' typeName='long' />
                        <ECProperty propertyName='S4' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert a row for each class",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Foo (L1, S1, L2, S2) VALUES (2, 't2', 3, 't3')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Goo (L1, S1, L2, S2, L3, S3) VALUES (4, 't4', 5, 't5', 6,'t6')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Boo (L1, S1, L2, S2, L3, S3, L4, S4) VALUES (5, 't5', 6, 't6', 7,'t7', 8,'t8')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "fe7c215ce898bc12a15e0a637d92bd374be6e1d2644a42b843561ea6a1322890";
    const auto SCHEMA2_HASH_ECDB_MAP = "116ca5c77ee25c3b5055e2ca89c85af4c53f7e793bb26832c2ec21192f1c7e01";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "f93c3722211619a903094007405292d4a5adc61320df811a157c8e535c3e58ee";
    Test(
        "Delete some properties",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Koo' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='L1' typeName='long' />
                        <ECProperty propertyName='S1' typeName='string' />
                        <ECProperty propertyName='D1' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Foo' modifier='None' >
                        <BaseClass>Koo</BaseClass>
                        <ECProperty propertyName='L2' typeName='long' />
                        <ECProperty propertyName='D2' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Goo' modifier='None' >
                        <BaseClass>Foo</BaseClass>
                        <ECProperty propertyName='L3' typeName='long' />
                        <ECProperty propertyName='D3' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Boo' modifier='None' >
                        <BaseClass>Goo</BaseClass>
                        <ECProperty propertyName='L4' typeName='long' />
                        <ECProperty propertyName='D4' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "see if ECSQL fail for a property which has been deleted",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Foo (L1, S1, L2, S2) VALUES (2, 't2',3, 't3')"));
            ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Goo (L1, S1, L2, S2, L3, S3) VALUES (4, 't4', 5, 't5', 6,'t6')"));
            ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Boo (L1, S1, L2, S2, L3, S3, L4, S4) VALUES (5, 't5', 6, 't6', 7,'t7', 8,'t8')"));
            ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Ensure new property is null for existing rows",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ECInstanceId FROM ONLY TestSchema.Koo WHERE D1 IS NULL"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ECInstanceId FROM ONLY TestSchema.Foo WHERE D1 IS NULL AND D2 IS NULL"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ECInstanceId FROM ONLY TestSchema.Goo WHERE D1 IS NULL AND D2 IS NULL AND D3 IS NULL"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ECInstanceId FROM ONLY TestSchema.Boo WHERE D1 IS NULL AND D2 IS NULL AND D3 IS NULL AND D4 IS NULL"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert new row with new value",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Koo (L1, S1, D1) VALUES (1, 't1', 'd1')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Foo (L1, S1, D1, L2, D2) VALUES (2, 't2', 'd2',3, 'd3')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Goo (L1, S1, D1, L2, D2, L3, D3) VALUES (4, 't3', 'd4', 5, 'd5',6 ,'d6')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Boo (L1, S1, D1, L2, D2, L3, D3, L4, D4) VALUES (5, 't4', 'd7', 6, 'd8',7 ,'d9', 8,'d10')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, DeleteProperties_JoinedTable)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "e7728296b1f07d78473151518deedfb2d7344f4c30a93a7eac3f0bee3804ce4b";
    const auto SCHEMA1_HASH_ECDB_MAP = "4c73213a48e882d88a453e9204d6f6067836588beb7351d7d3b24aabd7b914a4";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "9be4fbd40e62d7b5a8ed26922b331cd12f446a492e91a534ed713325d604250d";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Koo' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>
                        </ECCustomAttributes>
                        <ECProperty propertyName='L1' typeName='long' />
                        <ECProperty propertyName='S1' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Foo' modifier='None' >
                        <BaseClass>Koo</BaseClass>
                        <ECProperty propertyName='L2' typeName='long' />
                        <ECProperty propertyName='S2' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Goo' modifier='None' >
                        <BaseClass>Foo</BaseClass>
                        <ECProperty propertyName='L3' typeName='long' />
                        <ECProperty propertyName='S3' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Boo' modifier='None' >
                        <BaseClass>Goo</BaseClass>
                        <ECProperty propertyName='L4' typeName='long' />
                        <ECProperty propertyName='S4' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert a row for each class",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Foo (L1, S1, L2, S2) VALUES (2, 't2', 3, 't3')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Goo (L1, S1, L2, S2, L3, S3) VALUES (4, 't4', 5, 't5', 6,'t6')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Boo (L1, S1, L2, S2, L3, S3, L4, S4) VALUES (5, 't5', 6, 't6', 7,'t7', 8,'t8')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "f1fbfc4221450f71a2a90692e7c64c92ddc563ba9eff985f175bddd79e3a6cf1";
    const auto SCHEMA2_HASH_ECDB_MAP = "955ad0e567105cc07df037a47fd81f4376defdddc225ef83f6d666cf3b51bc24";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "0803494fab1a844dceae1d44d99227ac41727b1aa05fd0896ae903c72f295122";
    Test(
        "Delete some properties",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Koo' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>
                        </ECCustomAttributes>
                        <ECProperty propertyName='L1' typeName='long' />
                        <ECProperty propertyName='S1' typeName='string' />
                        <ECProperty propertyName='D1' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Foo' modifier='None' >
                        <BaseClass>Koo</BaseClass>
                        <ECProperty propertyName='L2' typeName='long' />
                        <ECProperty propertyName='D2' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Goo' modifier='None' >
                        <BaseClass>Foo</BaseClass>
                        <ECProperty propertyName='L3' typeName='long' />
                        <ECProperty propertyName='D3' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Boo' modifier='None' >
                        <BaseClass>Goo</BaseClass>
                        <ECProperty propertyName='L4' typeName='long' />
                        <ECProperty propertyName='D4' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "see if ECSQL fail for a property which has been deleted",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Koo (L1, S1) VALUES (1, 't1')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Foo (L1, S1, L2, S2) VALUES (2, 't2',3, 't3')"));
            ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Goo (L1, S1, L2, S2, L3, S3) VALUES (4, 't4', 5, 't5', 6,'t6')"));
            ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Boo (L1, S1, L2, S2, L3, S3, L4, S4) VALUES (5, 't5', 6, 't6', 7,'t7', 8,'t8')"));
            ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Ensure new property is null for existing rows",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ECInstanceId FROM ONLY TestSchema.Koo WHERE D1 IS NULL"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ECInstanceId FROM ONLY TestSchema.Foo WHERE D2 IS NULL"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ECInstanceId FROM ONLY TestSchema.Goo WHERE D2 IS NULL AND D3 IS NULL"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT ECInstanceId FROM ONLY TestSchema.Boo WHERE D2 IS NULL AND D3 IS NULL AND D4 IS NULL"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert new row with new value",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Koo (L1, S1, D1) VALUES (1, 't1', 'd1')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Foo (L1, S1, D1, L2, D2) VALUES (2, 't2', 'd2',3, 'd3')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Goo (L1, S1, D1, L2, D2, L3, D3) VALUES (4, 't3', 'd4', 5, 'd5',6 ,'d6')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO TestSchema.Boo (L1, S1, D1, L2, D2, L3, D3, L4, D4) VALUES (5, 't4', 'd7', 6, 'd8',7 ,'d9', 8,'d10')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddDeleteVirtualColumns)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "6019f52ffe28049c087c423d8b084443f68e24bb9a49b0aab382c9e0acb4e729";
    const auto SCHEMA1_HASH_ECDB_MAP = "13b955ae789dfff78ff1960ab1743a130b8fe229c55bb1c26c264ab30e3c20f6";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' alias='ecdbmap' />
                    <ECEntityClass typeName='Foo' modifier='Abstract' >
                        <ECProperty propertyName='L1' typeName='long' />
                        <ECProperty propertyName='S1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Delete and Add some properties",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "419b1e5280227d1e96e51c7c41f4cc3cf8dd20f8745d880fdb457285788d9bff";
            const auto SCHEMA_HASH_ECDB_MAP = "f13b5a76d30187c753890b60fe1752cc74ce8312e92ae883b127d0445700a9e7";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' alias='ecdbmap' />
                    <ECEntityClass typeName='Foo' modifier='Abstract' >
                        <ECProperty propertyName='L1' typeName='long' />
                        <ECProperty propertyName='D1' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->AbandonChanges());
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, DeleteOverriddenProperties)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "977ed7e8eb12c60df52729c509e26f6f4ff2a390e17f373a400c06b06e1ffbd0";
    const auto SCHEMA1_HASH_ECDB_MAP = "6fa37d7b2f78e0ce4a987535382ef9fda79dc792f7e5a1254066394328ae80e1";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "c11693c59fc4f4b172596da51727a1461cb6731a4c7586841dfeb296becedd03";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Foo' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='L1' typeName='long' />
                        <ECProperty propertyName='S1' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Goo' modifier='None' >
                        <BaseClass>Foo</BaseClass>
                        <ECProperty propertyName='L2' typeName='long' />
                        <ECProperty propertyName='S2' typeName='string' />
                        <ECProperty propertyName='L1' typeName='long' />
                        <ECProperty propertyName='S1' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Change Abstract to Sealed is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Foo' modifier='None' >
                        <ECProperty propertyName='L1' typeName='long' />
                        <ECProperty propertyName='S1' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Goo' modifier='None' >
                        <BaseClass>Foo</BaseClass>
                        <ECProperty propertyName='L2' typeName='long' />
                        <ECProperty propertyName='S2' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            auto newBriefcase = m_hub->CreateBriefcase();

            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->AbandonChanges());
            CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateCAProperties)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "b108c05f4bf633af9128112a359abd8bda883e1ba359ded02a8c1771bdf926f9";
    const auto SCHEMA1_HASH_ECDB_MAP = "9e81b5d344d5172ca9148b06caef2c1275ae36dc69cceabeeedd697a696bdb4e";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "77af11ada178c9b759bec7717d606ddd7d1ccfae9168df93c3918729958ef815";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECCustomAttributeClass typeName = 'TestCA' appliesTo = 'PrimitiveProperty'>
                        <ECProperty propertyName = 'ColumnName' typeName = 'string' description = 'If not specified, the ECProperty name is used. It must follow EC Identifier specification.' />
                        <ECProperty propertyName = 'IsNullable' typeName = 'boolean' description = 'If false, values must not be unset for this property.' />
                    </ECCustomAttributeClass>
                    <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' >
                            <ECCustomAttributes>
                                <TestCA xmlns='TestSchema.01.00'>
                                    <IsNullable>false</IsNullable>
                                </TestCA>
                            </ECCustomAttributes>
                        </ECProperty>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "5cdae8a05050281bcdf2fdd907388ed00b50db2a7904d68f48b00a8623928cb4";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECCustomAttributeClass typeName = 'TestCA' appliesTo = 'PrimitiveProperty'>
                        <ECProperty propertyName = 'ColumnName' typeName = 'string' description = 'If not specified, the ECProperty name is used. It must follow EC Identifier specification.' />
                        <ECProperty propertyName = 'IsNullable' typeName = 'boolean' description = 'If false, values must not be unset for this property.' />
                        <ECProperty propertyName = 'IsUnique' typeName = 'boolean' description = 'Only allow unique values for this property.' />
                        <ECProperty propertyName = 'Collation' typeName = 'string' description = 'Specifies how string comparisons should work for this property. Possible values: Binary (default): bit to bit matching. NoCase: The same as binary, except that the 26 upper case characters of ASCII are folded to their lower case equivalents before comparing. Note that it only folds ASCII characters. RTrim: The same as binary, except that trailing space characters are ignored.' />
                    </ECCustomAttributeClass>
                    <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >
                            <ECCustomAttributes>
                                <TestCA xmlns='TestSchema.01.00'>
                                    <IsNullable>false</IsNullable>
                                    <IsUnique>true</IsUnique>
                                </TestCA>
                            </ECCustomAttributes>
                        </ECProperty>
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify Schema, Class, property and CAClassProperties attributes upgraded successfully",
        [&]()
            {
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *m_briefcase,
                    [&]()
                        {
                        CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*m_briefcase));
                        ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
                        }
                )
            );

            auto testSchema = m_briefcase->Schemas().GetSchema("TestSchema");
            ASSERT_TRUE(testSchema != nullptr);
            ASSERT_TRUE(testSchema->GetAlias() == "ts");
            ASSERT_TRUE(testSchema->GetDisplayLabel() == "Modified Test Schema");
            ASSERT_TRUE(testSchema->GetDescription() == "modified test schema");

            auto testClass = testSchema->GetClassCP("TestClass");
            ASSERT_TRUE(testClass != nullptr);
            ASSERT_TRUE(testClass->GetDisplayLabel() == "Modified Test Class");
            ASSERT_TRUE(testClass->GetDescription() == "modified test class");

            auto testProperty = testClass->GetPropertyP("TestProperty");
            ASSERT_TRUE(testProperty != nullptr);
            ASSERT_TRUE(testProperty->GetDisplayLabel() == "Modified Test Property");
            ASSERT_TRUE(testProperty->GetDescription() == "this is modified property");

            //Verify attributes via ECSql using MataSchema
            ECSqlStatement statement;
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
            ASSERT_STREQ("modified test schema", statement.GetValueText(1));
            ASSERT_STREQ("ts", statement.GetValueText(2));
            statement.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
            ASSERT_STREQ("modified test class", statement.GetValueText(1));
            statement.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='TestProperty'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Modified Test Property", statement.GetValueText(0));
            ASSERT_STREQ("this is modified property", statement.GetValueText(1));
            statement.Finalize();

            //Verify class and Property accessible
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT TestProperty FROM ts.TestClass"));
            ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
            statement.Finalize();

            //verify CA changes
            testProperty = m_briefcase->Schemas().GetSchema("TestSchema")->GetClassCP("TestClass")->GetPropertyP("TestProperty");
            ASSERT_TRUE(testProperty != nullptr);
            auto propertyMapCA = testProperty->GetCustomAttribute("TestCA");
            ASSERT_TRUE(propertyMapCA != nullptr);
            ECValue val;
            ASSERT_EQ(ECObjectsStatus::Success, propertyMapCA->GetValue(val, "IsNullable"));
            ASSERT_FALSE(val.GetBoolean());

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// has empty schema that could be used elsewhere!!
TEST_F(SchemaSyncTestFixture, AddNewEntityClass)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "48f524bc0ff9862b036f277eddb9e089d2cd150795e16b39c83905b63a2468f5";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "bf8155d55306c3b2ea6e5791d30c4f70b57376f318d33d782478e531f6e59f26";
    const auto SCHEMA2_HASH_ECDB_MAP = "32608ea390282589dafbb36721fb70a9336a359434d056ab67dd245415c0f814";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "533fbcc8fc6a9a9efdb25aff7db7bdb91856955e6b8c236b3f5218b64361d533";
    Test(
        "Upgrade with some attributes and import schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' />
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "verify tables",
        [&]()
            {
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *m_briefcase,
                    [&]()
                        {
                        CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*m_briefcase));
                        ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
                        }
                )
            );

            //new class should be added with new namespace prefix
            ASSERT_TRUE(m_briefcase->TableExists("ts_TestClass"));

            //Verify Schema attributes upgraded successfully
            auto testSchema = m_briefcase->Schemas().GetSchema("TestSchema");
            ASSERT_TRUE(testSchema != nullptr);

            //Verify Newly Added Entity Class exists
            auto entityClass = testSchema->GetClassCP("TestClass");
            ASSERT_TRUE(entityClass != nullptr);
            ASSERT_TRUE(entityClass->GetDisplayLabel() == "Test Class");
            ASSERT_TRUE(entityClass->GetDescription() == "This is test Class");

            //Verify attributes via ECSql using MataSchema
            ECSqlStatement statement;
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Test Class", statement.GetValueText(0));
            ASSERT_STREQ("This is test Class", statement.GetValueText(1));
            statement.Finalize();

            //Query newly added Entity Class
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT * FROM ts.TestClass"));
            ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
            statement.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddNewSubClassForBaseWithTPH)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "cf03646c72c2cffef0dcc00aacefb3dd2f048fd02ea3197bb48414b08babeb3a";
    const auto SCHEMA1_HASH_ECDB_MAP = "0545286d311abcbdeb26e11f658b2add89b5e12b3be78806aeb9b0a1544979d8";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "97780c55b8972e6b86c8bd514db8948c6369dca78b94d4f14da12eb4d6dd8e59";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Parent' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P1' typeName='int' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "1ac4d33ea5b7b2d6a3108a19ccdf6f9fe746fd3c1b88f62bf1e3b711cf31c674";
    const auto SCHEMA2_HASH_ECDB_MAP = "d6b71706869c7a07d9fe63ffda73e42a35bab2426152021eea91277ee3fb3d40";
    Test(
        "Adding new Class to TPH is expected to succeed",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Parent' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P1' typeName='int' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub1' modifier='None' >
                        <BaseClass>Parent</BaseClass>
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA3_HASH_ECDB_SCHEMA = "d5a273fd85339166ac564843a06fa7024e62c6018f1cbf4442fd7e7490b16302";
    const auto SCHEMA3_HASH_ECDB_MAP = "89637627a86a2af369a6e85be5bb6f828c041295391b5aac58038e03ede27632";
    const auto SCHEMA3_HASH_SQLITE_SCHEMA = "b1cf0f4d87b6f8702dd08d08b90931141e441dc145a5e827c66110368acfe764";
    Test(
        "Adding new column to TPH is expected to succeed until strict mode is enforced",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Parent' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P1' typeName='int' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub1' modifier='None' >
                        <BaseClass>Parent</BaseClass>
                        <ECProperty propertyName='Sub1' typeName='int' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddNewClass_NewProperty_TPH_ShareColumns)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "fd6a18f3211e538152fa723175bc2814b4994b21b892b4c9e9f83764009e0db4";
    const auto SCHEMA1_HASH_ECDB_MAP = "1e2bf61e978994641f553af1df51767c7d3fb625c758fc5e41506d4c32a6bc3b";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "9afc1fa2bb0dc6be01dea2ad48253e05cc127bbbd5134a205ee35602735f2ad7";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Parent' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P1' typeName='int' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub1' modifier='None' >
                        <BaseClass>Parent</BaseClass>
                        <ECProperty propertyName='Sub1' typeName='int' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "c28e933a4bce30fbed9050b85aa6f616f2980e593f11fb89371d83a237d30b8d";
    const auto SCHEMA2_HASH_ECDB_MAP = "bf815e4ff9173a1640619d709c70014db6e235c6b5683776e73049474ee42743";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "48044d7a0f99acacd68e771434d1c52243f129131fcb6ea0f8d6d3442e3bab08";
    Test(
        "Adding new Class with new property to TPH+ShareColumns is expected to fail",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Parent' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P1' typeName='int' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub1' modifier='None' >
                        <BaseClass>Parent</BaseClass>
                        <ECProperty propertyName='Sub1' typeName='int' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub2' modifier='None' >
                        <BaseClass>Sub1</BaseClass>
                        <ECProperty propertyName='Sub2' typeName='int' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// suspicious SCHEMA_HASH_SQLITE_SCHEMA values where they shouldn't be, why is this happening??
TEST_F(SchemaSyncTestFixture, VerifyMappingOfPropertiesToOverflowOnJoinedTable)
    {
    auto assertSelectSql = [](ECDbCR ecdb, Utf8CP sql, int expectedColumnCount, int expectedRowCount, Utf8CP expectedColumnName)
        {
        Statement stmt;

        //Verify Column count
        ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(ecdb, sql)) << " ECSQL: " << sql;
        ASSERT_EQ(expectedColumnCount, stmt.GetColumnCount()) << " ECSQL: " << sql;

        //Verify Row count
        int actualRowCount = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
            actualRowCount++;

        ASSERT_EQ(expectedRowCount, actualRowCount) << " ECSQL: " << sql;

        //Verify that the columns generated are same as expected
        Utf8String actualColumnNames;
        for (int i = 0; i < stmt.GetColumnCount(); i++)
            actualColumnNames.append(stmt.GetColumnName(i));

        ASSERT_STREQ(expectedColumnName, actualColumnNames.c_str());
        stmt.Finalize();
        };

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "7dcdc4d4a7575c3ac09e0bef4b92b8a09eba6b6a520d1c4f8d71c306137cf95f";
    const auto SCHEMA1_HASH_ECDB_MAP = "f8bfd29165366bd4c1c8effe108400b02a223e89c359d380fa3d899143b57e90";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "73249e5c8fcbfb020a011b07a94b3a477390f12ecb63f2b6e491b7f8197366ae";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />
                    <ECEntityClass typeName='C1'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>
                        </ECCustomAttributes>
                        <ECProperty propertyName='A' typeName='int'/>
                        <ECProperty propertyName='B' typeName='string'/>
                    </ECEntityClass>
                    <ECEntityClass typeName='C2'>
                        <BaseClass>C1</BaseClass>
                        <ECProperty propertyName='C' typeName='int'/>
                        <ECProperty propertyName='D' typeName='int'/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Inserting Instances for C1 and C2",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.C2 (A,B,C,D) VALUES (1,'val1',2,33)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

            assertSelectSql(*m_briefcase, "SELECT * FROM ts_C1", 4, 1, "IdECClassIdAB");
            assertSelectSql(*m_briefcase, "SELECT * FROM ts_C2", 3, 1, "C1IdECClassIdjs1");
            assertSelectSql(*m_briefcase, "SELECT * FROM ts_C2_Overflow", 3, 1, "C1IdECClassIdos1");

            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verifying the inserted values for classes C1 and C2",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT A,B,C,D FROM ts.C2"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            ASSERT_EQ(1, stmt.GetValueInt(0));
            ASSERT_STREQ("val1", stmt.GetValueText(1));
            ASSERT_EQ(2, stmt.GetValueInt(2));
            ASSERT_EQ(33, stmt.GetValueInt(3));
            stmt.Finalize();
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "3d1099acdee3ee310287bbd41f7da83f2a76379b03b5ff5816fd2a704733c7d6";
    const auto SCHEMA2_HASH_ECDB_MAP = "0fa95b092bd284fbf574afacc413263e566d879dd4fbbd6ec126e5949d44a379";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "c81d0d817930d4a5015881ac11a63b3c15595a5ea74490a85ff61a0ba25ef5c6";
    Test(
        "Adding New Entity Class",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />
                    <ECEntityClass typeName='C1'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>
                        </ECCustomAttributes>
                        <ECProperty propertyName='A' typeName='int'/>
                        <ECProperty propertyName='B' typeName='string'/>
                    </ECEntityClass>
                    <ECEntityClass typeName='C2'>
                        <BaseClass>C1</BaseClass>
                        <ECProperty propertyName='C' typeName='int'/>
                        <ECProperty propertyName='D' typeName='int'/>
                    </ECEntityClass>
                    <ECEntityClass typeName='C3'>
                        <BaseClass>C1</BaseClass>
                        <ECProperty propertyName='E' typeName='double'/>
                        <ECProperty propertyName='F' typeName='int'/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        const auto SCHEMA_HASH_SQLITE_SCHEMA = "26b52f88896537eff99863fbe5c79c28d98468768b50f74badec5ee4a4693870"; // wtf is this shit???
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify that the class is added successfully",
        [&]()
            {
            m_schemaChannel->WithReadOnly(
                [&](ECDbR syncDb)
                    {
                    auto testSchema = syncDb.Schemas().GetSchema("TestSchema");
                    ASSERT_NE(testSchema, nullptr);
                    ASSERT_NE(testSchema->GetClassCP("C3"), nullptr);
                    }
            );
            }
    );

    const auto SCHEMA3_HASH_ECDB_SCHEMA = "5e184f0eb5eac83e37864babf3265119721fdfec5a07d6a9388ab592a0f06808";
    const auto SCHEMA3_HASH_ECDB_MAP = "f1ad5502c3d7f0b2fe51bdfbdd4616301fa108ed054cc05daf851cc9bc2306aa";
    const auto SCHEMA3_HASH_SQLITE_SCHEMA = "1f1cbc4d26ed14f4946aa009b6d03fc391a79e76b25935e62642af4bd3be1e9e";
    Test(
        "Adding Entity Classes C31 and C32",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />
                    <ECEntityClass typeName='C1'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>
                        </ECCustomAttributes>
                        <ECProperty propertyName='A' typeName='int'/>
                        <ECProperty propertyName='B' typeName='string'/>
                    </ECEntityClass>
                    <ECEntityClass typeName='C2'>
                        <BaseClass>C1</BaseClass>
                        <ECProperty propertyName='C' typeName='int'/>
                        <ECProperty propertyName='D' typeName='int'/>
                    </ECEntityClass>
                    <ECEntityClass typeName='C3'>
                        <BaseClass>C1</BaseClass>
                        <ECProperty propertyName='E' typeName='double'/>
                        <ECProperty propertyName='F' typeName='int'/>
                    </ECEntityClass>
                    <ECEntityClass typeName='C31'>
                        <BaseClass>C3</BaseClass>
                        <ECProperty propertyName='G' typeName='double'/>
                        <ECProperty propertyName='H' typeName='int'/>
                    </ECEntityClass>
                    <ECEntityClass typeName='C32'>
                        <BaseClass>C3</BaseClass>
                        <ECProperty propertyName='I' typeName='string'/>
                        <ECProperty propertyName='J' typeName='double'/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        const auto SCHEMA_HASH_SQLITE_SCHEMA = "9d6e9996e2c1ff5f3ec20eecfbd9c4ccafcfdce8a58e3fb831542ef483a94118"; // wtf is this shit???
                        CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP); });
            }
    );

    Test(
        "verify the the values",
        [&]()
            {
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "bde533a4f80841946dc2205235cac929ece8455b2682da69a7794aa3a1ef71ff"; // whats happening here??????
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *m_briefcase,
                    [&]()
                        {
                        CheckHashes(*m_briefcase, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*m_briefcase));
                        ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
                        }
                )
            );

            //Verify that the classes are added successfully
            auto testSchema = m_briefcase->Schemas().GetSchema("TestSchema");
            ASSERT_NE(testSchema, nullptr);
            ASSERT_NE(testSchema->GetClassCP("C31"), nullptr);
            ASSERT_NE(testSchema->GetClassCP("C32"), nullptr);

            //Tables for C1,C2,C3 should exist.
            ASSERT_TRUE(m_briefcase->TableExists("ts_C1"));
            ASSERT_TRUE(m_briefcase->TableExists("ts_C2"));
            ASSERT_TRUE(m_briefcase->TableExists("ts_C3"));
            ASSERT_TRUE(m_briefcase->TableExists("ts_C3_Overflow"));

            //C31 and C32 should not exist.
            ASSERT_FALSE(m_briefcase->TableExists("ts_C31"));
            ASSERT_FALSE(m_briefcase->TableExists("ts_C32"));

            //Inserting Instances in Classes C31 and C32
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.C31 (E,F,G,H) VALUES (10.32,3,11.1,50)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.C32 (E,F,I,J) VALUES (23.45,6,'val4',44.60)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

            //Verifying values
            assertSelectSql(*m_briefcase, "SELECT * FROM ts_C3", 3, 2, "C1IdECClassIdjs1");
            assertSelectSql(*m_briefcase, "SELECT * FROM ts_C3_Overflow", 5, 2, "C1IdECClassIdos1os2os3");

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT G,H FROM ts.C31"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            ASSERT_EQ(11.1, stmt.GetValueDouble(0));
            ASSERT_EQ(50, stmt.GetValueInt(1));
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT I,J FROM ts.C32"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            ASSERT_STREQ("val4", stmt.GetValueText(0));
            ASSERT_EQ(44.60, stmt.GetValueDouble(1));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddNewClassModifyAllExistingAttributes)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "482c8e3b064ef4866027ed52003f319d83d4374385fb9c82430a8360af5b5c59";
    const auto SCHEMA1_HASH_ECDB_MAP = "82a4f93a397725447cf558de379c6f5a4c044c2de789edf25e9f889fbaf52a43";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "77af11ada178c9b759bec7717d606ddd7d1ccfae9168df93c3918729958ef815";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' >
                        </ECProperty>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "cb1d2a420b7b40df0e48d5a2f70c009e7cdd82b1a64ed8e8f7ad1a6ce17a5545";
    const auto SCHEMA2_HASH_ECDB_MAP = "89c8df19fa639dbc42652c0a1e7ff24d6b494f89e91f89d25abca6326c049dc9";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "175954922d32d3addfe52c3c8197b4848879bcd07e9c48ff120bd59f22ed6157";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >
                        </ECProperty>
                    </ECEntityClass>
                    <ECEntityClass typeName='NewTestClass' displayLabel='New Test Class' description='This is New test Class' modifier='None' />
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify Schema, Class, property and CAClassProperties attributes upgraded successfully",
        [&]()
            {
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *m_briefcase,
                    [&]()
                        {
                        CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*m_briefcase));
                        ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
                        }
                )
            );
            //Verify Schema, Class, property and CAClassProperties attributes upgraded successfully
            auto testSchema = m_briefcase->Schemas().GetSchema("TestSchema");
            ASSERT_NE(testSchema, nullptr);
            ASSERT_EQ(testSchema->GetAlias(),"ts");
            ASSERT_EQ(testSchema->GetDisplayLabel(), "Modified Test Schema");
            ASSERT_EQ(testSchema->GetDescription(), "modified test schema");

            auto testClass = testSchema->GetClassCP("TestClass");
            ASSERT_NE(testClass, nullptr);
            ASSERT_EQ(testClass->GetDisplayLabel(), "Modified Test Class");
            ASSERT_EQ(testClass->GetDescription(), "modified test class");

            auto testProperty = testClass->GetPropertyP("TestProperty");
            ASSERT_NE(testProperty, nullptr);
            ASSERT_EQ(testProperty->GetDisplayLabel(), "Modified Test Property");
            ASSERT_EQ(testProperty->GetDescription(), "this is modified property");

            //verify newly added Entity Class exists
            auto newTestClass = testSchema->GetClassCP("NewTestClass");
            ASSERT_NE(newTestClass, nullptr);
            ASSERT_EQ(newTestClass->GetDisplayLabel(), "New Test Class");
            ASSERT_EQ(newTestClass->GetDescription(), "This is New test Class");

            //Verify attributes via ECSql using MataSchema
            ECSqlStatement statement;
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
            ASSERT_STREQ("modified test schema", statement.GetValueText(1));
            ASSERT_STREQ("ts", statement.GetValueText(2));
            statement.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
            ASSERT_STREQ("modified test class", statement.GetValueText(1));
            statement.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description From meta.ECClassDef WHERE Name='NewTestClass'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("New Test Class", statement.GetValueText(0));
            ASSERT_STREQ("This is New test Class", statement.GetValueText(1));
            statement.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='TestProperty'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Modified Test Property", statement.GetValueText(0));
            ASSERT_STREQ("this is modified property", statement.GetValueText(1));
            statement.Finalize();

            //Query existing and newly added Entity Classes
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT TestProperty FROM ts.TestClass"));
            ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
            statement.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT * FROM ts.NewTestClass"));
            ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
            statement.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddNewECDbMapCANotSupported)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "8a8d97dd38805188cf4dcafecb1a48e9045df0a4f941f4c8bbdfae18c8f04011";
    const auto SCHEMA1_HASH_ECDB_MAP = "d66e7330b944910eb2123301d081f2556e51dcd9c59c739082fe55b9c63ddd45";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "cada7f8e10fef1a4b8bfd574c110b6ecb84653e8e4a769a50ec281b56a4b3c00";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Goo' modifier='None'>
                        <ECProperty propertyName='S' typeName='string' />
                        <ECProperty propertyName='D' typeName='double' />
                        <ECProperty propertyName='L' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Add New ECDbMapCA on ECClass",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='4.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Goo' modifier='None'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName='S' typeName='string' />
                        <ECProperty propertyName='D' typeName='double' />
                        <ECProperty propertyName='L' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->AbandonChanges());
            CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AppendNewCA)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "fc9ad3bb46442c4af71fc02112e46cfbcf96e4e65d06f3d911c2f1081075a73c";
    const auto SCHEMA1_HASH_ECDB_MAP = "cf1ab83f94b23c6488da13b61ebb1d463504a4304a9f05c90813ac8c4a8f8f1b";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "533fbcc8fc6a9a9efdb25aff7db7bdb91856955e6b8c236b3f5218b64361d533";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECCustomAttributeClass typeName = 'UserCA1' appliesTo = 'Any' />
                    <ECCustomAttributeClass typeName = 'UserCA2' appliesTo = 'Any' />
                    <ECCustomAttributeClass typeName = 'UserCA3' appliesTo = 'Any' />
                    <ECCustomAttributeClass typeName = 'UserCA4' appliesTo = 'Any' />
                    <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='test class' modifier='None' >
                         <ECCustomAttributes>
                            <UserCA1 xmlns = 'TestSchema.01.00.00' />
                            <UserCA2 xmlns = 'TestSchema.01.00.00' />
                            <UserCA3 xmlns = 'TestSchema.01.00.00' />
                         </ECCustomAttributes>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "a8cc0df53bb1339d49f8c2609edeeb4ffcb0cf77eae648966ec01a1284ffd7d4";
    const auto SCHEMA2_HASH_ECDB_MAP = "80d2aae54ffc27813eb1bd1d660aa80e02c1bbdb8522af0c27e64040393bfc72";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "f5d18dd4acb5c7c8a7980d29bc65fdcb530ad0df51dfcfe94d001ce821b0b57e";
    Test(
        "Add new CustomAttribute",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name = 'CoreCustomAttributes' version = '01.00' alias = 'CoreCA' />
                    <ECCustomAttributeClass typeName = 'UserCA1' appliesTo = 'Any' />
                    <ECCustomAttributeClass typeName = 'UserCA2' appliesTo = 'Any' />
                    <ECCustomAttributeClass typeName = 'UserCA3' appliesTo = 'Any' />
                    <ECCustomAttributeClass typeName = 'UserCA4' appliesTo = 'Any' />
                    <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >
                         <ECCustomAttributes>
                            <UserCA1 xmlns = 'TestSchema.01.00.00' />
                            <UserCA2 xmlns = 'TestSchema.01.00.00' />
                            <UserCA3 xmlns = 'TestSchema.01.00.00' />
                            <UserCA4 xmlns = 'TestSchema.01.00.00' />
                            <ClassHasCurrentTimeStampProperty xmlns='CoreCustomAttributes.01.00'>
                                <PropertyName>LastMod</PropertyName>
                            </ClassHasCurrentTimeStampProperty>
                         </ECCustomAttributes>
                        <ECProperty propertyName='LastMod' typeName='dateTime' readOnly='True'/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify Schema, Class, property attributes upgraded successfully",
        [&]()
            {
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *m_briefcase,
                    [&]()
                        {
                        CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*m_briefcase));
                        ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
                        }
                )
            );

            auto testSchema = m_briefcase->Schemas().GetSchema("TestSchema");
            ASSERT_NE(testSchema, nullptr);
            ASSERT_EQ(testSchema->GetAlias(),"ts");
            ASSERT_EQ(testSchema->GetDisplayLabel(), "Modified Test Schema");
            ASSERT_EQ(testSchema->GetDescription(), "modified test schema");

            auto testClass = testSchema->GetClassCP("TestClass");
            ASSERT_NE(testClass, nullptr);
            ASSERT_EQ(testClass->GetDisplayLabel(), "Modified Test Class");
            ASSERT_EQ(testClass->GetDescription(), "modified test class");

            //verify tables
            ASSERT_TRUE(m_briefcase->TableExists("ts_TestClass"));

            //Verify attributes via ECSql using MataSchema
            ECSqlStatement statement;
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
            ASSERT_STREQ("modified test schema", statement.GetValueText(1));
            ASSERT_STREQ("ts", statement.GetValueText(2));
            statement.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
            ASSERT_STREQ("modified test class", statement.GetValueText(1));
            statement.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT * FROM ts.TestClass"));
            ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
            statement.Finalize();

            //Verify newly added CA
            testClass = m_briefcase->Schemas().GetSchema("TestSchema")->GetClassCP("TestClass");
            ASSERT_NE(testClass, nullptr);
            auto bsca = testClass->GetCustomAttribute("CoreCustomAttributes", "ClassHasCurrentTimeStampProperty");
            ASSERT_NE(bsca, nullptr);

            ECValue val;
            ASSERT_EQ(ECObjectsStatus::Success, bsca->GetValue(val, "PropertyName"));
            ASSERT_STRCASEEQ("LastMod", val.GetUtf8CP());

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddNewCA)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "f2f99d37b9073c43aba89e57cd57edd79f620892eaf9816806b378407da7bad2";
    const auto SCHEMA1_HASH_ECDB_MAP = "32608ea390282589dafbb36721fb70a9336a359434d056ab67dd245415c0f814";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "533fbcc8fc6a9a9efdb25aff7db7bdb91856955e6b8c236b3f5218b64361d533";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "eb85ce269f0d9cc012db7b6e7df1c18a8398897db796459bfb8ea1ec15f88336";
    const auto SCHEMA2_HASH_ECDB_MAP = "5888e8d32c483df1f7527c9790561eb86e8130cfec63713bffefdb614aae67f4";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "f5d18dd4acb5c7c8a7980d29bc65fdcb530ad0df51dfcfe94d001ce821b0b57e";
    Test(
        "Add new CustomAttribute",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name = 'CoreCustomAttributes' version = '01.00' alias = 'CoreCA' />
                    <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >
                        <ECCustomAttributes>
                            <ClassHasCurrentTimeStampProperty xmlns='CoreCustomAttributes.01.00'>
                                <PropertyName>LastMod</PropertyName>
                            </ClassHasCurrentTimeStampProperty>
                        </ECCustomAttributes>
                        <ECProperty propertyName='LastMod' typeName='dateTime' readOnly='True'/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );

            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify Schema, Class, property attributes upgraded successfully",
        [&]()
            {
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *m_briefcase,
                    [&]()
                        {
                        CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*m_briefcase));
                        ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
                        }
                )
            );

            auto testSchema = m_briefcase->Schemas().GetSchema("TestSchema");
            ASSERT_NE(testSchema, nullptr);
            ASSERT_EQ(testSchema->GetAlias(), "ts");
            ASSERT_EQ(testSchema->GetDisplayLabel(), "Modified Test Schema");
            ASSERT_EQ(testSchema->GetDescription(), "modified test schema");

            auto testClass = testSchema->GetClassCP("TestClass");
            ASSERT_NE(testClass, nullptr);
            ASSERT_EQ(testClass->GetDisplayLabel(), "Modified Test Class");
            ASSERT_EQ(testClass->GetDescription(), "modified test class");

            //verify tables
            ASSERT_TRUE(m_briefcase->TableExists("ts_TestClass"));

            //Verify attributes via ECSql using MataSchema
            ECSqlStatement statement;
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
            ASSERT_STREQ("modified test schema", statement.GetValueText(1));
            ASSERT_STREQ("ts", statement.GetValueText(2));
            statement.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
            ASSERT_STREQ("modified test class", statement.GetValueText(1));
            statement.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT * FROM ts.TestClass"));
            ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
            statement.Finalize();

            //Verify newly added CA
            testClass = m_briefcase->Schemas().GetSchema("TestSchema")->GetClassCP("TestClass");
            ASSERT_NE(testClass, nullptr);
            auto bsca = testClass->GetCustomAttribute("CoreCustomAttributes", "ClassHasCurrentTimeStampProperty");
            ASSERT_NE(bsca, nullptr);

            ECValue val;
            ASSERT_EQ(ECObjectsStatus::Success, bsca->GetValue(val, "PropertyName"));
            ASSERT_STRCASEEQ("LastMod", val.GetUtf8CP());

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddNewECProperty)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "f658d8871c57de304786ad7a38b5340463fd7fa8e008edff9c63962bf5c55a77";
    const auto SCHEMA1_HASH_ECDB_MAP = "32608ea390282589dafbb36721fb70a9336a359434d056ab67dd245415c0f814";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "533fbcc8fc6a9a9efdb25aff7db7bdb91856955e6b8c236b3f5218b64361d533";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='TestClass' modifier='None' >
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "ee0946063314af2790a376bc6104debedc11fcb0eea2648b39bac805542e9a1c";
    const auto SCHEMA2_HASH_ECDB_MAP = "82a4f93a397725447cf558de379c6f5a4c044c2de789edf25e9f889fbaf52a43";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "77af11ada178c9b759bec7717d606ddd7d1ccfae9168df93c3918729958ef815";
    Test(
        "Upgrade with some attributes and import schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='TestClass' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify newly added property exists",
        [&]()
            {
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *m_briefcase,
                    [&]()
                        {
                        CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*m_briefcase));
                        ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
                        }
                )
            );

            auto testSchema = m_briefcase->Schemas().GetSchema("TestSchema");
            ASSERT_NE(testSchema, nullptr);

            auto testClass = testSchema->GetClassCP("TestClass");
            ASSERT_NE(testClass, nullptr);

            auto testProperty = testClass->GetPropertyP("TestProperty");
            ASSERT_NE(testProperty, nullptr);
            ASSERT_EQ(testProperty->GetDisplayLabel(), "Test Property");
            ASSERT_EQ(testProperty->GetDescription(), "this is property");

            ECSqlStatement statement;
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='TestProperty'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Test Property", statement.GetValueText(0));
            ASSERT_STREQ("this is property", statement.GetValueText(1));
            statement.Finalize();

            //Query newly added Property
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT TestProperty FROM ts.TestClass"));
            ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
            statement.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, DeleteOverridePropertyOutOfOrderAndThenAddAnewPropertyCauseUniqueIndexToFail)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "e7289951c692a26c6a1955116cac7a7ce30486f371d3701ebcd16b71242736b3";
    const auto SCHEMA1_HASH_ECDB_MAP = "73eab5d68e91d13d229c85142f08bc4d1b87a995f05ec40ee70c9db3876e8358";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "b03c0e8b59618d07b7a2c8f04bfa0b488ceae1cf4ef7e5eabc09c11f1e7e42f9";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />
                    <ECEntityClass typeName='Base' modifier='Abstract' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00' />
                        </ECCustomAttributes>
                        <ECProperty propertyName='Prop1' typeName='string' />
                        <ECProperty propertyName='Prop2' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub' modifier='None' >
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName='Prop1' typeName='string' />
                        <ECProperty propertyName='Prop2' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("AddPropertyToBaseClass", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "41c0ce6c68454dc2c3f03c97f8d464608e4904b6a60c7aa468eb3272960906b8";
    Test(
        "Delete override property",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />
                    <ECEntityClass typeName='Base' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00' />
                        </ECCustomAttributes>
                       <ECProperty propertyName='Prop1' typeName='string' />
                       <ECProperty propertyName='Prop2' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub' modifier='None' >
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName='Prop2' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA3_HASH_ECDB_SCHEMA = "697f83b13ea95e1de816a553bb849ac8ff619b96389a97aa709f57e9f0691441";
    const auto SCHEMA3_HASH_ECDB_MAP = "7446bd9b37672cc32bc2e47bf321e86535036348978bf9561c482372893f7795";
    Test(
        "add Override Property back",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />
                    <ECEntityClass typeName='Base' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00' />
                        </ECCustomAttributes>
                       <ECProperty propertyName='Prop1' typeName='string' />
                       <ECProperty propertyName='Prop2' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub' modifier='None' >
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName='Prop1' typeName='string' />
                        <ECProperty propertyName='Prop2' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            //This cause unique index error in ec_Property(id, classId, ordinal) before fix
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddPropertyToBaseClass)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "a0ca69cadd1cb8eafe98293187746699ee27b52f4379cedae32f0933765a46c3";
    const auto SCHEMA1_HASH_ECDB_MAP = "edfbeec1cf01d1d8da8a43a740de68aad40cc26ef23f304cb0e286ee157548e5";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "b03c0e8b59618d07b7a2c8f04bfa0b488ceae1cf4ef7e5eabc09c11f1e7e42f9";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />
                    <ECEntityClass typeName='Base' modifier='Abstract' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00' />
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub' modifier='None' >
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName='Prop1' typeName='string' />
                        <ECProperty propertyName='Prop2' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("AddPropertyToBaseClass", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    ECSqlStatement stmt;
    ECInstanceKey row1;
    Test(
        "Insert values to initial schema",
        [&]()
            {
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub(Prop1,Prop2) VALUES ('Instance1 Prop1', 'Instance1 Prop2')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(row1));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "78c1bae7becca6bf4d60adbb62e75b2e778d8a8cd3d7d79471be018d1ab4a061";
    const auto SCHEMA2_HASH_ECDB_MAP = "b0cc01fa6a44ec8e1d12c58c2b9462a1283d4877b4056186082b0ac79ff14b5c";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "351e3a23da01181b90915602d96a7346fab461425708e7ddc36797c2e4340402";
    Test(
        "Upgrade with some attributes and import schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />
                    <ECEntityClass typeName='Base' modifier='Abstract' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00' />
                        </ECCustomAttributes>
                        <ECProperty propertyName='BaseProp1' typeName='string' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub' modifier='None' >
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName='Prop1' typeName='string' />
                        <ECProperty propertyName='Prop2' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    ECInstanceKey row2;
    Test(
        "Insert values to upgraded schema",
        [&]()
            {
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub(BaseProp1,Prop1,Prop2) VALUES ('Instance2 BaseProp1', 'Instance2 Prop1', 'Instance2 Prop2')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(row2));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            }
    );

    Test(
        "Verify select",
        [&]()
            {
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT BaseProp1, Prop1, Prop2 FROM ts.Sub WHERE ECInstanceId=?"));
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, row1.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            ASSERT_TRUE(stmt.IsValueNull(0));
            ASSERT_STREQ("Instance1 Prop1", stmt.GetValueText(1));
            ASSERT_STREQ("Instance1 Prop2", stmt.GetValueText(2));
            stmt.ClearBindings();
            stmt.Reset();
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, row2.GetInstanceId()));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            ASSERT_STREQ("Instance2 BaseProp1", stmt.GetValueText(0));
            ASSERT_STREQ("Instance2 Prop1", stmt.GetValueText(1));
            ASSERT_STREQ("Instance2 Prop2", stmt.GetValueText(2));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            }
    );

    Test(
        "verify that all three props map to different columns",
        [&]()
            {
            Statement statement;
            ASSERT_EQ(BE_SQLITE_OK, statement.Prepare(
                *m_briefcase,
                "select count(distinct pm.ColumnId) FROM ec_PropertyPath pp JOIN ec_PropertyMap pm "
                "ON pm.PropertyPathId = pp.Id JOIN ec_Property p ON p.Id = pp.RootPropertyId "
                "WHERE p.Name IN ('BaseProp1', 'Prop1', 'Prop2')"
                )
            );
            ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
            ASSERT_EQ(3, statement.GetValueInt(0)) << "The three properties of ECClass Sub must map to 3 different columns";

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddPropertyToSubclassThenPropertyToBaseClass_TPH_SharedColumns)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "99be3eca8c7db46e07cf934acae443775c940dd22a7d4dc84e743de18b3d6e0c";
    const auto SCHEMA1_HASH_ECDB_MAP = "01a0a473911bb6c60d37cc69a2fc06d41bd59429f8deb60314fd5a166d939d26";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "351e3a23da01181b90915602d96a7346fab461425708e7ddc36797c2e4340402";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Base" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00.00" />
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub1" modifier="None" >
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName="Prop1" typeName="string" />
                        <ECProperty propertyName="Prop2" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub2" modifier="None" >
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName="PropA" typeName="string" />
                        <ECProperty propertyName="PropB" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("AddPropertyToSubclassThenPropertyToBaseClass_TPH_SharedColumns", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert values to initial schema",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub1(Code,Prop1,Prop2) VALUES (10,'1-1', '1-2')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub2(Code,PropA,PropB) VALUES (20,'1-A', 1)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "2490b9dd86d960217c571897b65d1490b2efc93741e514c2f5aa33e6c0a54b30";
    const auto SCHEMA2_HASH_ECDB_MAP = "a5344168ddb4db575c1c18f29ae777ffd66fb147accd4257be35cca76f2f15c3";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "36a8357bd9daa5ac0a3d65079cab9ee91366fa480bc724bfc46ef22b340022df";
    Test(
        "Add property to Sub1",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Base" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00.00" />
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub1" modifier="None" >
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName="Prop1" typeName="string" />
                        <ECProperty propertyName="Prop2" typeName="string" />
                        <ECProperty propertyName="Prop3" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub2" modifier="None" >
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName="PropA" typeName="string" />
                        <ECProperty propertyName="PropB" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert and validate values to upgraded schema",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub1(Code,Prop1,Prop2,Prop3) VALUES (11,'2-1', '2-2', 2)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            auto expected = JsonValue(R"json([{"Code":10,"Prop1":"1-1","Prop2":"1-2"},{"Code":11,"Prop1":"2-1","Prop2":"2-2","Prop3":2}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code, Prop1, Prop2, Prop3 FROM ts.Sub1"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            expected = JsonValue(R"json([{"Code":20,"PropA":"1-A","PropB":1}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code, PropA, PropB FROM ts.Sub2"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            }
    );

    const auto SCHEMA3_HASH_ECDB_SCHEMA = "7f240c84ec90e9f38293e09e975586a915f77c8b8401ee91b180b84750e9d842";
    const auto SCHEMA3_HASH_ECDB_MAP = "d94bdbcace3c39f255192f674a1e0b804cb66fd6be1ebb4ca91ad25ec1908ed7";
    const auto SCHEMA3_HASH_SQLITE_SCHEMA = "8a852b9eb779c1e89a873952c34887f103f7679b9b473cc06c90e12d214818ce";
    Test(
        "Add property to Base",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Base" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00.00" />
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub1" modifier="None" >
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName="Prop1" typeName="string" />
                        <ECProperty propertyName="Prop2" typeName="string" />
                        <ECProperty propertyName="Prop3" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub2" modifier="None" >
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName="PropA" typeName="string" />
                        <ECProperty propertyName="PropB" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

            CheckHashes(*m_briefcase, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert and validate values to final schema",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub1(Code,Name,Prop1,Prop2,Prop3) VALUES (12,'Object 12', '3-1', '3-2', 3)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub2(Code,Name,PropA,PropB) VALUES (21,'Object 21', '2-A', 2)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            auto expected = JsonValue(R"json([{"Code":10,"Prop1":"1-1","Prop2":"1-2"},{"Code":11,"Prop1":"2-1","Prop2":"2-2","Prop3":2},{"Code":12, "Name":"Object 12","Prop1":"3-1","Prop2":"3-2","Prop3":3}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code, Name, Prop1, Prop2, Prop3 FROM ts.Sub1"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            expected = JsonValue(R"json([{"Code":20,"PropA":"1-A","PropB":1},{"Code":21,"Name":"Object 21","PropA":"2-A","PropB":2}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code, Name, PropA, PropB FROM ts.Sub2"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddPropertyToSubclassThenPropertyToBaseClass_TPH_JoinedTable_SharedCols)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "4db8e223774dfd62f4c1659f5a2bd79818cf09c5d248eeeebd27e884ed3c1bf7";
    const auto SCHEMA1_HASH_ECDB_MAP = "7d7cc795d852b7f497f0f3df236c2edbbf26fb6477d44bbf1bcd349f7080704d";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "ebe2cbd764b3e700395e4bccbac629b543fe5e1758da6db5e627ccbfd12f10be";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                        </ECCustomAttributes>
                        <BaseClass>Element</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <BaseClass>GeometricElement</BaseClass>
                        <ECProperty propertyName="Origin" typeName="Point2d" />
                    </ECEntityClass>
                    <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                        <BaseClass>Geometric2dElement</BaseClass>
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubBase" modifier="Abstract" >
                        <BaseClass>PhysicalElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub1" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="Prop1" typeName="string" />
                        <ECProperty propertyName="Prop2" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub2" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="PropA" typeName="string" />
                        <ECProperty propertyName="PropB" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("AddPropertyToSubclassThenPropertyToBaseClass_TPH_JoinedTable_SharedCols", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert values to initial schema",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Prop1,Prop2) VALUES (101,1,1,'Sub1 1','1', '2')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub2(Code,Origin.X,Origin.Y,Name,PropA,PropB) VALUES (201,1,1,'Sub2 1','A', 1)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "7da1f1c635893eab84b4262544def21794c6857d72b9ebcb90c192e6f39f6714";
    const auto SCHEMA2_HASH_ECDB_MAP = "6ab05671deb54d6d30fe6ca0474dedd12034295c9f67ac06e454b8371e80c3b2";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "fa35be3b944aeb606d51e60ff02e9a2b4caad5051882535bce90b0be77b66319";
    Test(
        "Add property to Sub1",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                        </ECCustomAttributes>
                        <BaseClass>Element</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <BaseClass>GeometricElement</BaseClass>
                        <ECProperty propertyName="Origin" typeName="Point2d" />
                    </ECEntityClass>
                    <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                        <BaseClass>Geometric2dElement</BaseClass>
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubBase" modifier="Abstract" >
                        <BaseClass>PhysicalElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub1" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="Prop1" typeName="string" />
                        <ECProperty propertyName="Prop2" typeName="string" />
                        <ECProperty propertyName="Prop3" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub2" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="PropA" typeName="string" />
                        <ECProperty propertyName="PropB" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert and validate values to upgraded schema",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Prop1,Prop2,Prop3) VALUES (102,2,2,'Sub1 2','1', '2', 3)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            auto expected = JsonValue(R"json([{"Code":101,"Origin": {"x":1.0,"y":1.0},"Name": "Sub1 1", "Prop1":"1","Prop2":"2"},{"Code":102,"Origin": {"x":2.0,"y":2.0},"Name": "Sub1 2", "Prop1":"1","Prop2":"2", "Prop3": 3}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code,Origin,Name, Prop1, Prop2, Prop3 FROM ts.Sub1"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            expected = JsonValue(R"json([{"Code":201,"Origin": {"x":1.0,"y":1.0},"Name": "Sub2 1", "PropA":"A","PropB":1}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code,Origin,Name,PropA, PropB FROM ts.Sub2"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            }
    );

    const auto SCHEMA3_HASH_ECDB_SCHEMA = "8c46915baf4e019bf8126756ce91af831b706528afd600bd3de7b0ccdbc1622e";
    const auto SCHEMA3_HASH_ECDB_MAP = "1fa65d6e13f36e0b14cc595d08d13cdb8f1e75e96f6c6bf477a344411c7d1d3e";
    const auto SCHEMA3_HASH_SQLITE_SCHEMA = "0535fb0fc5cd6b23e103f16be7eb1716127142cf81323d27af56be2d85d2211e";
    Test(
        "Add property to SubBase",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                        </ECCustomAttributes>
                        <BaseClass>Element</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <BaseClass>GeometricElement</BaseClass>
                        <ECProperty propertyName="Origin" typeName="Point2d" />
                    </ECEntityClass>
                    <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                        <BaseClass>Geometric2dElement</BaseClass>
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubBase" modifier="Abstract" >
                        <BaseClass>PhysicalElement</BaseClass>
                        <ECProperty propertyName="Kind" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub1" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="Prop1" typeName="string" />
                        <ECProperty propertyName="Prop2" typeName="string" />
                        <ECProperty propertyName="Prop3" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub2" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="PropA" typeName="string" />
                        <ECProperty propertyName="PropB" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

            CheckHashes(*m_briefcase, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert and validate values to final schema",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Kind,Prop1,Prop2,Prop3) VALUES (103,3,3,'Sub1 3',3,'1', '2', 3)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub2(Code,Origin.X,Origin.Y,Name,Kind,PropA,PropB) VALUES (202,2,2,'Sub2 2',2,'A', 2)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            auto expected = JsonValue(
                R"json([{"Code":101,"Origin": {"x":1.0,"y":1.0},"Name": "Sub1 1", "Prop1":"1","Prop2":"2"},
                {"Code":102,"Origin": {"x":2.0,"y":2.0},"Name": "Sub1 2", "Prop1":"1","Prop2":"2", "Prop3": 3},
                {"Code":103,"Origin": {"x":3.0,"y":3.0},"Name": "Sub1 3", "Kind":3, "Prop1":"1","Prop2":"2", "Prop3": 3}])json"
            );
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code,Origin,Name,Kind,Prop1,Prop2,Prop3 FROM ts.Sub1"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            expected = JsonValue(
                R"json([{"Code":201,"Origin": {"x":1.0,"y":1.0},"Name": "Sub2 1", "PropA":"A","PropB":1},
                {"Code":202,"Origin": {"x":2.0,"y":2.0},"Name": "Sub2 2", "Kind":2, "PropA":"A","PropB":2}])json"
            );
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code,Origin,Name,Kind,PropA,PropB FROM ts.Sub2"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddPropertyToSubclassThenPropertyToBaseClass_TPH_JoinedTable_SharedCols_AllAddedPropsToOverflow)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "df0df0956dc32d9c0be325ea76969728dfb1f0c7a51678e872012f046529145e";
    const auto SCHEMA1_HASH_ECDB_MAP = "972b210d0debf0168955060c25b696dd80e99e176a125b7e0a00621acdb76984";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "cdb949b55ac86f953832043bd5a5584b49e3f212e6c22012217319bb592a99c9";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                        </ECCustomAttributes>
                        <BaseClass>Element</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <BaseClass>GeometricElement</BaseClass>
                        <ECProperty propertyName="Origin" typeName="Point2d" />
                    </ECEntityClass>
                    <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                        <BaseClass>Geometric2dElement</BaseClass>
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubBase" modifier="Abstract" >
                        <BaseClass>PhysicalElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub1" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="Prop1" typeName="string" />
                        <ECProperty propertyName="Prop2" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub2" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="PropA" typeName="string" />
                        <ECProperty propertyName="PropB" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(
                SchemaImportResult::OK,
                SetupECDb(
                    "AddPropertyToSubclassThenPropertyToBaseClass_TPH_JoinedTable_SharedCols_AllAddedPropsToOverflow",
                    schema,
                    SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade
                )
            );
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert values to initial schema",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Prop1,Prop2) VALUES (101,1,1,'Sub1 1','1', '2')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub2(Code,Origin.X,Origin.Y,Name,PropA,PropB) VALUES (201,1,1,'Sub2 1','A', 1)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "b59d9d791934049a7f0e839c7df4e2cde2162fd8f1f282b8e43e2cdd079c04ea";
    const auto SCHEMA2_HASH_ECDB_MAP = "f05ebacafe4c08f4b3763914157284f3a2917e8c32b519d6afcb993c9f424de0";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "1c2d8fbbf412b7e4623c14af0fb850f5a96711103f3364184500c0605ae9c7a1";
    Test(
        "Add property to Sub1",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                        </ECCustomAttributes>
                        <BaseClass>Element</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <BaseClass>GeometricElement</BaseClass>
                        <ECProperty propertyName="Origin" typeName="Point2d" />
                    </ECEntityClass>
                    <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                        <BaseClass>Geometric2dElement</BaseClass>
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubBase" modifier="Abstract" >
                        <BaseClass>PhysicalElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub1" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="Prop1" typeName="string" />
                        <ECProperty propertyName="Prop2" typeName="string" />
                        <ECProperty propertyName="Prop3" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub2" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="PropA" typeName="string" />
                        <ECProperty propertyName="PropB" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert and validate values to upgraded schema",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Prop1,Prop2,Prop3) VALUES (102,2,2,'Sub1 2','1', '2', 3)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            auto expected = JsonValue(R"json([{"Code":101,"Origin": {"x":1.0,"y":1.0},"Name": "Sub1 1", "Prop1":"1","Prop2":"2"},{"Code":102,"Origin": {"x":2.0,"y":2.0},"Name": "Sub1 2", "Prop1":"1","Prop2":"2", "Prop3": 3}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code,Origin,Name, Prop1, Prop2, Prop3 FROM ts.Sub1"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            expected = JsonValue(R"json([{"Code":201,"Origin": {"x":1.0,"y":1.0},"Name": "Sub2 1", "PropA":"A","PropB":1}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code,Origin,Name,PropA, PropB FROM ts.Sub2"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            }
    );

    const auto SCHEMA3_HASH_ECDB_SCHEMA = "2362402456d838c9cfb15dd278e651571a84bba0c15189b2e0894966e78fff04";
    const auto SCHEMA3_HASH_ECDB_MAP = "12969517d427966e0c143181f523a33314fbee6c8d11c3ab1170d6eb5cbef552";
    const auto SCHEMA3_HASH_SQLITE_SCHEMA = "fa3932855399557ee728462fd837644af585c2c6b17284b679b69b9372a78ab1";
    Test(
        "Add property to SubBase",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                        </ECCustomAttributes>
                        <BaseClass>Element</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <BaseClass>GeometricElement</BaseClass>
                        <ECProperty propertyName="Origin" typeName="Point2d" />
                    </ECEntityClass>
                    <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                        <BaseClass>Geometric2dElement</BaseClass>
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubBase" modifier="Abstract" >
                        <BaseClass>PhysicalElement</BaseClass>
                        <ECProperty propertyName="Kind" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub1" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="Prop1" typeName="string" />
                        <ECProperty propertyName="Prop2" typeName="string" />
                        <ECProperty propertyName="Prop3" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub2" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="PropA" typeName="string" />
                        <ECProperty propertyName="PropB" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

            CheckHashes(*m_briefcase, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert and validate values to final schema",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Kind,Prop1,Prop2,Prop3) VALUES (103,3,3,'Sub1 3',3,'1', '2', 3)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub2(Code,Origin.X,Origin.Y,Name,Kind,PropA,PropB) VALUES (202,2,2,'Sub2 2',2,'A', 2)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            auto expected = JsonValue(
                R"json([{"Code":101,"Origin": {"x":1.0,"y":1.0},"Name": "Sub1 1", "Prop1":"1","Prop2":"2"},
                {"Code":102,"Origin": {"x":2.0,"y":2.0},"Name": "Sub1 2", "Prop1":"1","Prop2":"2", "Prop3": 3},
                {"Code":103,"Origin": {"x":3.0,"y":3.0},"Name": "Sub1 3", "Kind":3, "Prop1":"1","Prop2":"2", "Prop3": 3}])json"
            );
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code,Origin,Name,Kind,Prop1,Prop2,Prop3 FROM ts.Sub1"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            expected = JsonValue(
                R"json([{"Code":201,"Origin": {"x":1.0,"y":1.0},"Name": "Sub2 1", "PropA":"A","PropB":1},
                {"Code":202,"Origin": {"x":2.0,"y":2.0},"Name": "Sub2 2", "Kind":2, "PropA":"A","PropB":2}])json"
            );
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code,Origin,Name,Kind,PropA,PropB FROM ts.Sub2"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddPropertyToSubclassThenPropertyToBaseClass_TPH_JoinedTable_SharedCols_AddedBasePropToOverflow)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "2cc81e820cf5b5e866f641aeaee96cbedbc54e77c6d1a52de204e9b069207003";
    const auto SCHEMA1_HASH_ECDB_MAP = "07189aca401a6e2b70d563e7c10cb23cecc6783fe5b02185cd53952360b525da";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "ebe2cbd764b3e700395e4bccbac629b543fe5e1758da6db5e627ccbfd12f10be";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                        </ECCustomAttributes>
                        <BaseClass>Element</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <BaseClass>GeometricElement</BaseClass>
                        <ECProperty propertyName="Origin" typeName="Point2d" />
                    </ECEntityClass>
                    <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                        <BaseClass>Geometric2dElement</BaseClass>
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubBase" modifier="Abstract" >
                        <BaseClass>PhysicalElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub1" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="Prop1" typeName="string" />
                        <ECProperty propertyName="Prop2" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub2" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="PropA" typeName="string" />
                        <ECProperty propertyName="PropB" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("AddPropertyToSubclassThenPropertyToBaseClass_TPH_JoinedTable_SharedCols_AddedBasePropToOverflow", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert values to initial schema",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Prop1,Prop2) VALUES (101,1,1,'Sub1 1','1', '2')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub2(Code,Origin.X,Origin.Y,Name,PropA,PropB) VALUES (201,1,1,'Sub2 1','A', 1)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "bc135bcc4384862b3390356422497cc037a290741ef29d311e1d3f428ca87110";
    const auto SCHEMA2_HASH_ECDB_MAP = "97f7dba66cf9300e4851bfed1a4af5c3f2b3f34af2f255a5a49a27bc77cf9588";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "fa35be3b944aeb606d51e60ff02e9a2b4caad5051882535bce90b0be77b66319";
    Test(
        "Add property to Sub1",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                        </ECCustomAttributes>
                        <BaseClass>Element</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <BaseClass>GeometricElement</BaseClass>
                        <ECProperty propertyName="Origin" typeName="Point2d" />
                    </ECEntityClass>
                    <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                        <BaseClass>Geometric2dElement</BaseClass>
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubBase" modifier="Abstract" >
                        <BaseClass>PhysicalElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub1" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="Prop1" typeName="string" />
                        <ECProperty propertyName="Prop2" typeName="string" />
                        <ECProperty propertyName="Prop3" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub2" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="PropA" typeName="string" />
                        <ECProperty propertyName="PropB" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert and validate values to upgraded schema",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Prop1,Prop2,Prop3) VALUES (102,2,2,'Sub1 2','1', '2', 3)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            auto expected = JsonValue(R"json([{"Code":101,"Origin": {"x":1.0,"y":1.0},"Name": "Sub1 1", "Prop1":"1","Prop2":"2"},{"Code":102,"Origin": {"x":2.0,"y":2.0},"Name": "Sub1 2", "Prop1":"1","Prop2":"2", "Prop3": 3}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code,Origin,Name, Prop1, Prop2, Prop3 FROM ts.Sub1"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            expected = JsonValue(R"json([{"Code":201,"Origin": {"x":1.0,"y":1.0},"Name": "Sub2 1", "PropA":"A","PropB":1}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code,Origin,Name,PropA, PropB FROM ts.Sub2"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            }
    );

    const auto SCHEMA3_HASH_ECDB_SCHEMA = "d2001aabeb5413315fbccf51e27d2631e79bee7a4fb56aaa441b4095af993904";
    const auto SCHEMA3_HASH_ECDB_MAP = "41528e6148f887e82d1881ee11edd47428de14855940adeb00c03924f722df97";
    const auto SCHEMA3_HASH_SQLITE_SCHEMA = "fccbddbf34fd8230f25884931499595acb18dfa9345b273d605c2141bfbbe9f2";
    Test(
        "Add property to SubBase",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                        </ECCustomAttributes>
                        <BaseClass>Element</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Geometric2dElement" modifier="Abstract" >
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <BaseClass>GeometricElement</BaseClass>
                        <ECProperty propertyName="Origin" typeName="Point2d" />
                    </ECEntityClass>
                    <ECEntityClass typeName="PhysicalElement" modifier="Abstract" >
                        <BaseClass>Geometric2dElement</BaseClass>
                        <ECProperty propertyName="Name" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="SubBase" modifier="Abstract" >
                        <BaseClass>PhysicalElement</BaseClass>
                        <ECProperty propertyName="Kind" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub1" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="Prop1" typeName="string" />
                        <ECProperty propertyName="Prop2" typeName="string" />
                        <ECProperty propertyName="Prop3" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub2" modifier="None" >
                        <BaseClass>SubBase</BaseClass>
                        <ECProperty propertyName="PropA" typeName="string" />
                        <ECProperty propertyName="PropB" typeName="int" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());

            CheckHashes(*m_briefcase, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert and validate values to final schema",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub1(Code,Origin.X,Origin.Y,Name,Kind,Prop1,Prop2,Prop3) VALUES (103,3,3,'Sub1 3',3,'1', '2', 3)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub2(Code,Origin.X,Origin.Y,Name,Kind,PropA,PropB) VALUES (202,2,2,'Sub2 2',2,'A', 2)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            auto expected = JsonValue(
                R"json([{"Code":101,"Origin": {"x":1.0,"y":1.0},"Name": "Sub1 1", "Prop1":"1","Prop2":"2"},
                {"Code":102,"Origin": {"x":2.0,"y":2.0},"Name": "Sub1 2", "Prop1":"1","Prop2":"2", "Prop3": 3},
                {"Code":103,"Origin": {"x":3.0,"y":3.0},"Name": "Sub1 3", "Kind":3, "Prop1":"1","Prop2":"2", "Prop3": 3}])json"
            );
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code,Origin,Name,Kind,Prop1,Prop2,Prop3 FROM ts.Sub1"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            expected = JsonValue(
                R"json([{"Code":201,"Origin": {"x":1.0,"y":1.0},"Name": "Sub2 1", "PropA":"A","PropB":1},
                {"Code":202,"Origin": {"x":2.0,"y":2.0},"Name": "Sub2 2", "Kind":2, "PropA":"A","PropB":2}])json"
            );
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT Code,Origin,Name,Kind,PropA,PropB FROM ts.Sub2"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, Add_Delete_ECProperty_ShareColumns)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "f3ac3f3c779506fc8e0568bb94a0a348c4e12b482e3e554da176e0be644caee1";
    const auto SCHEMA1_HASH_ECDB_MAP = "7ccad25164023d18df6d27269b9d3d04a61c60e4ef4daed1fd9196b530c4f0a8";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "7e7c571fae587a105f76a300ffed5b4f1ac2a7297c51e2bd1142fccf161029ea";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Parent' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P1' typeName='int' />
                        <ECProperty propertyName='P2' typeName='int' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test("Check Parent table column count", [&]() { ASSERT_EQ(4, GetColumnCount("ts_Parent")); });

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "93138d9b82f580c3196bbbd7e01c6693561d53fa1c2b0c8a5ddd86bd4d55bf03";
    const auto SCHEMA2_HASH_ECDB_MAP = "e2885712613bbc0caf2b7710a20430c1199f88172002083602e8d0c9956ef968";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "616136b709172fd8b18c5d9213f3c87b5c3d05a852787260324b52e009c45ae4";
    Test(
        "Upgrade with some attributes and import schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Parent' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P1' typeName='int' />
                        <ECProperty propertyName='P3' typeName='int' />
                        <ECProperty propertyName='P4' typeName='int' />
                        <ECProperty propertyName='P5' typeName='int' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify attributes of upgraded successfully",
        [&]()
            {
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *m_briefcase,
                    [&]()
                        {
                        ASSERT_TRUE(m_briefcase->TableExists("ts_Parent"));
                        ASSERT_EQ(6, GetColumnCount("ts_Parent"));

                        CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*m_briefcase));
                        ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
                        }
                )
            );

            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Parent(P1, P3, P4, P5) VALUES(1, 2, 3, 4)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddNewPropertyModifyAllExistingAttributes)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "482c8e3b064ef4866027ed52003f319d83d4374385fb9c82430a8360af5b5c59";
    const auto SCHEMA1_HASH_ECDB_MAP = "82a4f93a397725447cf558de379c6f5a4c044c2de789edf25e9f889fbaf52a43";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "77af11ada178c9b759bec7717d606ddd7d1ccfae9168df93c3918729958ef815";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' >
                        </ECProperty>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "0def004efc014ab44efc82f5b84e08388897086e38bbd42bf3405028ab5c323c";
    const auto SCHEMA2_HASH_ECDB_MAP = "27eb6ad10db6eb8321f1bb6f50f4e7e758ae528f00dea0094ba8244cddbabd0f";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "8b832600509809548c2f2b03e1b963075e7b44395edb842ca660ed4c1c9ec9c1";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >
                        </ECProperty>
                        <ECProperty propertyName='NewTestProperty' displayLabel='New Test Property' description='this is new property' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify Schema, Class, property and CAClassProperties attributes upgraded successfully",
        [&]()
            {
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *m_briefcase,
                    [&]()
                        {
                        CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*m_briefcase));
                        ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
                        }
                )
            );

            //Verify Schema, Class, property and CAClassProperties attributes upgraded successfully
            auto testSchema = m_briefcase->Schemas().GetSchema("TestSchema");
            ASSERT_NE(testSchema, nullptr);
            ASSERT_EQ(testSchema->GetAlias(), "ts");
            ASSERT_EQ(testSchema->GetDisplayLabel(), "Modified Test Schema");
            ASSERT_EQ(testSchema->GetDescription(), "modified test schema");

            auto testClass = testSchema->GetClassCP("TestClass");
            ASSERT_NE(testClass, nullptr);
            ASSERT_EQ(testClass->GetDisplayLabel(), "Modified Test Class");
            ASSERT_EQ(testClass->GetDescription(), "modified test class");

            auto testProperty = testClass->GetPropertyP("TestProperty");
            ASSERT_NE(testProperty, nullptr);
            ASSERT_EQ(testProperty->GetDisplayLabel(), "Modified Test Property");
            ASSERT_EQ(testProperty->GetDescription(), "this is modified property");

            //verify newly added Property exists
            auto newTestProperty = testClass->GetPropertyP("NewTestProperty");
            ASSERT_NE(newTestProperty, nullptr);
            ASSERT_EQ(newTestProperty->GetDisplayLabel(), "New Test Property");
            ASSERT_EQ(newTestProperty->GetDescription(), "this is new property");

            //Verify attributes via ECSql using MataSchema
            ECSqlStatement statement;
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
            ASSERT_STREQ("modified test schema", statement.GetValueText(1));
            ASSERT_STREQ("ts", statement.GetValueText(2));
            statement.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
            ASSERT_STREQ("modified test class", statement.GetValueText(1));
            statement.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='TestProperty'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Modified Test Property", statement.GetValueText(0));
            ASSERT_STREQ("this is modified property", statement.GetValueText(1));
            statement.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='NewTestProperty'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("New Test Property", statement.GetValueText(0));
            ASSERT_STREQ("this is new property", statement.GetValueText(1));
            statement.Finalize();

            //Query existing and newly added Entity Classes
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT TestProperty, NewTestProperty FROM ts.TestClass"));
            ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
            statement.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddNewCAOnProperty)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "482c8e3b064ef4866027ed52003f319d83d4374385fb9c82430a8360af5b5c59";
    const auto SCHEMA1_HASH_ECDB_MAP = "82a4f93a397725447cf558de379c6f5a4c044c2de789edf25e9f889fbaf52a43";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "77af11ada178c9b759bec7717d606ddd7d1ccfae9168df93c3918729958ef815";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Test Schema' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='TestClass' displayLabel='Test Class' description='This is test Class' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "31f58e3f67f6e434de26af890cd60058af4d970ed51d621e06719f557bcf8f6c";
    const auto SCHEMA2_HASH_ECDB_MAP = "92d4761e5e147a0a15cc49571291ca1c7f21493b8d00f0a769afd38a2960d297";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "77af11ada178c9b759bec7717d606ddd7d1ccfae9168df93c3918729958ef815";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' displayLabel='Modified Test Schema' description='modified test schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECCustomAttributeClass typeName = 'TestCA' appliesTo = 'PrimitiveProperty'>
                        <ECProperty propertyName = 'IsUnique' typeName = 'boolean' description = 'Only allow unique values for this property.' />
                        <ECProperty propertyName = 'Collation' typeName = 'string' description = 'Specifies how string comparisons should work for this property. Possible values: Binary (default): bit to bit matching. NoCase: The same as binary, except that the 26 upper case characters of ASCII are folded to their lower case equivalents before comparing. Note that it only folds ASCII characters. RTrim: The same as binary, except that trailing space characters are ignored.' />
                    </ECCustomAttributeClass>
                    <ECEntityClass typeName='TestClass' displayLabel='Modified Test Class' description='modified test class' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Modified Test Property' description='this is modified property' typeName='string' >
                            <ECCustomAttributes>
                                <TestCA xmlns='TestSchema.01.00'>
                                    <IsUnique>false</IsUnique>
                                </TestCA>
                            </ECCustomAttributes>
                        </ECProperty>
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify Schema, Class and property attributes upgraded successfully",
        [&]()
            {
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *m_briefcase,
                    [&]()
                        {
                        CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*m_briefcase));
                        ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
                        }
                )
            );

            //Verify Schema, Class and property attributes upgraded successfully
            auto testSchema = m_briefcase->Schemas().GetSchema("TestSchema");
            ASSERT_NE(testSchema, nullptr);
            ASSERT_EQ(testSchema->GetAlias(), "ts");
            ASSERT_EQ(testSchema->GetDisplayLabel(), "Modified Test Schema");
            ASSERT_EQ(testSchema->GetDescription(), "modified test schema");

            auto testClass = testSchema->GetClassCP("TestClass");
            ASSERT_NE(testClass, nullptr);
            ASSERT_EQ(testClass->GetDisplayLabel(), "Modified Test Class");
            ASSERT_EQ(testClass->GetDescription(), "modified test class");

            auto testProperty = testClass->GetPropertyP("TestProperty");
            ASSERT_NE(testProperty, nullptr);
            ASSERT_EQ(testProperty->GetDisplayLabel(), "Modified Test Property");
            ASSERT_EQ(testProperty->GetDescription(), "this is modified property");

            //Verify attributes via ECSql using MataSchema
            ECSqlStatement statement;
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description, Alias FROM meta.ECSchemaDef WHERE Name='TestSchema'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Modified Test Schema", statement.GetValueText(0));
            ASSERT_STREQ("modified test schema", statement.GetValueText(1));
            ASSERT_STREQ("ts", statement.GetValueText(2));

            statement.Finalize();
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description FROM meta.ECClassDef WHERE Name='TestClass'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Modified Test Class", statement.GetValueText(0));
            ASSERT_STREQ("modified test class", statement.GetValueText(1));

            statement.Finalize();
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='TestProperty'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Modified Test Property", statement.GetValueText(0));
            ASSERT_STREQ("this is modified property", statement.GetValueText(1));

            //Query Property
            statement.Finalize();
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT TestProperty FROM ts.TestClass"));
            ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());

            //verify newly added CA on Property
            testProperty = m_briefcase->Schemas().GetSchema("TestSchema")->GetClassCP("TestClass")->GetPropertyP("TestProperty");
            ASSERT_NE(testProperty, nullptr);
            auto testCA = testProperty->GetCustomAttribute("TestCA");
            ASSERT_NE(testCA, nullptr);
            ECValue val;
            ASSERT_EQ(ECObjectsStatus::Success, testCA->GetValue(val, "IsUnique"));
            ASSERT_FALSE(val.GetBoolean());
            statement.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateECDbMapCA_AddMaxSharedColumnsBeforeOverflow)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "527e4f7e7f42ada3feb168b808bc7f7b4e8bcd0126c0feccb142ecbdc7ce4637";
    const auto SCHEMA1_HASH_ECDB_MAP = "cb2d5561bdf63a6e12212081a48862d73338de2d4964f0cb80fb675880d6815d";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "97780c55b8972e6b86c8bd514db8948c6369dca78b94d4f14da12eb4d6dd8e59";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Parent' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P1' typeName='int' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Parent' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                            </ECCustomAttributes>
                        <ECProperty propertyName='P1' typeName='int' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub' modifier='None' >
                        <BaseClass>Parent</BaseClass>
                        <ECProperty propertyName='P2' typeName='int' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->AbandonChanges());
            CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, MaxSharedColumnsBeforeOverflowForSubClasses_AddProperty)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "55e6002482091413020ed9d445251d173cd17c645ee32baee4e83fc1c2af211a";
    const auto SCHEMA1_HASH_ECDB_MAP = "6314fb2092ff0a231a12a50bcde1d90d64a6c58263d53718b4e34c8d58fa7591";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "9afc1fa2bb0dc6be01dea2ad48253e05cc127bbbd5134a205ee35602735f2ad7";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Parent' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P1' typeName='int' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub1' modifier='None'>
                        <BaseClass>Parent</BaseClass>
                        <ECProperty propertyName='S1' typeName='double' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test("Check Parent table column count", [&]() { ASSERT_EQ(4, GetColumnCount("ts_Parent")); });

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "62ff287b5e854e2b607ecfc7f4049941ff5a75447bb3d9b80c9b1823e8228a52";
    const auto SCHEMA2_HASH_ECDB_MAP = "5ca4e3f05088b4d40e4f8ed4c756ceba99ade166dc3d4ddd66f69b9c48bdfa66";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "bcd3a8ade2f995a66dc59b7dbeb79a18a19d566e76029e3029e065a6c9b49934";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Parent' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P1' typeName='int' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub1' modifier='None'>
                        <BaseClass>Parent</BaseClass>
                        <ECProperty propertyName='S1' typeName='double' />
                        <ECProperty propertyName='S2' typeName='double' />
                        <ECProperty propertyName='S3' typeName='double' />
                        <ECProperty propertyName='S4' typeName='double' />
                        <ECProperty propertyName='S5' typeName='double' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check Parent table column count of edited schema",
        [&]()
            {
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *m_briefcase,
                    [&]()
                        {
                        ASSERT_EQ(8, GetColumnCount("ts_Parent"));

                        CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*m_briefcase));
                        ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
                        }
                )
            );
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// Another place where the sqlite schema hash is not what expected. Why is this happening?
TEST_F(SchemaSyncTestFixture, MaxSharedColumnsBeforeOverflowWithJoinedTable_AddProperty)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "72d492c18ecdfb5c9d22e2fc9e1ebcae349972d7be78a7040d79afeaf67eea3e";
    const auto SCHEMA1_HASH_ECDB_MAP = "ad20c24f80db11abc958f5f88be6963f917a5aba3038cdef6a20cfa8427ec6e1";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "1ac9eb8ed7dde6b0057bc57f4d9d160800e495b9424c9fca2090540bef948116";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Parent' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P1' typeName='int' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub1' modifier='None'>
                        <BaseClass>Parent</BaseClass>
                        <ECProperty propertyName='S1' typeName='double' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check table column counts",
        [&]()
            {
            ASSERT_EQ(3, GetColumnCount("ts_Parent"));
            ASSERT_EQ(3, GetColumnCount("ts_Sub1"));
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "f4878d964a644b436dacb9fb48ce2e5965dd7aa5740386caf7006e9c2032ed18";
    const auto SCHEMA2_HASH_ECDB_MAP = "03b35120cfc7a025601e8383bb2d143b80c61a6126d9f2769fd13de69f430e34";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "84bbd4adf50d92be2f91e99b1908c5bfae8ece0ba5f9e4f8609552f033c44088";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Parent' modifier='None' >
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>
                        </ECCustomAttributes>
                        <ECProperty propertyName='P1' typeName='int' />
                   </ECEntityClass>
                    <ECEntityClass typeName='Sub1' modifier='None'>
                        <BaseClass>Parent</BaseClass>
                        <ECProperty propertyName='S1' typeName='double' />
                        <ECProperty propertyName='S2' typeName='double' />
                        <ECProperty propertyName='S3' typeName='double' />
                        <ECProperty propertyName='S4' typeName='double' />
                        <ECProperty propertyName='S5' typeName='double' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        const auto SCHEMA_HASH_SQLITE_SCHEMA = "ae5a6aa1c3e672de4ac4dc406cee1187661c7b5807c02988012e72c140647985"; // What is this?
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
            CheckHashes(*newBriefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check table column counts of edited schema",
        [&]()
            {
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *m_briefcase,
                    [&]()
                        {
                        const auto SCHEMA_HASH_SQLITE_SCHEMA = "f7c6cd08cd9cf6a6b39f6c048819b0ead2192e7fb67e4493025d20430742f5a1"; //What is this?
                        ASSERT_EQ(3, GetColumnCount("ts_Parent"));
                        ASSERT_EQ(7, GetColumnCount("ts_Sub1"));

                        CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*m_briefcase));
                        ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
                        }
                )
            );
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, ImportMultipleSchemaVersions_AddNewProperty)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "5c23555b96567e545629e02fccfcaf4ffdc181025242c5f53e8702da57356ea3";
    const auto SCHEMA1_HASH_ECDB_MAP = "32608ea390282589dafbb36721fb70a9336a359434d056ab67dd245415c0f814";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "533fbcc8fc6a9a9efdb25aff7db7bdb91856955e6b8c236b3f5218b64361d533";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.2.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='TestClass' modifier='None' >
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Schema upgrade with lower minor version not allowed",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='TestClass' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify newly added property must not exist at this point",
        [&]()
            {
            auto testSchema = m_briefcase->Schemas().GetSchema("TestSchema");
            ASSERT_NE(testSchema, nullptr);

            auto testClass = testSchema->GetClassCP("TestClass");
            ASSERT_NE(testClass, nullptr);

            auto testProperty = testClass->GetPropertyP("TestProperty");
            ASSERT_EQ(testProperty, nullptr);
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "19e2850510ac54893901108839240aa72f40b7a72cd6bd48c8dd3f47c8ef2b7b";
    const auto SCHEMA2_HASH_ECDB_MAP = "82a4f93a397725447cf558de379c6f5a4c044c2de789edf25e9f889fbaf52a43";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "77af11ada178c9b759bec7717d606ddd7d1ccfae9168df93c3918729958ef815";
    Test(
        "import edited schema with higher minor version with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.3.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='TestClass' modifier='None' >
                        <ECProperty propertyName='TestProperty' displayLabel='Test Property' description='this is property' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*m_briefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify newly added property must exist after third schema import",
        [&]()
            {
            auto testSchema = m_briefcase->Schemas().GetSchema("TestSchema");
            ASSERT_NE(testSchema, nullptr);

            auto testClass = testSchema->GetClassCP("TestClass");
            ASSERT_NE(testClass, nullptr);

            auto testProperty = testClass->GetPropertyP("TestProperty");
            ASSERT_NE(testProperty, nullptr);
            ASSERT_EQ(testProperty->GetDisplayLabel(), "Test Property");
            ASSERT_EQ(testProperty->GetDescription(), "this is property");

            ReopenECDb();

            ECSqlStatement statement;
            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT DisplayLabel, Description FROM meta.ECPropertyDef WHERE Name='TestProperty'"));
            ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
            ASSERT_STREQ("Test Property", statement.GetValueText(0));
            ASSERT_STREQ("this is property", statement.GetValueText(1));
            statement.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*m_briefcase, "SELECT TestProperty FROM ts.TestClass"));
            ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
            statement.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

// //---------------------------------------------------------------------------------------
// // @bsimethod
// //+---------------+---------------+---------------+---------------+---------------+------
// // Doesn't create from file
// TEST_F(SchemaSyncTestFixture, UpdateMultipleSchemasInDb)
//     {
//     const auto SCHEMA1_HASH_ECDB_SCHEMA = "5c23555b96567e545629e02fccfcaf4ffdc181025242c5f53e8702da57356ea3";
//     const auto SCHEMA1_HASH_ECDB_MAP = "32608ea390282589dafbb36721fb70a9336a359434d056ab67dd245415c0f814";
//     const auto SCHEMA1_HASH_SQLITE_SCHEMA = "533fbcc8fc6a9a9efdb25aff7db7bdb91856955e6b8c236b3f5218b64361d533";
//     Test(
//         "import initial schema",
//         [&]()
//             {
//             ASSERT_EQ(SchemaImportResult::OK, SetupECDb("updateStartupCompanyschema.ecdb", SchemaItem::CreateForFile("DSCacheSchema.01.00.00.ecschema.xml")));
//             CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
//             m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
//             }
//     );
//     Test(
//         "Schema upgrade with lower minor version not allowed",
//         [&]()
//             {
//             ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(SchemaItem::CreateForFile("DSCacheSchema.01.00.03.ecschema.xml")));
//             ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
//             CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
//             m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
//             }
//     );
//     }

// ---------------------------------------------------------------------------------------
// @bsimethod
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UnsettingSchemaAlias)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "196c3dba02927f22033ef960281758702f3f48a5e52fa7e1dde012472ec7ea5d";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );
    Test(
        "Schema alias can't be set to empty",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                </ECSchema>)xml"
            );
            BeTest::SetFailOnAssert(false);
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            BeTest::SetFailOnAssert(true);
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, InvalidValueForSchemaAlias)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "196c3dba02927f22033ef960281758702f3f48a5e52fa7e1dde012472ec7ea5d";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "615c7cbe8e5146d30d1ad2a078ef1d6d2b72cb835a9688a5f347b18a0dd8d149";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema1' alias='ts1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Try Upgrading to already existing alias",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, MajorVersionChange_WithoutMajorVersionIncremented)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "af7be50d60a542eea7ee73b740d6577b990de976e0bed125a0ca57f89e14469c";
    const auto SCHEMA1_HASH_ECDB_MAP = "fd957857f78f1211c894cd907c4f68fcc9a3cd9cd1f5066ad33575ce56899e8c";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "5f9d0f627116d1b5c25df711b3ace0455e1122e9552b90ebab2f5b4800f37653";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='Foo' modifier='None'>
                        <ECProperty propertyName='S' typeName='string' />
                        <ECProperty propertyName='D' typeName='double' />
                        <ECProperty propertyName='L' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Major Version change without Major Version incremented is expected to be not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Schema Update with ECSchema Major Version decremented is expected to be not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='Foo' modifier='None'>
                        <ECProperty propertyName='S' typeName='string' />
                        <ECProperty propertyName='D' typeName='double' />
                        <ECProperty propertyName='L' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, Delete_ECDbMapCANotSupported)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "32d7cd8f9671cdd1ab18f7a9d4da2fa1a6ec689f21df0ef2c386fb1874dca69d";
    const auto SCHEMA1_HASH_ECDB_MAP = "15f9bf3ac0037f5ce8c270d1014480dd46319bbe42aa9b3393ac95fdce07b745";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "66da69352d1835f2ce0c2d05ea1cb1baa8fcb9c7b4fd021ae29ba7c5fc73e683";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Goo' modifier='None'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName='S' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Delete ECDbMap CA",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Goo' modifier='None'>
                        <ECProperty propertyName='S' typeName='string' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );

            auto newBriefcase = m_hub->CreateBriefcase();
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *newBriefcase,
                    [&]()
                        {
                        CheckHashes(*newBriefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*newBriefcase));
                        ASSERT_EQ(BE_SQLITE_OK, newBriefcase->SaveChanges());
                        }
                )
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*newBriefcase, schema, SchemaManager::SchemaImportOptions::None, GetSharedChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Pull from shared channel and check hashes",
        [&]()
            {
            ASSERT_EQ(
                SharedSchemaChannel::Status::OK,
                m_schemaChannel->Pull(
                    *m_briefcase,
                    [&]()
                        {
                        CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
                        ASSERT_TRUE(ForeignkeyCheck(*m_briefcase));
                        ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
                        }
                )
            );
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, Delete_ECEntityClass_OwnTable)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "c7f8a071c0a9638b4fd840e3ede4aef82c3b264c37389457ee2f5a853ae9594f";
    const auto SCHEMA1_HASH_ECDB_MAP = "8d7ec81880ecdd60c65416cd3a1dfa453d06b1bfc3e3671332e3d3cee596e9c2";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "ba51a2faf251723c905bdebb6154d0ebfddfb90a7e3ac5d181eac666e3316e61";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='Foo' modifier='None'>
                        <ECProperty propertyName='S' typeName='string' />
                        <ECProperty propertyName='D' typeName='double' />
                        <ECProperty propertyName='L' typeName='long' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Goo' modifier='None'>
                        <ECProperty propertyName='S' typeName='string' />
                        <ECProperty propertyName='D' typeName='double' />
                        <ECProperty propertyName='L' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert values to initial schema",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Foo(S,D,L) VALUES ('test1', 1.3, 334)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Foo(S,D,L) VALUES ('test2', 23.3, 234)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Goo(S,D,L) VALUES ('test3', 44.32, 3344)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Goo(S,D,L) VALUES ('test4', 13.3, 2345)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_TRUE(m_briefcase->TableExists("ts_Foo"));
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Foo"), nullptr);

            ASSERT_TRUE(m_briefcase->TableExists("ts_Goo"));
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Goo"), nullptr);

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "7ab31faa04f42e598075de77298548b18f933aa7e6e3333eee2603ac2298f030";
    const auto SCHEMA2_HASH_ECDB_MAP = "0de8d96fbdff00a95da192a0c79dbfec8555e543eac1b4058784cf72411ba57e";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "cada7f8e10fef1a4b8bfd574c110b6ecb84653e8e4a769a50ec281b56a4b3c00";
    Test(
        "Delete class should be successful",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='Goo' modifier='None'>
                        <ECProperty propertyName='S' typeName='string' />
                        <ECProperty propertyName='D' typeName='double' />
                        <ECProperty propertyName='L' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            m_briefcase->GetTracker()->EnableTracking(false);
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            m_briefcase->GetTracker()->EnableTracking(true);
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Select values from edited schema",
        [&]()
            {
            //Following should not exist
            ASSERT_EQ(m_briefcase->Schemas().GetClass("TestSchema", "Foo"), nullptr);
            ASSERT_FALSE(m_briefcase->TableExists("ts_Foo"));

            //Following should exist
            ASSERT_TRUE(m_briefcase->TableExists("ts_Goo"));
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Goo"), nullptr);

            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(*m_briefcase, "SELECT S, D, L FROM ts.Foo"));
            ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT S, D, L FROM ts.Goo"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA3_HASH_ECDB_SCHEMA = "e34ba56177313a7b82c13ec695810ce7e38b14216cdef5e162a391bab7cdaade";
    const auto SCHEMA3_HASH_ECDB_MAP = "7c022b8b2fbff36eaf40d2dc709c21a41b0ed4784a8a197ffa54c9d6e0887831";
    const auto SCHEMA3_HASH_SQLITE_SCHEMA = "ba51a2faf251723c905bdebb6154d0ebfddfb90a7e3ac5d181eac666e3316e61";
    Test(
        "Add class should be successful",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='Foo' modifier='None'>
                        <ECProperty propertyName='S' typeName='string' />
                        <ECProperty propertyName='D' typeName='double' />
                        <ECProperty propertyName='L' typeName='long' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Goo' modifier='None'>
                        <ECProperty propertyName='S' typeName='string' />
                        <ECProperty propertyName='D' typeName='double' />
                        <ECProperty propertyName='L' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert and select values from edited schema",
        [&]()
            {
            ASSERT_TRUE(m_briefcase->TableExists("ts_Foo"));
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Foo"), nullptr);

            ASSERT_TRUE(m_briefcase->TableExists("ts_Goo"));
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Goo"), nullptr);

            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT S, D, L FROM ts.Foo"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT S, D, L FROM ts.Goo"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Foo(S,D,L) VALUES ('test1', 1.3, 334)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Foo(S,D,L) VALUES ('test2', 23.3, 234)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP, SCHEMA3_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA3_HASH_ECDB_SCHEMA, SCHEMA3_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, Delete_ECEntityClass_OwnTable_TrackerNotDisabled)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "c7f8a071c0a9638b4fd840e3ede4aef82c3b264c37389457ee2f5a853ae9594f";
    const auto SCHEMA1_HASH_ECDB_MAP = "8d7ec81880ecdd60c65416cd3a1dfa453d06b1bfc3e3671332e3d3cee596e9c2";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "ba51a2faf251723c905bdebb6154d0ebfddfb90a7e3ac5d181eac666e3316e61";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='Foo' modifier='None'>
                        <ECProperty propertyName='S' typeName='string' />
                        <ECProperty propertyName='D' typeName='double' />
                        <ECProperty propertyName='L' typeName='long' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Goo' modifier='None'>
                        <ECProperty propertyName='S' typeName='string' />
                        <ECProperty propertyName='D' typeName='double' />
                        <ECProperty propertyName='L' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Delete class should not be successful",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEntityClass typeName='Goo' modifier='None'>
                        <ECProperty propertyName='S' typeName='string' />
                        <ECProperty propertyName='D' typeName='double' />
                        <ECProperty propertyName='L' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            BeTest::SetFailOnAssert(false);
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            BeTest::SetFailOnAssert(true);
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, Delete_Add_ECEntityClass_TPH)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "3909058e8f808f06e094e0ccca25e3f373c501376d4e3764e5da9f2844b8a88f";
    const auto SCHEMA1_HASH_ECDB_MAP = "9ce71ce4e533c0e9b8bf57198bae832a502d2ef9fc85bf6632d3a4994a9ec8b4";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "16559b7b4323066a05e869346c07e4f52505db2adf89f6c2bcfab2701739ddf2";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Goo' modifier='None'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName='GS' typeName='string' />
                        <ECProperty propertyName='GD' typeName='double' />
                        <ECProperty propertyName='GL' typeName='long' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Foo' modifier='None'>
                        <BaseClass>Goo</BaseClass>
                        <ECProperty propertyName='FS' typeName='string' />
                        <ECProperty propertyName='FD' typeName='double' />
                        <ECProperty propertyName='FL' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert values to initial schema",
        [&]()
            {
            ASSERT_TRUE(m_briefcase->TableExists("ts_Goo"));
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Goo"), nullptr);

            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Foo"), nullptr);
            ASSERT_FALSE(m_briefcase->TableExists("ts_Foo"));

            ASSERT_EQ(8, GetColumnCount("ts_Goo"));

            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Foo(FS,FD,FL) VALUES ('test1', 1.3, 334)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Foo(FS,FD,FL) VALUES ('test2', 23.3, 234)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "71ae05d6c0cdb615eaa64224e6f81394dd4255388d30eda7a56b45b87ce350bb";
    const auto SCHEMA2_HASH_ECDB_MAP = "30633e146c8a31c57969fc262a33ab94dabb20268b622a426b8de4e8b6050b61";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "16559b7b4323066a05e869346c07e4f52505db2adf89f6c2bcfab2701739ddf2";
    Test(
        "Delete Foo class should be successful",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Goo' modifier='None'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName='GS' typeName='string' />
                        <ECProperty propertyName='GD' typeName='double' />
                        <ECProperty propertyName='GL' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Select values from edited schema",
        [&]()
            {
            //Following should not exist
            ASSERT_EQ(m_briefcase->Schemas().GetClass("TestSchema", "Foo"), nullptr);
            ASSERT_FALSE(m_briefcase->TableExists("ts_Foo"));

            //Following should exist
            ASSERT_TRUE(m_briefcase->TableExists("ts_Goo"));
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Goo"), nullptr);

            //Verify number of columns should not change as we don't delete columns
            ASSERT_EQ(8, GetColumnCount("ts_Goo"));

            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(*m_briefcase, "SELECT FS, FD, FL FROM ts.Foo"));
            ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT GS, GD, GL FROM ts.Goo"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA3_HASH_ECDB_SCHEMA = "99817469d21395253935dd4aeff611ea056e5bf3246a696157c10db99318e63f";
    Test(
        "Delete Goo class should be successful",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                </ECSchema>)xml"
            );
            m_briefcase->GetTracker()->EnableTracking(false);
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            m_briefcase->GetTracker()->EnableTracking(true);
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA3_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA3_HASH_ECDB_SCHEMA); });
            }
    );

    Test("Check if schema is empty", [&]() { ASSERT_FALSE(m_briefcase->TableExists("ts_Goo")); });

    const auto SCHEMA4_HASH_ECDB_SCHEMA = "4aac8d09932f3b6957a21bdf1714da366eecf7c7d99e21d4ffc178e79630b2db";
    const auto SCHEMA4_HASH_ECDB_MAP = "55e2687dc83dafc38a619543de112865985c13b5c36e13b9a03d4656587a0467";
    const auto SCHEMA4_HASH_SQLITE_SCHEMA = "4cd81a84ffd7e216822c864e570327987f12172d64dd88a764d606cba6ebf3d3";
    Test(
        "Adding new Goo class with ECDbMapCA applied on it is expected to be supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='4.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Goo' modifier='None'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName='GS' typeName='string' />
                        <ECProperty propertyName='GD' typeName='double' />
                        <ECProperty propertyName='GL' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA4_HASH_ECDB_SCHEMA, SCHEMA4_HASH_ECDB_MAP, SCHEMA4_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA4_HASH_ECDB_SCHEMA, SCHEMA4_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert and select values from edited schema",
        [&]()
            {
            //Following should not exist
            ASSERT_EQ(m_briefcase->Schemas().GetClass("TestSchema", "Foo"), nullptr);
            ASSERT_FALSE(m_briefcase->TableExists("ts_Foo"));

            //Following should exist
            ASSERT_TRUE(m_briefcase->TableExists("ts_Goo"));
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Goo"), nullptr);

            ASSERT_EQ(5, GetColumnCount("ts_Goo"));

            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test3', 44.32, 3344)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Goo(GS,GD,GL) VALUES ('test4', 13.3, 2345)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();


            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA4_HASH_ECDB_SCHEMA, SCHEMA4_HASH_ECDB_MAP, SCHEMA4_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA4_HASH_ECDB_SCHEMA, SCHEMA4_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA5_HASH_ECDB_SCHEMA = "13848851d97b4fa032f4c5fbede142684b08638135e05a4666bdff37db219908";
    const auto SCHEMA5_HASH_ECDB_MAP = "62f8e92fc2f15d29050e78f1b15e2bfbb427396abd2401bd14e1196bb3f777b7";
    const auto SCHEMA5_HASH_SQLITE_SCHEMA = "16559b7b4323066a05e869346c07e4f52505db2adf89f6c2bcfab2701739ddf2";
    Test(
        "Adding new derived class should be successful",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='5.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Goo' modifier='None'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName='GS' typeName='string' />
                        <ECProperty propertyName='GD' typeName='double' />
                        <ECProperty propertyName='GL' typeName='long' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Foo' modifier='None'>
                        <BaseClass>Goo</BaseClass>
                        <ECProperty propertyName='FS' typeName='string' />
                        <ECProperty propertyName='FD' typeName='double' />
                        <ECProperty propertyName='FL' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA5_HASH_ECDB_SCHEMA, SCHEMA5_HASH_ECDB_MAP, SCHEMA5_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA5_HASH_ECDB_SCHEMA, SCHEMA5_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert and select values from edited schema",
        [&]()
            {
            //should exist
            ASSERT_FALSE(m_briefcase->TableExists("ts_Foo"));
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Foo"), nullptr);

            //following should not exist
            ASSERT_TRUE(m_briefcase->TableExists("ts_Goo"));
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Goo"), nullptr);

            ASSERT_EQ(8, GetColumnCount("ts_Goo"));

            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT FS, FD, FL FROM ts.Foo"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT GS, GD, GL FROM ts.Goo"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Foo(FS,FD,FL) VALUES ('test1', 1.3, 334)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Foo(FS,FD,FL) VALUES ('test2', 23.3, 234)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA5_HASH_ECDB_SCHEMA, SCHEMA5_HASH_ECDB_MAP, SCHEMA5_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA5_HASH_ECDB_SCHEMA, SCHEMA5_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, Delete_Add_ECEntityClass_TPH_ShareColumns)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "57557dd238919eee8d9449f2bfb9eb99375599970806377dc5287ecb38532f4f";
    const auto SCHEMA1_HASH_ECDB_MAP = "b0d641919115e7f8bc8b45ff315a1d848b2d58f3a0c8559c5db40b373f915b94";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "5dfcae2814c747473d2e3d92fae39746d12fe3e1f4d54dd1444462c670eee54d";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Base' modifier='None'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='GS' typeName='string' />
                        <ECProperty propertyName='GD' typeName='double' />
                        <ECProperty propertyName='GL' typeName='long' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub' modifier='None'>
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName='FS' typeName='string' />
                        <ECProperty propertyName='FD' typeName='double' />
                        <ECProperty propertyName='FL' typeName='long' />
                        <ECProperty propertyName='FI' typeName='int' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert values to initial schema",
        [&]()
            {
            //following table should exist.
            ASSERT_TRUE(m_briefcase->TableExists("ts_Base"));
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Base"), nullptr);

            //Following table should not exist
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Sub"), nullptr);
            ASSERT_FALSE(m_briefcase->TableExists("ts_Sub"));

            ASSERT_EQ(6, GetColumnCount("ts_Base"));
            ASSERT_EQ(5, GetColumnCount("ts_Base_Overflow"));

            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Base(GS,GD,GL) VALUES ('test3', 44.32, 3344)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Base(GS,GD,GL) VALUES ('test4', 13.3, 2345)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "8cdc8bf9cc4f2a466434800dd6c9b1cbe5bd1589d846dba7d8bdc4906176b1b9";
    const auto SCHEMA2_HASH_ECDB_MAP = "07423d2427c5cb77993d4569c71b23473bb691790b9c2a086768a5cd5f4c9c3f";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "5dfcae2814c747473d2e3d92fae39746d12fe3e1f4d54dd1444462c670eee54d";
    Test(
        "Delete derived class should be successful",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Base' modifier='None'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='GS' typeName='string' />
                        <ECProperty propertyName='GD' typeName='double' />
                        <ECProperty propertyName='GL' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Select values from edited schema",
        [&]()
            {
            //Following should not exist
            ASSERT_EQ(m_briefcase->Schemas().GetClass("TestSchema", "Sub"), nullptr);
            ASSERT_FALSE(m_briefcase->TableExists("ts_Sub"));

            //Following should exist
            ASSERT_TRUE(m_briefcase->TableExists("ts_Base"));
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Base"), nullptr);

            ASSERT_EQ(6, GetColumnCount("ts_Base")) << "After deleting subclass Foo";

            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(*m_briefcase, "SELECT FS, FD, FL FROM ts.Sub"));
            ASSERT_EQ(BE_SQLITE_ERROR, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT GS, GD, GL FROM ts.Base"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "test that the index definitions in ec_Index are cleaned up when a table is deleted",
        [&]()
            {
            Statement stmt;
            ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_briefcase, "SELECT count(*) FROM ec_Index i JOIN ec_Table t ON i.TableId=t.Id WHERE t.Name LIKE 'ts_Base' COLLATE NOCASE"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetSql();
            ASSERT_EQ(1, stmt.GetValueInt(0)) << stmt.GetSql();
            stmt.Finalize();
            }
    );

    const auto SCHEMA3_HASH_ECDB_SCHEMA = "99817469d21395253935dd4aeff611ea056e5bf3246a696157c10db99318e63f";
    Test(
        "Delete class containing ECDbMap CA should be successful",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                </ECSchema>)xml"
            );
            m_briefcase->GetTracker()->EnableTracking(false);
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            m_briefcase->GetTracker()->EnableTracking(true);
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA3_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA3_HASH_ECDB_SCHEMA); });
            }
    );

    Test("Base table should not exist", [&]() { ASSERT_FALSE(m_briefcase->TableExists("ts_Base")); });

    const auto SCHEMA4_HASH_ECDB_SCHEMA = "4fedea135a9fefe7df72ce3fa7ea5363421112156a2fe61dbd68fb3ffeadf0aa";
    const auto SCHEMA4_HASH_ECDB_MAP = "8e1e2d1afcd62125a23959c7ed52b8dfa4c8bada6ebeb810e9b415eeecc83312";
    const auto SCHEMA4_HASH_SQLITE_SCHEMA = "351e3a23da01181b90915602d96a7346fab461425708e7ddc36797c2e4340402";
    Test(
        "Add Class with SharedTable:SharedColumns is expected to be supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='4.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Base' modifier='None'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='GS' typeName='string' />
                        <ECProperty propertyName='GD' typeName='double' />
                        <ECProperty propertyName='GL' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA4_HASH_ECDB_SCHEMA, SCHEMA4_HASH_ECDB_MAP, SCHEMA4_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA4_HASH_ECDB_SCHEMA, SCHEMA4_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert and select values from edited schema",
        [&]()
            {
            //Following should not exist
            ASSERT_EQ(m_briefcase->Schemas().GetClass("TestSchema", "Sub"), nullptr);
            ASSERT_FALSE(m_briefcase->TableExists("ts_Sub"));

            //Following should exist
            ASSERT_TRUE(m_briefcase->TableExists("ts_Base"));
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Base"), nullptr);

            ASSERT_EQ(5, GetColumnCount("ts_Base")) << "After deleting all classes and readding base class";

            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Base(GS,GD,GL) VALUES ('test3', 44.32, 3344)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Base(GS,GD,GL) VALUES ('test4', 13.3, 2345)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();


            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA4_HASH_ECDB_SCHEMA, SCHEMA4_HASH_ECDB_MAP, SCHEMA4_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA4_HASH_ECDB_SCHEMA, SCHEMA4_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA5_HASH_ECDB_SCHEMA = "93c5cb374d14a205fc169693c4338a9f12d547b361f246e0f78f421a4bfbc182";
    const auto SCHEMA5_HASH_ECDB_MAP = "b0d641919115e7f8bc8b45ff315a1d848b2d58f3a0c8559c5db40b373f915b94";
    const auto SCHEMA5_HASH_SQLITE_SCHEMA = "5dfcae2814c747473d2e3d92fae39746d12fe3e1f4d54dd1444462c670eee54d";
    Test(
        "Adding new derived entity class is expected to be supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='5.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Base' modifier='None'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='GS' typeName='string' />
                        <ECProperty propertyName='GD' typeName='double' />
                        <ECProperty propertyName='GL' typeName='long' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub' modifier='None'>
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName='FS' typeName='string' />
                        <ECProperty propertyName='FD' typeName='double' />
                        <ECProperty propertyName='FL' typeName='long' />
                        <ECProperty propertyName='FI' typeName='int' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA5_HASH_ECDB_SCHEMA, SCHEMA5_HASH_ECDB_MAP, SCHEMA5_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA5_HASH_ECDB_SCHEMA, SCHEMA5_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert and select values from edited schema",
        [&]()
            {
            //Table should not exist
            ASSERT_FALSE(m_briefcase->TableExists("ts_Sub"));
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Sub"), nullptr);

            //Table should exist
            ASSERT_TRUE(m_briefcase->TableExists("ts_Base"));
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Base"), nullptr);

            ASSERT_EQ(6, GetColumnCount("ts_Base")) << "After readding subclass";
            ASSERT_EQ(5, GetColumnCount("ts_Base_Overflow")) << "After readding subclass";

            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT FS, FD, FL FROM ts.Sub"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT GS, GD, GL FROM ts.Base"));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA5_HASH_ECDB_SCHEMA, SCHEMA5_HASH_ECDB_MAP, SCHEMA5_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA5_HASH_ECDB_SCHEMA, SCHEMA5_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, Delete_Add_ECEntityClass_TPH_ShareColumns_TrackerNotDisabled)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "57557dd238919eee8d9449f2bfb9eb99375599970806377dc5287ecb38532f4f";
    const auto SCHEMA1_HASH_ECDB_MAP = "b0d641919115e7f8bc8b45ff315a1d848b2d58f3a0c8559c5db40b373f915b94";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "5dfcae2814c747473d2e3d92fae39746d12fe3e1f4d54dd1444462c670eee54d";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Base' modifier='None'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='GS' typeName='string' />
                        <ECProperty propertyName='GD' typeName='double' />
                        <ECProperty propertyName='GL' typeName='long' />
                    </ECEntityClass>
                    <ECEntityClass typeName='Sub' modifier='None'>
                        <BaseClass>Base</BaseClass>
                        <ECProperty propertyName='FS' typeName='string' />
                        <ECProperty propertyName='FD' typeName='double' />
                        <ECProperty propertyName='FL' typeName='long' />
                        <ECProperty propertyName='FI' typeName='int' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert values to initial schema",
        [&]()
            {
            //following table should exist.
            ASSERT_TRUE(m_briefcase->TableExists("ts_Base"));
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Base"), nullptr);

            //Following table should not exist
            ASSERT_NE(m_briefcase->Schemas().GetClass("TestSchema", "Sub"), nullptr);
            ASSERT_FALSE(m_briefcase->TableExists("ts_Sub"));

            ASSERT_EQ(6, GetColumnCount("ts_Base"));
            ASSERT_EQ(5, GetColumnCount("ts_Base_Overflow"));

            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub(FS,FD,FL,FI) VALUES ('test1', 1.3, 334, 1)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Sub(FS,FD,FL,FI) VALUES ('test2', 23.3, 234, 2)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Base(GS,GD,GL) VALUES ('test3', 44.32, 3344)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.Base(GS,GD,GL) VALUES ('test4', 13.3, 2345)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "8cdc8bf9cc4f2a466434800dd6c9b1cbe5bd1589d846dba7d8bdc4906176b1b9";
    const auto SCHEMA2_HASH_ECDB_MAP = "07423d2427c5cb77993d4569c71b23473bb691790b9c2a086768a5cd5f4c9c3f";
    const auto SCHEMA2_HASH_SQLITE_SCHEMA = "5dfcae2814c747473d2e3d92fae39746d12fe3e1f4d54dd1444462c670eee54d";
    Test(
        "Delete derived class should be successful",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                    <ECEntityClass typeName='Base' modifier='None'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00'>
                                <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                        </ECCustomAttributes>
                        <ECProperty propertyName='GS' typeName='string' />
                        <ECProperty propertyName='GD' typeName='double' />
                        <ECProperty propertyName='GL' typeName='long' />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Delete class containing ECDbMap CA should be successful",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='3.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />
                </ECSchema>)xml"
            );
            BeTest::SetFailOnAssert(false);
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            BeTest::SetFailOnAssert(true);
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA2_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddEnumAndEnumProperty)
    {
    Test(
        "import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "265b20a58b21291292b3042c66949eebc7ba96ea9e702a275e6391a29e54db4f";
            const auto SCHEMA_HASH_ECDB_MAP = "e377918ed948b322302bce119e12d5743ec050b4cf05a4d342610a392bb20ca5";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "b9441b9a819f951681d5d0ddf736bfbed218f9cc5a522264b2c5f3abb5a8fbbd";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEntityClass typeName="Foo" >
                        <ECProperty propertyName="Name" typeName="string"/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("AddEnumAndEnumProperty", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import edited schema with some changes",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "f9ba8ad7fa731ceec8f97967caa357b5904e06ab73c69acc7b8e862ed7ca4301";
            const auto SCHEMA_HASH_ECDB_MAP = "656af8d059ec7de2b174ee0e9a151526d9c1cd33ea5f3d8003bef08c09348c6b";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "423d2d5fa08b3dc3192e8f3eb9ce6308f43ab16d6dd759a7a530ba23a5ad2fd5";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEnumeration typeName="MyEnum" backingTypeName="int" isStrict="False">
                        <ECEnumerator value="0" displayLabel="On" />
                        <ECEnumerator value="1" displayLabel="Off" />
                    </ECEnumeration>
                    <ECEntityClass typeName="Foo" >
                        <ECProperty propertyName="Name" typeName="string"/>
                        <ECProperty propertyName="Status" typeName="MyEnum"/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check enumerators",
        [&]()
            {
            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
            ECClassCP fooClass = m_briefcase->Schemas().GetClass("TestSchema", "Foo");
            ASSERT_TRUE(fooClass != nullptr);
            ECPropertyCP enumProp = fooClass->GetPropertyP("Status");
            ASSERT_TRUE(enumProp != nullptr && enumProp->GetIsPrimitive());
            ECEnumerationCP actualEnum = enumProp->GetAsPrimitiveProperty()->GetEnumeration();
            ASSERT_TRUE(actualEnum != nullptr);
            ASSERT_STREQ("MyEnum", actualEnum->GetName().c_str());
            ASSERT_EQ(actualEnum, m_briefcase->Schemas().GetEnumeration("TestSchema", "MyEnum"));
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddingECEnumerationIntegerType)
    {
    Test(
        "import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "2c70cb8f4c31c5b78b660c6e4bcf8c95c0b52d303bcb1cd5c3dc3f4d9ec189e7";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False' displayLabel='Test1Display' description='Test1Desc'>
                        <ECEnumerator value = '0' displayLabel = 'txt' />
                        <ECEnumerator value = '1' displayLabel = 'bat' />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "import edited schema with some changes",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "766d13e1e8494446aa9c79341812f8b45343c89418ed8508ee78306fa5982e83";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False' displayLabel='Test2Display' description='Test2Desc'>
                        <ECEnumerator value = '0' displayLabel = 'txt' />
                        <ECEnumerator value = '1' displayLabel = 'bat' />
                        <ECEnumerator value = '2' displayLabel = 'exe' />
                        <ECEnumerator value = '3' displayLabel = 'dll' />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Check enumerators",
        [&]()
            {
            auto updatedEnum = m_briefcase->Schemas().GetEnumeration("TestSchema", "NonStrictEnum");
            ASSERT_TRUE(updatedEnum != nullptr);
            ASSERT_STREQ("Test2Display", updatedEnum->GetDisplayLabel().c_str());
            ASSERT_STREQ("Test2Desc", updatedEnum->GetDescription().c_str());
            ASSERT_EQ(false, updatedEnum->GetIsStrict());
            ASSERT_EQ(PRIMITIVETYPE_Integer, updatedEnum->GetType());

            auto assertEnumerator = [&](int32_t value, Utf8CP displayLabel)
                {
                ECEnumeratorCP newEnum = updatedEnum->FindEnumerator(value);
                ASSERT_TRUE(newEnum != nullptr);
                ASSERT_STREQ(displayLabel, newEnum->GetDisplayLabel().c_str());
                };

            assertEnumerator(0, "txt");
            assertEnumerator(1, "bat");
            assertEnumerator(2, "exe");
            assertEnumerator(3, "dll");
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AddingECEnumerationStringType)
    {
    Test(
        "import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "547468a9bd8ffe054a4be4bd15b8c995c880b541fb77c24024dcc9b26ee57d45";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEnumeration typeName='NonStrictEnum' backingTypeName='string' isStrict='False' displayLabel='Test1Display' description='Test1Desc'>
                        <ECEnumerator value = 't0' displayLabel = 'txt' />
                        <ECEnumerator value = 't1' displayLabel = 'bat' />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "import edited schema with some changes",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "3c9b8e301359d4c02a921fa0bcc8479ac455c616277e430a7d7e2a21c9ea8a31";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEnumeration typeName='NonStrictEnum' backingTypeName='string' isStrict='False' displayLabel='Test2Display' description='Test2Desc'>
                        <ECEnumerator value = 't0' displayLabel = 'txt' />
                        <ECEnumerator value = 't1' displayLabel = 'bat' />
                        <ECEnumerator value = 't2' displayLabel = 'exe' />
                        <ECEnumerator value = 't3' displayLabel = 'dll' />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Check enumerators",
        [&]()
            {
            auto updatedEnum = m_briefcase->Schemas().GetEnumeration("TestSchema", "NonStrictEnum");
            ASSERT_TRUE(updatedEnum != nullptr);
            ASSERT_STREQ("Test2Display", updatedEnum->GetDisplayLabel().c_str());
            ASSERT_STREQ("Test2Desc", updatedEnum->GetDescription().c_str());
            ASSERT_EQ(false, updatedEnum->GetIsStrict());
            ASSERT_EQ(PRIMITIVETYPE_String, updatedEnum->GetType());

            auto assertEnumerator = [&](Utf8CP value, Utf8CP displayLabel)
                {
                ECEnumeratorCP newEnum = updatedEnum->FindEnumerator(value);
                ASSERT_TRUE(newEnum != nullptr);
                ASSERT_STREQ(displayLabel, newEnum->GetDisplayLabel().c_str());
                };

            assertEnumerator("t0", "txt");
            assertEnumerator("t1", "bat");
            assertEnumerator("t2", "exe");
            assertEnumerator("t3", "dll");
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateECEnumerationFromStrictToNonStrictAndUpdateEnumerators)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "b34189bc326584914f8d9aaf398519e635673545115d3a535c420cbff75bec4b";
    Test(
        "import initial schema",
        [&]()
            {
            
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='True' displayLabel='Test1Display' description='Test1Desc'>
                        <ECEnumerator value = '0' displayLabel = 'txt' />
                        <ECEnumerator value = '1' displayLabel = 'bat' />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Modifying enumerator values is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False' displayLabel='Test2Display' description='Test2Desc'>
                        <ECEnumerator value = '10' displayLabel = 'txt1' />
                        <ECEnumerator value = '11' displayLabel = 'bat1' />
                        <ECEnumerator value = '12' displayLabel = 'exe1' />
                        <ECEnumerator value = '13' displayLabel = 'dll1' />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateECEnumerationFromUnStrictToStrict)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "2c70cb8f4c31c5b78b660c6e4bcf8c95c0b52d303bcb1cd5c3dc3f4d9ec189e7";
    Test(
        "import initial schema",
        [&]()
            {
            
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False' displayLabel='Test1Display' description='Test1Desc'>
                        <ECEnumerator value = '0' displayLabel = 'txt' />
                        <ECEnumerator value = '1' displayLabel = 'bat' />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Cannot change IsStrict from false to true",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='True' displayLabel='Test1Display' description='Test1Desc'>
                        <ECEnumerator value = '0' displayLabel = 'txt' />
                        <ECEnumerator value = '1' displayLabel = 'bat' />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, ChangeECEnumeratorValue)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "772b43e4b4abd354409de2305ea61e8f9145044b8c3350be42d3b44197fb731e";
    Test(
        "import initial schema",
        [&]()
            {
            
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                        <ECEnumerator name="On" value="0" />
                        <ECEnumerator name="Off" value="1" />
                    </ECEnumeration>
                    <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                        <ECEnumerator name="On" value="On" />
                        <ECEnumerator name="Off" value="Off" />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("ChangeECEnumeratorValue", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "First change of enumerator values",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                        <ECEnumerator name="On" value="0" />
                        <ECEnumerator name="Off" value="2" />
                    </ECEnumeration>
                    <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                        <ECEnumerator name="On" value="On" />
                        <ECEnumerator name="Off" value="Off" />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Second change of enumerator values",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                        <ECEnumerator name="On" value="0" />
                        <ECEnumerator name="Off" value="1" />
                    </ECEnumeration>
                    <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                        <ECEnumerator name="On" value="Turn On" />
                        <ECEnumerator name="Off" value="Off" />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, ModifyEnumeratorNameInPre32ECSchema)
    {
    Test(
        "import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "52865b5f8ac7ba418068be2e36402f012eb7a11ad40d108d1df12809732442b1";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                        <ECEnumerator value="0" />
                        <ECEnumerator value="1" />
                    </ECEnumeration>
                    <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                        <ECEnumerator value="On" />
                        <ECEnumerator value="Off" />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("ModifyEnumeratorNameInPre32ECSchema", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA); });
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "33581d7a45f2bca32a89dfcf8c71a49033a0c90f0d0e10385ee3bf4cdb23929c";
    Test(
        "When coming from 3.1 schema, an enumerator name change is valid",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                        <ECEnumerator name="On" value="0" />
                        <ECEnumerator name="Off" value="1" />
                    </ECEnumeration>
                    <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                        <ECEnumerator name="An" value="On" />
                        <ECEnumerator name="Aus" value="Off" />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Once the name was changed after the 3.1 conversion, it cannot be changed anymore",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                        <ECEnumerator name="TurnOn" value="0" />
                        <ECEnumerator name="TurnOff" value="1" />
                    </ECEnumeration>
                    <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                        <ECEnumerator name="On" value="On" />
                        <ECEnumerator name="Off" value="Off" />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "9ed0e7e9a5520a1b3d7b1256f9585c86eb1fe0ba41dec0296d833776470f406e";
    Test(
        "start with EC3.2 enum which should never allow to rename an enumerator",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                        <ECEnumerator name="IntEnum0" value="0" />
                        <ECEnumerator name="IntEnum1" value="1" />
                    </ECEnumeration>
                    <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                        <ECEnumerator name="On" value="On" />
                        <ECEnumerator name="Off" value="Off" />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("ModifyEnumeratorNameInPre32ECSchema", schema));
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Even if the enumerator name is the default EC3.2 conversion name, the change is not valid",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                        <ECEnumerator name="On" value="0" />
                        <ECEnumerator name="Off" value="1" />
                    </ECEnumeration>
                    <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                        <ECEnumerator name="On" value="On" />
                        <ECEnumerator name="Off" value="Off" />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Once the name was changed away from the EC3.2 conversion default name, it cannot be changed anymore",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                        <ECEnumerator name="TurnOn" value="0" />
                        <ECEnumerator name="TurnOff" value="1" />
                    </ECEnumeration>
                    <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                        <ECEnumerator name="On" value="On" />
                        <ECEnumerator name="Off" value="Off" />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateECEnumerationAddDeleteEnumerators)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "b34189bc326584914f8d9aaf398519e635673545115d3a535c420cbff75bec4b";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='True' displayLabel='Test1Display' description='Test1Desc'>
                        <ECEnumerator value = '0' displayLabel = 'txt' />
                        <ECEnumerator value = '1' displayLabel = 'bat' />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Cannot change Strict Enum (Only Adding new properties allowed)",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                    <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='True' displayLabel='Test2Display' description='Test2Desc'>
                        <ECEnumerator value = '10' displayLabel = 'txt1' />
                        <ECEnumerator value = '11' displayLabel = 'bat1' />
                        <ECEnumerator value = '12' displayLabel = 'exe1' />
                        <ECEnumerator value = '13' displayLabel = 'dll1' />
                    </ECEnumeration>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, PropertyCategoryAddUpdateDelete)
    {
    Test(
        "import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "c45c3c0f7214348837c5767840c56ce6150976bc03c184de9e58147bf0fe9536";
            const auto SCHEMA_HASH_ECDB_MAP = "514167887c868d4f20c05038b2d21989a511a4ecdffdce0f2b14b4b7da5acb05";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "9ba4d9ec2205473e823ffd7fb10ba60f98f1e0e2224c38549b82c1918c08c9f9";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <PropertyCategory typeName="C1" description="C1" displayLabel="C1" priority="1" />
                    <PropertyCategory typeName="C2" description="C2" displayLabel="C2" priority="2" />
                    <PropertyCategory typeName="C3" description="C3" displayLabel="C3" priority="3" />
                    <PropertyCategory typeName="C5" description="C5" displayLabel="C5" priority="5" />
                    <ECEntityClass typeName="Foo" >
                        <ECProperty propertyName="P1" typeName="double" category="C1" />
                        <ECProperty propertyName="P2" typeName="double" category="C2" />
                        <ECProperty propertyName="P3" typeName="double" category="C3" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("getpropertycategories", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Assert properties in the initial schema",
        [&]()
            {
            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            PropertyCategoryCP c1 = m_briefcase->Schemas().GetPropertyCategory("Schema1", "C1");
            ASSERT_TRUE(c1 != nullptr);
            ASSERT_STREQ("C1", c1->GetName().c_str());
            ASSERT_EQ(1, (int) c1->GetPriority());

            PropertyCategoryCP c2 = m_briefcase->Schemas().GetPropertyCategory("Schema1", "C2");
            ASSERT_TRUE(c1 != nullptr);
            ASSERT_STREQ("C2", c2->GetName().c_str());
            ASSERT_EQ(2, (int) c2->GetPriority());

            PropertyCategoryCP c3 = m_briefcase->Schemas().GetPropertyCategory("Schema1", "C3");
            ASSERT_TRUE(c1 != nullptr);
            ASSERT_STREQ("C3", c3->GetName().c_str());
            ASSERT_EQ(3, (int) c3->GetPriority());

            PropertyCategoryCP c5 = m_briefcase->Schemas().GetPropertyCategory("Schema1", "C5");
            ASSERT_TRUE(c5 != nullptr);
            ASSERT_STREQ("C5", c5->GetName().c_str());
            ASSERT_EQ(5, (int) c5->GetPriority());

            ECSchemaCP schema1 = m_briefcase->Schemas().GetSchema("Schema1", false);
            ASSERT_TRUE(schema1 != nullptr);

            ECClassCP fooClass = m_briefcase->Schemas().GetClass("Schema1", "Foo");
            ASSERT_TRUE(fooClass != nullptr);
            ASSERT_EQ(4, schema1->GetPropertyCategoryCount());

            ECPropertyCP p1 = fooClass->GetPropertyP("P1");
            ASSERT_TRUE(p1 != nullptr);
            ASSERT_TRUE(p1->GetCategory() != nullptr);
            ASSERT_STREQ("C1", p1->GetCategory()->GetName().c_str());

            ECPropertyCP p2 = fooClass->GetPropertyP("P2");
            ASSERT_TRUE(p2 != nullptr);
            ASSERT_TRUE(p2->GetCategory() != nullptr);
            ASSERT_STREQ("C2", p2->GetCategory()->GetName().c_str());

            ECPropertyCP p3 = fooClass->GetPropertyP("P3");
            ASSERT_TRUE(p3 != nullptr);
            ASSERT_TRUE(p3->GetCategory() != nullptr);
            ASSERT_STREQ("C3", p3->GetCategory()->GetName().c_str());
            }
    );

    const auto SCHEMA1_HASH_ECDB_MAP = "680aeba74606e5052fa275fc3401baa3a1e84f19128eb854f37d68554a4d64d6";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "745bdc17145801a827c5404d318d1cd8903b73f2bf163d46c24aee86f173f7c7";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "0a4fb1822f4492fd9954a3d68283f59fc30b76698bf5be26ba5c1a8478f5e367";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="2.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <PropertyCategory typeName="C1" description="C1" displayLabel="C1" priority="1" />
                    <PropertyCategory typeName="C2" description="C2" displayLabel="C2" priority="2" />
                    <PropertyCategory typeName="C3" description="C3" displayLabel="C3" priority="3" />
                    <PropertyCategory typeName="C4" description="C4" displayLabel="C4" priority="4" />
                    <PropertyCategory typeName="C5" description="C5" displayLabel="C5" priority="5" />
                    <ECEntityClass typeName="Foo" >
                        <ECProperty propertyName="P1" typeName="double" category="C4" />
                        <ECProperty propertyName="P2" typeName="double" />
                        <ECProperty propertyName="P3" typeName="double" category="C3" />
                        <ECProperty propertyName="P4" typeName="double" category="C1" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Assert properties in the edited schema",
        [&]()
            {
            ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

            PropertyCategoryCP c1 = m_briefcase->Schemas().GetPropertyCategory("Schema1", "C1");
            ASSERT_TRUE(c1 != nullptr);
            ASSERT_STREQ("C1", c1->GetName().c_str());
            ASSERT_EQ(1, (int) c1->GetPriority());

            PropertyCategoryCP c2 = m_briefcase->Schemas().GetPropertyCategory("Schema1", "C2");
            ASSERT_TRUE(c1 != nullptr);
            ASSERT_STREQ("C2", c2->GetName().c_str());
            ASSERT_EQ(2, (int) c2->GetPriority());

            PropertyCategoryCP c3 = m_briefcase->Schemas().GetPropertyCategory("Schema1", "C3");
            ASSERT_TRUE(c1 != nullptr);
            ASSERT_STREQ("C3", c3->GetName().c_str());
            ASSERT_EQ(3, (int) c3->GetPriority());

            PropertyCategoryCP c4 = m_briefcase->Schemas().GetPropertyCategory("Schema1", "C4");
            ASSERT_TRUE(c4 != nullptr);
            ASSERT_STREQ("C4", c4->GetName().c_str());
            ASSERT_EQ(4, (int) c4->GetPriority());

            PropertyCategoryCP c5 = m_briefcase->Schemas().GetPropertyCategory("Schema1", "C5");
            ASSERT_TRUE(c5 != nullptr);
            ASSERT_STREQ("C5", c5->GetName().c_str());
            ASSERT_EQ(5, (int) c5->GetPriority());

            ECSchemaCP schema1 = m_briefcase->Schemas().GetSchema("Schema1", false);
            ASSERT_TRUE(schema1 != nullptr);

            ECClassCP fooClass = m_briefcase->Schemas().GetClass("Schema1", "Foo");
            ASSERT_TRUE(fooClass != nullptr);
            ASSERT_EQ(5, schema1->GetPropertyCategoryCount());

            ECPropertyCP p1 = fooClass->GetPropertyP("P1");
            ASSERT_TRUE(p1 != nullptr);
            ASSERT_TRUE(p1->GetCategory() != nullptr);
            ASSERT_STREQ("C4", p1->GetCategory()->GetName().c_str());

            ECPropertyCP p2 = fooClass->GetPropertyP("P2");
            ASSERT_TRUE(p2 != nullptr);
            ASSERT_TRUE(p2->GetCategory() == nullptr);

            ECPropertyCP p3 = fooClass->GetPropertyP("P3");
            ASSERT_TRUE(p3 != nullptr);
            ASSERT_TRUE(p3->GetCategory() != nullptr);
            ASSERT_STREQ("C3", p3->GetCategory()->GetName().c_str());;

            ECPropertyCP p4 = fooClass->GetPropertyP("P4");
            ASSERT_TRUE(p4 != nullptr);
            ASSERT_TRUE(p4->GetCategory() != nullptr);
            ASSERT_STREQ("C1", p4->GetCategory()->GetName().c_str());
            }
    );

    Test(
        "Delete a Category",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "2f339962b84b8e2eae98a4572b85d1104f5fdfb22ad3aea05c6c775fb7645f41";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="3.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <PropertyCategory typeName="C1" description="C1" displayLabel="C1" priority="1" />
                    <PropertyCategory typeName="C2" description="C2" displayLabel="C2" priority="2" />
                    <PropertyCategory typeName="C3" description="C3" displayLabel="C3" priority="3" />
                    <PropertyCategory typeName="C4" description="C4" displayLabel="C4" priority="4" />
                    <ECEntityClass typeName="Foo" >
                        <ECProperty propertyName="P1" typeName="double" category="C4" />
                        <ECProperty propertyName="P2" typeName="double" />
                        <ECProperty propertyName="P3" typeName="double" category="C3" />
                        <ECProperty propertyName="P4" typeName="double" category="C1" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, PropertyCategoryDelete)
    {
    const auto SCHEMA1_HASH_ECDB_MAP = "680aeba74606e5052fa275fc3401baa3a1e84f19128eb854f37d68554a4d64d6";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "745bdc17145801a827c5404d318d1cd8903b73f2bf163d46c24aee86f173f7c7";
    Test(
        "import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "0bf11fb1ab30ee77f896a0460071a736e04fae1abbe4d1639d8179f27213799a";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <PropertyCategory typeName="C1" description="C1" displayLabel="C1" priority="1" />
                    <PropertyCategory typeName="C2" description="C2" displayLabel="C2" priority="2" />
                    <PropertyCategory typeName="C3" description="C3" displayLabel="C3" priority="3" />
                    <PropertyCategory typeName="C4" description="C4" displayLabel="C4" priority="4" />
                    <ECEntityClass typeName="Foo" >
                        <ECProperty propertyName="P1" typeName="double" category="C1" />
                        <ECProperty propertyName="P2" typeName="double" category="C2" />
                        <ECProperty propertyName="P3" typeName="double" category="C3" />
                        <ECProperty propertyName="P4" typeName="double" category="C4" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("getpropertycategories", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "PropertyCategory deletion should work if there are no dangling references",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "ea07aca8062e1563c12f43cbd662c6e74d936dff282bbb5dbd5a6ee707bc250f";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="2.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <PropertyCategory typeName="C1" description="C1" displayLabel="C1" priority="1" />
                    <PropertyCategory typeName="C2" description="C2" displayLabel="C2" priority="2" />
                    <PropertyCategory typeName="C3" description="C3" displayLabel="C3" priority="3" />
                    <PropertyCategory typeName="C5" description="C5" displayLabel="C5" priority="5" />
                    <ECEntityClass typeName="Foo" >
                        <ECProperty propertyName="P1" typeName="double" category="C1" />
                        <ECProperty propertyName="P2" typeName="double" category="C2"/>
                        <ECProperty propertyName="P3" typeName="double" category="C3" />
                        <ECProperty propertyName="P4" typeName="double" category="C5" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, LegalPropertyCategoryDeleteWithDoNotFailFlag)
    {
    const auto SCHEMA1_HASH_ECDB_MAP = "680aeba74606e5052fa275fc3401baa3a1e84f19128eb854f37d68554a4d64d6";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "745bdc17145801a827c5404d318d1cd8903b73f2bf163d46c24aee86f173f7c7";
    Test(
        "Import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "0bf11fb1ab30ee77f896a0460071a736e04fae1abbe4d1639d8179f27213799a";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <PropertyCategory typeName="C1" description="C1" displayLabel="C1" priority="1" />
                    <PropertyCategory typeName="C2" description="C2" displayLabel="C2" priority="2" />
                    <PropertyCategory typeName="C3" description="C3" displayLabel="C3" priority="3" />
                    <PropertyCategory typeName="C4" description="C4" displayLabel="C4" priority="4" />
                    <ECEntityClass typeName="Foo" >
                        <ECProperty propertyName="P1" typeName="double" category="C1" />
                        <ECProperty propertyName="P2" typeName="double" category="C2" />
                        <ECProperty propertyName="P3" typeName="double" category="C3" />
                        <ECProperty propertyName="P4" typeName="double" category="C4" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("getpropertycategories", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "PropertyCategory deletion should work if there are no dangling references",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "ea07aca8062e1563c12f43cbd662c6e74d936dff282bbb5dbd5a6ee707bc250f";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="2.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <PropertyCategory typeName="C1" description="C1" displayLabel="C1" priority="1" />
                    <PropertyCategory typeName="C2" description="C2" displayLabel="C2" priority="2" />
                    <PropertyCategory typeName="C3" description="C3" displayLabel="C3" priority="3" />
                    <PropertyCategory typeName="C5" description="C5" displayLabel="C5" priority="5" />
                    <ECEntityClass typeName="Foo" >
                        <ECProperty propertyName="P1" typeName="double" category="C1" />
                        <ECProperty propertyName="P2" typeName="double" category="C2"/>
                        <ECProperty propertyName="P3" typeName="double" category="C3" />
                        <ECProperty propertyName="P4" typeName="double" category="C5" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::DoNotFailForDeletionsOrModifications));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import edited schema with some changes",
        [&]()
            {
            ECSchemaCP schema = m_briefcase->Schemas().GetSchema("Schema1");
            ASSERT_TRUE(schema != nullptr);

            PropertyCategoryCP cat = schema->GetPropertyCategoryCP("C4");
            ASSERT_TRUE(cat == nullptr) << "C4";
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, IllegalPropertyCategoryDeleteWithDoNotFailFlag)
    {
    Test(
        "initial schema1 setup should succeed",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "4697b8cef1943d01cf8f188b4a74a8542f61fa2a346fd0f9fc41120735e10412";
            const auto SCHEMA_HASH_ECDB_MAP = "cc05ef847358155e0fb798592b7674399d51317c2d023cd237ff3abcc26a43a3";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "dd425fde9290efc7080acb828fb87a371d83f2c073c9f246a301294d4253b5a2";

            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <PropertyCategory typeName="C1" description="C1" displayLabel="C1" priority="1" />
                    <PropertyCategory typeName="C2" description="C2" displayLabel="C2" priority="2" />
                    <ECEntityClass typeName="Foo">
                        <ECProperty propertyName="P1" typeName="double" category="C2" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("getpropertycategories", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    
    const auto SCHEMA1_HASH_ECDB_MAP = "1b1ae0f5665cd383b79513c33a1ae6f0dab45115c58f7dc2d047015f91f80444";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "ae9a7a553d09613c7f8cf5711dca7a996af71202254deca5cdddde9df2c229ce";
    Test(
        "importing second schema should succeed",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "54f7e30f2a27972d4aebc9425545cf720d05536da5565201a4f9a71d9a4bcf86";

            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema2" alias="s2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="Schema1" version="1.0" alias="s1" />
                    <ECEntityClass typeName="Foo">
                        <ECProperty propertyName="P2" typeName="double" category="s1:C2" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Illegal Property Category should be ignored when DoNotFailForDeletionsOrModifications flag is set",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "aeafa2da8beda0edc3403bf237f8b08dba5016c0f4848f6a1e538b035b4b76b6";

            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="2.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <PropertyCategory typeName="C1" description="C1" displayLabel="C1" priority="1" />
                    <ECEntityClass typeName="Foo">
                        <ECProperty propertyName="P1" typeName="double" category="C1" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::DoNotFailForDeletionsOrModifications));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check property",
        [&]()
            {
            ECSchemaCP schema = m_briefcase->Schemas().GetSchema("Schema1");
            ASSERT_TRUE(schema != nullptr);

            PropertyCategoryCP cat = schema->GetPropertyCategoryCP("C2");
            ASSERT_TRUE(cat != nullptr) << "C2";
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, PropertyCategoryDeleteReferencedFails)
    {
    Test(
        "initial schema setup should succeed",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "4697b8cef1943d01cf8f188b4a74a8542f61fa2a346fd0f9fc41120735e10412";
            const auto SCHEMA_HASH_ECDB_MAP = "cc05ef847358155e0fb798592b7674399d51317c2d023cd237ff3abcc26a43a3";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "dd425fde9290efc7080acb828fb87a371d83f2c073c9f246a301294d4253b5a2";

            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <PropertyCategory typeName="C1" description="C1" displayLabel="C1" priority="1" />
                    <PropertyCategory typeName="C2" description="C2" displayLabel="C2" priority="2" />
                    <ECEntityClass typeName="Foo">
                        <ECProperty propertyName="P1" typeName="double" category="C2" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("getpropertycategories", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "54f7e30f2a27972d4aebc9425545cf720d05536da5565201a4f9a71d9a4bcf86";
    const auto SCHEMA1_HASH_ECDB_MAP = "1b1ae0f5665cd383b79513c33a1ae6f0dab45115c58f7dc2d047015f91f80444";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "ae9a7a553d09613c7f8cf5711dca7a996af71202254deca5cdddde9df2c229ce";
    Test(
        "importing second schema should succeed",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema2" alias="s2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="Schema1" version="1.0" alias="s1" />
                    <ECEntityClass typeName="Foo">
                        <ECProperty propertyName="P2" typeName="double" category="s1:C2" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "deleting category while a property still references it should be an error",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <PropertyCategory typeName="C1" description="C1" displayLabel="C1" priority="1" />
                    <ECEntityClass typeName="Foo">
                        <ECProperty propertyName="P1" typeName="double" category="C1" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "deleting category and removing referencing properties should succeed",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "26920616ab580d49d92dac00ed4e17be79b013eb2baab8dc0f13c4c38edb8ace";
            const auto SCHEMA_HASH_ECDB_MAP = "1b1ae0f5665cd383b79513c33a1ae6f0dab45115c58f7dc2d047015f91f80444";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "ae9a7a553d09613c7f8cf5711dca7a996af71202254deca5cdddde9df2c229ce";

            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema2" alias="s2" version="1.1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="Schema1" version="1.0" alias="s1" />
                    <ECEntityClass typeName="Foo">
                        <ECProperty propertyName="P2" typeName="double" category="s1:C1" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// deleting and removing reference doesn't work for some reason?!
TEST_F(SchemaSyncTestFixture, PropertyCategoryOverwriteDeleteReferencedFails)
    {
    // Same as previous but Import final schemas in different order and overwrite the previous version by
    // using the same version
    Test(
        "initial schema setup should succeed",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "4697b8cef1943d01cf8f188b4a74a8542f61fa2a346fd0f9fc41120735e10412";
            const auto SCHEMA_HASH_ECDB_MAP = "cc05ef847358155e0fb798592b7674399d51317c2d023cd237ff3abcc26a43a3";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "dd425fde9290efc7080acb828fb87a371d83f2c073c9f246a301294d4253b5a2";

            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <PropertyCategory typeName="C1" description="C1" displayLabel="C1" priority="1" />
                    <PropertyCategory typeName="C2" description="C2" displayLabel="C2" priority="2" />
                    <ECEntityClass typeName="Foo">
                        <ECProperty propertyName="P1" typeName="double" category="C2" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("getpropertycategories", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "b04d8d36bdd856e54f39355a093e73bca97f1d5dba387f79329c84d840630c62";
    const auto SCHEMA1_HASH_ECDB_MAP = "cc05ef847358155e0fb798592b7674399d51317c2d023cd237ff3abcc26a43a3";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "dd425fde9290efc7080acb828fb87a371d83f2c073c9f246a301294d4253b5a2";
    Test(
        "importing second schema should succeed",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <PropertyCategory typeName="C1" description="C1" displayLabel="C1" priority="1" />
                    <ECEntityClass typeName="Foo">
                        <ECProperty propertyName="P1" typeName="double" category="C1" />
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    auto schemas = {
        SchemaItem(
            R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema schemaName="Schema2" alias="s2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="Schema1" version="1.0" alias="s1" />
                <ECEntityClass typeName="Foo">
                    <ECProperty propertyName="P2" typeName="double" category="s1:C2" />
                </ECEntityClass>
            </ECSchema>)xml"
        ),
        SchemaItem(
            R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema schemaName="Schema2" alias="s2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="Schema1" version="1.0" alias="s1" />
                <ECEntityClass typeName="Foo">
                    <ECProperty propertyName="P2" typeName="double" category="s1:C1" />
                </ECEntityClass>
            </ECSchema>)xml"
        )
    };

    Test(
        "deleting category while a property still references it should be an error",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(*schemas.begin()));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "deleting category and removing referencing properties should succeed",
        [&]()
            {
            // const auto SCHEMA_HASH_ECDB_SCHEMA = "b04d8d36bdd856e54f39355a093e73bca97f1d5dba387f79329c84d840630c62";
            // const auto SCHEMA_HASH_ECDB_MAP = "cc05ef847358155e0fb798592b7674399d51317c2d023cd237ff3abcc26a43a3";
            // const auto SCHEMA_HASH_SQLITE_SCHEMA = "dd425fde9290efc7080acb828fb87a371d83f2c073c9f246a301294d4253b5a2";

            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchemas(*m_briefcase, schemas, SchemaManager::SchemaImportOptions::None, m_schemaChannel->GetChannelUri())); // why is this an error?
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            // CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            // m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, PropertyCategory)
    {
    auto assertCategory = [](ECSchemaCR schema, Utf8CP name, Utf8CP displayLabel, Utf8CP description, uint32_t priority)
        {
        PropertyCategoryCP cat = schema.GetPropertyCategoryCP(name);
        ASSERT_TRUE(cat != nullptr) << name;
        EXPECT_STREQ(name, cat->GetName().c_str()) << name;
        EXPECT_EQ(priority, cat->GetPriority()) << name;
        if (Utf8String::IsNullOrEmpty(displayLabel))
            EXPECT_FALSE(cat->GetIsDisplayLabelDefined()) << name;
        else
            EXPECT_STREQ(displayLabel, cat->GetDisplayLabel().c_str()) << name;

        if (Utf8String::IsNullOrEmpty(description))
            description = "";

        EXPECT_STREQ(description, cat->GetDescription().c_str()) << name;
        };

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "6bd56dc507cb2ba339b2a8713929e70523f83f0c2923a02f0811593b3bf8f15c";
    Test(
        "Import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <PropertyCategory typeName="C1" description="My Category 1" displayLabel="Category 1" priority="1" />
                        <PropertyCategory typeName="C2" description="My Category 2" displayLabel="Category 2" priority="2" />
                        <PropertyCategory typeName="C3" description="My Category 3" displayLabel="Category 3" priority="3" />
                        <PropertyCategory typeName="C4" description="My Category 4" displayLabel="Category 4" priority="4" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupgrade_PropertyCategory", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Check schema categories",
        [&]()
            {
            ECSchemaCP schema = m_briefcase->Schemas().GetSchema("Schema1");
            ASSERT_NE(nullptr, schema);

            assertCategory(*schema, "C1", "Category 1", "My Category 1", 1);
            assertCategory(*schema, "C2", "Category 2", "My Category 2", 2);
            assertCategory(*schema, "C3", "Category 3", "My Category 3", 3);
            assertCategory(*schema, "C4", "Category 4", "My Category 4", 4);
            }
    );

    Test(
        "Deleting a category is supported",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "4dd9fefd9904dcfae6daaabc9bafc5a35577c15a7cf51bc6a58bc4899947c03b";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <PropertyCategory typeName="C1" description="My Category 1" displayLabel="Category 1" priority="1" />
                    <PropertyCategory typeName="C2" description="My Category 2" displayLabel="Category 2" priority="2" />
                    <PropertyCategory typeName="C4" description="My Category 4" displayLabel="Category 4" priority="4" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Adding a category is supported",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "81b79056177d032b15429d8870bf65babf3a6b14ec9e081f9c86ed5976ea4102";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <PropertyCategory typeName="C1" description="My Category 1" displayLabel="Category 1" priority="1" />
                    <PropertyCategory typeName="C2" description="My Category 2" displayLabel="Category 2" priority="2" />
                    <PropertyCategory typeName="C3" description="My Category 3" displayLabel="Category 3" priority="3" />
                    <PropertyCategory typeName="C4" description="My Category 4" displayLabel="Category 4" priority="4" />
                    <PropertyCategory typeName="C5" description="My Category 5" displayLabel="Category 5" priority="5" />
                    <PropertyCategory typeName="C6" description="My Category 6" displayLabel="Category 6" priority="6" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Check schema categories after deleting and adding",
        [&]()
            {
            ECSchemaCP schema = m_briefcase->Schemas().GetSchema("Schema1");
            ASSERT_NE(nullptr, schema);

            assertCategory(*schema, "C1", "Category 1", "My Category 1", 1);
            assertCategory(*schema, "C2", "Category 2", "My Category 2", 2);
            assertCategory(*schema, "C3", "Category 3", "My Category 3", 3);
            assertCategory(*schema, "C4", "Category 4", "My Category 4", 4);
            assertCategory(*schema, "C5", "Category 5", "My Category 5", 5);
            assertCategory(*schema, "C6", "Category 6", "My Category 6", 6);
            }
    );

    Test(
        "Modifying a category is supported",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "b0d12529f36a9a57412eba5d05db96ca8bb152aef11adfa73433d7d1cb0333e6";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <PropertyCategory typeName="C1" displayLabel="Category 1" priority="1" />
                    <PropertyCategory typeName="C2" description="My Category 2" priority="2" />
                    <PropertyCategory typeName="C3" description="My nice Category 3" displayLabel="Category 3" priority="3" />
                    <PropertyCategory typeName="C4" description="My Category 4" displayLabel="Nice Category 4" priority="4" />
                    <PropertyCategory typeName="C5" description="My Category 5" displayLabel="Category 5" priority="50" />
                    <PropertyCategory typeName="C6" description="My Category 6" displayLabel="Category 6" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Check schema categories after modifying",
        [&]()
            {
            ECSchemaCP schema = m_briefcase->Schemas().GetSchema("Schema1");
            ASSERT_NE(nullptr, schema);

            assertCategory(*schema, "C1", "Category 1", nullptr, 1);
            assertCategory(*schema, "C2", nullptr, "My Category 2", 2);
            assertCategory(*schema, "C3", "Category 3", "My nice Category 3", 3);
            assertCategory(*schema, "C4", "Nice Category 4", "My Category 4", 4);
            assertCategory(*schema, "C5", "Category 5", "My Category 5", 50);
            //deleting the priority from the schema amounts to setting it to 0
            assertCategory(*schema, "C6", "Category 6", "My Category 6", 0);
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UnitSystems)
    {
    auto assertUnitSystem = [] (ECSchemaCR schema, Utf8CP name, Utf8CP displayLabel, Utf8CP description)
        {
        UnitSystemCP system = schema.GetUnitSystemCP(name);
        ASSERT_TRUE(system != nullptr) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(name, system->GetName().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(displayLabel, system->GetDisplayLabel().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(description, system->GetDescription().c_str()) << schema.GetFullSchemaName() << ":" << name;
        };

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "6d3aadb0464230000d9ba16f46ea51064257e12e7fa5ac5378d159066d72eb5c";
    Test(
        "Import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                    <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                    <UnitSystem typeName="UNUSEDU" displayLabel="Unused" description="Unused" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupgrade_unitsystems", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Check schema values",
        [&]()
            {
            ECSchemaCP schema = m_briefcase->Schemas().GetSchema("Schema1");
            ASSERT_TRUE(schema != nullptr);

            assertUnitSystem(*schema, "METRIC", "Metric", "Metric Units of measure");
            assertUnitSystem(*schema, "IMPERIAL", "Imperial", "Units of measure from the British Empire");
            assertUnitSystem(*schema, "UNUSEDU", "Unused", "Unused");
            }
    );

    Test(
        "Deleting a unit system is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                    <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Renaming a UnitSystem is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                    <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                    <UnitSystem typeName="UNUSEDUS" displayLabel="Unused" description="Unused" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Adding a unit system, modifying display label and description of unit system, removing display label and description of unit system",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "e683b3ba3e15e0d340badaba3d9e0d787575f19fc4103947b7af23038d83d125";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.3" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <UnitSystem typeName="METRIC" />
                    <UnitSystem typeName="IMPERIAL" displayLabel="ImperialSystem" description="Units of measure from the British Empire." />
                    <UnitSystem typeName="UNUSEDU" displayLabel="Unused" description="Unused" />
                    <UnitSystem typeName="MyLocalOne" displayLabel="My Local one"  />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Check schema values after modifications",
        [&]()
            {
            ECSchemaCP schema = m_briefcase->Schemas().GetSchema("Schema1");
            ASSERT_TRUE(schema != nullptr);

            assertUnitSystem(*schema, "METRIC", "METRIC", "");
            assertUnitSystem(*schema, "IMPERIAL", "ImperialSystem", "Units of measure from the British Empire.");
            assertUnitSystem(*schema, "UNUSEDU", "Unused", "Unused");
            assertUnitSystem(*schema, "MyLocalOne", "My Local one", "");
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, Phenomena)
    {
    auto assertPhenomenon = [] (ECSchemaCR schema, Utf8CP name, Utf8CP displayLabel, Utf8CP description, Utf8CP definition)
        {
        PhenomenonCP phen = schema.GetPhenomenonCP(name);
        ASSERT_TRUE(phen != nullptr) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(name, phen->GetName().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(definition, phen->GetDefinition().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(displayLabel, phen->GetDisplayLabel().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(description, phen->GetDescription().c_str()) << schema.GetFullSchemaName() << ":" << name;
        };

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "ccaf6d9238bf9dc822867ba374bebdb8fa62b9fcf58b556cdd5c3afda0ed5604";
    Test(
        "Import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                    <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupgrade_phenomena", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Check schema phenomenon",
        [&]()
            {
            ECSchemaCP schema = m_briefcase->Schemas().GetSchema("Schema1");
            ASSERT_TRUE(schema != nullptr);

            assertPhenomenon(*schema, "AREA", "Area", "", "LENGTH*LENGTH");
            assertPhenomenon(*schema, "UNUSEDP", "Unused", "", "LENGTH*LENGTH");
            }
    );

    Test(
        "Deleting a phenomenon is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Renaming a Phenomenon is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                    <Phenomenon typeName="UNUSEDPHEN" displayLabel="Unused" definition="LENGTH*LENGTH" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Modifying Phenomenon.Definition is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.3" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH*LENGTH" />
                    <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Adding a phenomenon, modifying display label and description of phenomenon",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "277108218ae9caa645b900c85fd9c0806343ace92cc4da016cb6f18b0ea03255";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.4" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <Phenomenon typeName="AREA" displayLabel="Areal" description="Area" definition="LENGTH*LENGTH" />
                    <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                    <Phenomenon typeName="VOLUME" displayLabel="Volume" definition="LENGTH*LENGTH*LENGTH" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Check schema phenomenon after modifications",
        [&]()
            {
            ECSchemaCP schema = m_briefcase->Schemas().GetSchema("Schema1");
            ASSERT_TRUE(schema != nullptr);

            assertPhenomenon(*schema, "AREA", "Areal", "Area", "LENGTH*LENGTH");
            assertPhenomenon(*schema, "UNUSEDP", "Unused", "", "LENGTH*LENGTH");
            assertPhenomenon(*schema, "VOLUME", "Volume", "", "LENGTH*LENGTH*LENGTH");
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, Units)
    {
    auto assertUnit = [](ECSchemaCR schema, Utf8CP name, Utf8CP displayLabel, Utf8CP description, Utf8CP definition, double numerator, double denominator, double offset, Utf8CP phenomenon, Utf8CP unitSystem)
        {
        ECUnitCP unit = schema.GetUnitCP(name);
        ASSERT_TRUE(unit != nullptr) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(name, unit->GetName().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(definition, unit->GetDefinition().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(displayLabel, unit->GetDisplayLabel().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(description, unit->GetDescription().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_DOUBLE_EQ(numerator, unit->GetNumerator()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_DOUBLE_EQ(denominator, unit->GetDenominator()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_DOUBLE_EQ(offset, unit->GetOffset()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(phenomenon, unit->GetPhenomenon()->GetFullName().c_str()) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(unitSystem, unit->GetUnitSystem()->GetFullName().c_str()) << schema.GetFullSchemaName() << ":" << name;
        };

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "3780bc9b30cc32f1633478be3098e3a37df04075f41f86edf4b3e7cd6bec0d39";
    Test(
        "Import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                    <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                    <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                    <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                    <Unit typeName="SquareM" displayLabel="Square Meter" definition="M*M" numerator="1.0" phenomenon="AREA" unitSystem="METRIC" />
                    <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupgrade_units", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Check schema units",
        [&]()
            {
            ECSchemaCP schema = m_briefcase->Schemas().GetSchema("Schema1");
            ASSERT_TRUE(schema != nullptr);

            assertUnit(*schema, "SquareM", "Square Meter", "", "M*M", 1.0, 1.0, 0.0, "Schema1:AREA", "Schema1:METRIC");
            assertUnit(*schema, "SquareFt", "Square Feet", "", "Ft*Ft", 10.0, 1.0, 0.4, "Schema1:AREA", "Schema1:IMPERIAL");
            }
    );

    Test(
        "Deleting a unit is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                    <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                    <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                    <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                    <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Renaming a Unit is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                    <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                    <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                    <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                    <Unit typeName="SquareMeter" displayLabel="Square Meter" definition="M*M" numerator="1.0" phenomenon="AREA" unitSystem="METRIC" />
                    <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Modifying Unit.Definition is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.3" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                    <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                    <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                    <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                    <Unit typeName="SquareM" displayLabel="Square Meter" definition="M*M*" numerator="1.0" phenomenon="AREA" unitSystem="METRIC" />
                    <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Modifying Unit.Numerator is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.4" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                    <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                    <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                    <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                    <Unit typeName="SquareM" displayLabel="Square Meter" definition="M*M" numerator="1.5" phenomenon="AREA" unitSystem="METRIC" />
                    <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Modifying Unit.Denominator is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.5" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                    <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                    <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                    <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                    <Unit typeName="SquareM" displayLabel="Square Meter" definition="M*M" numerator="1.0" denominator="2.0" phenomenon="AREA" unitSystem="METRIC" />
                    <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Modifying Unit.Offset is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.6" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                    <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                    <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                    <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                    <Unit typeName="SquareM" displayLabel="Square Meter" definition="M*M" numerator="1.0" offset="1.0" phenomenon="AREA" unitSystem="METRIC" />
                    <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Modifying Unit.UnitSystem is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.7" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                    <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                    <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                    <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                    <Unit typeName="SquareM" displayLabel="Square Meter" definition="M*M" numerator="1.0" phenomenon="AREA" unitSystem="IMPERIAL" />
                    <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Modifying Unit.Phenomenon is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.8" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <UnitSystem typeName="METRIC" displayLabel="Metric" description="Metric Units of measure" />
                    <UnitSystem typeName="IMPERIAL" displayLabel="Imperial" description="Units of measure from the British Empire" />
                    <Phenomenon typeName="AREA" displayLabel="Area" definition="LENGTH*LENGTH" />
                    <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                    <Unit typeName="SquareM" displayLabel="Square Meter" definition="M*M" numerator="1.0" phenomenon="UNUSEDP" unitSystem="METRIC" />
                    <Unit typeName="SquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Adding a unit, modifying display label and description of unit, removing display label of unit",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "1f84f073fb5628db9f18be0152af5a91b44350b61519965b024e6e9caf9daf53";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema1" alias="s1" version="1.0.9" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <UnitSystem typeName="METRIC" />
                    <UnitSystem typeName="IMPERIAL" displayLabel="ImperialSystem" description="Units of measure from the British Empire." />
                    <Phenomenon typeName="AREA" displayLabel="Areal" description="Area" definition="LENGTH*LENGTH" />
                    <Phenomenon typeName="UNUSEDP" displayLabel="Unused" definition="LENGTH*LENGTH" />
                    <Unit typeName="SquareM" displayLabel="Square Metre" description="Square Metre" definition="M*M" numerator="1.0" phenomenon="AREA" unitSystem="METRIC" />
                    <Unit typeName="SquareFt" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="AREA" unitSystem="IMPERIAL" />
                    <Unit typeName="MyUnit" displayLabel="My Unit" description="my nice unit" definition="M*M" numerator="1.0" phenomenon="AREA" unitSystem="METRIC" />
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Check schema units after modifications",
        [&]()
            {
            ECSchemaCP schema = m_briefcase->Schemas().GetSchema("Schema1");
            ASSERT_TRUE(schema != nullptr);

            assertUnit(*schema, "SquareM", "Square Metre", "Square Metre", "M*M", 1.0, 1.0, 0.0, "Schema1:AREA", "Schema1:METRIC");
            assertUnit(*schema, "SquareFt", "SquareFt", "", "Ft*Ft", 10.0, 1.0, 0.4, "Schema1:AREA", "Schema1:IMPERIAL");
            assertUnit(*schema, "MyUnit", "My Unit", "my nice unit", "M*M", 1.0, 1.0, 0.0, "Schema1:AREA", "Schema1:METRIC");
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AllowedChangingUnitConversionProperties)
    {
    Test(
        "Import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "7c971f1b51995e5c3ebc26d3d6b05f1f34189cc9c77c7a3492cc1696e7a0ff9f";
            auto schema = SchemaItem(
                R"schema(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="01.00.07" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <UnitSystem typeName="TestUnitSystem"/>
                    <Phenomenon typeName="LENGTH" definition="LENGTH"/>
                    <Unit typeName="TestUnit1" phenomenon="LENGTH" unitSystem="TestUnitSystem" definition="TestUnit1" description="Test unit 1." displayLabel="TestLabel1"/>
                </ECSchema>)schema"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupgrade_unitsystems", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Changing from undefined to default values is allowed",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "d580a1e84b62ad088235a2ac5bb02da98a0f01ece96add8d81b391917c87ed33";
            auto schema = SchemaItem(
                R"schema(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="01.00.07" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <UnitSystem typeName="TestUnitSystem"/>
                    <Phenomenon typeName="LENGTH" definition="LENGTH"/>
                    <Unit typeName="TestUnit1" phenomenon="LENGTH" unitSystem="TestUnitSystem" definition="TestUnit1" description="Test unit 1." displayLabel="TestLabel1" numerator="1.0" denominator="1.0" offset="0.0"/>
                </ECSchema>)schema"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, NotAllowedChangingUnitConversionProperties)
    {
    // Numerator, denominator and offset propertise are undefined
    SchemaItem schemaWithoutUnitProperties(R"schema(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.07" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="TestUnitSystem"/>
            <Phenomenon typeName="LENGTH" definition="LENGTH"/>
            <Unit typeName="TestUnit1" phenomenon="LENGTH" unitSystem="TestUnitSystem" definition="TestUnit1" description="Test unit 1." displayLabel="TestLabel1"/>
        </ECSchema>)schema");
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "7c971f1b51995e5c3ebc26d3d6b05f1f34189cc9c77c7a3492cc1696e7a0ff9f";

    // Numerator, denominator and offset propertise are not default values
    SchemaItem schemaWithUnitProperties(R"schema(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.07" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="TestUnitSystem"/>
            <Phenomenon typeName="LENGTH" definition="LENGTH"/>
            <Unit typeName="TestUnit1" phenomenon="LENGTH" unitSystem="TestUnitSystem" definition="TestUnit1" description="Test unit 1." displayLabel="TestLabel1" numerator="3.0" denominator="2.0" offset="2.0"/>
        </ECSchema>)schema");
    const auto SCHEMA2_HASH_ECDB_SCHEMA = "7d63bc23dc3764364c0b0a0e17df9ebc7ee921375e6b802fed561e119cbcbc5a";


    Test(
        "Import initial schemas without unit properties",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("testDB", schemaWithoutUnitProperties));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Deleting a unit system is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml()xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schemaWithUnitProperties)) << "ECSchema Upgrade failed. Changing properties of Unit 'TestSchema:TestUnit1' is not supported except for DisplayLabel and Description.";
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Import initial schemas with unit properties",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("testDB1", schemaWithUnitProperties));
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Deleting a unit system is not supported",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schemaWithoutUnitProperties)) << "ECSchema Upgrade failed. Changing properties of Unit 'TestSchema:TestUnit1' is not supported except for DisplayLabel and Description.";
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, AllowedChangingConstantConversionProperties)
    {
    // Denominator property is undefined
    SchemaItem schemaWithoutConstantProperties(
        R"schema(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2" version="01.02.03" schemaName="TestSchema" alias="t">
            <Constant typeName="TestConstant" displayLabel="Test Constant" description="testing a constant" definition="TestConstant" phenomenon="TestPhenomenon" numerator="1.0"/>
            <Phenomenon typeName="TestPhenomenon" definition="TestPhenomenon" />
            <UnitSystem typeName="TestUnitSystem" />
        </ECSchema>)schema"
    );
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "679b0da3c4f6709e829c8f97b1e7dcf0a1d7b2f67732e53b1e80df0160536bad";

    // Denominator property is set to default
    SchemaItem schemaWithConstantProperties(
        R"schema(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2" version="01.02.03" schemaName="TestSchema" alias="t">
            <Constant typeName="TestConstant" displayLabel="Test Constant" description="testing a constant" definition="TestConstant" phenomenon="TestPhenomenon" numerator="1.0" denominator="1.0"/>
            <Phenomenon typeName="TestPhenomenon" definition="TestPhenomenon" />
            <UnitSystem typeName="TestUnitSystem" />
        </ECSchema>)schema"
    );
    const auto SCHEMA2_HASH_ECDB_SCHEMA = "25059b91331afdf2125ad07a307e7e72f0e63c351c05180d7966b4b969af0f03";

    Test(
        "import schema without constant property",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("testDB", schemaWithoutConstantProperties));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "import schema with constant property",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schemaWithConstantProperties));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, NotAllowedChangingConstantConversionProperties)
    {
    // Denominator property is undefined
    SchemaItem schemaWithoutConstantProperty(
        R"schema(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.07" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="TestUnitSystem"/>
            <Phenomenon typeName="LENGTH" definition="LENGTH"/>
            <Constant typeName="TestConstant" phenomenon="LENGTH" definition="TestConstant" displayLabel="TestLabel1" numerator="1.0"/>
        </ECSchema>)schema"
    );
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "c33e8ca5dc27ca03ea7583fced2289aaa95e6637ab924559435f675fb08a4f12";

    // Denominator and numerator properties are not default value
    SchemaItem schemaWithConstantProperty(
        R"schema(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.07" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="TestUnitSystem"/>
            <Phenomenon typeName="LENGTH" definition="LENGTH"/>
            <Constant typeName="TestConstant" phenomenon="LENGTH" definition="TestConstant" displayLabel="TestLabel1" numerator="3.0" denominator="2.0"/>
        </ECSchema>)schema"
    );
    const auto SCHEMA2_HASH_ECDB_SCHEMA = "f63bff95aad63665233b21d41d5ed7170e37c38f869fbbf8cda78d6632e34787";
    
    Test(
        "import schema without constant property",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("testDB", schemaWithoutConstantProperty));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "fail to import schema with constant property",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schemaWithConstantProperty));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "import schema with constant property",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("testDB1", schemaWithConstantProperty));
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "fail to import schema without constant property",
        [&]()
            {
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schemaWithoutConstantProperty));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, Formats)
    {
    auto assertFormat = [](ECDbCR ecdb, Utf8CP name, Utf8CP displayLabel, Utf8CP description, JsonValue const& numericSpec, JsonValue const& compSpec)
        {
        ECFormatCP format = ecdb.Schemas().GetFormat("Schema", name);
        ASSERT_TRUE(format != nullptr) << "Schema." << name;

        Utf8String assertMessage(format->GetSchema().GetFullSchemaName());
        assertMessage.append(".").append(format->GetName());

        ASSERT_STREQ(name, format->GetName().c_str()) << assertMessage;
        if (Utf8String::IsNullOrEmpty(displayLabel))
            ASSERT_FALSE(format->GetIsDisplayLabelDefined()) << assertMessage;
        else
            ASSERT_STREQ(displayLabel, format->GetInvariantDisplayLabel().c_str()) << assertMessage;

        if (Utf8String::IsNullOrEmpty(description))
            ASSERT_FALSE(format->GetIsDescriptionDefined()) << assertMessage;
        else
            ASSERT_STREQ(description, format->GetInvariantDescription().c_str()) << assertMessage;

        if (numericSpec.m_value.isNull())
            ASSERT_FALSE(format->HasNumeric()) << assertMessage;
        else
            {
            ASSERT_TRUE(format->HasNumeric()) << assertMessage;
            Json::Value jval;
            ASSERT_TRUE(format->GetNumericSpec()->ToJson(jval, false)) << assertMessage;
            ASSERT_EQ(numericSpec, JsonValue(jval)) << assertMessage;
            }

        if (compSpec.m_value.isNull())
            ASSERT_FALSE(format->HasComposite()) << assertMessage;
        else
            {
            Json::Value jval;
            ASSERT_TRUE(format->GetCompositeSpec()->ToJson(jval)) << assertMessage;
            ASSERT_TRUE(format->HasComposite()) << assertMessage;
            ASSERT_EQ(compSpec, JsonValue(jval)) << assertMessage;
            }
        };

    Test(
        "Import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "cd2dda9f6c1a53f61840d444920f85cdbde731a5dd94d56bbcbd81ee14890da4";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                    <Format typeName="MyFormat" displayLabel="My Format" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero"
                            precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                    </Format>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupgrade_formats", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Check initial schema format",
        [&]()
            {
            auto numericSpec = JsonValue(
                R"json({
                    "roundFactor":0.3,
                    "type": "Fractional",
                    "showSignOption": "OnlyNegative",
                    "formatTraits": ["trailZeroes", "keepSingleZero"],
                    "precision": 4,
                    "decimalSeparator": ".",
                    "thousandSeparator": ",",
                    "uomSeparator": " "
                })json"
            );
            assertFormat(*m_briefcase, "MyFormat", "My Format", "", numericSpec, JsonValue());
            }
    );

    Test(
        "Modify DisplayLabel, Description, NumericSpec",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "7c7ea8c1414f2134e81838883967d05060ffbdd25d38f328f0572ecdfb2eeda5";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                    <Format typeName="MyFormat" displayLabel="My nice Format" description="Real nice format" roundFactor="1.3" type="Scientific" scientificType="ZeroNormalized" showSignOption="SignAlways" formatTraits="KeepSingleZero"
                            precision="5" decimalSeparator="," thousandSeparator="." uomSeparator="#">
                    </Format>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Check modified schema format",
        [&]()
            {
            auto numericSpec = JsonValue(
                R"json({
                    "roundFactor":1.3,
                    "type": "Scientific",
                    "scientificType":"ZeroNormalized",
                    "showSignOption": "SignAlways",
                    "formatTraits": ["keepSingleZero"],
                    "precision": 5, "decimalSeparator": ",",
                    "thousandSeparator": ".",
                    "uomSeparator": "#"
                })json"
            );
            assertFormat(*m_briefcase, "MyFormat", "My nice Format", "Real nice format", numericSpec, JsonValue());
            }
    );


    const auto SCHEMA1_HASH_ECDB_SCHEMA = "d5dff39b8b48d1be6f9a948cff9120662a2fcd4a0e7932eae004768b96a8937b";
    Test(
        "remove optional attributes from num spec",
        [&]()
            {
            
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema" alias="ts" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                    <Format typeName="MyFormat" roundFactor="1.3" type="Scientific" scientificType="ZeroNormalized" showSignOption="SignAlways"
                            precision="5">
                    </Format>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Check schema format with removed optional attributes",
        [&]()
            {
            auto numericSpec = JsonValue(R"json({"roundFactor":1.3, "type": "Scientific", "scientificType":"ZeroNormalized", "showSignOption": "SignAlways", "precision": 5})json");
            assertFormat(*m_briefcase, "MyFormat", "", "", numericSpec, JsonValue());
            }
    );

    Test(
        "Adding composite is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema" alias="ts" version="1.0.3" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                    <Format typeName="MyFormat" roundFactor="1.3" type="Scientific" scientificType="ZeroNormalized" showSignOption="SignAlways"
                            precision="5">
                        <Composite>
                            <Unit label="mm">u:MM</Unit>
                        </Composite>
                    </Format>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "start with format that already has a composite",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "8c7c078f21bc308c27938d87795802b5cfe37ca99c612b8e285f394fea5e0499";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema" alias="ts" version="2.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                    <Unit typeName="MyMeter" displayLabel="My Metre" definition="u:M" numerator="1.0" phenomenon="u:LENGTH" unitSystem="u:METRIC" />
                    <Format typeName="MyFormat" displayLabel="My Format" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero"
                            precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                        <Composite>
                            <Unit label="m">MyMeter</Unit>
                            <Unit label="mm">u:MM</Unit>
                        </Composite>
                    </Format>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupgrade_formats", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Check new schema format",
        [&]()
            {
            assertFormat(*m_briefcase, "MyFormat", "My Format", "",
                 JsonValue(R"json({"roundFactor":0.3, "type": "Fractional", "showSignOption": "OnlyNegative", "formatTraits": ["trailZeroes", "keepSingleZero"], "precision": 4, "decimalSeparator": ".", "thousandSeparator": ",", "uomSeparator": " "})json"),
                 JsonValue(R"json({"includeZero":true, "units": [{"name":"MyMeter", "label":"m"}, {"name":"MM", "label":"mm"}]})json"));
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "8cd1f8af64f3ea785b6a24ee7edbbd9dac06c0f376e483daa61585115c61f1a9";
    Test(
        "Modify CompSpec except for units",
        [&]()
            {
            
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema" alias="ts" version="2.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                    <Unit typeName="MyMeter" displayLabel="My Metre" definition="u:M" numerator="1.0" phenomenon="u:LENGTH" unitSystem="u:METRIC" />
                    <Format typeName="MyFormat" displayLabel="My Format" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero"
                            precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                        <Composite spacer="=" includeZero="False">
                            <Unit label="meterle">MyMeter</Unit>
                            <Unit label="millimeterle">u:MM</Unit>
                        </Composite>
                    </Format>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Check modified schema format",
        [&]()
            {
            assertFormat(*m_briefcase, "MyFormat", "My Format", "",
                 JsonValue(R"json({"roundFactor":0.3, "type": "Fractional", "showSignOption": "OnlyNegative", "formatTraits": ["trailZeroes", "keepSingleZero"], "precision": 4, "decimalSeparator": ".", "thousandSeparator": ",", "uomSeparator": " "})json"),
                 JsonValue(R"json({"spacer":"=", "includeZero":false, "units": [{"name":"MyMeter", "label":"meterle"}, {"name":"MM", "label":"millimeterle"}]})json"));
            }
    );

    Test(
        "Modifying Composite Unit is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema" alias="ts" version="2.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                    <Unit typeName="MyMeter" displayLabel="My Metre" definition="u:M" numerator="1.0" phenomenon="u:LENGTH" unitSystem="u:METRIC" />
                    <Format typeName="MyFormat" displayLabel="My Format" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero"
                            precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                        <Composite spacer="=" includeZero="False">
                            <Unit label="meterle">u:M</Unit>
                            <Unit label="millimeterle">u:MM</Unit>
                        </Composite>
                    </Format>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Deleting a Composite Unit is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema" alias="ts" version="2.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                    <Unit typeName="MyMeter" displayLabel="My Metre" definition="u:M" numerator="1.0" phenomenon="u:LENGTH" unitSystem="u:METRIC" />
                    <Format typeName="MyFormat" displayLabel="My Format" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero"
                            precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                        <Composite spacer="=" includeZero="False">
                            <Unit label="meterle">MyMeter</Unit>
                        </Composite>
                    </Format>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA); });
            }
    );

    Test(
        "Adding a Composite Unit is not supported",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="Schema" alias="ts" version="2.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                    <Unit typeName="MyMeter" displayLabel="My Metre" definition="u:M" numerator="1.0" phenomenon="u:LENGTH" unitSystem="u:METRIC" />
                    <Format typeName="MyFormat" displayLabel="My Format" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero"
                            precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                        <Composite spacer="=" includeZero="False">
                            <Unit label="meterle">MyMeter</Unit>
                            <Unit label="millimeterle">u:MM</Unit>
                            <Unit label="kilometerle">u:KM</Unit>
                        </Composite>
                    </Format>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, MultiSessionSchemaImport_TPC)
    {
    Test(
        "Import initial schema with L1",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "11471d4438add3071f6407442bf1f7101a09abbbdd4c01d65f48661ea96cc538";
            const auto SCHEMA_HASH_ECDB_MAP = "a3a4f4bd3e7c375e583bb9d5dde2d4ee593dab571dd1ab2bae88d5c531753137";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "2821726bf97bd871e7e7ca78101175541782ba81be1b692eee2a68547f403a48";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema1' alias='ts1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
                    <ECEntityClass typeName='TestClassA' >
                        <ECProperty propertyName='L1' typeName='double'/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("multisession_si", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert schema values",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts1.TestClassA (ECInstanceId, L1) VALUES(1, 101)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();
            }
    );

    Test(
        "import schema with L2",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "7743a2728733771e9c5c24229e5a584c5421b54b8b618c71c97c140ba1496c19";
            const auto SCHEMA_HASH_ECDB_MAP = "e27a81dde2bb68b528d15f9955238ba42ee0ebb407693b6ee296fb38aca7f55f";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "ac93dde76353c71cd46529e0dc9fb3c0db7adb9fd487261692c8b4c7efae03cd";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName='TestSchema2' alias='ts2' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
                    <ECSchemaReference name='TestSchema1' version='01.00.00' alias='ts1'/>
                    <ECEntityClass typeName='TestClassB' >
                        <BaseClass>ts1:TestClassA</BaseClass>
                        <ECProperty propertyName='L2' typeName='double'/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Second insert schema values",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts2.TestClassB (ECInstanceId, L1, L2) VALUES(2, 102, 202)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "cf04062a9bf74ccd5f5e3daea8bb0db6a820dd2a02a51d71d909fa38e6252e0e";
    const auto SCHEMA1_HASH_ECDB_MAP = "58a0204bc83ebddc5fbb9e3d119e07413090cae17ebe26fb633f12c20f22d958";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "1ff0f991ff1ef6ba274718b360b3d53311bb194ea555eb064a67d16c4e6a694e";
    Test(
        "import schema with L3",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName='TestSchema3' alias='ts3' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
                    <ECSchemaReference name='TestSchema2' version='01.00.00' alias='ts2'/>
                    <ECEntityClass typeName='TestClassC' >
                        <BaseClass>ts2:TestClassB</BaseClass>
                        <ECProperty propertyName='L3' typeName='double'/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Third insert schema values",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts3.TestClassC (ECInstanceId, L1, L2, L3) VALUES(3, 103, 203, 303)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();
            }
    );

    const ECClassCP classTestClassA = m_briefcase->Schemas().GetClass("TestSchema1", "TestClassA");
    const ECClassCP classTestClassB = m_briefcase->Schemas().GetClass("TestSchema2", "TestClassB");
    const ECClassCP classTestClassC = m_briefcase->Schemas().GetClass("TestSchema3", "TestClassC");

    ASSERT_NE(nullptr, classTestClassA);
    ASSERT_NE(nullptr, classTestClassB);
    ASSERT_NE(nullptr, classTestClassC);
    Test(
        "L1",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=1 AND ECClassId=%s AND L1=101",
                                                                                 classTestClassA->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=2 AND ECClassId=%s AND L1=102",
                                                                                 classTestClassB->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103",
                                                                                 classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();
            }
    );

    Test(
        "L2",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=1")));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=2 AND ECClassId=%s AND L1=102 AND L2=202",
                                                                                 classTestClassB->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103 AND L2=203",
                                                                                 classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();
            }
    );

   Test(
        "L3",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=1")));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=2 ")));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103 AND L2=203 AND L3=303",
                                                                                 classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();
            }
    );

    Test(
        "Final hash check",
        [&]()
            {
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

// -------------------------------------------------------------------------------------- -
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, MultiSessionSchemaImport_TPH_Joined_OnDerivedClass)
    {
    Test(
        "Import initial schema with L1",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "11471d4438add3071f6407442bf1f7101a09abbbdd4c01d65f48661ea96cc538";
            const auto SCHEMA_HASH_ECDB_MAP = "a3a4f4bd3e7c375e583bb9d5dde2d4ee593dab571dd1ab2bae88d5c531753137";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "2821726bf97bd871e7e7ca78101175541782ba81be1b692eee2a68547f403a48";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema1' alias='ts1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
                    <ECEntityClass typeName='TestClassA' >
                        <ECProperty propertyName='L1' typeName='double'/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("multisession_si", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert schema values",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts1.TestClassA (ECInstanceId, L1) VALUES(1, 101)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();
            }
    );

    Test(
        "import schema with L2",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "0e72376b34d8116a76a63dccc11def3103ccd9944c5ad4283f8ab3a820d701ff";
            const auto SCHEMA_HASH_ECDB_MAP = "c11d8ab770b965c462d934f1f9177fe749fd87824d4a8738fd3eeacb753bd099";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "fd751d28824929a1249194c4d4018c7a5333b5f08b70f771af607225d972eb79";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName='TestSchema2' alias='ts2' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
                    <ECSchemaReference name='TestSchema1' version='01.00.00' alias='ts1'/>
                    <ECEntityClass typeName='TestClassB' >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                         <JoinedTablePerDirectSubclass xmlns = "ECDbMap.02.00" />
                        </ECCustomAttributes>
                        <BaseClass>ts1:TestClassA</BaseClass>
                        <ECProperty propertyName='L2' typeName='double'/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Second insert schema values",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts2.TestClassB (ECInstanceId, L1, L2) VALUES(2, 102, 202)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "899a1b01dd12478aa304ab2a763754903ed0d7fed7118b7f099caec2ea69b7cc";
    const auto SCHEMA1_HASH_ECDB_MAP = "1d21c19eb85166662cd57b4219628730a413c81737e54beb07f1272800fe9923";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "19eef9c97b8be9f7a82a64658c99fc156331906dfd285100146f46f04304454a";
    Test(
        "import schema with L3",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName='TestSchema3' alias='ts3' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
                    <ECSchemaReference name='TestSchema2' version='01.00.00' alias='ts2'/>
                    <ECEntityClass typeName='TestClassC' >
                        <BaseClass>ts2:TestClassB</BaseClass>
                        <ECProperty propertyName='L3' typeName='double'/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Third insert schema values",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts3.TestClassC (ECInstanceId, L1, L2, L3) VALUES(3, 103, 203, 303)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();
            }
    );

    const ECClassCP classTestClassA = m_briefcase->Schemas().GetClass("TestSchema1", "TestClassA");
    const ECClassCP classTestClassB = m_briefcase->Schemas().GetClass("TestSchema2", "TestClassB");
    const ECClassCP classTestClassC = m_briefcase->Schemas().GetClass("TestSchema3", "TestClassC");

    ASSERT_NE(nullptr, classTestClassA);
    ASSERT_NE(nullptr, classTestClassB);
    ASSERT_NE(nullptr, classTestClassC);
    Test(
        "L1",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=1 AND ECClassId=%s AND L1=101",
                                                                                 classTestClassA->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=2 AND ECClassId=%s AND L1=102",
                                                                                 classTestClassB->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103",
                                                                                 classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();
            }
    );

    Test(
        "L2",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=1")));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=2 AND ECClassId=%s AND L1=102 AND L2=202",
                                                                                 classTestClassB->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103 AND L2=203",
                                                                                 classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();
            }
    );

   Test(
        "L3",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=1")));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=2 ")));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103 AND L2=203 AND L3=303",
                                                                                 classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();
            }
    );

    Test(
        "Final hash check",
        [&]()
            {
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, MultiSessionSchemaImport_TPH_OnDerivedClass)
    {
    Test(
        "Import initial schema with L1",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "11471d4438add3071f6407442bf1f7101a09abbbdd4c01d65f48661ea96cc538";
            const auto SCHEMA_HASH_ECDB_MAP = "a3a4f4bd3e7c375e583bb9d5dde2d4ee593dab571dd1ab2bae88d5c531753137";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "2821726bf97bd871e7e7ca78101175541782ba81be1b692eee2a68547f403a48";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema1' alias='ts1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
                    <ECEntityClass typeName='TestClassA' >
                        <ECProperty propertyName='L1' typeName='double'/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("multisession_si", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Insert schema values",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts1.TestClassA (ECInstanceId, L1) VALUES(1, 101)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();
            }
    );

    Test(
        "import schema with L2",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "e978f82f18a5e05f88d89cd8246585a917605699f9885f287dd50229bdd9f622";
            const auto SCHEMA_HASH_ECDB_MAP = "5d7fb10acf8c16c084b42898ce97a1cc2c0e443b514770f9e887d4a7f93f250d";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "fd751d28824929a1249194c4d4018c7a5333b5f08b70f771af607225d972eb79";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName='TestSchema2' alias='ts2' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
                    <ECSchemaReference name='TestSchema1' version='01.00.00' alias='ts1'/>
                    <ECEntityClass typeName='TestClassB' >
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <BaseClass>ts1:TestClassA</BaseClass>
                        <ECProperty propertyName='L2' typeName='double'/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Second insert schema values",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts2.TestClassB (ECInstanceId, L1, L2) VALUES(2, 102, 202)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "d9c1dd492913ab5e653859b3f36145bfa9caa28bc585e71b6e85b97d9b2f6106";
    const auto SCHEMA1_HASH_ECDB_MAP = "a74b32d6668ddad9fc906692d307983567b2e5d915120a9bdcb632cd387ef6c4";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "3a9a899ac76e337b3e442da538e53889dfbc5988e682014083fb76759e691e54";
    Test(
        "import schema with L3",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName='TestSchema3' alias='ts3' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
                    <ECSchemaReference name='TestSchema2' version='01.00.00' alias='ts2'/>
                    <ECEntityClass typeName='TestClassC' >
                        <BaseClass>ts2:TestClassB</BaseClass>
                        <ECProperty propertyName='L3' typeName='double'/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Third insert schema values",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts3.TestClassC (ECInstanceId, L1, L2, L3) VALUES(3, 103, 203, 303)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();
            }
    );

    const ECClassCP classTestClassA = m_briefcase->Schemas().GetClass("TestSchema1", "TestClassA");
    const ECClassCP classTestClassB = m_briefcase->Schemas().GetClass("TestSchema2", "TestClassB");
    const ECClassCP classTestClassC = m_briefcase->Schemas().GetClass("TestSchema3", "TestClassC");

    ASSERT_NE(nullptr, classTestClassA);
    ASSERT_NE(nullptr, classTestClassB);
    ASSERT_NE(nullptr, classTestClassC);
    Test(
        "L1",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=1 AND ECClassId=%s AND L1=101",
                                                                                 classTestClassA->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=2 AND ECClassId=%s AND L1=102",
                                                                                 classTestClassB->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts1.TestClassA WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103",
                                                                                 classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();
            }
    );

    Test(
        "L2",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=1")));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=2 AND ECClassId=%s AND L1=102 AND L2=202",
                                                                                 classTestClassB->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts2.TestClassB WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103 AND L2=203",
                                                                                 classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();
            }
    );

   Test(
        "L3",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=1")));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=2 ")));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Not Expecting Row : " << stmt.GetECSql();
            stmt.Finalize();

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, SqlPrintfString("SELECT 1 FROM ts3.TestClassC WHERE ECInstanceId=3 AND ECClassId=%s AND L1=103 AND L2=203 AND L3=303",
                                                                                 classTestClassC->GetId().ToString(BeInt64Id::UseHex::Yes).c_str())));
            ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Expect Row : " << stmt.GetECSql();
            stmt.Finalize();
            }
    );

    Test(
        "Final hash check",
        [&]()
            {
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateReferencesFromDifferentContext)
    {
    Test("Setup empty ECDb", [&]() { ASSERT_EQ(SchemaImportResult::OK, SetupECDb("failingImport")); });

    Test(
        "first schema set",
        [&]()
            {
            ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
            ctx->AddSchemaLocater(m_briefcase->GetSchemaLocater());
            ECSchemaPtr myRef, mySchema;
            // newer references 1.0.1
            ECSchema::ReadFromXmlString(myRef, R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="MyRef" alias="myref" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="MyRefClass"  modifier="none">
                    <ECProperty propertyName="MyRefProperty" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml", *ctx);

            // reference older version (1.0.0)
            ECSchema::ReadFromXmlString(mySchema, R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="MySchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="MyRef" version="01.00.00" alias="myref" />
                <ECEntityClass typeName="MyClass"  modifier="none">
                    <BaseClass>myref:MyRefClass</BaseClass>
                    <ECProperty propertyName="MyProperty" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml", *ctx);

            const auto SCHEMA_HASH_ECDB_SCHEMA = "cc65bd9568f94ed2e0b246272e0ade4e1347aefd1cf4ea2f5448e2a95b7279ce";
            const auto SCHEMA_HASH_ECDB_MAP = "50e8780fda26200b3f97e18a48265eaf1e01c3850f783599dac578eefeacf1b9";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "cd4463b1058dc63446a6aab613ec85ed6364d441fb98e2526ffab5914a84d726";
            ASSERT_EQ(SchemaImportResult::OK, m_briefcase->Schemas().ImportSchemas(ctx->GetCache().GetSchemas(), SchemaManager::SchemaImportOptions::None, nullptr, m_schemaChannel->GetChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "second without locator",
        [&]()
            {
            ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();

            // no locator
            // ctx->AddSchemaLocater(m_briefcase->GetSchemaLocater());
            ECSchemaPtr myRef, mySchema;
            // newer references 1.0.1
            ECSchema::ReadFromXmlString(myRef, R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="MyRef" alias="myref" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="MyRefClass"  modifier="none">
                    <ECProperty propertyName="MyRefProperty" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml", *ctx);

            // reference older version (1.0.0)
            ECSchema::ReadFromXmlString(mySchema, R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="MySchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="MyRef" version="01.00.00" alias="myref" />
                <ECEntityClass typeName="MyClass"  modifier="none">
                    <BaseClass>myref:MyRefClass</BaseClass>
                    <ECProperty propertyName="MyProperty" typeName="int" />
                    <ECProperty propertyName="MyProperty1" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml", *ctx);

            const auto SCHEMA_HASH_ECDB_SCHEMA = "3efbfdedc865b69a0f69deecc5d96edb811a98a2ff98773703d0eaf4cd4bc4da";
            const auto SCHEMA_HASH_ECDB_MAP = "16cf0552a8bdaec4c6d759dd89b4c40de262969f4d0b905248f81a1ef7aa87b4";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "ff0d88d2b854d847016398b4a1740f41963695c4403f5af75c679a6aba3b8060";
            ASSERT_EQ(SchemaImportResult::OK, m_briefcase->Schemas().ImportSchemas(ctx->GetCache().GetSchemas(), SchemaManager::SchemaImportOptions::None, nullptr, m_schemaChannel->GetChannelUri())) << "This should fail";
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateRelationshipConstraintClassGeneralize)
    {
    const auto SCHEMA1_HASH_ECDB_MAP = "b2fad42e3ef33786344ad4c6907c85d86ce8a054eccdb4377e15970e7d6150bf";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "5c2cb7332bd42ab1b5531bbd12a493c967aa83058a21f6b9ff8af82a28a89ecf";
    Test(
        "import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "ac62e787980a0f6e665dd93d732e8893ecdd46243140fec4670defc0f5d28863";
            
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="LinearReferencing" alias="lr" version="02.00.02" description="Base schema for Linear Referencing." xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName="ILinearlyLocated" modifier="Abstract" description="">
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                                <AppliesToEntityClass>GeometricElement3d</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName="ILinearLocationElement" description="" modifier="Abstract">
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                                <AppliesToEntityClass>GeometricElement3d</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECRelationshipClass typeName="ILinearLocationLocatesElement" strength="referencing" modifier="None" description="">
                        <Source multiplicity="(0..*)" polymorphic="true" roleLabel="linearly-locates">
                            <Class class="ILinearLocationElement"/>
                        </Source>
                        <Target multiplicity="(0..1)" polymorphic="true" roleLabel="linearly-located by">
                            <Class class="GeometricElement3d"/>
                        </Target>
                    </ECRelationshipClass>
                    <ECEntityClass typeName="LinearLocationElement" description="" modifier="Abstract">
                        <BaseClass>GeometricElement3d</BaseClass>
                        <BaseClass>ILinearLocationElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Element" modifier="Abstract" description="">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement3d" modifier="Abstract" displayLabel="">
                        <BaseClass>Element</BaseClass>
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("schemaupdate", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check values of initial schema",
        [&]()
            {
            auto aILinearLocationElement = m_briefcase->Schemas().GetClass("LinearReferencing", "ILinearLocationElement");
            auto aILinearlyLocated = m_briefcase->Schemas().GetClass("LinearReferencing", "ILinearlyLocated");
            auto aILinearLocationLocatesElement = m_briefcase->Schemas().GetClass("LinearReferencing", "ILinearLocationLocatesElement")->GetRelationshipClassCP();
            ECValue r;
            aILinearLocationElement->GetCustomAttributeLocal("IsMixin")->GetValue(r, "AppliesToEntityClass");
            ASSERT_STREQ("GeometricElement3d", r.GetUtf8CP());

            aILinearlyLocated->GetCustomAttributeLocal("IsMixin")->GetValue(r, "AppliesToEntityClass");
            ASSERT_STREQ("GeometricElement3d", r.GetUtf8CP());

            auto aConstraintClass = aILinearLocationLocatesElement->GetTarget().GetConstraintClasses().front();
            ASSERT_STREQ("GeometricElement3d", aConstraintClass->GetName().c_str());
            }
    );

    Test(
        "Import schema with smaller read version",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "d1dc6c3d20d2de827bad7418bb80343f3f1a073a1e0ba1ca482c18c90d583c1e";
            auto schema = SchemaItem(
                R"xml(<?xml version="1.0" encoding="UTF-8"?>
                <ECSchema schemaName="LinearReferencing" alias="lr" version="02.00.02" description="Base schema for Linear Referencing." xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName="ILinearlyLocated" modifier="Abstract" description="">
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName="ILinearLocationElement" description="" modifier="Abstract">
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes>
                            <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                                <AppliesToEntityClass>Element</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECRelationshipClass typeName="ILinearLocationLocatesElement" strength="referencing" modifier="None" description="">
                        <Source multiplicity="(0..*)" polymorphic="true" roleLabel="linearly-locates">
                            <Class class="ILinearLocationElement"/>
                        </Source>
                        <Target multiplicity="(0..1)" polymorphic="true" roleLabel="linearly-located by">
                            <Class class="Element"/>
                        </Target>
                    </ECRelationshipClass>
                    <ECEntityClass typeName="LinearLocationElement" description="" modifier="Abstract">
                        <BaseClass>GeometricElement3d</BaseClass>
                        <BaseClass>ILinearLocationElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName="Element" modifier="Abstract" description="">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName="GeometricElement3d" modifier="Abstract" displayLabel="">
                        <BaseClass>Element</BaseClass>
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Check values of modified schema",
        [&]()
            {
            auto aILinearLocationElement = m_briefcase->Schemas().GetClass("LinearReferencing", "ILinearLocationElement");
            auto aILinearlyLocated = m_briefcase->Schemas().GetClass("LinearReferencing", "ILinearlyLocated");
            auto aILinearLocationLocatesElement = m_briefcase->Schemas().GetClass("LinearReferencing", "ILinearLocationLocatesElement")->GetRelationshipClassCP();
            ECValue r;
            aILinearLocationElement->GetCustomAttributeLocal("IsMixin")->GetValue(r, "AppliesToEntityClass");
            ASSERT_STREQ("Element", r.GetUtf8CP());

            aILinearlyLocated->GetCustomAttributeLocal("IsMixin")->GetValue(r, "AppliesToEntityClass");
            ASSERT_STREQ("Element", r.GetUtf8CP());

            auto aConstraintClass = aILinearLocationLocatesElement->GetTarget().GetConstraintClasses().front();
            ASSERT_STREQ("Element", aConstraintClass->GetName().c_str());
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// No property or enumeration found for expression 'color.red'.
// Select after insert verification doesn't work
TEST_F(SchemaSyncTestFixture, UpdateClass_AddPropertyDeeplyNestedStruct)
    {
    Test(
        "import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "e51908b526d962bd2ebc85005e1639273ff6b7e3a6149faac13d3d4fd909c031";
            const auto SCHEMA_HASH_ECDB_MAP = "dbb55e277e5d656ea1e6547b5fd996374f1732d80d5e8ac81b03679d7a89ce92";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "479b48bde854f27df13fa92eda4ebbc7affece5d08af6d536761fd68e985c937";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8' ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" displayLabel="Nested Struct Test Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECStructClass typeName="ColorType">
                        <ECProperty propertyName="blue" typeName="string" displayLabel="Blue"/>
                        <ECProperty propertyName="green" typeName="string" displayLabel="Green"/>
                        <ECProperty propertyName="red" typeName="string" displayLabel="Red"/>
                    </ECStructClass>
                    <ECStructClass typeName="ColorNestedType" displayLabel="PSet_Draughting.ColorNested">
                        <ECStructProperty propertyName="color" typeName="ColorType"/>
                    </ECStructClass>
                    <ECStructClass typeName="NestedColorNestedType" >
                        <ECStructProperty propertyName="nestedColor" typeName="ColorNestedType" />
                    </ECStructClass>
                    <ECEntityClass typeName="TestClass">
                        <ECStructProperty propertyName="ColorNested" typeName="NestedColorNestedType"/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("addPropertyToDeeplyNestedStruct", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    // No property or enumeration found for expression 'color.red'.
    // This select verification doesn't work
    Test(
        "Verify inserted instances",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.TestClass (ColorNested.nestedColor.color.red, ColorNested.nestedColor.color.green, ColorNested.nestedColor.color.blue) VALUES ('red', 'green', 'blue')"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            // auto expected = JsonValue(R"json([{"ColorNested":{"nestedColor":{"color":{"blue":"blue","green":"green","red":"red"}}}}}])json");
            // ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT color.red, color.green, color.blue, color.testnewproperty, colornested FROM ts.testclass"));
            // ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt));
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "c880f7669a6441ca0f075dfc7a50ab0cc9fa75cd43e65dabd17473160990e55e";
    const auto SCHEMA1_HASH_ECDB_MAP = "1676154019631a5edb66edfc3562351d5fe82bc47b66cefbea1498c26d79cf24";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "bde3d19daf523466084dca405ce2aae51b89a6ce327125a89b1d1b4b2ca67738";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8' ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" displayLabel="Nested Struct Test Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECStructClass typeName="ColorType">
                        <ECProperty propertyName="blue" typeName="string" displayLabel="Blue"/>
                        <ECProperty propertyName="green" typeName="string" displayLabel="Green"/>
                        <ECProperty propertyName="red" typeName="string" displayLabel="Red"/>
                        <ECProperty propertyName="new" typeName="int" />
                    </ECStructClass>
                    <ECStructClass typeName="ColorNestedType" displayLabel="PSet_Draughting.ColorNested">
                        <ECStructProperty propertyName="color" typeName="ColorType"/>
                    </ECStructClass>
                    <ECStructClass typeName="NestedColorNestedType" >
                        <ECStructProperty propertyName="nestedColor" typeName="ColorNestedType" />
                    </ECStructClass>
                    <ECEntityClass typeName="TestClass">
                        <ECProperty propertyName="prop1" typeName="double" />
                        <ECStructProperty propertyName="ColorNested" typeName="NestedColorNestedType"/>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify we can insert and select",
        [&]()
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "INSERT INTO ts.TestClass (prop1, ColorNested.nestedColor.color.red, ColorNested.nestedColor.color.green, ColorNested.nestedColor.color.blue, ColorNested.nestedColor.color.new) VALUES (42.42, 'red2', 'green2', 'blue2', 42)"));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Finalize();

            auto expected = JsonValue(R"json([{"ColorNested":{"nestedColor":{"color":{"blue":"blue","green":"green","red":"red"}}}},{"prop1":42.42,"ColorNested":{"nestedColor":{"color":{"blue":"blue2","green":"green2","red":"red2","new":42}}}}])json");
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_briefcase, "SELECT prop1, colornested FROM ts.testclass"));
            ASSERT_EQ(expected, GetHelper().ExecutePreparedECSql(stmt)) << "Verify inserted instances";
            }
    );

    Test(
        "Final hash check",
        [&]()
            {
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateRelationshipConstraintWithMixin)
    {
    Test(
        "import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "50dba9f402e569f78dfa736c4e19edd1c4633e37771f6d75d050c35745dbda05";
            const auto SCHEMA_HASH_ECDB_MAP = "7fd0b7ef5b3520e219471363ea5c59afdd4ddd13958f1bbb580eaa54cd3f43f6";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "ff29f114c33fe47a161cbe7aee80437b8e8c3bd943ab5a38c4ae34af07100a69";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='BaseElement' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00.00'>
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element1' modifier='Abstract'>
                        <BaseClass>BaseElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element2' modifier='Abstract'>
                        <BaseClass>Element1</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocated' modifier='Abstract'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocatedAttribution' modifier='Abstract'>
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                        <ECNavigationProperty propertyName = 'AttributedElement' relationshipName = 'ILinearlyLocatedAttributesElement' direction = 'Forward' displayLabel = 'Attributed Element'/>
                    </ECEntityClass>
                    <ECEntityClass typeName = 'ILinearElementSource'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00' modifier='Abstract'>
                            <AppliesToEntityClass>Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECRelationshipClass typeName = 'ILinearlyLocatedAttributesElement' strength = 'referencing' modifier='None'>
                        <Source multiplicity = '(0..*)' polymorphic = 'true' roleLabel = 'attributes'><Class class = 'ILinearlyLocatedAttribution'/></Source>
                        <Target multiplicity = '(0..1)' polymorphic = 'true' roleLabel = 'is attributed by'><Class class = 'ILinearElementSource'/></Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("UpdateRelationshipConstraintWithMixin", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import edited schema with some changes",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "71b9d0239bb83761d4ae0d27b2ad5305620f4d7cddbc58e18b436f190f58f701";
            const auto SCHEMA_HASH_ECDB_MAP = "7fd0b7ef5b3520e219471363ea5c59afdd4ddd13958f1bbb580eaa54cd3f43f6";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "ff29f114c33fe47a161cbe7aee80437b8e8c3bd943ab5a38c4ae34af07100a69";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' description='This is Test Schema' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='BaseElement' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00.00'>
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element1' modifier='Abstract'>
                        <BaseClass>BaseElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element2' modifier='Abstract'>
                        <BaseClass>Element1</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocated' modifier='Abstract'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocatedAttribution' modifier='Abstract'>
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                        <ECNavigationProperty propertyName = 'AttributedElement' relationshipName = 'ILinearlyLocatedAttributesElement' direction = 'Forward' displayLabel = 'Attributed Element'/>
                    </ECEntityClass>
                    <ECEntityClass typeName = 'ILinearElementSource'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00' modifier='Abstract'>
                            <AppliesToEntityClass>Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECRelationshipClass typeName = 'ILinearlyLocatedAttributesElement' strength = 'referencing' modifier='None'>
                        <Source multiplicity = '(0..*)' polymorphic = 'true' roleLabel = 'attributes'><Class class = 'ILinearlyLocatedAttribution'/></Source>
                        <Target multiplicity = '(0..1)' polymorphic = 'true' roleLabel = 'is attributed by'><Class class = 'Element1'/></Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpgradeRelationshipConstraintWithBrokenMixin)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "50dba9f402e569f78dfa736c4e19edd1c4633e37771f6d75d050c35745dbda05";
    const auto SCHEMA1_HASH_ECDB_MAP = "7fd0b7ef5b3520e219471363ea5c59afdd4ddd13958f1bbb580eaa54cd3f43f6";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "ff29f114c33fe47a161cbe7aee80437b8e8c3bd943ab5a38c4ae34af07100a69";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='BaseElement' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00.00'>
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element1' modifier='Abstract'>
                        <BaseClass>BaseElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element2' modifier='Abstract'>
                        <BaseClass>Element1</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocated' modifier='Abstract'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocatedAttribution' modifier='Abstract'>
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                        <ECNavigationProperty propertyName = 'AttributedElement' relationshipName = 'ILinearlyLocatedAttributesElement' direction = 'Forward' displayLabel = 'Attributed Element'/>
                    </ECEntityClass>
                    <ECEntityClass typeName = 'ILinearElementSource'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00' modifier='Abstract'>
                            <AppliesToEntityClass>Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECRelationshipClass typeName = 'ILinearlyLocatedAttributesElement' strength = 'referencing' modifier='None'>
                        <Source multiplicity = '(0..*)' polymorphic = 'true' roleLabel = 'attributes'><Class class = 'ILinearlyLocatedAttribution'/></Source>
                        <Target multiplicity = '(0..1)' polymorphic = 'true' roleLabel = 'is attributed by'><Class class = 'ILinearElementSource'/></Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("UpgradeRelationshipConstraintWithBrokenMixin", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import not allowed edited schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' description='This is Test Schema' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='BaseElement' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00.00'>
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element1' modifier='Abstract'>
                        <BaseClass>BaseElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element2' modifier='Abstract'>
                        <BaseClass>Element1</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocated' modifier='Abstract'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocatedAttribution' modifier='Abstract'>
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                        <ECNavigationProperty propertyName = 'AttributedElement' relationshipName = 'ILinearlyLocatedAttributesElement' direction = 'Forward' displayLabel = 'Attributed Element'/>
                    </ECEntityClass>
                    <ECEntityClass typeName = 'ILinearElementSource'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00' modifier='Abstract'>
                            <AppliesToEntityClass>Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECRelationshipClass typeName = 'ILinearlyLocatedAttributesElement' strength = 'referencing' modifier='None'>
                        <Source multiplicity = '(0..*)' polymorphic = 'true' roleLabel = 'attributes'><Class class = 'ILinearlyLocatedAttribution'/></Source>
                        <Target multiplicity = '(0..1)' polymorphic = 'true' roleLabel = 'is attributed by'><Class class = 'Element2'/></Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateMixinRelationshipConstraintNoPolymorphs)
    {
    const auto SCHEMA1_HASH_ECDB_SCHEMA = "50dba9f402e569f78dfa736c4e19edd1c4633e37771f6d75d050c35745dbda05";
    const auto SCHEMA1_HASH_ECDB_MAP = "7fd0b7ef5b3520e219471363ea5c59afdd4ddd13958f1bbb580eaa54cd3f43f6";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "ff29f114c33fe47a161cbe7aee80437b8e8c3bd943ab5a38c4ae34af07100a69";
    Test(
        "import initial schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='BaseElement' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00.00'>
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element1' modifier='Abstract'>
                        <BaseClass>BaseElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element2' modifier='Abstract'>
                        <BaseClass>Element1</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocated' modifier='Abstract'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocatedAttribution' modifier='Abstract'>
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                        <ECNavigationProperty propertyName = 'AttributedElement' relationshipName = 'ILinearlyLocatedAttributesElement' direction = 'Forward' displayLabel = 'Attributed Element'/>
                    </ECEntityClass>
                    <ECEntityClass typeName = 'ILinearElementSource'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00' modifier='Abstract'>
                            <AppliesToEntityClass>Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECRelationshipClass typeName = 'ILinearlyLocatedAttributesElement' strength = 'referencing' modifier='None'>
                        <Source multiplicity = '(0..*)' polymorphic = 'true' roleLabel = 'attributes'><Class class = 'ILinearlyLocatedAttribution'/></Source>
                        <Target multiplicity = '(0..1)' polymorphic = 'true' roleLabel = 'is attributed by'><Class class = 'ILinearElementSource'/></Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("UpdateMixinRelationshipConstraintNoPolymorphs", schema));
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import not allowed edited schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' description='This is Test Schema' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='BaseElement' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00.00'>
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element1' modifier='Abstract'>
                        <BaseClass>BaseElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element2' modifier='Abstract'>
                        <BaseClass>Element1</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocated' modifier='Abstract'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocatedAttribution' modifier='Abstract'>
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                        <ECNavigationProperty propertyName = 'AttributedElement' relationshipName = 'ILinearlyLocatedAttributesElement' direction = 'Forward' displayLabel = 'Attributed Element'/>
                    </ECEntityClass>
                    <ECEntityClass typeName = 'ILinearElementSource'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00' modifier='Abstract'>
                            <AppliesToEntityClass>Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECRelationshipClass typeName = 'ILinearlyLocatedAttributesElement' strength = 'referencing' modifier='None'>
                        <Source multiplicity = '(0..*)' polymorphic = 'false' roleLabel = 'attributes'><Class class = 'ILinearlyLocatedAttribution'/></Source>
                        <Target multiplicity = '(0..1)' polymorphic = 'false' roleLabel = 'is attributed by'><Class class = 'Element1'/></Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateMixinRelationshipConstraintAcrossFiles)
    {
    Test(
        "Import initial BaseSchema schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "e6283be671f3c6456896cfd457f8a4d6eedc1c7c0bea21f8f2826a1009fa7eaf";
            const auto SCHEMA_HASH_ECDB_MAP = "8b979abbb9dd61bc9e5784a847e76d09ada26d9eaab8f9729cd5cce95317140a";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "2df78b6e0177d9f4c85d510e067e13ea7adc08caaf7e653d015b717679f55356";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='BaseSchema' alias='base' description='Holds base classes' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='BaseElement' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00.00'>
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element1' modifier='Abstract'>
                        <BaseClass>BaseElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element2' modifier='Abstract'>
                        <BaseClass>Element1</BaseClass>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("UpdateMixinRelationshipConstraintAcrossFiles", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    
    const auto SCHEMA1_HASH_ECDB_MAP = "736277e733963d032c0ddeaec92f005a59a257836f2024cfa05462b36ab3046f";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "88b6554b08bdf2eb97a7cd5dd97ca0da3727df37c1689e4649063d63835f2fdb";
    Test(
        "import TestSchema schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "b636b29d184c72bfb26b7c20ae4d1cc4762d72b5887a5623600993339a51b030";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='BaseSchema' version='01.00.00' alias='base' />
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='ILinearlyLocated' modifier='Abstract'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base:Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocatedAttribution' modifier='Abstract'>
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base:Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                        <ECNavigationProperty propertyName = 'AttributedElement' relationshipName = 'ILinearlyLocatedAttributesElement' direction = 'Forward' displayLabel = 'Attributed Element'/>
                    </ECEntityClass>
                    <ECEntityClass typeName = 'ILinearElementSource'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00' modifier='Abstract'>
                            <AppliesToEntityClass>base:Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECRelationshipClass typeName = 'ILinearlyLocatedAttributesElement' strength = 'referencing' modifier='None'>
                        <Source multiplicity = '(0..*)' polymorphic = 'true' roleLabel = 'attributes'><Class class = 'ILinearlyLocatedAttribution'/></Source>
                        <Target multiplicity = '(0..1)' polymorphic = 'true' roleLabel = 'is attributed by'><Class class = 'ILinearElementSource'/></Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import edited TestSchema schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "4c70c9542bf703ac67c24ed8f7222711cdb1824789b29365783fc766da61807b";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' description='This is Test Schema' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='BaseSchema' version='01.00.00' alias='base' />
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='ILinearlyLocated' modifier='Abstract'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base:Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocatedAttribution' modifier='Abstract'>
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base:Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                        <ECNavigationProperty propertyName = 'AttributedElement' relationshipName = 'ILinearlyLocatedAttributesElement' direction = 'Forward' displayLabel = 'Attributed Element'/>
                    </ECEntityClass>
                    <ECEntityClass typeName = 'ILinearElementSource'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00' modifier='Abstract'>
                            <AppliesToEntityClass>base:Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECRelationshipClass typeName = 'ILinearlyLocatedAttributesElement' strength = 'referencing' modifier='None'>
                        <Source multiplicity = '(0..*)' polymorphic = 'true' roleLabel = 'attributes'><Class class = 'ILinearlyLocatedAttribution'/></Source>
                        <Target multiplicity = '(0..1)' polymorphic = 'true' roleLabel = 'is attributed by'><Class class = 'base:Element1'/></Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, FailedMixinRelationshipConstraintAcrossFiles)
    {
    Test(
        "Import initial BaseSchema schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "8cc82573fd2e067fbd2c87ecbc6c34f0e4cfcb151739e7819223f831340c7ae1";
            const auto SCHEMA_HASH_ECDB_MAP = "8b979abbb9dd61bc9e5784a847e76d09ada26d9eaab8f9729cd5cce95317140a";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "2df78b6e0177d9f4c85d510e067e13ea7adc08caaf7e653d015b717679f55356";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='BaseSchema' alias='base' description='Holds base classes' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='BaseElement' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00.00'>
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element1' modifier='Abstract'>
                        <BaseClass>BaseElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element2' modifier='Abstract'>
                        <BaseClass>BaseElement</BaseClass>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("FailedMixinRelationshipConstraintAcrossFiles", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "f09be58693f7d68cdd9912230c27aa9565d2eecfaf439f8ef53e8328840eb596";
    const auto SCHEMA1_HASH_ECDB_MAP = "736277e733963d032c0ddeaec92f005a59a257836f2024cfa05462b36ab3046f";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "88b6554b08bdf2eb97a7cd5dd97ca0da3727df37c1689e4649063d63835f2fdb";
    Test(
        "import TestSchema schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='BaseSchema' version='01.00.00' alias='base' />
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='ILinearlyLocated' modifier='Abstract'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base:Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocatedAttribution' modifier='Abstract'>
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base:Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                        <ECNavigationProperty propertyName = 'AttributedElement' relationshipName = 'ILinearlyLocatedAttributesElement' direction = 'Forward' displayLabel = 'Attributed Element'/>
                    </ECEntityClass>
                    <ECEntityClass typeName = 'ILinearElementSource'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00' modifier='Abstract'>
                            <AppliesToEntityClass>base:Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECRelationshipClass typeName = 'ILinearlyLocatedAttributesElement' strength = 'referencing' modifier='None'>
                        <Source multiplicity = '(0..*)' polymorphic = 'true' roleLabel = 'attributes'><Class class = 'ILinearlyLocatedAttribution'/></Source>
                        <Target multiplicity = '(0..1)' polymorphic = 'true' roleLabel = 'is attributed by'><Class class = 'ILinearElementSource'/></Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import not allowed edited TestSchema schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' description='This is Test Schema' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='BaseSchema' version='01.00.00' alias='base' />
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='ILinearlyLocated' modifier='Abstract'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base:Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocatedAttribution' modifier='Abstract'>
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base:Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                        <ECNavigationProperty propertyName = 'AttributedElement' relationshipName = 'ILinearlyLocatedAttributesElement' direction = 'Forward' displayLabel = 'Attributed Element'/>
                    </ECEntityClass>
                    <ECEntityClass typeName = 'ILinearElementSource'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00' modifier='Abstract'>
                            <AppliesToEntityClass>base:Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECRelationshipClass typeName = 'ILinearlyLocatedAttributesElement' strength = 'referencing' modifier='None'>
                        <Source multiplicity = '(0..*)' polymorphic = 'true' roleLabel = 'attributes'><Class class = 'ILinearlyLocatedAttribution'/></Source>
                        <Target multiplicity = '(0..1)' polymorphic = 'true' roleLabel = 'is attributed by'><Class class = 'base:Element1'/></Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, UpdateMixinRelationshipConstraintAcrossMultiFiles)
    {
    Test(
        "Import initial BaseSchema schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "3aa1a57b93d31a79cf55f3afa779fb11751825858c66b21d927fa98a8dfed21d";
            const auto SCHEMA_HASH_ECDB_MAP = "d115cdd34a808986a3d360510fee3be9edb0db7221a6543513d6e3c3564d8aea";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "0342b920a1b694aa41064dba9c68044e717faeeb3b785123bfe46546d74b80e1";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='BaseSchema' alias='base1' description='Holds base classes' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='BaseElement' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00.00'>
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element1' modifier='Abstract'>
                        <BaseClass>BaseElement</BaseClass>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("UpdateMixinRelationshipConstraintAcrossMultiFiles", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import BaseSchema2 schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "5e558436ad82987ae29b13f591772ffac51c79bafde17acb966348a422657cec";
            const auto SCHEMA_HASH_ECDB_MAP = "2a73a5d77fd887a494c931f5fa366b43764755a5342825f4a6df8bcea2cd7f80";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "0342b920a1b694aa41064dba9c68044e717faeeb3b785123bfe46546d74b80e1";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='BaseSchema2' alias='base2' description='Holds more base classes' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='BaseSchema' version='01.00.00' alias='base1' />
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='Element2' modifier='Abstract'>
                        <BaseClass>base1:Element1</BaseClass>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import TestSchema schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "eefca43484628966f68ee30899895c8c864bf0d400af488c8920d8d42a6ab35f";
            const auto SCHEMA_HASH_ECDB_MAP = "42d46d24970bcdfd196edc29de8c9a5c1f7874d43c0960673512306c8db3d782";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "0342b920a1b694aa41064dba9c68044e717faeeb3b785123bfe46546d74b80e1";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='BaseSchema2' version='01.00.00' alias='base2' />
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='ILinearlyLocated' modifier='Abstract'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base2:Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocatedAttribution' modifier='Abstract'>
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base2:Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                        <ECNavigationProperty propertyName = 'AttributedElement' relationshipName = 'ILinearlyLocatedAttributesElement' direction = 'Forward' displayLabel = 'Attributed Element'/>
                    </ECEntityClass>
                    <ECRelationshipClass typeName = 'ILinearlyLocatedAttributesElement' strength = 'referencing' modifier='None'>
                        <Source multiplicity = '(0..*)' polymorphic = 'true' roleLabel = 'attributes'><Class class = 'ILinearlyLocatedAttribution'/></Source>
                        <Target multiplicity = '(0..1)' polymorphic = 'true' roleLabel = 'is attributed by'><Class class = 'base2:Element2'/></Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import edited TestSchema schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "fdb79f95329f1828ccafa86f1bd11739497308838b84707e41921104383f23d6";
            const auto SCHEMA_HASH_ECDB_MAP = "42d46d24970bcdfd196edc29de8c9a5c1f7874d43c0960673512306c8db3d782";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "0342b920a1b694aa41064dba9c68044e717faeeb3b785123bfe46546d74b80e1";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' description='This is Test Schema' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='BaseSchema' version='01.00.00' alias='base1' />
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='ILinearlyLocated' modifier='Abstract'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base1:Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocatedAttribution' modifier='Abstract'>
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base1:Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                        <ECNavigationProperty propertyName = 'AttributedElement' relationshipName = 'ILinearlyLocatedAttributesElement' direction = 'Forward' displayLabel = 'Attributed Element'/>
                    </ECEntityClass>
                    <ECRelationshipClass typeName = 'ILinearlyLocatedAttributesElement' strength = 'referencing' modifier='None'>
                        <Source multiplicity = '(0..*)' polymorphic = 'true' roleLabel = 'attributes'><Class class = 'ILinearlyLocatedAttribution'/></Source>
                        <Target multiplicity = '(0..1)' polymorphic = 'true' roleLabel = 'is attributed by'><Class class = 'base1:Element1'/></Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, FailMixinRelationshipConstraintAcrossMultiFiles)
    {
    Test(
        "Import initial BaseSchema schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "3aa1a57b93d31a79cf55f3afa779fb11751825858c66b21d927fa98a8dfed21d";
            const auto SCHEMA_HASH_ECDB_MAP = "d115cdd34a808986a3d360510fee3be9edb0db7221a6543513d6e3c3564d8aea";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "0342b920a1b694aa41064dba9c68044e717faeeb3b785123bfe46546d74b80e1";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='BaseSchema' alias='base1' description='Holds base classes' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='BaseElement' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00.00'>
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element1' modifier='Abstract'>
                        <BaseClass>BaseElement</BaseClass>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("FailMixinRelationshipConstraintMultiFileVersioning", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import BaseSchema2 schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "5e558436ad82987ae29b13f591772ffac51c79bafde17acb966348a422657cec";
            const auto SCHEMA_HASH_ECDB_MAP = "2a73a5d77fd887a494c931f5fa366b43764755a5342825f4a6df8bcea2cd7f80";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "0342b920a1b694aa41064dba9c68044e717faeeb3b785123bfe46546d74b80e1";
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='BaseSchema2' alias='base2' description='Holds more base classes' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='BaseSchema' version='01.00.00' alias='base1' />
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='Element2' modifier='Abstract'>
                        <BaseClass>base1:Element1</BaseClass>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "eefca43484628966f68ee30899895c8c864bf0d400af488c8920d8d42a6ab35f";
    const auto SCHEMA1_HASH_ECDB_MAP = "42d46d24970bcdfd196edc29de8c9a5c1f7874d43c0960673512306c8db3d782";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "0342b920a1b694aa41064dba9c68044e717faeeb3b785123bfe46546d74b80e1";
    Test(
        "import TestSchema schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='BaseSchema2' version='01.00.00' alias='base2' />
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='ILinearlyLocated' modifier='Abstract'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base2:Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocatedAttribution' modifier='Abstract'>
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base2:Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                        <ECNavigationProperty propertyName = 'AttributedElement' relationshipName = 'ILinearlyLocatedAttributesElement' direction = 'Forward' displayLabel = 'Attributed Element'/>
                    </ECEntityClass>
                    <ECRelationshipClass typeName = 'ILinearlyLocatedAttributesElement' strength = 'referencing' modifier='None'>
                        <Source multiplicity = '(0..*)' polymorphic = 'true' roleLabel = 'attributes'><Class class = 'ILinearlyLocatedAttribution'/></Source>
                        <Target multiplicity = '(0..1)' polymorphic = 'true' roleLabel = 'is attributed by'><Class class = 'base2:Element2'/></Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import not allowed edited TestSchema schema",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' description='This is Test Schema' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='ILinearlyLocated' modifier='Abstract'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base1:Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocatedAttribution' modifier='Abstract'>
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base1:Element1</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                        <ECNavigationProperty propertyName = 'AttributedElement' relationshipName = 'ILinearlyLocatedAttributesElement' direction = 'Forward' displayLabel = 'Attributed Element'/>
                    </ECEntityClass>
                    <ECRelationshipClass typeName = 'ILinearlyLocatedAttributesElement' strength = 'referencing' modifier='None'>
                        <Source multiplicity = '(0..*)' polymorphic = 'true' roleLabel = 'attributes'><Class class = 'ILinearlyLocatedAttribution'/></Source>
                        <Target multiplicity = '(0..1)' polymorphic = 'true' roleLabel = 'is attributed by'><Class class = 'base1:Element1'/></Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, FailMixinRelationshipConstraintMultiFileVersioning)
    {
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "0342b920a1b694aa41064dba9c68044e717faeeb3b785123bfe46546d74b80e1";
    Test(
        "Import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "7ae06651c57adc149df8765158a9e135711d4c004f055840df71762b81a79b9f";
            const auto SCHEMA_HASH_ECDB_MAP = "2a73a5d77fd887a494c931f5fa366b43764755a5342825f4a6df8bcea2cd7f80";
            
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='BaseSchema' alias='base1' description='Holds base classes' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='BaseElement' modifier='Abstract'>
                        <ECCustomAttributes>
                            <ClassMap xmlns='ECDbMap.02.00.00'>
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns='ECDbMap.02.00.00'>
                                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                            </ShareColumns>
                        </ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element1' modifier='Abstract'>
                        <BaseClass>BaseElement</BaseClass>
                    </ECEntityClass>
                    <ECEntityClass typeName='Element2' modifier='Abstract'>
                        <BaseClass>Element1</BaseClass>
                    </ECEntityClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("FailMixinRelationshipConstraintMultiFileVersioning", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    const auto SCHEMA2_HASH_ECDB_SCHEMA = "c92d66e490be72769d7f9be1cfe10c4b82a41b6c5bd7942eef904429ccd81cdb";
    const auto SCHEMA2_HASH_ECDB_MAP = "42d46d24970bcdfd196edc29de8c9a5c1f7874d43c0960673512306c8db3d782";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<?xml version='1.0' encoding='utf-8'?>
                <ECSchema schemaName='TestSchema' alias='ts' description='This is Test Schema' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                <ECSchemaReference name='BaseSchema' version='01.00.00' alias='base1' />
                <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                    <ECEntityClass typeName='ILinearlyLocated' modifier='Abstract'>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base1:Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                    </ECEntityClass>
                    <ECEntityClass typeName='ILinearlyLocatedAttribution' modifier='Abstract'>
                        <BaseClass>ILinearlyLocated</BaseClass>
                        <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                            <AppliesToEntityClass>base1:Element2</AppliesToEntityClass>
                        </IsMixin></ECCustomAttributes>
                        <ECNavigationProperty propertyName = 'AttributedElement' relationshipName = 'ILinearlyLocatedAttributesElement' direction = 'Forward' displayLabel = 'Attributed Element'/>
                    </ECEntityClass>
                    <ECRelationshipClass typeName = 'ILinearlyLocatedAttributesElement' strength = 'referencing' modifier='None'>
                        <Source multiplicity = '(0..*)' polymorphic = 'true' roleLabel = 'attributes'><Class class = 'ILinearlyLocatedAttribution'/></Source>
                        <Target multiplicity = '(0..1)' polymorphic = 'true' roleLabel = 'is attributed by'><Class class = 'base1:Element2'/></Target>
                    </ECRelationshipClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );

    Test(
        "import not allowed schemas",
        [&]()
            {
            auto schemas = {
                SchemaItem(
                    R"xml(<?xml version='1.0' encoding='utf-8'?>
                    <ECSchema schemaName='BaseSchema' alias='base1' description='Holds base classes' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                        <ECEntityClass typeName='BaseElement' modifier='Abstract'>
                            <ECCustomAttributes>
                                <ClassMap xmlns='ECDbMap.02.00.00'>
                                    <MapStrategy>TablePerHierarchy</MapStrategy>
                                </ClassMap>
                                <ShareColumns xmlns='ECDbMap.02.00.00'>
                                    <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                                    <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                                </ShareColumns>
                            </ECCustomAttributes>
                        </ECEntityClass>
                        <ECEntityClass typeName='Element1' modifier='Abstract'>
                            <BaseClass>BaseElement</BaseClass>
                        </ECEntityClass>
                    </ECSchema>)xml"
                ),
                SchemaItem(
                    R"xml(<?xml version='1.0' encoding='utf-8'?>
                    <ECSchema schemaName='TestSchema' alias='ts' description='This is Test Schema' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
                    <ECSchemaReference name='BaseSchema' version='01.00.01' alias='base1' />
                    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                        <ECEntityClass typeName='ILinearlyLocated' modifier='Abstract'>
                            <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                                <AppliesToEntityClass>base1:Element1</AppliesToEntityClass>
                            </IsMixin></ECCustomAttributes>
                        </ECEntityClass>
                        <ECEntityClass typeName='ILinearlyLocatedAttribution' modifier='Abstract'>
                            <BaseClass>ILinearlyLocated</BaseClass>
                            <ECCustomAttributes> <IsMixin xmlns='CoreCustomAttributes.01.00.00'>
                                <AppliesToEntityClass>base1:Element1</AppliesToEntityClass>
                            </IsMixin></ECCustomAttributes>
                            <ECNavigationProperty propertyName = 'AttributedElement' relationshipName = 'ILinearlyLocatedAttributesElement' direction = 'Forward' displayLabel = 'Attributed Element'/>
                        </ECEntityClass>
                        <ECRelationshipClass typeName = 'ILinearlyLocatedAttributesElement' strength = 'referencing' modifier='None'>
                            <Source multiplicity = '(0..*)' polymorphic = 'true' roleLabel = 'attributes'><Class class = 'ILinearlyLocatedAttribution'/></Source>
                            <Target multiplicity = '(0..1)' polymorphic = 'true' roleLabel = 'is attributed by'><Class class = 'base1:Element1'/></Target>
                        </ECRelationshipClass>
                    </ECSchema>)xml"
                )
            };
            ASSERT_EQ(SchemaImportResult::ERROR, ImportSchemas(*m_briefcase, schemas, SchemaManager::SchemaImportOptions::None, m_schemaChannel->GetChannelUri()));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->AbandonChanges());
            CheckHashes(*m_briefcase, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA2_HASH_ECDB_SCHEMA, SCHEMA2_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, OverflowedStructClass_NestedStruct)
    {
    Test(
        "Import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "65db43b14a2585e983b2401fee9fa856ca73c9f06b6ef87b20301925c38b46c4";
            const auto SCHEMA_HASH_ECDB_MAP = "69c97fac1f94d513495caf65e1722fc0fcbd4f414b17910fe446117bfbcccb10";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "e567d8390f0b253a968210f626c58f7be75b1f306d6e3def12f575cb576256a2";
            auto schema = SchemaItem(
                R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element">
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECStructProperty propertyName="S" typeName="S" />
                    </ECEntityClass>
                    <ECStructClass typeName="S" modifier="None">
                        <ECStructProperty propertyName="T" typeName="T" />
                        <ECStructArrayProperty propertyName="T_ARRAY" typeName="T" minOccurs="0" maxOccurs="unbounded"/>
                    </ECStructClass>
                    <ECStructClass typeName="T" modifier="None">
                        <ECProperty propertyName="I" typeName="int"/>
                        <ECProperty propertyName="P2D" typeName="point2d"/>
                        <ECArrayProperty propertyName="P2D_ARRAY" typeName="point2d" minOccurs="0" maxOccurs="unbounded"/>
                    </ECStructClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("struct_prop", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify before schema map for struct property this will change after v2 import",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Element");
            Json::Value expected = R"(
                [
                    "TestSchema:Element:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Element:ECInstanceId:ts_Element:Id",
                    "TestSchema:Element:S.T.I:ts_Element:ps1",
                    "TestSchema:Element:S.T.P2D.X:ts_Element:ps2",
                    "TestSchema:Element:S.T.P2D.Y:ts_Element:ps3",
                    "TestSchema:Element:S.T.P2D_ARRAY:ts_Element:ps4",
                    "TestSchema:Element:S.T_ARRAY:ts_Element:ps5"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );
    auto inst1 = R"({
        "className": "ts.Element",
        "data": {
            "S": {
                "T": {
                    "I": 7242,
                    "P2D" : {"x": 3793.0, "y": 5133.0},
                    "P2D_ARRAY": [
                        {"x": 1183.0, "y": 1243.0},
                        {"x": 1805.0, "y": 9804.0},
                        {"x": 7940.0, "y": 1088.0}
                        ]
                },
                "T_ARRAY": [{
                    "I": 7765,
                    "P2D" : {"x": 8360.0, "y": 8429.0},
                    "P2D_ARRAY": [
                        {"x": 4662.0, "y": 9892.0},
                        {"x": 5177.0, "y": 5730.0},
                        {"x": 1030.0, "y": 5774.0}
                        ]
                    }
                ]
            }
        }
    })"_json;

    auto key1 = InsertInstance(*m_briefcase, inst1);
    Test(
        "verify instance was written correctly",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key1, "S");
            ASSERT_STREQ(inst1["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "c703d2256f4f9143b2696b1c4714f672064b863a6a122467f75f8c2b67643069";
    const auto SCHEMA1_HASH_ECDB_MAP = "ff333615117c5dd2bc1a1aae1e93013942cc59f18297a15d93ef310caf13db93";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "dbdde34ca352550fe87ae4c0a8bc7150d45089a4093b2fff5fd3439c3fc1a884";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element">
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECStructProperty propertyName="S" typeName="S" />
                        <ECProperty propertyName="L" typeName="int"/>
                    </ECEntityClass>
                    <ECStructClass typeName="S" modifier="None">
                        <ECStructProperty propertyName="T" typeName="T" />
                        <ECStructArrayProperty propertyName="T_ARRAY" typeName="T" minOccurs="0" maxOccurs="unbounded"/>
                    </ECStructClass>
                    <ECStructClass typeName="T" modifier="None">
                        <ECProperty propertyName="I" typeName="int"/>
                        <ECProperty propertyName="P2D" typeName="point2d"/>
                        <ECProperty propertyName="P3D" typeName="point3d"/>
                        <ECArrayProperty propertyName="P2D_ARRAY" typeName="point2d" minOccurs="0" maxOccurs="unbounded"/>
                    </ECStructClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "verify property map after schema upgrade",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Element");
            Json::Value expected = R"(
                [
                    "TestSchema:Element:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Element:ECClassId:ts_Element_Overflow:ECClassId",
                    "TestSchema:Element:ECInstanceId:ts_Element:Id",
                    "TestSchema:Element:ECInstanceId:ts_Element_Overflow:Id",
                    "TestSchema:Element:L:ts_Element:ps6",
                    "TestSchema:Element:S.T.I:ts_Element_Overflow:os4",
                    "TestSchema:Element:S.T.P2D.X:ts_Element_Overflow:os5",
                    "TestSchema:Element:S.T.P2D.Y:ts_Element_Overflow:os6",
                    "TestSchema:Element:S.T.P2D_ARRAY:ts_Element_Overflow:os7",
                    "TestSchema:Element:S.T.P3D.X:ts_Element_Overflow:os1",
                    "TestSchema:Element:S.T.P3D.Y:ts_Element_Overflow:os2",
                    "TestSchema:Element:S.T.P3D.Z:ts_Element_Overflow:os3",
                    "TestSchema:Element:S.T_ARRAY:ts_Element_Overflow:os8"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );

    auto inst2 = R"({
        "className": "ts.Element",
        "data": {
            "L": 1910,
            "S": {
                "T": {
                    "I": 4853,
                    "P2D" : {"x": 6484.0, "y": 2779.0},
                    "P3D" : {"x": 1131.0, "y": 1540.0, "z": 4873.0},
                    "P2D_ARRAY": [
                        {"x": 7712.0, "y": 1171.0},
                        {"x": 5858.0, "y": 6044.0},
                        {"x": 5606.0, "y": 2163.0}
                        ]
                },
                "T_ARRAY": [{
                    "I": 2839,
                    "P2D" : {"x": 7595.0, "y": 8429.0},
                    "P3D" : {"x": 8397.0, "y": 3303.0, "z": 1096.0},
                    "P2D_ARRAY": [
                        {"x": 4662.0, "y": 2686.0},
                        {"x": 1996.0, "y": 6779.0},
                        {"x": 8576.0, "y": 9819.0}
                        ]
                    }
                ]
            }
        }
    })"_json;
    auto key2 = InsertInstance(*m_briefcase, inst2);

    Test(
        "verify instance was transformed correctly after schema import",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key1, "S");
            ASSERT_STREQ(inst1["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "verify instance inserted after schema upgrade",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key2, "L,S");
            ASSERT_STREQ(inst2["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "Make sure the column where property used to reside is set to null",
        [&]()
            {
            auto vs = m_briefcase->GetCachedStatement("SELECT ps1, ps2, ps3, ps4, ps5 FROM ts_element where id = ?");
            vs->BindId(1, key1.GetInstanceId());
            ASSERT_EQ(BE_SQLITE_ROW, vs->Step());
            ASSERT_TRUE(vs->IsColumnNull(0)) << "Expect ps1 to be null";
            ASSERT_TRUE(vs->IsColumnNull(1)) << "Expect ps2 to be null";
            ASSERT_TRUE(vs->IsColumnNull(2)) << "Expect ps3 to be null";
            ASSERT_TRUE(vs->IsColumnNull(3)) << "Expect ps4 to be null";
            ASSERT_TRUE(vs->IsColumnNull(4)) << "Expect ps5 to be null";
            }
    );

    Test(
        "Final hash check",
        [&]()
            {
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, OverflowedStructClass_Simple)
    {
    Test(
        "Import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "bbb6bcb18fcdc074f74decefadd33f48f5cc899cfdf3f4ae95553e881e7cc085";
            const auto SCHEMA_HASH_ECDB_MAP = "efb2203a0db50751d6ee39ffb4097b664af42dcad0c296f6307024338459228c";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "372b9817ba28a59811387f40c58bb6b6e2d25f854aee946216c37495c99fb16c";
            auto schema = SchemaItem(
                R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element">
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECStructProperty propertyName="structProp" typeName="S" />
                    </ECEntityClass>
                    <ECStructClass typeName="S" modifier="None">
                        <ECProperty propertyName="P1" typeName="int" />
                        <ECProperty propertyName="P2" typeName="int" />
                        <ECProperty propertyName="P3" typeName="int" />
                        <ECProperty propertyName="P4" typeName="int" />
                    </ECStructClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("struct_prop", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "Verify before schema map for struct property this will change after v2 importe",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Element");
            Json::Value expected = R"(
                [
                    "TestSchema:Element:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Element:ECInstanceId:ts_Element:Id",
                    "TestSchema:Element:structProp.P1:ts_Element:ps1",
                    "TestSchema:Element:structProp.P2:ts_Element:ps2",
                    "TestSchema:Element:structProp.P3:ts_Element:ps3",
                    "TestSchema:Element:structProp.P4:ts_Element:ps4"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );

    auto inst1 = R"({
        "className": "ts.Element",
        "data": {
            "structProp": {
                "P1": 2210,
                "P2": 6703,
                "P3": 8481,
                "P4": 5339
            }
        }
    })"_json;

    auto key1 = InsertInstance(*m_briefcase, inst1);
    Test(
        "verify instance was written correctlye",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key1, "structProp");
            ASSERT_STREQ(inst1["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "f0302f6e61eb9a203726835757419db1e0ace30646e380ea858b1b83b18dc379";
    const auto SCHEMA1_HASH_ECDB_MAP = "dda7c3a60923cb4f4f40e4251df34d47a716f65e88cef94a2be0511b39d4e527";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "84bda55de37b3c29de3a3bc49fffb942f5b4a688b58e5eaa947b90bd7fa65325";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element">
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                    <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECStructProperty propertyName="structProp" typeName="S" />
                    </ECEntityClass>
                    <ECStructClass typeName="S" modifier="None">
                        <ECProperty propertyName="P1" typeName="int" />
                        <ECProperty propertyName="P2" typeName="int" />
                        <ECProperty propertyName="P3" typeName="int" />
                        <ECProperty propertyName="P4" typeName="int" />
                        <ECProperty propertyName="P5" typeName="int" />
                        <ECProperty propertyName="P6" typeName="int" />
                    </ECStructClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "verify property map after schema upgrade",
        [&]()
        {
        Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Element");
        Json::Value expected = R"(
            [
                "TestSchema:Element:ECClassId:ts_Element:ECClassId",
                "TestSchema:Element:ECClassId:ts_Element_Overflow:ECClassId",
                "TestSchema:Element:ECInstanceId:ts_Element:Id",
                "TestSchema:Element:ECInstanceId:ts_Element_Overflow:Id",
                "TestSchema:Element:structProp.P1:ts_Element_Overflow:os3",
                "TestSchema:Element:structProp.P2:ts_Element_Overflow:os4",
                "TestSchema:Element:structProp.P3:ts_Element_Overflow:os5",
                "TestSchema:Element:structProp.P4:ts_Element_Overflow:os6",
                "TestSchema:Element:structProp.P5:ts_Element_Overflow:os1",
                "TestSchema:Element:structProp.P6:ts_Element_Overflow:os2"
            ]
        )"_json;
        ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
        }
    );

    auto inst2 = R"({
        "className": "ts.Element",
        "data": {
            "structProp": {
                "P1": 3472,
                "P2": 4663,
                "P3": 8695,
                "P4": 9963,
                "P5": 5040,
                "P6": 2494
            }
        }
    })"_json;
    auto key2 = InsertInstance(*m_briefcase, inst2);

    Test(
        "verify instance was transformed correctly after schema import",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key1, "structProp");
            ASSERT_STREQ(inst1["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "verify instance inserted after schema upgrade",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key2, "structProp");
            ASSERT_STREQ(inst2["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "Make sure the column where property used to reside is set to null",
        [&]()
            {
            auto vs = m_briefcase->GetCachedStatement("SELECT ps1, ps2, ps3, ps4 FROM ts_element where id = ?");
            vs->BindId(1, key1.GetInstanceId());
            ASSERT_EQ(BE_SQLITE_ROW, vs->Step());
            ASSERT_TRUE(vs->IsColumnNull(0)) << "Expect ps1 to be null";
            ASSERT_TRUE(vs->IsColumnNull(1)) << "Expect ps2 to be null";
            ASSERT_TRUE(vs->IsColumnNull(2)) << "Expect ps3 to be null";
            ASSERT_TRUE(vs->IsColumnNull(3)) << "Expect ps4 to be null";
            }
    );

    Test(
        "Final hash check",
        [&]()
            {
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, OverflowedStructClass_OverflowTableDoesNotExist_CheckDataMoveCorrectly)
    {
    Test(
        "Import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "c5200fb69896736a0aaaf2e4932f694b7368ff9a525c0b51a848913e760727f5";
            const auto SCHEMA_HASH_ECDB_MAP = "0c2517eea45e5719f75e11bfc33cb9508f9ea880493c703b48dc7e37dc7e5279";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "e567d8390f0b253a968210f626c58f7be75b1f306d6e3def12f575cb576256a2";
            auto schema = SchemaItem(
                R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element">
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                    <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECStructProperty propertyName="structProp" typeName="S" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Geom2d">
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName="G1" typeName="int" />
                    </ECEntityClass>
                    <ECStructClass typeName="S" modifier="None">
                        <ECProperty propertyName="P1" typeName="int" />
                        <ECProperty propertyName="P2" typeName="int" />
                        <ECProperty propertyName="P3" typeName="int" />
                        <ECProperty propertyName="P4" typeName="int" />
                    </ECStructClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("struct_prop", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "verify element mapping before schema upgrade",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Element");
            Json::Value expected = R"(
                [
                    "TestSchema:Element:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Element:ECInstanceId:ts_Element:Id",
                    "TestSchema:Element:structProp.P1:ts_Element:ps1",
                    "TestSchema:Element:structProp.P2:ts_Element:ps2",
                    "TestSchema:Element:structProp.P3:ts_Element:ps3",
                    "TestSchema:Element:structProp.P4:ts_Element:ps4"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );
    Test(
        "verify geom2d mapping before schema upgrade",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Geom2d");
            Json::Value expected = R"(
                [
                    "TestSchema:Geom2d:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Geom2d:ECInstanceId:ts_Element:Id",
                    "TestSchema:Geom2d:G1:ts_Element:ps5",
                    "TestSchema:Geom2d:structProp.P1:ts_Element:ps1",
                    "TestSchema:Geom2d:structProp.P2:ts_Element:ps2",
                    "TestSchema:Geom2d:structProp.P3:ts_Element:ps3",
                    "TestSchema:Geom2d:structProp.P4:ts_Element:ps4"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );

    auto inst1 = R"({
        "className": "ts.Element",
        "data": {
            "structProp": {
                "P1": 3194,
                "P2": 7305,
                "P3": 3092,
                "P4": 9935
            }
        }
    })"_json;
    auto key1 = InsertInstance(*m_briefcase, inst1);
    Test(
        "verify element instance",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key1, "structProp");
            ASSERT_STREQ(inst1["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );

    auto inst2 = R"({
        "className": "ts.Geom2d",
        "data": {
            "G1": 9247,
            "structProp": {
                "P1": 5627,
                "P2": 9315,
                "P3": 9333,
                "P4": 6570
            }
        }
    })"_json;
    auto key2 = InsertInstance(*m_briefcase, inst2);
    Test(
        "verify geom2d instance",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key2, "G1, structProp");
            ASSERT_STREQ(inst2["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "dd15f16c1053022d6df3f6a8ceb55f2d8f77a4140ef12f4cf3a199861724514a";
    const auto SCHEMA1_HASH_ECDB_MAP = "d6cd2aef1ea2160ed150483729e0203f321e65f67fcf1776e81954cf8038566b";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "e7775dfcaeb21886063311e476adc049aabbfdd5fdf87254ff597e0b448759ed";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element">
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                    <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECStructProperty propertyName="structProp" typeName="S" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Geom2d">
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName="G1" typeName="int" />
                    </ECEntityClass>
                    <ECStructClass typeName="S" modifier="None">
                        <ECProperty propertyName="P1" typeName="int" />
                        <ECProperty propertyName="P2" typeName="int" />
                        <ECProperty propertyName="P3" typeName="int" />
                        <ECProperty propertyName="P4" typeName="int" />
                        <ECProperty propertyName="P5" typeName="int" />
                        <ECProperty propertyName="P6" typeName="int" />
                    </ECStructClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "verify map for element",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Element");
            Json::Value expected = R"(
                [
                    "TestSchema:Element:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Element:ECClassId:ts_Element_Overflow:ECClassId",
                    "TestSchema:Element:ECInstanceId:ts_Element:Id",
                    "TestSchema:Element:ECInstanceId:ts_Element_Overflow:Id",
                    "TestSchema:Element:structProp.P1:ts_Element_Overflow:os3",
                    "TestSchema:Element:structProp.P2:ts_Element_Overflow:os4",
                    "TestSchema:Element:structProp.P3:ts_Element_Overflow:os5",
                    "TestSchema:Element:structProp.P4:ts_Element_Overflow:os6",
                    "TestSchema:Element:structProp.P5:ts_Element_Overflow:os1",
                    "TestSchema:Element:structProp.P6:ts_Element_Overflow:os2"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );
    Test(
        "verify map for geom2d",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Geom2d");
            Json::Value expected = R"(
                [
                    "TestSchema:Geom2d:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Geom2d:ECClassId:ts_Element_Overflow:ECClassId",
                    "TestSchema:Geom2d:ECInstanceId:ts_Element:Id",
                    "TestSchema:Geom2d:ECInstanceId:ts_Element_Overflow:Id",
                    "TestSchema:Geom2d:G1:ts_Element:ps5",
                    "TestSchema:Geom2d:structProp.P1:ts_Element_Overflow:os3",
                    "TestSchema:Geom2d:structProp.P2:ts_Element_Overflow:os4",
                    "TestSchema:Geom2d:structProp.P3:ts_Element_Overflow:os5",
                    "TestSchema:Geom2d:structProp.P4:ts_Element_Overflow:os6",
                    "TestSchema:Geom2d:structProp.P5:ts_Element_Overflow:os1",
                    "TestSchema:Geom2d:structProp.P6:ts_Element_Overflow:os2"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );
    // insert a second instance with additional properties
    auto inst3 = R"({
        "className": "ts.Element",
        "data": {
            "structProp": {
                "P1": 4855,
                "P2": 3449,
                "P3": 2623,
                "P4": 6854,
                "P5": 3769,
                "P6": 9612
            }
        }
    })"_json;
    auto key3 = InsertInstance(*m_briefcase, inst3);

    auto inst4 = R"({
        "className": "ts.Geom2d",
        "data": {
            "G1": 2866,
            "structProp": {
                "P1": 9663,
                "P2": 6961,
                "P3": 5681,
                "P4": 6747,
                "P5": 2143,
                "P6": 3404
            }
        }
    })"_json;
    auto key4 = InsertInstance(*m_briefcase, inst4);

    Test(
        "check element before schema upgrade",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key1, "structProp");
            ASSERT_STREQ(inst1["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "check geom2d before schema upgrade",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key2, "G1, structProp");
            ASSERT_STREQ(inst2["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "check element after schema upgrade",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key3, "structProp");
            ASSERT_STREQ(inst3["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "check geom3d after schema upgrade",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key4, "G1, structProp");
            ASSERT_STREQ(inst4["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "check data was moved and left behind",
        [&]()
            {
            auto vs = m_briefcase->GetCachedStatement("SELECT ps1, ps2, ps3, ps4 FROM ts_element where id = ?");
            vs->BindId(1, key1.GetInstanceId());
            ASSERT_EQ(BE_SQLITE_ROW, vs->Step());
            ASSERT_TRUE (vs->IsColumnNull(0))  << "Expect ps1 to be null";
            ASSERT_TRUE (vs->IsColumnNull(1))  << "Expect ps2 to be null";
            ASSERT_TRUE (vs->IsColumnNull(2))  << "Expect ps3 to be null";
            ASSERT_TRUE (vs->IsColumnNull(3))  << "Expect ps4 to be null";
            ASSERT_TRUE (vs->IsColumnNull(4))  << "Expect ps5 to be not null not used by element";
            }
    );
    Test(
        "check data was moved and left behind",
        [&]()
            {
            auto vs = m_briefcase->GetCachedStatement("SELECT ps1, ps2, ps3, ps4, ps5 FROM ts_element where id = ?");
            vs->BindId(1, key2.GetInstanceId());
            ASSERT_EQ(BE_SQLITE_ROW, vs->Step());
            ASSERT_TRUE (vs->IsColumnNull(0))  << "Expect ps1 to be null";
            ASSERT_TRUE (vs->IsColumnNull(1))  << "Expect ps2 to be null";
            ASSERT_TRUE (vs->IsColumnNull(2))  << "Expect ps3 to be null";
            ASSERT_TRUE (vs->IsColumnNull(3))  << "Expect ps4 to be null";
            ASSERT_FALSE(vs->IsColumnNull(4))  << "Expect ps5 to be not null (map to g1)";
            }
    );
    Test(
        "Final hash check",
        [&]()
            {
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// Cant import initial schema and setup ecdb, without import options?
TEST_F(SchemaSyncTestFixture, OverflowedStructClass_OverflowTableAlreadyExist_CheckDataMoveCorrectly)
    {
    Test(
        "Import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "ce510eb7d0be3a75330f873efb678f2d688cea07092076cc82b56b4b80ac2f35";
            const auto SCHEMA_HASH_ECDB_MAP = "2eae9b435cceabd71de698f1dd8e9cba678b8121108f7ce1fcc3e51860813950";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "b8e94c37841400f9757e0e288442da9f1b0ac9256478492b694d3b6af807d30e";
            auto schema = SchemaItem(
                R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element">
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                    <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECStructProperty propertyName="structProp" typeName="S" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Geom2d">
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName="G1" typeName="int" />
                    </ECEntityClass>
                    <ECStructClass typeName="S" modifier="None">
                        <ECProperty propertyName="P1" typeName="int" />
                        <ECProperty propertyName="P2" typeName="int" />
                        <ECProperty propertyName="P3" typeName="int" />
                        <ECProperty propertyName="P4" typeName="int" />
                    </ECStructClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("struct_prop", schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade)); // Fails?????? Should be OK and uncomment the lower code
            // ASSERT_EQ(SchemaImportResult::OK, SetupECDb("struct_prop", schema)); // Fails?????? Should be OK and uncomment the lower code
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );

    Test(
        "verify element mapping before schema upgrade",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Element");
            Json::Value expected = R"(
                [
                    "TestSchema:Element:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Element:ECInstanceId:ts_Element:Id",
                    "TestSchema:Element:structProp.P1:ts_Element:ps1",
                    "TestSchema:Element:structProp.P2:ts_Element:ps2",
                    "TestSchema:Element:structProp.P3:ts_Element:ps3",
                    "TestSchema:Element:structProp.P4:ts_Element:ps4"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );
    Test(
        "verify geom2d mapping before schema upgrade",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Geom2d");
            Json::Value expected = R"(
                [
                    "TestSchema:Geom2d:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Geom2d:ECClassId:ts_Element_Overflow:ECClassId",
                    "TestSchema:Geom2d:ECInstanceId:ts_Element:Id",
                    "TestSchema:Geom2d:ECInstanceId:ts_Element_Overflow:Id",
                    "TestSchema:Geom2d:G1:ts_Element_Overflow:os1",
                    "TestSchema:Geom2d:structProp.P1:ts_Element:ps1",
                    "TestSchema:Geom2d:structProp.P2:ts_Element:ps2",
                    "TestSchema:Geom2d:structProp.P3:ts_Element:ps3",
                    "TestSchema:Geom2d:structProp.P4:ts_Element:ps4"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );

    auto inst1 = R"({
        "className": "ts.Element",
        "data": {
            "structProp": {
                "P1": 3194,
                "P2": 7305,
                "P3": 3092,
                "P4": 9935
            }
        }
    })"_json;
    auto key1 = InsertInstance(*m_briefcase, inst1);
    Test(
        "verify element instance",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key1, "structProp");
            ASSERT_STREQ(inst1["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );

    auto inst2 = R"({
        "className": "ts.Geom2d",
        "data": {
            "G1": 9247,
            "structProp": {
                "P1": 5627,
                "P2": 9315,
                "P3": 9333,
                "P4": 6570
            }
        }
    })"_json;
    auto key2 = InsertInstance(*m_briefcase, inst2);
    Test(
        "verify geom2d instance",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key2, "G1, structProp");
            ASSERT_STREQ(inst2["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "5919d7b1c50a3c0ddb794a16b0864a10a21013f41b71aeb96360410af71942e2";
    const auto SCHEMA1_HASH_ECDB_MAP = "64945d90feb8b07124e81548483f3da799e4d64ee688cfe5f93ce1a729c8889a";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "ea86fc0107881cc77fabeaf7877ddf1a94b8e00621010b62932e72aa766e6331";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element">
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                    <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECStructProperty propertyName="structProp" typeName="S" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Geom2d">
                        <BaseClass>Element</BaseClass>
                        <ECProperty propertyName="G1" typeName="int" />
                    </ECEntityClass>
                    <ECStructClass typeName="S" modifier="None">
                        <ECProperty propertyName="P1" typeName="int" />
                        <ECProperty propertyName="P2" typeName="int" />
                        <ECProperty propertyName="P3" typeName="int" />
                        <ECProperty propertyName="P4" typeName="int" />
                        <ECProperty propertyName="P5" typeName="int" />
                        <ECProperty propertyName="P6" typeName="int" />
                    </ECStructClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "verify map for element",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Element");
            Json::Value expected = R"(
                [
                "TestSchema:Element:ECClassId:ts_Element:ECClassId",
                "TestSchema:Element:ECClassId:ts_Element_Overflow:ECClassId",
                "TestSchema:Element:ECInstanceId:ts_Element:Id",
                "TestSchema:Element:ECInstanceId:ts_Element_Overflow:Id",
                "TestSchema:Element:structProp.P1:ts_Element_Overflow:os4",
                "TestSchema:Element:structProp.P2:ts_Element_Overflow:os5",
                "TestSchema:Element:structProp.P3:ts_Element_Overflow:os6",
                "TestSchema:Element:structProp.P4:ts_Element_Overflow:os7",
                "TestSchema:Element:structProp.P5:ts_Element_Overflow:os2",
                "TestSchema:Element:structProp.P6:ts_Element_Overflow:os3"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );
    Test(
        "verify map for geom2d",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Geom2d");
            Json::Value expected = R"(
                [
                    "TestSchema:Geom2d:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Geom2d:ECClassId:ts_Element_Overflow:ECClassId",
                    "TestSchema:Geom2d:ECInstanceId:ts_Element:Id",
                    "TestSchema:Geom2d:ECInstanceId:ts_Element_Overflow:Id",
                    "TestSchema:Geom2d:G1:ts_Element_Overflow:os1",
                    "TestSchema:Geom2d:structProp.P1:ts_Element_Overflow:os4",
                    "TestSchema:Geom2d:structProp.P2:ts_Element_Overflow:os5",
                    "TestSchema:Geom2d:structProp.P3:ts_Element_Overflow:os6",
                    "TestSchema:Geom2d:structProp.P4:ts_Element_Overflow:os7",
                    "TestSchema:Geom2d:structProp.P5:ts_Element_Overflow:os2",
                    "TestSchema:Geom2d:structProp.P6:ts_Element_Overflow:os3"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );

    // insert a second instance with additional properties
    auto inst3 = R"({
        "className": "ts.Element",
        "data": {
            "structProp": {
                "P1": 4855,
                "P2": 3449,
                "P3": 2623,
                "P4": 6854,
                "P5": 3769,
                "P6": 9612
            }
        }
    })"_json;
    auto key3 = InsertInstance(*m_briefcase, inst3);

    auto inst4 = R"({
        "className": "ts.Geom2d",
        "data": {
            "G1": 2866,
            "structProp": {
                "P1": 9663,
                "P2": 6961,
                "P3": 5681,
                "P4": 6747,
                "P5": 2143,
                "P6": 3404
            }
        }
    })"_json;
    auto key4 = InsertInstance(*m_briefcase, inst4);

    Test(
        "check element before schema upgrade",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key1, "structProp");
            ASSERT_STREQ(inst1["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "check geom2d before schema upgrade",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key2, "G1, structProp");
            ASSERT_STREQ(inst2["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "check element after schema upgrade",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key3, "structProp");
            ASSERT_STREQ(inst3["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "check geom3d after schema upgrade",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key4, "G1, structProp");
            ASSERT_STREQ(inst4["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "check data was moved and left behind - element",
        [&]()
            {
            auto vs = m_briefcase->GetCachedStatement("SELECT ps1, ps2, ps3, ps4 FROM ts_element where id = ?");
            vs->BindId(1, key1.GetInstanceId());
            ASSERT_EQ(BE_SQLITE_ROW, vs->Step());
            ASSERT_TRUE (vs->IsColumnNull(0))  << "Expect ps1 to be null";
            ASSERT_TRUE (vs->IsColumnNull(1))  << "Expect ps2 to be null";
            ASSERT_TRUE (vs->IsColumnNull(2))  << "Expect ps3 to be null";
            ASSERT_TRUE (vs->IsColumnNull(3))  << "Expect ps4 to be null";
            ASSERT_TRUE (vs->IsColumnNull(4))  << "Expect ps5 to be not null not used by element";
            }
    );
    Test(
        "check data was moved and left behind - geom2d",
        [&]()
            {
            auto vs = m_briefcase->GetCachedStatement("SELECT ps1, ps2, ps3, ps4 FROM ts_element where id = ?");
            vs->BindId(1, key2.GetInstanceId());
            ASSERT_EQ(BE_SQLITE_ROW, vs->Step());
            ASSERT_TRUE (vs->IsColumnNull(0))  << "Expect ps1 to be null";
            ASSERT_TRUE (vs->IsColumnNull(1))  << "Expect ps2 to be null";
            ASSERT_TRUE (vs->IsColumnNull(2))  << "Expect ps3 to be null";
            ASSERT_TRUE (vs->IsColumnNull(3))  << "Expect ps4 to be null";
            }
    );

    Test(
        "Check hashes",
        [&]()
            {
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncTestFixture, OverflowedStructClass)
    {
    Test(
        "Import initial schema",
        [&]()
            {
            const auto SCHEMA_HASH_ECDB_SCHEMA = "ec048a655e5d0ccfd29cd06365c23e9161224f46344c6d78a6178197ef1ef301";
            const auto SCHEMA_HASH_ECDB_MAP = "e7363dc95eaed6ea937a43ed36e8396f77a60200ab079b5d6adae812d6ac6436";
            const auto SCHEMA_HASH_SQLITE_SCHEMA = "b36128b828d977fe970b87ca3d5196003f5f5f86cd46f90084eeee825ae2506b";
            auto schema = SchemaItem(
                R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element">
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECStructProperty propertyName="S" typeName="S" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Geom2d">
                        <BaseClass>Element</BaseClass>
                        <ECStructProperty propertyName="G" typeName="G" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Geom2da">
                        <BaseClass>Geom2d</BaseClass>
                        <ECProperty propertyName="I" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Geom3d">
                        <BaseClass>Element</BaseClass>
                        <ECStructProperty propertyName="G" typeName="G" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Geom3da">
                        <BaseClass>Geom3d</BaseClass>
                        <ECProperty propertyName="I" typeName="int" />
                    </ECEntityClass>
                    <ECStructClass typeName="S" modifier="None">
                        <ECProperty propertyName="P1" typeName="int" />
                        <ECProperty propertyName="P2" typeName="int" />
                        <ECProperty propertyName="P3" typeName="int" />
                        <ECProperty propertyName="P4" typeName="int" />
                    </ECStructClass>
                    <ECStructClass typeName="G" modifier="None">
                        <ECProperty propertyName="G1" typeName="int" />
                        <ECProperty propertyName="G2" typeName="int" />
                    </ECStructClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, SetupECDb("struct_prop", schema));
            CheckHashes(*m_briefcase, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP, SCHEMA_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA_HASH_ECDB_SCHEMA, SCHEMA_HASH_ECDB_MAP); });
            }
    );
    
    Test(
        "verify element mapping before schema upgrade",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Element");
            Json::Value expected = R"(
                [
                    "TestSchema:Element:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Element:ECInstanceId:ts_Element:Id",
                    "TestSchema:Element:S.P1:ts_Element:ps1",
                    "TestSchema:Element:S.P2:ts_Element:ps2",
                    "TestSchema:Element:S.P3:ts_Element:ps3",
                    "TestSchema:Element:S.P4:ts_Element:ps4"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );

    Test(
        "verify geom2d mapping before schema upgrade",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Geom2d");
            Json::Value expected = R"(
                [
                    "TestSchema:Geom2d:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Geom2d:ECInstanceId:ts_Element:Id",
                    "TestSchema:Geom2d:G.G1:ts_Element:ps5",
                    "TestSchema:Geom2d:G.G2:ts_Element:ps6",
                    "TestSchema:Geom2d:S.P1:ts_Element:ps1",
                    "TestSchema:Geom2d:S.P2:ts_Element:ps2",
                    "TestSchema:Geom2d:S.P3:ts_Element:ps3",
                    "TestSchema:Geom2d:S.P4:ts_Element:ps4"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );

    Test(
        "verify geom2da mapping before schema upgrade",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Geom2da");
            Json::Value expected = R"(
                [
                    "TestSchema:Geom2da:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Geom2da:ECInstanceId:ts_Element:Id",
                    "TestSchema:Geom2da:G.G1:ts_Element:ps5",
                    "TestSchema:Geom2da:G.G2:ts_Element:ps6",
                    "TestSchema:Geom2da:I:ts_Element:ps7",
                    "TestSchema:Geom2da:S.P1:ts_Element:ps1",
                    "TestSchema:Geom2da:S.P2:ts_Element:ps2",
                    "TestSchema:Geom2da:S.P3:ts_Element:ps3",
                    "TestSchema:Geom2da:S.P4:ts_Element:ps4"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );

    Test(
        "verify geom3d mapping before schema upgrade",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Geom3d");
            Json::Value expected = R"(
                [
                    "TestSchema:Geom3d:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Geom3d:ECInstanceId:ts_Element:Id",
                    "TestSchema:Geom3d:G.G1:ts_Element:ps5",
                    "TestSchema:Geom3d:G.G2:ts_Element:ps6",
                    "TestSchema:Geom3d:S.P1:ts_Element:ps1",
                    "TestSchema:Geom3d:S.P2:ts_Element:ps2",
                    "TestSchema:Geom3d:S.P3:ts_Element:ps3",
                    "TestSchema:Geom3d:S.P4:ts_Element:ps4"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );

    Test(
        "verify geom3da mapping before schema upgrade",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Geom3da");
            Json::Value expected = R"(
                [
                    "TestSchema:Geom3da:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Geom3da:ECInstanceId:ts_Element:Id",
                    "TestSchema:Geom3da:G.G1:ts_Element:ps5",
                    "TestSchema:Geom3da:G.G2:ts_Element:ps6",
                    "TestSchema:Geom3da:I:ts_Element:ps7",
                    "TestSchema:Geom3da:S.P1:ts_Element:ps1",
                    "TestSchema:Geom3da:S.P2:ts_Element:ps2",
                    "TestSchema:Geom3da:S.P3:ts_Element:ps3",
                    "TestSchema:Geom3da:S.P4:ts_Element:ps4"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );

    auto inst1 = R"({
        "className": "ts.Element",
        "data": {
            "S": {
                "P1": 3194,
                "P2": 7305,
                "P3": 3092,
                "P4": 9935
            }
        }
    })"_json;
    auto key1 = InsertInstance(*m_briefcase, inst1);
    Test(
        "verify element instance",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key1, "S");
            ASSERT_STREQ(inst1["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );

    auto inst2 = R"({
        "className": "ts.Geom2d",
        "data": {
            "S": {
                "P1": 2241,
                "P2": 0929,
                "P3": 4361,
                "P4": 9375
            },
            "G" : {
                "G1": 2345,
                "G2": 5675
            }
        }
    })"_json;
    auto key2 = InsertInstance(*m_briefcase, inst2);
    Test(
        "verify geom2d instance",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key2, "S, G");
            ASSERT_STREQ(inst2["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );

    auto inst3 = R"({
        "className": "ts.Geom2da",
        "data": {
            "S": {
                "P1": 8834,
                "P2": 4765,
                "P3": 1998,
                "P4": 1661
            },
            "G" : {
                "G1": 6213,
                "G2": 1597
            },
            "I": 9154
        }
    })"_json;
    auto key3 = InsertInstance(*m_briefcase, inst3);
    Test(
        "verify geom2da instance",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key3, "S, G, I");
            ASSERT_STREQ(inst3["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );

    auto inst4 = R"({
        "className": "ts.Geom3d",
        "data": {
            "S": {
                "P1": 7017,
                "P2": 3955,
                "P3": 5015,
                "P4": 6628
            },
            "G" : {
                "G1": 1553,
                "G2": 1605
            }
        }
    })"_json;
    auto key4 = InsertInstance(*m_briefcase, inst4);
    Test(
        "verify geom3d instance",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key4, "S, G");
            ASSERT_STREQ(inst4["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );

    auto inst5 = R"({
        "className": "ts.Geom3da",
        "data": {
            "S": {
                "P1": 4724,
                "P2": 3456,
                "P3": 6019,
                "P4": 5530
            },
            "G" : {
                "G1": 6595,
                "G2": 8000
            },
            "I": 4478
        }
    })"_json;
    auto key5 = InsertInstance(*m_briefcase, inst5);
    Test(
        "verify geom3da instance",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key5, "S, G, I");
            ASSERT_STREQ(inst5["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );

    const auto SCHEMA1_HASH_ECDB_SCHEMA = "fad6adfc63a6c21f551a7105e98aba26b63b3f3e26e61512be8ef18c59ab37c2";
    const auto SCHEMA1_HASH_ECDB_MAP = "53fc11ba801953ade8b480bcacd72d6857a21020ae541fbe16086fbb0402bfb2";
    const auto SCHEMA1_HASH_SQLITE_SCHEMA = "be89d6bf51757e5e6b18f0969e4e5997b5ca7db158ebc8a78cdf9d4f79bef254";
    Test(
        "import edited schema with some changes",
        [&]()
            {
            auto schema = SchemaItem(
                R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                    <ECEntityClass typeName="Element">
                        <ECCustomAttributes>
                            <ShareColumns xmlns="ECDbMap.02.00.00">
                                <MaxSharedColumnsBeforeOverflow>7</MaxSharedColumnsBeforeOverflow>
                            </ShareColumns>
                            <ClassMap xmlns="ECDbMap.02.00.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECStructProperty propertyName="S" typeName="S" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Geom2d">
                        <BaseClass>Element</BaseClass>
                        <ECStructProperty propertyName="G" typeName="G" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Geom2da">
                        <BaseClass>Geom2d</BaseClass>
                        <ECProperty propertyName="I" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Geom3d">
                        <BaseClass>Element</BaseClass>
                        <ECStructProperty propertyName="G" typeName="G" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Geom3da">
                        <BaseClass>Geom3d</BaseClass>
                        <ECProperty propertyName="I" typeName="int" />
                    </ECEntityClass>
                    <ECStructClass typeName="S" modifier="None">
                        <ECProperty propertyName="P1" typeName="int" />
                        <ECProperty propertyName="P2" typeName="int" />
                        <ECProperty propertyName="P3" typeName="int" />
                        <ECProperty propertyName="P4" typeName="int" />
                        <ECProperty propertyName="P5" typeName="int" />
                        <ECProperty propertyName="P6" typeName="int" />
                    </ECStructClass>
                    <ECStructClass typeName="G" modifier="None">
                        <ECProperty propertyName="G1" typeName="int" />
                        <ECProperty propertyName="G2" typeName="int" />
                    </ECStructClass>
                </ECSchema>)xml"
            );
            ASSERT_EQ(SchemaImportResult::OK, ImportSchema(schema, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
            ASSERT_EQ(BE_SQLITE_OK, m_briefcase->SaveChanges());
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );

    Test(
        "verify element mapping before schema upgrade",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Element");
            Json::Value expected = R"(
                [
                    "TestSchema:Element:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Element:ECClassId:ts_Element_Overflow:ECClassId",
                    "TestSchema:Element:ECInstanceId:ts_Element:Id",
                    "TestSchema:Element:ECInstanceId:ts_Element_Overflow:Id",
                    "TestSchema:Element:S.P1:ts_Element_Overflow:os3",
                    "TestSchema:Element:S.P2:ts_Element_Overflow:os4",
                    "TestSchema:Element:S.P3:ts_Element_Overflow:os5",
                    "TestSchema:Element:S.P4:ts_Element_Overflow:os6",
                    "TestSchema:Element:S.P5:ts_Element_Overflow:os1",
                    "TestSchema:Element:S.P6:ts_Element_Overflow:os2"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );

    Test(
        "verify geom2d mapping before schema upgrade",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Geom2d");
            Json::Value expected = R"(
                [
                    "TestSchema:Geom2d:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Geom2d:ECClassId:ts_Element_Overflow:ECClassId",
                    "TestSchema:Geom2d:ECInstanceId:ts_Element:Id",
                    "TestSchema:Geom2d:ECInstanceId:ts_Element_Overflow:Id",
                    "TestSchema:Geom2d:G.G1:ts_Element:ps5",
                    "TestSchema:Geom2d:G.G2:ts_Element:ps6",
                    "TestSchema:Geom2d:S.P1:ts_Element_Overflow:os3",
                    "TestSchema:Geom2d:S.P2:ts_Element_Overflow:os4",
                    "TestSchema:Geom2d:S.P3:ts_Element_Overflow:os5",
                    "TestSchema:Geom2d:S.P4:ts_Element_Overflow:os6",
                    "TestSchema:Geom2d:S.P5:ts_Element_Overflow:os1",
                    "TestSchema:Geom2d:S.P6:ts_Element_Overflow:os2"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );

    Test(
        "verify geom2da mapping before schema upgrade",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Geom2da");
            Json::Value expected = R"(
                [
                    "TestSchema:Geom2da:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Geom2da:ECClassId:ts_Element_Overflow:ECClassId",
                    "TestSchema:Geom2da:ECInstanceId:ts_Element:Id",
                    "TestSchema:Geom2da:ECInstanceId:ts_Element_Overflow:Id",
                    "TestSchema:Geom2da:G.G1:ts_Element:ps5",
                    "TestSchema:Geom2da:G.G2:ts_Element:ps6",
                    "TestSchema:Geom2da:I:ts_Element:ps7",
                    "TestSchema:Geom2da:S.P1:ts_Element_Overflow:os3",
                    "TestSchema:Geom2da:S.P2:ts_Element_Overflow:os4",
                    "TestSchema:Geom2da:S.P3:ts_Element_Overflow:os5",
                    "TestSchema:Geom2da:S.P4:ts_Element_Overflow:os6",
                    "TestSchema:Geom2da:S.P5:ts_Element_Overflow:os1",
                    "TestSchema:Geom2da:S.P6:ts_Element_Overflow:os2"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );

    Test(
        "verify geom3d mapping before schema upgrade",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Geom3d");
            Json::Value expected = R"(
                [
                    "TestSchema:Geom3d:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Geom3d:ECClassId:ts_Element_Overflow:ECClassId",
                    "TestSchema:Geom3d:ECInstanceId:ts_Element:Id",
                    "TestSchema:Geom3d:ECInstanceId:ts_Element_Overflow:Id",
                    "TestSchema:Geom3d:G.G1:ts_Element:ps5",
                    "TestSchema:Geom3d:G.G2:ts_Element:ps6",
                    "TestSchema:Geom3d:S.P1:ts_Element_Overflow:os3",
                    "TestSchema:Geom3d:S.P2:ts_Element_Overflow:os4",
                    "TestSchema:Geom3d:S.P3:ts_Element_Overflow:os5",
                    "TestSchema:Geom3d:S.P4:ts_Element_Overflow:os6",
                    "TestSchema:Geom3d:S.P5:ts_Element_Overflow:os1",
                    "TestSchema:Geom3d:S.P6:ts_Element_Overflow:os2"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );

    Test(
        "verify geom3da mapping before schema upgrade",
        [&]()
            {
            Json::Value actual = GetPropertyMap(*m_briefcase, "ts.Geom3da");
            Json::Value expected = R"(
                [
                    "TestSchema:Geom3da:ECClassId:ts_Element:ECClassId",
                    "TestSchema:Geom3da:ECClassId:ts_Element_Overflow:ECClassId",
                    "TestSchema:Geom3da:ECInstanceId:ts_Element:Id",
                    "TestSchema:Geom3da:ECInstanceId:ts_Element_Overflow:Id",
                    "TestSchema:Geom3da:G.G1:ts_Element:ps5",
                    "TestSchema:Geom3da:G.G2:ts_Element:ps6",
                    "TestSchema:Geom3da:I:ts_Element:ps7",
                    "TestSchema:Geom3da:S.P1:ts_Element_Overflow:os3",
                    "TestSchema:Geom3da:S.P2:ts_Element_Overflow:os4",
                    "TestSchema:Geom3da:S.P3:ts_Element_Overflow:os5",
                    "TestSchema:Geom3da:S.P4:ts_Element_Overflow:os6",
                    "TestSchema:Geom3da:S.P5:ts_Element_Overflow:os1",
                    "TestSchema:Geom3da:S.P6:ts_Element_Overflow:os2"
                ]
            )"_json;
            ASSERT_STRCASEEQ(expected.toStyledString().c_str(), actual.toStyledString().c_str());
            }
    );

    auto inst6 = R"({
        "className": "ts.Element",
        "data": {
            "S": {
                "P1": 3194,
                "P2": 7305,
                "P3": 3092,
                "P4": 9935,
                "P5": 2345,
                "P6": 7617
            }
        }
    })"_json;
    auto key6 = InsertInstance(*m_briefcase, inst6);
    Test(
        "verify element instance",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key6, "S");
            ASSERT_STREQ(inst6["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );

    auto inst7 = R"({
        "className": "ts.Geom2d",
        "data": {
            "S": {
                "P1": 2052,
                "P2": 6957,
                "P3": 4117,
                "P4": 3313,
                "P5": 4296,
                "P6": 8375
            },
            "G" : {
                "G1": 7337,
                "G2": 8308
            }
        }
    })"_json;
    auto key7 = InsertInstance(*m_briefcase, inst7);
    Test(
        "verify geom2d instance",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key7, "S, G");
            ASSERT_STREQ(inst7["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );

    auto inst8 = R"({
        "className": "ts.Geom2da",
        "data": {
            "S": {
                "P1": 0216,
                "P2": 0729,
                "P3": 1331,
                "P4": 8791,
                "P5": 6558,
                "P6": 3267
            },
            "G" : {
                "G1": 2893,
                "G2": 2684
            },
            "I": 8480
        }
    })"_json;
    auto key8 = InsertInstance(*m_briefcase, inst8);
    Test(
        "verify geom2da instance",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key8, "S, G, I");
            ASSERT_STREQ(inst8["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );

    auto inst9 = R"({
        "className": "ts.Geom3d",
        "data": {
            "S": {
                "P1": 7709,
                "P2": 3660,
                "P3": 3677,
                "P4": 4565,
                "P5": 5576,
                "P6": 0439
            },
            "G" : {
                "G1": 5652,
                "G2": 0269
            }
        }
    })"_json;
    auto key9 = InsertInstance(*m_briefcase, inst9);
    Test(
        "verify geom3d instance",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key9, "S, G");
            ASSERT_STREQ(inst9["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );

    auto inst10 = R"({
        "className": "ts.Geom3da",
        "data": {
            "S": {
                "P1": 4335,
                "P2": 9415,
                "P3": 2146,
                "P4": 6059,
                "P5": 0582,
                "P6": 8747
            },
            "G" : {
                "G1": 8710,
                "G2": 7719
            },
            "I": 5249
        }
    })"_json;
    auto key10 = InsertInstance(*m_briefcase, inst10);
    Test(
        "verify geom3da instance",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key10, "S, G, I");
            ASSERT_STREQ(inst10["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "check element before schema upgrade",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key1, "S");
            ASSERT_STREQ(inst1["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "check geom2d before schema upgrade",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key2, "S, G");
            ASSERT_STREQ(inst2["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "check geom2da after schema upgrade",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key3, "S, G, I");
            ASSERT_STREQ(inst3["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "check geom3d after schema upgrade",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key4, "S, G");
            ASSERT_STREQ(inst4["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "check geom3da after schema upgrade",
        [&]()
            {
            auto out = ReadInstance(*m_briefcase, key5, "S, G, I");
            ASSERT_STREQ(inst5["data"].toStyledString().c_str(), out.toStyledString().c_str());
            }
    );
    Test(
        "check data was moved and left behind - element",
        [&]()
            {
            auto vs = m_briefcase->GetCachedStatement("SELECT ps1, ps2, ps3, ps4 FROM ts_element where id = ?");
            vs->BindId(1, key1.GetInstanceId());
            ASSERT_EQ(BE_SQLITE_ROW, vs->Step());
            ASSERT_TRUE (vs->IsColumnNull(0))  << "Expect ps1 to be null";
            ASSERT_TRUE (vs->IsColumnNull(1))  << "Expect ps2 to be null";
            ASSERT_TRUE (vs->IsColumnNull(2))  << "Expect ps3 to be null";
            ASSERT_TRUE (vs->IsColumnNull(3))  << "Expect ps4 to be null";
            }
    );
    Test(
        "check data was moved and left behind - geom2d",
        [&]()
            {
            auto vs = m_briefcase->GetCachedStatement("SELECT ps1, ps2, ps3, ps4 FROM ts_element where id = ?");
            vs->BindId(1, key2.GetInstanceId());
            ASSERT_EQ(BE_SQLITE_ROW, vs->Step());
            ASSERT_TRUE  (vs->IsColumnNull(0))  << "Expect ps1 to be null";
            ASSERT_TRUE  (vs->IsColumnNull(1))  << "Expect ps2 to be null";
            ASSERT_TRUE  (vs->IsColumnNull(2))  << "Expect ps3 to be null";
            ASSERT_TRUE  (vs->IsColumnNull(3))  << "Expect ps4 to be null";
            }
    );
    Test(
        "check data was moved and left behind - geom2da",
        [&]()
            {
            auto vs = m_briefcase->GetCachedStatement("SELECT ps1, ps2, ps3, ps4 FROM ts_element where id = ?");
            vs->BindId(1, key3.GetInstanceId());
            ASSERT_EQ(BE_SQLITE_ROW, vs->Step());
            ASSERT_TRUE (vs->IsColumnNull(0))  << "Expect ps1 to be null";
            ASSERT_TRUE (vs->IsColumnNull(1))  << "Expect ps2 to be null";
            ASSERT_TRUE (vs->IsColumnNull(2))  << "Expect ps3 to be null";
            ASSERT_TRUE (vs->IsColumnNull(3))  << "Expect ps4 to be null";
            }
    );
    Test(
        "check data was moved and left behind - geom3d",
        [&]()
            {
            auto vs = m_briefcase->GetCachedStatement("SELECT ps1, ps2, ps3, ps4 FROM ts_element where id = ?");
            vs->BindId(1, key4.GetInstanceId());
            ASSERT_EQ(BE_SQLITE_ROW, vs->Step());
            ASSERT_TRUE (vs->IsColumnNull(0))  << "Expect ps1 to be null";
            ASSERT_TRUE (vs->IsColumnNull(1))  << "Expect ps2 to be null";
            ASSERT_TRUE (vs->IsColumnNull(2))  << "Expect ps3 to be null";
            ASSERT_TRUE (vs->IsColumnNull(3))  << "Expect ps4 to be null";
            }
    );
    Test(
        "check data was moved and left behind - geom3da",
        [&]()
            {
            auto vs = m_briefcase->GetCachedStatement("SELECT ps1, ps2, ps3, ps4 FROM ts_element where id = ?");
            vs->BindId(1, key5.GetInstanceId());
            ASSERT_EQ(BE_SQLITE_ROW, vs->Step());
            ASSERT_TRUE (vs->IsColumnNull(0))  << "Expect ps1 to be null";
            ASSERT_TRUE (vs->IsColumnNull(1))  << "Expect ps2 to be null";
            ASSERT_TRUE (vs->IsColumnNull(2))  << "Expect ps3 to be null";
            ASSERT_TRUE (vs->IsColumnNull(3))  << "Expect ps4 to be null";
            }
    );

    Test(
        "Check hashes",
        [&]()
            {
            CheckHashes(*m_briefcase, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP, SCHEMA1_HASH_SQLITE_SCHEMA);
            m_schemaChannel->WithReadOnly([&](ECDbR syncDb) { CheckHashes(syncDb, SCHEMA1_HASH_ECDB_SCHEMA, SCHEMA1_HASH_ECDB_MAP); });
            }
    );
    }

END_ECDBUNITTESTS_NAMESPACE
