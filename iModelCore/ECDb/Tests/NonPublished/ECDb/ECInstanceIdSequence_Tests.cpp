/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/ECDb/ECInstanceIdSequence_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../PublicApi/NonPublished/ECDb/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

static Utf8CP const ECINSTANCEIDSEQUENCE_BELOCAL_KEY = "ec_ecinstanceidsequence";

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String CreateTestDb
(
Utf8CP dbName,
Utf8CP schemaFullName,
int instanceCount,
BeRepositoryId repoId = BeRepositoryId (0)
)
    {
    WString schemaFileName;
    schemaFileName.AssignUtf8(schemaFullName);
    schemaFileName.append (L".ecschema.xml");
    ECDbTestProject testProject;
    testProject.Create (dbName, schemaFileName.c_str (), instanceCount);
    if (repoId.GetValue () > 0)
        {
        testProject.GetECDb ().ChangeRepositoryId (repoId);
        EXPECT_EQ (BE_SQLITE_OK, testProject.GetECDb ().SaveChanges ()) << L"Changing repository id in test DgnDb failed";
        }
    
    return Utf8String (testProject.GetECDbCR ().GetDbFileName ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
void OpenTestDb
(
ECDbR testDb,
Utf8String dbPath,
Utf8CP schemaFullName,
Db::OpenMode openMode,
ECSchemaCP* schema = nullptr
)
    {
    DbResult stat = testDb.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams(openMode));
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Opening test db failed";

    if (schema != nullptr)
        {
        Utf8String name;
        uint32_t minor, major;
        ECSchema::ParseSchemaFullName (name, minor, major, schemaFullName);
        Utf8String schemaName(name);
        *schema = testDb. Schemas().GetECSchema (schemaName.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
void CreateAndReopenTestDb
(
ECDbR testDb,
Utf8CP dbName,
Utf8CP schemaFullName,
int instanceCount,
Db::OpenMode openMode,
ECSchemaCP* schema = nullptr
)
    {
    Utf8String dbPath = CreateTestDb (dbName, schemaFullName, instanceCount);

    OpenTestDb (testDb, dbPath, schemaFullName, openMode, schema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
void CreateAndReopenTestDb
(
ECDbR testDb,
Utf8CP dbName,
Utf8CP schemaFullName,
int instanceCount,
Db::OpenMode openMode,
BeRepositoryId repoId,
ECSchemaCP* schema = nullptr
)
    {
    Utf8String dbPath = CreateTestDb (dbName, schemaFullName, instanceCount, repoId);

    OpenTestDb (testDb, dbPath, schemaFullName, openMode, schema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
IECInstancePtr InsertInstance
(
ECInstanceKey& instanceKey,
ECDbR ecdb,
ECClassCR ecClass
)
    {
    ECInstanceInserter inserter (ecdb, ecClass);
    POSTCONDITION (inserter.IsValid (), nullptr);

    IECInstancePtr testInstance = ECDbTestProject::CreateArbitraryECInstance (ecClass);

    auto insertStatus = inserter.Insert (instanceKey, *testInstance);
    if (insertStatus != SUCCESS)
        return nullptr;

    if (SUCCESS != ECDbTestUtility::SetECInstanceId (*testInstance, instanceKey.GetECInstanceId ()))
        return nullptr;

    return testInstance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceKey InsertRelation
(
ECDbR ecdb,
ECRelationshipClassCR relationClass,
IECInstancePtr source,
IECInstancePtr target
)
    {
    StandaloneECRelationshipEnablerPtr enabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (relationClass);
    IECRelationshipInstancePtr relation = enabler->CreateRelationshipInstance ();
    relation->SetSource (source.get ());
    relation->SetTarget (target.get ());

    ECInstanceInserter inserter (ecdb, relationClass);
    POSTCONDITION (inserter.IsValid (), ECInstanceKey ());

    ECInstanceKey instanceKey;
    auto insertStatus = inserter.Insert (instanceKey, *relation);
    POSTCONDITION (SUCCESS == insertStatus, ECInstanceKey ());

    return instanceKey;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECInstanceIdSequenceTests, ECInstanceIdSequenceFirstIdTest)
    {
    ECDb ecdb;
    ECSchemaCP schema = nullptr;
    CreateAndReopenTestDb (ecdb, "StartupCompany.ecdb", "StartupCompany.02.00", 0, Db::OpenMode::ReadWrite, &schema);

    const BeRepositoryBasedId expectedFirstId (ecdb.GetRepositoryId (), 1);

    ECInstanceKey actualFirstKey;
    InsertInstance (actualFirstKey, ecdb, *(schema->GetClassCP ("AAA")));

    EXPECT_EQ (expectedFirstId.GetValue (), actualFirstKey.GetECInstanceId ().GetValue ()) << "First ECInstanceId generated per repository id differs from expected.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECInstanceIdSequenceTests, ECInstanceIdSequenceExcessTest)
    {
    Utf8CP schemaFullName = "StartupCompany.02.00";

    Utf8String dbPath;
    //max in this test means max per repository id.
    int64_t maxId = 0LL;
    int64_t twoBelowMaxId = 0LL;
    size_t sequenceIndex = 0;
        {
        dbPath = CreateTestDb ("StartupCompany.ecdb", schemaFullName, 1);
        
        //only open as Db without EC profile, so that ECInstanceId Sequence does not overwrite our manual value on closing the db
        Db db;
        DbResult stat = db.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams(Db::OpenMode::ReadWrite));
        EXPECT_EQ (BE_SQLITE_OK, stat) << "Opening test db failed.";

        ASSERT_EQ (BE_SQLITE_OK, db.GetRLVCache().Register(sequenceIndex, ECINSTANCEIDSEQUENCE_BELOCAL_KEY));
        //artificially set sequence close to maximum
        BeRepositoryId repoId = db.GetRepositoryId ();
        maxId = BeRepositoryBasedId (repoId, std::numeric_limits<uint32_t>::max ()).GetValue ();

        twoBelowMaxId = maxId - 2LL;
        stat = db.GetRLVCache().SaveValue (sequenceIndex, twoBelowMaxId);
        EXPECT_EQ (BE_SQLITE_OK, stat) << "Setting ECInstanceIdSequence to two below per repository max in be_Local failed.";
        db.SaveChanges ();
        }

    ECDb ecdb;
    ECSchemaCP schema = nullptr;
    OpenTestDb (ecdb, dbPath, schemaFullName, Db::OpenMode::ReadWrite, &schema);

    ECClassCP testClass = schema->GetClassCP ("AAA");
    ASSERT_TRUE (testClass != nullptr) << "Test class not found";

    int64_t lastId = -1LL;
    DbResult stat = ecdb.GetRLVCache().QueryValue(lastId, sequenceIndex);
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Retrieving ECInstanceIdSequence state from be_Local failed.";
    ASSERT_EQ (twoBelowMaxId, lastId) << "Retrieved ECInstanceIdSequence state doesn't match what the state that was stored";
    
    ECInstanceKey id;
    InsertInstance (id, ecdb, *testClass);
    ASSERT_EQ (twoBelowMaxId + 1LL, id.GetECInstanceId ().GetValue ());

    InsertInstance (id, ecdb, *testClass);
    ASSERT_EQ (twoBelowMaxId + 2LL, id.GetECInstanceId ().GetValue ());

    //will succeed, but fire an assertion
    BeTest::SetFailOnAssert (false);
        {
        IECInstancePtr instance = InsertInstance (id, ecdb, *testClass);
        ASSERT_TRUE (instance.IsValid ()) << L"Insert instance which generates an ECInstanceId that excesses the per repository id maximum is expected to succeed, with an undefined ECInstanceId";
        }
    BeTest::SetFailOnAssert (true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECInstanceIdSequenceTests, ECInstanceIdSequenceTestWithMaximumRepoId)
    {
    Utf8CP const schemaFullName = "StartupCompany.02.00";

    const int32_t int32Max = std::numeric_limits<int32_t>::max ();
    const uint32_t uint32Max = std::numeric_limits<uint32_t>::max ();
    const BeRepositoryId expectedRepoId (int32Max);
    const int64_t oneBelowMaxIdValue = BeRepositoryBasedId (expectedRepoId, uint32Max - 1).GetValue ();
    Utf8String dbPath;
    size_t sequenceIndex = 0;
        //create the test db and set the sequence to short before total excess
        {
        ECDb db;
        CreateAndReopenTestDb (db, "StartupCompany.ecdb", schemaFullName, 0, Db::OpenMode::ReadWrite, expectedRepoId);
        dbPath = Utf8String (db.GetDbFileName ());

        ASSERT_TRUE (db.GetRLVCache().TryGetIndex(sequenceIndex, ECINSTANCEIDSEQUENCE_BELOCAL_KEY));
        DbResult stat = db.GetRLVCache().SaveValue (sequenceIndex, oneBelowMaxIdValue);
        EXPECT_EQ (BE_SQLITE_OK, stat) << L"Setting ECInstanceIdSequence to three below max in be_Local failed.";
        db.SaveChanges ();
        }

    ECDb db;
    ECSchemaCP schema = nullptr;
    OpenTestDb (db, dbPath, schemaFullName, Db::OpenMode::ReadWrite, &schema);

    ECClassCP testClass = schema->GetClassCP ("AAA");
    EXPECT_TRUE (testClass != nullptr) << "Test class not found";

    //now insert an instance and it should get the last available ECInstanceId
    const int64_t maxId = BeRepositoryBasedId (expectedRepoId, uint32Max).GetValue ();
    ECInstanceKey id;
    InsertInstance (id, db, *testClass);
    EXPECT_EQ (maxId, id.GetECInstanceId ().GetValue ()) << "Inserted instance is expected to have maximally possible ECInstanceId.";

    //try one more insert which will succeed, with an undefined ECInstanceId, but fire an assertion
    BeTest::SetFailOnAssert (false);
        {
        IECInstancePtr instance = InsertInstance (id, db, *testClass);
        EXPECT_TRUE (instance.IsValid ()) << "Insert instance which generates an ECInstanceId that excesses the total maximum is expected to succeed, but with an undefined ECInstanceId";
        }
    BeTest::SetFailOnAssert (true);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECInstanceIdSequenceTests, ECInstanceIdSequenceExistsAcrossSessionTest)
    {
    ECDb ecdb;
    CreateAndReopenTestDb (ecdb, "StartupCompany.ecdb", "StartupCompany.02.00", 10, Db::OpenMode::ReadWrite);
    size_t sequenceIndex = 0;
    ASSERT_TRUE (ecdb.GetRLVCache().TryGetIndex (sequenceIndex, ECINSTANCEIDSEQUENCE_BELOCAL_KEY));

    BeRepositoryId repoId = ecdb.GetRepositoryId ();
    int64_t lastId = -1LL;
    DbResult stat = ecdb.GetRLVCache().QueryValue (lastId, sequenceIndex);
    EXPECT_EQ (BE_SQLITE_OK, stat) << "Querying last id of ECInstanceIdSequence from be_Local failed";

    EXPECT_GT (lastId, 0LL) << "Last id of ECInstanceIdSequence in be_Local must be greater than 0 as instances were already inserted.";

    const BeRepositoryBasedId maxThreshold (repoId.GetNextRepositoryId (), 0);
    EXPECT_LT (lastId, maxThreshold.GetValue ()) << "Last id of ECInstanceIdSequence is expected to be less than maximum threshold which is the 0 id of the next repository id.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECInstanceIdSequenceTests, ECInstanceIdSequenceIncrementationTest)
    {
    ECDb ecdb;
    ECSchemaCP schema = nullptr;
    CreateAndReopenTestDb (ecdb, "StartupCompany.ecdb", "StartupCompany.02.00", 10, Db::OpenMode::ReadWrite, &schema);

    ECInstanceKey id1;
    IECInstancePtr instance1 = InsertInstance (id1, ecdb, *(schema->GetClassCP ("AAA")));
        
    ECInstanceKey id2;
    IECInstancePtr instance2 = InsertInstance (id2, ecdb, *(schema->GetClassCP ("ClassWithPrimitiveProperties")));

    EXPECT_EQ (id1.GetECInstanceId ().GetValue () + 1LL, id2.GetECInstanceId ().GetValue ()) << "Instance id for instance with only primitive properties inserted right after another is expected to be greater by 1.";

    //now insert an instance with one struct
    ECInstanceKey id3;
    IECInstancePtr instance3 = InsertInstance (id3, ecdb, *(schema->GetClassCP ("Foo")));

    EXPECT_EQ (id2.GetECInstanceId ().GetValue () + 1LL, id3.GetECInstanceId ().GetValue ()) << "Instance id for instance with one struct property inserted right after another is expected to be greater by 1.";

    //now insert an instance with arrays
    ECInstanceKey id4;
    IECInstancePtr instance4 = InsertInstance (id4, ecdb, *(schema->GetClassCP ("ClassWithPrimitiveArrayProperties")));

    EXPECT_EQ (id3.GetECInstanceId ().GetValue () + 1LL + 3LL, id4.GetECInstanceId ().GetValue ()) << "Instance id for instance with five array properties inserted right after another is expected to be greater by 1.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECInstanceIdSequenceTests, ECInstanceIdSequenceIncrementationWithOneToManyRelationshipsTest)
    {
    ECDb ecdb;
    ECSchemaCP schema = nullptr;
    CreateAndReopenTestDb (ecdb, "StartupCompany.ecdb", "StartupCompany.02.00", 10, Db::OpenMode::ReadWrite, &schema);

    //insert parent instance
    ECInstanceKey parentId;
    IECInstancePtr parentInstance = InsertInstance (parentId, ecdb, *(schema->GetClassCP ("Company")));

    //insert first child instance
    ECInstanceKey child1Id;
    IECInstancePtr child1Instance = InsertInstance (child1Id, ecdb, *(schema->GetClassCP ("Employee")));
    EXPECT_EQ (parentId.GetECInstanceId ().GetValue () + 4LL, child1Id.GetECInstanceId ().GetValue ()) << "Instance id for child #1 is expected to be greater by 1 than the id of the parent.";

    //insert second child instance
    ECInstanceKey child2Id;
    IECInstancePtr child2Instance = InsertInstance (child2Id, ecdb, *(schema->GetClassCP ("Employee")));
    EXPECT_EQ (parentId.GetECInstanceId ().GetValue () + 8LL, child2Id.GetECInstanceId ().GetValue ()) << "Instance id for child #2 is expected to be greater by 2 than the id of the parent.";

    ECInstanceKey lastId = child2Id;

    //insert relationship between parent and child #1
    ECRelationshipClassCP relClass = schema->GetClassCP ("EmployeeCompany")->GetRelationshipClassCP();
    EXPECT_TRUE (relClass != nullptr);

    ECInstanceKey relId = InsertRelation (ecdb, *relClass, child1Instance, parentInstance);
    EXPECT_EQ (child1Id.GetECInstanceId ().GetValue (), relId.GetECInstanceId ().GetValue ()) << "Instance id of relationship between parent and child #1 is expected to equal child id as this is a one-to-many relationship.";

    //insert relationship between parent and child #2
    relId = InsertRelation (ecdb, *relClass, child2Instance, parentInstance);
    EXPECT_EQ (child2Id.GetECInstanceId ().GetValue (), relId.GetECInstanceId ().GetValue ()) << "Instance id of relationship between parent and child #2 is expected to equal child id as this is a one-to-many relationship.";

    //finally create an ordinary instance to check that the above relationship inserts didn't increment the sequence
    ECInstanceKey id;
    InsertInstance (id, ecdb, *(schema->GetClassCP ("AAA")));
    EXPECT_EQ (lastId.GetECInstanceId ().GetValue () + 4LL, id.GetECInstanceId ().GetValue ()) << "Instance id of instance is expected to be greater by one than the previously inserted one-to-many relationship.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECInstanceIdSequenceTests, ECInstanceIdSequenceIncrementationWithManyToManyRelationshipsTest)
    {
    ECDb ecdb;
    ECSchemaCP schema = nullptr;
    CreateAndReopenTestDb (ecdb, "StartupCompany.ecdb", "StartupCompany.02.00", 10, Db::OpenMode::ReadWrite, &schema);
    
    //*** insert instances to relate ***
    //insert source instance #1
    ECClassCP sourceClass = schema->GetClassCP ("Employee");
    ECInstanceKey id;
    IECInstancePtr source1Instance = InsertInstance (id, ecdb, *sourceClass);
    ECInstanceKey lastId = id;

    //insert source instance #1
    IECInstancePtr source2Instance = InsertInstance (id, ecdb, *sourceClass);
    EXPECT_EQ (lastId.GetECInstanceId ().GetValue () + 4LL, id.GetECInstanceId ().GetValue ()) << "Instance id for source #2 is expected to be greater by 1 than the id of the previously inserted instance.";
    lastId = id;

    //insert target instance #1
    ECClassCP targetClass = schema->GetClassCP ("Hardware");
    IECInstancePtr target1Instance = InsertInstance (id, ecdb, *targetClass);
    EXPECT_EQ (lastId.GetECInstanceId ().GetValue () + 4LL, id.GetECInstanceId ().GetValue ()) << "Instance id for target #1 is expected to be greater by 1 than the id of the previously inserted instance.";
    lastId = id;

    //insert target instance #2
    IECInstancePtr target2Instance = InsertInstance (id, ecdb, *targetClass);
    EXPECT_EQ (lastId.GetECInstanceId ().GetValue () + 1LL, id.GetECInstanceId ().GetValue ()) << "Instance id for target #2 is expected to be greater by 1 than the id of the previously inserted instance.";
    lastId = id;
    
    //*** insert relations ***
    //insert relationship between source #1 and target #1
    ECRelationshipClassCP relClass = schema->GetClassCP ("EmployeeHardware")->GetRelationshipClassCP();
    EXPECT_TRUE (relClass != nullptr);

    ECInstanceKey relId = InsertRelation (ecdb, *relClass, source1Instance, target1Instance);
    EXPECT_EQ (lastId.GetECInstanceId ().GetValue () + 1LL, relId.GetECInstanceId ().GetValue ()) << "Instance id of relationship between source #1 and target #1 is expected to be greater by one than lastly inserted id.";
    lastId = relId;

    //insert relationship between source #1 and target #2
    relId = InsertRelation (ecdb, *relClass, source1Instance, target2Instance);
    EXPECT_EQ (lastId.GetECInstanceId ().GetValue () + 1LL, relId.GetECInstanceId ().GetValue ()) << "Instance id of relationship between source #1 and target #2 is expected to be greater by one than lastly inserted id.";
    lastId = relId;

    //insert relationship between source #2 and target #2
    relId = InsertRelation (ecdb, *relClass, source2Instance, target2Instance);
    EXPECT_EQ (lastId.GetECInstanceId ().GetValue () + 1LL, relId.GetECInstanceId ().GetValue ()) << "Instance id of relationship between source #2 and target #2 is expected to be greater by one than lastly inserted id.";
    lastId = relId;

    //finally create an ordinary instance to check that the above relationship inserts didn't increment the sequence
    InsertInstance (id, ecdb, *(schema->GetClassCP ("AAA")));
    EXPECT_EQ (lastId.GetECInstanceId ().GetValue () + 1LL, id.GetECInstanceId ().GetValue ()) << "Instance id of instance is expected to be greater by one than the previously inserted many-to-many relationship.";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECInstanceIdSequenceTests, ChangeRepositoryIdTest)
    {
    ECDb ecdb;
    ECSchemaCP schema = nullptr;
    CreateAndReopenTestDb (ecdb, "StartupCompany.ecdb", "StartupCompany.02.00", 10, Db::OpenMode::ReadWrite, &schema);

    size_t sequenceIndex = 0;
    ASSERT_TRUE (ecdb.GetRLVCache().TryGetIndex(sequenceIndex, ECINSTANCEIDSEQUENCE_BELOCAL_KEY));
    ECClassCR testClass = *(schema->GetClassCP ("AAA"));

    //insert instance before repo id change
    ECInstanceKey idBeforeRepoIdChange;
    IECInstancePtr instance = InsertInstance (idBeforeRepoIdChange, ecdb, testClass);

    //change repo id.
    const BeRepositoryId expectedRepoId (213);
    DbResult stat = ecdb.ChangeRepositoryId (expectedRepoId);
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Changing the repository id failed unexpectedly.";

    int64_t sequenceLastValue = -1LL;
    stat = ecdb.GetRLVCache().QueryValue(sequenceLastValue, sequenceIndex);
    EXPECT_EQ (BE_SQLITE_OK, stat) << "Failed to retrieve last ECInstanceIdSequence value from be_local";

    const BeRepositoryBasedId expectedSequenceLastValueAfterReset (expectedRepoId, 0);
    EXPECT_EQ (expectedSequenceLastValueAfterReset.GetValue (), sequenceLastValue) << "ECInstanceIdSequence last value is expected to be reset after call to ChangeRepositoryId.";

    //now insert new instance
    ECInstanceKey actualId;
    instance = InsertInstance (actualId, ecdb, testClass);
    ASSERT_TRUE (instance.IsValid ()) << "Inserting ECInstance failed after having changed the repository id.";

    const BeRepositoryBasedId expectedId (expectedRepoId, 1);
    ASSERT_EQ (expectedId.GetValue (), actualId.GetECInstanceId ().GetValue ()) << "First instance inserted after repository id change is expected to be {new repo id | 1}.";

    //finally query for the instance inserted under the old repo id
    ECSqlSelectBuilder ecsqlBuilder;
    SqlPrintfString whereStr ("%s=%lld", ECSqlBuilder::ECINSTANCEID_SYSTEMPROPERTY, idBeforeRepoIdChange.GetECInstanceId ().GetValue ());
    ecsqlBuilder.From (testClass).Select (ECSqlBuilder::ECINSTANCEID_SYSTEMPROPERTY).Where (whereStr);

    ECSqlStatement statement;
    auto ecsqlStat = statement.Prepare (ecdb, ecsqlBuilder.ToString ().c_str ());
    ASSERT_EQ ((int) ECSqlStatus::Success, (int) ecsqlStat) << L"Preparing SQL to retrieve instance inserted before repo id change failed";

    int rowCount = 0;
    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        rowCount++;
        ECInstanceId retrievedId = statement.GetValueId<ECInstanceId> (0);

        EXPECT_EQ (idBeforeRepoIdChange.GetECInstanceId ().GetValue (), retrievedId.GetValue ()) << L"ECInstanceId of retrieved instance doesn't match.";
        }

    EXPECT_EQ (1, rowCount) << L"Query to retrieve instance inserted before repo id change didn't return the expected row.";
    }

END_ECDBUNITTESTS_NAMESPACE