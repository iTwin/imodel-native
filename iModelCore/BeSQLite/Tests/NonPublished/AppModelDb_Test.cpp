/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BeSQLiteNonPublishedTests.h"
#include <BeSQLite/AppModelDb.h>

USING_NAMESPACE_BENTLEY_APPMODEL

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
static BeFileName GetOutputFile(WCharCP name)
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLiteLib::Initialize(tempDir);

    BeFileName fileName;
    BeTest::GetHost().GetOutputRoot(fileName);
    fileName.AppendToPath(name);
    if (BeFileName::DoesPathExist(fileName))
        BeFileName::BeDeleteFile(fileName);

    return fileName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(AppModelDb, CreateStampsProfileAndCreatesTables)
    {
    BeFileName fileName = GetOutputFile(L"appmodeldb_create.db");

    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(fileName.GetNameUtf8().c_str()));

    // The AppModel profile creates its own set of tables internally.
    EXPECT_TRUE(db.TableExists("Dgns"));
    EXPECT_TRUE(db.TableExists("DgnStreams"));
    EXPECT_TRUE(db.TableExists("Models"));
    EXPECT_TRUE(db.TableExists("Elements"));
    EXPECT_TRUE(db.TableExists("DgnReferencesModels"));
    EXPECT_TRUE(db.TableExists("ModelUsesDgnLibElement"));
    EXPECT_TRUE(db.TableExists("Schemas"));
    EXPECT_TRUE(db.TableExists("DgnContainsSchemas"));
    EXPECT_TRUE(db.TableExists("SchemaClasses"));
    EXPECT_TRUE(db.TableExists("RasterImages"));

    ProfileVersion version(0, 0, 0, 0);
    ASSERT_EQ(BE_SQLITE_OK, db.QueryProfileVersion(version));
    EXPECT_EQ(0, version.CompareTo(AppModelDb::CurrentProfileVersion())) << "Created file must carry the current AppModel profile version";

    db.SaveChanges();
    db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(AppModelDb, ReopenSucceedsForCurrentProfile)
    {
    BeFileName fileName = GetOutputFile(L"appmodeldb_reopen.db");

    {
    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(fileName.GetNameUtf8().c_str()));
    db.SaveChanges();
    db.CloseDb();
    }

    AppModelDb reopened;
    ASSERT_EQ(BE_SQLITE_OK, reopened.OpenBeSQLiteDb(fileName.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::ReadWrite)))
        << "A file with the current AppModel profile version should open without upgrade";
    EXPECT_TRUE(reopened.TableExists("Elements"));
    }

//---------------------------------------------------------------------------------------
// A plain BeSQLite file has no AppModel profile, so opening it as an AppModelDb must fail.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(AppModelDb, OpenRejectsFileWithoutAppModelProfile)
    {
    BeFileName fileName = GetOutputFile(L"appmodeldb_plain.db");

    {
    Db plain;
    ASSERT_EQ(BE_SQLITE_OK, plain.CreateNewDb(fileName.GetNameUtf8().c_str()));
    plain.SaveChanges();
    plain.CloseDb();
    }

    AppModelDb db;
    EXPECT_NE(BE_SQLITE_OK, db.OpenBeSQLiteDb(fileName.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::Readonly)))
        << "A plain BeSQLite file (no AppModel profile) must be rejected";
    }

//---------------------------------------------------------------------------------------
// Helper: count rows in a table.
//+---------------+---------------+---------------+---------------+---------------+------
static int CountRows(Db& db, Utf8CP table)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(db, Utf8PrintfString("SELECT COUNT(*) FROM %s", table).c_str()))
        return -1;
    stmt.Step();
    return stmt.GetValueInt(0);
    }

//---------------------------------------------------------------------------------------
// Change tracking is on by default; a SaveChanges that produced data changes writes one txn row.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(AppModelDb, SaveChangesRecordsDataTxn)
    {
    BeFileName fileName = GetOutputFile(L"appmodeldb_txn_data.db");

    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(fileName.GetNameUtf8().c_str()));
    EXPECT_TRUE(db.GetTxns().IsTracking()) << "Change tracking should be active after create";
    EXPECT_FALSE(db.GetTxns().HasPendingTxns()) << "Creating the profile must not record any txn";

    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql("INSERT INTO Dgns(Name,IsDgnLib) VALUES('dgn1',0)"));
    db.SaveChanges();

    ASSERT_EQ(1, CountRows(db, "appmodel_txns")) << "A data change should record exactly one txn";

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "SELECT Type,Ddl,(Change IS NOT NULL) FROM appmodel_txns"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ((int) TxnType::Data, stmt.GetValueInt(0));
    EXPECT_TRUE(stmt.IsColumnNull(1)) << "A data-only txn has no DDL";
    EXPECT_NE(0, stmt.GetValueInt(2)) << "A data-only txn has a Change blob";
    db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// Metadata (description + JSON props) is stored on the next committed txn.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(AppModelDb, SaveChangesRecordsMetadata)
    {
    BeFileName fileName = GetOutputFile(L"appmodeldb_txn_meta.db");

    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(fileName.GetNameUtf8().c_str()));

    db.GetTxns().SetTxnProps(R"({"author":"tester"})");
    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql("INSERT INTO Dgns(Name,IsDgnLib) VALUES('dgn1',0)"));
    db.SaveChanges("added dgn1"); // the SaveChanges description is stored on the txn

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "SELECT Description,Props FROM appmodel_txns"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_STREQ("added dgn1", stmt.GetValueText(0));
    EXPECT_STREQ(R"({"author":"tester"})", stmt.GetValueText(1));
    db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// A txn that contains DDL is recorded with Type=Schema and the DDL text in the Ddl column.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(AppModelDb, SaveChangesRecordsSchemaTxn)
    {
    BeFileName fileName = GetOutputFile(L"appmodeldb_txn_schema.db");

    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(fileName.GetNameUtf8().c_str()));

    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteDdl("CREATE TABLE app_Custom(Id INTEGER PRIMARY KEY, Val TEXT)"));
    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql("INSERT INTO app_Custom(Val) VALUES('x')"));
    db.SaveChanges();

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "SELECT Type,Ddl FROM appmodel_txns"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ((int) TxnType::Schema, stmt.GetValueInt(0));
    EXPECT_FALSE(stmt.IsColumnNull(1)) << "A txn with DDL stores the DDL text";
    db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// AbandonChanges must not record a txn.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(AppModelDb, AbandonChangesRecordsNoTxn)
    {
    BeFileName fileName = GetOutputFile(L"appmodeldb_txn_abandon.db");

    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(fileName.GetNameUtf8().c_str()));
    db.SaveChanges();

    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql("INSERT INTO Dgns(Name,IsDgnLib) VALUES('dgn1',0)"));
    db.AbandonChanges();

    EXPECT_EQ(0, CountRows(db, "appmodel_txns"));
    EXPECT_EQ(0, CountRows(db, "Dgns"));
    db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// Recorded txns can be combined into a single AppModel changeset file, with a SHA1 id.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(AppModelDb, CreateChangesetWritesFileAndComputesId)
    {
    BeFileName fileName = GetOutputFile(L"appmodeldb_cs_source.db");
    BeFileName changesetFile = GetOutputFile(L"appmodeldb.changeset");

    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(fileName.GetNameUtf8().c_str()));
    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql("INSERT INTO Dgns(Name,IsDgnLib) VALUES('dgn1',0)"));
    db.SaveChanges();

    ChangesetProps props;
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().BeginCreateChangeset(props, changesetFile));
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().EndCreateChangeset());
    EXPECT_TRUE(BeFileName::DoesPathExist(changesetFile));
    EXPECT_EQ(40u, props.GetChangesetId().length()) << "changeset id should be a 40-char SHA1 hex string";
    EXPECT_TRUE(props.GetParentId().empty()) << "first changeset has no parent";
    EXPECT_STREQ(db.GetDbGuid().ToString().c_str(), props.GetDbGuid().c_str());

    // The props validate against this db (correct db guid + recomputed SHA1 id).
    EXPECT_EQ(BE_SQLITE_OK, props.ValidateContent(db));

    // Tampering with the recorded id must fail validation.
    ChangesetProps bad = props;
    bad.m_id = "0000000000000000000000000000000000000000";
    EXPECT_EQ(BE_SQLITE_ERROR_InvalidChangeSetVersion, bad.ValidateContent(db));
    db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// BeginCreateChangeset writes the file but keeps the captured txns and does not advance the changeset id;
// EndCreateChangeset deletes the captured txns and advances the changeset id, so a changeset created afterward
// chains onto the previous one.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(AppModelDb, BeginEndCreateChangesetClearsTxnsAndChains)
    {
    BeFileName fileName = GetOutputFile(L"appmodeldb_beginend.db");
    BeFileName cs1File = GetOutputFile(L"appmodeldb_beginend1.changeset");
    BeFileName cs2File = GetOutputFile(L"appmodeldb_beginend2.changeset");

    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(fileName.GetNameUtf8().c_str()));

    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql("INSERT INTO Dgns(Name,IsDgnLib) VALUES('dgn1',0)"));
    db.SaveChanges();
    EXPECT_TRUE(db.GetTxns().HasPendingTxns());

    ChangesetProps cs1;
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().BeginCreateChangeset(cs1, cs1File));
    EXPECT_TRUE(cs1.GetParentId().empty()) << "first changeset has no parent";
    EXPECT_TRUE(db.GetTxns().HasPendingTxns()) << "BeginCreateChangeset must not delete the captured txns";
    EXPECT_TRUE(db.GetTxns().GetParentChangesetId().empty()) << "BeginCreateChangeset must not advance the changeset id";

    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().EndCreateChangeset());
    EXPECT_FALSE(db.GetTxns().HasPendingTxns()) << "EndCreateChangeset must delete the captured txns";
    EXPECT_STREQ(cs1.GetChangesetId().c_str(), db.GetTxns().GetParentChangesetId().c_str()) << "EndCreateChangeset advances the changeset id";

    // A second changeset created after the first chains onto it.
    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql("INSERT INTO Dgns(Name,IsDgnLib) VALUES('dgn2',0)"));
    db.SaveChanges();
    ChangesetProps cs2;
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().BeginCreateChangeset(cs2, cs2File));
    EXPECT_STREQ(cs1.GetChangesetId().c_str(), cs2.GetParentId().c_str()) << "the second changeset chains onto the first";
    EXPECT_STRNE(cs1.GetChangesetId().c_str(), cs2.GetChangesetId().c_str());
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().EndCreateChangeset());

    // EndCreateChangeset with no create in progress is an error.
    EXPECT_NE(BE_SQLITE_OK, db.GetTxns().EndCreateChangeset()) << "EndCreateChangeset without a matching Begin must fail";
    db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// A changeset created from one copy of a db can be merged into another copy (same db guid),
// and the merged db advances its parent changeset id.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(AppModelDb, CreateAndMergeChangeset)
    {
    BeFileName seed = GetOutputFile(L"appmodeldb_seed.db");
    BeFileName source = GetOutputFile(L"appmodeldb_src.db");
    BeFileName target = GetOutputFile(L"appmodeldb_tgt.db");
    BeFileName changesetFile = GetOutputFile(L"appmodeldb_merge.changeset");

    // Create an empty seed and copy it to source and target (both share the same db guid).
    {
    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(seed.GetNameUtf8().c_str()));
    db.SaveChanges();
    db.CloseDb();
    }
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seed, source));
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seed, target));

    // Make a change on the source and produce a changeset.
    ChangesetProps props;
    {
    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(source.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::ReadWrite)));
    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql("INSERT INTO Dgns(Name,IsDgnLib) VALUES('dgn1',0)"));
    db.SaveChanges("add dgn1");
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().BeginCreateChangeset(props, changesetFile));
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().EndCreateChangeset());
    db.CloseDb();
    }

    // Merge the changeset into the target and verify the change arrived and the parent advanced.
    {
    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(target.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::ReadWrite)));
    EXPECT_EQ(0, CountRows(db, "Dgns"));
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().MergeChangeset(props));
    EXPECT_EQ(1, CountRows(db, "Dgns")) << "the merged change should be present";
    EXPECT_STREQ(props.GetChangesetId().c_str(), db.GetTxns().GetParentChangesetId().c_str()) << "parent id should advance to the merged changeset";
    EXPECT_EQ(0, CountRows(db, "appmodel_txns")) << "merging must not record new txns";
    db.CloseDb();
    }
    }

//---------------------------------------------------------------------------------------
// CreateChangeset with no recorded txns is an error (nothing to serialize).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(AppModelDb, CreateChangesetFailsWithNoTxns)
    {
    BeFileName fileName = GetOutputFile(L"appmodeldb_cs_empty.db");
    BeFileName changesetFile = GetOutputFile(L"appmodeldb_empty.changeset");

    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(fileName.GetNameUtf8().c_str()));
    db.SaveChanges();

    ChangesetProps props;
    EXPECT_NE(BE_SQLITE_OK, db.GetTxns().BeginCreateChangeset(props, changesetFile)) << "creating a changeset with no txns must fail";
    EXPECT_FALSE(BeFileName::DoesPathExist(changesetFile)) << "no changeset file should be produced";
    db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// A changeset whose txns include DDL is marked as a Schema changeset.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(AppModelDb, CreateChangesetMarksSchemaType)
    {
    BeFileName fileName = GetOutputFile(L"appmodeldb_cs_schema.db");
    BeFileName changesetFile = GetOutputFile(L"appmodeldb_schema.changeset");

    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(fileName.GetNameUtf8().c_str()));
    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteDdl("CREATE TABLE app_Custom(Id INTEGER PRIMARY KEY, Val TEXT)"));
    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql("INSERT INTO app_Custom(Val) VALUES('x')"));
    db.SaveChanges();

    ChangesetProps props;
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().BeginCreateChangeset(props, changesetFile));
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().EndCreateChangeset());
    EXPECT_EQ(ChangesetType::Schema, props.GetChangesetType()) << "a changeset containing DDL is a Schema changeset";
    EXPECT_TRUE(props.ContainsSchemaChanges());
    db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// A changeset can only be merged into a db that shares the originating db's guid.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(AppModelDb, MergeRejectsWrongDbGuid)
    {
    BeFileName source = GetOutputFile(L"appmodeldb_guid_src.db");
    BeFileName other = GetOutputFile(L"appmodeldb_guid_other.db");
    BeFileName changesetFile = GetOutputFile(L"appmodeldb_guid.changeset");

    ChangesetProps props;
    {
    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(source.GetNameUtf8().c_str()));
    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql("INSERT INTO Dgns(Name,IsDgnLib) VALUES('dgn1',0)"));
    db.SaveChanges();
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().BeginCreateChangeset(props, changesetFile));
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().EndCreateChangeset());
    db.CloseDb();
    }

    // An independently-created db has a different guid, so the merge must be rejected.
    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(other.GetNameUtf8().c_str()));
    db.SaveChanges();
    EXPECT_EQ(BE_SQLITE_MISMATCH, db.GetTxns().MergeChangeset(props)) << "a changeset from a different db must be rejected";
    EXPECT_EQ(0, CountRows(db, "Dgns"));
    db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// A changeset must chain onto the target's current changeset; re-merging one whose parent no longer
// matches is rejected.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(AppModelDb, MergeRejectsWrongParent)
    {
    BeFileName seed = GetOutputFile(L"appmodeldb_par_seed.db");
    BeFileName source = GetOutputFile(L"appmodeldb_par_src.db");
    BeFileName target = GetOutputFile(L"appmodeldb_par_tgt.db");
    BeFileName changesetFile = GetOutputFile(L"appmodeldb_par.changeset");

    {
    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(seed.GetNameUtf8().c_str()));
    db.SaveChanges();
    db.CloseDb();
    }
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seed, source));
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seed, target));

    ChangesetProps props;
    {
    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(source.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::ReadWrite)));
    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql("INSERT INTO Dgns(Name,IsDgnLib) VALUES('dgn1',0)"));
    db.SaveChanges();
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().BeginCreateChangeset(props, changesetFile));
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().EndCreateChangeset());
    db.CloseDb();
    }

    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(target.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::ReadWrite)));
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().MergeChangeset(props)); // first merge advances parent to props.id
    EXPECT_STREQ(props.GetChangesetId().c_str(), db.GetTxns().GetParentChangesetId().c_str());
    // Re-merging the same changeset now fails: its parent ("") no longer matches the target's current changeset.
    EXPECT_EQ(BE_SQLITE_MISMATCH, db.GetTxns().MergeChangeset(props));
    EXPECT_EQ(1, CountRows(db, "Dgns")) << "the rejected re-merge must not change data";
    db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// A changeset whose id does not match its contents is rejected on merge.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(AppModelDb, MergeRejectsTamperedId)
    {
    BeFileName seed = GetOutputFile(L"appmodeldb_tamper_seed.db");
    BeFileName source = GetOutputFile(L"appmodeldb_tamper_src.db");
    BeFileName target = GetOutputFile(L"appmodeldb_tamper_tgt.db");
    BeFileName changesetFile = GetOutputFile(L"appmodeldb_tamper.changeset");

    {
    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(seed.GetNameUtf8().c_str()));
    db.SaveChanges();
    db.CloseDb();
    }
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seed, source));
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seed, target));

    ChangesetProps props;
    {
    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(source.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::ReadWrite)));
    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql("INSERT INTO Dgns(Name,IsDgnLib) VALUES('dgn1',0)"));
    db.SaveChanges();
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().BeginCreateChangeset(props, changesetFile));
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().EndCreateChangeset());
    db.CloseDb();
    }

    props.m_id = "0000000000000000000000000000000000000000"; // wrong id for the contents

    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(target.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::ReadWrite)));
    EXPECT_EQ(BE_SQLITE_ERROR_InvalidChangeSetVersion, db.GetTxns().MergeChangeset(props));
    EXPECT_EQ(0, CountRows(db, "Dgns")) << "a changeset with a bad id must not be applied";
    db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// A conflicting change aborts the whole merge and rolls back; the target keeps its own data and its
// parent changeset id does not advance.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST(AppModelDb, MergeConflictAbortsAndRollsBack)
    {
    BeFileName seed = GetOutputFile(L"appmodeldb_conf_seed.db");
    BeFileName source = GetOutputFile(L"appmodeldb_conf_src.db");
    BeFileName target = GetOutputFile(L"appmodeldb_conf_tgt.db");
    BeFileName changesetFile = GetOutputFile(L"appmodeldb_conf.changeset");

    {
    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(seed.GetNameUtf8().c_str()));
    db.SaveChanges();
    db.CloseDb();
    }
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seed, source));
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seed, target));

    // Source inserts Dgns row with explicit ID=1 and produces a changeset.
    ChangesetProps props;
    {
    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(source.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::ReadWrite)));
    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql("INSERT INTO Dgns(ID,Name,IsDgnLib) VALUES(1,'from-source',0)"));
    db.SaveChanges();
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().BeginCreateChangeset(props, changesetFile));
    ASSERT_EQ(BE_SQLITE_OK, db.GetTxns().EndCreateChangeset());
    db.CloseDb();
    }

    // Target independently inserts a different row at the same primary key, then merging conflicts.
    AppModelDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(target.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::ReadWrite)));
    ASSERT_EQ(BE_SQLITE_OK, db.ExecuteSql("INSERT INTO Dgns(ID,Name,IsDgnLib) VALUES(1,'from-target',0)"));
    db.SaveChanges();

    EXPECT_NE(BE_SQLITE_OK, db.GetTxns().MergeChangeset(props)) << "a primary-key conflict must abort the merge";
    EXPECT_TRUE(db.GetTxns().GetParentChangesetId().empty()) << "an aborted merge must not advance the parent";

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "SELECT Name FROM Dgns WHERE ID=1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_STREQ("from-target", stmt.GetValueText(0)) << "the target's own data must be preserved after rollback";
    db.CloseDb();
    }

