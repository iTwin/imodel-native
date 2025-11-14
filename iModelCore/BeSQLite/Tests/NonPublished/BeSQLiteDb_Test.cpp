/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BeSQLiteNonPublishedTests.h"
#include "BeSQLite/ChangeSet.h"
#include <map>
#include <vector>
//---------------------------------------------------------------------------------------
// Creating a new Db for the test
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SetupDb(Db& db, WCharCP dbName)
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLiteLib::Initialize(tempDir);

    BeFileName dbFileName;
    BeTest::GetHost().GetOutputRoot(dbFileName);
    dbFileName.AppendToPath(dbName);

    if (BeFileName::DoesPathExist(dbFileName))
        BeFileName::BeDeleteFile(dbFileName);

    DbResult result = db.CreateNewDb(dbFileName.GetNameUtf8().c_str());
    EXPECT_EQ(BE_SQLITE_OK, result) << "Db Creation failed";
    if (result == BE_SQLITE_OK)
        db.SaveChanges();


    return result;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeSQLiteDb, SchemaDiff) {
    Db lhsDb;
    auto rc = SetupDb(lhsDb, L"lhs.db");
    ASSERT_EQ(BE_SQLITE_OK, rc) << "Creation of test BeSQLite DB failed.";
    lhsDb.SaveChanges();

    Db rhsDb;
    rc = SetupDb(rhsDb, L"rhs.db");
    ASSERT_EQ(BE_SQLITE_OK, rc) << "Creation of test BeSQLite DB failed.";
    rhsDb.SaveChanges();

    auto getTableSchemaAsJson = [](Db& db, Utf8CP tableName, Utf8CP schemaName = "main") {
        BeJsDocument schema;
        MetaData::CompleteTableInfo tbl;
        auto rc = MetaData::QueryTable(db, schemaName, tableName, tbl);
        EXPECT_EQ(BE_SQLITE_OK, rc) << "Getting schema for table failed";
        MetaData::ToJson(tbl, schema);
        return schema.Stringify(StringifyFormat::Indented);
    };
    auto performSchemaDiff = [](Db& lhsDb, Db& rhsDb) {
        std::vector<Utf8String> patches;
        auto rc = MetaData::SchemaDiff(lhsDb, rhsDb, patches);
        EXPECT_EQ(BE_SQLITE_OK, rc) << "SchemaDiff failed";
        return patches;
    };
    auto parseJson = [](Utf8CP json) {
        BeJsDocument doc(json);
        return doc.Stringify(StringifyFormat::Indented);
    };
    auto applyPatches = [](Db& db, std::vector<Utf8String> const& patches) {
        for (auto const& patch : patches) {
            auto rc = db.ExecuteDdl(patch.c_str());
            EXPECT_EQ(BE_SQLITE_OK, rc) << "Applying patch failed";
        }
        db.SaveChanges();
    };
    if ("add table") {
        rc = lhsDb.ExecuteDdl("CREATE TABLE t(a)");
        ASSERT_EQ(BE_SQLITE_OK, rc) << "Creation of table table t failed.";
        lhsDb.SaveChanges();

        ASSERT_STREQ(
            parseJson(R"json({
                "name": "t",
                "schema": "main",
                "type": "table",
                "nColumns": 1,
                "hasRowId": false,
                "isStrict": false,
                "sql": "CREATE TABLE t(a)",
                "columns": [
                    {
                        "cid": 0,
                        "name": "a",
                        "dataType": "",
                        "notNull": false,
                        "defaultValue": "",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    }
                ],
                "indexes": [],
                "triggers": [],
                "foreignKeys": []
                })json").c_str(),
            getTableSchemaAsJson(lhsDb, "t").c_str());


        auto patch = performSchemaDiff(lhsDb, rhsDb);
        ASSERT_EQ(1, patch.size());
        ASSERT_STREQ("CREATE TABLE t(a)", patch[0].c_str());
        applyPatches(rhsDb, patch);
    }
    if ("add column with not null and default constraint") {
        rc = lhsDb.ExecuteDdl("ALTER TABLE t ADD COLUMN b TEXT NOT NULL DEFAULT ('abc')");
        ASSERT_EQ(BE_SQLITE_OK, rc) << "Creation of table table t failed.";
        lhsDb.SaveChanges();
        ASSERT_STREQ(
            parseJson(R"json({
                "name": "t",
                "schema": "main",
                "type": "table",
                "nColumns": 2,
                "hasRowId": false,
                "isStrict": false,
                "sql": "CREATE TABLE t(a, b TEXT NOT NULL DEFAULT ('abc'))",
                "columns": [
                    {
                        "cid": 0,
                        "name": "a",
                        "dataType": "",
                        "notNull": false,
                        "defaultValue": "",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    },
                    {
                        "cid": 1,
                        "name": "b",
                        "dataType": "TEXT",
                        "notNull": true,
                        "defaultValue": "'abc'",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    }
                ],
                "indexes": [],
                "triggers": [],
                "foreignKeys": []
                })json").c_str(),
            getTableSchemaAsJson(lhsDb, "t").c_str());


        auto patches = performSchemaDiff(lhsDb, rhsDb);
        ASSERT_EQ(1, patches.size());
        ASSERT_STREQ("ALTER TABLE [main].[t] ADD COLUMN [b] TEXT NOT NULL DEFAULT ('abc');", patches[0].c_str());
        applyPatches(rhsDb, patches);
    }

    if ("add primary key column should fail") {
        rc = lhsDb.TryExecuteSql("ALTER TABLE t ADD COLUMN id INTEGER PRIMARY KEY ASC AUTOINCREMENT");
        ASSERT_EQ(BE_SQLITE_ERROR, rc) <<  "Adding primary key column is expected to fail.";
        lhsDb.SaveChanges();
    }

    if ("create trigger") {
        rc = lhsDb.ExecuteDdl("CREATE TRIGGER t_default AFTER INSERT ON t BEGIN UPDATE t SET b = 'xyz' WHERE rowid = new.rowid; END");
        ASSERT_EQ(BE_SQLITE_OK, rc) <<  "Creation of trigger t_default failed.";
        lhsDb.SaveChanges();

        ASSERT_STREQ(
            parseJson(R"json({
                "name": "t",
                "schema": "main",
                "type": "table",
                "nColumns": 2,
                "hasRowId": false,
                "isStrict": false,
                "sql": "CREATE TABLE t(a, b TEXT NOT NULL DEFAULT ('abc'))",
                "columns": [
                    {
                        "cid": 0,
                        "name": "a",
                        "dataType": "",
                        "notNull": false,
                        "defaultValue": "",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    },
                    {
                        "cid": 1,
                        "name": "b",
                        "dataType": "TEXT",
                        "notNull": true,
                        "defaultValue": "'abc'",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    }
                ],
                "indexes": [],
                "triggers": [
                    {
                        "name": "t_default",
                        "sql": "CREATE TRIGGER t_default AFTER INSERT ON t BEGIN UPDATE t SET b = 'xyz' WHERE rowid = new.rowid; END"
                    }
                ],
                "foreignKeys": []
                })json").c_str(),
            getTableSchemaAsJson(lhsDb, "t").c_str());

        auto patches = performSchemaDiff(lhsDb, rhsDb);
        ASSERT_EQ(1, patches.size());
        ASSERT_STREQ("CREATE TRIGGER t_default AFTER INSERT ON t BEGIN UPDATE t SET b = 'xyz' WHERE rowid = new.rowid; END", patches[0].c_str());
        applyPatches(rhsDb, patches);
    }
    if ("update trigger") {
        rc = lhsDb.ExecuteDdl("DROP TRIGGER IF EXISTS [main].[t_default];");
        ASSERT_EQ(BE_SQLITE_OK, rc) <<  "Dropping trigger t_default failed.";

        rc = lhsDb.ExecuteDdl("CREATE TRIGGER t_default AFTER INSERT ON t BEGIN UPDATE t SET b = 'abc' WHERE rowid = new.rowid; END");
        ASSERT_EQ(BE_SQLITE_OK, rc) <<  "Dropping trigger t_default failed.";

        lhsDb.SaveChanges();
        ASSERT_STREQ(
            parseJson(R"json({
                "name": "t",
                "schema": "main",
                "type": "table",
                "nColumns": 2,
                "hasRowId": false,
                "isStrict": false,
                "sql": "CREATE TABLE t(a, b TEXT NOT NULL DEFAULT ('abc'))",
                "columns": [
                    {
                        "cid": 0,
                        "name": "a",
                        "dataType": "",
                        "notNull": false,
                        "defaultValue": "",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    },
                    {
                        "cid": 1,
                        "name": "b",
                        "dataType": "TEXT",
                        "notNull": true,
                        "defaultValue": "'abc'",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    }
                ],
                "indexes": [],
                "triggers": [
                    {
                        "name": "t_default",
                        "sql": "CREATE TRIGGER t_default AFTER INSERT ON t BEGIN UPDATE t SET b = 'abc' WHERE rowid = new.rowid; END"
                    }
                ],
                "foreignKeys": []
                })json").c_str(),
            getTableSchemaAsJson(lhsDb, "t").c_str());

        auto patches = performSchemaDiff(lhsDb, rhsDb);
        ASSERT_EQ(2, patches.size());
        ASSERT_STREQ("DROP TRIGGER IF EXISTS [main].[t_default];", patches[0].c_str());
        ASSERT_STREQ("CREATE TRIGGER t_default AFTER INSERT ON t BEGIN UPDATE t SET b = 'abc' WHERE rowid = new.rowid; END", patches[1].c_str());
        applyPatches(rhsDb, patches);
    }
    if ("create index") {
        rc = lhsDb.ExecuteDdl("CREATE INDEX idx1 ON t(b)");
        ASSERT_EQ(BE_SQLITE_OK, rc) <<  "Creation of index idx1 failed.";
        lhsDb.SaveChanges();

        ASSERT_STREQ(
            parseJson(R"json({
                "name": "t",
                "schema": "main",
                "type": "table",
                "nColumns": 2,
                "hasRowId": false,
                "isStrict": false,
                "sql": "CREATE TABLE t(a, b TEXT NOT NULL DEFAULT ('abc'))",
                "columns": [
                    {
                        "cid": 0,
                        "name": "a",
                        "dataType": "",
                        "notNull": false,
                        "defaultValue": "",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    },
                    {
                        "cid": 1,
                        "name": "b",
                        "dataType": "TEXT",
                        "notNull": true,
                        "defaultValue": "'abc'",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    }
                ],
                "indexes": [
                    {
                        "name": "idx1",
                        "unique": false,
                        "origin": "c",
                        "partial": false,
                        "sql": "CREATE INDEX idx1 ON t(b)",
                        "columns": [
                            {
                            "cid": 1,
                            "name": "b",
                            "desc": false,
                            "collSeq": "BINARY",
                            "key": true
                            }
                        ]
                    }
                ],
                "triggers": [
                    {
                        "name": "t_default",
                        "sql": "CREATE TRIGGER t_default AFTER INSERT ON t BEGIN UPDATE t SET b = 'abc' WHERE rowid = new.rowid; END"
                    }
                ],
                "foreignKeys": []
                })json").c_str(),
            getTableSchemaAsJson(lhsDb, "t").c_str());

        auto patches = performSchemaDiff(lhsDb, rhsDb);
        ASSERT_EQ(1, patches.size());
        ASSERT_STREQ("CREATE INDEX idx1 ON t(b)", patches[0].c_str());
        applyPatches(rhsDb, patches);
    }
    if ("update index") {
        rc = lhsDb.ExecuteDdl("DROP INDEX IF EXISTS [main].[idx1];");
        ASSERT_EQ(BE_SQLITE_OK, rc) <<  "Dropping index idx1 failed.";

        rc = lhsDb.ExecuteDdl("CREATE UNIQUE INDEX idx1 ON t(a,b)");
        ASSERT_EQ(BE_SQLITE_OK, rc) <<  "Creation of index idx1 failed.";
        lhsDb.SaveChanges();

        ASSERT_STREQ(
            parseJson(R"json({
                "name": "t",
                "schema": "main",
                "type": "table",
                "nColumns": 2,
                "hasRowId": false,
                "isStrict": false,
                "sql": "CREATE TABLE t(a, b TEXT NOT NULL DEFAULT ('abc'))",
                "columns": [
                    {
                        "cid": 0,
                        "name": "a",
                        "dataType": "",
                        "notNull": false,
                        "defaultValue": "",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    },
                    {
                        "cid": 1,
                        "name": "b",
                        "dataType": "TEXT",
                        "notNull": true,
                        "defaultValue": "'abc'",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    }
                ],
                "indexes": [
                    {
                        "name": "idx1",
                        "unique": true,
                        "origin": "c",
                        "partial": false,
                        "sql": "CREATE UNIQUE INDEX idx1 ON t(a,b)",
                        "columns": [
                            {
                            "cid": 0,
                            "name": "a",
                            "desc": false,
                            "collSeq": "BINARY",
                            "key": true
                            },
                            {
                            "cid": 1,
                            "name": "b",
                            "desc": false,
                            "collSeq": "BINARY",
                            "key": true
                            }
                        ]
                    }
                ],
                "triggers": [
                    {
                        "name": "t_default",
                        "sql": "CREATE TRIGGER t_default AFTER INSERT ON t BEGIN UPDATE t SET b = 'abc' WHERE rowid = new.rowid; END"
                    }
                ],
                "foreignKeys": []
                })json").c_str(),
            getTableSchemaAsJson(lhsDb, "t").c_str());

        auto patches = performSchemaDiff(lhsDb, rhsDb);
        ASSERT_EQ(2, patches.size());
        ASSERT_STREQ("DROP INDEX IF EXISTS [main].[idx1];", patches[0].c_str());
        ASSERT_STREQ("CREATE UNIQUE INDEX idx1 ON t(a,b)", patches[1].c_str());
        applyPatches(rhsDb, patches);
    }
  if ("add fk") {
        rc = lhsDb.ExecuteDdl("ALTER TABLE t ADD COLUMN p INTEGER REFERENCES t(a) ON DELETE CASCADE");
        ASSERT_EQ(BE_SQLITE_OK, rc) <<  "Creation of fk failed.";
        lhsDb.SaveChanges();

        ASSERT_STREQ(
            parseJson(R"json({
                "name": "t",
                "schema": "main",
                "type": "table",
                "nColumns": 3,
                "hasRowId": false,
                "isStrict": false,
                "sql": "CREATE TABLE t(a, b TEXT NOT NULL DEFAULT ('abc'), p INTEGER REFERENCES t(a) ON DELETE CASCADE)",
                "columns": [
                    {
                        "cid": 0,
                        "name": "a",
                        "dataType": "",
                        "notNull": false,
                        "defaultValue": "",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    },
                    {
                        "cid": 1,
                        "name": "b",
                        "dataType": "TEXT",
                        "notNull": true,
                        "defaultValue": "'abc'",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    },
                    {
                        "cid": 2,
                        "name": "p",
                        "dataType": "INTEGER",
                        "notNull": false,
                        "defaultValue": "",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    }
                ],
                "indexes": [
                    {
                        "name": "idx1",
                        "unique": true,
                        "origin": "c",
                        "partial": false,
                        "sql": "CREATE UNIQUE INDEX idx1 ON t(a,b)",
                        "columns": [
                            {
                            "cid": 0,
                            "name": "a",
                            "desc": false,
                            "collSeq": "BINARY",
                            "key": true
                            },
                            {
                            "cid": 1,
                            "name": "b",
                            "desc": false,
                            "collSeq": "BINARY",
                            "key": true
                            }
                        ]
                    }
                ],
                "triggers": [
                    {
                        "name": "t_default",
                        "sql": "CREATE TRIGGER t_default AFTER INSERT ON t BEGIN UPDATE t SET b = 'abc' WHERE rowid = new.rowid; END"
                    }
                ],
                "foreignKeys": [
                    {
                        "table": "t",
                        "fromColumns": [
                            "p"
                        ],
                        "toColumns": [
                            "p"
                        ],
                        "onUpdate": "NO ACTION",
                        "onDelete": "CASCADE",
                        "match": "NONE"
                    }
                ]
                })json").c_str(),
            getTableSchemaAsJson(lhsDb, "t").c_str());

        auto patches = performSchemaDiff(lhsDb, rhsDb);
        ASSERT_EQ(1, patches.size());
        ASSERT_STREQ("ALTER TABLE [main].[t] ADD COLUMN [p] INTEGER REFERENCES [t]([a]) ON DELETE CASCADE;", patches[0].c_str());
        applyPatches(rhsDb, patches);
    }

  if ("drop column p") {
        rc = lhsDb.ExecuteDdl("ALTER TABLE t DROP COLUMN p");
        ASSERT_EQ(BE_SQLITE_OK, rc) <<  "Creation of fk failed.";

        lhsDb.SaveChanges();

        ASSERT_STREQ(
            parseJson(R"json({
                "name": "t",
                "schema": "main",
                "type": "table",
                "nColumns": 2,
                "hasRowId": false,
                "isStrict": false,
                "sql": "CREATE TABLE t(a, b TEXT NOT NULL DEFAULT ('abc'))",
                "columns": [
                    {
                        "cid": 0,
                        "name": "a",
                        "dataType": "",
                        "notNull": false,
                        "defaultValue": "",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    },
                    {
                        "cid": 1,
                        "name": "b",
                        "dataType": "TEXT",
                        "notNull": true,
                        "defaultValue": "'abc'",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    }
                ],
                "indexes": [
                    {
                        "name": "idx1",
                        "unique": true,
                        "origin": "c",
                        "partial": false,
                        "sql": "CREATE UNIQUE INDEX idx1 ON t(a,b)",
                        "columns": [
                            {
                            "cid": 0,
                            "name": "a",
                            "desc": false,
                            "collSeq": "BINARY",
                            "key": true
                            },
                            {
                            "cid": 1,
                            "name": "b",
                            "desc": false,
                            "collSeq": "BINARY",
                            "key": true
                            }
                        ]
                    }
                ],
                "triggers": [
                    {
                        "name": "t_default",
                        "sql": "CREATE TRIGGER t_default AFTER INSERT ON t BEGIN UPDATE t SET b = 'abc' WHERE rowid = new.rowid; END"
                    }
                ],
                "foreignKeys": []
                })json").c_str(),
            getTableSchemaAsJson(lhsDb, "t").c_str());

        auto patches = performSchemaDiff(lhsDb, rhsDb);
        ASSERT_EQ(3, patches.size());

        ASSERT_STREQ("ALTER TABLE [main].[t] DROP COLUMN [p];", patches[0].c_str());
        // SQLite reformat the SQL in sqlite master and replace double quoted string to single quoted string.
        ASSERT_STREQ("DROP TRIGGER IF EXISTS [main].[delete_embeddedFiles];", patches[1].c_str());
        ASSERT_STREQ("CREATE TRIGGER delete_embeddedFiles AFTER DELETE ON be_EmbedFile BEGIN DELETE FROM be_Prop WHERE Namespace='be_Db' AND NAME='EmbdBlob' AND Id=OLD.Id; END", patches[2].c_str());
        applyPatches(rhsDb, patches);
    }
  if ("drop column b") {
        rc = lhsDb.ExecuteDdl("DROP INDEX [main].[idx1];");
        ASSERT_EQ(BE_SQLITE_OK, rc) <<  "Creation of fk failed.";
        rc = lhsDb.ExecuteDdl("ALTER TABLE t DROP COLUMN b");
        ASSERT_EQ(BE_SQLITE_OK, rc) <<  "Creation of fk failed.";
        lhsDb.SaveChanges();

        ASSERT_STREQ(
            parseJson(R"json({
                    "name": "t",
                    "schema": "main",
                    "type": "table",
                    "nColumns": 1,
                    "hasRowId": false,
                    "isStrict": false,
                    "sql": "CREATE TABLE t(a)",
                    "columns": [
                        {
                            "cid": 0,
                            "name": "a",
                            "dataType": "",
                            "notNull": false,
                            "defaultValue": "",
                            "primaryKey": false,
                            "collSeq": "BINARY",
                            "autoIncrement": false
                        }
                    ],
                    "indexes": [],
                    "triggers": [
                        {
                            "name": "t_default",
                            "sql": "CREATE TRIGGER t_default AFTER INSERT ON t BEGIN UPDATE t SET b = 'abc' WHERE rowid = new.rowid; END"
                        }
                    ],
                    "foreignKeys": []
                })json").c_str(),
            getTableSchemaAsJson(lhsDb, "t").c_str());

        auto patches = performSchemaDiff(lhsDb, rhsDb);
        ASSERT_EQ(2, patches.size());

        ASSERT_STREQ("DROP INDEX IF EXISTS [main].[idx1];", patches[0].c_str());
        ASSERT_STREQ("ALTER TABLE [main].[t] DROP COLUMN [b];", patches[1].c_str());
        applyPatches(rhsDb, patches);
    }
    // rhsDb should be same as lhsDb
    ASSERT_STREQ(
        parseJson(R"json({
                "name": "t",
                "schema": "main",
                "type": "table",
                "nColumns": 1,
                "hasRowId": false,
                "isStrict": false,
                "sql": "CREATE TABLE t(a)",
                "columns": [
                    {
                        "cid": 0,
                        "name": "a",
                        "dataType": "",
                        "notNull": false,
                        "defaultValue": "",
                        "primaryKey": false,
                        "collSeq": "BINARY",
                        "autoIncrement": false
                    }
                ],
                "indexes": [],
                "triggers": [
                    {
                        "name": "t_default",
                        "sql": "CREATE TRIGGER t_default AFTER INSERT ON t BEGIN UPDATE t SET b = 'abc' WHERE rowid = new.rowid; END"
                    }
                ],
                "foreignKeys": []
            })json").c_str(),
        getTableSchemaAsJson(rhsDb, "t").c_str());

    auto patches = performSchemaDiff(lhsDb, rhsDb);
    ASSERT_EQ(0, patches.size());
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeSQLiteDb, MetaData) {
    Db lhsDb;
    auto rc = SetupDb(lhsDb, L"lhs.db");
    ASSERT_EQ(BE_SQLITE_OK, rc) << "Creation of test BeSQLite DB failed.";

    Db rhsDb;
    rc = SetupDb(rhsDb, L"rhs.db");
    ASSERT_EQ(BE_SQLITE_OK, rc) << "Creation of test BeSQLite DB failed.";

    rc = lhsDb.ExecuteDdl(R"sql(
        CREATE TABLE [t1](
        [a] INTEGER PRIMARY KEY ASC ON CONFLICT ABORT AUTOINCREMENT,
        [b] TEXT NOT NULL ON CONFLICT ABORT,
        [e] TEXT DEFAULT ('s' || 'k'),
        [f] INTEGER DEFAULT (100),
        [g] INTEGER DEFAULT (-1.2),
        [h] TEXT COLLATE NOCASE,
        [p] INTEGER REFERENCES [t1]([a]) ON DELETE CASCADE,
        [i] TEXT GENERATED ALWAYS AS ([p] || ',') STORED);
    )sql");
    ASSERT_EQ(BE_SQLITE_OK, rc) << "Creation of table table t1 failed.";

    rc = lhsDb.ExecuteDdl(R"sql(
        CREATE TABLE [t2](
        [a] INTEGER PRIMARY KEY,
        [b] TEXT NOT NULL,
        FOREIGN KEY (a,b) REFERENCES t1(a,b));
    )sql");
    ASSERT_EQ(BE_SQLITE_OK, rc) << "Creation of table table t1 failed.";

    rc = lhsDb.ExecuteDdl(R"sql(
        CREATE TRIGGER [tr1] AFTER INSERT ON [t1] BEGIN
        INSERT INTO [t2] VALUES (new.a, new.b);
        END;
    )sql");
    ASSERT_EQ(BE_SQLITE_OK, rc) << "Creation of trigger tr1 failed.";

    rc = lhsDb.ExecuteDdl(R"sql(
        CREATE INDEX [idx1] ON [t1]([b]);
    )sql");
    ASSERT_EQ(BE_SQLITE_OK, rc) << "Creation of index idx1 failed.";

    lhsDb.SaveChanges();
    rhsDb.SaveChanges();

    MetaData::CompleteTableInfo t1;
    rc = MetaData::QueryTable(lhsDb, "main", "t1", t1);
    ASSERT_EQ(BE_SQLITE_OK, rc) << "Querying table t1 failed.";

    MetaData::CompleteTableInfo t2;
    rc = MetaData::QueryTable(lhsDb, "main", "t1", t2);
    ASSERT_EQ(BE_SQLITE_OK, rc) << "Querying table t1 failed.";

    ASSERT_STREQ("main", t1.schema.c_str());
    ASSERT_STREQ("t1", t1.name.c_str());
    ASSERT_EQ(7, t1.columns.size()); // GENERATED column is not counted.
    ASSERT_EQ(1, t1.triggers.size());
    ASSERT_EQ(1, t1.indexes.size());

    std::vector<Utf8String> patches;
    MetaData::SchemaDiff(lhsDb, rhsDb, patches);
    for (auto const& patch : patches) {
        rc = rhsDb.ExecuteDdl(patch.c_str());
        ASSERT_EQ(BE_SQLITE_OK, rc) << "Applying patch failed.";
    }
    rhsDb.SaveChanges();
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeSQLiteDb, BeBriefcaseBasedIdTest)
    {
    BeBriefcaseId bc(0x103);
    BeBriefcaseBasedId id1(bc, 0x108d7de7e);
    EXPECT_TRUE(bc == id1.GetBriefcaseId());
    EXPECT_TRUE(0x108d7de7e == id1.GetLocalId());
    Utf8String val=id1.ToHexStr();
    EXPECT_TRUE(val == "0x1030108d7de7e");

    BeBriefcaseBasedId id2 = BeBriefcaseBasedId::CreateFromJson(BeJsDocument(val, true));
    EXPECT_TRUE(id2 == id1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(BeSQLiteDb, CheckProfileVersion)
    {
    ProfileVersion expectedProfileVersion(2, 4, 5, 3);
    ProfileVersion minimumUpgradeProfileVersion(2, 3, 0, 0);

    std::map<ProfileVersion, ProfileState> testDataset {{ProfileVersion(0, 0, 0, 0), ProfileState::Older(ProfileState::CanOpen::No, false)},
    {ProfileVersion(1, 0, 0, 0), ProfileState::Older(ProfileState::CanOpen::No, false)},
    {ProfileVersion(1, 9, 0, 0), ProfileState::Older(ProfileState::CanOpen::No, false)},
    {ProfileVersion(2, 3, 0, 0), ProfileState::Older(ProfileState::CanOpen::Readonly, true)},
    {ProfileVersion(2, 4, 0, 0), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},
    {ProfileVersion(2, 4, 2, 3), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},
    {ProfileVersion(2, 4, 5, 0), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},
    {ProfileVersion(2, 4, 5, 2), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},

    {ProfileVersion(2, 4, 5, 3), ProfileState::UpToDate()},

    {ProfileVersion(2, 4, 5, 4), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},
    {ProfileVersion(2, 4, 5, 33), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},

    {ProfileVersion(2, 4, 6, 0), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},
    {ProfileVersion(2, 4, 6, 99), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},
    {ProfileVersion(2, 4, 99, 0), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},
    {ProfileVersion(2, 4, 99, 99), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},

    {ProfileVersion(2, 5, 0, 0), ProfileState::Newer(ProfileState::CanOpen::Readonly)},
    {ProfileVersion(2, 5, 0, 1), ProfileState::Newer(ProfileState::CanOpen::Readonly)},
    {ProfileVersion(2, 99, 0, 1), ProfileState::Newer(ProfileState::CanOpen::Readonly)},
    {ProfileVersion(3, 0, 0, 0), ProfileState::Newer(ProfileState::CanOpen::No)},
    {ProfileVersion(99, 99, 99, 99), ProfileState::Newer(ProfileState::CanOpen::No)}};

    for (auto const& testItem : testDataset)
        {
        ProfileVersion const& actualProfileVersion = testItem.first;
        ProfileState const& expectedState = testItem.second;

        ProfileState actualState = Db::CheckProfileVersion(expectedProfileVersion, actualProfileVersion, minimumUpgradeProfileVersion, "Test");

        EXPECT_EQ(expectedState, actualState) <<  "Expected version: " << expectedProfileVersion.ToJson().c_str() << " - Actual version: " << actualProfileVersion.ToJson().c_str();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(BeSQLiteDb, AssignBriefcaseIdInReadonlyMode)
    {
    Utf8String dbPath;

    //prepare test dgn db
    {
    Db db;
    auto stat = SetupDb(db, L"changerepoid.ibim");
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Creation of test BeSQLite DB failed.";
    dbPath.assign(db.GetDbFileName());
    db.CloseDb();
    }

    Db db;
    DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ(BE_SQLITE_OK, stat) << L"Reopening test Bim '" << dbPath.c_str() << L"' failed.";

    BeTest::SetFailOnAssert(false);
    stat = db.ResetBriefcaseId(BeBriefcaseId(12345));
    BeTest::SetFailOnAssert(true);
    ASSERT_EQ(BE_SQLITE_READONLY, stat) << L"Calling ResetBriefcaseId on readonly Bim file is expected to fail.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(BeSQLiteDb, AssignBriefcaseId)
    {
    Utf8String dbPath;

    //prepare test dgn db
    {
    Db db;
    auto stat = SetupDb(db, L"assignbriefcaseid.db");
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Creation of test BeSQLite DB failed.";
    dbPath.assign(db.GetDbFileName());

    std::vector<int> localValues = {1234, 111, -111, 0};
    std::vector<Utf8String> localValueNames = {"key0", "key1", "key2", "key3"};
    const size_t valueCount = localValues.size();
    for (size_t i = 0; i < valueCount; i++)
        {
        int val = localValues[i];
        size_t keyIndex = 0;
        ASSERT_EQ(BE_SQLITE_OK, db.GetBLVCache().Register(keyIndex, localValueNames[i].c_str())) << "Registration of RLV " << localValueNames[i].c_str() << " is expected to succeed.";
        auto result = db.GetBLVCache().SaveValue(keyIndex, val);
        ASSERT_EQ(BE_SQLITE_OK, result) << "Saving test BLV '" << localValueNames[i].c_str() << "=" << val << "' failed";
        }

    ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges()) << "Committing briefcase local values failed.";
    db.CloseDb();
    }

    //reopen Bim again, change briefcase id and close again (to avoid that caches linger around)
    BeBriefcaseId expectedBriefcaseId;
    expectedBriefcaseId.Invalidate();

    auto newId = BeBriefcaseId(BeBriefcaseId::FirstValidBriefcaseId());

    {
    Db db;
    DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Reopening test Bim '" << dbPath.c_str() << "' failed.";

    //now change briefcase id. This should truncate be_local and reinsert the new briefcase id
    stat = db.ResetBriefcaseId(BeBriefcaseId(newId));
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Changing the briefcase id is not expected to fail.";
    }

    //now reopen from scratch
    Db db;
    DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Reopening test Bim '" << dbPath.c_str() << "' failed.";

    EXPECT_TRUE(newId == db.GetBriefcaseId());
    }

// #include "BeSQLitePublishedTests.h"
// #include "BeSQLite/ChangeSet.h"
// #include <vector>
// #include <limits>
// #include <string>
// #include <initializer_list>

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Db
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct BeSQLiteDbTests : public ::testing::Test
    {
    public:
        Db              m_db;
        DbResult        m_result;

        static DbResult SetupDb(Db& db, WCharCP dbName, BeGuidCR dbGuid=BeGuid(), Db::CreateParams const& createParams=Db::CreateParams());
        void SetupDb(WCharCP dbName);
        static BeFileName getDbFilePath(WCharCP dbName);
    };

/*---------------------------------------------------------------------------------**//**
* Creating a new Db for the test
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName BeSQLiteDbTests::getDbFilePath(WCharCP dbName)
    {
    BeFileName dbFileName;
    BeTest::GetHost().GetOutputRoot(dbFileName);
    dbFileName.AppendToPath(dbName);
    return dbFileName;
    }

//---------------------------------------------------------------------------------------
// Creating a new Db for the test
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult BeSQLiteDbTests::SetupDb(Db& db, WCharCP dbName, BeGuidCR dbGuid, Db::CreateParams const& createParams)
    {
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    BeSQLiteLib::Initialize(temporaryDir);

    BeFileName dbFullName = getDbFilePath(dbName);
    if (BeFileName::DoesPathExist(dbFullName))
        BeFileName::BeDeleteFile(dbFullName);
    DbResult result = db.CreateNewDb(dbFullName.GetNameUtf8().c_str(), createParams, dbGuid);
    EXPECT_EQ (BE_SQLITE_OK, result) << "Db Creation failed";
    db.SaveChanges();
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* Creating a new Db for the test
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteDbTests::SetupDb(WCharCP dbName)
    {
    m_result = SetupDb(m_db, dbName);
    ASSERT_EQ (BE_SQLITE_OK, m_result) << "Db Creation failed";
    }

/*---------------------------------------------------------------------------------**//**
* Creating a new Db
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, CreateNewDb)
    {
    SetupDb(L"blank");
    EXPECT_FALSE (m_db.IsReadonly());
    EXPECT_TRUE (m_db.IsDbOpen());

    //Verifying it's name
    Utf8CP dbName = m_db.GetDbFileName();
    WString strName(dbName, true);
    WString shortName = strName.substr(strName.length() - 5, strName.length());
    EXPECT_STREQ (L"blank", shortName.c_str()) << L"The returned DbFileName is not correct. it is: " << shortName.c_str();
    m_db.SaveChanges();
    m_db.CloseDb();
    EXPECT_FALSE (m_db.IsDbOpen());
    }

/*---------------------------------------------------------------------------------**//**
* Opening an existing Db
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, OpenDb)
    {
    SetupDb(L"one.db");
    EXPECT_TRUE (m_db.IsDbOpen());

    //now close and re-open it
    m_db.CloseDb();
    EXPECT_FALSE (m_db.IsDbOpen());
    m_result = m_db.OpenBeSQLiteDb(getDbFilePath(L"one.db"), Db::OpenParams(Db::OpenMode::Readonly, DefaultTxn::Yes));
    EXPECT_EQ (BE_SQLITE_OK, m_result);
    EXPECT_TRUE (m_db.IsDbOpen());

    }

int GetPageSize(BeFileName dbFile)
    {
    Db db;
    Db::OpenParams openParams(Db::OpenMode::Readonly);
    DbResult result = db.OpenBeSQLiteDb(dbFile, openParams);
    EXPECT_EQ(BE_SQLITE_OK, result);
    Statement stmt;
    EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "pragma page_size"));
    stmt.Step();
    return stmt.GetValueInt(0);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, VacuumWithPageSize)
    {
    Db db;
    WCharCP dbName = L"smalldb.db";
    SetupDb(db, dbName);
    ASSERT_TRUE(db.IsDbOpen());
    BeFileName dbFileName(db.GetDbFileName(), BentleyCharEncoding::Utf8);
    db.CloseDb();

    ASSERT_EQ((int)Db::PageSize::PAGESIZE_4K, GetPageSize(dbFileName));
    auto pageSizes = std::vector<int>{
        (int)Db::PageSize::PAGESIZE_512,
        (int)Db::PageSize::PAGESIZE_1K,
        (int)Db::PageSize::PAGESIZE_2K,
        (int)Db::PageSize::PAGESIZE_4K,
        (int)Db::PageSize::PAGESIZE_8K,
        (int)Db::PageSize::PAGESIZE_16K,
        (int)Db::PageSize::PAGESIZE_32K,
        (int)Db::PageSize::PAGESIZE_64K};

    for (const int ps : pageSizes)
        {
        db.OpenBeSQLiteDb(dbFileName.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_EQ((int)BE_SQLITE_OK, db.Vacuum(ps));
        db.CloseDb();
        ASSERT_EQ(ps, GetPageSize(dbFileName));
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, VacuumInto)
    {
    Db db;
    WCharCP dbName = L"smalldb.db";
    SetupDb(db, dbName);
    ASSERT_TRUE(db.IsDbOpen());
    BeFileName dbFileName(db.GetDbFileName(), BentleyCharEncoding::Utf8);
    BeFileName vacuumedFile;
    vacuumedFile.append(dbFileName.GetDirectoryName());
    vacuumedFile.append(L"small_vacuum.db");
    if (vacuumedFile.DoesPathExist())
        vacuumedFile.BeDeleteFile();

    ASSERT_EQ(BE_SQLITE_OK, db.VacuumInto(vacuumedFile.GetNameUtf8().c_str()));
    db.SaveChanges();
    db.CloseDb();

    ASSERT_TRUE(vacuumedFile.DoesPathExist());
    }
//=======================================================================================
// @bsiclass
//=======================================================================================
struct TestOtherConnectionDb : Db
    {
    int m_changeCount;
    TestOtherConnectionDb() {m_changeCount=0;}
    virtual void _OnDbChangedByOtherConnection() override {++m_changeCount; Db::_OnDbChangedByOtherConnection();}
    };

/*---------------------------------------------------------------------------------**//**
* Test to ensure that PRAGMA data_version changes when another connection changes a database
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, TwoConnections)
    {
    SetupDb(L"one.db");
    EXPECT_TRUE (m_db.IsDbOpen());
    m_db.CloseDb();

    TestOtherConnectionDb db1, db2;
    DbResult result = db1.OpenBeSQLiteDb(getDbFilePath(L"one.db"), Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::No));

    EXPECT_EQ (BE_SQLITE_OK, result);
    EXPECT_TRUE (db1.IsDbOpen());

    result = db2.OpenBeSQLiteDb(getDbFilePath(L"one.db"), Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::No));
    EXPECT_EQ (BE_SQLITE_OK, result);
    EXPECT_TRUE (db2.IsDbOpen());

    { // make a change to the database from the first connection
    Savepoint t1(db1, "tx1");
    // the first transaction should not call _OnDbChangedByOtherConnection
    EXPECT_EQ (0, db1.m_changeCount);
    db1.CreateTable("TEST", "Col1 INTEGER");
    }

    { // make a change to the database from the second connection
    Savepoint t2(db2, "tx2");
    // the first transaction on the second connection should not call _OnDbChangedByOtherConnection
    EXPECT_EQ (0, db2.m_changeCount);
    db2.ExecuteSql("INSERT INTO TEST(Col1) VALUES(3)");
    }

    { // start another transaction on the first connection. This should notice that the second connection changed the db.
    Savepoint t3(db1, "tx1");
    EXPECT_EQ (1, db1.m_changeCount);
    db1.ExecuteSql("INSERT INTO TEST(Col1) VALUES(4)");
    }

    { // additional changes from the same connnection should not trigger additional calls to _OnDbChangedByOtherConnection
    Savepoint t3(db1, "tx1");
    EXPECT_EQ (1, db1.m_changeCount);
    db1.ExecuteSql("INSERT INTO TEST(Col1) VALUES(5)");
    }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestBusyRetry : BusyRetry
    {
    int m_maxCount;
    mutable int m_onBusyCalls;

    TestBusyRetry() : m_maxCount(2), m_onBusyCalls(0) {}
    void Reset() {m_onBusyCalls = 0;}

    virtual int _OnBusy(int count) const
        {
        m_onBusyCalls++;
        if (count >= m_maxCount)
            return 0;

        return 1;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, Concurrency_UsingSavepoints)
    {
    SetupDb(L"one.db");
    m_db.CloseDb();
    BeFileName dbname = getDbFilePath(L"one.db");

    // TEST 2 CONNECTIONS
    TestBusyRetry retry1;
    TestBusyRetry retry2;
    retry1.AddRef();
    retry2.AddRef();

    Db::OpenParams openParams1(Db::OpenMode::ReadWrite, DefaultTxn::No, &retry1);
    Db::OpenParams openParams2(Db::OpenMode::ReadWrite, DefaultTxn::No, &retry2);

    Db db1, db2;
    DbResult result = db1.OpenBeSQLiteDb(dbname, openParams1);
    EXPECT_EQ (BE_SQLITE_OK, result);
    result = db2.OpenBeSQLiteDb(dbname, openParams2);
    EXPECT_EQ (BE_SQLITE_OK, result);

    {
    Savepoint sp1(db1, "DB1", false);
    Savepoint sp2(db2, "DB2", false, BeSQLiteTxnMode::Immediate);

    result = sp1.Begin();
    EXPECT_EQ (BE_SQLITE_OK, result);
    result = sp2.Begin();
    EXPECT_EQ (BE_SQLITE_OK, result);

    result = db2.SavePropertyString(PropertySpec("Foo", "DB2"), "Test2");
    EXPECT_EQ (BE_SQLITE_OK, result);

    result = sp2.Commit();
    EXPECT_EQ (BE_SQLITE_BUSY, result);
    EXPECT_EQ (3, retry2.m_onBusyCalls);

    result = sp1.Commit();
    EXPECT_EQ (BE_SQLITE_OK, result);

    result = sp2.Commit();
    EXPECT_EQ (BE_SQLITE_OK, result);
    }

    {
    retry2.Reset();
    Savepoint sp1(db1, "DB1", false, BeSQLiteTxnMode::Immediate);
    Savepoint sp2(db2, "DB2", false, BeSQLiteTxnMode::Immediate);

    result = sp1.Begin();
    EXPECT_EQ (BE_SQLITE_OK, result);
    result = sp2.Begin();
    EXPECT_EQ (BE_SQLITE_BUSY, result);
    EXPECT_EQ (3, retry2.m_onBusyCalls);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, ClosesDbAfterErrorWhenCommitingTransactionFromSavepointDestructor)
    {
    m_db.CloseDb();

    SetupDb(L"one.db");
    BeFileName dbname = getDbFilePath(L"one.db");

    // we need the commit to fail, will use BE_SQLITE_BUSY as our error and for
    // that we need another connection to the same db

    Db db1;
    RefCountedPtr<TestBusyRetry> retry1 = new TestBusyRetry();
    DbResult result = db1.OpenBeSQLiteDb(dbname, Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::No, retry1.get()));
    EXPECT_EQ (BE_SQLITE_OK, result);

    Db db2;
    RefCountedPtr<TestBusyRetry> retry2 = new TestBusyRetry();
    result = db2.OpenBeSQLiteDb(dbname, Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::No, retry2.get()));
    EXPECT_EQ (BE_SQLITE_OK, result);

    // Create a read transaction which blocks all commits. Then create a write transaction, make some change and attempt to
    // close it - that attempts to commit transaction which should fail with BE_SQLITE_BUSY
    auto blockingSavepoint = std::make_unique<Savepoint>(db2, "DB2", true);
    EXPECT_TRUE(blockingSavepoint->IsActive());

    auto errorSavepoint = std::make_unique<Savepoint>(db1, "DB1", true, BeSQLiteTxnMode::Immediate);
    EXPECT_TRUE(errorSavepoint->IsActive());

    result = db1.SavePropertyString(PropertySpec("Foo", "DB1"), "Test1");
    EXPECT_EQ(BE_SQLITE_OK, result);

    errorSavepoint = nullptr; // closing this transaction fails to commit the change
    blockingSavepoint = nullptr;

    // expect closing both dbs to succeed
    db1.CloseDb();
    db2.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* Setting ProjectGuid
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, SetGuids)
    {
    SetupDb(L"SetGuids.db");

    EXPECT_FALSE (m_db.QueryProjectGuid().IsValid());
    BeGuid projGuid(true);
    m_db.SaveProjectGuid(projGuid);
    BeGuid projGuidOut = m_db.QueryProjectGuid();

    EXPECT_TRUE (projGuidOut.IsValid());
    EXPECT_TRUE (projGuidOut==projGuid);

    // try round-tripping a GUID through a string and back
    Utf8String guidstr(projGuid.ToString());
    EXPECT_EQ (SUCCESS, projGuidOut.FromString(guidstr.c_str()));
    EXPECT_TRUE (projGuidOut==projGuid);

    // roundtrip a GUID that started as a string
    Utf8CP strGuid = "c69ec318-60d6-4c54-8f58-e34b78fb8110";
    BeGuid guid;
    EXPECT_FALSE(guid.IsValid());
    guid.FromString(strGuid);
    EXPECT_TRUE(guid.IsValid());
    EXPECT_STREQ(strGuid, guid.ToString().c_str());

    //get the BeGUID
    BeSQLite::BeGuid dbGuid = m_db.GetDbGuid();
    EXPECT_TRUE (dbGuid.IsValid());

    //create a new Db with explicit BeSQLite::BeGuid value
    Db db2;
    BeSQLite::BeGuid dbGuid2(100, 400), dbGuid3(false);

    BeFileName dbName2 = getDbFilePath(L"new.db");
    if (BeFileName::DoesPathExist(dbName2))
        BeFileName::BeDeleteFile(dbName2);

    m_result = db2.CreateNewDb(dbName2.GetNameUtf8().c_str(), BeSQLite::Db::CreateParams(), dbGuid2);
    dbGuid3 = db2.GetDbGuid();
    EXPECT_TRUE (dbGuid3.IsValid());
    //get the BriefcaseId
    BeBriefcaseId repId = m_db.GetBriefcaseId();
    EXPECT_TRUE (repId.IsValid());
    EXPECT_EQ (0, repId.GetValue());
    db2.SaveChanges();
    m_db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* Attach and then Detach Db
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, AttachDb)
    {
    SetupDb(L"Main.db");
    Db db2;
    BeFileName dbName2 = getDbFilePath(L"sub.db");
    if (BeFileName::DoesPathExist(dbName2))
        BeFileName::BeDeleteFile(dbName2);
    m_result = db2.CreateNewDb(dbName2.GetNameUtf8().c_str());
    ASSERT_EQ (BE_SQLITE_OK, m_result) << "Db Creation failed";

    m_result = m_db.AttachDb(db2.GetDbFileName(), "Attachment1");
    EXPECT_EQ (BE_SQLITE_OK, m_result) << "AttachDb() failed";
    m_result = m_db.DetachDb("Attachment1");
    EXPECT_EQ (BE_SQLITE_OK, m_result) << "DetachDb() failed";

    //crash on DetachDb on incorrect alias
    //m_result = m_db.DetachDb ("Aaaaa");

    //AttachDb() passes for any values
    m_result = m_db.AttachDb(getDbFilePath(L"dummy.db").GetNameUtf8().c_str(), "dummy");
    EXPECT_EQ (BE_SQLITE_OK, m_result);
    db2.SaveChanges();
    m_db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteDbTests, RenameAndDropTable)
{
    SetupDb(L"RenameAndDropTable.db");

    Utf8CP testTableName = "TestTable";
    EXPECT_FALSE(m_db.TableExists(testTableName)) << "Table '" << testTableName << "' is expected to not exist.";
    EXPECT_EQ(BE_SQLITE_OK, m_db.CreateTable(testTableName, "id NUMERIC, name TEXT")) << "Creating test table '" << testTableName << "' failed.";
    EXPECT_TRUE(m_db.TableExists(testTableName)) << "Table '" << testTableName << "' is expected to exist as it was created right before this check.";

    // Now rename the table
    Utf8CP newTableName = "TestTable2";
    EXPECT_TRUE(BE_SQLITE_OK == m_db.RenameTable(testTableName, newTableName));
    EXPECT_FALSE(m_db.TableExists(testTableName)) << "Table '" << testTableName << "' is expected to not exist.";
    EXPECT_TRUE(m_db.TableExists(newTableName)) << "Table '" << testTableName << "' is expected to exist as it was created right before this check.";

    // Now let's drop it
    EXPECT_EQ(BE_SQLITE_OK, m_db.DropTable(newTableName));
    EXPECT_FALSE(m_db.TableExists(newTableName)) << "Table '" << testTableName << "' is expected to not exist.";

    m_db.SaveChanges();
    m_db.CloseDb();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteDbTests, GetLastRowId)
{
    SetupDb(L"explainQuery.db");

    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable("TestTable", "id NUMERIC, name TEXT")) << "Creating table failed.";

    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO TestTable Values(1, 'test')"));
    int64_t rowId = m_db.GetLastInsertRowId();
    EXPECT_EQ(1, rowId);

    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO TestTable Values(2, 'test2')"));
    rowId = m_db.GetLastInsertRowId();
    EXPECT_EQ(2, rowId);

    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable("TestTable2", "id NUMERIC, name TEXT")) << "Creating table failed.";
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO TestTable2 Values(1, 'test')"));
    rowId = m_db.GetLastInsertRowId();
    EXPECT_EQ(1, rowId);

    m_db.SaveChanges();
    m_db.CloseDb();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteDbTests, Serialize)
{
    SetupDb(L"explainQuery.db");

    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable("TestTable", "id NUMERIC, name TEXT")) << "Creating table failed.";

    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO TestTable Values(1, 'test')"));
    int64_t rowId = m_db.GetLastInsertRowId();
    EXPECT_EQ(1, rowId);

    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO TestTable Values(2, 'test2')"));
    rowId = m_db.GetLastInsertRowId();
    EXPECT_EQ(2, rowId);

    DbBuffer buffer = m_db.Serialize();
    EXPECT_EQ(buffer.Size(), 36864);

    Db db2;
    EXPECT_EQ(BE_SQLITE_OK, Db::Deserialize(buffer, db2));
    EXPECT_EQ(buffer.Size(), 0);

    Statement stmt;
    EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(db2, "SELECT COUNT(*) FROM TestTable"));

    EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(stmt.GetValueInt(0), 2);

    m_db.SaveChanges();
    m_db.CloseDb();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteDbTests, QueryCreationDate)
{
    SetupDb(L"CreationDate.db");

    DateTime creationDate;
    //CreationDate is only inserted by Publishers/Convertors? And always return an error
    EXPECT_EQ(BE_SQLITE_ERROR, m_db.QueryCreationDate(creationDate));

    m_db.CloseDb();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteDbTests, TableExists)
    {
    SetupDb(L"tableexists.db");

    Utf8CP testTableName = "testtable";

    EXPECT_FALSE (m_db.TableExists(testTableName)) << "Table '" << testTableName << "' is expected to not exist.";

    EXPECT_EQ (BE_SQLITE_OK, m_db.CreateTable(testTableName, "id NUMERIC, name TEXT")) << "Creating test table '" << testTableName << "' failed.";

    EXPECT_TRUE (m_db.TableExists(testTableName)) << "Table '" << testTableName << "' is expected to exist as it was created right before this check.";

    //now test with closed connection
    m_db.SaveChanges();
    m_db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (BeSQLiteDbTests, BlobTest)
    {
    SetupDb(L"blobtest.db");

    Utf8CP testTableName = "testtable";
    ASSERT_EQ (BE_SQLITE_OK, m_db.CreateTable(testTableName, "val BLOB")) << "Creating test table '" << testTableName << "' failed.";

    const int64_t expectedValue = 123456789LL;
    Statement insertStmt;
    ASSERT_EQ (BE_SQLITE_OK, insertStmt.Prepare(m_db, "INSERT INTO testtable (val) VALUES (?)"));

    ASSERT_EQ (BE_SQLITE_OK, insertStmt.BindBlob(1, &expectedValue, sizeof (expectedValue), Statement::MakeCopy::Yes));
    ASSERT_EQ (BE_SQLITE_DONE, insertStmt.Step());

    Statement selectStmt;
    ASSERT_EQ (BE_SQLITE_OK, selectStmt.Prepare(m_db, "SELECT val FROM testtable LIMIT 1"));
    ASSERT_EQ (BE_SQLITE_ROW, selectStmt.Step());

    void const* actualBlob = selectStmt.GetValueBlob(0);
    int64_t actualValue = -1LL;
    memcpy(&actualValue, actualBlob, sizeof (actualValue));
    ASSERT_EQ (expectedValue, actualValue);

    int actualBlobSize = selectStmt.GetColumnBytes(0);
    ASSERT_EQ ((int) sizeof(int64_t), actualBlobSize);

    //now read Int64 directly
    selectStmt.Reset();
    ASSERT_EQ (BE_SQLITE_ROW, selectStmt.Step());
    ASSERT_EQ (0LL, selectStmt.GetValueInt64(0)) << "is expected to not convert the blob implicitly to the expected int64_t";
    m_db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteDbTests, BigUInt64Test)
    {
    SetupDb(L"biguint64test.db");

    Utf8CP testTableName = "testtable";
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(testTableName, "id INTEGER PRIMARY KEY, val INTEGER")) << "Creating test table '" << testTableName << "' failed.";

    const uint64_t bigNumber = ((uint64_t) std::numeric_limits<int64_t>::max()) + 100;
    ASSERT_TRUE(static_cast<int64_t>(bigNumber) < 0) << "Ensure the uint64 is larger than the max of int64";

    auto assertUInt64 = [] (Db const& db, int64_t id, uint64_t expected)
        {
        CachedStatementPtr selectStmt = db.GetCachedStatement("SELECT val FROM testtable WHERE id=?");
        ASSERT_TRUE(selectStmt != nullptr);
        ASSERT_EQ(BE_SQLITE_OK, selectStmt->BindInt64(1, id));
        ASSERT_EQ(BE_SQLITE_ROW, selectStmt->Step());
        ASSERT_EQ(expected, selectStmt->GetValueUInt64(0));
        ASSERT_EQ(expected, (uint64_t) selectStmt->GetValueInt64(0));
        };

    {
    Statement insertStmt;
    ASSERT_EQ(BE_SQLITE_OK, insertStmt.Prepare(m_db, "INSERT INTO testtable(val) VALUES (?)"));

    ASSERT_EQ(BE_SQLITE_OK, insertStmt.BindUInt64(1, bigNumber));
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    assertUInt64(m_db, m_db.GetLastInsertRowId(), bigNumber);
    }


    {
    Utf8String sql;
    sql.Sprintf("INSERT INTO testtable(val) VALUES (%lld)", bigNumber);
    Statement insertStmt;
    ASSERT_EQ(BE_SQLITE_OK, insertStmt.Prepare(m_db, sql.c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    assertUInt64(m_db, m_db.GetLastInsertRowId(), bigNumber);
    }
    m_db.SaveChanges();
    }
/*---------------------------------------------------------------------------------**//**
* Save and Query PropertyString
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, PropertyString)
    {
    SetupDb(L"props.db");

    PropertySpec spec1("TestSpec", "TestApplication");
    Utf8String stringValue("This is test value");

    m_result = m_db.SavePropertyString(spec1, stringValue);
    EXPECT_EQ (BE_SQLITE_OK, m_result) << "SavePropertyString failed";
    EXPECT_TRUE (m_db.HasProperty(spec1));

    Utf8String stringValue2;
    m_result = m_db.QueryProperty(stringValue2, spec1);
    EXPECT_EQ (BE_SQLITE_ROW, m_result);

    EXPECT_STREQ (stringValue.c_str(), stringValue2.c_str());

    m_db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* Save and Query Property
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, Property)
    {
    SetupDb(L"Props2.db");

    PropertySpec spec1("TestSpec", "TestApplication");
    m_result = m_db.SaveProperty(spec1, L"Any Value", 10, 400, 10);
    EXPECT_EQ (BE_SQLITE_OK, m_result) << "SaveProperty failed";

    EXPECT_TRUE (m_db.HasProperty(spec1, 400, 10));

    Utf8CP buffer[10];
    m_result = m_db.QueryProperty(buffer, 10, spec1, 400, 10);
    EXPECT_EQ (BE_SQLITE_ROW, m_result);
    //EXPECT_TRUE (false) << buffer;

    m_result = m_db.DeleteProperty(spec1, 400, 10);
    EXPECT_EQ (BE_SQLITE_DONE, m_result) << "DeleteProperty failed";
    EXPECT_FALSE (m_db.HasProperty(spec1, 400, 10));

    m_db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, CachedProperties)
    {
    SetupDb(L"Props3.db");

    Byte values[] = {1,2,3,4,5,6};
    PropertySpec spec1("TestSpec", "CachedProp", PropertySpec::Mode::Cached);
    m_result = m_db.SaveProperty(spec1, values, sizeof(values), 400, 10);
    EXPECT_TRUE (m_db.HasProperty(spec1, 400, 10));

    PropertySpec spec2("TestSpec", "CachedProp2", PropertySpec::Mode::Cached);
    m_result = m_db.SaveProperty(spec2, values, sizeof(values));
    EXPECT_TRUE (m_db.HasProperty(spec2));

    Utf8CP spec3Val="Spec 3 value";
    PropertySpec spec3("Spec3", "CachedProp", PropertySpec::Mode::Cached);
    m_result = m_db.SavePropertyString(spec3, spec3Val);
    EXPECT_TRUE (m_db.HasProperty(spec3));

    Utf8String origStr = "String value";
    m_result = m_db.SavePropertyString(spec1, origStr, 400, 10);

    Byte buffer[10];
    m_result = m_db.QueryProperty(buffer, sizeof(values), spec1, 400, 10);
    EXPECT_EQ (BE_SQLITE_ROW, m_result);
    EXPECT_TRUE(0==memcmp(values, buffer, sizeof(values)));

    Byte values2[] = {10,20};
    m_result = m_db.SaveProperty(spec1, values2, sizeof(values2), 400, 10);

    m_db.SaveChanges();

    Utf8String strval;
    m_result = m_db.QueryProperty(strval, spec1, 400, 10);
    EXPECT_EQ (BE_SQLITE_ROW, m_result);
    EXPECT_TRUE(0==strval.CompareTo(origStr));

    m_result = m_db.QueryProperty(strval, spec2);
    EXPECT_EQ (BE_SQLITE_ROW, m_result);
    EXPECT_TRUE(0==strval.CompareTo(""));

    m_result = m_db.QueryProperty(buffer, sizeof(values), spec2);
    EXPECT_EQ (BE_SQLITE_ROW, m_result);
    EXPECT_TRUE(0==memcmp(values, buffer, sizeof(values)));

    m_result = m_db.QueryProperty(strval, spec3);
    EXPECT_EQ (BE_SQLITE_ROW, m_result);
    EXPECT_TRUE(0==strval.CompareTo(spec3Val));
    //EXPECT_TRUE (false) << buffer;

    m_result = m_db.DeleteProperty(spec1, 400, 10);
    EXPECT_EQ (BE_SQLITE_DONE, m_result) << "DeleteProperty failed";
    EXPECT_FALSE (m_db.HasProperty(spec1, 400, 10));

    Utf8String val2Str = "value 2";
    m_result = m_db.SavePropertyString(spec1, val2Str, 400, 10);

    if (true)
        {
        Savepoint savepoint(m_db, "intermediate");
        m_result = m_db.SavePropertyString(spec1, "ChangedStr", 400, 10);
        savepoint.Cancel();
        }

    m_result = m_db.QueryProperty(strval, spec1, 400, 10);
    EXPECT_EQ (BE_SQLITE_ROW, m_result);
    EXPECT_TRUE(0==strval.CompareTo(val2Str));
    EXPECT_TRUE (m_db.HasProperty(spec1, 400, 10));
    m_db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* Save and Query BriefcaseLocal Values
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, BriefcaseLocalValues)
    {
    SetupDb(L"local.db");

    //Working with RLVs through RLVCache
    int val = -1345;
    Utf8CP testPropValueName = "TestProp";
    size_t rlvIndex = 0;
    ASSERT_EQ (BE_SQLITE_OK, m_db.GetBLVCache().Register(rlvIndex, testPropValueName));
    m_result = m_db.GetBLVCache().SaveValue(rlvIndex, val);
    EXPECT_EQ (BE_SQLITE_OK, m_result) << "SaveBriefcaseLocalValue failed";

    uint64_t actualVal = -1LL;
    m_result = m_db.GetBLVCache().QueryValue(actualVal, rlvIndex);
    EXPECT_EQ (BE_SQLITE_OK, m_result);
    EXPECT_EQ (val, (int) actualVal);

    m_db.SaveChanges();

    actualVal = -1LL;
    m_result = m_db.GetBLVCache().QueryValue(actualVal, rlvIndex);
    EXPECT_EQ (BE_SQLITE_OK, m_result);
    EXPECT_EQ (val, (int) actualVal);

    ASSERT_TRUE (m_db.GetBLVCache().TryGetIndex(rlvIndex, testPropValueName));
    ASSERT_FALSE (m_db.GetBLVCache().TryGetIndex(rlvIndex, "GarbageProp"));

    ASSERT_EQ (BE_SQLITE_ERROR, m_db.GetBLVCache().Register(rlvIndex, testPropValueName));

    //Work with RLVs directly
    Utf8CP testProp2 = "TestProp2";
    m_result = m_db.SaveBriefcaseLocalValue(testProp2, "Test Value");
    EXPECT_EQ(BE_SQLITE_DONE, m_result);

    Utf8String val2 = "None";
    m_result = m_db.QueryBriefcaseLocalValue(val2, testProp2);
    EXPECT_EQ(BE_SQLITE_ROW, m_result);
    EXPECT_STREQ("Test Value", val2.c_str());

    m_db.SaveChanges();
    m_db.CloseDb();
    }


/*---------------------------------------------------------------------------------**//**
* Simulate a LineStyle bim case
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, linestyleDB)
    {
    SetupDb(L"linestyle.db");

    //creating a table
    EXPECT_EQ (BE_SQLITE_OK, m_db.CreateTable("linestyles", "lsId NUMERIC, lsName TEXT"));
    EXPECT_TRUE (m_db.TableExists("linestyles"));
    EXPECT_TRUE (m_db.ColumnExists("linestyles", "lsId"));

    //Add data
    ASSERT_EQ (BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO linestyles (lsId, lsName) values (10, 'ARROW')"));

    //Dump the result for display
    m_db.DumpSqlResults("SELECT * from linestyles");
    m_db.SaveChanges();
    }


#ifdef PUBLISHER_WONT_PUBLISH_EXPIRED_FILES
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeSQLiteDbOpenTest, Expired)
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLiteLib::Initialize(tempDir);

    BeFileName docDir;
    BeTest::GetHost().GetDocumentsRoot(docDir);

    BeFileName expiredFileNameW (docDir);
    expiredFileNameW.AppendToPath(L"Bim");
    expiredFileNameW.AppendToPath(L"expired.ibim");
    ASSERT_TRUE( BeFileName::DoesPathExist(expiredFileNameW) );

    Utf8String expiredFileName(expiredFileNameW);

    BeSQLite::Db::OpenParams parms(BeSQLite::Db::OpenMode::Readonly);

    BeSQLite::Db db;
    ASSERT_TRUE( db.OpenBeSQLiteDb(expiredFileName.c_str(), parms) == BE_SQLITE_OK );
    ASSERT_TRUE( db.IsDbOpen() );
    ASSERT_TRUE( db.IsExpired() );
    DateTime xdate;
    ASSERT_TRUE( db.GetExpirationDate(xdate) == BE_SQLITE_OK );
    ASSERT_TRUE( DateTime::Compare(DateTime::GetCurrentTimeUtc(), xdate) != DateTime::CompareResult::EarlierThan );

    db.CloseDb();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeSQLiteDbOpenTest, Expired2)
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLiteLib::Initialize(tempDir);

    Utf8String expiredFileName;
    if (true)
        {
        BeFileName dbFullName = BeSQLiteDbTests::getDbFilePath(L"expired2.ibim");
        if (BeFileName::DoesPathExist(dbFullName))
            BeFileName::BeDeleteFile(dbFullName);
        BeSQLite::Db db;
        BeSQLite::Db::CreateParams createParms;
        createParms.SetExpirationDate(DateTime::GetCurrentTimeUtc());
        ASSERT_EQ( BE_SQLITE_OK, db.CreateNewDb(dbFullName.GetNameUtf8().c_str(), createParms) );
        ASSERT_TRUE( db.IsDbOpen() );
        ASSERT_TRUE( db.IsExpired() );
        expiredFileName.assign(db.GetDbFileName());
        db.SaveChanges();
        }

    BeSQLite::Db::OpenParams parms(BeSQLite::Db::OpenMode::Readonly);

    BeSQLite::Db db;
    ASSERT_TRUE( db.OpenBeSQLiteDb(expiredFileName.c_str(), parms) == BE_SQLITE_OK );
    ASSERT_TRUE( db.IsDbOpen() );
    ASSERT_TRUE( db.IsExpired() );
    DateTime xdate;
    ASSERT_TRUE( db.QueryExpirationDate(xdate) == BE_SQLITE_ROW );
    ASSERT_TRUE( DateTime::Compare(DateTime::GetCurrentTimeUtc(), xdate) != DateTime::CompareResult::EarlierThan );

    db.CloseDb();
    }

struct BeSQLiteEmbeddedFileTests : BeSQLiteDbTests
    {
protected:
    //---------------------------------------------------------------------------------------
    // @bsimethod
    //+---------------+---------------+---------------+---------------+---------------+------
    void deleteExistingFile(BeFileName filePath)
        {
        if (BeFileName::DoesPathExist(filePath.GetName()))
            {
            // Delete any previously exported file
            BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile(filePath.GetName());
            ASSERT_EQ(BeFileNameStatus::Success, fileDeleteStatus);
            }
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteEmbeddedFileTests, ReplaceExistingEmbeddedFile)
    {
    SetupDb(L"embeddedfiles.db");

    //test file
    //  Using a much larger file so I could check that the embedded blobs were removed from the BE_Prop table.
    Utf8CP testFileNameOld = "Bentley_Standard_CustomAttributes.01.14.ecschema.xml";
    WString testFileNameOldW(testFileNameOld, BentleyCharEncoding::Utf8);

    BeFileName testFilePathOld;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testFilePathOld);
    testFilePathOld.AppendToPath(L"ECSchemas");
    testFilePathOld.AppendToPath(L"Standard");
    testFilePathOld.AppendToPath(testFileNameOldW.c_str());

    //INSERT scenario
    DbEmbeddedFileTable& embeddedFileTable = m_db.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime expectedLastModified = DateTime::GetCurrentTimeUtc();

    BeBriefcaseBasedId embeddedFileId = embeddedFileTable.Import(&stat, true, testFileNameOld, testFilePathOld.GetNameUtf8().c_str(),  &expectedLastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(embeddedFileId.IsValid());

    BeFileName testFilePath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testFilePath);
    testFilePath.AppendToPath(L"ECSchemas");
    testFilePath.AppendToPath(L"Standard");
    testFilePath.AppendToPath(testFileNameOldW.c_str());
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Replace(testFileNameOld, testFilePath.GetNameUtf8().c_str()));

    //Query to check that embedded blobs were removed form BE_Prop
    Statement stmt;
    DbResult dbr = stmt.Prepare(m_db, "SELECT * FROM " BEDB_TABLE_Property " WHERE Id=? AND SubId>0");
    ASSERT_EQ(BE_SQLITE_OK, dbr);
    stmt.BindId(1, embeddedFileId);
    dbr = stmt.Step();
    ASSERT_EQ(BE_SQLITE_DONE, dbr);

    BeFileName exportFilePath;
    BeTest::GetHost().GetOutputRoot(exportFilePath);
    exportFilePath.AppendToPath(testFileNameOldW.c_str());
    deleteExistingFile(exportFilePath);
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Export(exportFilePath.GetNameUtf8().c_str(), testFileNameOld));
    m_db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteEmbeddedFileTests, ReadAddNewEntrySaveEmbeddedFile)
    {
    SetupDb(L"embeddedfiles.db");

    //test file
    //  Used a fairly large file for this to verify that it correctly handles files that are larger than one blob.
    Utf8CP testFileName = "Bentley_Standard_CustomAttributes.01.14.ecschema.xml";
    WString testFileNameW(testFileName, BentleyCharEncoding::Utf8);

    BeFileName testFilePath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testFilePath);
    testFilePath.AppendToPath(L"ECSchemas");
    testFilePath.AppendToPath(L"Standard");
    testFilePath.AppendToPath(testFileNameW.c_str());

    //INSERT scenario
    DbEmbeddedFileTable& embeddedFileTable = m_db.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime expectedLastModified = DateTime::GetCurrentTimeUtc();
    double expectedLastModifiedJd = 0.0;
    ASSERT_EQ(SUCCESS, expectedLastModified.ToJulianDay(expectedLastModifiedJd));

    BeBriefcaseBasedId embeddedFileId = embeddedFileTable.Import(&stat, true, testFileName, testFilePath.GetNameUtf8().c_str(),  &expectedLastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(embeddedFileId.IsValid());

    Utf8CP NewFileName = "Copy_Bentley_Standard_CustomAttributes.01.14.ecschema.xml";
    WString NewFileNameW(NewFileName, BentleyCharEncoding::Utf8);
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.AddEntry(NewFileName, "xml"));

    uint64_t size = 0;
    ASSERT_EQ(embeddedFileId, embeddedFileTable.QueryFile(testFileName, &size));
    ASSERT_TRUE(size > 0);

    ChunkedArray buffer;
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Read(buffer, testFileName));
    ASSERT_TRUE(size == buffer.m_size);
    m_db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteEmbeddedFileTests, ImportExportEmptyFile)
    {
    SetupDb(L"embeddedfiles.db");

    //test file
    Utf8CP testFileName = "EmptyFile.txt";
    BeFileName testFilePath;
    BeTest::GetHost().GetOutputRoot(testFilePath);
    testFilePath.AppendToPath(WString(testFileName, BentleyCharEncoding::Utf8).c_str());
    deleteExistingFile(testFilePath);

    BeFile testFile;
    ASSERT_EQ(BeFileStatus::Success, testFile.Create(testFilePath.c_str()));

    //INSERT scenario
    DbEmbeddedFileTable& embeddedFileTable = m_db.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime expectedLastModified = DateTime::GetCurrentTimeUtc();
    double expectedLastModifiedJd = 0.0;
    ASSERT_EQ(SUCCESS, expectedLastModified.ToJulianDay(expectedLastModifiedJd));

    BeBriefcaseBasedId embeddedFileId = embeddedFileTable.Import(&stat, true, testFileName, testFilePath.GetNameUtf8().c_str(), &expectedLastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(embeddedFileId.IsValid());

    BeFileName testFileOutPath;
    BeTest::GetHost().GetOutputRoot(testFileOutPath);
    testFileOutPath.AppendToPath(L"EmptyFileOut.txt");
    deleteExistingFile(testFileOutPath);
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Export(testFileOutPath.GetNameUtf8().c_str(), testFileName));
    m_db.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteEmbeddedFileTests, EmbedFileWithInvalidPath)
    {
    SetupDb(L"embeddedfiles.db");

    //test file
    Utf8CP testFileName = "StartupCompany.json";
    WString testFileNameW(testFileName, BentleyCharEncoding::Utf8);

    //Test File Path
    BeFileName testFilePath;
    testFilePath.AppendToPath(testFileNameW.c_str());

    //INSERT scenario
    DbEmbeddedFileTable& embeddedFileTable = m_db.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime expectedLastModified = DateTime::GetCurrentTimeUtc();
    double expectedLastModifiedJd = 0.0;
    ASSERT_EQ(SUCCESS, expectedLastModified.ToJulianDay(expectedLastModifiedJd));

    BeBriefcaseBasedId embeddedFileId = embeddedFileTable.Import(&stat, true, testFileName, testFilePath.GetNameUtf8().c_str(),  &expectedLastModified);
    ASSERT_EQ(BE_SQLITE_ERROR_FileNotFound, stat);
    ASSERT_FALSE(embeddedFileId.IsValid());
    }

struct MyChangeTracker : ChangeTracker
    {
    MyChangeTracker(DbR db) : ChangeTracker("Test") { SetDb(&db); }
    virtual OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override { BeAssert(false); return OnCommitStatus::Abort; }
    };

struct MyChangeSet : ChangeSet
    {
    virtual ConflictResolution _OnConflict(ConflictCause clause, Changes::Change iter) { BeAssert(false); return ConflictResolution::Abort; }
    };
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQLiteDbTests, ApplyTable_Filter) {
    const auto kMainFile = "changeset.db";
    auto cloneDb = [&](DbR from, DbR out, Utf8CP name) {
        ASSERT_EQ (BE_SQLITE_OK, from.SaveChanges());
        Utf8String fileName = from.GetDbFileName();
        fileName.ReplaceAll(kMainFile, name);
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(BeFileName(m_db.GetDbFileName(), true), BeFileName(fileName.c_str(), true)));
        ASSERT_EQ(BE_SQLITE_OK, out.OpenBeSQLiteDb(fileName.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite)));
    };

    SetupDb(WString(kMainFile, true).c_str());
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TABLE ec_t1 (ID INTEGER PRIMARY KEY)"));
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TABLE t2 (ID INTEGER PRIMARY KEY)"));
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TABLE t3 (ID INTEGER PRIMARY KEY)"));
    m_db.SaveChanges();

    Db anotherDb;
    cloneDb(m_db, anotherDb, "changeset2.db");

    MyChangeTracker changeTracker(m_db);
    changeTracker.EnableTracking(true);
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO ec_t1 (ID) values (NULL)"));
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO t2 (ID) values (NULL)"));
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO t3 (ID) values (NULL)"));
    changeTracker.EnableTracking(false);

    MyChangeSet cs;
    cs.FromChangeTrack(changeTracker);
    m_db.SaveChanges();

    auto getRowCount = [&](DbR db, Utf8CP tableName) -> int {
        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(db, SqlPrintfString("SELECT COUNT(*) FROM %s", tableName)));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        return stmt.GetValueInt(0);
    };

    ASSERT_EQ(BE_SQLITE_OK, cs.ApplyChanges(anotherDb, ApplyChangesArgs::Default().ApplyOnlySchemaChanges()));
    ASSERT_EQ(1, getRowCount(anotherDb, "ec_t1"));
    ASSERT_EQ(0, getRowCount(anotherDb, "t2"));
    ASSERT_EQ(0, getRowCount(anotherDb, "t3"));

    anotherDb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, cs.ApplyChanges(anotherDb, ApplyChangesArgs::Default().ApplyOnlyDataChanges()));
    ASSERT_EQ(0, getRowCount(anotherDb, "ec_t1"));
    ASSERT_EQ(1, getRowCount(anotherDb, "t2"));
    ASSERT_EQ(1, getRowCount(anotherDb, "t3"));

    anotherDb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, cs.ApplyChanges(anotherDb, ApplyChangesArgs::Default().ApplyAnyChanges()));
    ASSERT_EQ(1, getRowCount(anotherDb, "ec_t1"));
    ASSERT_EQ(1, getRowCount(anotherDb, "t2"));
    ASSERT_EQ(1, getRowCount(anotherDb, "t3"));

    anotherDb.SaveChanges();
    m_db.SaveChanges();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQLiteDbTests, ChangeGroup_Filter) {
    SetupDb(L"changeset.db");
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TABLE t1 (ID INTEGER PRIMARY KEY)"));
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TABLE t2 (ID INTEGER PRIMARY KEY)"));
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TABLE t3 (ID INTEGER PRIMARY KEY)"));

    MyChangeTracker changeTracker(m_db);
    changeTracker.EnableTracking(true);
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO t1 (ID) values (NULL)"));
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO t2 (ID) values (NULL)"));
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO t3 (ID) values (NULL)"));
    changeTracker.EnableTracking(false);

    auto getChangeTable = [&](Changes::Change const& change) -> Utf8String {
        Utf8CP tableName;
        int nCols;
        DbOpcode opCode;
        int  indirect;
        EXPECT_EQ(BE_SQLITE_OK, change.GetOperation(&tableName, &nCols, &opCode, &indirect));
        return Utf8String(tableName);
    };
    auto getChangeTables = [&](ChangeStream& changeset) -> std::vector<Utf8String> {
        std::vector<Utf8String> tableNames;
        for (auto change : changeset.GetChanges()) {
            tableNames.push_back(getChangeTable(change));
        }
        return tableNames;
    };

    MyChangeSet cs;
    cs.FromChangeTrack(changeTracker);
    m_db.SaveChanges();

    const auto tables = getChangeTables(cs);
    ASSERT_EQ(3, tables.size());
    ASSERT_STREQ("t1", tables[0].c_str());
    ASSERT_STREQ("t2", tables[1].c_str());
    ASSERT_STREQ("t3", tables[2].c_str());

    // Filter changeset using ChangeGroup::FilterIf
    ChangeGroup t1Group(m_db);
    ASSERT_EQ(BE_SQLITE_OK, ChangeGroup::FilterIf(cs,
        [&](Changes::Change const& change) {
            return getChangeTable(change).EqualsIAscii("t1");
        }, t1Group)
    );
    ChangeSet t1Changeset;
    t1Changeset.FromChangeGroup(t1Group);
    const auto t1Tables = getChangeTables(t1Changeset);
    ASSERT_EQ(1, t1Tables.size());
    ASSERT_STREQ("t1", t1Tables[0].c_str());

    // Filter changeset using ChangeGroup::FilterIfElse
    ChangeGroup t2Group(m_db);
    ChangeGroup t1t3Group(m_db);
    ASSERT_EQ(BE_SQLITE_OK, ChangeGroup::FilterIfElse(cs,
        [&](Changes::Change const& change) {
            return getChangeTable(change).EqualsIAscii("t2");
        }, t2Group, t1t3Group)
    );

    ChangeSet t2Changeset;
    t2Changeset.FromChangeGroup(t2Group);
    const auto t2Tables = getChangeTables(t2Changeset);
    ASSERT_EQ(1, t2Tables.size());
    ASSERT_STREQ("t2", t2Tables[0].c_str());

    ChangeSet t1t3Changeset;
    t1t3Changeset.FromChangeGroup(t1t3Group);
    const auto t1t3Tables = getChangeTables(t1t3Changeset);
    ASSERT_EQ(2, t1t3Tables.size());
    ASSERT_STREQ("t1", t1t3Tables[0].c_str());
    ASSERT_STREQ("t3", t1t3Tables[1].c_str());
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQLiteDbTests, SchemaChangeBetweenDataChangesets)
    {
    SetupDb(L"changeset.db");
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TABLE TestTable (ID INTEGER PRIMARY KEY, T REAL)"));

    MyChangeTracker changeTracker(m_db);

    changeTracker.EnableTracking(true);
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO TestTable (T) values (1)"));

    MyChangeSet cs1;
    ASSERT_EQ(BE_SQLITE_OK, cs1.FromChangeTrack(changeTracker));
    changeTracker.EndTracking();

    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("ALTER TABLE TestTable ADD COLUMN P REAL"));

    changeTracker.EnableTracking(true);
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO TestTable (T, P) values (1, 1)"));

    MyChangeSet cs2;
    ASSERT_EQ(BE_SQLITE_OK, cs2.FromChangeTrack(changeTracker));
    changeTracker.EndTracking();

    ChangeGroup group;
    ASSERT_EQ(BE_SQLITE_OK, cs1.AddToChangeGroup(group));
    ASSERT_EQ(BE_SQLITE_SCHEMA, cs2.AddToChangeGroup(group));


    ChangeGroup groupS(m_db);
    ASSERT_EQ(BE_SQLITE_OK, cs1.AddToChangeGroup(groupS));
    ASSERT_EQ(BE_SQLITE_OK, cs2.AddToChangeGroup(groupS));
    groupS.Finalize();

    m_db.SaveChanges();
    m_db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQLiteDbTests, RealUpdateTest)
    {
    SetupDb(L"RealTest.db");

    DbResult result = m_db.ExecuteSql("CREATE TABLE TestTable ([Id] INTEGER PRIMARY KEY, [ZeroReal] REAL, [IntegralReal] REAL, [FractionalReal] REAL)");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    /* Baseline: Entry with just null-s */
    result = m_db.ExecuteSql("INSERT INTO TestTable (ZeroReal,IntegralReal,FractionalReal) values (null, 1.0, 1.1)");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    int64_t rowId = m_db.GetLastInsertRowId();
    ASSERT_EQ(1, rowId);

    MyChangeTracker changeTracker(m_db);
    changeTracker.EnableTracking(true);

    /* Test 1: Update null with 0.0 */
    result = m_db.ExecuteSql("UPDATE TestTable SET ZeroReal=0.0, IntegralReal=1.0, FractionalReal=1.1  WHERE ROWID=1");
    ASSERT_TRUE(result == BE_SQLITE_OK);
    ASSERT_TRUE(changeTracker.HasChanges());

    MyChangeSet changeSet;
    changeSet.FromChangeTrack(changeTracker);
    auto size = changeSet.GetSize();
    ASSERT_TRUE(size > 0);

    changeTracker.EndTracking();
    changeSet.Clear();
    changeTracker.EnableTracking(true);

    /* Test 2: Update with no changes to integral values
    * Note: SQlite fixed a bug where this got reported as a change: https://www.sqlite.org/src/info/5f3e602831ba2eca */
    result = m_db.ExecuteSql("UPDATE TestTable SET ZeroReal=0.0, IntegralReal=1.0, FractionalReal=1.1 WHERE ROWID=1");
    ASSERT_TRUE(result == BE_SQLITE_OK);
    ASSERT_TRUE(changeTracker.HasChanges());

    changeSet.FromChangeTrack(changeTracker);
    size = changeSet.GetSize();
    ASSERT_TRUE(size == 0);

    changeTracker.EndTracking();
    changeSet.Clear();
    m_db.SaveChanges();
    m_db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQLiteDbTests, InsertMismatchedColumns)
    {
    SetupDb(L"MismatchedColumnsTest.db");

    // Create a change set with 3 columns
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TABLE TestTable ([Column1] INTEGER PRIMARY KEY, [Column2] INTEGER, [Column3] INTEGER)"));

    MyChangeTracker changeTracker(m_db);
    changeTracker.EnableTracking(true);

    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO TestTable Values(1, 2, 3)"));

    MyChangeSet changeSet;
    changeSet.FromChangeTrack(changeTracker);
    auto size = changeSet.GetSize();
    ASSERT_TRUE(size > 0);
    changeTracker.EndTracking();

    // Add a column, and attempt to apply the change set
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("ALTER TABLE TestTable ADD COLUMN [Column4]"));
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("DELETE FROM TestTable"));

    DbResult result = changeSet.ApplyChanges(m_db);
    EXPECT_TRUE(result == BE_SQLITE_OK); // SQLite lets this through, and that's good!

    // Drop a column, and attempt to apply the change set
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("DROP TABLE TestTable"));
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TABLE TestTable ([Column1] INTEGER PRIMARY KEY, [Column2] INTEGER)"));

    result = changeSet.ApplyChanges(m_db);
    EXPECT_TRUE(result == BE_SQLITE_OK); // SQLite should ideally fail here - we have reported this to them 8/31/2017
    m_db.SaveChanges();
    }

namespace {
    void CloneDb(Utf8StringCR seedFileName, DbR from, DbR out, Utf8StringCR name) {
        ASSERT_EQ (BE_SQLITE_OK, from.SaveChanges());
        Utf8String fileName = from.GetDbFileName();
        fileName.ReplaceAll(seedFileName.c_str(), name.c_str());
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(BeFileName(from.GetDbFileName(), true), BeFileName(fileName.c_str(), true)));
        ASSERT_EQ(BE_SQLITE_OK, out.OpenBeSQLiteDb(fileName.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite)));
    };
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteDbTests, ChangeSetApply_IgnoreNoop)
{
    const auto kMainFile = "test1.db";
    SetupDb (WString(kMainFile, true).c_str());
    EXPECT_TRUE (m_db.IsDbOpen ());

    ASSERT_EQ (BE_SQLITE_OK, m_db.CreateTable ("t1", "id integer primary key")) << "Creating table T1 failed.";
    ASSERT_EQ (BE_SQLITE_OK, m_db.CreateTable ("t2", "id integer primary key, t1_id integer not null references t1(id) on delete cascade")) << "Creating table T2 failed.";

    Db beforeDb;
    CloneDb(kMainFile, m_db, beforeDb, "before.db");

    // Make a changeset
    MyChangeTracker changeTracker(m_db);
    changeTracker.EnableTracking(true);
    EXPECT_EQ (BE_SQLITE_OK, m_db.ExecuteSql ("insert into t1 values(100)"));
    EXPECT_EQ (BE_SQLITE_OK, m_db.ExecuteSql ("insert into t2 values(201, 100)"));
    EXPECT_EQ (BE_SQLITE_OK, m_db.ExecuteSql ("insert into t2 values(202, 100)"));
    MyChangeSet changeSet;
    changeSet.FromChangeTrack(changeTracker);
    auto size = changeSet.GetSize();
    ASSERT_TRUE(size > 0);
    changeTracker.EndTracking();

    Db afterDb;
    CloneDb(kMainFile, m_db, afterDb, "after.db");

    BeTest::SetFailOnAssert(false);
    // Apply changeset to db that not have the data and should not cause conflicts
    ASSERT_EQ(BE_SQLITE_OK, changeSet.ApplyChanges(beforeDb)) << "this should not cause conflict";


    // Apply to a db that already have data
    ASSERT_EQ(BE_SQLITE_ABORT, changeSet.ApplyChanges(afterDb)) << "this should cause conflict and abort";

    BeTest::SetFailOnAssert(true);

    // Apply to a db that already have data but with ignoreNoop flag
    ASSERT_EQ(BE_SQLITE_OK, changeSet.ApplyChanges(afterDb, false, /* ignoreNoop = */true)) << "with ignore noop flag this should succeed";

    beforeDb.SaveChanges();
    afterDb.SaveChanges();
}

TEST_F(BeSQLiteDbTests, ChangeSetApply_IgnoreNoopShouldNotSupressConflict)
{
    const auto kMainFile = "test1.db";
    SetupDb(WString(kMainFile, true).c_str());
    ASSERT_TRUE(m_db.IsDbOpen());

    // Data setup with a table and some data
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable("t1", "id integer primary key, val int"));
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("insert into t1 values(1, 10)"));
    ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("insert into t1 values(2, 20)"));
    m_db.SaveChanges();

    Db firstBriefcaseDb, secondBriefcaseDb;
    CloneDb(kMainFile, m_db, firstBriefcaseDb, "firstBriefcase.db");
    CloneDb(kMainFile, m_db, secondBriefcaseDb, "secondBriefcase.db");
    m_db.CloseDb();

    // Delete a row from the first briefcase and create a changeset
    MyChangeTracker changeTracker(firstBriefcaseDb);
    changeTracker.EnableTracking(true);
    ASSERT_EQ(BE_SQLITE_OK, firstBriefcaseDb.ExecuteSql("delete from t1 where id=2"));

    MyChangeSet changeSet;
    changeSet.FromChangeTrack(changeTracker);
    ASSERT_GT(changeSet.GetSize(), 0);
    changeTracker.EndTracking();
    firstBriefcaseDb.SaveChanges();

    // Update the value in the second briefcase
    ASSERT_EQ(BE_SQLITE_OK, secondBriefcaseDb.ExecuteSql("update t1 set val=500 where id=2"));
    secondBriefcaseDb.SaveChanges();

    {
        // Make sure the row is deleted from the first briefcase
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(firstBriefcaseDb, "SELECT val from t1 where id=2"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        stmt.Finalize();
    }
    {
        // Make sure the row is updated in the second briefcase
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(secondBriefcaseDb, "SELECT val from t1 where id=2"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(500, stmt.GetValueDouble(0));
        stmt.Finalize();
    }

    BeTest::SetFailOnAssert(false);
    // This is not a no-op and should trigger a conflict: Trying to delete a row that was updated in secondBriefcaseDb.
    ASSERT_EQ(BE_SQLITE_ABORT, changeSet.ApplyChanges(secondBriefcaseDb, false, /*ignoreNoop=*/true));
    BeTest::SetFailOnAssert(true);

    firstBriefcaseDb.SaveChanges();
    secondBriefcaseDb.SaveChanges();

    firstBriefcaseDb.CloseDb();
    secondBriefcaseDb.CloseDb();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteDbTests, GetColumn)
{
    SetupDb (L"test1.db");
    EXPECT_TRUE (m_db.IsDbOpen ());

    ASSERT_EQ (BE_SQLITE_OK, m_db.CreateTable ("TestTable1", "id NUMERIC, name TEXT")) << "Creating table failed.";
    EXPECT_EQ (BE_SQLITE_OK, m_db.ExecuteSql ("INSERT INTO TestTable1 Values(1, 'test')"));

    bvector<Utf8String> buff;
    bool res1= m_db.GetColumns (buff ,"TestTable1");
    EXPECT_EQ (res1, true);

    bool IdStatus = false;
    IdStatus = buff[0].Equals(Utf8String ("id"));
    EXPECT_EQ (IdStatus, true);

    bool nameStatus = false;
    nameStatus = buff[1].Equals(Utf8String ("name"));
    EXPECT_EQ (nameStatus, true);

    EXPECT_EQ (BE_SQLITE_OK, m_db.AbandonChanges ());
    m_db.CloseDb ();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteDbTests, SaveCreationDate)
{
    bool result=false;
    SetupDb (L"test2.db");
    EXPECT_TRUE (m_db.IsDbOpen ());

    BentleyM0200::DateTime currentDate = BentleyM0200::DateTime::GetCurrentTimeUtc ();
    EXPECT_EQ (BE_SQLITE_OK, m_db.SaveCreationDate ());

    BentleyM0200::DateTime newDate;
    EXPECT_EQ(BE_SQLITE_ROW, m_db.QueryCreationDate(newDate));

    if ( (newDate.GetYear()==currentDate.GetYear()) && (newDate.GetMonth()==currentDate.GetMonth()) && (newDate.GetDay () == currentDate.GetDay ()) && (newDate.GetHour()==currentDate.GetHour()) && (newDate.GetMinute()==currentDate.GetMinute()) && (newDate.GetSecond()==currentDate.GetSecond()) )
    {
        result=true;
    }
    EXPECT_TRUE (result);

    m_db.SaveChanges();
    m_db.CloseDb ();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteDbTests, AddColumnToTable)
{
    SetupDb (L"test1.db");
    EXPECT_TRUE (m_db.IsDbOpen ());

    ASSERT_EQ (BE_SQLITE_OK, m_db.CreateTable ("TestTable2", "id NUMERIC, name TEXT")) << "Creating table failed.";
    EXPECT_EQ (BE_SQLITE_OK, m_db.AddColumnToTable ("TestTable2", "TitleId", "INTEGER"));

    bvector<Utf8String> buff;
    bool res1 = m_db.GetColumns (buff, "TestTable2");
    EXPECT_EQ (res1, true);

    bool IdStatus = false;
    IdStatus = buff[2].Equals (Utf8String ("TitleId"));
    EXPECT_EQ (IdStatus, true);

    EXPECT_EQ (BE_SQLITE_OK, m_db.CreateIndex("newInd", "TestTable2",true,"id",nullptr));

    m_db.SaveChanges();
    m_db.CloseDb ();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQLiteDbTests, CreateChangeSetWithSchemaAndDataChanges)
    {
    /* Tests that
     * 1. Schema and data changes are not allowed to the same tables in the same change set.
     * This validates our assumptions in storing transactions and creating revisions
     * 2. Schema and data changes can be made to different tables in the same change set.
     * This validates our assumptions in allowing this for bridge-framework workflows when
     * importing v8 legacy schemas.
     * */

    SetupDb(L"RealTest.db");

    DbResult result = m_db.ExecuteSql("CREATE TABLE TestTable1 ([Id] INTEGER PRIMARY KEY, [Column1] REAL)");
    ASSERT_TRUE(result == BE_SQLITE_OK);
    result = m_db.ExecuteSql("CREATE TABLE TestTable2 ([Id] INTEGER PRIMARY KEY, [Column1] REAL)");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    MyChangeTracker changeTracker(m_db);
    changeTracker.EnableTracking(true);

    // Add row to TestTable1
    result = m_db.ExecuteSql("INSERT INTO TestTable1 (Column1) values (1.1)");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    // Add column to TestTable2
    result = m_db.AddColumnToTable("TestTable2", "Column2", "REAL");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    // Add row to TestTable2
    result = m_db.ExecuteSql("INSERT INTO TestTable2 (Column1,Column2) values (1.1,2.2)");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    // Create change set - succeeds!
    MyChangeSet changeSet;
    result = changeSet.FromChangeTrack(changeTracker);
    ASSERT_TRUE(result == BE_SQLITE_OK);

    // Add column to TestTable1
    result = m_db.AddColumnToTable("TestTable1", "Column2", "REAL");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    // Create change set - fails!
    changeSet.Clear();
    result = changeSet.FromChangeTrack(changeTracker);
    ASSERT_TRUE(result == BE_SQLITE_OK); // This does not fail any more

    // Add row to TestTable 1
    result = m_db.ExecuteSql("INSERT INTO TestTable1 (Column1,Column2) values (3.3,4.4)");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    // Create change set - fails!
    changeSet.Clear();
    result = changeSet.FromChangeTrack(changeTracker);
    ASSERT_TRUE(result == BE_SQLITE_OK); // This does not fail any more

    changeTracker.EndTracking();
    changeSet.Clear();

    m_db.SaveChanges();
    m_db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQLiteDbTests, ApplyChangeSetAfterSchemaChanges)
    {
    /* Tests that you can create a change set, make a schema change, and then apply that change
     * set to the Db. This validates our assumption that we can merge a schema revision when
     * there are local changes */

    SetupDb(L"RealTest.db");

    DbResult result = m_db.ExecuteSql("CREATE TABLE TestTable ([Id] INTEGER PRIMARY KEY, [Column1] REAL, [Column2] REAL)");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    MyChangeTracker changeTracker(m_db);
    changeTracker.EnableTracking(true);

    // Add row
    result = m_db.ExecuteSql("INSERT INTO TestTable (Column1,Column2) values (1.1,2.2)");
    EXPECT_TRUE(result == BE_SQLITE_OK);

    // Create change set
    MyChangeSet changeSet;
    result = changeSet.FromChangeTrack(changeTracker);
    EXPECT_TRUE(result == BE_SQLITE_OK);
    changeTracker.EndTracking();

    changeSet.Dump("ChangeSet", m_db);

    // Delete all rows
    result = m_db.ExecuteSql("DELETE FROM TestTable");
    EXPECT_TRUE(result == BE_SQLITE_OK);

    // Add column
    result = m_db.AddColumnToTable("TestTable", "Column3", "REAL");
    EXPECT_TRUE(result == BE_SQLITE_OK);

    // Apply change set
    result = changeSet.ApplyChanges(m_db);
    EXPECT_TRUE(result == BE_SQLITE_OK);

    // Validate
    Statement stmt;
    result = stmt.Prepare(m_db, "SELECT Column1 FROM TestTable WHERE Column2=2.2");
    EXPECT_TRUE(result == BE_SQLITE_OK);
    result = stmt.Step();
    EXPECT_TRUE(result == BE_SQLITE_ROW);
    EXPECT_EQ(1.1, stmt.GetValueDouble(0));

    changeSet.Clear();
    stmt.Finalize();
    m_db.SaveChanges();
    m_db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteDbTests, SaveQueryDelBreifCaselocalValue)
{
    SetupDb (L"testb.db");
    EXPECT_TRUE (m_db.IsDbOpen ());

    Utf8CP name = "test";
    EXPECT_EQ (BE_SQLITE_DONE, m_db.SaveBriefcaseLocalValue (name, 2));

    uint64_t value;
    EXPECT_EQ (BE_SQLITE_ROW, m_db.QueryBriefcaseLocalValue (value, name));
    ASSERT_EQ (value, 2);

    EXPECT_EQ (BE_SQLITE_DONE, m_db.DeleteBriefcaseLocalValue (name));
    EXPECT_NE (BE_SQLITE_ROW, m_db.QueryBriefcaseLocalValue (value, name));
    m_db.SaveChanges();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteDbTests, DelProperties)
{
    SetupDb (L"Props3.db");

    PropertySpec spec1 ("TestSpec", "TestApplication");
    m_result = m_db.SaveProperty (spec1, L"Any Value", 10, 400, 10);
    EXPECT_EQ (BE_SQLITE_OK, m_result) << "SaveProperty failed";
    EXPECT_TRUE (m_db.HasProperty (spec1, 400, 10));

    uint64_t * mid = 0;
    m_result = m_db.DeleteProperties (spec1, mid);
    EXPECT_EQ (BE_SQLITE_DONE, m_result) << "DeleteProperty failed";
    EXPECT_FALSE (m_db.HasProperty (spec1, 400, 10));
    m_db.SaveChanges();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteDbTests, MaxBlobSize)
{
    SetupDb (L"Props3.db");
    EXPECT_TRUE (m_db.IsDbOpen ());

    auto hardMaxLimit = 2147483647;
    auto maxLength = m_db.GetLimit(DbLimits::Length);
    ASSERT_EQ(hardMaxLimit, maxLength);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteDbTests, Limits)
{
    SetupDb (L"limits.db");
    EXPECT_TRUE (m_db.IsDbOpen ());

    ASSERT_EQ(10, m_db.GetLimit(DbLimits::Attached));
    ASSERT_EQ(2200, m_db.GetLimit(DbLimits::Column));
    ASSERT_EQ(500, m_db.GetLimit(DbLimits::CompoundSelect));
    ASSERT_EQ(3000, m_db.GetLimit(DbLimits::ExprDepth));
    ASSERT_EQ(1000, m_db.GetLimit(DbLimits::FunctionArg));
    ASSERT_EQ(2147483647, m_db.GetLimit(DbLimits::Length));
    ASSERT_EQ(50000, m_db.GetLimit(DbLimits::LikePatternLength));
    ASSERT_EQ(1000000000, m_db.GetLimit(DbLimits::SqlLength));
    ASSERT_EQ(1000, m_db.GetLimit(DbLimits::TriggerDepth));
    ASSERT_EQ(20000, m_db.GetLimit(DbLimits::VariableNumber));
    ASSERT_EQ(250000000, m_db.GetLimit(DbLimits::VdbeOp));
    ASSERT_EQ(0, m_db.GetLimit(DbLimits::WorkerThreads));
    m_db.AbandonChanges();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
#if 0 // Require ICU
TEST_F (BeSQLiteDbTests, icu_upper_lower_func) {

    SetupDb (L"icu_case.db");
    EXPECT_TRUE (m_db.IsDbOpen ());
    auto toLower = [&](Utf8String str) {
        auto stmt = m_db.GetCachedStatement("SELECT LOWER(?)");
        stmt->BindText(1, str.c_str(), Statement::MakeCopy::Yes);
        stmt->Step();
        return Utf8String(stmt->GetValueText(0));
    };
    auto toUpper = [&](Utf8String str) {
        auto stmt = m_db.GetCachedStatement("SELECT UPPER(?)");
        stmt->BindText(1, str.c_str(), Statement::MakeCopy::Yes);
        stmt->Step();
        return Utf8String(stmt->GetValueText(0));
    };

    const Utf8String expectedUpper = "À Á Â Ã Ä Å Æ Ç È É Ê Ë Ì Í Î Ï Ð Ñ Ò Ó Ô Õ Ö × Ø Ù Ú Û Ü Ý Ÿ Þ ¡ ¢ £ ¤ ¥ ¦ § ¨ © ª « ¬ ­ ® ¯ ° ± ² ³ ´ Μ ¶ ¸ ¹ º » ¼ ½ ¾ ¿ Ƒ ·";
    const Utf8String expectedLower = "à á â ã ä å æ ç è é ê ë ì í î ï ð ñ ò ó ô õ ö × ø ù ú û ü ý ÿ þ ¡ ¢ £ ¤ ¥ ¦ § ¨ © ª « ¬ ­ ® ¯ ° ± ² ³ ´ μ ¶ ¸ ¹ º » ¼ ½ ¾ ¿ ƒ ·";
    const Utf8String actualUpper = toUpper(expectedLower);
    const Utf8String actualLower = toLower(expectedUpper);

    ASSERT_STREQ(expectedUpper.c_str(), actualUpper.c_str());
    ASSERT_STREQ(expectedLower.c_str(), actualLower.c_str());
    ASSERT_STREQ("SS", toUpper("ß").c_str());
}
#endif
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteDbTests, nocase_latin1_ascii_support)
{
    SetupDb (L"icu.db");
    EXPECT_TRUE (m_db.IsDbOpen ());
    auto setupTable = [&](Utf8CP tableName, Utf8CP collation) {
        ASSERT_EQ(BE_SQLITE_OK, m_db.ExecuteDdl(SqlPrintfString("CREATE TABLE [%s](str TEXT UNIQUE COLLATE %s)", tableName, collation)));
    };
    auto insert = [&](Utf8CP tableName, Utf8CP str) {
        auto stmt = m_db.GetCachedStatement(SqlPrintfString("INSERT INTO [%s](str) VALUES(?1)", tableName));
        stmt->BindText(1, str, Statement::MakeCopy::Yes);
        return stmt->Step();
    };
    auto countWhere = [&](Utf8CP tableName, Utf8CP str) {
        auto stmt = m_db.GetCachedStatement(SqlPrintfString("SELECT COUNT(*) FROM %s WHERE str = ?", tableName));
        stmt->BindText(1, str, Statement::MakeCopy::No);
        if(stmt->Step() == BE_SQLITE_ROW){
            return stmt->GetValueInt(0);
        }
        return -1;
    };
    auto countWhereWithoutIndex = [&](Utf8CP tableName, Utf8CP str) {
        auto stmt = m_db.GetCachedStatement(SqlPrintfString("SELECT COUNT(*) FROM %s WHERE +str = ?", tableName));
        stmt->BindText(1, str, Statement::MakeCopy::No);
        if(stmt->Step() == BE_SQLITE_ROW){
            return stmt->GetValueInt(0);
        }
        return -1;
    };
    auto reindex = [&](Utf8CP tableName) {
        auto stmt = m_db.GetCachedStatement(SqlPrintfString("REINDEX %s", tableName));
        return stmt->Step();
    };
    ASSERT_EQ(m_db.GetNoCaseCollation(), NoCaseCollation::ASCII);
    setupTable("test1", "NOCASE");
    ASSERT_EQ(BE_SQLITE_DONE, insert("test1", "ÀÁÂÃÄÅ"));
    ASSERT_EQ(BE_SQLITE_DONE, insert("test1", "àáâãäå"));
    ASSERT_EQ(BE_SQLITE_DONE, insert("test1", "ÀÁÂãäå"));
    ASSERT_EQ(BE_SQLITE_DONE, insert("test1", "àáâÃÄÅ"));
    ASSERT_EQ(BE_SQLITE_DONE, insert("test1", "aaaaaa"));

    ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, insert("test1", "ÀÁÂÃÄÅ"));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, insert("test1", "àáâãäå"));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, insert("test1", "AAAaaa"));
    m_db.SaveChanges();

    ASSERT_EQ(countWhere("test1", "ÀÁÂÃÄÅ"), 1);
    ASSERT_EQ(countWhere("test1", "AAAAAA"), 1);
    ASSERT_EQ(countWhere("test1", "àáâãäå"), 1);
    ASSERT_EQ(countWhere("test1", "aaaaaa"), 1);
    ASSERT_EQ(countWhere("test1", "AAAaaa"), 1);
    ASSERT_EQ(countWhere("test1", "ÀÁÂãäå"), 1);

    m_db.GetStatementCache().Empty();
    ASSERT_EQ(m_db.SetNoCaseCollation(NoCaseCollation::Latin1), BE_SQLITE_OK);
    ASSERT_EQ(m_db.GetNoCaseCollation(), NoCaseCollation::Latin1);
    // with index we still going to get wrong answer after
    // enabling Latin1 case insensitive with ignore accents
    ASSERT_EQ(countWhere("test1", "ÀÁÂÃÄÅ"), 1);
    ASSERT_EQ(countWhere("test1", "AAAAAA"), 1);
    ASSERT_EQ(countWhere("test1", "àáâãäå"), 1);
    ASSERT_EQ(countWhere("test1", "aaaaaa"), 1);
    ASSERT_EQ(countWhere("test1", "AAAaaa"), 1);
    ASSERT_EQ(countWhere("test1", "ÀÁÂãäå"), 1);

    // without index count correlate with NOCASE
    ASSERT_EQ(countWhereWithoutIndex("test1", "ÀÁÂÃÄÅ"), 5);
    ASSERT_EQ(countWhereWithoutIndex("test1", "AAAAAA"), 5);
    ASSERT_EQ(countWhereWithoutIndex("test1", "àáâãäå"), 5);
    ASSERT_EQ(countWhereWithoutIndex("test1", "aaaaaa"), 5);
    ASSERT_EQ(countWhereWithoutIndex("test1", "AAAaaa"), 5);
    ASSERT_EQ(countWhereWithoutIndex("test1", "ÀÁÂãäå"), 5);

    ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, insert("test1", "aaaÃÄÅ"));

    // index failed due to duplicate values
    ASSERT_EQ(reindex("test1"), BE_SQLITE_CONSTRAINT_UNIQUE);

    // switch back to ascii
    m_db.GetStatementCache().Empty();
    ASSERT_EQ(m_db.SetNoCaseCollation(NoCaseCollation::ASCII), BE_SQLITE_OK);
    ASSERT_EQ(m_db.GetNoCaseCollation(), NoCaseCollation::ASCII);
    ASSERT_EQ(BE_SQLITE_DONE, insert("test1", "ÀÁÂÃÄå"));
    ASSERT_EQ(BE_SQLITE_DONE, insert("test1", "ÀÁâãÄÅ"));


    m_db.SaveChanges();
}
