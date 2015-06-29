/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/FieldEngineer_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.r.h>
#include "ECDbPublishedTests.h"
#include <BeJsonCpp/BeJsonUtilities.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

extern bool InsertInstance (ECDbR db, ECClassCR ecClass, IECInstanceR ecInstance);
extern IECRelationshipInstancePtr CreateRelationship (ECN::ECRelationshipClassCR relationshipClass, IECInstanceR source, IECInstanceR target);
extern ECInstanceId InstanceToId (IECInstanceCR ecInstance);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void InitializeFieldEngineerTest()
    {
    // Initialize ECDb (sets up BeSQLite and
    // standard schema dependencies in the application directory).
    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);

    BeFileName applicationSchemaDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (applicationSchemaDir);

    ECDb::Initialize (temporaryDir, &applicationSchemaDir);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
int GetNextId()
    {
    static int id = 1;
    return id++;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr CreateSubContractor (ECClassCR subContractorClass, WCharCP firstName, WCharCP lastName)
    {
    IECInstancePtr subContractor = subContractorClass.GetDefaultStandaloneEnabler()->CreateInstance(0);
    
    ECValue ecValue;
    ecValue.SetString (firstName, true);
    subContractor->SetValue (L"FirstName", ecValue);

    ecValue.SetString (lastName, true);
    subContractor->SetValue (L"LastName", ecValue);

    ecValue.SetInteger (GetNextId());
    subContractor->SetValue (L"SubContractorId", ecValue);

    return subContractor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr CreateManager (ECClassCR managerClass, WCharCP firstName, WCharCP lastName)
    {
    IECInstancePtr manager = managerClass.GetDefaultStandaloneEnabler()->CreateInstance(0);
    
    ECValue ecValue;
    ecValue.SetString (firstName, true);
    manager->SetValue (L"FirstName", ecValue);

    ecValue.SetString (lastName, true);
    manager->SetValue (L"LastName", ecValue);

    ecValue.SetInteger (GetNextId());
    manager->SetValue (L"EmployeeId", ecValue);

    DateTime dateJoined (1986, 1, 1);
    ecValue.SetDateTime (dateJoined);
    manager->SetValue (L"DateJoined", ecValue);

    ecValue.SetString (L"PhD");
    manager->SetValue (L"LatestEducation.Degree", ecValue);
    ecValue.SetInteger (1980);
    manager->SetValue (L"LatestEducation.Year", ecValue);
    ecValue.SetDouble (4.0);
    manager->SetValue (L"LatestEducation.GPA", ecValue);
    
    return manager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr CreateEmployee (ECClassCR employeeClass, WCharCP firstName, WCharCP lastName)
    {
    IECInstancePtr employee = employeeClass.GetDefaultStandaloneEnabler()->CreateInstance(0);
    
    ECValue ecValue;
    ecValue.SetString (firstName, true);
    employee->SetValue (L"FirstName", ecValue);

    ecValue.SetString (lastName, true);
    employee->SetValue (L"LastName", ecValue);

    ecValue.SetInteger (GetNextId());
    employee->SetValue (L"EmployeeId", ecValue);

    DateTime dateJoined (1995, 1, 1);
    ecValue.SetDateTime (dateJoined);
    employee->SetValue (L"DateJoined", ecValue);

    ecValue.SetString (L"MS");
    employee->SetValue (L"LatestEducation.Degree", ecValue);
    ecValue.SetInteger (1990);
    employee->SetValue (L"LatestEducation.Year", ecValue);
    ecValue.SetDouble (4.5);
    employee->SetValue (L"LatestEducation.GPA", ecValue);
    
    ECPropertyP ecProperty = employeeClass.GetPropertyP (L"EducationHistory");
    ArrayECPropertyCP arrayProperty = ecProperty->GetAsArrayProperty();
    ECClassCP educationHistoryClass = arrayProperty->GetStructElementType();

    employee->AddArrayElements (L"EducationHistory", 3);

    IECInstancePtr masters = educationHistoryClass->GetDefaultStandaloneEnabler()->CreateInstance (0);
    ecValue.SetString (L"MS");
    masters->SetValue (L"Degree", ecValue);
    ecValue.SetInteger (1990);
    masters->SetValue (L"Year", ecValue);
    ecValue.SetDouble (4.5);
    masters->SetValue (L"GPA", ecValue);
    ECValue structValue;
    structValue.SetStruct (masters.get());
    employee->SetValue (L"EducationHistory", structValue, 0);

    IECInstancePtr bachelors = educationHistoryClass->GetDefaultStandaloneEnabler()->CreateInstance (0);
    ecValue.SetString (L"BS");
    bachelors->SetValue (L"Degree", ecValue);
    ecValue.SetInteger (1986);
    bachelors->SetValue (L"Year", ecValue);
    ecValue.SetDouble (5.0);
    bachelors->SetValue (L"GPA", ecValue);
    structValue.SetStruct (bachelors.get());
    employee->SetValue (L"EducationHistory", structValue, 1);

    IECInstancePtr highSchool = educationHistoryClass->GetDefaultStandaloneEnabler()->CreateInstance (0);
    ecValue.SetString (L"HighSchool");
    highSchool->SetValue (L"Degree", ecValue);
    ecValue.SetInteger (1982);
    highSchool->SetValue (L"Year", ecValue);
    ecValue.SetDouble (3.5);
    highSchool->SetValue (L"GPA", ecValue);
    structValue.SetStruct (highSchool.get());
    employee->SetValue (L"EducationHistory", structValue, 2);

    return employee;
    }

    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool CreateSimpleCompany (ECDbR ecDb)
    {
    // Create a new empty db
    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot (outputDir);
    BeFileName projectFile (nullptr, outputDir.GetName(), L"SampleInspector.ecdb", nullptr);
    if (projectFile.DoesPathExist())
        {
        // Delete any previously created file
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (projectFile);
        if (fileDeleteStatus != BeFileNameStatus::Success)
            return false;
        }
    DbResult stat = ecDb.CreateNewDb (projectFile);
    if (stat != BE_SQLITE_OK)
        return false;

    /* Read the sample schema */

    // Construct the sample schema path name
    BeFileName schemaPath;
    BeTest::GetHost().GetDocumentsRoot (schemaPath);
    schemaPath.AppendToPath (L"DgnDb\\ECDb\\Schemas");
    BeFileName schemaPathname (schemaPath);
    schemaPathname.AppendToPath (L"SimpleCompany.01.00.ecschema.xml");
    if (!schemaPathname.DoesPathExist())
        return false;

    // Read the sample schema from disk
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext ();
    context->AddSchemaPath (schemaPathname.GetName()); // Setup locating any dependencies in the same folder
    ECSchemaPtr schema;
    ECSchema::ReadFromXmlFile (schema, schemaPathname.GetName(), *context);
    if (!schema.IsValid())
        return false;

    /* Import schema into Db */
    auto importSchemaStatus = ecDb.Schemas().ImportECSchemas (context->GetCache());
    return (importSchemaStatus == SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool PopulateSimpleCompany (ECDbR ecDb)
    {
    ECSchemaCP schema = ecDb.Schemas().GetECSchema ("SimpleCompany", true);
    if (schema == nullptr)
        return false;

    /* Import sample instances into Db */
    ECClassCP employeeClass = schema->GetClassCP (L"Employee");
    ECClassCP managerClass = schema->GetClassCP (L"Manager");
    ECClassCP subContractorClass = schema->GetClassCP (L"SubContractor");
    ECRelationshipClassCP managerRelClass = schema->GetClassCP (L"ManagerRelationship")->GetRelationshipClassCP();
    ECRelationshipClassCP subContractorRelClass = schema->GetClassCP (L"SubContractorRelationship")->GetRelationshipClassCP();

    IECInstancePtr goldFish = CreateEmployee (*employeeClass,  L"Gold",  L"Fish");
    if (!InsertInstance (ecDb, *employeeClass, *goldFish))
        return false;
    IECInstancePtr salmonFish = CreateEmployee (*employeeClass,  L"Salmon",  L"Fish");
    if (!InsertInstance (ecDb, *employeeClass, *salmonFish))
        return false;
    IECInstancePtr lionSeal = CreateManager (*managerClass, L"Lion",  L"Seal");
    if (!InsertInstance (ecDb, *managerClass, *lionSeal))
        return false;
    IECInstancePtr subContractor = CreateSubContractor (*subContractorClass, L"Sub", L"Contractor");
    if (!InsertInstance (ecDb, *subContractorClass, *subContractor))
        return false;
    IECInstancePtr whiteShark = CreateManager (*managerClass, L"White", L"Shark");
    if (!InsertInstance (ecDb, *managerClass, *whiteShark))
        return false;

    IECRelationshipInstancePtr managerRel;
    managerRel = CreateRelationship (*managerRelClass, *lionSeal, *salmonFish);
    if (!InsertInstance (ecDb, *managerRelClass, *managerRel))
        return false;

    managerRel = CreateRelationship (*managerRelClass, *whiteShark, *goldFish);
    if (!InsertInstance (ecDb, *managerRelClass, *managerRel))
        return false;

    managerRel = CreateRelationship (*managerRelClass, *whiteShark, *lionSeal);
    if (!InsertInstance (ecDb, *managerRelClass, *managerRel))
        return false;

    managerRel = CreateRelationship (*managerRelClass, *whiteShark, *subContractor);
    if (!InsertInstance (ecDb, *managerRelClass, *managerRel))
        return false;

    IECRelationshipInstancePtr subContractorRel;
    subContractorRel = CreateRelationship (*subContractorRelClass, *subContractor, *salmonFish);
    if (!InsertInstance (ecDb, *subContractorRelClass, *subContractorRel))
        return false;

    ecDb.SaveChanges();
    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                   Ramanujam.Raman                  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceId GetIdOfPerson (ECDbR ecDb, ECClassCR ecClass, Utf8CP firstName, Utf8CP lastName)
    {
    ECSqlSelectBuilder sqlBuilder;
    sqlBuilder.From (ecClass).Select (ECSqlBuilder::ECINSTANCEID_SYSTEMPROPERTY).Where ("FirstName = ? AND LastName = ?");

    ECSqlStatement statement;
    auto stat = statement.Prepare (ecDb, sqlBuilder.ToString ().c_str ());
    if (stat != ECSqlStatus::Success)
        return ECInstanceId ();

    statement.BindText (1, firstName, IECSqlBinder::MakeCopy::No);
    statement.BindText (2, lastName, IECSqlBinder::MakeCopy::No);
    if (ECSqlStepStatus::HasRow != statement.Step())
        return ECInstanceId ();

    return statement.GetValueId<ECInstanceId> (0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                   Ramanujam.Raman                  09/13
+---------------+---------------+---------------+---------------+---------------+------*/
int CountRows (ECDbR ecDb, Utf8CP ecSqlWithoutSelect)
    {
    ECSqlStatement stmt;

    Utf8String ecSql = "SELECT COUNT (*) ";
    ecSql.append (ecSqlWithoutSelect);
    if (ECSqlStatus::Success != stmt.Prepare (ecDb, ecSql.c_str()))
        return -1;
    if (ECSqlStepStatus::HasRow != stmt.Step())
        return 0;
    return stmt.GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                   Ramanujam.Raman                  09/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BuildECSql (Utf8CP selectClause, Utf8CP ecSqlWithoutSelect)
    {
    Utf8String ecSql = selectClause;
    ecSql.append (" ");
    ecSql.append (ecSqlWithoutSelect);
    return ecSql;
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                   Ramanujam.Raman                  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (FieldEngineer, Workflow)
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
     
    ECSchemaCP simpleCompanySchema = ecDb.Schemas().GetECSchema ("SimpleCompany", true);
    ASSERT_TRUE (simpleCompanySchema != nullptr);
    ECClassCP employeeClass = simpleCompanySchema->GetClassCP (L"Employee");
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
    JsonUpdater updater (ecDb, *employeeClass);
    currentObject["FirstName"] = "Silver";
    StatusInt updateStatus = updater.Update (currentObject); // Uses ECInstanceId in the currentObject to find the entry to update
    ASSERT_TRUE (updateStatus == SUCCESS);
    int afterUpdateCount = CountRows (ecDb, "FROM sico.Employee WHERE LastName = 'Fish' AND FirstName = 'Gold'");
    ASSERT_EQ (0, afterUpdateCount);
    afterUpdateCount = CountRows (ecDb, "FROM sico.Employee WHERE LastName = 'Fish' AND FirstName = 'Silver'");
    ASSERT_EQ (1, afterUpdateCount);

    /* 
     * Insert a new instance in the Db
     */
    JsonInserter inserter (ecDb, *employeeClass);
    currentObject.removeMember ("$ECInstanceId"); // to ensure it gets populated after the insert
    currentObject["FirstName"] = "Bronze";
    currentObject["EmployeeId"] = GetNextId(); // Otherwise there's a conflict
    StatusInt insertStatus = inserter.Insert (currentObject);
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
    JsonDeleter deleter (ecDb, *employeeClass);
    StatusInt deleteStatus = deleter.Delete (bronzeFishId); 
    ASSERT_TRUE (deleteStatus == SUCCESS);
    int afterDeleteCount = CountRows (ecDb, ecSqlWithoutSelect);
    ASSERT_EQ (0, afterDeleteCount);

    statement.Reset();

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
    ECClassCP managerClass = simpleCompanySchema->GetClassCP (L"Manager");
    ASSERT_TRUE (managerClass != nullptr);
    ECInstanceId whiteSharkId = GetIdOfPerson (ecDb, *managerClass, "White", "Shark");
    ASSERT_TRUE (whiteSharkId.IsValid ());
    ECInstanceId lionSealId = GetIdOfPerson (ecDb, *managerClass, "Lion", "Seal");
    ASSERT_TRUE (lionSealId.IsValid ());
    ECRelationshipClassCP managerRelClass = simpleCompanySchema->GetClassCP (L"ManagerRelationship")->GetRelationshipClassCP();
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
     JsonDeleter deleter2 (ecDb, *managerRelClass);
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
     ECClassCP subContractorClass = simpleCompanySchema->GetClassCP (L"SubContractor");
     ASSERT_TRUE (subContractorClass != nullptr);
     ECInstanceId subContractorId = GetIdOfPerson (ecDb, *subContractorClass, "Sub", "Contractor");
     ASSERT_TRUE (subContractorId.IsValid ());
     ECInstanceId salmonFishId = GetIdOfPerson (ecDb, *employeeClass, "Salmon", "Fish");
     ASSERT_TRUE (salmonFishId.IsValid ());
     ECRelationshipClassCP subContractorRelClass = simpleCompanySchema->GetClassCP (L"SubContractorRelationship")->GetRelationshipClassCP();
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
     JsonDeleter deleter3 (ecDb, *subContractorRelClass);
     deleteStatus = deleter3.Delete (subContractorRelId); 
     ASSERT_TRUE (deleteStatus == SUCCESS);

    statement.Finalize();
    prepareStatus = statement.Prepare (ecDb, BuildECSql ("SELECT *", ecSqlWithoutSelect).c_str());
    ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);
    stepStatus = statement.Step();
    ASSERT_TRUE (stepStatus == ECSqlStepStatus::Done); // Validate that there aren't any rows for relationship!!

    statement.Reset();

    ecDb.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                   Ramanujam.Raman                  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (FieldEngineer, RelationshipIssue)
    {
    ECDb ecDb;
    InitializeFieldEngineerTest();

    /* Create a new empty db */
    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot (outputDir);
    BeFileName projectFile (nullptr, outputDir.GetName(), L"FieldEngineerRelationship.ecdb", nullptr);
    if (BeFileName::DoesPathExist (projectFile.GetName()))
        {
        // Delete any previously created file
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (projectFile.GetName());
        ASSERT_TRUE (fileDeleteStatus == BeFileNameStatus::Success);
        }
    DbResult stat = ecDb.CreateNewDb (projectFile.GetNameUtf8().c_str());
    ASSERT_TRUE (stat == BE_SQLITE_OK);

    /* Import DSCacheJoinSchema */

    // Construct the sample schema path name
    BeFileName schemaPath;
    BeTest::GetHost().GetDocumentsRoot (schemaPath);
    schemaPath.AppendToPath (L"DgnDb\\ECDb\\Schemas");
    BeFileName schemaPathname (schemaPath);
    schemaPathname.AppendToPath (L"DSCacheJoinSchema.01.00.ecschema.xml");
    ASSERT_TRUE (BeFileName::DoesPathExist (schemaPathname.GetName()));

    // Read the sample schema from disk
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext ();
    context->AddSchemaPath (schemaPathname.GetName()); // Setup locating any dependencies in the same folder
    ECSchemaPtr dscJoinSchema;
    ECSchema::ReadFromXmlFile (dscJoinSchema, schemaPathname.GetName(), *context);
    ASSERT_TRUE (dscJoinSchema.IsValid());

    // Import schema into Db
    auto importSchemaStatus = ecDb.Schemas().ImportECSchemas (context->GetCache());
    ASSERT_EQ (SUCCESS, importSchemaStatus);

    /* Import sample instances into Db */
    ECClassCP cachedFileInfoClass = ecDb.Schemas().GetECClass ("DSCacheSchema", "CachedFileInfo");
    ASSERT_TRUE (cachedFileInfoClass != nullptr);

    ECClassCP folderClass = ecDb.Schemas().GetECClass ("eB_PW_CommonSchema_WSB", "Folder");
    ASSERT_TRUE (folderClass != nullptr);

    ECClassCP tmpClass = ecDb.Schemas().GetECClass ("DSCacheJoinSchema", "CachedFileInfoRelationship");
    ECRelationshipClassCP cachedFileInfoRelClass = tmpClass->GetRelationshipClassCP();
    ASSERT_TRUE (cachedFileInfoRelClass != nullptr);

    IECInstancePtr cachedFileInfo = cachedFileInfoClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    IECInstancePtr folder = folderClass->GetDefaultStandaloneEnabler()->CreateInstance(0);

    ASSERT_TRUE (InsertInstance (ecDb, *cachedFileInfoClass, *cachedFileInfo));
    ASSERT_TRUE (InsertInstance (ecDb, *folderClass, *folder));

    IECRelationshipInstancePtr cachedFileInfoRel;
    cachedFileInfoRel = CreateRelationship (*cachedFileInfoRelClass, *cachedFileInfo, *folder);
    ASSERT_TRUE (InsertInstance (ecDb, *cachedFileInfoRelClass, *cachedFileInfoRel));

    // Test retrieval of relationship instance directly
    ECSqlStatement statement;
    ECSqlStatus prepareStatus = statement.Prepare (ecDb, "SELECT * FROM ONLY DSCJS.CachedFileInfoRelationship WHERE CachedFileInfoRelationship.ECInstanceId = ?");
    ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);
    statement.BindId (1, InstanceToId (*cachedFileInfoRel));
    ECSqlStepStatus stepStatus = statement.Step();
    ASSERT_TRUE (stepStatus == ECSqlStepStatus::HasRow); 

    // Test retrieval of related instances
    statement.Finalize();
    prepareStatus = statement.Prepare (ecDb, "SELECT * FROM DSC.CachedFileInfo JOIN eBPWC.Folder USING DSCJS.CachedFileInfoRelationship WHERE Folder.ECInstanceId = ?");
    ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);
    statement.BindId (1, InstanceToId (*folder));
    stepStatus = statement.Step();
    ASSERT_TRUE (stepStatus == ECSqlStepStatus::HasRow); 

    ecDb.SaveChanges();
    }
    
/*---------------------------------------------------------------------------------**//**
* Monitors the fix for a  crash due to circular references 
* @bsiclass                                   Ramanujam.Raman                  10/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (FieldEngineer, CircularReferences)
    {
    ECDb ecDb;
    InitializeFieldEngineerTest();
    bool status;
    status = CreateSimpleCompany (ecDb);
    ASSERT_TRUE (status);

    ECSchemaCP schema = ecDb.Schemas().GetECSchema ("SimpleCompany", true);
    ASSERT_TRUE (schema != nullptr);

    ECClassCP managerClass = schema->GetClassCP (L"Manager");
    ECRelationshipClassCP managerRelClass = schema->GetClassCP (L"ManagerRelationship")->GetRelationshipClassCP();

    IECInstancePtr manager1 = CreateManager (*managerClass, L"ImBig", L"Manager");
    ASSERT_TRUE (InsertInstance (ecDb, *managerClass, *manager1));
    IECInstancePtr manager2 = CreateManager (*managerClass, L"ImBigger", L"Manager");
    ASSERT_TRUE (InsertInstance (ecDb, *managerClass, *manager2));
    IECInstancePtr manager3 = CreateManager (*managerClass, L"ImBiggest", L"Manager");
    ASSERT_TRUE (InsertInstance (ecDb, *managerClass, *manager3));

    // Create all permutations of relationships
    IECRelationshipInstancePtr managerRel;
    managerRel = CreateRelationship (*managerRelClass, *manager1, *manager2);
    ASSERT_TRUE (InsertInstance (ecDb, *managerRelClass, *managerRel));
    managerRel = CreateRelationship (*managerRelClass, *manager2, *manager1);
    ASSERT_TRUE (InsertInstance (ecDb, *managerRelClass, *managerRel));
    managerRel = CreateRelationship (*managerRelClass, *manager2, *manager3);
    ASSERT_TRUE (InsertInstance (ecDb, *managerRelClass, *managerRel));
    managerRel = CreateRelationship (*managerRelClass, *manager3, *manager2);
    ASSERT_TRUE (InsertInstance (ecDb, *managerRelClass, *managerRel));
    managerRel = CreateRelationship (*managerRelClass, *manager1, *manager3);
    ASSERT_TRUE (InsertInstance (ecDb, *managerRelClass, *managerRel));
    managerRel = CreateRelationship (*managerRelClass, *manager3, *manager1);
    ASSERT_TRUE (InsertInstance (ecDb, *managerRelClass, *managerRel));

    ECSqlStatement statement;
    ECSqlStatus prepareStatus = statement.Prepare (ecDb, "SELECT ECInstanceId FROM SimpleCompany.Manager WHERE FirstName = 'ImBig'"); 
    ASSERT_TRUE (prepareStatus == ECSqlStatus::Success);
    ECInstanceKeyMultiMap seedInstanceKeyMap;
    ECSqlStepStatus stepStatus;
    while ((stepStatus = statement.Step()) == ECSqlStepStatus::HasRow)
        {
        ECInstanceKeyMultiMapPair instanceEntry (managerClass->GetId(), statement.GetValueId<ECInstanceId> (0));
        seedInstanceKeyMap.insert (instanceEntry);
        }
    ASSERT_TRUE (seedInstanceKeyMap.size() > 0);

    ECInstanceKeyMultiMap instanceKeyMap;
    ECInstanceFinder instanceFinder (ecDb);
    instanceFinder.FindInstances (instanceKeyMap, seedInstanceKeyMap, ECInstanceFinder::FindOptions (ECInstanceFinder::RelatedDirection_Referencing, 0));
    ASSERT_EQ (1, (int) instanceKeyMap.size());
    instanceFinder.FindInstances (instanceKeyMap, seedInstanceKeyMap, ECInstanceFinder::FindOptions (ECInstanceFinder::RelatedDirection_Referencing, 1));
    ASSERT_EQ (3, (int) instanceKeyMap.size());
    instanceFinder.FindInstances (instanceKeyMap, seedInstanceKeyMap, ECInstanceFinder::FindOptions (ECInstanceFinder::RelatedDirection_Referencing, UINT8_MAX));
    ASSERT_EQ (3, (int) instanceKeyMap.size());

    instanceFinder.Finalize();
    }

//**********************************************************************************
// Reproduces delete with referential integrity performance issue reported by Vincas
//**********************************************************************************
void RelateInstances (ECDb& ecdb, ECClassCP sourceClass, ECInstanceId sourceECInstanceId, ECClassCP targetClass, ECInstanceId targetECInstanceId, ECRelationshipClassCP relClass)
    {
    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance ();

    IECInstancePtr sourceInstance = sourceClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    IECInstancePtr targetInstance = targetClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    ECDbTestUtility::SetECInstanceId (*sourceInstance, sourceECInstanceId);
    ECDbTestUtility::SetECInstanceId (*targetInstance, targetECInstanceId);
    relationshipInstance->SetSource (sourceInstance.get ());
    relationshipInstance->SetTarget (targetInstance.get ());

    ECInstanceInserter inserter (ecdb, *relClass);
    ASSERT_TRUE (inserter.IsValid ());
    ECInstanceKey ecInstanceKey;
    BentleyStatus insertStatus = inserter.Insert (ecInstanceKey, *relationshipInstance.get ());
    ASSERT_EQ (SUCCESS, insertStatus);
    }

ECInstanceKey InsertInstance (ECDbR ecdb, ECClassCP ecClass)
    {
    JsonInserter inserter (ecdb, *ecClass);
    Json::Value instance (Json::objectValue);
    ECInstanceKey instanceKey;
    inserter.Insert (instanceKey, instance);
    EXPECT_TRUE (instanceKey.IsValid ());
    return instanceKey;
    }

void CreateDeleteReferentialIntegrityTestDb (BeFileNameR testDbPath)
    {
    BeFileName tempDir;
    BeTest::GetHost ().GetTempDir (tempDir);
    BeFileName assetsDir;
    BeTest::GetHost ().GetDgnPlatformAssetsDirectory (assetsDir);
    ECDb::Initialize (tempDir, &assetsDir);

    /* Create a new empty db */
    BeFileName outputDir;
    BeTest::GetHost ().GetOutputRoot (outputDir);
    testDbPath = BeFileName (nullptr, outputDir.GetName (), L"fieldengineerdeletereferentialintegrity.ecdb", nullptr);
    if (BeFileName::DoesPathExist (testDbPath.GetName ()))
        {
        // Delete any previously created file
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (testDbPath.GetName ());
        ASSERT_TRUE (fileDeleteStatus == BeFileNameStatus::Success);
        }

    ECDb ecdb;
    DbResult stat = ecdb.CreateNewDb (testDbPath.GetNameUtf8 ().c_str ());
    ASSERT_TRUE (stat == BE_SQLITE_OK);

    // Test schema
    Utf8CP schemaXml =
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"TS\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"

        "  <ECClass typeName=\"TestClass\" >"
        "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
        "  </ECClass>"

        "  <ECRelationshipClass typeName = \"TestClassRelationship\" isDomainClass = \"True\" strength = \"holding\" strengthDirection = \"forward\">"
        "    <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "      <Class class = \"TestClass\" />"
        "    </Source>"
        "    <Target cardinality = \"(0, N)\" polymorphic = \"True\">"
        "      <Class class = \"TestClass\" />"
        "    </Target>"
        "  </ECRelationshipClass>"

        // Classes below reduces performance significantly
        "  <ECClass typeName=\"CachedInfoClass\" >"
        "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
        "  </ECClass>"

        "  <ECRelationshipClass typeName = \"CachedInfoClassRelationship\" isDomainClass = \"True\" strength = \"holding\" strengthDirection = \"backward\">"
        "    <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "      <Class class = \"CachedInfoClass\" />"
        "    </Source>"
        "    <Target cardinality = \"(0, N)\" polymorphic = \"True\">"
        "      <Class class = \"TestClass\" />"
        "    </Target>"
        "  </ECRelationshipClass>"

        "</ECSchema>";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext ();
    ECSchemaPtr schema = nullptr;
    ECSchema::ReadFromXmlString (schema, schemaXml, *context);

    // Create DB
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create ();
    schemaCache->AddSchema (*schema);

    ASSERT_EQ (SUCCESS, ecdb. Schemas ().ImportECSchemas (*schemaCache));
    }

void RunDeleteReferentialIntegrityTest (bool withRelationsToCachedInfo)
    {
    BeFileName testDbPath;
    CreateDeleteReferentialIntegrityTestDb (testDbPath);

    Utf8CP testSchemaName = "TestSchema";
    Utf8CP testClassName = "TestClass";

    const int childCount = 10000;
    ECInstanceKey testECInstanceKey;

        {
        ECDb ecdb;
        ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (testDbPath, ECDb::OpenParams (Db::OpenMode::ReadWrite)));

        ECClassCP testClass = ecdb. Schemas ().GetECClass (testSchemaName, testClassName);

        ECClassCP cachedInfoClass = ecdb. Schemas ().GetECClass (testSchemaName, "CachedInfoClass");

        ECClassCP tempClass = ecdb. Schemas ().GetECClass (testSchemaName, "TestClassRelationship");
        ECRelationshipClassCP testRelationshipClass = tempClass->GetRelationshipClassCP ();

        tempClass = ecdb. Schemas ().GetECClass (testSchemaName, "CachedInfoClassRelationship");
        ECRelationshipClassCP cachedInfoRelationshipClass = tempClass->GetRelationshipClassCP ();

        // insert test data
        testECInstanceKey = InsertInstance (ecdb, testClass);
        for (int i = 0; i < childCount; i++)
            {
            ECInstanceKey childKey = InsertInstance (ecdb, testClass);
            RelateInstances (ecdb, testClass, testECInstanceKey.GetECInstanceId (), testClass, childKey.GetECInstanceId (), testRelationshipClass);
            if (withRelationsToCachedInfo)
                {
                ECInstanceKey infoKey = InsertInstance (ecdb, cachedInfoClass);
                RelateInstances (ecdb, cachedInfoClass, infoKey.GetECInstanceId (), testClass, childKey.GetECInstanceId (), cachedInfoRelationshipClass);
                }
            }

        ecdb.SaveChanges ();
        }

    //reopen to minimize any caching from the creation and population of the test ecdb
    ECDb ecdb;
    ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (testDbPath, ECDb::OpenParams (Db::OpenMode::ReadWrite)));

    ECClassCP testClass = ecdb. Schemas ().GetECClass (testSchemaName, testClassName);

    //printf ("Attach to profiler\n");
    //getchar ();

    // Delete
    StopWatch timer (true);
    ECInstanceDeleter deleter (ecdb, *testClass);
    auto status = deleter.Delete (testECInstanceKey.GetECInstanceId ());
    timer.Stop ();
    ASSERT_EQ (SUCCESS, status);

    //printf ("Detach from profiler\n");
    //getchar ();

    Utf8CP traceMessage = nullptr;
    if (withRelationsToCachedInfo)
        traceMessage = "ECInstanceDeleter::Delete for one instance with %d related child instances which each are related to one cache info instances took %.4f ms.";
    else
        traceMessage = "ECInstanceDeleter::Delete for one instance with 1 set of %d related child instances took %.4f ms.";

    LOG.tracev (traceMessage, childCount, timer.GetElapsedSeconds () * 1000.0);
    }

void RunDeleteReferentialIntegrityTestUsingECSql (bool withRelationsToCachedInfo)
    {
    BeFileName testDbPath;
    CreateDeleteReferentialIntegrityTestDb (testDbPath);

    Utf8CP testSchemaName = "TestSchema";
    Utf8CP testClassName = "TestClass";

    const int childCount = 10000;
    ECInstanceKey testECInstanceKey;

        {
        ECDb ecdb;
        ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (testDbPath, ECDb::OpenParams (Db::OpenMode::ReadWrite)));

        ECClassCP testClass = ecdb. Schemas ().GetECClass (testSchemaName, testClassName);

        ECClassCP cachedInfoClass = ecdb. Schemas ().GetECClass (testSchemaName, "CachedInfoClass");

        ECClassCP tempClass = ecdb. Schemas ().GetECClass (testSchemaName, "TestClassRelationship");
        ECRelationshipClassCP testRelationshipClass = tempClass->GetRelationshipClassCP ();

        tempClass = ecdb. Schemas ().GetECClass (testSchemaName, "CachedInfoClassRelationship");
        ECRelationshipClassCP cachedInfoRelationshipClass = tempClass->GetRelationshipClassCP ();

        // insert test data
        testECInstanceKey = InsertInstance (ecdb, testClass);
        for (int i = 0; i < childCount; i++)
            {
            ECInstanceKey childKey = InsertInstance (ecdb, testClass);
            RelateInstances (ecdb, testClass, testECInstanceKey.GetECInstanceId (), testClass, childKey.GetECInstanceId (), testRelationshipClass);
            if (withRelationsToCachedInfo)
                {
                ECInstanceKey infoKey = InsertInstance (ecdb, cachedInfoClass);
                RelateInstances (ecdb, cachedInfoClass, infoKey.GetECInstanceId (), testClass, childKey.GetECInstanceId (), cachedInfoRelationshipClass);
                }
            }

        ecdb.SaveChanges ();
        }

    //reopen to minimize any caching from the creation and population of the test ecdb
    ECDb ecdb;
    ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (testDbPath, ECDb::OpenParams (Db::OpenMode::ReadWrite)));

    //printf ("Attach to profiler\n");
    //getchar ();

    // Delete
    StopWatch timer (true);

    ECSqlStatement stmt;
    auto preparedStatus = stmt.Prepare (ecdb, SqlPrintfString ("DELETE FROM ONLY [%s].[%s] WHERE ECInstanceId = ?", testSchemaName, testClassName).GetUtf8CP());
    ASSERT_EQ ((int)preparedStatus, (int)ECSqlStatus::Success);
    stmt.BindId (1, testECInstanceKey.GetECInstanceId ());
    auto stepStatus = stmt.Step ();
    ASSERT_EQ ((int)stepStatus, (int)ECSqlStepStatus::Done);
    timer.Stop ();
    //printf ("Detach from profiler\n");
    //getchar ();

    Utf8CP traceMessage = nullptr;
    if (withRelationsToCachedInfo)
        traceMessage = "ECSQL DELETE for one instance with %d related child instances which each are related to one cache info instances took %.4f ms.";
    else
        traceMessage = "ECSQL DELETE for one instance with 1 set of %d related child instances took %.4f ms.";

    LOG.tracev (traceMessage, childCount, timer.GetElapsedSeconds () * 1000.0);
    }


//---------------------------------------------------------------------------------
// @bsiclass                                   Krischan.Eberle            02/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (PerformanceFieldEngineer, DeleteReferentialIntegrityWithTwoRelationshipsWithECSql)
    {
    RunDeleteReferentialIntegrityTestUsingECSql (true);
    }

//---------------------------------------------------------------------------------
// @bsiclass                                   Krischan.Eberle            02/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (PerformanceFieldEngineer, DeleteReferentialIntegrityWithOneRelationshipWithECSql)
    {
    RunDeleteReferentialIntegrityTestUsingECSql (false);
    }

//---------------------------------------------------------------------------------
// @bsiclass                                   Krischan.Eberle            02/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (PerformanceFieldEngineer, DeleteReferentialIntegrityWithTwoRelationships)
    {
    RunDeleteReferentialIntegrityTest (true);
    }

//---------------------------------------------------------------------------------
// @bsiclass                                   Krischan.Eberle            02/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (PerformanceFieldEngineer, DeleteReferentialIntegrityWithOneRelationship)
    {
    RunDeleteReferentialIntegrityTest (false);
    }

END_ECDBUNITTESTS_NAMESPACE


