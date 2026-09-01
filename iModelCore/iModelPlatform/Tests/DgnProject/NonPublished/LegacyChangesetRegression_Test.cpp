/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// Backward-compatibility regression harness for applying legacy changesets with Schema Sync disabled.
//
// These tests are environment-gated and are skipped unless IMODEL_REGRESSION_DIR is set.
// The directory is expected to contain one subfolder per iModel (named by iModel id/GUID),
// plus an imodel-store.ecdb that maps iModels to their ordered changesets:
//
//      <IMODEL_REGRESSION_DIR>/
//          imodel-store.ecdb           (contains table hub_imodel_changeset: IModelId, ChangesetId, Index)
//          <imodelId>/                 (e.g. 1b641c41-b314-4519-a493-7a9aae74c672)
//              seed.bim                (exactly one .bim file - the seed/first version)
//              changesets/
//                  <changesetId>.cs    (e.g. 0a6ebd864fd3ad8ee0645d133973cead05559785.cs)
//                  ...
//
// The harness replays all changesets from seed to tip and records per-changeset artifacts
// (integrity_check, foreign_key_check, normalized sqlite_master DDL, table row counts) into
// the test output directory.

#include "DgnHandlersTests.h"
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <DgnPlatform/TxnManager.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeFile.h>
#include <cstdlib>
#include "iostream"

#if defined (_MSC_VER)
#pragma warning (disable:4996)
#endif

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE

// If set to a specific iModel id
// Leave empty to test every folder in the regression directory.
static Utf8CP const s_onlyIModelId = "";

//=======================================================================================
// @bsiclass
//=======================================================================================
struct LegacyChangesetRegression : ::testing::Test
    {
    ScopedDgnHost m_host;

    //! One legacy iModel dataset entry: a seed .bim plus its ordered changeset files.
    struct Dataset
        {
        Utf8String m_name;                    // iModel id (folder name)
        BeFileName m_seedFile;
        bvector<BeFileName> m_changesetFiles; // ordered by hub [Index]; file names are <changesetId>.cs
        bvector<Utf8String> m_changesetIds;   // hub changeset ids, parallel to m_changesetFiles
        };

    static BeFileName GetRegressionDir()
        {
        char const* dir = std::getenv("IMODEL_REGRESSION_DIR");
        if (nullptr == dir || '\0' == *dir)
            return BeFileName();
        return BeFileName(dir, BentleyCharEncoding::Utf8);
        }

    //! Discover all datasets under the regression dir, using imodel-store.ecdb for changeset ordering.
    static bvector<Dataset> DiscoverDatasets(BeFileNameCR regressionDir)
        {
        bvector<Dataset> datasets;

        // Open the store that maps iModels to their ordered changesets.
        BeFileName storeFile(regressionDir);
        storeFile.AppendToPath(L"imodel-store.ecdb");
        Db store;
        if (BE_SQLITE_OK != store.OpenBeSQLiteDb(storeFile, Db::OpenParams(Db::OpenMode::Readonly)))
            {
            ADD_FAILURE() << "Could not open " << storeFile.GetNameUtf8().c_str();
            return datasets;
            }

        BeFileName entry;
        bool isDir = false;
        for (BeDirectoryIterator it(regressionDir); it.GetCurrentEntry(entry, isDir) == SUCCESS; it.ToNext())
            {
            if (!isDir)
                continue;

            Dataset dataset;
            dataset.m_name = Utf8String(entry.GetFileNameWithoutExtension().c_str());

            // If a specific iModel id is hardcoded, skip all others.
            if ('\0' != *s_onlyIModelId && !dataset.m_name.EqualsI(s_onlyIModelId))
                continue;

            // Find the single .bim seed file
            BeFileName child;
            bool childIsDir = false;
            for (BeDirectoryIterator cit(entry); cit.GetCurrentEntry(child, childIsDir) == SUCCESS; cit.ToNext())
                {
                if (!childIsDir && child.GetExtension().EqualsI(L"bim"))
                    {
                    dataset.m_seedFile = child;
                    break;
                    }
                }

            if (dataset.m_seedFile.IsEmpty())
                continue; // not a dataset folder

            // Query the store for this iModel's changesets in order, and resolve <changesetId>.cs files.
            BeFileName changesetDir(entry);
            changesetDir.AppendToPath(L"changesets");

            Statement stmt;
            if (BE_SQLITE_OK != stmt.Prepare(store,
                "SELECT [ChangesetId], [Index] FROM [hub_imodel_changeset] WHERE [IModelId] = ? ORDER BY [Index]"))
                {
                ADD_FAILURE() << "Could not prepare hub_imodel_changeset query against imodel-store.ecdb";
                return datasets;
                }
            stmt.BindText(1, dataset.m_name.c_str(), Statement::MakeCopy::No);

            bool complete = true;
            while (BE_SQLITE_ROW == stmt.Step())
                {
                Utf8String changesetId = stmt.GetValueText(0);
                BeFileName csFile(changesetDir);
                WString csName(changesetId.c_str(), BentleyCharEncoding::Utf8);
                csName.append(L".cs");
                csFile.AppendToPath(csName.c_str());
                if (!csFile.DoesPathExist())
                    {
                    ADD_FAILURE() << dataset.m_name << ": missing changeset file for id " << changesetId.c_str()
                                  << " (index " << stmt.GetValueInt(1) << "): " << csFile.GetNameUtf8().c_str();
                    complete = false;
                    break;
                    }
                dataset.m_changesetFiles.push_back(csFile);
                dataset.m_changesetIds.push_back(changesetId);
                }

            if (complete && !dataset.m_changesetFiles.empty())
                datasets.push_back(dataset);
            }
        return datasets;
        }

    //! Root output dir for this run's artifacts.
    static BeFileName GetArtifactRoot()
        {
        BeFileName root;
        BeTest::GetHost().GetOutputRoot(root);
        root.AppendToPath(L"LegacyChangesetRegression");
        return root;
        }

    //! Copy the seed into the output area so the dataset is never mutated.
    static BeFileName MakeWorkingCopy(Dataset const& dataset, WCharCP suffix)
        {
        BeFileName workDir = GetArtifactRoot();
        workDir.AppendToPath(WString(dataset.m_name.c_str(), BentleyCharEncoding::Utf8).c_str());
        BeFileName::CreateNewDirectory(workDir.GetName());

        BeFileName workFile(workDir);
        WString fileName;
        fileName.Sprintf(L"briefcase_%ls.bim", suffix);
        workFile.AppendToPath(fileName.c_str());
        if (workFile.DoesPathExist())
            workFile.BeDeleteFile();
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(dataset.m_seedFile, workFile));
        return workFile;
        }

    static DgnDbPtr OpenBriefcase(BeFileNameCR fileName)
        {
        DbResult status;
        DgnDbPtr db = DgnDb::OpenIModelDb(&status, fileName,
            DgnDb::OpenParams(Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes,
                SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck)));
        EXPECT_EQ(BE_SQLITE_OK, status) << "Failed to open " << fileName.GetNameUtf8().c_str();
        if (!db.IsValid())
            return db;

        if (!db->GetBriefcaseId().IsBriefcase())
            {
            EXPECT_EQ(BE_SQLITE_OK, db->ResetBriefcaseId(BeBriefcaseId(2)));
            EXPECT_EQ(BE_SQLITE_OK, db->SaveChanges());
            }
        return db;
        }

    //! Sync-db file used for a working briefcase in the schema-sync experiment mode.
    static BeFileName SyncDbFileFor(BeFileNameCR briefcaseFile)
        {
        BeFileName syncFile(briefcaseFile);
        syncFile.AppendString(L".syncdb.ecdb");
        return syncFile;
        }

    //! Build ChangesetProps for the given file, chained onto the briefcase's current parent id.
    static ChangesetPropsPtr MakeChangesetProps(DgnDbR db, BeFileNameCR csFile, Utf8StringCR expectedId, int32_t index)
        {
        Utf8String parentId = db.Txns().GetParentChangesetId();
        Utf8String id = ChangesetProps::ComputeChangesetId(parentId, csFile, Napi::Env(nullptr));
        EXPECT_STREQ(expectedId.c_str(), id.c_str()) << "Computed changeset id does not match hub id for " << csFile.GetNameUtf8().c_str();
        Utf8String dbGuid = db.GetDbGuid().ToString();

        // First construct as Regular so we can probe the file for DDL/schema changes,
        // then reconstruct with the correct type (Schema is required for ECDb cache invalidation).
        ChangesetPropsPtr probe = new ChangesetProps(id, index, parentId, dbGuid, csFile, ChangesetProps::ChangesetType::Regular);
        bool isSchema = probe->ContainsDdlChanges(db); // reads the schema-changes flag from the changeset file itself
        if (!isSchema)
            return probe;
        return new ChangesetProps(id, index, parentId, dbGuid, csFile, ChangesetProps::ChangesetType::Schema);
        }

    //! Run a pragma/query and append all result rows to `out`.
    static void AppendQueryRows(Utf8StringR out, DgnDbR db, Utf8CP sql)
        {
        Statement stmt;
        DbResult rc = stmt.Prepare(db, sql);
        if (BE_SQLITE_OK != rc)
            {
            out.append(Utf8PrintfString("Prepare failed (%d): %s\n", static_cast<int>(rc), sql));
            return;
            }
        while (BE_SQLITE_ROW == stmt.Step())
            {
            for (int i = 0; i < stmt.GetColumnCount(); ++i)
                {
                if (i > 0)
                    out.append("|");
                Utf8CP val = stmt.GetValueText(i);
                out.append(nullptr != val ? val : "<null>");
                }
            out.append("\n");
            }
        }

    //! Record the full state artifact for the briefcase after applying changeset `index`.
    static Utf8String RecordArtifact(DgnDbR db, Dataset const& dataset, Utf8CP phase, int32_t index)
        {
        Utf8String art;

        art.append("== integrity_check ==\n");
        AppendQueryRows(art, db, "PRAGMA integrity_check");

        art.append("== foreign_key_check ==\n");
        AppendQueryRows(art, db, "PRAGMA foreign_key_check");

        // Normalized DDL: all tables/indexes/triggers/views. This is the authoring-vs-receiving
        // DDL parity check - a receiving briefcase must end up with identical sqlite_master DDL.
        art.append("== sqlite_master ==\n");
        AppendQueryRows(art, db,
            "SELECT type, name, tbl_name, coalesce(replace(replace(trim(sql), char(13), ''), char(10), ' '), '<null>')"
            " FROM sqlite_master WHERE name NOT LIKE 'sqlite_%' ORDER BY type, name");

        // EC metadata shape (schema/class/property/table/column counts + mapping fingerprint).
        art.append("== ec_counts ==\n");
        AppendQueryRows(art, db,
            "SELECT 'schemas', count(*) FROM ec_Schema UNION ALL"
            " SELECT 'classes', count(*) FROM ec_Class UNION ALL"
            " SELECT 'properties', count(*) FROM ec_Property UNION ALL"
            " SELECT 'tables', count(*) FROM ec_Table UNION ALL"
            " SELECT 'columns', count(*) FROM ec_Column UNION ALL"
            " SELECT 'propertymaps', count(*) FROM ec_PropertyMap");

        art.append("== ec_table_map ==\n");
        AppendQueryRows(art, db,
            "SELECT t.Name, t.Type, count(c.Id) FROM ec_Table t LEFT JOIN ec_Column c ON c.TableId=t.Id"
            " GROUP BY t.Id ORDER BY t.Name");

        // Content fingerprint of the EC cache tables. The schemasync branch derives FK
        art.append("== ec_cache_fingerprint ==\n");
        AppendQueryRows(art, db,
            "SELECT 'ClassHierarchy', count(*), coalesce(sum(Id),0), coalesce(sum(ClassId),0),"
            " coalesce(sum(BaseClassId),0), coalesce(min(Id),0), coalesce(max(Id),0)"
            " FROM ec_cache_ClassHierarchy");
        AppendQueryRows(art, db,
            "SELECT 'ClassHasTables', count(*), coalesce(sum(Id),0), coalesce(sum(ClassId),0),"
            " coalesce(sum(TableId),0), coalesce(min(Id),0), coalesce(max(Id),0)"
            " FROM ec_cache_ClassHasTables");

        // Trigger inventory
        art.append("== triggers ==\n");
        AppendQueryRows(art, db,
            "SELECT name, tbl_name FROM sqlite_master WHERE type='trigger' ORDER BY name");

        // Row counts for every real table (data parity).
        art.append("== row_counts ==\n");
            {
            Statement tables;
            tables.Prepare(db, "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name");
            while (BE_SQLITE_ROW == tables.Step())
                {
                Utf8String tableName = tables.GetValueText(0);
                Utf8PrintfString countSql("SELECT '%s', count(*) FROM [%s]", tableName.c_str(), tableName.c_str());
                AppendQueryRows(art, db, countSql.c_str());
                }
            }

        // Write to disk for cross-build diffing.
        BeFileName artDir = GetArtifactRoot();
        artDir.AppendToPath(WString(dataset.m_name.c_str(), BentleyCharEncoding::Utf8).c_str());
        BeFileName::CreateNewDirectory(artDir.GetName());
        BeFileName artFile(artDir);
        WString artName;
        artName.Sprintf(L"%ls_%04d.txt", WString(phase, BentleyCharEncoding::Utf8).c_str(), index);
        artFile.AppendToPath(artName.c_str());

        BeFile file;
        if (BeFileStatus::Success == file.Create(artFile.GetName(), true))
            {
            uint32_t bytesWritten = 0;
            file.Write(&bytesWritten, art.c_str(), (uint32_t)art.size());
            file.Close();
            }

        return art;
        }

    //! Assert the health sections of an artifact are clean.
    static void ExpectHealthy(Utf8StringCR artifact, Dataset const& dataset, int32_t index)
        {
        EXPECT_NE(Utf8String::npos, artifact.find("== integrity_check ==\nok\n")) << dataset.m_name << ": integrity_check failed after changeset " << index;

        size_t fkStart = artifact.find("== foreign_key_check ==\n");
        size_t fkEnd = artifact.find("== sqlite_master ==");
        ASSERT_NE(Utf8String::npos, fkStart);
        ASSERT_NE(Utf8String::npos, fkEnd);
        Utf8String fkSection = artifact.substr(fkStart + strlen("== foreign_key_check ==\n"), fkEnd - fkStart - strlen("== foreign_key_check ==\n"));
        fkSection.Trim();
        EXPECT_TRUE(fkSection.empty()) << dataset.m_name << ": foreign_key_check violations after changeset " << index << ":\n" << fkSection.c_str();
        }

    //! Apply one changeset file (1-based index into the dataset) to the briefcase, verifying success.
    static void ApplyChangeset(DgnDbR db, Dataset const& dataset, int32_t index)
        {
        ChangesetPropsPtr props = MakeChangesetProps(db, dataset.m_changesetFiles[index - 1], dataset.m_changesetIds[index - 1], index);
        ASSERT_TRUE(props.IsValid());
        ASSERT_EQ(ChangesetStatus::Success, db.Txns().PullMergeApply(*props)) << dataset.m_name << ": failed to apply changeset " << index;

        ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges());
        ASSERT_STREQ(props->GetChangesetId().c_str(), db.Txns().GetParentChangesetId().c_str()) << dataset.m_name << ": parent changeset id not advanced after changeset " << index;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LegacyChangesetRegression, SeedToTipReplay)
    {
    BeFileName regressionDir = GetRegressionDir();
    if (regressionDir.IsEmpty() || !regressionDir.DoesPathExist())
        GTEST_SKIP() << "Set IMODEL_REGRESSION_DIR to a legacy iModel dataset directory to run this test.";

    bvector<Dataset> datasets = DiscoverDatasets(regressionDir);
    ASSERT_FALSE(datasets.empty()) << "No datasets found under " << regressionDir.GetNameUtf8().c_str();

    for (Dataset const& dataset : datasets)
        {
        SCOPED_TRACE(dataset.m_name.c_str());

        BeFileName workFile = MakeWorkingCopy(dataset, L"replay");
        DgnDbPtr db = OpenBriefcase(workFile);
        ASSERT_TRUE(db.IsValid());

        // Baseline artifact for the seed itself (index 0).
        Utf8String seedArtifact = RecordArtifact(*db, dataset, "replay", 0);
        ExpectHealthy(seedArtifact, dataset, 0);

        for (int32_t i = 1; i <= static_cast<int32_t>(dataset.m_changesetFiles.size()); ++i)
            {
            ApplyChangeset(*db, dataset, i);
            if (::testing::Test::HasFatalFailure())
                return;
            Utf8String artifact = RecordArtifact(*db, dataset, "replay", i);
            ExpectHealthy(artifact, dataset, i);

            std::cout << "Changeset index: " << i << " processed" << std::endl;
            }

        db->CloseDb();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LegacyChangesetRegression, ResumeAtEveryBoundary)
    {
    BeFileName regressionDir = GetRegressionDir();
    if (regressionDir.IsEmpty() || !regressionDir.DoesPathExist())
        GTEST_SKIP() << "Set IMODEL_REGRESSION_DIR to a legacy iModel dataset directory to run this test.";

    bvector<Dataset> datasets = DiscoverDatasets(regressionDir);
    ASSERT_FALSE(datasets.empty()) << "No datasets found under " << regressionDir.GetNameUtf8().c_str();

    for (Dataset const& dataset : datasets)
        {
        SCOPED_TRACE(dataset.m_name.c_str());

        // Reference run: single session, seed to tip.
        Utf8String referenceTip;
            {
            BeFileName refFile = MakeWorkingCopy(dataset, L"resume_ref");
            DgnDbPtr db = OpenBriefcase(refFile);
            ASSERT_TRUE(db.IsValid());
            for (int32_t i = 1; i <= static_cast<int32_t>(dataset.m_changesetFiles.size()); ++i)
                {
                ApplyChangeset(*db, dataset, i);
                if (::testing::Test::HasFatalFailure())
                    return;
                std::cout << "Changeset index: " << i << " processed" << std::endl;
                }
            referenceTip = RecordArtifact(*db, dataset, "resume_ref_tip", static_cast<int32_t>(dataset.m_changesetFiles.size()));
            ExpectHealthy(referenceTip, dataset, static_cast<int32_t>(dataset.m_changesetFiles.size()));
            db->CloseDb();
            }

        // Resumed run: close/reopen at every changeset boundary.
        BeFileName workFile = MakeWorkingCopy(dataset, L"resume");
        for (int32_t i = 1; i <= static_cast<int32_t>(dataset.m_changesetFiles.size()); ++i)
            {
            DgnDbPtr db = OpenBriefcase(workFile);
            ASSERT_TRUE(db.IsValid());
            ApplyChangeset(*db, dataset, i);
            if (::testing::Test::HasFatalFailure())
                return;
            db->CloseDb(); // "interruption": everything torn down, caches discarded
            db = nullptr;
            }

        // Final verification: reopen and compare against the single-session reference.
        DgnDbPtr db = OpenBriefcase(workFile);
        ASSERT_TRUE(db.IsValid());
        Utf8String resumedTip = RecordArtifact(*db, dataset, "resume_tip", static_cast<int32_t>(dataset.m_changesetFiles.size()));
        ExpectHealthy(resumedTip, dataset, static_cast<int32_t>(dataset.m_changesetFiles.size()));
        EXPECT_EQ(referenceTip, resumedTip) << dataset.m_name << ": resumed pull produced a different end state than single-session replay. Diff the resume_ref_tip_*.txt and resume_tip_*.txt artifacts.";
        db->CloseDb();
        }
    }
