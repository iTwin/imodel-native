/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ECObjects/ECExpressions.h>
#include <BeSQLite/Profiler.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ExecutionPlanTests : ECDbTestFixture
    {
    public:
      //---------------------------------------------------------------------------------------
      // @bsimethod
      // Run sql against profile db and store execution plan as text
      //+---------------+---------------+---------------+---------------+---------------+------
      BeFileName SaveExecutionPlan(BeFileName profileDbFile) 
        {
        Db profileDb;
        if (profileDb.OpenBeSQLiteDb(profileDbFile, Db::OpenParams(Db::OpenMode::Readonly)) != BE_SQLITE_OK)
          return BeFileName("");
        auto stats = profileDb.GetCachedStatement("SELECT sql, plan FROM sql_list");
        FILE* snapshotFile = nullptr;
        BeFileName snapshotFullPath;
        BeTest::GetHost().GetOutputRoot(snapshotFullPath);
        BeFileName snapshotFileName = profileDbFile.GetBaseName();
        snapshotFileName.AppendString(L"-snapshot.txt");
        snapshotFullPath.AppendToPath(snapshotFileName);

        BeFile::Fopen(&snapshotFile, snapshotFullPath.GetNameUtf8().c_str(), "a+");
        
        while (BE_SQLITE_ROW == stats->Step()) 
            {
            auto plan = stats->GetValueText(1);
            fprintf(snapshotFile, "%s", plan);
            }
        fclose(snapshotFile);
        return snapshotFullPath;
        }

      //---------------------------------------------------------------------------------------
      // @bsimethod
      // Compare two snapshots as simple string comparison
      //+---------------+---------------+---------------+---------------+---------------+------
      bool CompareSnapshots(BeFileName baseSnapshot, BeFileName currSnapshot) 
        {
          Utf8String baseString, currString;
          TestUtilities::ReadFile(baseString, baseSnapshot);
          TestUtilities::ReadFile(currString, currSnapshot);
          if (baseString == currString)
            return true;
          else
            return false;
        }
    };

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ExecutionPlanTests, CompareSnapshots)
    {
    auto dbName = L"bc.bim";
    BeFileName dbFullPath;
    BeTest::GetHost().GetDocumentsRoot(dbFullPath);
    dbFullPath.AppendToPath(L"ECDb");
    dbFullPath.AppendToPath(L"ExecutionPlan");
    BeFileName jsonPath = dbFullPath;
    dbFullPath.AppendToPath(dbName);
    jsonPath.AppendToPath(L"dataset.json");

    // Read queries to run from json file
    Json::Value jsonInput;
    ASSERT_EQ(SUCCESS, TestUtilities::ReadFile(jsonInput, jsonPath));
    Json::Value queries = jsonInput.get("queries", "");
    for (size_t i = 0; i < queries.size(); i++) {
      Json::Value query = queries.get(Json::ArrayIndex(i), "");
      for (auto const& queryName : query.getMemberNames()) {
        Json::Value ecSqlVal = query.get(queryName, "");
        Utf8String testDbName;
        testDbName.Sprintf("%ls_%s.ecdb", dbName, queryName.c_str());
        EXPECT_EQ(BE_SQLITE_OK, CloneECDb(m_ecdb, testDbName.c_str(), dbFullPath, ECDb::OpenParams(Db::OpenMode::Readonly)));

        EXPECT_EQ(BE_SQLITE_OK, Profiler::InitScope(m_ecdb, "plan scope", "plan", Profiler::Params()));
        auto scope = Profiler::GetScope(m_ecdb);
        EXPECT_EQ(BE_SQLITE_OK, scope->Start());

        ECSqlStatement stmt;
        // run query to get Execution plan
        stmt.Prepare(m_ecdb, ecSqlVal.asCString());
        while (BE_SQLITE_ROW == stmt.Step()) 
          {
          stmt.Step();
          }

        ASSERT_EQ(BE_SQLITE_OK, scope->Stop());

        // save execution plan as a text file
        BeFileName currSnapshotPath = SaveExecutionPlan(scope->GetProfileDbFileName());
        auto snapshotName = currSnapshotPath.GetBaseName();
        stmt.Finalize();
        m_ecdb.CloseDb();

        BeFileName baseSnapshotPath = dbFullPath.GetDirectoryName();
        baseSnapshotPath.AppendToPath(snapshotName);

        // compare if base and current snapshot text is same
        EXPECT_TRUE(CompareSnapshots(baseSnapshotPath, currSnapshotPath)) << "Snapshot files are not matching for: " << snapshotName.GetName();

      }
    }
  }

END_ECDBUNITTESTS_NAMESPACE