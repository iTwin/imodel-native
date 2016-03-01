/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbReadTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct ReadTests : ECDbTestFixture {};
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
int CountInstancesOfClass (ECDbR db, ECClassCR ecClass, bool isPolymorphic)
    {
    Utf8String ecsql("SELECT count(*) FROM ");

    if (!isPolymorphic)
        ecsql.append("ONLY ");

    ecsql.append(ecClass.GetECSqlName());

    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare (db, ecsql.c_str ());
    if (stat != ECSqlStatus::Success)
        return -1;

    if (statement.Step() != BE_SQLITE_ROW)
        return -1;

    return statement.GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ReadTests, ReadPolymorphic)
    {
    ECDb& db = SetupECDb("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"), 3);

    /*
     * Test retrieval when parent and children are all in the same table (TablePerHierarchy)
     */
    ECClassCP furniture = db.Schemas().GetECClass("StartupCompany", "Furniture");
    ASSERT_TRUE (furniture != nullptr);
    int count = CountInstancesOfClass (db, *furniture, false);
    ASSERT_EQ (3, count);
    count = CountInstancesOfClass (db, *furniture, true);
    ASSERT_EQ (9, count);

    /*
     * Test retrieval when parent and children are all in different tables (TablePerClass)
     */
    ECClassCP hardware = db.Schemas().GetECClass("StartupCompany", "Hardware");
    ASSERT_TRUE (hardware != nullptr);
    count = CountInstancesOfClass (db, *hardware, false);
    ASSERT_EQ (3, count);
    count = CountInstancesOfClass (db, *hardware, true);
    ASSERT_EQ (9, count);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                   Ramanujam.Raman                  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool SetStringValue (IECInstanceR instance, Utf8CP propertyAccessor, Utf8CP stringValue)
    {
    ECValue ecValue;
    if (SUCCESS != ecValue.SetUtf8CP (stringValue, true))
        return false;
    if (ECObjectsStatus::Success != instance.SetValue (propertyAccessor, ecValue))
        return false;
    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                   Ramanujam.Raman                  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool SetIntValue (IECInstanceR instance, Utf8CP propertyAccessor, int intValue)
    {
    ECValue ecValue;
    if (SUCCESS != ecValue.SetInteger (intValue))
        return false;
    if (ECObjectsStatus::Success != instance.SetValue (propertyAccessor, ecValue))
        return false;
    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                   Ramanujam.Raman                  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool InsertInstance (ECDbR db, ECClassCR ecClass, IECInstanceR ecInstance)
    {
    ECInstanceInserter inserter (db, ecClass);
    if (!inserter.IsValid ())
        return false;

    auto insertStatus = inserter.Insert (ecInstance);
    return SUCCESS == insertStatus;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                   Ramanujam.Raman                  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr CreatePerson (ECClassCR ecClass, Utf8CP firstName, Utf8CP lastName)
    {
    IECInstancePtr ecInstance = ecClass.GetDefaultStandaloneEnabler()->CreateInstance(0);
    if (ecInstance.IsNull())
        return nullptr;
    if (!SetStringValue (*ecInstance, "FirstName", firstName))
        return nullptr;
    if (!SetStringValue (*ecInstance, "LastName", lastName))
        return nullptr;
    return ecInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                   Ramanujam.Raman                  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ReadTests, OrderBy)
    {
    // Create StartupCompany 
    ECDbR db = SetupECDb("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"));

    // Add some employees
    ECClassCP employeeClass = db.Schemas().GetECClass("StartupCompany", "Employee");
    IECInstancePtr employee = CreatePerson(*employeeClass, "Leonardo", "Da Vinci");
    InsertInstance(db, *employeeClass, *employee);
    employee = CreatePerson(*employeeClass, "Galileo", "Galilei");
    InsertInstance(db, *employeeClass, *employee);
    employee = CreatePerson(*employeeClass, "Nikola", "Tesla");
    InsertInstance(db, *employeeClass, *employee);
    employee = CreatePerson(*employeeClass, "Niels", "Bohr");
    InsertInstance(db, *employeeClass, *employee);
    employee = CreatePerson(*employeeClass, "Albert", "Einstein");
    InsertInstance(db, *employeeClass, *employee);
    employee = CreatePerson(*employeeClass, "Albert", "Einstein");
    InsertInstance(db, *employeeClass, *employee);
    employee = CreatePerson(*employeeClass, "Srinivasa", "Ramanujan");
    InsertInstance(db, *employeeClass, *employee);
    db.SaveChanges();

    // Retrieve them in alphabetical order
    Utf8String ecsql("SELECT FirstName,LastName FROM ");
    ecsql.append(employeeClass->GetECSqlName()).append(" ORDER BY LastName, FirstName");
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare(db, ecsql.c_str());
    ASSERT_EQ(ECSqlStatus::Success, stat);

    // Just log for a manual check
    Utf8CP firstName, lastName;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        firstName = statement.GetValueText(0);
        lastName = statement.GetValueText(1);
        LOG.debugv("%s, %s", lastName, firstName);
        }

    // Validate the first few entries
    statement.Reset();
    DbResult stepStat = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStat);
    lastName = statement.GetValueText(1);
    ASSERT_EQ(0, strcmp(lastName, "Bohr"));

    stepStat = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStat);
    lastName = statement.GetValueText(1);
    ASSERT_EQ(0, strcmp(lastName, "Da Vinci"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                   Ramanujam.Raman                  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ReadTests, LimitOffset)
    {
    ECDbR db = SetupECDb ("StartupCompany.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"));

    // Populate 100 instances
    ECClassCP ecClass = db.Schemas().GetECClass("StartupCompany", "ClassWithPrimitiveProperties");
    IECInstancePtr ecInstance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    ECInstanceInserter inserter (db, *ecClass);
    ASSERT_TRUE (inserter.IsValid ());
    for (int ii = 0; ii < 100; ii++)
        {
        bool instanceStatus = SetIntValue (*ecInstance, "intProp", ii);
        ASSERT_TRUE (instanceStatus);
        ECInstanceKey instanceKey;
        ASSERT_EQ(SUCCESS, inserter.Insert (instanceKey, *ecInstance));
        }
    db.SaveChanges();

    // Setup query for a page of instances
    Utf8String ecsql("SELECT intProp FROM ");
    ecsql.append(ecClass->GetECSqlName()).append(" LIMIT :pageSize OFFSET :pageSize * (:pageNumber - 1)");
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare (db, ecsql.c_str ())) << ecsql.c_str();
    int pageSizeIndex = statement.GetParameterIndex ("pageSize");
    ASSERT_TRUE (pageSizeIndex >= 0);
    int pageNumberIndex = statement.GetParameterIndex ("pageNumber");
    ASSERT_TRUE (pageNumberIndex >= 0);

    // Start getting the 5th page, where each page is 10 instances
    int pageSize = 10;
    int pageNumber = 5;
    statement.BindInt (pageSizeIndex, pageSize);
    statement.BindInt (pageNumberIndex, pageNumber);

    // Verify the first result
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    int actualValue = statement.GetValueInt (0);
    int expectedValue = pageSize * (pageNumber - 1);
    ASSERT_EQ (actualValue, expectedValue);
    }

TEST_F(ReadTests, WriteCalculatedECProperty)
{
    ECDbR db = SetupECDb("SimpleCompany.ecdb", BeFileName(L"SimpleCompany.01.00.ecschema.xml"));
    Utf8String instanceXml = "<Manager xmlns = \"SimpleCompany.01.00\" >"
        "<FirstName>StRiNg - 10002</FirstName>"
        "<LastName>StRiNg - 10002</LastName>"
        "<EmployeeId>10002</EmployeeId>"
        "<LatestEducation>"
        "<Degree>StRiNg - 10007</Degree>"
        "<Year>10010</Year>"
        "</LatestEducation>"
        "<EducationHistory>"
        "<EducationStruct>"
        "<Degree>StRiNg - 10008</Degree>"
        "<GPA>10004.5</GPA>"
        "</EducationStruct>"
        "<EducationStruct>"
        "<Degree>StRiNg - 10009</Degree>"
        "<Year>10011</Year>"
        "<GPA>10005</GPA>"
        "</EducationStruct>"
        "<EducationStruct>"
        "<Year>10012</Year>"
        "<GPA>10005.5</GPA>"
        "</EducationStruct>"
        "</EducationHistory>"
        "<FullName>StRiNg - 10002 StRiNg - 10002</FullName>"
        "</Manager>";
    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(db.GetSchemaLocater());
    ECSchemaPtr schema;
    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext(*schemaReadContext, *schema, &schema);
    IECInstancePtr instance;
    InstanceReadStatus status= IECInstance::ReadFromXmlString(instance, instanceXml.c_str(), *instanceContext);
    ASSERT_TRUE(InstanceReadStatus::Success==status);

    Savepoint s(db, "Populate");
    auto &ecClass = instance->GetClass();
    ECInstanceInserter inserter(db, ecClass);
    ECInstanceKey id;
    auto insertStatus = inserter.Insert(id, *(instance.get()));
    ASSERT_TRUE(insertStatus == BentleyStatus::SUCCESS);
    instance->SetInstanceId("");
    insertStatus = inserter.Insert(id, *(instance.get()));
    ASSERT_TRUE(insertStatus == BentleyStatus::SUCCESS);
    instance->SetInstanceId("");
    insertStatus = inserter.Insert(id, *(instance.get()));
    ASSERT_TRUE(insertStatus == BentleyStatus::SUCCESS);
    s.Commit();
    db.SaveChanges();

    SqlPrintfString ecSql("SELECT * FROM ONLY [%s].[%s]", Utf8String(ecClass.GetSchema().GetName()).c_str(), Utf8String(ecClass.GetName()).c_str());
    ECSqlStatement ecStatement;
    ECSqlStatus prepareStatus = ecStatement.Prepare(db, ecSql.GetUtf8CP());
    ASSERT_TRUE(ECSqlStatus::Success == prepareStatus);
    bvector<IECInstancePtr> instances;
    ECInstanceECSqlSelectAdapter adapter(ecStatement);
    while (BE_SQLITE_ROW == ecStatement.Step())
    {
        IECInstancePtr newInstance = adapter.GetInstance();
        ASSERT_TRUE(ECDbTestUtility::CompareECInstances(*newInstance, *instance));
    }
}

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ReadTests, CreateECDbWithArbitraryNumberOfECInstances)
{
    ECDbR ecdbr = SetupECDb("ecdbWithArbitratyInstances.ecdb", BeFileName(L"SimpleCompany.01.00.ecschema.xml"), 3);

    ECSchemaCP ecschemap = ecdbr. Schemas ().GetECSchema ("SimpleCompany", true);
    ASSERT_TRUE (ecschemap != nullptr);
    ECClassCP employee = ecschemap->GetClassCP("Employee");
    ASSERT_TRUE(employee != NULL);
    int count = CountInstancesOfClass(ecdbr, *employee, false);
    ASSERT_EQ(3, count);
    count = CountInstancesOfClass(ecdbr, *employee, true);
    ASSERT_EQ(2*3, count);
}

END_ECDBUNITTESTS_NAMESPACE
