/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Published/FieldEngineer_Test_Old.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <BeJsonCpp/BeJsonUtilities.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

extern bool InsertInstance (ECDbR db, ECClassCR ecClass, IECInstanceR ecInstance);
extern IECRelationshipInstancePtr CreateRelationship (ECN::ECRelationshipClassCR relationshipClass, IECInstanceR source, IECInstanceR target);
extern ECInstanceId InstanceToId (IECInstanceCR ecInstance);
extern void InitializeFieldEngineerTest ();
extern int GetNextId ();
extern bool CreateSimpleCompany (ECDbR ecDb);
extern bool PopulateSimpleCompany (ECDbR ecDb);
    
extern ECInstanceId GetIdOfPerson (ECDbR ecDb, ECClassCR ecClass, Utf8CP firstName, Utf8CP lastName);
extern int CountRows (ECDbR ecDb, Utf8CP ecSqlWithoutSelect);
extern Utf8String BuildECSql (Utf8CP selectClause, Utf8CP ecSqlWithoutSelect);

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                   Ramanujam.Raman                  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (FieldEngineer_Old, Workflow)
    {
    /* 
     * Note: This test has sample code that intentionally doesn't use any ATP helpers
     * for creating/populating the Db. The intent was to provide some sample code for
     * FieldEngineer, and monitor contracts related to FieldEngineer 
     * for regressions. 
     */
    ECDb ecDb;
    InitializeFieldEngineerTest();

    bool bStatus;
    bStatus = CreateSimpleCompany (ecDb);
    ASSERT_TRUE (bStatus);
    bStatus = PopulateSimpleCompany (ecDb);
    ASSERT_TRUE (bStatus);

    /*
     * SimpleCompany Schema:
     *    
     *                          ManagerRelationship 
     *                          
     *                               WhiteShark
     *                                   |
     *         +_________________________|_______________________+
     *         |                         |                       |
     *         |                     Lion Seal                   |
     *         |                         |                       |
     *         |                         |                       |
     *    GoldFish/SilverFish       SalmonFish           SubContractor(400)  
     *
     *                          ContractorRelationship
     *                  SubContractor ______  SalmonFish
     *
     *                            Inheritance Hierarchy
     *                            
     *    Employee (GoldFish, SalmonFish, BronzeFish) -> Manager (LionSeal, WhiteShark)
     *    SubContractor (SubContractor)
     *
     */

    /* 
     * Read the instance from the Db by querying for it. 
     */
     
    ECSchemaP simpleCompanySchema = nullptr;
    auto schemaStatus = ecDb.GetEC().GetSchemaManager().GetECSchema (simpleCompanySchema, "SimpleCompany", true);
    ASSERT_EQ (SUCCESS, schemaStatus);
    ECClassCP employeeClass = simpleCompanySchema->GetClassP (L"Employee");
    ASSERT_TRUE (employeeClass != nullptr);

    // Prepare statement
    ECSqlStatement statement;
    Utf8CP ecSqlWithoutSelect = "FROM sico.Employee WHERE LastName = 'Fish' AND FirstName = 'Gold'";
    ECSqlStatus prepareStatus = statement.Prepare (ecDb, BuildECSql ("SELECT *", ecSqlWithoutSelect).c_str());
    ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);
    ECSqlStepStatus stepStatus = statement.Step();
    ASSERT_TRUE (ECSqlStepStatus::HasRow == stepStatus);

    // Get (Adapt) results as JSON values
    JsonECSqlSelectAdapter jsonAdapter (statement, JsonECSqlSelectAdapter::FormatOptions (ECValueFormat::RawNativeValues));
    Json::Value currentObject;
    bool status = jsonAdapter.GetRowInstance (currentObject);
    ASSERT_TRUE (status);
    
    // Validate
    ASSERT_TRUE (!currentObject["$ECInstanceId"].isNull());
    Utf8String idString = currentObject["$ECInstanceId"].asString();
    ASSERT_TRUE (idString[0] != 0);
    /* Note: Fetching raw values won't set $ECInstanceLabel, $ECClassLabel */
    int initialCount = CountRows (ecDb, ecSqlWithoutSelect);
    ASSERT_EQ (1, initialCount);

    /*
     * Read the instance from the Db by ECInstanceId (lesser code by using a helper)
     */
    ECInstanceId ecInstanceId (BeJsonUtilities::Int64FromValue (currentObject["$ECInstanceId"]));
    JsonReader reader (ecDb, employeeClass->GetId());
    auto readStatus = reader.ReadInstance (currentObject, ecInstanceId, ECValueFormat::RawNativeValues);
    ASSERT_EQ (SUCCESS, readStatus);
    //ECDbTestUtility::DebugDumpJson (currentObject);

    /* 
     * Update the instance in the Db
     */
    ECJsonUpdater updater (ecDb, employeeClass->GetId ());
    currentObject["FirstName"] = "Silver";
    auto updateStatus = updater.Update (currentObject); // Uses ECInstanceId in the currentObject to find the entry to update
    ASSERT_TRUE (updateStatus == SUCCESS);
    int afterUpdateCount = CountRows (ecDb, "FROM sico.Employee WHERE LastName = 'Fish' AND FirstName = 'Gold'");
    ASSERT_EQ (0, afterUpdateCount);
    afterUpdateCount = CountRows (ecDb, "FROM sico.Employee WHERE LastName = 'Fish' AND FirstName = 'Silver'");
    ASSERT_EQ (1, afterUpdateCount);

    /* 
     * Insert a new instance in the Db
     */
    ECJsonInserter inserter (ecDb, employeeClass->GetId ());
    currentObject.removeMember ("$ECInstanceId"); // to ensure it gets populated after the insert
    currentObject["FirstName"] = "Bronze";
    currentObject["EmployeeId"] = GetNextId(); // Otherwise there's a conflict
    auto insertStatus = inserter.Insert (currentObject);
    ASSERT_TRUE (insertStatus == SUCCESS);
    ecSqlWithoutSelect = "FROM sico.Employee WHERE LastName = 'Fish' AND FirstName = 'Bronze'";
    int afterInsertCount = CountRows (ecDb, ecSqlWithoutSelect);
    ASSERT_EQ (1, afterInsertCount);
    ECInstanceId bronzeFishId (BeJsonUtilities::Int64FromValue (currentObject["$ECInstanceId"]));
    ASSERT_TRUE (bronzeFishId.IsValid ());
    
    statement.Finalize();    
    prepareStatus = statement.Prepare (ecDb, BuildECSql ("SELECT *", ecSqlWithoutSelect).c_str());
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    stepStatus = statement.Step();
    ASSERT_TRUE (stepStatus == ECSqlStepStatus::HasRow);
    JsonECSqlSelectAdapter jsonAdapter2 (statement, JsonECSqlSelectAdapter::FormatOptions (ECValueFormat::RawNativeValues));
    status = jsonAdapter2.GetRowInstance (currentObject);
    ASSERT_TRUE (status);

    /*
     * Delete the instance
     */
    ECJsonDeleter deleter (ecDb, employeeClass->GetId ());
    auto deleteStatus = deleter.Delete (bronzeFishId); 
    ASSERT_TRUE (deleteStatus == SUCCESS);
    int afterDeleteCount = CountRows (ecDb, ecSqlWithoutSelect);
    ASSERT_EQ (0, afterDeleteCount);

    ecDb.SaveChanges();

    /* 
     * Retrieving all related items - Get all managers that report to "WhiteShark"
     */
     
    // TODO: Self-joins don't seem to work. Have emailed Affan.
    //statement.Finalize();
    //ecSqlWithoutSelect = "FROM sico.Manager AS ManagerRelated JOIN sico.Manager AS ManagerOrigin USING sico.ManagerRelationship REVERSE "
    //    "WHERE ManagerOrigin.LastName = 'Shark' AND ManagerOrigin.FirstName = 'White'";
    //prepareStatus = statement.Prepare (ecDb, BuildECSql("SELECT *", ecSqlWithoutSelect).c_str());
    //ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);
    //int actualCount = CountRows (ecDb, ecSqlWithoutSelect);
    //ASSERT_EQ (1, actualCount);

    //ecDb.SaveChanges();

    /* 
     * Retrieve specific relationship - Get the ManagerRelationship between "WhiteShark" and "LionSeal"
     * (Tests relationships stored in a link table)
     */
    ECClassCP managerClass = simpleCompanySchema->GetClassP (L"Manager");
    ASSERT_TRUE (managerClass != nullptr);
    ECInstanceId whiteSharkId = GetIdOfPerson (ecDb, *managerClass, "White", "Shark");
    ASSERT_TRUE (whiteSharkId.IsValid ());
    ECInstanceId lionSealId = GetIdOfPerson (ecDb, *managerClass, "Lion", "Seal");
    ASSERT_TRUE (lionSealId.IsValid ());
    ECRelationshipClassCP managerRelClass = simpleCompanySchema->GetClassP (L"ManagerRelationship")->GetRelationshipClassCP();
    ASSERT_TRUE (managerRelClass != nullptr);

    ecSqlWithoutSelect = "FROM sico.ManagerRelationship WHERE SourceECInstanceId=? AND TargetECInstanceId=?";
    statement.Finalize();
    prepareStatus = statement.Prepare (ecDb, BuildECSql("SELECT ECInstanceId", ecSqlWithoutSelect).c_str());
    ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);
    statement.BindId (1, whiteSharkId);
    statement.BindId (2, lionSealId);
    stepStatus = statement.Step();
    ASSERT_TRUE (stepStatus == ECSqlStepStatus::HasRow);
    ECInstanceId managerRelId = statement.GetValueId<ECInstanceId> (0);
    ASSERT_TRUE (managerRelId.IsValid ());
     
     /*
     * Remove relationship and validate
     * (Tests relationships stored in a link table)
     */
     ECJsonDeleter deleter2 (ecDb, managerRelClass->GetId());
     deleteStatus = deleter2.Delete (managerRelId); 
     ASSERT_TRUE (deleteStatus == SUCCESS);

     statement.Finalize();
     prepareStatus = statement.Prepare (ecDb, BuildECSql ("SELECT *", ecSqlWithoutSelect).c_str());
     ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);
     stepStatus = statement.Step();
     ASSERT_TRUE (stepStatus == ECSqlStepStatus::Done); // Validate that there aren't any rows for relationship!!

    /*
    * Retrieve specific relationship - Get SubContractorRelationship between "SubContractor" and "SalmonFish"
    * (Tests relationships stored in one of the end tables)
    */
     ECClassCP subContractorClass = simpleCompanySchema->GetClassP (L"SubContractor");
     ASSERT_TRUE (subContractorClass != nullptr);
     ECInstanceId subContractorId = GetIdOfPerson (ecDb, *subContractorClass, "Sub", "Contractor");
     ASSERT_TRUE (subContractorId.IsValid ());
     ECInstanceId salmonFishId = GetIdOfPerson (ecDb, *employeeClass, "Salmon", "Fish");
     ASSERT_TRUE (salmonFishId.IsValid ());
     ECRelationshipClassCP subContractorRelClass = simpleCompanySchema->GetClassP (L"SubContractorRelationship")->GetRelationshipClassCP();
     ASSERT_TRUE (subContractorRelClass != nullptr);

     ecSqlWithoutSelect = "FROM sico.SubContractorRelationship WHERE SourceECInstanceId=? AND TargetECInstanceId=?";
     statement.Finalize();
     prepareStatus = statement.Prepare (ecDb, BuildECSql ("SELECT *", ecSqlWithoutSelect).c_str());
     ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);
     statement.BindId (1, subContractorId);
     statement.BindId (2, salmonFishId);
     stepStatus = statement.Step();
     ASSERT_TRUE (stepStatus == ECSqlStepStatus::HasRow); 
     ECInstanceId subContractorRelId = statement.GetValueId<ECInstanceId> (0);
     ASSERT_TRUE (subContractorRelId.IsValid ());

    /*
    * Remove relationship and validate
    * (Tests relationships stored in an end table)
    */
     ECJsonDeleter deleter3 (ecDb, subContractorRelClass->GetId());
     deleteStatus = deleter3.Delete (subContractorRelId); 
     ASSERT_TRUE (deleteStatus == SUCCESS);

    statement.Finalize();
    prepareStatus = statement.Prepare (ecDb, BuildECSql ("SELECT *", ecSqlWithoutSelect).c_str());
    ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);
    stepStatus = statement.Step();
    ASSERT_TRUE (stepStatus == ECSqlStepStatus::Done); // Validate that there aren't any rows for relationship!!

    ecDb.SaveChanges();
    }


//**********************************************************************************
// Reproduces delete with referential integrity performance issue reported by Vincas
//**********************************************************************************
void RelateInstances_Old (ECDb& ecdb, ECClassCP sourceClass, ECInstanceId sourceECInstanceId, ECClassCP targetClass, ECInstanceId targetECInstanceId, ECRelationshipClassCP relClass)
    {
    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance ();

    IECInstancePtr sourceInstance = sourceClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    IECInstancePtr targetInstance = targetClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    ECDbTestUtility::SetECInstanceId (*sourceInstance, sourceECInstanceId);
    ECDbTestUtility::SetECInstanceId (*targetInstance, targetECInstanceId);

    relationshipInstance->SetSource (sourceInstance.get ());
    relationshipInstance->SetTarget (targetInstance.get ());

    ECPersistencePtr persistence = ecdb.GetEC ().GetECPersistence (nullptr, *relClass);
    ECInstanceId ecInstanceId;
    InsertStatus insertStatus = persistence->Insert (&ecInstanceId, *relationshipInstance.get ());
    EXPECT_EQ (InsertStatus::INSERT_Success, insertStatus);
    }

ECInstanceId InsertInstance_Old (ECDbR ecdb, ECClassCP ecClass)
    {
    ECJsonInserter inserter (ecdb, ecClass->GetId ());
    Json::Value instance (Json::objectValue);
    inserter.Insert (instance);
    ECInstanceId ecInstanceId (BeJsonUtilities::Int64FromValue (instance["$ECInstanceId"]));
    EXPECT_TRUE (ecInstanceId.IsValid ());
    return ecInstanceId;
    }

extern void CreateDeleteReferentialIntegrityTestDb (BeFileNameR testDbPath);

void RunDeleteReferentialIntegrityTest_Old (bool withRelationsToCachedInfo)
    {
    BeFileName testDbPath;
    CreateDeleteReferentialIntegrityTestDb (testDbPath);

    Utf8CP testSchemaName = "TestSchema";
    Utf8CP testClassName = "TestClass";

    const int childCount = 10000;
    ECInstanceId testECInstanceId;

        {
        ECDb ecdb;
        ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (testDbPath, ECDb::OpenParams (Db::OPEN_ReadWrite)));

        ECClassP testClass = nullptr;
        ecdb.GetEC ().GetSchemaManager ().GetECClass (testClass, testSchemaName, testClassName);

        ECClassP cachedInfoClass = nullptr;
        ecdb.GetEC ().GetSchemaManager ().GetECClass (cachedInfoClass, testSchemaName, "CachedInfoClass");

        ECClassP tempClass = nullptr;
        ecdb.GetEC ().GetSchemaManager ().GetECClass (tempClass, testSchemaName, "TestClassRelationship");
        ECRelationshipClassCP testRelationshipClass = tempClass->GetRelationshipClassCP ();

        tempClass = nullptr;
        ecdb.GetEC ().GetSchemaManager ().GetECClass (tempClass, testSchemaName, "CachedInfoClassRelationship");
        ECRelationshipClassCP cachedInfoRelationshipClass = tempClass->GetRelationshipClassCP ();

        // insert test data
        testECInstanceId = InsertInstance_Old (ecdb, testClass);
        for (int i = 0; i < childCount; i++)
            {
            ECInstanceId childId = InsertInstance_Old (ecdb, testClass);
            RelateInstances_Old (ecdb, testClass, testECInstanceId, testClass, childId, testRelationshipClass);
            if (withRelationsToCachedInfo)
                {
                ECInstanceId infoId = InsertInstance_Old (ecdb, cachedInfoClass);
                RelateInstances_Old (ecdb, cachedInfoClass, infoId, testClass, childId, cachedInfoRelationshipClass);
                }
            }

        ecdb.SaveChanges ();
        }

    //reopen to minimize any caching from the creation and population of the test ecdb
    ECDb ecdb;
    ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (testDbPath, ECDb::OpenParams (Db::OPEN_ReadWrite)));

    ECClassP testClass = nullptr;
    ecdb.GetEC ().GetSchemaManager ().GetECClass (testClass, testSchemaName, testClassName);

    //printf ("Attach to profiler\n");
    //getchar ();

    // Delete
    StopWatch timer (true);
    auto status = ecdb.GetEC ().GetECPersistence (nullptr, *testClass)->Delete (testECInstanceId);
    timer.Stop ();
    ASSERT_EQ (DELETE_Success, status);

    //printf ("Detach from profiler\n");
    //getchar ();

    Utf8CP traceMessage = nullptr;
    if (withRelationsToCachedInfo)
        traceMessage = "ECPersistence::Delete for one instance with %d related child instances which each are related to one cache info instances took %.4f ms.";
    else
        traceMessage = "ECPersistence::Delete for one instance with 1 set of %d related child instances took %.4f ms.";

    LOG.tracev (traceMessage, childCount, timer.GetElapsedSeconds () * 1000.0);
    }


//---------------------------------------------------------------------------------
// @bsiclass                                   Krischan.Eberle            02/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (PerformanceFieldEngineer_Old, DeleteReferentialIntegrityWithTwoRelationships)
    {
    RunDeleteReferentialIntegrityTest_Old (true);
    }

//---------------------------------------------------------------------------------
// @bsiclass                                   Krischan.Eberle            02/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (PerformanceFieldEngineer_Old, DeleteReferentialIntegrityWithOneRelationship)
    {
    RunDeleteReferentialIntegrityTest_Old (false);
    }

END_ECDBUNITTESTS_NAMESPACE


