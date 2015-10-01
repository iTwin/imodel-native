/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbRelationshipTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../BackDoor/PublicApi/BackDoor/ECDb/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
bool SetStringValue(IECInstanceR instance, Utf8CP propertyAccessor, Utf8CP stringValue);
bool InsertInstance(ECDbR db, ECClassCR ecClass, IECInstanceR ecInstance);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
IECRelationshipInstancePtr CreateRelationship (ECN::ECRelationshipClassCR relationshipClass, IECInstanceR source, IECInstanceR target)
    {
    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (relationshipClass);
    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance();
    if (relationshipInstance == nullptr)
        return nullptr;

    relationshipInstance->SetSource (&source);
    relationshipInstance->SetTarget (&target);
    relationshipInstance->SetInstanceId ("source->target");
    return relationshipInstance;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Adeel Shoukat                   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
IECRelationshipInstancePtr CreateRelationshipWithProperty(ECN::ECRelationshipClassCR relationshipClass, IECInstanceR source, IECInstanceR target)
{
    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(relationshipClass);
    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance();
    SetStringValue(*relationshipInstance, "price", "200");
    if (relationshipInstance == nullptr)
        return nullptr;

    relationshipInstance->SetSource(&source);
    relationshipInstance->SetTarget(&target);
    relationshipInstance->SetInstanceId("source->target");
    return relationshipInstance;
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
IECRelationshipInstancePtr CreateRelationship 
(
ECDbTestProject& test, 
Utf8CP schemaName,
Utf8CP sourceClassName,
Utf8CP targetClassName, 
Utf8CP relationshipClassName
)
    {
    bvector<IECInstancePtr> instances;
    auto stat = test.GetInstances (instances, schemaName, sourceClassName);
    if (stat != SUCCESS)
        //GetInstances asserts already on failure
        return nullptr;

    IECInstancePtr sourceInstance = instances[0];

    stat = test.GetInstances (instances, schemaName, targetClassName);
    if (stat != SUCCESS)
        //GetInstances asserts already on failure
        return nullptr;

    IECInstancePtr targetInstance = instances[0];

    ECRelationshipClassCP relClass = test.GetECDbCR().Schemas().GetECClass(schemaName, relationshipClassName)->GetRelationshipClassCP();
    EXPECT_TRUE (relClass != nullptr) << "Could not find relationship class " << relationshipClassName << " in test schema";
    if (relClass == nullptr)
        return nullptr;

    return CreateRelationship (*relClass, *sourceInstance, *targetInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceId InstanceToId (IECInstanceCR ecInstance)
    {
    Utf8StringCR idString = ecInstance.GetInstanceId();
    ECInstanceId ecInstanceId;
    bool status = ECInstanceIdHelper::FromString (ecInstanceId, idString.c_str());
    BeAssert (status);
    return ecInstanceId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassId ClassToId (ECClassCR ecClass)
    {
    return ecClass.GetId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PersistRelationship (IECRelationshipInstanceR relInstance, ECDbR ecdb)
    {
    ECInstanceInserter inserter (ecdb, relInstance.GetClass ());
    ASSERT_TRUE (inserter.IsValid ());
    auto insertStatus = inserter.Insert (relInstance);
    ASSERT_EQ (SUCCESS, insertStatus);    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ValidatePersistingRelationship
(
DbR db, 
Utf8CP tableName, 
ECInstanceId whereECInstanceId, 
Utf8CP expectedIdColumnName, 
int64_t expectedId
)
    {
    Utf8String whereClause;
    whereClause.Sprintf ("WHERE ECInstanceId=%lld", whereECInstanceId.GetValue ());
    const int64_t actualId = ECDbTestUtility::ReadCellValueAsInt64 (db, tableName, expectedIdColumnName, whereClause.c_str());
    ASSERT_EQ (expectedId, actualId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   02/14
//+---------------+---------------+---------------+---------------+---------------+------
bool GetECInstanceIdFromECInstance (ECInstanceId& ecInstanceId, IECInstanceCR instance)
    {
    Utf8String instanceId = instance.GetInstanceId ();
    if (instanceId.empty ())
        return false;

    return ECInstanceIdHelper::FromString (ecInstanceId, instanceId.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ValidateReadingRelationship (ECDbR ecdb, Utf8CP relationshipSchemaName, Utf8CP relationshipName, IECRelationshipInstanceCR importedRelInstance)
    {
    // Get relationship class
    ECClassCP tmpClass = ecdb.Schemas().GetECClass (relationshipSchemaName, relationshipName);
    ASSERT_TRUE (tmpClass != nullptr);
    ECRelationshipClassCP relationshipClass = tmpClass->GetRelationshipClassCP();
    ASSERT_TRUE (relationshipClass != nullptr);

    SqlPrintfString ecSql ("SELECT rel.* FROM %s.%s rel WHERE rel.ECInstanceId=?", Utf8String(relationshipClass->GetSchema().GetName()).c_str(), Utf8String(relationshipClass->GetName()).c_str());
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare (ecdb, ecSql.GetUtf8CP())) << ecSql.GetUtf8CP();
    statement.BindId(1, InstanceToId(importedRelInstance));

    ASSERT_TRUE(BE_SQLITE_ROW == statement.Step());
    int64_t ecInstanceId = statement.GetValueInt64 (0);
    int64_t sourceECInstanceId = statement.GetValueInt64 (1);
    int64_t sourceECClassId = statement.GetValueInt64 (2);
    int64_t targetECInstanceId = statement.GetValueInt64 (3);
    int64_t targetECClassId = statement.GetValueInt64 (4);
    ASSERT_TRUE (ecInstanceId != 0 && sourceECInstanceId != 0 && sourceECClassId != 0 && targetECInstanceId != 0 && targetECClassId != 0);

    // Get and check result
    ECInstanceECSqlSelectAdapter adapter (statement);
    IECInstancePtr readRelInstance = adapter.GetInstance();
    ASSERT_FALSE (readRelInstance.IsNull());
    ASSERT_TRUE (importedRelInstance.GetInstanceId() == readRelInstance->GetInstanceId());
    ASSERT_TRUE (ECDbTestUtility::CompareECInstances (importedRelInstance, *readRelInstance));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ValidateReadingRelated (ECDbR ecdb, Utf8CP relationshipSchemaName, Utf8CP relationshipName, IECInstancePtr relSourceInstance, IECInstancePtr relTargetInstance)
    {
    // Get join end
    ECClassCR relSourceClass = relSourceInstance->GetClass();
    ECInstanceId relSourceECInstanceId = InstanceToId (*relSourceInstance);
    Utf8String relSourceClassName(relSourceClass.GetName());

    // Get related end
    ECClassCR relTargetClass = relTargetInstance->GetClass();
    Utf8String relTargetClassName(Utf8String(relTargetClass.GetName()));

    // Get relationship class
    ECClassCP tmpClass = ecdb.Schemas().GetECClass(relationshipSchemaName, relationshipName);
    ASSERT_TRUE (tmpClass != nullptr);
    ECRelationshipClassCP relationshipClass = tmpClass->GetRelationshipClassCP();
    ASSERT_TRUE (relationshipClass != nullptr);

    SqlPrintfString ecSql ("SELECT %s.* FROM ONLY %s.%s JOIN %s.%s USING %s.%s WHERE %s.ECInstanceId = ?",
        relTargetClassName.c_str(),
        relTargetClass.GetSchema().GetName().c_str(), relTargetClassName.c_str(),
        relSourceClass.GetSchema().GetName().c_str(), relSourceClassName.c_str(),
        tmpClass->GetSchema().GetName().c_str(), tmpClass->GetName().c_str(),
        relSourceClassName.c_str());

    // Build SQL
    // Prepare and execute ECSqlStatement
    ECSqlStatement statement;
    ECSqlStatus prepareStatus = statement.Prepare (ecdb, ecSql.GetUtf8CP());
    ASSERT_EQ(ECSqlStatus::Success, prepareStatus) << ecSql.GetUtf8CP();
    statement.BindId(1, relSourceECInstanceId);
    ASSERT_TRUE(BE_SQLITE_ROW == statement.Step());

    // Get related instance
    ECInstanceECSqlSelectAdapter adapter (statement);
    IECInstancePtr actualRelTargetInstance = adapter.GetInstance();
    ASSERT_TRUE (actualRelTargetInstance.IsValid());
    ASSERT_TRUE (actualRelTargetInstance->GetInstanceId() == relTargetInstance->GetInstanceId());
    ASSERT_TRUE (ECDbTestUtility::CompareECInstances (*actualRelTargetInstance, *relTargetInstance));
    }


//-------------------------------------------------------------------------------------
// This test is to document and back the current contract about what the ECInstanceId of an inserted ECRelationship is.
// @bsimethod                                   Krischan.Eberle                   06/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbRelationships, RelationshipECInstanceIdContract)
    {
    // Create and populate a sample project
    ECDbTestProject test;
    auto& ecdb = test.Create ("ecrelationshipecinstanceidcontract.ecdb", L"StartupCompany.02.00.ecschema.xml", true);

        {
        //Inserting relationship with non-link-table mapping
        auto relationshipInstance = CreateRelationship (test, "StartupCompany", "Employee", "Chair", "EmployeeFurniture");
        ASSERT_TRUE(relationshipInstance.IsValid());
        PersistRelationship (*relationshipInstance, ecdb);

        ECInstanceId relECInstanceId;
        ASSERT_TRUE (GetECInstanceIdFromECInstance (relECInstanceId, *relationshipInstance));
        ECInstanceId sourceECInstanceId;
        ASSERT_TRUE (GetECInstanceIdFromECInstance (sourceECInstanceId, *relationshipInstance->GetSource ()));
        ECInstanceId targetECInstanceId;
        ASSERT_TRUE (GetECInstanceIdFromECInstance (targetECInstanceId, *relationshipInstance->GetTarget ()));

        EXPECT_EQ (targetECInstanceId.GetValue (), relECInstanceId.GetValue ()) << "For non-link table mappings, the ECInstanceId of the relationship is equal to the ECInstanceId of the child instance, i.e. the instance on the end of the relationship into whose table the relationship is persisted";
        EXPECT_NE (sourceECInstanceId.GetValue (), relECInstanceId.GetValue ()) << "For non-link table mappings, the ECInstanceId of the relationship must not be equal to the ECInstanceId of the parent instance.";
        LOG.infov (L"ECRelationship ECInstanceId Contract for non-link-table mappings: Relationship ECInstanceId: %lld - Source ECInstanceId: %lld - Target ECInstanceId: %lld (Target is where relationship is persisted to)",
            relECInstanceId.GetValue (), sourceECInstanceId.GetValue (), targetECInstanceId.GetValue ());
        }

        {
        //Inserting relationship with link-table mapping
        auto relationshipInstance = CreateRelationship (test, "StartupCompany", "Employee", "Hardware", "EmployeeHardware");
        ASSERT_TRUE(relationshipInstance.IsValid());
        PersistRelationship (*relationshipInstance, ecdb);

        ECInstanceId relECInstanceId;
        ASSERT_TRUE (GetECInstanceIdFromECInstance (relECInstanceId, *relationshipInstance));
        ECInstanceId sourceECInstanceId;
        ASSERT_TRUE (GetECInstanceIdFromECInstance (sourceECInstanceId, *relationshipInstance->GetSource ()));
        ECInstanceId targetECInstanceId;
        ASSERT_TRUE (GetECInstanceIdFromECInstance (targetECInstanceId, *relationshipInstance->GetTarget ()));

        EXPECT_NE (sourceECInstanceId.GetValue (), relECInstanceId.GetValue ()) << "For link table mappings, the ECInstanceId of the relationship is different from the ECInstanceIds of source and target instance.";
        EXPECT_NE (targetECInstanceId.GetValue (), relECInstanceId.GetValue ()) << "For link table mappings, the ECInstanceId of the relationship is different from the ECInstanceIds of source and target instance.";
        LOG.infov (L"ECRelationship ECInstanceId Contract for link-table mappings: Relationship ECInstanceId: %lld - Source ECInstanceId: %lld - Target ECInstanceId: %lld",
            relECInstanceId.GetValue (), sourceECInstanceId.GetValue (), targetECInstanceId.GetValue ());
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbRelationships, MappingRelationshipsWithAdditionalECProperties)
    {
    // Create and populate a sample project
    ECDbTestProject test;
    auto& ecdb = test.Create ("ecrelationshipmapping.ecdb", L"StartupCompany.02.00.ecschema.xml", true);

    const DateTime created (2010, 8, 1);

   ASSERT_TRUE (ecdb.ColumnExists ("sc_RelationWithLinkTableMapping", "Created")) << "Link table is expected to have columns for any additional property on the ECRelationshipClass";

        {
        BeTest::SetFailOnAssert (false);
        //Inserting relationship with link-table mapping
        auto relationshipInstance = CreateRelationship (test, "StartupCompany", "Employee", "Hardware", "RelationWithLinkTableMapping");
        ASSERT_TRUE (relationshipInstance != nullptr) << "Creating the relationship instance in memory is not expected to fail.";
        ASSERT_EQ (ECOBJECTS_STATUS_Success, relationshipInstance->SetValue ("Created", ECValue (created)));

        ECInstanceInserter inserter (ecdb, relationshipInstance->GetClass());
        ASSERT_TRUE (inserter.IsValid ());
        ECInstanceKey instanceKey;
        ASSERT_EQ (SUCCESS, inserter.Insert (instanceKey, *relationshipInstance)) << "Inserting relationship with additional properties is expected to be supported for link table mapping";
        BeTest::SetFailOnAssert (true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ECDbRelationships, ImportECRelationshipInstances)
    {
    // Create and populate a sample project
    ECDbTestProject test;
    ECDbR db = test.Create ("importecrelationshipinstances.ecdb", L"StartupCompany.02.00.ecschema.xml", true);

    IECRelationshipInstancePtr relInstance;

    // Import a 1-to-1 relationship
    // EmployeePhone
    //   Inherits: AssetRelationshipsBase, EmployeeRelationshipsBase
    //   Relates:  Employee (1) -> Phone (1)
    //   Should write to sc_Phone table
    relInstance = CreateRelationship (test, "StartupCompany", "Employee", "Phone", "EmployeePhone");
    ASSERT_TRUE (relInstance.IsValid());
    PersistRelationship (*relInstance, db);
    ValidatePersistingRelationship (db, "sc_Phone", InstanceToId (*relInstance->GetTarget()), 
        "Employee__src_01_id", InstanceToId (*relInstance->GetSource()).GetValue ());
    ValidateReadingRelationship (db,"StartupCompany", "EmployeePhone", *relInstance);
    ValidateReadingRelated (db, "StartupCompany", "EmployeePhone", relInstance->GetSource(), relInstance->GetTarget());

    // Import another 1-to-1 relationships (with multiple classes on one end)
    // Foo_has_SomethingInOneOfManyTables
    //   Relates: Foo (1) -> Asset, Employee (1)
    relInstance = CreateRelationship (test, "StartupCompany", "Foo", "Asset", "Foo_has_SomethingInOneOfManyTables");
    ASSERT_TRUE (relInstance.IsValid());
    PersistRelationship (*relInstance, db);

    db.SaveChanges ();
    ValidateReadingRelationship (db, "StartupCompany", "Foo_has_SomethingInOneOfManyTables", *relInstance);
    ValidateReadingRelated (db, "StartupCompany", "Foo_has_SomethingInOneOfManyTables", relInstance->GetSource(), relInstance->GetTarget());

    //now attempt to insert a second relation to same Foo object which will violate cardinality
    relInstance = CreateRelationship (test, "StartupCompany", "Foo", "Employee", "Foo_has_SomethingInOneOfManyTables");
    ASSERT_TRUE (relInstance.IsValid());
    ECInstanceInserter inserter (db, relInstance->GetClass ());
    ASSERT_TRUE (inserter.IsValid ());
    BentleyStatus insertStatus = inserter.Insert (*relInstance);
    ASSERT_EQ (ERROR, insertStatus) << "Inserting second relationship for same Foo instance is expected to violate cardinality 1:1";

    // Import 1-to-M relationships
    // EmployeeFurniture
    //   Inherits: AssetRelationshipsBase, EmployeeRelationshipsBase
    //   Relates: Employee (1) -> Chair, Desk  (M)
    relInstance = CreateRelationship (test, "StartupCompany", "Employee", "Chair", "EmployeeFurniture");
    ASSERT_TRUE (relInstance.IsValid());
    PersistRelationship (*relInstance, db);
    ValidatePersistingRelationship (db, "sc_Furniture", InstanceToId (*relInstance->GetTarget()), 
        "Employee__src_01_id", InstanceToId (*relInstance->GetSource()).GetValue ());
    ValidateReadingRelationship (db, "StartupCompany", "EmployeeFurniture", *relInstance);
    ValidateReadingRelated (db, "StartupCompany", "EmployeeFurniture", relInstance->GetSource(), relInstance->GetTarget());

    relInstance = CreateRelationship (test, "StartupCompany", "Employee", "Desk", "EmployeeFurniture");
    ASSERT_TRUE (relInstance.IsValid());
    PersistRelationship (*relInstance, db);
    ValidatePersistingRelationship (db, "sc_Furniture", InstanceToId (*relInstance->GetTarget()), 
        "Employee__src_01_id", InstanceToId (*relInstance->GetSource()).GetValue ());
    ValidateReadingRelationship (db, "StartupCompany", "EmployeeFurniture", *relInstance);
    ValidateReadingRelated (db, "StartupCompany", "EmployeeFurniture", relInstance->GetSource(), relInstance->GetTarget());

    // Import a M-to-1 relationship
    // EmployeeCompany
    //   Inherits: EmployeeRelationshipsBase
    //   Relates: Employee (M) -> Company (1)
    relInstance = CreateRelationship (test, "StartupCompany", "Employee", "Company", "EmployeeCompany");
    ASSERT_TRUE (relInstance.IsValid());
    PersistRelationship (*relInstance, db);
    ValidatePersistingRelationship (db, "sc_Employee", InstanceToId (*relInstance->GetSource()), 
        "Company__trg_11_id", InstanceToId (*relInstance->GetTarget()).GetValue ());
    ValidateReadingRelationship (db, "StartupCompany", "EmployeeCompany", *relInstance);
    ValidateReadingRelated (db, "StartupCompany", "EmployeeCompany", relInstance->GetSource(), relInstance->GetTarget());

    // Import a M-to-M relationship
    // EmployeeHardware
    //   Inherits: AssetRelationshipsBase, EmployeeRelationshipsBase
    //   Relates: Employee (M) -> Hardware (M)
    db.SaveChanges();
    relInstance = CreateRelationship (test, "StartupCompany", "Employee", "Hardware", "EmployeeHardware");
    ASSERT_TRUE (relInstance.IsValid());
    PersistRelationship (*relInstance, db);
    db.SaveChanges();
    ValidatePersistingRelationship (db, "sc_EmployeeHardware", InstanceToId (*relInstance), 
        "Employee__src_0N_id", InstanceToId (*relInstance->GetSource()).GetValue ());
    ValidatePersistingRelationship (db, "sc_EmployeeHardware", InstanceToId (*relInstance), 
        "Hardware__trg_0N_id", InstanceToId (*relInstance->GetTarget()).GetValue ());

    ValidateReadingRelationship (db, "StartupCompany", "EmployeeHardware", *relInstance);
    ValidateReadingRelated (db, "StartupCompany", "EmployeeHardware", relInstance->GetSource(), relInstance->GetTarget());

    db.SaveChanges();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Umer Sufyan                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbRelationships, AmbiguousJoin)
{
    const auto perClassRowCount = 0;
    ECDbTestProject testProj;
    auto& ecdb = testProj.Create("ecdb.ecdbAmbiguousJoin", L"Computers.01.00.ecschema.xml", perClassRowCount);
    ECSqlStatement stmt;

    auto stat = stmt.Prepare(ecdb, "SELECT * FROM TR.Laptop JOIN TR.Laptop USING TR.LaptopHasLaptop FORWARD");
   
    ASSERT_NE(ECSqlStatus::Success, stat);
    stmt.Finalize();
    ecdb.CloseDb();
}
//-------------------------------------------------------------------------------------
// @bsimethod                                   Umer Sufyan                   08/14
//+---------------+---------------+---------------+---------------+---------------+----
TEST(ECDbRelationships, JoinTests)
{
    const auto perClassRowCount = 0;
    ECDbTestProject testProj;
    auto& ecdb = testProj.Create("ecdbJoinTests.ecdb", L"Computers.01.00.ecschema.xml", perClassRowCount);
    ECSqlStatement stmt;

    ECClassCP laptopClass = ecdb.Schemas().GetECClass("Computers", "Laptop");
    IECInstancePtr laptopInstance = laptopClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    SetStringValue(*laptopInstance, "OS", "Linux");
    InsertInstance(ecdb, *laptopClass, *laptopInstance);
    IECInstancePtr laptopInstance2 = laptopClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    SetStringValue(*laptopInstance2, "OS", "Windows");
    InsertInstance(ecdb, *laptopClass, *laptopInstance2);
    ECClassCP ramClass = ecdb.Schemas().GetECClass("Computers", "RAM");
    IECInstancePtr ramInstance = ramClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    SetStringValue(*ramInstance, "Size", "4");
    InsertInstance(ecdb, *ramClass, *ramInstance);
    IECInstancePtr ramInstance2 = ramClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    SetStringValue(*ramInstance2, "Size", "2");
    InsertInstance(ecdb, *ramClass, *ramInstance2);

    ECRelationshipClassCP laptopHasRam = ecdb.Schemas().GetECClass("Computers", "LaptopHasRam")->GetRelationshipClassCP();//1-1
    IECRelationshipInstancePtr rel1;
    rel1 = CreateRelationship(*laptopHasRam, *laptopInstance, *ramInstance);
    InsertInstance(ecdb, *laptopHasRam, *rel1);
    auto stat = stmt.Prepare(ecdb, "SELECT a.OS FROM TR.Laptop a LEFT JOIN TR.LaptopHasRam b ON a.ECInstanceId=b.SourceECInstanceId LEFT JOIN TR.RAM c ON c.ECInstanceId=b.TargetECInstanceId");
    ASSERT_EQ(ECSqlStatus::Success, stat);
    auto stepStat = stmt.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStat);
    Utf8CP val = stmt.GetValue(0).GetText();
    EXPECT_STREQ(val, "Linux");
    stepStat = stmt.Step();
    ASSERT_EQ(static_cast<int> (BE_SQLITE_ROW), static_cast<int> (stepStat));
    val = stmt.GetValue(0).GetText();
    EXPECT_STREQ(val, "Windows");
    stmt.Finalize();

    //test for right outer join
    //still not working support for right outer join work in progress
    auto prepareStat = stmt.Prepare(ecdb, "SELECT a.Size FROM TR.Laptop a RIGHT JOIN TR.LaptopHasRam b ON a.ECInstanceId=b.SourceECInstanceId RIGHT JOIN TR.RAM c ON c.ECInstanceId=b.TargetECInstanceId");
    ASSERT_EQ(ECSqlStatus::InvalidECSql, prepareStat);
    stmt.Finalize();
    ecdb.CloseDb();
    }

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  12/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbRelationships, TestRelationshipKeys)
    {
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create("relationshipKeystest.ecdb", L"UserWorkBench.01.00.ecschema.xml", true);
    ecdbr.ClearECDbCache();
    ECSchemaCP ecSchema = ecdbr.Schemas().GetECSchema("UserWorkBench", true);
    ASSERT_TRUE(ecSchema != NULL);

    ECRelationshipClassCP areaTown = ecSchema->GetClassCP("area_town")->GetRelationshipClassCP();
    for (auto constraintClass : areaTown->GetSource().GetConstraintClasses())
        {
        auto keys = constraintClass->GetKeys();
        ASSERT_EQ(0, keys.size());
        }
    for (auto constraintClass : areaTown->GetTarget().GetConstraintClasses())
        {
        auto keys = constraintClass->GetKeys();
        ASSERT_EQ(1, keys.size());
        ASSERT_TRUE(std::find(keys.begin(), keys.end(), "area_id") != keys.end());
        }
    ECRelationshipClassCP countryContinent = ecSchema->GetClassCP("country_continent")->GetRelationshipClassCP();
    for (auto constraintClass : countryContinent->GetSource().GetConstraintClasses())
        {
        Utf8String key = constraintClass->GetKeys().at(0);
        ASSERT_TRUE(key.Equals("country_id"));
        }
    for (auto constraintClass : countryContinent->GetTarget().GetConstraintClasses())
        {
        Utf8String key = constraintClass->GetKeys().at(0);
        ASSERT_TRUE(key.Equals("continent_id"));
        }
    ECRelationshipClassCP houseUser = ecSchema->GetClassCP("house_user")->GetRelationshipClassCP();
    for (auto constraintClass : houseUser->GetSource().GetConstraintClasses())
        {
        auto keys = constraintClass->GetKeys();
        ASSERT_EQ(0, keys.size());
        }
    for (auto constraintClass : houseUser->GetTarget().GetConstraintClasses())
        {
        auto keys = constraintClass->GetKeys();
        ASSERT_EQ(0, keys.size());
        }
    }

TEST(ECDbRelationships, ECRelationshipContraintKeyProperties)
    {
    const auto perClassRowCount = 0;
    ECDbTestProject testProj;
    auto& ecdb = testProj.Create("ecdbKeysTests.ecdb", L"ECSqlTestKeys.01.00.ecschema.xml", perClassRowCount);
    ECSqlStatement stmt;

    auto ecsql = "INSERT INTO ecsqltestKeys.P (I) VALUES(123)";
    ECSqlStatement statement;
    auto stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";
    ECInstanceKey instanceKey;
    auto stepStatus = statement.Step(instanceKey);
    ASSERT_EQ((int)BE_SQLITE_DONE, (int)stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();

    ecsql = "INSERT INTO ecsqltestKeys.PSA (I) VALUES(?)";
    stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";
    statement.BindId(1, instanceKey.GetECInstanceId());

    stepStatus = statement.Step();
    ASSERT_EQ((int)BE_SQLITE_DONE, (int)stepStatus) << "Step for '" << ecsql << "' failed";
    statement.Finalize();

    ecsql = "SELECT * FROM ecsqltestKeys.PSAHasPKey_N1 ";
    stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";
    ASSERT_TRUE(BE_SQLITE_ROW == statement.Step());
    ASSERT_TRUE(BE_SQLITE_DONE == statement.Step());
    statement.Finalize();

    ECClassCP pClass = ecdb.Schemas().GetECClass("ecsqltestKeys", "P",ResolveSchema::BySchemaNamespacePrefix);
    IECInstancePtr pInstance = pClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    InsertInstance(ecdb, *pClass, *pInstance);
    ECClassCP psaClass = ecdb.Schemas().GetECClass("ecsqltestKeys", "PSA", ResolveSchema::BySchemaNamespacePrefix);
    IECInstancePtr psaInstance = psaClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    InsertInstance(ecdb, *psaClass, *psaInstance);
    auto PSAHasPKey_N1Class = ecdb.Schemas().GetECClass("ecsqltestKeys", "PSAHasPKey_N1", ResolveSchema::BySchemaNamespacePrefix)->GetRelationshipClassCP();
    IECInstancePtr PSAHasPKey_N1Instance = PSAHasPKey_N1Class->GetRelationshipClassCP()->GetDefaultStandaloneEnabler()->CreateInstance(0);
    auto relationshipInstance = CreateRelationship(*PSAHasPKey_N1Class, *psaInstance.get(),*pInstance.get());
    ASSERT_TRUE(relationshipInstance.IsValid());
    PersistRelationship(*relationshipInstance, ecdb);

    ecsql = "SELECT * FROM ecsqltestKeys.PSAHasPKey_N1 ";
    stat = statement.Prepare(ecdb, ecsql);
    ASSERT_EQ(ECSqlStatus::Success, stat) << "Preparation of '" << ecsql << "' failed";
    ASSERT_TRUE(BE_SQLITE_ROW == statement.Step());
    ASSERT_TRUE(BE_SQLITE_ROW == statement.Step());
    ASSERT_TRUE(BE_SQLITE_DONE == statement.Step());
    statement.Finalize();

    }

END_ECDBUNITTESTS_NAMESPACE

