/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceBeSQLiteDb_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//#ifdef WIP_DOES_NOT_BUILD_ON_ANDROID_OR_IOS

#include "BeSQLitePerformanceTests.h"
#include <PerformanceTestingHelper/PerformanceTestingHelpers.h>
#ifdef WIP_PUBLISHED_API
#include <BeSQLite/SQLiteAPI.h> //only needed for test to directly work with SQLite
#endif

#include <vector>

struct PerformanceBeSQLiteDbTests : public PerformanceTestFixtureBase
{

public:


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
void IncrementRepositoryLocalValue
(
Db& db,
size_t repositoryLocalKeyIndex
)
    {
    int64_t newValue = 0LL;
    /*auto stat = */db.GetRLVCache().IncrementValue (newValue, repositoryLocalKeyIndex);
    //ASSERT_EQ (BE_SQLITE_OK, stat) << L"IncrementRepositoryLocalValueInt64 failed.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
void IncrementRepositoryLocalValueWithSaveChanges
(
Db& db,
size_t repositoryLocalKeyIndex
)
    {
    IncrementRepositoryLocalValue (db, repositoryLocalKeyIndex);
    DbResult stat = db.SaveChanges ();
    ASSERT_EQ (BE_SQLITE_OK, stat) << L"SaveChanges failed.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
void IncrementRepositoryLocalValueWithinSavepoint
(
Db& db,
size_t repositoryLocalKeyIndex
)
    {
    Savepoint savepoint (db, "IncrementRepositoryLocalValue");
    IncrementRepositoryLocalValue (db, repositoryLocalKeyIndex);
    DbResult stat = savepoint.Commit ();
    ASSERT_EQ (BE_SQLITE_OK, stat) << L"Committing savepoint failed.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
void SetupIncrementRepositoryLocalValueTestDgnDb
(
Db& testDb,
Utf8CP repositoryLocalKey
)
    {
    Utf8String dbPath;

    //create test DgnDb file with a mock sequence being set up
        {
        Db db;
        DbResult stat = SetupDb (db, L"repositorylocalvalueperformance.ecdb");
        dbPath.assign (db.GetDbFileName ());

        size_t repositoryLocalKeyIndex = 0;
        ASSERT_EQ (BE_SQLITE_OK, db.GetRLVCache().Register(repositoryLocalKeyIndex, repositoryLocalKey));
        const int64_t zero = 0LL;
        stat = db.GetRLVCache().SaveValue(repositoryLocalKeyIndex, zero);
        ASSERT_EQ (BE_SQLITE_OK, stat) << L"Saving initial value of repository local value failed";
        ASSERT_EQ (BE_SQLITE_OK, db.SaveChanges ()) << L"Committing repository local values failed.";
        }

    //reopen test db
    DbResult stat = testDb.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes));
    ASSERT_EQ (BE_SQLITE_OK, stat) << L"Reopening test DgnDb '" << dbPath.c_str () << L"' failed.";
    }

}; //end of Test Fixture

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceBeSQLiteDbTests, IncrementRepositoryLocalValueNoTransaction)
    {
    const int64_t iterationCount = 10000000LL;

    Utf8CP const mockSequenceKey = "mocksequence";
    Db testDb;
    SetupIncrementRepositoryLocalValueTestDgnDb ( testDb, mockSequenceKey);

    size_t mockSequenceKeyIndex = 0;
    ASSERT_EQ (BE_SQLITE_OK, testDb.GetRLVCache().Register(mockSequenceKeyIndex, mockSequenceKey));
    //printf ("Attach to profiler and press any key...\r\n"); getchar ();
    m_stopwatch.Start();
    for (int64_t i = 0LL; i < iterationCount; i++)
        {
        IncrementRepositoryLocalValue (testDb, mockSequenceKeyIndex);
        }

    const double totalSecs = m_stopwatch.GetCurrentSeconds ();
    //printf ("Detach to profiler and press any key...\r\n"); getchar ();
    testDb.SaveChanges ();
    m_stopwatch.Stop ();

    LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds(), "Increment RLV");
    PERFORMANCELOG.infov ("Incrementing repository local value %d times took: %.4f msecs. %.4f msecs with saving outermost transaction at end.", 
                        iterationCount, 
                        totalSecs * 1000.0,
                        m_stopwatch.GetElapsedSeconds () * 1000.0);

    //timing an in-memory sequence as base line
    struct CacheValue
        {
    private:
        bool m_isunset;
        bool m_isdirty;
        int64_t m_value;

    public:
        CacheValue ()
            : m_value (0LL)
            {}

        int64_t Increment ()
            {
            m_value++;
            return m_value;
            }

        int64_t GetValue () const { return m_value; }
        };

    std::vector<std::unique_ptr<CacheValue>> sequenceVector;
    sequenceVector.push_back (std::move (std::unique_ptr<CacheValue> (new CacheValue ())));
    sequenceVector.push_back (std::move (std::unique_ptr<CacheValue> (new CacheValue ())));
    sequenceVector.push_back (std::move (std::unique_ptr<CacheValue> (new CacheValue ())));

    std::vector<Utf8String> keyVector = { "sequence_1", "sequence_2", "sequence_3" };

    bmap<Utf8CP, CacheValue> sequenceMap;
    for (auto const& key : keyVector)
        {
        sequenceMap[key.c_str ()] = CacheValue ();
        }

    m_stopwatch.Start ();
    for (int64_t i = 0LL; i < iterationCount; i++)
        {
        sequenceVector[2]->Increment ();
        }
    m_stopwatch.Stop ();
    ASSERT_EQ (iterationCount, sequenceVector[2]->GetValue ());
    LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds(), "Increment RLV with vector lookup");
    PERFORMANCELOG.infov ("Incrementing simple in-memory sequence with vector lookup %d times took: %.4f msecs.", iterationCount,
        m_stopwatch.GetElapsedSeconds () * 1000.0);

    m_stopwatch.Start ();
    for (int64_t i = 0LL; i < iterationCount; i++)
        {
        auto it = sequenceMap.find (keyVector[2].c_str ());
        if (it != sequenceMap.end ())
            it->second.Increment ();
        }
    m_stopwatch.Stop ();
    ASSERT_EQ (iterationCount, sequenceMap[keyVector[2].c_str ()].GetValue ());
    LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds(), "Increment RLV with bmap lookup");
    PERFORMANCELOG.infov ("Incrementing simple in-memory sequence with bmap lookup %d times took: %.4f msecs.", iterationCount,
        m_stopwatch.GetElapsedSeconds () * 1000.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceBeSQLiteDbTests, IncrementRepositoryLocalValueWithoutInMemoryCache)
    {
    const int iterationCount = 1000000;

    Utf8CP const mockSequenceKey = "mocksequence";
    Db testDb;
    SetupIncrementRepositoryLocalValueTestDgnDb (testDb, mockSequenceKey);

    size_t mockSequenceKeyIndex = 0;
    ASSERT_EQ (BE_SQLITE_OK, testDb.GetRLVCache().Register(mockSequenceKeyIndex, mockSequenceKey));

    Statement queryStmt;
    DbResult stat = queryStmt.Prepare (testDb, SqlPrintfString ("select Val from " BEDB_TABLE_Local " where Name='%s'", mockSequenceKey));
    ASSERT_EQ (BE_SQLITE_OK, stat) << L"Preparing query repo local value SQL failed.";
    Statement updateStmt;
    stat = updateStmt.Prepare (testDb, SqlPrintfString ("update " BEDB_TABLE_Local " set Val=?1 where Name='%s'", mockSequenceKey));
    ASSERT_EQ (BE_SQLITE_OK, stat) << L"Preparing update repo local value SQL failed.";

    m_stopwatch.Start();
    for (int i = 0; i < iterationCount; i++)
        {
        //query last value
        queryStmt.Reset ();
        EXPECT_EQ (BE_SQLITE_ROW, queryStmt.Step ()) << L"Executing query SQL didn't return the expected row";
        const void* blob = queryStmt.GetValueBlob (0);
        int blobSize = queryStmt.GetColumnBytes (0);
        int64_t lastValue = -1LL;
        memcpy ((Byte*) &lastValue, blob, blobSize);
        EXPECT_EQ (static_cast<int64_t> (i), lastValue) << L"Retrieved repository local value is wrong.";

        lastValue++;

        //now save incremented value back
        updateStmt.Reset ();
        updateStmt.ClearBindings ();
        EXPECT_EQ (BE_SQLITE_OK, updateStmt.BindBlob (1, &lastValue, sizeof(lastValue), Statement::MakeCopy::No)) << L"Binding blob to update statement failed";
        EXPECT_EQ (BE_SQLITE_DONE, updateStmt.Step ()) << L"Executing update SQL failed";
        }

    const double totalSecs = m_stopwatch.GetCurrentSeconds ();
    testDb.SaveChanges ();
    m_stopwatch.Stop ();

    LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds());
    PERFORMANCELOG.infov ("Incrementing repository local value %d times took: %.4f msecs. %.4f msecs with saving outermost transaction at end.", 
        iterationCount, 
        totalSecs * 1000.0,
        m_stopwatch.GetElapsedSeconds () * 1000.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceBeSQLiteDbTests, IncrementRepositoryLocalValueWrappedInSavepoint)
    {
    const int iterationCount = 1000000;

    Utf8CP const mockSequenceKey = "mocksequence";
    Db testDb;
    SetupIncrementRepositoryLocalValueTestDgnDb (testDb, mockSequenceKey);

    size_t mockSequenceKeyIndex = 0;
    ASSERT_EQ (BE_SQLITE_OK, testDb.GetRLVCache().Register(mockSequenceKeyIndex, mockSequenceKey));

    m_stopwatch.Start();
    for (int i = 0; i < iterationCount; i++)
        {
        IncrementRepositoryLocalValueWithinSavepoint (testDb, mockSequenceKeyIndex);
        }

    const double totalSecs = m_stopwatch.GetCurrentSeconds ();
    testDb.SaveChanges ();
    m_stopwatch.Stop ();

    LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds());
    PERFORMANCELOG.infov ("Incrementing repository local value %d times took: %.4f msecs. %.4f msecs with saving outermost transaction at end.", 
        iterationCount, 
        totalSecs * 1000.0,
        m_stopwatch.GetElapsedSeconds () * 1000.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceBeSQLiteDbTests, IncrementRepositoryLocalValueAndCommittingOutermostTransactionInEachIteration)
    {
    const int iterationCount = 1000;

    Utf8CP const mockSequenceKey = "mocksequence";
    Db testDb;
    SetupIncrementRepositoryLocalValueTestDgnDb (testDb, mockSequenceKey);

    size_t mockSequenceKeyIndex = 0;
    ASSERT_EQ (BE_SQLITE_OK, testDb.GetRLVCache().Register(mockSequenceKeyIndex, mockSequenceKey));

    m_stopwatch.Start();
    for (int i = 0; i < iterationCount; i++)
        {
        IncrementRepositoryLocalValueWithSaveChanges (testDb, mockSequenceKeyIndex);
        }

    const double totalSecs = m_stopwatch.GetCurrentSeconds ();
    testDb.SaveChanges ();
    m_stopwatch.Stop ();
    
    LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds());
    PERFORMANCELOG.infov ("Incrementing repository local value %d times took: %.4f msecs. %.4f msecs with saving outermost transaction at end.", 
        iterationCount, 
        totalSecs * 1000.0,
        m_stopwatch.GetElapsedSeconds () * 1000.0);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceBeSQLiteDbTests, SaveRepositoryLocalValueWithSavepointPerIteration)
    {
    const int iterationCount = 10000;

    Utf8CP const mockSequenceKey = "mocksequence";
    Db testDb;
    SetupIncrementRepositoryLocalValueTestDgnDb (testDb, mockSequenceKey);

    size_t mockSequenceKeyIndex = 0;
    ASSERT_EQ (BE_SQLITE_OK, testDb.GetRLVCache().Register(mockSequenceKeyIndex, mockSequenceKey));

    m_stopwatch.Start();
    for (int i = 0; i < iterationCount; i++)
        {
        Savepoint savepoint (testDb, "", true);
        
        int64_t val = i * 1000LL;
        DbResult stat = testDb.GetRLVCache().SaveValue(mockSequenceKeyIndex, val);
        ASSERT_EQ (BE_SQLITE_OK, stat) << "SaveRepositoryLocalValue failed";
        savepoint.Commit ();
        }

    m_stopwatch.Stop ();
    
    LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds());
    PERFORMANCELOG.infov ("Calling SaveRepositoryLocalValue %d times took: %.4f msecs.", 
        iterationCount, 
        m_stopwatch.GetElapsedSeconds () * 1000.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceBeSQLiteDbTests, InsertOrReplaceBlobVsIntegerWithSavepointPerIteration)
    {
    Utf8String dbPath;
    
    //create test DgnDb file with a mock sequence being set up
        {
        Db db;
        DbResult stat = SetupDb (db, L"repositorylocalvalueperformance.idgndb");
        dbPath.assign (db.GetDbFileName ());

        //mimick be_local table
        stat = db.ExecuteSql ("CREATE TABLE test (name CHAR NOT NULL COLLATE NOCASE UNIQUE, intval integer, blobval blob)");
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Creating test table failed.";
        }

    //reopen test db
    Db dgndb;
    DbResult stat = dgndb.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes));
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Reopening test DgnDb '" << dbPath.c_str () << "' failed.";

    const int iterationCount = 10000;

    //insert ints
        {
        Statement stmt;
        stat = stmt.Prepare (dgndb, "INSERT OR REPLACE INTO test (name, intval) VALUES ('ec_ecinstanceidsequence_int', ?)");
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Preparing insert statement failed";

        //StopWatch stopwatch ("", true);
        m_stopwatch.Start();
        for (int i = 0; i < iterationCount; i++)
            {
            Savepoint savepoint (dgndb, "insertint", true);
            stmt.BindInt64 (1, i * 1000LL);
            stat = stmt.Step ();
            ASSERT_EQ (BE_SQLITE_DONE, stat) << "Executing insert statement failed";
            stmt.Reset ();
            stmt.ClearBindings ();
            savepoint.Commit ();
            }

        m_stopwatch.Stop();
        LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds(), "Insert or Replace Integers");
        PERFORMANCELOG.infov("Inserting or replacing integers %d times took: %.4f msecs.",
            iterationCount, 
            m_stopwatch.GetElapsedSeconds() * 1000.0);
        }

        //insert int as blob
        {
        Statement stmt;
        stat = stmt.Prepare (dgndb, "INSERT OR REPLACE INTO test (name, blobval) VALUES ('ec_ecinstanceidsequence_blob', ?)");
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Preparing insert statement failed";

        //StopWatch stopwatch ("", true);
        m_stopwatch.Start();
        for (int i = 0; i < iterationCount; i++)
            {
            Savepoint savepoint (dgndb, "insertblob", true);
            const int64_t val = i * 1000LL;
            stmt.BindBlob (1, &val, sizeof (val), Statement::MakeCopy::No);
            stat = stmt.Step ();
            ASSERT_EQ (BE_SQLITE_DONE, stat) << "Executing insert statement failed";
            stmt.Reset ();
            stmt.ClearBindings ();
            savepoint.Commit ();
            }

        m_stopwatch.Stop ();
        LOGTODB(TEST_DETAILS, m_stopwatch.GetElapsedSeconds(), "Insert or Replace Integers as Blobs");
        PERFORMANCELOG.infov("Inserting or replacing integers as blobs %d times took: %.4f msecs.",
            iterationCount, 
            m_stopwatch.GetElapsedSeconds() * 1000.0);
        }
    }

//#endif//def WIP_DOES_NOT_BUILD_ON_ANDROID_OR_IOS

