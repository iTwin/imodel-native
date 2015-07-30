/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbBrownBag_Demo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECDb/ECDbApi.h>
#include <UnitTests/BackDoor/ECDb/ECDbTests.h>

using namespace BentleyApi::ECN;

BEGIN_ECDBUNITTESTS_NAMESPACE
BeFileName GetAssetsDirectory ();
BeFileName GetTempDirectory ();

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbBrownBagDemo, CreateECDbFromScratch)
    {
    //** ECDb Initialization (once per process)
    BeFileName tempDirectory = GetTempDirectory ();
    BeFileName assetsDirectory = GetAssetsDirectory ();
    ECDb::Initialize (tempDirectory, &assetsDirectory);

    //** Create new ECDb file
    ECDb ecdb;
    DbResult stat = ecdb.CreateNewDb ("C:\\temp\\brownbag\\StartupCompany_empty.ecdb");
    if (stat != BE_SQLITE_OK)
        {
        wprintf (L"Creating ECDb failed.");
        return;
        }

    //** Read ECSchema from XML file
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext ();
    schemaContext->AddSchemaLocater (ecdb. GetSchemaLocater ());
    //if you have referenced schema in other places, add them here
    schemaContext->AddSchemaPath (L"C:\\temp\\brownbag\\schemareferences\\");

    ECSchemaPtr schema = nullptr;
    SchemaReadStatus deserializeStat = ECSchema::ReadFromXmlFile (schema, L"C:\\temp\\brownbag\\StartupCompany.03.00.ecschema.xml", *schemaContext);
    if (deserializeStat != SCHEMA_READ_STATUS_Success)
        {
        wprintf (L"Deserializing ECSchema from XML file failed.");
        return;
        }

    //** Import ECSchema into ECDb file
    BentleyStatus importStat = ecdb. Schemas ().ImportECSchemas (schemaContext->GetCache ());
    if (importStat != SUCCESS)
        {
        wprintf (L"Importing ECSchema failed.");
        return;
        }

    //** Save changes to ECDb file
    ecdb.SaveChanges ();
    //** Close ECDb file
    ecdb.CloseDb ();

    //*** Note: An ECDb file is self-contained. All schemas and their references are stored in the ECDb file. 
    //So after the schema import is done, the ECSchema object deserialized from XML is no longer needed.
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbBrownBagDemo, SchemaManager)
    {
    //** ECDb Initialization (once per process)
    BeFileName tempDirectory = GetTempDirectory ();
    BeFileName assetsDirectory = GetAssetsDirectory ();
    ECDb::Initialize (tempDirectory, &assetsDirectory);

    //** Opening ECDb file
    ECDb ecdb;
    DbResult stat = ecdb.OpenBeSQLiteDb ("C:\\temp\\brownbag\\StartupCompany_empty.ecdb", ECDb::OpenParams (ECDb::OpenMode::Readonly));
    if (stat == BE_SQLITE_OK)
        printf ("ECDb file opened.");
    else
        {
        printf ("Opening ECDb failed.");
        return;
        }

    ECDbSchemaManagerCR schemaManager = ecdb. Schemas ();

    //** Get ECSchema from ECDb file
    ECSchemaCP schema = schemaManager.GetECSchema ("StartupCompany");

    schema->DebugDump ();
    printf ("\r\n");

    //** Get ECClass from ECDb file
    ECClassCP employeeClass = schemaManager.GetECClass ("StartupCompany", "Employee");
    printf ("ECClass Employee\r\n");
    for (ECPropertyCP prop : employeeClass->GetProperties ())
        {
        printf ("\t%s\r\n", prop->GetName ().c_str ());
        }

    ecdb.CloseDb ();
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbBrownBagDemo, ECSqlSelect)
    {
    //** ECDb Initialization (once per process)
    BeFileName tempDirectory = GetTempDirectory ();
    BeFileName assetsDirectory = GetAssetsDirectory ();
    ECDb::Initialize (tempDirectory, &assetsDirectory);

    //** Opening ECDb file
    ECDb ecdb;
    if (BE_SQLITE_OK == ecdb.OpenBeSQLiteDb ("C:\\temp\\brownbag\\StartupCompany.ecdb", ECDb::OpenParams (ECDb::OpenMode::Readonly)))
        printf ("ECDb file '%s' opened.\r\n", ecdb.GetDbFileName ());
    else
        {
        printf ("Opening ECDb failed.\r\n");
        return;
        }

    //** Prepare ECSQL SELECT statement
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare (ecdb, "SELECT FirstName, LastName, Birthday FROM stco.Employee");
    if (stat == ECSqlStatus::Success)
        printf ("Prepared ECSQL statement.\r\n\r\n");
    else
        {
        printf ("Preparing ECSQL statement failed.\r\n");
        return;
        }

    //** Execute ECSQL SELECT statement
    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        // Column indices in result set are 0-based
        Utf8CP firstName = statement.GetValueText (0);
        Utf8CP lastName = statement.GetValueText (1);
        DateTime birthday = statement.GetValueDateTime (2);
        printf ("Name: %s %s\tBirthday: %s\r\n", 
               firstName, lastName, birthday.ToUtf8String ().c_str ());     
        }

    statement.Finalize ();
    ecdb.CloseDb ();
    }























//******* Not working yet in ECSQL API ************
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbBrownBagDemo, ECSqlSelectWithBinding)
    {
    //** ECDb Initialization (once per process)
    BeFileName tempDirectory = GetTempDirectory ();
    BeFileName assetsDirectory = GetAssetsDirectory ();
    ECDb::Initialize (tempDirectory, &assetsDirectory);

    //** Opening ECDb file
    ECDb ecdb;
    if (BE_SQLITE_OK != ecdb.OpenBeSQLiteDb ("C:\\temp\\brownbag\\StartupCompany.ecdb", ECDb::OpenParams (ECDb::OpenMode::Readonly)))
        {
        printf ("Opening ECDb failed.\r\n");
        return;
        }

    //** Prepare ECSQL SELECT statement
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare (ecdb, "SELECT AssetID, Make, Cost FROM stco.Computer WHERE Cost <= ?");
    if (stat != ECSqlStatus::Success)
        {
        auto errorMessage = statement.GetLastStatusMessage ();
        printf ("Preparing ECSQL statement failed: %s\r\n", errorMessage.c_str ());
        return;
        }

    //** Binding values to parameters
    statement.BindDouble (1, 800.0);

    //** Execute ECSQL SELECT statement
    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        // Column indices in result set are 0-based
        Utf8CP assetid = statement.GetValueText (0);
        Utf8CP make = statement.GetValueText (1);
        double cost = statement.GetValueDouble (2);
        printf ("Computer %s [Asset ID %s] - Cost: %6.2f\r\n", 
               make, assetid, cost);     
        }

    statement.Finalize ();
    ecdb.CloseDb ();
    }





IECInstancePtr InsertCompany (ECDbR ecdb, ECSchemaCP schema, Utf8CP name, Utf8CP street, Utf8CP city, Utf8CP state, Utf8CP zip, Utf8CP country, int numberOfEmployees);
IECInstancePtr InsertEmployee (ECDbR ecdb, ECSchemaCP schema, int id, Utf8CP firstName, Utf8CP lastName, DateTimeCR birthday, Utf8CP street, Utf8CP city, Utf8CP state, Utf8CP zip, Utf8CP country, IECInstanceR compInstance);
IECInstancePtr InsertDesk (ECDbR ecdb, ECSchemaCP schema, Utf8CP assetId, double cost, int material, double weight, double length, double breadth, IECInstanceR employeeInstance);
IECInstancePtr InsertComputer (ECDbR ecdb, ECSchemaCP schema, Utf8CP assetId, double cost, double weight, Utf8CP make, bool isCompanyProperty, DateTimeCR warrantyExpiry, bool isLaptop, bool hasDockingStation, IECInstanceR employeeInstance);

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbBrownBagDemo, PopulateSampleECDb)
    {
    //** ECDb Initialization (once per process)
    BeFileName tempDirectory = GetTempDirectory ();
    BeFileName assetsDirectory = GetAssetsDirectory ();
    ECDb::Initialize (tempDirectory, &assetsDirectory);

    //** Opening ECDb file
    ECDb ecdb;
    if (BE_SQLITE_OK != ecdb.OpenBeSQLiteDb ("C:\\temp\\brownbag\\StartupCompany.ecdb", ECDb::OpenParams (ECDb::OpenMode::ReadWrite)))
        {
        wprintf (L"Opening ECDb failed.");
        return;
        }

    ECSchemaCP schema = ecdb. Schemas ().GetECSchema ("StartupCompany", true);

    auto companyInst = InsertCompany (ecdb, schema, "Bentley Systems Inc.", "685 Stockton Drive", "Exton", "PA", "19341",
                                        "USA", 4000);
   
    auto john = InsertEmployee (ecdb, schema, 1, "John", "Smith", DateTime (1975, 4, 19), "331 Hill Drive", "Exton", "PA", "19341", "USA", *companyInst);
    auto bill = InsertEmployee (ecdb, schema, 2, "Bill", "Miller", DateTime (1967, 12, 31), "2000 Main Street", "Philadelphia", "PA", "19019", "USA", *companyInst);
    auto sandy = InsertEmployee (ecdb, schema, 3, "Sandy", "Driscoll", DateTime (1980, 1, 31), "231 Chester Street", "Exton", "PA", "19341", "USA", *companyInst);

    InsertDesk (ecdb, schema, "A-1", 340.0, 2, 35.3, 4.0, 2.5, *john);
    InsertDesk (ecdb, schema, "A-2", 299.0, 1, 25.4, 2.0, 1.5, *bill);
    InsertDesk (ecdb, schema, "A-3", 299.0, 1, 25.4, 2.0, 1.5, *sandy);

    InsertComputer (ecdb, schema, "A-4", 999.9, 2.0, "HP EliteBook 8540w", true, DateTime (2014, 3, 31), true, true, *john);
    InsertComputer (ecdb, schema, "A-5", 699.9, 10.0, "HP Z600 Workstation 2x", true, DateTime (2016, 12, 31), false, false, *bill);
    InsertComputer (ecdb, schema, "A-6", 699.9, 10.0, "HP Z600 Workstation 2x", true, DateTime (2016, 12, 31), false, false, *sandy);
    InsertComputer (ecdb, schema, "A-7", 899.9, 4.0, "HP Envy dv7", false, DateTime (2016, 4, 30), true, false, *sandy);

    ecdb.SaveChanges ();
    }

IECInstancePtr InsertCompany (ECDbR ecdb, ECSchemaCP schema, Utf8CP name, Utf8CP street, Utf8CP city, Utf8CP state, Utf8CP zip, Utf8CP country, int numberOfEmployees)
    {
    ECClassCP ecClass = schema->GetClassCP ("Company");

    auto companyInst = ecClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    EXPECT_EQ (ECOBJECTS_STATUS_Success, companyInst->SetValue ("Name", ECValue (name)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, companyInst->SetValue ("Headquarters.Street", ECValue (street)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, companyInst->SetValue ("Headquarters.City", ECValue (city)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, companyInst->SetValue ("Headquarters.State", ECValue (state)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, companyInst->SetValue ("Headquarters.Zip", ECValue (zip)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, companyInst->SetValue ("Headquarters.Country", ECValue (country)));

    EXPECT_EQ (ECOBJECTS_STATUS_Success, companyInst->SetValue ("NumberOfEmployees", ECValue (numberOfEmployees)));

    ECInstanceInserter inserter (ecdb, *ecClass);
    ECInstanceKey instanceKey;
    auto stat = inserter.Insert (instanceKey, *companyInst);
    EXPECT_EQ (SUCCESS, stat) << "Inserting company instance failed";
    return companyInst;
    }

IECInstancePtr InsertEmployee (ECDbR ecdb, ECSchemaCP schema, int id, Utf8CP firstName, Utf8CP lastName, DateTimeCR birthday, Utf8CP street, Utf8CP city, Utf8CP state, Utf8CP zip, Utf8CP country,
                      IECInstanceR compInst)
    {
    ECClassCP ecClass = schema->GetClassCP ("Employee");

    auto inst = ecClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("EmployeeID", ECValue (id)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("FirstName", ECValue (firstName)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("LastName", ECValue (lastName)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("Birthday", ECValue (birthday)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("Address.Street", ECValue (street)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("Address.City", ECValue (city)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("Address.State", ECValue (state)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("Address.Zip", ECValue (zip)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("Address.Country", ECValue (country)));

    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->AddArrayElements ("MobilePhones", 2));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("MobilePhones", ECValue ("+1-610-766-1324"), 0));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("MobilePhones", ECValue ("+1-610-766-6531"), 1));

    ECInstanceInserter inserter (ecdb, *ecClass);
    ECInstanceKey instanceKey;
    auto stat = inserter.Insert (instanceKey, *inst);
    EXPECT_EQ (SUCCESS, stat) << "Inserting Employee";

    //now insert relationship to company
    auto relClass = schema->GetClassCP ("EmployeeCompany")->GetRelationshipClassCP ();
    auto enabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    auto relInst = enabler->CreateRelationshipInstance ();
    relInst->SetSource (inst.get ());
    relInst->SetTarget (&compInst);
    ECInstanceInserter relInserter (ecdb, *relClass);
    ECInstanceKey relInstanceKey;
    stat = relInserter.Insert (relInstanceKey, *relInst);
    EXPECT_EQ (SUCCESS, stat) << "Inserting EmployeeCompany relationship";
    
    return inst;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
IECInstancePtr InsertDesk (ECDbR ecdb, ECSchemaCP schema, Utf8CP assetId, double cost, int material, double weight, double length, double breadth, IECInstanceR employeeInstance)
    {
    ECClassCP ecClass = schema->GetClassCP ("Desk");

    auto inst = ecClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("AssetID", ECValue (assetId)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("Cost", ECValue (cost)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("Material", ECValue (material)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("Weight", ECValue (weight)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("Length", ECValue (length)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("Breadth", ECValue (breadth)));

    ECInstanceInserter inserter (ecdb, *ecClass);
    ECInstanceKey instanceKey;
    auto stat = inserter.Insert (instanceKey, *inst);
    EXPECT_EQ (SUCCESS, stat) << "Inserting Desk";

    //now insert relationship to employee
    auto relClass = schema->GetClassCP ("EmployeeFurniture")->GetRelationshipClassCP ();
    auto enabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    auto relInst = enabler->CreateRelationshipInstance ();
    relInst->SetSource (&employeeInstance);
    relInst->SetTarget (inst.get ());
    ECInstanceInserter relInserter (ecdb, *relClass);
    ECInstanceKey relInstanceKey;
    stat = relInserter.Insert (relInstanceKey, *relInst);
    EXPECT_EQ (SUCCESS, stat) << "Inserting EmployeeFurniture relationship";

    return inst;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
IECInstancePtr InsertComputer (ECDbR ecdb, ECSchemaCP schema, Utf8CP assetId, double cost, double weight, Utf8CP make, bool isCompanyProperty, DateTimeCR warrantyExpiry, bool isLaptop, bool hasDockingStation, IECInstanceR employeeInstance)
    {
    ECClassCP ecClass = nullptr;
    if (isLaptop)
        ecClass = schema->GetClassCP ("Laptop");
    else
        ecClass = schema->GetClassCP ("Computer");

    auto inst = ecClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("AssetID", ECValue (assetId)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("Cost", ECValue (cost)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("Weight", ECValue (weight)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("Make", ECValue (make)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("IsCompanyProperty", ECValue (isCompanyProperty)));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("WarrantyExpiryDate", ECValue (warrantyExpiry)));
    if (isLaptop)
        EXPECT_EQ (ECOBJECTS_STATUS_Success, inst->SetValue ("HasDockingStation", ECValue (hasDockingStation)));

    ECInstanceInserter inserter (ecdb, *ecClass);
    ECInstanceKey instanceKey;
    auto stat = inserter.Insert (instanceKey, *inst);
    EXPECT_EQ (SUCCESS, stat) << "Inserting Computer / Laptop";

    //now insert relationship to employee
    auto relClass = schema->GetClassCP ("HardwareUsedByEmployee")->GetRelationshipClassCP ();
    auto enabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    auto relInst = enabler->CreateRelationshipInstance ();
    relInst->SetSource (&employeeInstance);
    relInst->SetTarget (inst.get ());
    ECInstanceInserter relInserter (ecdb, *relClass);
    ECInstanceKey relInstanceKey;
    stat = relInserter.Insert (relInstanceKey, *relInst);
    EXPECT_EQ (SUCCESS, stat) << "Inserting HardwareUsedByEmployee relationship";

    return inst;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
BeFileName GetTempDirectory ()
    {
    BeFileName tempDir;
    BeTest::GetHost ().GetTempDir (tempDir);

    return std::move (tempDir);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
BeFileName GetAssetsDirectory ()
    {
    //establish standard schema search paths (they are in the application dir)
    BeFileName applicationSchemaDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (applicationSchemaDir);

    return std::move (applicationSchemaDir);
    }

END_ECDBUNITTESTS_NAMESPACE