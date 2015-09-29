/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbSchemas_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <initializer_list>
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/12
+---------------+---------------+---------------+---------------+---------------+------*/
static int GetColumnCount(DbR db, Utf8CP table)
    {
    Statement stmt;
    stmt.Prepare(db, SqlPrintfString("SELECT * FROM %s LIMIT 1", table));
    return stmt.GetColumnCount();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteECSchemaDiffToLog (ECDiffR diff, NativeLogging::SEVERITY severity = NativeLogging::LOG_INFO)
    {
    Utf8String diffString;
    ASSERT_EQ (diff.WriteToString(diffString, 2), DIFFSTATUS_Success);
    LOG.message (severity,  "ECDiff: Legend [L] Added from left schema, [R] Added from right schema, [!] conflicting value");
    LOG.message (severity, "=====================================[ECDiff Start]=====================================");
    //LOG doesnt allow single large string
    Utf8String eol = "\r\n";
    Utf8String::size_type i = 0;
    Utf8String::size_type j = diffString.find (eol, i);
    while ( j > i && j != Utf8String::npos)
        {
        Utf8String line = diffString.substr (i, j - i);
        LOG.messagev (severity, "> %s" , line.c_str()); //print out the difference
        i = j + eol.size();
        j = diffString.find (eol, i);
        }
    LOG.message (severity, "=====================================[ECDiff End]=====================================");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                          04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PopulatePrimitiveValueWithCustomDataSet2 (ECValueR value, PrimitiveType primitiveType, ECPropertyCP ecProperty)
    {
    value.Clear();
    switch (primitiveType)
        {
        case PRIMITIVETYPE_String  : value.SetUtf8CP("Tim Cook"); break;
        case PRIMITIVETYPE_Integer : value.SetInteger(987); break;
        case PRIMITIVETYPE_Long    : value.SetLong(987654321); break;
        case PRIMITIVETYPE_Double  : value.SetDouble(PI*3);break;
        case PRIMITIVETYPE_DateTime: 
            {
            DateTime dt;
            DateTimeInfo dti;
            if (ecProperty != nullptr && StandardCustomAttributeHelper::GetDateTimeInfo (dti, *ecProperty) == ECOBJECTS_STATUS_Success)
                {
                DateTime::Info info = dti.GetInfo (true);
                if (info.GetKind () == DateTime::Kind::Local)
                    {
                    //local date times are not supported by ECObjects
                    break;
                    }

                DateTime::FromJulianDay (dt, 2456341.75, info); 
                }
            else
                {
                DateTime::FromJulianDay (dt, 2456341.75, DateTimeInfo::GetDefault ()); 
                }

            value.SetDateTime (dt); 
            break;
            }
        case PRIMITIVETYPE_Boolean : value.SetBoolean(false); break;
        case PRIMITIVETYPE_Binary  : 
            {
            Byte blob[]= {0x1a, 0x0a, 0x0c, 0x0c, 0x0e, 0x0e, 0x3a, 0xaa, 0xff, 0xb };
            value.SetBinary(blob, 10);
            break;
            }
        case PRIMITIVETYPE_Point2D : 
            {
            DPoint2d point2d;
            point2d.x=12.41;
            point2d.y=53.12;
            value.SetPoint2D(point2d);
            break;
            }
        case PRIMITIVETYPE_Point3D :
            {
            DPoint3d point3d;
            point3d.x=32.33;
            point3d.y=54.54;
            point3d.z=25.55;
            value.SetPoint3D(point3d);
            break;
            }
        }
    }    

ECSchemaReadContextPtr LocateECSchema (ECDbR ecDB, BeFileNameCR ecSchemaFile, ECSchemaPtr& ecSchema)
    {
    static  WCharCP ecSchemaExt = L".ecschema.xml";
    WString schemaFullName = ecSchemaFile.GetFileNameAndExtension ();
    size_t extPos = schemaFullName.length () - wcslen (ecSchemaExt);
    if (schemaFullName.substr (extPos).CompareToI (ecSchemaExt) == 0)
        {
        Utf8String schemaName;
        uint32_t schemaMajor, schemaMinor;
        if (ECSchema::ParseSchemaFullName (schemaName, schemaMajor, schemaMinor, Utf8String(schemaFullName.substr (0, extPos).c_str())) == ECOBJECTS_STATUS_Success)
            {
            ECSchemaReadContextPtr contextPtr = ECSchemaReadContext::CreateContext ();
            contextPtr->AddSchemaLocater (ecDB. GetSchemaLocater ());
            contextPtr->AddSchemaPath (ecSchemaFile.GetDirectoryName ().c_str ());
            auto sk = SchemaKey (schemaName.c_str (), schemaMajor, schemaMinor);
            ecSchema = contextPtr->LocateSchema (sk, SCHEMAMATCHTYPE_Identical);
            if (!ecSchema.IsNull ())
                return contextPtr;
            }
        }
    return nullptr;
    }

bool ImportECSchema (ECDbR ecDB, ECSchemaReadContextPtr contextPtr)
    {
    if (contextPtr.IsValid ())
        {
        BentleyStatus state = ecDB. Schemas ().ImportECSchemas (contextPtr->GetCache ());
        return (SUCCESS == state);
        }
    return false;
    }

static ECSchemaPtr importECSchema (ECDbR ecDB, BeFileNameCR ecSchemaFile)
    {
    ECSchemaPtr ecSchema;
    ECSchemaReadContextPtr context = LocateECSchema (ecDB, ecSchemaFile, ecSchema);
    if (context.IsNull ())
        {
        BeAssert (false && "cannot locate ECSchema XML file.");
        return nullptr;
        }

    if (!ImportECSchema (ecDB, context)) // Import schema if it's not already.
        {
        BeAssert (false && "cannot import ECSchema XML file.");
        return nullptr;
        }
    return ecSchema;
    }


TEST (ECDbSchemas, OrderOfPropertyIsPreservedInTableColumns)
    {
    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create ("propertyOrderTest.ecdb");
    auto schema =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<ECSchema schemaName=\"OrderSchema\" nameSpacePrefix=\"os\" version=\"1.0\" xmlns = \"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"OrderedStruct\" isDomainClass=\"True\" isStruct =\"True\">"
        "   <ECProperty propertyName=\"a\" typeName=\"string\"/>"
        "	 <ECProperty propertyName=\"g\" typeName=\"integer\"/>"
        "	 <ECProperty propertyName=\"c\" typeName=\"dateTime\"/>"
        "   <ECProperty propertyName=\"z\" typeName=\"point3d\"/>"
        "	 <ECProperty propertyName=\"y\" typeName=\"point2d\"/>"
        "	 <ECProperty propertyName=\"t\" typeName=\"boolean\"/>"
        "   <ECProperty propertyName=\"u\" typeName=\"double\"/>"
        "	 <ECProperty propertyName=\"k\" typeName=\"string\"/>"
        "	 <ECProperty propertyName=\"r\" typeName=\"string\"/>"
        "  </ECClass>"
        "  <ECClass typeName=\"PropertyOrderTest\" isDomainClass=\"True\" isStruct =\"True\">"
        "   <ECProperty propertyName=\"x\" typeName=\"string\"/>"
        "	 <ECProperty propertyName=\"h\" typeName=\"integer\"/>"
        "	 <ECProperty propertyName=\"i\" typeName=\"dateTime\"/>"
        "   <ECProperty propertyName=\"d\" typeName=\"point3d\"/>"
        "	 <ECProperty propertyName=\"u\" typeName=\"point2d\"/>"
        "	 <ECProperty propertyName=\"f\" typeName=\"boolean\"/>"
        "   <ECProperty propertyName=\"e\" typeName=\"double\"/>"
        "	 <ECProperty propertyName=\"p\" typeName=\"string\"/>"
        "	 <ECStructProperty propertyName=\"o\" typeName=\"OrderedStruct\"/>"
        "	 <ECProperty propertyName=\"z\" typeName=\"long\"/>"
        "  </ECClass>"
        "</ECSchema>";

    ECSchemaPtr orderSchema;
    auto readContext = ECSchemaReadContext::CreateContext ();
    ECSchema::ReadFromXmlString (orderSchema, schema, *readContext);
    ASSERT_TRUE (orderSchema != nullptr);
    auto importStatus = db. Schemas ().ImportECSchemas (readContext->GetCache ());
    ASSERT_TRUE (importStatus == BentleyStatus::SUCCESS);
    
    Statement stmt1;
    stmt1.Prepare (db, "PRAGMA table_info('os_ArrayOfPropertyOrderTest')");
    Utf8String order_PropertyOrderTest;
    while (stmt1.Step () == BE_SQLITE_ROW)
        {
        order_PropertyOrderTest.append (stmt1.GetValueText (1)).append (" ");
        }

    ASSERT_TRUE (order_PropertyOrderTest == "ECInstanceId ParentECInstanceId ECPropertyPathId ECArrayIndex x h i d_X d_Y d_Z u_X u_Y f e p o_a o_g o_c o_z_X o_z_Y o_z_Z o_y_X o_y_Y o_t o_u o_k o_r z ");
    
    Statement stmt2;
    stmt2.Prepare (db, "PRAGMA table_info('os_ArrayOfOrderedStruct')");
    Utf8String order_OrderedStruct;
    while (stmt2.Step () == BE_SQLITE_ROW)
        {
        order_OrderedStruct.append (stmt2.GetValueText (1)).append (" ");
        }

    ASSERT_TRUE (order_OrderedStruct == "ECInstanceId ParentECInstanceId ECPropertyPathId ECArrayIndex a g c z_X z_Y z_Z y_X y_Y t u k r "); 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ECDbSchemas, DWGRTest)
    {
    //DWGR19_L0.01.01.ecschema.xml
    //DWGR19_L1.01.01.ecschema.xml
    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create ("dwgr.ecdb");
    db.ClearECDbCache();
    BeFileName sourceDir;
    BeTest::GetHost ().GetDocumentsRoot (sourceDir);
    sourceDir.AppendToPath (L"DgnDb");
    sourceDir.AppendToPath (L"ECDb");
    sourceDir.AppendToPath (L"Schemas");

    BeFileName dwgr19_l0_file (nullptr, sourceDir.GetName (), L"DWGR19_L0.01.01.ecschema.xml", nullptr);
    BeFileName dwgr19_l1_file (nullptr, sourceDir.GetName (), L"DWGR19_L1.01.01.ecschema.xml", nullptr);

    auto dwgr19_l0 = importECSchema (db, dwgr19_l0_file);
    ASSERT_TRUE (dwgr19_l0 != nullptr);

    auto dwgr19_l1 = importECSchema (db, dwgr19_l1_file);
    ASSERT_TRUE (dwgr19_l1 != nullptr);

    WString dwgr19_l0_xml, dwgr19_l1_xml;
    dwgr19_l0->WriteToXmlString (dwgr19_l0_xml);
    dwgr19_l1->WriteToXmlString (dwgr19_l1_xml);

    ASSERT_TRUE (dwgr19_l0_xml.find (L"Category") != WString::npos);
    ASSERT_TRUE (dwgr19_l1_xml.find (L"Category") != WString::npos);



    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, UpdatingExistingECSchema)
    {
    // Save a test project
    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create ("RSComponents.ecdb", L"RSComponents.01.00.ecschema.xml", true);

    ECSchemaPtr modifiedECSchema;
    ECSchemaReadContextPtr schemaContext = nullptr;

    ECDbTestUtility::ReadECSchemaFromDisk (modifiedECSchema, schemaContext, L"RSComponents.02.00.ecschema.xml", nullptr);
    ASSERT_TRUE(modifiedECSchema.IsValid());
    modifiedECSchema->SetVersionMajor(1); //Major version should match
    modifiedECSchema->SetVersionMinor(1); //Minor version should be greater then existing schema minor version

    auto importSchemaStatus = db. Schemas ().ImportECSchemas (schemaContext->GetCache (), ECDbSchemaManager::ImportOptions (true, true));
    ASSERT_EQ (SUCCESS, importSchemaStatus);

    ECSchemaCP  updatedECSchema = db.Schemas().GetECSchema("RSComponents");

    ECDiffPtr diff = ECDiff::Diff(*updatedECSchema, *modifiedECSchema);
    ASSERT_EQ (diff->GetStatus() , DIFFSTATUS_Success);
    if (!diff->IsEmpty())
        {
        bmap<Utf8String, DiffNodeState> searchResults;
        diff->GetNodesState(searchResults, "*.ArrayInfo");
        if (!searchResults.empty())
            LOG.error("*** Feature missing : Array type property Maxoccurs and Minoccurs are not stored currently by ECDbSchemaManager");
        WriteECSchemaDiffToLog(*diff, NativeLogging::LOG_ERROR);
        ASSERT_TRUE(false && "There should be no difference between in memory and stored ECSchema after update");
        }
    //Read back schema and generate some new instance with additional properties
    //ECSchemaP storedSchema;
    //db.Schemas().GetECSchema(storedSchema, "RSComponents", true);
    for(auto ecClass : modifiedECSchema->GetClasses())
        {
        if (ecClass->GetRelationshipClassCP() || ecClass->GetIsStruct() || ecClass->GetIsCustomAttributeClass())
            continue; 

        ECInstanceInserter inserter (db, *ecClass);
        if (!inserter.IsValid ())
            {
            LOG.errorv("Failed to create ECInstanceInserter for %s", ecClass->GetName().c_str());
            }
        ASSERT_TRUE (inserter.IsValid ());

        for( int i=0; i<3; i++)
            {
            auto newInst = ECDbTestProject::CreateArbitraryECInstance (*ecClass, PopulatePrimitiveValueWithCustomDataSet2);
            ECInstanceKey instanceKey;
            auto insertStatus = inserter.Insert (instanceKey, *newInst);
            ASSERT_EQ (SUCCESS, insertStatus);    
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, UpdateExistingECSchemaWithNewProperties)
    {
    ECDbTestProject testProject;
    ECDbR db = testProject.Create("updateSchemaMinorVersion.ecdb");

    ECSchemaPtr schema12;
    ECSchema::CreateSchema(schema12, "TestSchema", 1, 2);
    schema12->SetNamespacePrefix("ts");
    schema12->SetDescription("Schema for testing upgrades");
    schema12->SetDisplayLabel("Test Schema");

    ECClassP widget12;
    schema12->CreateClass(widget12, "WIDGET");
    PrimitiveECPropertyP stringProp12;
    widget12->CreatePrimitiveProperty(stringProp12, "propA");

    auto schemaCache12 = ECSchemaCache::Create ();
    schemaCache12->AddSchema (*schema12);

    auto importSchemaStatus = db.Schemas().ImportECSchemas (*schemaCache12);
    ASSERT_EQ (SUCCESS, importSchemaStatus);

    Utf8String ecdbFileName = Utf8String(db.GetDbFileName ());
    db.CloseDb();
    db.OpenBeSQLiteDb(ecdbFileName.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));

    ECSchemaPtr schema11;
    ECSchema::CreateSchema(schema11, "TestSchema", 1, 1);
    schema11->SetNamespacePrefix("ts");
    schema11->SetDescription("Schema for testing upgrades");
    schema11->SetDisplayLabel("Test Schema");

    ECClassP widget11;
    schema11->CreateClass(widget11, "WIDGET");
    PrimitiveECPropertyP stringProp11;
    widget11->CreatePrimitiveProperty(stringProp11, "propA");

    auto schemaCache11 = ECSchemaCache::Create ();
    schemaCache11->AddSchema (*schema11);
    importSchemaStatus = db.Schemas ().ImportECSchemas (*schemaCache11, ECDbSchemaManager::ImportOptions (true, true));
    ASSERT_EQ(ERROR, importSchemaStatus);

    ECSchemaPtr schema13;
    ECSchema::CreateSchema(schema13, "TestSchema", 1, 3);
    schema13->SetNamespacePrefix("ts");
    schema13->SetDescription("Schema for testing upgrades");
    schema13->SetDisplayLabel("Test Schema");

    ECClassP widget13;
    ECClassP gadget13;
    schema13->CreateClass(widget13, "WIDGET");
    schema13->CreateClass(gadget13, "GADGET");
    PrimitiveECPropertyP stringProp13;
    PrimitiveECPropertyP intProp13;
    widget13->CreatePrimitiveProperty(stringProp13, "propA");
    widget13->CreatePrimitiveProperty(intProp13, "propB");

    auto schemaCache13 = ECSchemaCache::Create ();
    schemaCache13->AddSchema (*schema13);
    importSchemaStatus = db. Schemas ().ImportECSchemas (*schemaCache13, ECDbSchemaManager::ImportOptions (true, true));
    ASSERT_EQ(SUCCESS, importSchemaStatus);
    db.CloseDb();
    db.OpenBeSQLiteDb(ecdbFileName.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));

    ECSchemaCP updatedECSchema = db. Schemas ().GetECSchema ("TestSchema");
    ASSERT_TRUE (updatedECSchema != nullptr);

    ECClassCP updatedGadget = updatedECSchema->GetClassCP("GADGET");
    ASSERT_TRUE(nullptr != updatedGadget);
    ECClassCP updatedWidget = updatedECSchema->GetClassCP("WIDGET");
    ASSERT_TRUE(nullptr != updatedWidget);
    //ECPropertyP pProperty = updatedWidget->GetPropertyP(L"propB");
    //ASSERT_TRUE(nullptr != pProperty);
    }

//---------------------------------------------------------------------------------------
// @bsimethods                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String CopyOldProfileTestFileEx (Utf8CP fileName)
    {
    WString fileNameW (fileName, BentleyCharEncoding::Utf8);

    BeFileName sourceDir;
    BeTest::GetHost ().GetDocumentsRoot (sourceDir);
    sourceDir.AppendToPath (L"DgnDb");
    sourceDir.AppendToPath (L"ECDb");

    BeFileName targetDir;
    BeTest::GetHost ().GetOutputRoot (targetDir);

    BeFileName sourcePath (nullptr, sourceDir.GetName (), fileNameW.c_str (), nullptr);
    BeFileName targetPath (nullptr, targetDir.GetName (), fileNameW.c_str (), nullptr);

    if (BeFileNameStatus::Success != BeFileName::BeCopyFile (sourcePath.GetName (), targetPath.GetName (), false))
        {
        return nullptr;
        }

    return Utf8String (targetPath.GetNameUtf8 ());
    }

//---------------------------------------------------------------------------------------
// @bsimethods                                     Affan.Khan                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemas, SqliteIssue)
    {
    // Save a test project
    ECDbTestProject saveTestProject;
    saveTestProject.Create ("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", false);

    // Reopen the test project
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb (saveTestProject.GetECDb ().GetDbFileName (), Db::OpenParams (Db::OpenMode::Readonly));
    EXPECT_EQ (BE_SQLITE_OK, stat);

    auto sql = "SELECT [Element].[ElementId] FROM (SELECT NULL ECClassId, NULL ECInstanceId, NULL [ElementId] LIMIT 0) Element ";
    // Validate the expected ECSchemas in the project
    Statement stmt;
    stmt.Prepare (db, sql);
    stmt.Step ();
    }

//---------------------------------------------------------------------------------------
// @bsimethods                                     Affan.Khan                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbSchemas, UpdatingSchemaShouldNotDeleteExistingRelationshipsOrIndexes)
    {
    ECDbTestProject::Initialize ();
    Utf8String ecdbPath = CopyOldProfileTestFileEx ("ecschema_upgrade.ecdb");

    ECDbTestProject testProject;
    testProject.Open (ecdbPath.c_str(), Db::OpenParams (Db::OpenMode::ReadWrite));

    ECDbR ecDb = testProject.GetECDb ();

    BeFileName schemaDir;
    BeTest::GetHost ().GetDocumentsRoot (schemaDir);
    schemaDir.AppendToPath (L"DgnDb");
    schemaDir.AppendToPath (L"ECDb");
    schemaDir.AppendToPath (L"Schemas");
    BeFileName dsCacheSchema1_3 (nullptr, schemaDir.GetName (), L"DSCacheSchema.01.03.ecschema.xml", nullptr);

    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schemaOut;
    ECSchema::ReadFromXmlFile (schemaOut, dsCacheSchema1_3.GetName (), *readContext);
    ECSchemaCacheR  schemaCache = readContext->GetCache ();

    auto status = ecDb. Schemas ().ImportECSchemas (schemaCache, ECDbSchemaManager::ImportOptions (true, true));
    ASSERT_EQ (SUCCESS, status);


    ecDb.SaveChanges ();
    ASSERT_EQ (ecDb.ColumnExists("DSC_CachedFileInfo", "ForeignECClassId_CachedFileInfoRelationship"), true);
    ASSERT_EQ (ecDb.ColumnExists("DSC_CachedFileInfo", "ForeignECInstanceId_CachedFileInfoRelationship"), true);

    ASSERT_EQ (ecDb.ColumnExists("DSC_CachedInstanceInfo", "ForeignECInstanceId_CachedInstanceInfoRelationship"), true);
    ASSERT_EQ (ecDb.ColumnExists("DSC_CachedInstanceInfo", "ForeignECClassId_CachedInstanceInfoRelationship"), true);

    ASSERT_EQ (ecDb.ColumnExists("DSCJS_RootRelationship", "SourceECInstanceId"), true);
    ASSERT_EQ (ecDb.ColumnExists("DSCJS_RootRelationship", "TargetECInstanceId"), true);
    ASSERT_EQ (ecDb.ColumnExists("DSCJS_RootRelationship", "TargetECClassId"), true);

    ASSERT_EQ (ecDb.ColumnExists("DSCJS_NavigationBaseRelationship", "SourceECInstanceId"), true);
    ASSERT_EQ (ecDb.ColumnExists("DSCJS_NavigationBaseRelationship", "TargetECInstanceId"), true);
    ASSERT_EQ (ecDb.ColumnExists("DSCJS_NavigationBaseRelationship", "TargetECClassId"), true);

    auto ecsql = "SELECT s.* FROM ONLY [DSC].[CachedInstanceInfo] s JOIN ONLY [DSC].[NavigationBase] t USING [DSCJS].[CachedInstanceInfoRelationship] FORWARD WHERE t.ECInstanceId = 8 LIMIT 1";

    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare (ecDb, ecsql);
    ASSERT_TRUE(prepareStatus ==  ECSqlStatus::Success);
    auto stepStatus = stmt.Step ();
    ASSERT_TRUE (stepStatus == BE_SQLITE_ROW);
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, LoadECSchemas)
    {
    // Save a test project
    ECDbTestProject saveTestProject;
    saveTestProject.Create ("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", false);

    // Reopen the test project
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb (saveTestProject.GetECDb().GetDbFileName(), Db::OpenParams(Db::OpenMode::Readonly));
    EXPECT_EQ (BE_SQLITE_OK, stat);
   
    bset<Utf8String> expectedSchemas;
    expectedSchemas.insert ("Bentley_Standard_CustomAttributes");
    expectedSchemas.insert ("Bentley_Standard_Classes");
    expectedSchemas.insert ("EditorCustomAttributes");
    expectedSchemas.insert("ECDbMap");
    expectedSchemas.insert("ECDb_System");
    expectedSchemas.insert ("ECDb_FileInfo");
    expectedSchemas.insert ("StartupCompany");
    expectedSchemas.insert ("Unit_Attributes");

    // Validate the expected ECSchemas in the project
    Statement stmt;
    stmt.Prepare (db, "SELECT NAME FROM ec_Schema");
    int nSchemas = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        nSchemas++;
        Utf8String schemaName(stmt.GetValueText(0));
        if (expectedSchemas.end() == expectedSchemas.find(schemaName))
            LOG.errorv("Found unexpected ECSchema '%s'", schemaName.c_str());
        }
    EXPECT_EQ (expectedSchemas.size(), nSchemas);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                       10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void AssertParseECSql (ECDbCR ecdb, Utf8CP ecsql)
    {
    Utf8String parseTree;
    ASSERT_EQ(SUCCESS, ECSqlParseTreeFormatter::ParseAndFormatECSqlParseNodeTree(parseTree, ecdb, ecsql)) << "Failed to parse ECSQL";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                       10/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECSqlParseTest, ForAndroid)
    {
    ECDb ecdb; // only needed for issue listener, doesn't need to represent a file on disk
    AssertParseECSql (ecdb, "SELECT '''' FROM stco.Hardware");
    AssertParseECSql (ecdb, "SELECT 'aa', '''', b FROM stco.Hardware WHERE Name = 'a''b'");
    AssertParseECSql (ecdb, "SELECT _Aa, _bC, _123, Abc, a123, a_123, a_b, _a_b_c FROM stco.Hardware WHERE Name = 'Fusion'");
    AssertParseECSql (ecdb, "SELECT * FROM stco.Hardware WHERE Name = 'Fusion'");
    AssertParseECSql (ecdb, "SELECT [Foo].[Name] FROM stco.[Hardware] [Foo]");
    AssertParseECSql (ecdb, "SELECT [Foo].[Name] FROM stco.[Hardware] [Foo] WHERE [Name] = 'HelloWorld'");
    AssertParseECSql (ecdb, "Select EQUIP_NO From only appdw.Equipment where EQUIP_NO = '50E-101A' ");
    AssertParseECSql (ecdb, "INSERT INTO [V8TagsetDefinitions].[STRUCTURE_IL1] ([VarFixedStartZ], [DeviceID1], [ObjectType], [PlaceMethod], [CopyConstrDrwToProj]) VALUES ('?', '-E1-1', 'SGL', '1', 'Y')");
    AssertParseECSql (ecdb, "INSERT INTO [V8TagsetDefinitions].[grid__x0024__0__x0024__CB_1] ([CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457],[CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454]) VALUES ('', '1.1', '', '', '', '2.2', '', '', '', '2.5', '', '', '', '2.5', '', '', '', '2.1', '', '', '', 'E.3', '', '', '', 'B.4', '', '', '', 'D.4', '', '')");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                       10/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, VerifyEmptyECSchemaCannotBeRead)
    {
    ECDb ecdb;
    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    BeFileName assetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (assetsDir);
    ECDb::Initialize (temporaryDir, &assetsDir);

    DbResult stat = ecdb.CreateNewDb (nullptr);
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test ECDb file failed.";
    ECSchemaPtr emptySchema;
    ECSchema::CreateSchema(emptySchema,"EmptyECSchema", 1, 0);
    auto cache = ECSchemaCache::Create();
    cache->AddSchema (*emptySchema);
    auto schemaStat = ecdb.Schemas().ImportECSchemas(*cache);
    ASSERT_EQ (ERROR, schemaStat) << "Importing empty ECSchema succeeded unexpectedly";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                       03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, VerifyDatabaseSchemaAfterImport)
    {
    // Create a sample project
    ECDbTestProject test;
    ECDbR db = test.Create ("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", false);
    //========================[sc_ClassWithPrimitiveProperties===================================
   
    Utf8CP tblClassWithPrimitiveProperties = "sc_ClassWithPrimitiveProperties";
    EXPECT_TRUE (db.TableExists(tblClassWithPrimitiveProperties));
    EXPECT_TRUE (db.ColumnExists (tblClassWithPrimitiveProperties, "ECInstanceId"));
    EXPECT_EQ   (13, GetColumnCount(db, tblClassWithPrimitiveProperties));
    //Verify columns columns in this class is renamed to 
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_intProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_longProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_doubleProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_stringProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_dateTimeProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_binaryProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_booleanProp"));
    //point2Prop is stored as x,y 2 columns
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_point2dProp_X"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_point2dProp_Y"));
    //point3Prop is stored as x,y,z 3 columns
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_point3dProp_X"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_point3dProp_Y"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_point3dProp_Z"));

    //========================[sc_StructWithPrimitiveProperties==================================
    Utf8CP tblStructWithPrimitiveProperties = "sc_ArrayOfStructWithPrimitiveProperties";
    EXPECT_TRUE (db.TableExists(tblStructWithPrimitiveProperties));
    EXPECT_EQ   (16, GetColumnCount(db, tblStructWithPrimitiveProperties));
    ASSERT_TRUE(db.ColumnExists(tblStructWithPrimitiveProperties, "ECInstanceId"));
    ASSERT_TRUE(db.ColumnExists(tblStructWithPrimitiveProperties, "ParentECInstanceId"));
    EXPECT_TRUE(db.ColumnExists(tblStructWithPrimitiveProperties, "ECPropertyPathId"));
    EXPECT_TRUE(db.ColumnExists(tblStructWithPrimitiveProperties, "ECArrayIndex"));

    //Verify columns
    EXPECT_TRUE(db.ColumnExists(tblStructWithPrimitiveProperties, "intProp"));
    EXPECT_TRUE(db.ColumnExists(tblStructWithPrimitiveProperties, "longProp"));
    EXPECT_TRUE(db.ColumnExists(tblStructWithPrimitiveProperties, "doubleProp"));
    EXPECT_TRUE(db.ColumnExists(tblStructWithPrimitiveProperties, "stringProp"));
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveProperties, "dateTimeProp"));
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveProperties, "binaryProp"));
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveProperties, "booleanProp"));
    //point2Prop is stored as x,y 2 columns
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveProperties, "point2dProp_X"));
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveProperties, "point2dProp_Y"));
    //point3Prop is stored as x,y,z 3 columns
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveProperties, "point3dProp_X"));
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveProperties, "point3dProp_Y"));
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveProperties, "point3dProp_Z"));

    //========================[sc_ClassWithPrimitiveArrayProperties==============================
    //Array properties doesnt take any column currently it will take in case of embeded senario but
    //we need to make sure it doesnt exist right now. They uses special System arrray tables 
    Utf8CP tblClassWithPrimitiveArrayProperties = "sc_ClassWithPrimitiveArrayProperties";
    EXPECT_TRUE (db.TableExists(tblClassWithPrimitiveArrayProperties));
    EXPECT_EQ   (10, GetColumnCount(db, tblClassWithPrimitiveArrayProperties));    
    EXPECT_TRUE (db.ColumnExists(tblClassWithPrimitiveArrayProperties, "ECInstanceId"));

    //Verify columns
    EXPECT_TRUE (db.ColumnExists(tblClassWithPrimitiveArrayProperties, "intArrayProp"));
    EXPECT_TRUE (db.ColumnExists(tblClassWithPrimitiveArrayProperties, "longArrayProp"));
    EXPECT_TRUE (db.ColumnExists(tblClassWithPrimitiveArrayProperties, "doubleArrayProp"));
    EXPECT_TRUE  (db.ColumnExists(tblClassWithPrimitiveArrayProperties, "stringArrayProp"));// MapStrategy=Blob
    EXPECT_TRUE (db.ColumnExists(tblClassWithPrimitiveArrayProperties, "dateTimeArrayProp"));
    EXPECT_TRUE (db.ColumnExists(tblClassWithPrimitiveArrayProperties, "binaryArrayProp"));
    EXPECT_TRUE (db.ColumnExists(tblClassWithPrimitiveArrayProperties, "booleanArrayProp"));
    EXPECT_TRUE (db.ColumnExists(tblClassWithPrimitiveArrayProperties, "point2dArrayProp"));
    EXPECT_TRUE  (db.ColumnExists(tblClassWithPrimitiveArrayProperties, "point3dArrayProp")); // MapStrategy=Blob

    //========================[sc_StructWithPrimitiveArrayProperties=============================
    //Array properties doesnt have any column currently it will take in case of embeded senario but
    //we need to make sure it doesnt exist right now. They uses special System arrray tables 
    Utf8CP tblStructWithPrimitiveArrayProperties = "sc_ArrayOfStructWithPrimitiveArrayProperties";
    EXPECT_TRUE(db.TableExists(tblStructWithPrimitiveArrayProperties));
    EXPECT_EQ   (13, GetColumnCount(db, tblStructWithPrimitiveArrayProperties));    
    ASSERT_TRUE (db.ColumnExists(tblStructWithPrimitiveArrayProperties, "ECInstanceId"));
    ASSERT_TRUE (db.ColumnExists(tblStructWithPrimitiveArrayProperties, "ParentECInstanceId"));
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveArrayProperties, "ECPropertyPathId"));
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveArrayProperties, "ECArrayIndex"));

    //Verify columns
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveArrayProperties, "intArrayProp"));
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveArrayProperties, "longArrayProp"));
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveArrayProperties, "doubleArrayProp"));
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveArrayProperties, "stringArrayProp"));
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveArrayProperties, "dateTimeArrayProp"));
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveArrayProperties, "binaryArrayProp"));
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveArrayProperties, "booleanArrayProp"));
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveArrayProperties, "point2dArrayProp"));
    EXPECT_TRUE (db.ColumnExists(tblStructWithPrimitiveArrayProperties, "point3dArrayProp"));

    //verify system array tables. They are created if  a primitive array property is ecounter in schema

    //========================[TablePerHieracrchy Test]============================================
    // TablePerHierarchy Should have one table for all base class
    //========================[sc_Asset]=========================================================
    //baseClass
    Utf8CP tblAsset = "sc_Asset";
    EXPECT_TRUE (db.TableExists(tblAsset));    
    EXPECT_EQ   (8, GetColumnCount(db, tblAsset));            
    EXPECT_TRUE (db.ColumnExists(tblAsset, "ECInstanceId"));
    
    EXPECT_TRUE (db.ColumnExists(tblAsset, "AssetID"));
    EXPECT_TRUE (db.ColumnExists(tblAsset, "AssetOwner"));
    EXPECT_TRUE (db.ColumnExists(tblAsset, "BarCode"));
    EXPECT_TRUE (db.ColumnExists(tblAsset, "AssetUserID"));
    EXPECT_TRUE (db.ColumnExists(tblAsset, "Cost"));
    EXPECT_TRUE (db.ColumnExists(tblAsset, "Room"));
    EXPECT_TRUE (db.ColumnExists(tblAsset, "AssetRecordKey")); 
    
    //========================[sc_Furniture]=====================================================
    //TablePerHierarchy
    Utf8CP tblFurniture = "sc_Furniture"; //Drived from Asset
    EXPECT_TRUE (db.TableExists(tblFurniture));
    EXPECT_EQ   (21, GetColumnCount (db, tblFurniture));
    //Table for this child classes of Furniture should not exist    
    EXPECT_FALSE (db.TableExists("sc_Desk")); 
    EXPECT_FALSE (db.TableExists("sc_Chair"));
    
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "ECInstanceId"));
    //It must have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "ECClassId")); 
    
    //BaseClass properties
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "AssetID"));
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "AssetOwner"));
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "BarCode"));
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "AssetUserID"));
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "Cost"));
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "Room"));
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "AssetRecordKey")); 
    //Local properties of Furniture   
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "Condition")); 
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "Material")); 
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "Weight")); 
    // Properties of Chair which is derived from Furniture
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "ChairFootPrint"));
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "Type")); 
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "Color")); 
    
    // Properties of Desk which is derived from Furniture    
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "DeskFootPrint")); 
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "NumberOfCabinets")); 
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "Size")); 
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "Type")); 
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "Breadth")); 
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "Length")); 
    
    //relation key
    EXPECT_TRUE (db.ColumnExists(tblFurniture, "Employee__src_01_id")); 
    
    //========================[sc_Employee]======================================================
    //Related to Furniture. Employee can have one or more furniture
    Utf8CP tblEmployee = "sc_Employee";
    EXPECT_TRUE (db.TableExists(tblEmployee));    
    EXPECT_EQ   (31, GetColumnCount(db, tblEmployee));
    EXPECT_TRUE (db.ColumnExists(tblEmployee, "ECInstanceId"));
    
    EXPECT_TRUE (db.ColumnExists(tblEmployee, "EmployeeID"));
    EXPECT_TRUE (db.ColumnExists(tblEmployee, "FirstName"));
    EXPECT_TRUE (db.ColumnExists(tblEmployee, "JobTitle"));
    EXPECT_TRUE (db.ColumnExists(tblEmployee, "LastName"));
    EXPECT_TRUE (db.ColumnExists(tblEmployee, "ManagerID"));
    EXPECT_TRUE (db.ColumnExists(tblEmployee, "Room"));
    EXPECT_TRUE (db.ColumnExists(tblEmployee, "SSN")); 
    EXPECT_TRUE (db.ColumnExists(tblEmployee, "Project")); 
    EXPECT_TRUE (db.ColumnExists(tblEmployee, "FullName")); 
    EXPECT_TRUE (db.ColumnExists(tblEmployee, "EmployeeType")); 
    EXPECT_TRUE (db.ColumnExists(tblEmployee, "EmployeeRecordKey")); 
    EXPECT_TRUE (db.ColumnExists(tblEmployee, "Company__trg_11_id")); 
    EXPECT_FALSE(db.ColumnExists(tblEmployee, "EmployeeCertification")); //struct array property

    //========================[sc_Company]=======================================================
    Utf8CP tblCompany = "sc_Company";
    EXPECT_TRUE (db.TableExists(tblCompany));    
    EXPECT_EQ   (14, GetColumnCount(db, tblCompany));
    EXPECT_TRUE (db.ColumnExists(tblCompany, "ECInstanceId"));
    
    EXPECT_TRUE (db.ColumnExists(tblCompany, "Name"));
    EXPECT_TRUE (db.ColumnExists(tblCompany, "NumberOfEmployees"));
    EXPECT_TRUE (db.ColumnExists(tblCompany, "ContactAddress"));
    EXPECT_TRUE (db.ColumnExists(tblCompany, "RecordKey"));

    //========================[sc_EmployeeCertifications]========================================
    Utf8CP tblEmployeeCertification = "sc_ArrayOfEmployeeCertification";
    EXPECT_TRUE (db.TableExists(tblEmployeeCertification));    
    EXPECT_EQ   (9, GetColumnCount(db, tblEmployeeCertification));

    ASSERT_TRUE (db.ColumnExists(tblEmployeeCertification, "ECInstanceId"));
    ASSERT_TRUE (db.ColumnExists(tblEmployeeCertification, "ParentECInstanceId"));
    EXPECT_TRUE (db.ColumnExists(tblEmployeeCertification, "ECPropertyPathId"));
    EXPECT_TRUE (db.ColumnExists(tblEmployeeCertification, "ECArrayIndex"));
    
    EXPECT_TRUE (db.ColumnExists(tblEmployeeCertification, "Name"));
    EXPECT_TRUE (db.ColumnExists(tblEmployeeCertification, "StartDate"));
    EXPECT_TRUE (db.ColumnExists(tblEmployeeCertification, "ExpiryDate"));
    EXPECT_TRUE (db.ColumnExists(tblEmployeeCertification, "Technology"));
    EXPECT_TRUE (db.ColumnExists(tblEmployeeCertification, "Level"));
    
    //========================[sc_Hardware]======================================================
    //TablePerClass
    Utf8CP tblHardware = "sc_Hardware"; //Drived from Asset
    EXPECT_TRUE (db.TableExists(tblHardware));
    EXPECT_EQ   (14, GetColumnCount(db, tblHardware));            
    
    EXPECT_TRUE (db.ColumnExists(tblHardware, "ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblHardware, "ECClassId")); 
    
    //base class Asset properties
    EXPECT_TRUE (db.ColumnExists(tblHardware, "AssetID"));
    EXPECT_TRUE (db.ColumnExists(tblHardware, "AssetOwner"));
    EXPECT_TRUE (db.ColumnExists(tblHardware, "BarCode"));
    EXPECT_TRUE (db.ColumnExists(tblHardware, "AssetUserID"));
    EXPECT_TRUE (db.ColumnExists(tblHardware, "Cost"));
    EXPECT_TRUE (db.ColumnExists(tblHardware, "Room"));
    EXPECT_TRUE (db.ColumnExists(tblHardware, "AssetRecordKey")); 
    //Local properties of Hardware   
    EXPECT_TRUE (db.ColumnExists(tblHardware, "HasWarranty")); 
    EXPECT_TRUE (db.ColumnExists(tblHardware, "IsCompanyProperty")); 
    EXPECT_TRUE (db.ColumnExists(tblHardware, "Make")); 
    EXPECT_TRUE (db.ColumnExists(tblHardware, "Model")); 
    EXPECT_TRUE (db.ColumnExists(tblHardware, "WarrantyExpiryDate")); 
        
    //========================[sc_Computer]======================================================
    //TablePerClass
    Utf8CP tblComputer = "sc_Computer"; //Drived from Asset
    EXPECT_TRUE (db.TableExists(tblComputer));
    EXPECT_EQ   (17, GetColumnCount(db, tblComputer));            
    
    EXPECT_TRUE (db.ColumnExists(tblComputer, "ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblComputer, "ECClassId")); 
    
    //base class properties of Asset 
    EXPECT_TRUE (db.ColumnExists(tblComputer, "AssetID"));
    EXPECT_TRUE (db.ColumnExists(tblComputer, "AssetOwner"));
    EXPECT_TRUE (db.ColumnExists(tblComputer, "BarCode"));
    EXPECT_TRUE (db.ColumnExists(tblComputer, "AssetUserID"));
    EXPECT_TRUE (db.ColumnExists(tblComputer, "Cost"));
    EXPECT_TRUE (db.ColumnExists(tblComputer, "Room"));
    EXPECT_TRUE (db.ColumnExists(tblComputer, "AssetRecordKey")); 
    //base class properties of Hardware   
    EXPECT_TRUE (db.ColumnExists(tblComputer, "HasWarranty")); 
    EXPECT_TRUE (db.ColumnExists(tblComputer, "IsCompanyProperty")); 
    EXPECT_TRUE (db.ColumnExists(tblComputer, "Make")); 
    EXPECT_TRUE (db.ColumnExists(tblComputer, "Model")); 
    EXPECT_TRUE (db.ColumnExists(tblComputer, "WarrantyExpiryDate")); 
    //local properties
    EXPECT_TRUE (db.ColumnExists(tblComputer, "Vendor")); 
    EXPECT_TRUE (db.ColumnExists(tblComputer, "Weight")); 
    EXPECT_TRUE (db.ColumnExists(tblComputer, "Type")); 

    //========================[sc_Monitor]======================================================
    //TablePerClass
    Utf8CP tblMonitor = "sc_Monitor"; //Drived from Asset
    EXPECT_TRUE (db.TableExists(tblMonitor));
    EXPECT_EQ   (18, GetColumnCount(db, tblMonitor));            
    
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblMonitor, "ECClassId")); 
    
    //base class properties of Asset 
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "AssetID"));
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "AssetOwner"));
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "BarCode"));
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "AssetUserID"));
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "Cost"));
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "Room"));
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "AssetRecordKey")); 
    //base class properties of Hardware   
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "HasWarranty")); 
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "IsCompanyProperty")); 
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "Make")); 
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "Model")); 
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "WarrantyExpiryDate")); 
    //local properties
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "Size")); 
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "Type")); 
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "Vendor")); 
    EXPECT_TRUE (db.ColumnExists(tblMonitor, "Weight")); 
    
    //========================[sc_Phone]=========================================================
    //TablePerClass
    Utf8CP tblPhone = "sc_Phone"; //Drived from Asset
    EXPECT_TRUE (db.TableExists(tblPhone));
    EXPECT_EQ   (12, GetColumnCount(db, tblPhone));            
    
    EXPECT_TRUE (db.ColumnExists(tblPhone, "ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblPhone, "ECClassId")); 
    
    //base class properties of Asset 
    EXPECT_TRUE (db.ColumnExists(tblPhone, "AssetID"));
    EXPECT_TRUE (db.ColumnExists(tblPhone, "AssetOwner"));
    EXPECT_TRUE (db.ColumnExists(tblPhone, "BarCode"));
    EXPECT_TRUE (db.ColumnExists(tblPhone, "AssetUserID"));
    EXPECT_TRUE (db.ColumnExists(tblPhone, "Cost"));
    EXPECT_TRUE (db.ColumnExists(tblPhone, "Room"));
    EXPECT_TRUE (db.ColumnExists(tblPhone, "AssetRecordKey")); 
    //local properties
    EXPECT_TRUE (db.ColumnExists(tblPhone, "Number")); 
    EXPECT_TRUE (db.ColumnExists(tblPhone, "Owner")); 
    EXPECT_TRUE (db.ColumnExists(tblPhone, "User")); 
    
    //========================[sc_Widget]========================================================
    Utf8CP tblWidget = "sc_Widget"; 
    EXPECT_TRUE (db.TableExists(tblWidget));
    EXPECT_EQ   (3, GetColumnCount(db, tblWidget));            
    
    EXPECT_TRUE (db.ColumnExists(tblWidget, "ECInstanceId"));
    EXPECT_TRUE (db.ColumnExists(tblWidget, "stringOfWidget")); 
    
    //========================[sc_Project]=======================================================
    Utf8CP tblProject = "sc_Project"; 
    EXPECT_TRUE (db.TableExists(tblProject));
    EXPECT_EQ   (14, GetColumnCount(db, tblProject));            
    
    EXPECT_TRUE (db.ColumnExists(tblProject, "ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblProject, "ECClassId")); 

    EXPECT_TRUE (db.ColumnExists(tblProject, "CompletionDate")); 
    EXPECT_TRUE (db.ColumnExists(tblProject, "EstimatedCost")); 
    EXPECT_TRUE (db.ColumnExists(tblProject, "ProjectName")); 
    EXPECT_TRUE (db.ColumnExists(tblProject, "ProjectDescription")); 
    EXPECT_TRUE (db.ColumnExists(tblProject, "ProjectState")); 
    EXPECT_TRUE (db.ColumnExists(tblProject, "StartDate")); 
    EXPECT_TRUE (db.ColumnExists(tblProject, "InProgress")); 
    EXPECT_TRUE (db.ColumnExists(tblProject, "TeamSize")); 
    EXPECT_TRUE (db.ColumnExists(tblProject, "Logo")); 
    EXPECT_TRUE (db.ColumnExists(tblProject, "Manager")); 
    EXPECT_TRUE (db.ColumnExists(tblProject, "ProjectRecordKey")); 
    //struct/arrays mapped to table
    EXPECT_TRUE(db.ColumnExists(tblProject, "TeamMemberList"));  //int array
    //relation
    EXPECT_TRUE (db.ColumnExists(tblProject, "Company__src_11_id")); 
    
    //========================[sc_Building]======================================================
    Utf8CP tblBuilding = "sc_Building"; 
    EXPECT_TRUE (db.TableExists(tblBuilding));    
    EXPECT_EQ   (14, GetColumnCount(db, tblBuilding));
    
    EXPECT_TRUE (db.ColumnExists(tblBuilding, "ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblBuilding, "ECClassId")); 

    EXPECT_TRUE (db.ColumnExists(tblBuilding, "Number")); 
    EXPECT_TRUE (db.ColumnExists(tblBuilding, "Name")); 
    EXPECT_TRUE (db.ColumnExists(tblBuilding, "NumberOfFloors")); 
    EXPECT_TRUE (db.ColumnExists(tblBuilding, "BuildingCode")); 
    EXPECT_TRUE (db.ColumnExists(tblBuilding, "RecordKey")); 
    //struct array
    EXPECT_FALSE(db.ColumnExists(tblBuilding, "Location"));
    
    //========================[sc_Location]======================================================
    Utf8CP tblLocation = "sc_ArrayOfLocation"; 
    EXPECT_TRUE (db.TableExists(tblLocation));
    EXPECT_EQ   (12, GetColumnCount(db, tblLocation));            

    ASSERT_TRUE (db.ColumnExists(tblLocation, "ECInstanceId"));
    ASSERT_TRUE (db.ColumnExists(tblLocation, "ParentECInstanceId"));
    EXPECT_TRUE (db.ColumnExists(tblLocation, "ECPropertyPathId"));
    EXPECT_TRUE (db.ColumnExists(tblLocation, "ECArrayIndex"));

    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblLocation, "ECClassId")); 

    EXPECT_TRUE (db.ColumnExists(tblLocation, "Coordinate_X")); 
    EXPECT_TRUE (db.ColumnExists(tblLocation, "Coordinate_Y")); 
    EXPECT_TRUE (db.ColumnExists(tblLocation, "Coordinate_Z")); 
    EXPECT_TRUE (db.ColumnExists(tblLocation, "Street")); 
    EXPECT_TRUE (db.ColumnExists(tblLocation, "City")); 
    EXPECT_TRUE (db.ColumnExists(tblLocation, "State")); 
    EXPECT_TRUE (db.ColumnExists(tblLocation, "Country")); 
    EXPECT_TRUE (db.ColumnExists(tblLocation, "Zip")); 
    
    //========================[sc_BuildingFloor]=================================================
    Utf8CP tblBuildingFloor = "sc_BuildingFloor"; 
    EXPECT_TRUE (db.TableExists(tblBuildingFloor));
    EXPECT_EQ   (8, GetColumnCount(db, tblBuildingFloor));            
    
    EXPECT_TRUE (db.ColumnExists(tblBuildingFloor, "ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblBuildingFloor, "ECClassId")); 

    EXPECT_TRUE (db.ColumnExists(tblBuildingFloor, "FloorNumber")); 
    EXPECT_TRUE (db.ColumnExists(tblBuildingFloor, "BuildingCode")); 
    EXPECT_TRUE (db.ColumnExists(tblBuildingFloor, "NumberOfOffices")); 
    EXPECT_TRUE (db.ColumnExists(tblBuildingFloor, "Area")); 
    EXPECT_TRUE (db.ColumnExists(tblBuildingFloor, "FloorCode")); 
    EXPECT_TRUE (db.ColumnExists(tblBuildingFloor, "RecordKey")); 
    //relation
    EXPECT_TRUE (db.ColumnExists(tblBuildingFloor, "Building__src_11_Id")); 
    
    
//========================[sc_Cubicle]=================================================
    Utf8CP tblCubicle = "sc_Cubicle"; 
    EXPECT_TRUE (db.TableExists(tblCubicle));
    EXPECT_EQ   (13, GetColumnCount(db, tblCubicle));            
    
    EXPECT_TRUE (db.ColumnExists(tblCubicle, "ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblCubicle, "ECClassId")); 

    EXPECT_TRUE (db.ColumnExists(tblCubicle, "Bay")); 
    EXPECT_TRUE (db.ColumnExists(tblCubicle, "IsOccupied")); 
    EXPECT_TRUE (db.ColumnExists(tblCubicle, "BuildingFloor")); 
    EXPECT_TRUE (db.ColumnExists(tblCubicle, "Length")); 
    EXPECT_TRUE (db.ColumnExists(tblCubicle, "Breadth")); 
    EXPECT_TRUE (db.ColumnExists(tblCubicle, "NumberOfOccupants")); 
    EXPECT_TRUE (db.ColumnExists(tblCubicle, "BuildingCode")); 
    EXPECT_TRUE (db.ColumnExists(tblCubicle, "OfficeCode")); 
    EXPECT_TRUE (db.ColumnExists(tblCubicle, "Area")); 
    EXPECT_TRUE (db.ColumnExists(tblCubicle, "RecordKey"));     
    //array    
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "OccupiedBy"));         
    //relation
    EXPECT_TRUE (db.ColumnExists(tblCubicle, "BuildingFloor__src_11_id")); 
        
    //========================[sc_AnglesStruct]======================================================
    Utf8CP tblAnglesStruct = "sc_ArrayOfAnglesStruct"; 
    EXPECT_TRUE (db.TableExists(tblAnglesStruct));
    EXPECT_EQ   (7, GetColumnCount(db, tblAnglesStruct));            

    ASSERT_TRUE (db.ColumnExists(tblAnglesStruct, "ECInstanceId"));
    ASSERT_TRUE (db.ColumnExists(tblAnglesStruct, "ParentECInstanceId"));
    EXPECT_TRUE (db.ColumnExists(tblAnglesStruct, "ECPropertyPathId"));
    EXPECT_TRUE (db.ColumnExists(tblAnglesStruct, "ECArrayIndex"));

    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblAnglesStruct, "ECClassId")); 

    EXPECT_TRUE (db.ColumnExists(tblAnglesStruct, "Alpha")); 
    EXPECT_TRUE (db.ColumnExists(tblAnglesStruct, "Beta")); 
    EXPECT_TRUE (db.ColumnExists(tblAnglesStruct, "Theta")); 

    //========================[sc_ABFoo]======================================================
    Utf8CP tblABFoo = "sc_ABFoo"; 
    EXPECT_TRUE (db.TableExists(tblABFoo));
    EXPECT_EQ   (2, GetColumnCount(db, tblABFoo));            
    
    EXPECT_TRUE (db.ColumnExists(tblABFoo, "ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblABFoo, "ECClassId")); 
    
    EXPECT_TRUE (db.ColumnExists(tblABFoo, "stringABFoo")); 
    
    //========================[sc_AAFoo]=========================================================
    Utf8CP tblAAFoo = "sc_AAFoo"; 
    EXPECT_TRUE (db.TableExists(tblAAFoo));
    EXPECT_FALSE(db.TableExists("AFooChild")); //This child class of AAFoo which have TablePerHierarchy so table for its child classes should not be created
    EXPECT_EQ   (25, GetColumnCount(db, tblAAFoo));
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "ECInstanceId"));
    //This a TablePerHieracrchy
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "ECClassId")); 
    
    //Local properties
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "FooTag"));
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "intAAFoo"));
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "longAAFoo"));
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "stringAAFoo"));
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "doubleAAFoo"));
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "datetimeAAFoo"));
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "binaryAAFoo"));
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "booleanAAFoo"));
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "point2dAAFoo_X"));
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "point2dAAFoo_Y"));
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "point3dAAFoo_X"));
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "point3dAAFoo_Y"));
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "point3dAAFoo_Z"));
    //EXPECT_TRUE (db.ColumnExists(tblAAFoo, "anglesAAFoo")); // we are no longer stuffing structs into blobs
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "commonGeometryAAFoo"));
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "colorAAFoo"));

    // arrays
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "arrayOfIntsAAFoo"));
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "arrayOfpoint2dAAFoo"));
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "arrayOfpoint3dAAFoo"));
    
    //From ABFoo since its one of the base class of child class AFooChild
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "stringABFoo"));
    //From AFooChild which is child of AAFoo
    EXPECT_TRUE (db.ColumnExists(tblAAFoo, "binaryAFooChild"));

    //========================[sc_Bar]===========================================================
    Utf8CP tblBar = "sc_Bar";
    EXPECT_TRUE (db.TableExists(tblBar));
    EXPECT_EQ   (4, GetColumnCount(db, tblBar));
    EXPECT_TRUE (db.ColumnExists(tblBar, "ECInstanceId"));
    //This a TablePerHieracrchy
    EXPECT_FALSE(db.ColumnExists(tblBar, "ECClassId"));
    //Local properties
    EXPECT_TRUE (db.ColumnExists(tblBar, "stringBar"));
    //Relations
    EXPECT_TRUE (db.ColumnExists(tblBar, "ForeignECInstanceId_Foo_has_Bars"));
    EXPECT_TRUE (db.ColumnExists(tblBar, "ForeignECInstanceId_Foo_has_Bars_hint"));
    
    //========================[sc_Foo]===========================================================
    Utf8CP tblFoo = "sc_Foo";
    EXPECT_TRUE (db.TableExists(tblFoo));
    EXPECT_EQ   (19, GetColumnCount(db, tblFoo));
    
    EXPECT_TRUE (db.ColumnExists(tblFoo, "ECInstanceId"));
    //This a TablePerHieracrchy
    EXPECT_TRUE (db.ColumnExists(tblFoo, "ECClassId")); 
    
    //Local properties
    EXPECT_TRUE (db.ColumnExists(tblFoo, "intFoo"));
    EXPECT_TRUE (db.ColumnExists(tblFoo, "longFoo"));
    EXPECT_TRUE (db.ColumnExists(tblFoo, "stringFoo"));
    EXPECT_TRUE (db.ColumnExists(tblFoo, "doubleFoo"));
    EXPECT_TRUE (db.ColumnExists(tblFoo, "datetimeFoo"));
    EXPECT_TRUE (db.ColumnExists(tblFoo, "binaryFoo"));
    EXPECT_TRUE (db.ColumnExists(tblFoo, "booleanFoo"));
    EXPECT_TRUE (db.ColumnExists(tblFoo, "point2dFoo_X"));
    EXPECT_TRUE (db.ColumnExists(tblFoo, "point2dFoo_Y"));
    EXPECT_TRUE (db.ColumnExists(tblFoo, "point3dFoo_X"));
    EXPECT_TRUE (db.ColumnExists(tblFoo, "point3dFoo_Y"));
    EXPECT_TRUE (db.ColumnExists(tblFoo, "point3dFoo_Z"));
    EXPECT_TRUE (db.ColumnExists(tblFoo, "commonGeometryFoo"));
    
    // arrays/struct
    EXPECT_TRUE (db.ColumnExists(tblFoo, "arrayOfIntsFoo"));
    EXPECT_FALSE (db.ColumnExists(tblFoo, "arrayOfAnglesStructsFoo"));
    EXPECT_FALSE (db.ColumnExists(tblFoo, "anglesFoo"));

    //========================[sc_ArrayOfStructDomainClass]===========================================================
    Utf8CP tbl = "sc_ArrayOfStructDomainClass"; 
    EXPECT_TRUE (db.TableExists(tbl));
    EXPECT_EQ   (5, GetColumnCount(db, tbl));

    EXPECT_TRUE (db.ColumnExists(tbl, "ECInstanceId"));
    EXPECT_TRUE (db.ColumnExists(tbl, "ParentECInstanceId"));
    //Local properties
    EXPECT_TRUE (db.ColumnExists(tbl, "stringProp"));
    EXPECT_TRUE (db.ColumnExists(tbl, "ECPropertyPathId"));
    EXPECT_TRUE (db.ColumnExists(tbl, "ECArrayIndex"));

    //========================[sc_ArrayOfStructNoneDomainClass]===========================================================
    tbl = "sc_ArrayOfStructNoneDomainClass"; 
    EXPECT_TRUE (db.TableExists(tbl));
    EXPECT_EQ   (5, GetColumnCount(db, tbl));

    EXPECT_TRUE (db.ColumnExists(tbl, "ECInstanceId"));
    EXPECT_TRUE (db.ColumnExists(tbl, "ParentECInstanceId"));
    //Local properties
    EXPECT_TRUE (db.ColumnExists(tbl, "stringProp"));
    EXPECT_TRUE (db.ColumnExists(tbl, "ECPropertyPathId"));
    EXPECT_TRUE (db.ColumnExists(tbl, "ECArrayIndex"));
    
    //========================[sc_ArrayOfStructDomainClassWithNoProperties]===========================================================
    tbl = "sc_ArrayOfStructDomainClassWithNoProperties"; 
    EXPECT_TRUE (db.TableExists(tbl));
    
    //========================[sc_ArrayOfStructNoneDomainClassWithNoProperties]===========================================================
    tbl = "sc_ArrayOfStructNoneDomainClassWithNoProperties"; 
    EXPECT_TRUE (db.TableExists(tbl));

    //========================[sc_DomainClass]===========================================================
    tbl = "sc_DomainClass"; 
    EXPECT_TRUE (db.TableExists(tbl));
    EXPECT_EQ   (2, GetColumnCount(db, tbl));
    
    EXPECT_TRUE (db.ColumnExists(tbl, "ECInstanceId"));
    //Local properties
    EXPECT_TRUE (db.ColumnExists(tbl, "stringProp"));

    //========================[sc_NoneDomainClass]===========================================================
    tbl = "sc_NoneDomainClass"; 
    EXPECT_FALSE (db.TableExists(tbl));
    
    //========================[sc_DomainClassWithNoProperties]===========================================================
    tbl = "sc_DomainClassWithNoProperties"; 
    EXPECT_TRUE (db.TableExists(tbl));
    
    //========================[sc_NoneDomainClassWithNoProperties]===========================================================
    tbl = "sc_NoneDomainClassWithNoProperties"; 
    EXPECT_FALSE (db.TableExists(tbl));
 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaCachePtr CreateImportSchemaAgainstExistingTablesTestSchema ()
    {
    ECSchemaPtr testSchema = nullptr;
    ECSchema::CreateSchema (testSchema, "test", 1, 0);
    testSchema->SetNamespacePrefix ("t");
    ECClassP fooClass = nullptr;
    testSchema->CreateClass (fooClass, "Foo");
    PrimitiveECPropertyP prop = nullptr;
    fooClass->CreatePrimitiveProperty (prop, "Name", PRIMITIVETYPE_String);

    ECClassP gooClass = nullptr;
    testSchema->CreateClass (gooClass, "Goo");
    prop = nullptr;
    gooClass->CreatePrimitiveProperty (prop, "Price", PRIMITIVETYPE_Double);

    ECRelationshipClassP oneToManyRelClass = nullptr;
    testSchema->CreateRelationshipClass (oneToManyRelClass, "FooHasGoo");
    oneToManyRelClass->SetStrength (STRENGTHTYPE_Holding);
    oneToManyRelClass->GetSource ().AddClass (*fooClass);
    oneToManyRelClass->GetSource ().SetCardinality (RelationshipCardinality::OneOne ());
    oneToManyRelClass->GetTarget ().AddClass (*gooClass);
    oneToManyRelClass->GetTarget ().SetCardinality (RelationshipCardinality::ZeroMany ());

    ECRelationshipClassP manyToManyRelClass = nullptr;
    testSchema->CreateRelationshipClass (manyToManyRelClass, "RelFooGoo");
    manyToManyRelClass->SetStrength (STRENGTHTYPE_Referencing);
    manyToManyRelClass->GetSource ().AddClass (*fooClass);
    manyToManyRelClass->GetSource ().SetCardinality (RelationshipCardinality::ZeroMany ());
    manyToManyRelClass->GetTarget ().AddClass (*gooClass);
    manyToManyRelClass->GetTarget ().SetCardinality (RelationshipCardinality::ZeroMany ());

    auto schemaCache = ECSchemaCache::Create ();
    schemaCache->AddSchema (*testSchema);

    return schemaCache;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   04/13
//+---------------+---------------+---------------+---------------+---------------+------
void AssertImportedSchema (DbR ecdb, Utf8CP expectedSchemaName, Utf8CP expectedClassName, Utf8CP expectedPropertyName)
    {
    CachedStatementPtr findClassStmt = nullptr;
    ecdb.GetCachedStatement (findClassStmt, "SELECT NULL FROM ec_Class c, ec_Schema s WHERE c.SchemaId = s.Id AND s.Name = ? AND c.Name = ? LIMIT 1");
    findClassStmt->BindText (1, expectedSchemaName, Statement::MakeCopy::No);
    findClassStmt->BindText (2, expectedClassName, Statement::MakeCopy::No);
    EXPECT_EQ (BE_SQLITE_ROW, findClassStmt->Step ()) << "ECClass " << expectedClassName << " of ECSchema " << expectedSchemaName << " is expected to be found in ec_Class table.";

    if (expectedPropertyName != nullptr)
        {
        CachedStatementPtr findPropertyStmt = nullptr;
        ecdb.GetCachedStatement (findPropertyStmt, "SELECT NULL FROM ec_Property p, ec_Class c, ec_Schema s WHERE p.ClassId = c.Id AND c.SchemaId = s.Id AND s.Name = ? AND c.Name = ? AND p.Name = ? LIMIT 1");
        findPropertyStmt->BindText (1, expectedSchemaName, Statement::MakeCopy::No);
        findPropertyStmt->BindText (2, expectedClassName, Statement::MakeCopy::No);
        findPropertyStmt->BindText (3, expectedPropertyName, Statement::MakeCopy::No);
        EXPECT_EQ (BE_SQLITE_ROW, findPropertyStmt->Step ()) << "ECProperty " << expectedPropertyName << " in ECClass " << expectedClassName << " of ECSchema " << expectedSchemaName << " is expected to be found in ec_Property table.";;
        }
    }

//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>06/2013</date>
//---------------+---------------+---------------+---------------+---------------+-----
TEST(ECDbSchemas, CreateCloseOpenImport)
    {
    ECDbTestProject test;
    auto& ecdb = test.Create("importecschema.ecdb");
    Utf8String filename = ecdb.GetDbFileName();
    ecdb.CloseDb();
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb (filename.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
    EXPECT_EQ (BE_SQLITE_OK, stat);

    ECSchemaPtr ecSchema = nullptr;
    ECSchemaReadContextPtr schemaContext = nullptr;

    ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, schemaContext, L"StartupCompany.02.00.ecschema.xml");
    ASSERT_EQ (SUCCESS, db. Schemas ().ImportECSchemas (schemaContext->GetCache (), ECDbSchemaManager::ImportOptions (false, false))) << "ImportECSchema should have imported successfully after closing and re-opening the database.";

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   04/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemas, ImportSchemaAgainstExistingTableWithoutECInstanceIdColumn)
    {
    // Create a sample project
    ECDbTestProject test;
    auto& ecdb = test.Create ("importecschema.ecdb");

    //create ec table bypassing ECDb API, but don't add it to the ec_ profile tables
    ASSERT_EQ (BE_SQLITE_OK, ecdb.ExecuteSql ("CREATE TABLE t_Foo (Name TEXT)"));

    auto testSchemaCache = CreateImportSchemaAgainstExistingTablesTestSchema ();
    //now import test schema where the table already exists for the ECClass. This is expected to fail.
    BeTest::SetFailOnAssert (false);
        {
        ASSERT_EQ (ERROR, ecdb. Schemas ().ImportECSchemas (*testSchemaCache, ECDbSchemaManager::ImportOptions (false, false))) << "ImportECSchema is expected to return success for schemas with classes that map to an existing table.";
        }
    BeTest::SetFailOnAssert (true);

    EXPECT_TRUE (ecdb.ColumnExists ("t_Foo", "Name")) << "Existing column is expected to still be in the table after ImportECSchemas.";
    EXPECT_FALSE (ecdb.ColumnExists ("t_Foo", "ECInstanceId")) << "ECInstanceId column not expected to be in the table after ImportECSchemas as ImportECSchemas is not expected to modify existing tables.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   04/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemas, ImportSchemaAgainstExistingTableWithECInstanceIdColumn)
    {
    // Create a sample project
    ECDbTestProject test;
    auto& ecdb = test.Create ("importecschema.ecdb");

    //create ec table bypassing ECDb API, but don't add it to the ec_ profile tables
    ASSERT_EQ (BE_SQLITE_OK, ecdb.ExecuteSql ("CREATE TABLE t_Foo (ECInstanceId INTEGER PRIMARY KEY, Name TEXT)"));

    auto testSchemaCache = CreateImportSchemaAgainstExistingTablesTestSchema ();
    //now import test schema where the table already exists for the ECClass
    ASSERT_EQ (SUCCESS, ecdb. Schemas ().ImportECSchemas (*testSchemaCache, 
        ECDbSchemaManager::ImportOptions (false, false))) << "ImportECSchema is expected to return success for schemas with classes that map to an existing table.";

    //ImportSchema does not (yet) modify the existing tables. So it is expected that the ECInstanceId column is not added
    AssertImportedSchema (ecdb, "test", "Foo", "Name");
    EXPECT_TRUE (ecdb.ColumnExists ("t_Foo", "Name")) << "Existing column is expected to still be in the table after ImportECSchemas.";
    EXPECT_TRUE (ecdb.ColumnExists ("t_Foo", "ECInstanceId")) << "Existing column is expected to still be in the table after ImportECSchemas.";
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                       08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbSchemas, DiegoRelationshipTest)
    {
    // Create a sample project
    ECDbTestProject test;
    auto& ecdb = test.Create ("importecschema.ecdb");

    ECSchemaPtr s1, s2;
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext ();
    ECDbTestUtility::ReadECSchemaFromDisk (s1, ctx, L"DiegoSchema1.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE (s1.IsValid ());
    ECDbTestUtility::ReadECSchemaFromDisk (s2, ctx, L"DiegoSchema2.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE (s2.IsValid ());


    //now import test schema where the table already exists for the ECClass
    ASSERT_EQ (SUCCESS, ecdb. Schemas ().ImportECSchemas (ctx->GetCache(),
        ECDbSchemaManager::ImportOptions (false, false))) << "ImportECSchema is expected to return success for schemas with classes that map to an existing table.";


    auto aCivilModel = ecdb. Schemas ().GetECClass ("DiegoSchema1", "CivilModel");
    auto aDatasetModel = ecdb. Schemas ().GetECClass ("DiegoSchema1", "DataSetModel");
    auto aCivilModelHasDataSetModel = ecdb. Schemas ().GetECClass ("DiegoSchema1", "CivilModelHasDataSetModel");
    auto aGeometricModel = ecdb. Schemas ().GetECClass ("DiegoSchema2", "GeometricModel");
    ASSERT_TRUE (aCivilModel != nullptr);
    ASSERT_TRUE (aDatasetModel != nullptr);
    ASSERT_TRUE (aCivilModelHasDataSetModel != nullptr);
    ASSERT_TRUE (aGeometricModel != nullptr);


    auto  iCivilModel1 = ECDbTestProject::CreateArbitraryECInstance (*aCivilModel);
    auto  iCivilModel2 = ECDbTestProject::CreateArbitraryECInstance (*aCivilModel);

    auto  iGeometricModel = ECDbTestProject::CreateArbitraryECInstance (*aGeometricModel);

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*(aCivilModelHasDataSetModel->GetRelationshipClassCP()));

    StandaloneECRelationshipInstancePtr iCivilModelHasDataSetModel2 = relationshipEnabler->CreateRelationshipInstance ();

    iCivilModelHasDataSetModel2->SetSource (iCivilModel2.get ());
    iCivilModelHasDataSetModel2->SetTarget (iGeometricModel.get ());

    ECInstanceInserter aiCivilModel (ecdb, *aCivilModel);
    ASSERT_TRUE (aiCivilModel.IsValid ());
    auto insertStatus = aiCivilModel.Insert (*iCivilModel1);
    insertStatus = aiCivilModel.Insert (*iCivilModel2);


    ECInstanceInserter aiGeometricModel (ecdb, *aGeometricModel);
    ASSERT_TRUE (aiGeometricModel.IsValid ());
    insertStatus = aiGeometricModel.Insert (*iGeometricModel);

    ECInstanceInserter aiCivilModelHasDataSetModel (ecdb, *aCivilModelHasDataSetModel);
    ASSERT_TRUE (aiCivilModelHasDataSetModel.IsValid ());
    insertStatus = aiCivilModelHasDataSetModel.Insert (*iCivilModelHasDataSetModel2);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   04/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemas, ImportSchemaWithRelationshipAgainstExistingTable)
    {
    // Create a sample project
    ECDbTestProject test;
    auto& ecdb = test.Create ("importecschema.ecdb");

    //create ec table bypassing ECDb API, but don't add it to the ec_ profile tables
    ASSERT_EQ (BE_SQLITE_OK, ecdb.ExecuteSql ("CREATE TABLE t_Foo (ECInstanceId INTEGER PRIMARY KEY, Name TEXT)"));
    ASSERT_EQ (BE_SQLITE_OK, ecdb.ExecuteSql ("CREATE TABLE t_Goo (ECInstanceId INTEGER PRIMARY KEY, Price REAL)"));

    auto testSchemaCache = CreateImportSchemaAgainstExistingTablesTestSchema ();
    //now import test schema where the table already exists for the ECClass
    //missing link tables are created if true is passed for createTables
    ASSERT_EQ (SUCCESS, ecdb. Schemas ().ImportECSchemas (*testSchemaCache, ECDbSchemaManager::ImportOptions (false, false))) << "ImportECSchema is expected to return success for schemas with classes that map to an existing table.";

    //ImportSchema does not (yet) modify the existing tables. So it is expected that the ECInstanceId column is not added
    EXPECT_TRUE (ecdb.ColumnExists ("t_Goo", "ECInstanceId")) << "Existing column is expected to still be in the table after ImportECSchemas.";
    EXPECT_TRUE (ecdb.ColumnExists ("t_Goo", "Price")) << "Existing column is expected to still be in the table after ImportECSchemas.";
    EXPECT_TRUE (ecdb.ColumnExists ("t_Goo", "ForeignECInstanceId_FooHasGoo")) << "ForeignECInstanceId_FooHasGoo column not expected to be in the table after ImportECSchemas as ImportECSchemas is not expected to modify existing tables.";
    EXPECT_TRUE (ecdb.TableExists ("t_RelFooGoo")) << "Existence of Link table not as expected.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
void CreateCustomAttributeTestSchema
(
ECSchemaPtr& testSchema,
ECSchemaCachePtr& testSchemaCache
)
    {
    ECSchemaPtr schema = nullptr;
    ECObjectsStatus stat = ECSchema::CreateSchema (schema, "foo", 1, 0);
    ASSERT_EQ (ECOBJECTS_STATUS_Success, stat) << "Creating test schema failed";
    schema->SetNamespacePrefix("f");

    ECClassP domainClass = nullptr;
    stat = schema->CreateClass (domainClass, "domain1");
    ASSERT_EQ (ECOBJECTS_STATUS_Success, stat) << "Creating domain class 1 in schema failed";
    domainClass->SetIsDomainClass (true);
    domainClass->SetIsCustomAttributeClass (false);
    domainClass->SetIsStruct (false);

    ECClassP domainClass2 = nullptr;
    stat = schema->CreateClass (domainClass2, "domain2");
    ASSERT_EQ (ECOBJECTS_STATUS_Success, stat) << "Creating domain class 2 in schema failed";
    domainClass2->SetIsDomainClass (true);
    domainClass2->SetIsCustomAttributeClass (false);
    domainClass2->SetIsStruct (false);

    ECClassP caClass = nullptr;
    stat = schema->CreateClass (caClass, "MyCA");
    ASSERT_EQ (ECOBJECTS_STATUS_Success, stat) << "Creating CA class in schema failed";
    caClass->SetIsDomainClass (false);
    caClass->SetIsCustomAttributeClass (true);
    caClass->SetIsStruct (false);

    PrimitiveECPropertyP dateProp = nullptr;
    caClass->CreatePrimitiveProperty (dateProp, "dateprop", PRIMITIVETYPE_DateTime);

    PrimitiveECPropertyP stringProp = nullptr;
    caClass->CreatePrimitiveProperty (stringProp, "stringprop", PRIMITIVETYPE_String);

    PrimitiveECPropertyP doubleProp = nullptr;
    caClass->CreatePrimitiveProperty (doubleProp, "doubleprop", PRIMITIVETYPE_Double);

    PrimitiveECPropertyP pointProp = nullptr;
    caClass->CreatePrimitiveProperty (pointProp, "pointprop", PRIMITIVETYPE_Point3D);

    ECSchemaCachePtr cache = ECSchemaCache::Create ();
    cache->AddSchema (*schema);

    testSchema = schema;
    testSchemaCache = cache;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
void AssignCustomAttribute
(
IECInstancePtr& caInstance,
ECSchemaPtr schema,
Utf8CP containerClassName,
Utf8CP caClassName,
Utf8CP instanceId,
bmap<Utf8String, ECValue> const& caPropValues
)
    {
    ECClassP caClass = schema->GetClassP (caClassName);
    IECInstancePtr ca = caClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ASSERT_TRUE (ca.IsValid ());

    ECObjectsStatus stat;
    if (instanceId != nullptr)
        {
        stat = ca->SetInstanceId (instanceId);
        ASSERT_EQ (ECOBJECTS_STATUS_Success, stat) << "Setting instance id in CA instance failed";
        }

    typedef bpair<Utf8String, ECValue> T_PropValuePair;

    for (T_PropValuePair const& pair : caPropValues)
        {
        stat = ca->SetValue (pair.first.c_str (), pair.second);
        ASSERT_EQ (ECOBJECTS_STATUS_Success, stat) << "Assigning property value to CA instance failed";
        }

    ECClassP containerClass = schema->GetClassP (containerClassName);
    stat = containerClass->SetCustomAttribute (*ca);
    ASSERT_EQ (ECOBJECTS_STATUS_Success, stat) << "Assigning CA instance to container class failed";

    caInstance = ca;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
IECInstancePtr CreateAndAssignRandomCAInstance
(
ECSchemaPtr testSchema
)
    {
    //assign CA with instance id and all props populated
    bmap<Utf8String, ECValue> propValueMap;
    propValueMap[Utf8String ("dateprop")] = ECValue (DateTime (DateTime::Kind::Unspecified, 1971, 4, 30, 21, 9, 0, 0));
    propValueMap[Utf8String ("stringprop")] = ECValue ("hello world", true);
    propValueMap[Utf8String ("doubleprop")] = ECValue (3.14);
    DPoint3d point;
    point.x = 1.0;
    point.y = -2.0;
    point.z = 3.0;
    propValueMap[Utf8String("pointprop")] = ECValue (point);

    IECInstancePtr ca = nullptr;
    AssignCustomAttribute (ca, testSchema, "domain1", "MyCA", "bla bla", propValueMap);

    return ca;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemas, ReadCustomAttributesTest)
    {
    Utf8CP const CAClassName = "MyCA";

    ECSchemaPtr testSchema = nullptr;
    ECSchemaCachePtr testSchemaCache = nullptr;
    CreateCustomAttributeTestSchema (testSchema, testSchemaCache);

    //assign CA with instance id and all props populated
    IECInstancePtr expectedCAInstanceWithInstanceId = CreateAndAssignRandomCAInstance (testSchema);

    //assign CA without instance id and only a few props populated
    bmap<Utf8String, ECValue> propValueMap;
    propValueMap[Utf8String ("doubleprop")] = ECValue (3.14);
    IECInstancePtr expectedCAInstanceWithoutInstanceId = nullptr;
    AssignCustomAttribute (expectedCAInstanceWithoutInstanceId, testSchema, "domain2", CAClassName, nullptr, propValueMap);

    //create test db and close it again
    Utf8String dbPath;
        {
        ECDbTestProject testProject;
        ECDbR db = testProject.Create ("customattributestest.ecdb");
        auto importStat = db. Schemas ().ImportECSchemas (*testSchemaCache);
        ASSERT_EQ (SUCCESS, importStat) << "Could not import test schema into ECDb file";

        dbPath = testProject.GetECDbPath ();
        }

    //reopen test ECDb file (to make sure that the stored schema is read correctly)
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Could not open test ECDb file";

    ECSchemaCP readSchema = db. Schemas ().GetECSchema (testSchema->GetName().c_str ());
    ASSERT_TRUE (readSchema != nullptr) << "Could not read test schema from reopened ECDb file.";
    //*** assert custom attribute instance with instance id
    ECClassCP domainClass1 = readSchema->GetClassCP ("domain1");
    ASSERT_TRUE (domainClass1 != nullptr) << "Could not retrieve domain class 1 from re-read test schema.";
    IECInstancePtr actualCAInstanceWithInstanceId = domainClass1->GetCustomAttribute (CAClassName);
    ASSERT_TRUE (actualCAInstanceWithInstanceId.IsValid ()) << "Test custom attribute instance not found on domain class 1.";

    //compare instance ids
    ASSERT_STREQ (expectedCAInstanceWithInstanceId->GetInstanceId ().c_str (), actualCAInstanceWithInstanceId->GetInstanceId ().c_str ()) << "Instance Ids of retrieved custom attribute instance doesn't match.";
    
    //compare rest of instance
    bool equal = ECDbTestUtility::CompareECInstances (*expectedCAInstanceWithInstanceId, *actualCAInstanceWithInstanceId);
    ASSERT_TRUE (equal) << "Read custom attribute instance with instance id differs from expected.";

    //*** assert custom attribute instance without instance id
    ECClassCP domainClass2 = readSchema->GetClassCP ("domain2");
    ASSERT_TRUE (domainClass2 != nullptr) << "Could not retrieve domain class 2 from re-read test schema.";
    IECInstancePtr actualCAInstanceWithoutInstanceId = domainClass2->GetCustomAttribute (CAClassName);
    ASSERT_TRUE (actualCAInstanceWithoutInstanceId.IsValid ()) << "Test custom attribute instance not found on domain class 2.";

    //compare instance ids
    ASSERT_STREQ (expectedCAInstanceWithoutInstanceId->GetInstanceId ().c_str (), actualCAInstanceWithoutInstanceId->GetInstanceId ().c_str ()) << "Instance Ids of retrieved custom attribute instance doesn't match.";
    ASSERT_STREQ ("", actualCAInstanceWithoutInstanceId->GetInstanceId ().c_str ()) << "Instance Ids of retrieved custom attribute instance is expected to be empty";
    
    //compare rest of instance
    equal = ECDbTestUtility::CompareECInstances (*expectedCAInstanceWithoutInstanceId, *actualCAInstanceWithoutInstanceId);
    ASSERT_TRUE (equal) << "Read custom attribute instance without instance id differs from expected.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemas, CheckCustomAttributesXmlFormatTest)
    {
    ECSchemaPtr testSchema = nullptr;
    ECSchemaCachePtr testSchemaCache = nullptr;
    CreateCustomAttributeTestSchema (testSchema, testSchemaCache);

    //assign CA with instance id
    CreateAndAssignRandomCAInstance (testSchema);

    ECDbTestProject testProject;
    ECDbR db = testProject.Create ("customattributestest.ecdb");
    auto importStat = db. Schemas ().ImportECSchemas (*testSchemaCache);
    ASSERT_EQ (SUCCESS, importStat) << "Could not import test schema into ECDb file";

    //now retrieve the persisted CA XML from ECDb directly
    Statement stmt;
    DbResult stat = stmt.Prepare (db, "SELECT Instance from ec_CustomAttribute ca, ec_Class c where ca.ClassId = c.Id AND c.Name = 'MyCA'");
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Preparing the SQL statement to fetch the persisted CA XML string failed.";

    int rowCount = 0;
    while (stmt.Step () == BE_SQLITE_ROW)
        {
        rowCount++;
        Utf8CP caXml = stmt.GetValueText (0);
        ASSERT_TRUE (caXml != nullptr) << "Retrieved custom attribute XML string is expected to be not null.";
        Utf8String caXmlString (caXml);
        EXPECT_LT (0, (int) caXmlString.length ()) << "Retrieved custom attribute XML string is not expected to be empty.";

        //It is expected that the XML string doesn't contain the XML descriptor.
        size_t found = caXmlString.find ("<?xml");
        EXPECT_EQ (Utf8String::npos, found) << "The custom attribute XML string is expected to not contain the XML description tag.";

        //It is expected that the XML string does contain the instance id if the original CA was assigned one
        found = caXmlString.find ("instanceID=");
        EXPECT_NE (Utf8String::npos, found) << "The custom attribute XML string is expected to contain the instance id for the given custom attribute instance.";
        }

    ASSERT_EQ (1, rowCount) << "Only one test custom attribute instance had been created.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemas, HandlingMismatchesBetweenCAInstanceAndCAClassTest)
    {
    Utf8CP const CAClassName = "MyCA";
    Utf8CP nameOfCAPropertyToRemove = "dateprop";

    ECSchemaPtr testSchema = nullptr;
    ECSchemaCachePtr testSchemaCache = nullptr;
    CreateCustomAttributeTestSchema (testSchema, testSchemaCache);

    //assign CA with instance id and all props populated
    IECInstancePtr expectedCAInstance = CreateAndAssignRandomCAInstance (testSchema);

    //create test ECDb file and delete one of the properties of the CA in the EC tables
    Utf8String dbPath;
        {
        ECDbTestProject testProject;
        ECDbR db = testProject.Create ("customattributestest.ecdb");
        auto importStat = db. Schemas ().ImportECSchemas (*testSchemaCache);
        ASSERT_EQ (SUCCESS, importStat) << "Could not import test schema into ECDb file";

        //now remove one of the CA properties only in the CA class definition again
        Statement stmt;
        DbResult stat = stmt.Prepare (db, "delete from ec_Property WHERE Name = 'dateprop' and ClassId = (select Id from ec_Class where Name = 'MyCA')");
        ASSERT_EQ (BE_SQLITE_OK, stat) << "Preparing the SQL statement to delete row from ec_Property failed.";
        stat = stmt.Step ();
        ASSERT_EQ (BE_SQLITE_DONE, stat) << "Executing SQL statement to delete row from ec_Property failed";
        EXPECT_EQ (1, db.GetModifiedRowCount ()) << "The SQL statement to delete row from ec_Property is expected to only delete one row";

        dbPath = testProject.GetECDbPath ();
        }

    //now reopen the out-synched ECDb file (to make sure that the schema stuff is read into memory from scratch
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Could not open test ECDb file";

    ECSchemaCP readSchema = db. Schemas ().GetECSchema (Utf8String (testSchema->GetName ().c_str ()).c_str ());
    ASSERT_TRUE (readSchema != nullptr) << "Could not read test schema from reopened ECDb file.";

    //assert custom attribute instance with instance id
    ECClassCP domainClass1 = readSchema->GetClassCP ("domain1");
    ASSERT_TRUE (domainClass1 != nullptr) << "Could not retrieve domain class 1 from re-read test schema.";

    IECInstancePtr actualCAInstance = domainClass1->GetCustomAttribute (CAClassName);
    ASSERT_TRUE (actualCAInstance.IsValid ()) << "Test custom attribute instance not found on domain class 1.";

    //removed property is expected to not be found anymore in the instance
    bool isNull = false;
    ECObjectsStatus ecStat = actualCAInstance->IsPropertyNull (isNull, nameOfCAPropertyToRemove);
    EXPECT_EQ (ECOBJECTS_STATUS_PropertyNotFound, ecStat) << "Calling IsPropertyNull on CA instance";

    //now check whether the rest of the instance is still the same
    ECValuesCollectionPtr expectedValueCollection = ECValuesCollection::Create (*expectedCAInstance);
    for (ECPropertyValueCR expectedPropertyValue : *expectedValueCollection)
        {
        Utf8CP expectedPropertyName = expectedPropertyValue.GetValueAccessor ().GetAccessString ();
        if (BeStringUtilities::Stricmp (expectedPropertyName, nameOfCAPropertyToRemove) == 0)
            {
            continue;
            }

        ECValue actualValue;
        ecStat = actualCAInstance->GetValue (actualValue, expectedPropertyName);
        EXPECT_EQ (ECOBJECTS_STATUS_Success, stat) << "Property '" << expectedPropertyName << "' not found in actual CA instance.";
        EXPECT_TRUE (expectedPropertyValue.GetValue ().Equals (actualValue)) << "Property values for property '" << expectedPropertyName << "' do not match";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        07/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, ImportSupplementalSchemas)
    {
    BeFileName outputRoot, assetsDir;
    BeTest::GetHost ().GetOutputRoot (outputRoot);
    BeTest::GetHost ().GetDgnPlatformAssetsDirectory (assetsDir);

    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    ECDb::Initialize (temporaryDir, &assetsDir);

    ECSchemaPtr ecSchema = nullptr;
    ECSchemaReadContextPtr schemaContext = nullptr;

    //ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, *schemaContext,L"OpenPlant_3D.01.02.ecschema.xml");
    //ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, *schemaContext,L"OpenPlant_3D_Supplemental_Isometrics_Isoextractor.01.02.ecschema.xml");
    //ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, *schemaContext,L"OpenPlant_3D_Supplemental_Metric_DefaultValues.01.02.ecschema.xml");
    //ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, *schemaContext,L"OpenPlant_3D_Supplemental_Model_Server.01.02.ecschema.xml");
    //ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, *schemaContext,L"OpenPlant_3D_Supplemental_Modeling.01.02.ecschema.xml");
    //ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, *schemaContext,L"OpenPlant_3D_Supplemental_ModelingViews.01.02.ecschema.xml");
    //ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, *schemaContext,L"OpenPlant_Supplemental_Tagging.01.01.ecschema.xml");
    //ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, *schemaContext,L"OpenPlant_Supplemental_Units_Metric.01.02.ecschema.xml");

    //ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, *schemaContext,L"OpenPlant.01.02.ecschema.xml");
    //ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, *schemaContext,L"OpenPlant_Supplemental_Tagging.01.01.ecschema.xml");
    //ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, *schemaContext,L"OpenPlant_Supplemental_Units_Metric.01.02.ecschema.xml");
    ECSchemaPtr startup;
    ECSchemaPtr supple;
    ECDbTestUtility::ReadECSchemaFromDisk (startup, schemaContext, L"StartupCompany.02.00.ecschema.xml");
    ECDbTestUtility::ReadECSchemaFromDisk (supple, schemaContext, L"StartupCompany_Supplemental_ECDbTest.01.00.ecschema.xml");
    SchemaKey key ("StartupCompany", 2, 0);

    bvector<ECSchemaP> supplementalSchemas;
    supplementalSchemas.push_back (supple.get ());
    SupplementedSchemaBuilder builder;

    BeFileName projectFile (nullptr, outputRoot.GetName (), L"SupplementedSchemaTest.ecdb", nullptr);
    if (BeFileName::DoesPathExist (projectFile.GetName ()))
        {
        // Delete any previously created file
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (projectFile.GetName ());
        ASSERT_TRUE (fileDeleteStatus == BeFileNameStatus::Success) << "Could not delete preexisting test ecdb file '" << projectFile.GetName () << "'.";
        }
    ECDb db;
    DbResult stat = db.CreateNewDb (projectFile.GetNameUtf8 ().c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test ECDb file failed.";
    auto schemaStatus = db. Schemas ().ImportECSchemas (schemaContext->GetCache (),
        ECDbSchemaManager::ImportOptions (true, false));
    ASSERT_EQ (SUCCESS, schemaStatus);

    db.SaveChanges ();
    db.CloseDb ();

    db.OpenBeSQLiteDb (projectFile.GetNameUtf8 ().c_str (), Db::OpenParams (Db::OpenMode::Readonly));
    ECSchemaCP startupCompanySchema = db. Schemas ().GetECSchema ("StartupCompany");
    ASSERT_TRUE (startupCompanySchema != nullptr);
    ECClassCP aaa2 = startupCompanySchema->GetClassCP ("AAA");

    ECCustomAttributeInstanceIterable allCustomAttributes2 = aaa2->GetCustomAttributes (false);
    uint32_t allCustomAttributesCount2 = 0;
    for (IECInstancePtr attribute : allCustomAttributes2)
        {
        allCustomAttributesCount2++;
        }
    EXPECT_EQ (2, allCustomAttributesCount2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/13
! This test need to be moved to ECF test suit
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, SystemSchemaTest)
   {
   ECDbTestProject saveTestProject;

   ECDbR db = saveTestProject.Create ("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", false);
   ECSchemaCP startupCompanySchema = db.Schemas().GetECSchema ("StartupCompany");
   ASSERT_TRUE (startupCompanySchema != nullptr);
   ECSchemaCP ecdbSystemSchema = db.Schemas().GetECSchema ("ECDb_System");
   ASSERT_TRUE (ecdbSystemSchema != nullptr);

   EXPECT_TRUE (ecdbSystemSchema->IsSystemSchema ());
   EXPECT_TRUE (StandardCustomAttributeHelper::IsSystemSchema (*ecdbSystemSchema));

   EXPECT_FALSE (startupCompanySchema->IsSystemSchema ());
   EXPECT_FALSE (StandardCustomAttributeHelper::IsSystemSchema (*startupCompanySchema));
   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ECDbSchemas, ArrayPropertyTest)
    {
    ECDbTestProject saveTestProject;

    ECDbR db = saveTestProject.Create ("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", false);
    db.ClearECDbCache();
    ECSchemaCP startupCompanySchema = db. Schemas ().GetECSchema ("StartupCompany", true);
    ASSERT_TRUE (startupCompanySchema != nullptr);

    auto arrayTestClass = startupCompanySchema->GetClassCP ("ArrayTestclass");
    ASSERT_TRUE (arrayTestClass != nullptr);

    auto p0_unbounded = arrayTestClass->GetPropertyP ("p0_unbounded")->GetAsArrayProperty ();
    ASSERT_TRUE (p0_unbounded != nullptr);
    ASSERT_EQ (p0_unbounded->GetMinOccurs (), 0);
    ASSERT_EQ (p0_unbounded->GetMaxOccurs (), UINT32_MAX);

    auto p1_unbounded = arrayTestClass->GetPropertyP ("p1_unbounded")->GetAsArrayProperty ();
    ASSERT_TRUE (p1_unbounded != nullptr);
    ASSERT_EQ (p1_unbounded->GetMinOccurs (), 1);
    ASSERT_EQ (p1_unbounded->GetMaxOccurs (), UINT32_MAX);

    auto p0_1 = arrayTestClass->GetPropertyP ("p0_1")->GetAsArrayProperty ();
    ASSERT_TRUE (p0_1 != nullptr);
    ASSERT_EQ (p0_1->GetMinOccurs (), 0);
    ASSERT_EQ (p0_1->GetMaxOccurs (), UINT32_MAX);

    auto p1_1 = arrayTestClass->GetPropertyP ("p1_1")->GetAsArrayProperty ();
    ASSERT_TRUE (p1_1 != nullptr);
    ASSERT_EQ (p1_1->GetMinOccurs (), 1);
    ASSERT_EQ (p1_1->GetMaxOccurs (), UINT32_MAX);

    auto p1_10000 = arrayTestClass->GetPropertyP ("p1_10000")->GetAsArrayProperty ();
    ASSERT_TRUE (p1_10000 != nullptr);
    ASSERT_EQ (p1_10000->GetMinOccurs (), 1);
    ASSERT_EQ (p1_10000->GetMaxOccurs (), UINT32_MAX);

    auto p100_10000 = arrayTestClass->GetPropertyP ("p100_10000")->GetAsArrayProperty ();
    ASSERT_TRUE (p100_10000 != nullptr);
    ASSERT_EQ (p100_10000->GetMinOccurs (), 100);
    ASSERT_EQ (p100_10000->GetMaxOccurs (), UINT32_MAX);

    auto p123_12345 = arrayTestClass->GetPropertyP ("p123_12345")->GetAsArrayProperty ();
    ASSERT_TRUE (p123_12345 != nullptr);
    ASSERT_EQ (p123_12345->GetMinOccurs (), 123);
    ASSERT_EQ (p123_12345->GetMaxOccurs (), UINT32_MAX);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/13
! This test need to be moved to ECF test suit
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, DynamicSchemaTest)
    {
    BeFileName outputRoot, assetsDir;
    BeTest::GetHost().GetOutputRoot (outputRoot);
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (assetsDir);

    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    ECDb::Initialize (temporaryDir, &assetsDir);

    ECSchemaPtr testSchema; 
    ASSERT_EQ (ECSchema::CreateSchema(testSchema, "TestSchema", 1, 1), ECOBJECTS_STATUS_Success);
    ASSERT_EQ (testSchema->IsDynamicSchema(), false);
    ASSERT_EQ (testSchema->SetIsDynamicSchema(true), ECOBJECTS_STATUS_DynamicSchemaCustomAttributeWasNotFound);
    //reference BCSA, DynamicSchema CA introduce in 1.6
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    SchemaKey bscaKey ("Bentley_Standard_CustomAttributes", 1, 6);
    ECSchemaPtr bscaSchema =  ctx->LocateSchema (bscaKey, SCHEMAMATCHTYPE_Latest);
    ASSERT_TRUE (bscaSchema.IsValid());
    ASSERT_EQ (testSchema->AddReferencedSchema(*bscaSchema), ECOBJECTS_STATUS_Success);
    ASSERT_EQ (testSchema->SetIsDynamicSchema(true), ECOBJECTS_STATUS_Success);
    ASSERT_TRUE (testSchema->IsDynamicSchema());
    ASSERT_TRUE (StandardCustomAttributeHelper::IsDynamicSchema (*testSchema));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        07/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, ECDbSchemaManagerAPITest)
    {
    // Save a test project
    ECDbTestProject saveTestProject;
    saveTestProject.Create ("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", false); 

    ECSchemaPtr diskSchema = saveTestProject.GetTestSchemaManager ().GetTestSchema ();
    
    // Reopen the test project
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb (saveTestProject.GetECDb().GetDbFileName(), Db::OpenParams(Db::OpenMode::Readonly));
    EXPECT_EQ (BE_SQLITE_OK, stat);

    ECSchemaKeys schemasInDb;
    //Get DbECSchemaKeys
    ECSchemaList ecDbSchemaList;

    ECDbSchemaManagerCR schemaManager = db.Schemas ();
    ///////////////////////Load StartupCompany//////////////////////////////
    StopWatch s0 ("Loading StartupCompany Schema From Database", true);
    ECSchemaCP openPlant3D = schemaManager.GetECSchema ("StartupCompany", true /* Load full schemas*/);
    EXPECT_TRUE (openPlant3D != nullptr);
    s0.Stop();
    LOG.infov ("Loading %s From DataBase Took : %.4lf seconds", openPlant3D->GetFullSchemaName().c_str(), s0.GetElapsedSeconds());
    StopWatch s1 ("Comparing ECSchema", true);
    
    //Diff two schema too see if they are different in any way diff.Merge();
    ECDiffPtr diff = ECDiff::Diff(*diskSchema, *openPlant3D);
    ASSERT_EQ (diff->GetStatus() , DIFFSTATUS_Success);
    if (!diff->IsEmpty())
        {
        bmap<Utf8String, DiffNodeState> searchResults;
        diff->GetNodesState(searchResults, "*.ArrayInfo");
        if (!searchResults.empty())
            LOG.error("*** Feature missing : Array type property MaxOccurs and MinOccurs are not stored currently by ECDbSchemaManager");
        WriteECSchemaDiffToLog(*diff, NativeLogging::LOG_ERROR);

        }
#if 0
    EXPECT_EQ (diff->IsEmpty() , true);
#endif   

    s1.Stop();
    LOG.infov ("Comparing Db %s to disk version Took : %.4lf seconds", openPlant3D->GetFullSchemaName().c_str(), s1.GetElapsedSeconds());

    //////////////////////////////////////////////////////////////////////
    db.ClearECDbCache();
    ECClassKeys inSchemaClassKeys;
    EXPECT_EQ (SUCCESS, schemaManager.GetECClassKeys (inSchemaClassKeys, "StartupCompany"));
    LOG.infov("No of classes in StartupCompany is %d", (int)inSchemaClassKeys.size());
    EXPECT_EQ (56, inSchemaClassKeys.size());

    StopWatch randomClassSW ("Loading Random Class", false);
    int maxClassesToLoad = 100;
    double totalTime = 0;
    for(int i=0; i < maxClassesToLoad; i++)
        {
        ECClassKey key = inSchemaClassKeys[(int)(((float)rand()/RAND_MAX)*(inSchemaClassKeys.size()-1))];
        randomClassSW.Init(true);
        EXPECT_TRUE (nullptr != schemaManager.GetECClass (key.GetECClassId ()));
        randomClassSW.Stop();
        totalTime += randomClassSW.GetElapsedSeconds();

        LOG.infov ("%3ld. Accessing random class took : %.4lf seconds (%s)", i, randomClassSW.GetElapsedSeconds(), key.GetName());
        }

    LOG.infov ("It took Total : %.4lf seconds to load %d classes", totalTime, maxClassesToLoad);
     
    EXPECT_EQ (SUCCESS, schemaManager.GetECSchemaKeys (schemasInDb));

    LOG.info ("Testing SchemaManager APIs");
    for (ECSchemaKey const& schemaKey : schemasInDb)
        {
         ECSchemaCP outSchema = schemaManager.GetECSchema (schemaKey.GetName ());
         EXPECT_TRUE(outSchema != nullptr);
         EXPECT_TRUE(outSchema->HasId());
         ECSchemaId ecSchemaId = outSchema->GetId();
         EXPECT_TRUE(ecSchemaId != 0);

         ECClassKeys classKeys;
         EXPECT_EQ (SUCCESS, schemaManager.GetECClassKeys(classKeys, schemaKey.GetName ()));
         //verify GetECClass() class API
         for (ECClassKey const& classKey : classKeys)
             {
             auto outClass = schemaManager.GetECClass (classKey.GetECClassId ());
             EXPECT_TRUE(outClass != nullptr);
             outClass = schemaManager.GetECClass (schemaKey.GetName (), classKey.GetName ());
             EXPECT_TRUE(outClass != nullptr);
             outClass = nullptr;
             }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        07/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, SchemaDiff)
    {
    BeFileName outputRoot, assetsDir;
    BeTest::GetHost().GetOutputRoot (outputRoot);
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (assetsDir);

    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    ECDb::Initialize (temporaryDir, &assetsDir);

    ECSchemaReadContextPtr leftSchemaContext = nullptr;
    ECSchemaReadContextPtr rightSchemaContext = nullptr;

    ECSchemaPtr leftSchema, rightSchema;   
    ECDbTestUtility::ReadECSchemaFromDisk (leftSchema, leftSchemaContext, L"LeftSchema.01.00.ecschema.xml");
    ECDbTestUtility::ReadECSchemaFromDisk (rightSchema, rightSchemaContext, L"RightSchema.01.00.ecschema.xml");

    ECDiffPtr diff = ECDiff::Diff(*leftSchema, *rightSchema);
    ASSERT_TRUE ( diff.IsValid());

    bmap<Utf8String,DiffNodeState> unitStates;
    diff->GetNodesState (unitStates, "*.CustomAttributes.Unit_Attributes:UnitSpecification");
    diff->GetNodesState (unitStates, "*.CustomAttributes.Unit_Attributes:UnitSpecifications");
    ASSERT_EQ(unitStates.size(), 2);
    WriteECSchemaDiffToLog (*diff);
    ECSchemaPtr mergedSchema;
    MergeStatus status = diff->Merge (mergedSchema, CONFLICTRULE_TakeLeft);
    ASSERT_EQ(status , MERGESTATUS_Success);   
    ASSERT_TRUE(mergedSchema.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void VerifyRelationshipConstraint(ECN::ECSchemaCR schema, Utf8CP relationName, Utf8CP sourceClass, Utf8CP targetClass)
    {
    auto ecClass = schema.GetClassCP(relationName);
    ASSERT_TRUE(ecClass != nullptr);
    auto ecRelationshipClass = ecClass->GetRelationshipClassCP();
    ASSERT_TRUE(ecRelationshipClass != nullptr);
    ASSERT_EQ(ecRelationshipClass->GetSource().GetClasses().size(), 1);
    ASSERT_EQ(ecRelationshipClass->GetTarget().GetClasses().size(), 1);    
    ASSERT_TRUE(ecRelationshipClass->GetSource().GetClasses().at(0)->GetName().Equals(sourceClass));
    ASSERT_TRUE(ecRelationshipClass->GetTarget().GetClasses().at(0)->GetName().Equals(targetClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        06/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, PFLModulePPCS_ECDiffTest)
    {
    BeFileName outputRoot, assetsDir;
    BeTest::GetHost().GetOutputRoot (outputRoot);
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (assetsDir);
    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    ECDb::Initialize (temporaryDir, &assetsDir);

    ECSchemaReadContextPtr leftSchemaContext = nullptr;
    ECSchemaReadContextPtr rightSchemaContext = nullptr;

    ECSchemaPtr leftSchema, rightSchema;   
    ECDbTestUtility::ReadECSchemaFromDisk (leftSchema, leftSchemaContext, L"PFLModulePPCS.01.00.ecschema.xml");
    ECDbTestUtility::ReadECSchemaFromDisk (rightSchema, rightSchemaContext, L"PFLModulePPCS.02.00.ecschema.xml");
    rightSchema->SetVersionMajor(1);

    ECDiffPtr diff = ECDiff::Diff(*leftSchema, *rightSchema);
    ASSERT_TRUE ( diff.IsValid());

    WriteECSchemaDiffToLog (*diff);
    ECSchemaPtr mergedSchema;
    MergeStatus status = diff->Merge (mergedSchema, CONFLICTRULE_TakeLeft);
    ASSERT_EQ(status , MERGESTATUS_Success);   
    ASSERT_TRUE(mergedSchema.IsValid());
    
    VerifyRelationshipConstraint(*mergedSchema, "STRUFRMW",   "STRU", "FRMW");
    VerifyRelationshipConstraint(*mergedSchema, "SUBELEVEL5", "SUBE", "LEVEL5");
    VerifyRelationshipConstraint(*mergedSchema, "ZONEEQUI",   "ZONE", "EQUI");
    VerifyRelationshipConstraint(*mergedSchema, "ZONESTRU",   "ZONE", "STRU");
    VerifyRelationshipConstraint(*mergedSchema, "EQUISUBE",   "EQUI", "SUBE");
    VerifyRelationshipConstraint(*mergedSchema, "FRMWLEVEL5", "FRMW", "LEVEL5");
    VerifyRelationshipConstraint(*mergedSchema, "FRMWSBFR",   "FRMW", "SBFR");

    }
/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Adeel.Shoukat                        05/3
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, ClassDiff)
    {
    BeFileName outputRoot, applicationDir;
    BeTest::GetHost().GetOutputRoot (outputRoot);
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (applicationDir);
    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    ECDb::Initialize (temporaryDir, &applicationDir);

    ECSchemaReadContextPtr leftSchemaContext = nullptr;
    ECSchemaReadContextPtr rightSchemaContext = nullptr;
    ECSchemaPtr leftSchema, rightSchema;   
    ECDbTestUtility::ReadECSchemaFromDisk (leftSchema, leftSchemaContext, L"LeftSchema.01.00.ecschema.xml");
    ECDbTestUtility::ReadECSchemaFromDisk (rightSchema, rightSchemaContext, L"RightSchema.01.00.ecschema.xml");
    ECDiffPtr diff = ECDiff::Diff(*leftSchema, *rightSchema);
    ASSERT_TRUE ( diff.IsValid());
    bmap<Utf8String,DiffNodeState> unitStates;
    diff->GetNodesState (unitStates, "*.CustomAttributes.Unit_Attributes:UnitSpecification");
    diff->GetNodesState (unitStates, "*.CustomAttributes.Unit_Attributes:UnitSpecifications");
    ASSERT_EQ(unitStates.size(), 2);
    WriteECSchemaDiffToLog (*diff);
    ECSchemaPtr mergedSchema;
    MergeStatus status = diff->Merge (mergedSchema, CONFLICTRULE_TakeLeft);
    ASSERT_EQ(status , MERGESTATUS_Success);   
    ASSERT_TRUE(mergedSchema.IsValid());
    ECClassP classPtr=mergedSchema->GetClassP("Employee");
    uint32_t classCount=mergedSchema->GetClassCount();
    EXPECT_EQ(classCount,8);
    bool bclassDisplayLabel=classPtr->GetIsDisplayLabelDefined();
    Utf8String className=classPtr->GetName();
    EXPECT_STREQ(className.c_str(),"Employee");
    EXPECT_TRUE(bclassDisplayLabel);
    classPtr=mergedSchema->GetClassP("RightFoo");
    className=classPtr->GetName();
    EXPECT_STREQ(className.c_str(),"RightFoo");
    classPtr=mergedSchema->GetClassP("StableClass");
    className=classPtr->GetName();
    EXPECT_STREQ(className.c_str(),"StableClass");
    classPtr=mergedSchema->GetClassP("TestR");
    className=classPtr->GetName();
    EXPECT_STREQ(className.c_str(),"TestR");
    classPtr=mergedSchema->GetClassP("UnitConflictClass");
    className=classPtr->GetName();
    ECSchemaCR SchemaToCheck=classPtr->GetSchema();
    Utf8String SchemaName=SchemaToCheck.GetFullSchemaName();
    EXPECT_STREQ(SchemaName.c_str(),"LeftSchema.01.00");
    EXPECT_STREQ(className.c_str(),"UnitConflictClass");
    classPtr=mergedSchema->GetClassP("Employee");
    className=classPtr->GetName();
    EXPECT_STREQ(className.c_str(),"Employee");
    Utf8String classDisplayLabel =classPtr->GetDisplayLabel();
    EXPECT_STREQ(classDisplayLabel.c_str(),"Employee Left");
    classPtr=mergedSchema->GetClassP("LeftFoo");
    className=classPtr->GetName();
    EXPECT_STREQ(className.c_str(),"LeftFoo");
    classPtr=mergedSchema->GetClassP("StableClass");
    className=classPtr->GetName();
    EXPECT_STREQ(className.c_str(),"StableClass");
    classPtr=mergedSchema->GetClassP("StableClass");
    className=classPtr->GetName();
    EXPECT_STREQ(className.c_str(),"StableClass");
    }
/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Adeel.Shoukat                        05/3
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, RelationshiClassDiff)
    {
    BeFileName outputRoot, applicationDir;
    BeTest::GetHost().GetOutputRoot (outputRoot);
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (applicationDir);
    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    ECDb::Initialize (temporaryDir, &applicationDir);

    ECSchemaReadContextPtr leftSchemaContext = nullptr;
    ECSchemaReadContextPtr rightSchemaContext = nullptr;
    ECSchemaPtr leftSchema, rightSchema;   
    ECDbTestUtility::ReadECSchemaFromDisk (leftSchema, leftSchemaContext, L"LeftSchema.01.00.ecschema.xml");
    ECDbTestUtility::ReadECSchemaFromDisk (rightSchema, rightSchemaContext, L"RightSchema.01.00.ecschema.xml");
    ECDiffPtr diff = ECDiff::Diff(*leftSchema, *rightSchema);
    ASSERT_TRUE ( diff.IsValid());
    bmap<Utf8String,DiffNodeState> unitStates;
    diff->GetNodesState (unitStates, "*.CustomAttributes.Unit_Attributes:UnitSpecification");
    diff->GetNodesState (unitStates, "*.CustomAttributes.Unit_Attributes:UnitSpecifications");
    ASSERT_EQ(unitStates.size(), 2);
    WriteECSchemaDiffToLog (*diff);
    ECSchemaPtr mergedSchema;
    MergeStatus status = diff->Merge (mergedSchema, CONFLICTRULE_TakeLeft);
    ASSERT_EQ(status , MERGESTATUS_Success);   
    ASSERT_TRUE(mergedSchema.IsValid());
    ECRelationshipClassCP relationshipClassPtr = mergedSchema->GetClassP("RightRelationshipClass")->GetRelationshipClassP();
    Utf8String relationshipClassName=relationshipClassPtr->GetName();
    EXPECT_STREQ(relationshipClassName.c_str(), "RightRelationshipClass");
    relationshipClassPtr=mergedSchema->GetClassP("TestRelationshipClass")->GetRelationshipClassP();
    relationshipClassName=relationshipClassPtr->GetName();
    EXPECT_STREQ(relationshipClassName.c_str(), "TestRelationshipClass");
    relationshipClassPtr=mergedSchema->GetClassP("TestR")->GetRelationshipClassP();
    relationshipClassName=relationshipClassPtr->GetName();
    EXPECT_STREQ(relationshipClassName.c_str(), "TestR");
    }
/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Adeel.Shoukat                        05/3
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, PropertiesDiff)
    {
    BeFileName outputRoot, applicationDir;
    BeTest::GetHost().GetOutputRoot (outputRoot);
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (applicationDir);
    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    ECDb::Initialize (temporaryDir, &applicationDir);

    ECSchemaReadContextPtr leftSchemaContext = nullptr;
    ECSchemaReadContextPtr rightSchemaContext = nullptr;
    ECSchemaPtr leftSchema, rightSchema;   
    ECDbTestUtility::ReadECSchemaFromDisk (leftSchema, leftSchemaContext, L"LeftSchema.01.00.ecschema.xml");
    ECDbTestUtility::ReadECSchemaFromDisk (rightSchema, rightSchemaContext, L"RightSchema.01.00.ecschema.xml");
    ECDiffPtr diff = ECDiff::Diff(*leftSchema, *rightSchema);
    ASSERT_TRUE ( diff.IsValid());
    bmap<Utf8String,DiffNodeState> unitStates;
    diff->GetNodesState (unitStates, "*.CustomAttributes.Unit_Attributes:UnitSpecification");
    diff->GetNodesState (unitStates, "*.CustomAttributes.Unit_Attributes:UnitSpecifications");
    ASSERT_EQ(unitStates.size(), 2);
    WriteECSchemaDiffToLog (*diff);
    ECSchemaPtr mergedSchema;
    MergeStatus status = diff->Merge (mergedSchema, CONFLICTRULE_TakeLeft);
    ASSERT_EQ(status , MERGESTATUS_Success);   
    ASSERT_TRUE(mergedSchema.IsValid());
    ECClassP ecClassPtr = mergedSchema->GetClassP("Employee");
    ECPropertyP  ecPropertyPtr=  ecClassPtr->GetPropertyP("Address");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    Utf8String ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(), "Address");
    ecPropertyPtr=  ecClassPtr->GetPropertyP("Department");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(), "Department");
    ecPropertyPtr=  ecClassPtr->GetPropertyP("EmployeeId");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(), "EmployeeId");
    ecPropertyPtr=  ecClassPtr->GetPropertyP("LeftAddedThisProperty");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(), "LeftAddedThisProperty");
    ecPropertyPtr=  ecClassPtr->GetPropertyP("Name");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(), "Name");
    ecPropertyPtr=  ecClassPtr->GetPropertyP("PhoneNumbers");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(), "PhoneNumbers");
    ecClassPtr = mergedSchema->GetClassP("UnitConflictClass");
    ecPropertyPtr=  ecClassPtr->GetPropertyP("UCCProp");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(),"UCCProp");
    ecClassPtr = mergedSchema->GetClassP("Employee");
    ecPropertyPtr=  ecClassPtr->GetPropertyP("RightAddedThisProperty");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(), "RightAddedThisProperty");
    ecClassPtr = mergedSchema->GetClassP("StableClass");
    ecPropertyPtr=  ecClassPtr->GetPropertyP("arrayProperty");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(), "arrayProperty");
    ecClassPtr = mergedSchema->GetClassP("RightFoo");
    ecPropertyPtr=  ecClassPtr->GetPropertyP("RightDoubleProperty");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(), "RightDoubleProperty");
    ECRelationshipClassP  ecRelationshipClassPtr = mergedSchema->GetClassP("RightRelationshipClass")->GetRelationshipClassP();
    ecPropertyPtr=  ecRelationshipClassPtr->GetPropertyP("Property");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(), "Property");
    ecRelationshipClassPtr = mergedSchema->GetClassP("TestRelationshipClass")->GetRelationshipClassP();
    ecPropertyPtr=  ecRelationshipClassPtr->GetPropertyP("PropertyB");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(), "PropertyB");
    }
/*---------------------------------------------------------------------------------**//**
 * @bsiclass                                   Adeel.Shoukat                        04/13
 +---------------+---------------+---------------+---------------+---------------+------*/
struct ECDbSchemaFixture:public::testing::Test
    {
    public:
        ECSchemaPtr     MappingSchema;
        BeFileName      outputRoot, applicationDir;
        ECSchemaReadContextPtr MappingSchemaContext;
        ECDb db;
        BeFileName projectFile;
        virtual void SetUp() override;
        void deleteExistingDgnb(WCharCP);

    };
/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Adeel.Shoukat                        04/13
 +---------------+---------------+---------------+---------------+---------------+------*/
void ECDbSchemaFixture::SetUp()
    {
    MappingSchema=nullptr;
    BeTest::GetHost().GetOutputRoot (outputRoot);
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (applicationDir);
    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    ECDb::Initialize (temporaryDir, &applicationDir);

    MappingSchemaContext=nullptr;
    }
/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Adeel.Shoukat                        04/13
 +---------------+---------------+---------------+---------------+---------------+------*/
void ECDbSchemaFixture::deleteExistingDgnb(WCharCP ECDbName)
    {
    db.CloseDb();
    projectFile=BeFileName (nullptr, outputRoot.GetName(),ECDbName, nullptr);
    if (BeFileName::DoesPathExist (projectFile.GetName()))
        {
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (projectFile.GetName());
        ASSERT_TRUE (fileDeleteStatus == BeFileNameStatus::Success)  << "Could not delete preexisting test ecdb file '" << projectFile.GetName () << "'.";
        }
    
    }
/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Adeel.Shoukat                        04/13
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbSchemaFixture,SchemaMapCustomAttributeTablePrefix)
    {
    ECDbTestUtility::ReadECSchemaFromDisk(MappingSchema,MappingSchemaContext,L"SchemaMapping.01.00.ecschema.xml");
    SchemaKey schemaKey ("ECDbMap", 1, 0);
    ECSchemaPtr ecdbMapSchema = MappingSchemaContext->LocateSchema(schemaKey, SCHEMAMATCHTYPE_LatestCompatible);
    EXPECT_TRUE(ecdbMapSchema != nullptr) << "Schema '" << schemaKey.GetFullSchemaName().c_str() << "' not found.";
    ECClassCP testClass = ecdbMapSchema->GetClassCP("SchemaMap");
    IECInstancePtr ecInctance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ecInctance->SetValue("TablePrefix", ECValue("Pre"));
    MappingSchema->SetCustomAttribute(*ecInctance);
    WCharCP fileName=L"SchemaHintTablePrefix.ecdb";
    deleteExistingDgnb(fileName);
    DbResult stat = db.CreateNewDb (projectFile.GetNameUtf8 ().c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test ECDb file failed.";
    auto status = db. Schemas ().ImportECSchemas (MappingSchemaContext->GetCache (), ECDbSchemaManager::ImportOptions (false, false));
    ASSERT_EQ (SUCCESS, status);
    EXPECT_TRUE(db.TableExists("Pre_A"));
    EXPECT_TRUE(db.TableExists("Pre_ArrayOfB"));
    EXPECT_TRUE(db.TableExists("Pre_ArrayOfC"));
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Adeel.Shoukat                        04/13
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbSchemaFixture,ClassMapCustomAttributeOwnTableNonPolymorphic)
    {
    ECSchemaReadContextPtr MappingSchemaContext=ECSchemaReadContext::CreateContext();
    ECDbTestUtility::ReadECSchemaFromDisk(MappingSchema,MappingSchemaContext,L"SchemaMapping.01.00.ecschema.xml");
    SchemaKey schemaKey ("ECDbMap", 1, 0);
    ECSchemaPtr ecdbMapSchema = MappingSchemaContext->LocateSchema(schemaKey, SCHEMAMATCHTYPE_LatestCompatible);
    EXPECT_TRUE(ecdbMapSchema != nullptr) << "Schema '" << schemaKey.GetFullSchemaName().c_str() << "' not found.";
    ECClassCP testClass = ecdbMapSchema->GetClassCP("ClassMap");
    IECInstancePtr ecInctance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ecInctance->SetValue("MapStrategy.Strategy", ECValue("OwnTable"));
    MappingSchema->GetClassP("B")->SetCustomAttribute(*ecInctance);
    WCharCP fileName=L"OwnTableNonPolymorphic.ecdb";
    deleteExistingDgnb(fileName);
    DbResult stat = db.CreateNewDb (projectFile.GetNameUtf8 ().c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test ECDb file failed.";
    auto status = db. Schemas ().ImportECSchemas (MappingSchemaContext->GetCache (), ECDbSchemaManager::ImportOptions (false, false));
    ASSERT_EQ (SUCCESS, status);
    EXPECT_TRUE(db.TableExists("sm_ArrayOfB"));
    }
/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Adeel.Shoukat                        04/13
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbSchemaFixture, ClassMapCustomAttributeOwnTablePolymorphic)
    {
    ECSchemaReadContextPtr MappingSchemaContext=ECSchemaReadContext::CreateContext();
    ECDbTestUtility::ReadECSchemaFromDisk(MappingSchema,MappingSchemaContext,L"SchemaMapping.01.00.ecschema.xml");
    SchemaKey schemaKey ("ECDbMap", 1, 0);
    ECSchemaPtr ecdbMapSchema = MappingSchemaContext->LocateSchema(schemaKey, SCHEMAMATCHTYPE_LatestCompatible);
    EXPECT_TRUE(ecdbMapSchema != nullptr) << "Schema '" << schemaKey.GetFullSchemaName().c_str() << "' not found.";
    ECClassCP testClass = ecdbMapSchema->GetClassCP("ClassMap");
    IECInstancePtr ecInctance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ecInctance->SetValue("MapStrategy.Strategy", ECValue("OwnTable"));
    ecInctance->SetValue("MapStrategy.AppliesToSubclasses", ECValue(true));
    MappingSchema->GetClassP("B")->SetCustomAttribute(*ecInctance);
    WCharCP fileName=L"OwnTablePolymorphic.ecdb";
    deleteExistingDgnb(fileName);
    DbResult stat = db.CreateNewDb (projectFile.GetNameUtf8 ().c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test ECDb file failed.";
    auto status = db. Schemas ().ImportECSchemas (MappingSchemaContext->GetCache (), ECDbSchemaManager::ImportOptions (false, false));
    ASSERT_EQ (SUCCESS, status);
    EXPECT_TRUE(db.TableExists("sm_ArrayOfB"));
    }
/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Adeel.Shoukat                        04/13
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbSchemaFixture, ClassMapCustomAttributeNotMapped)
    {
    ECSchemaReadContextPtr MappingSchemaContext=ECSchemaReadContext::CreateContext();
    ECDbTestUtility::ReadECSchemaFromDisk(MappingSchema,MappingSchemaContext,L"SchemaMapping.01.00.ecschema.xml");
    SchemaKey schemaKey ("ECDbMap", 1, 0);
    ECSchemaPtr ecdbMapSchema = MappingSchemaContext->LocateSchema(schemaKey, SCHEMAMATCHTYPE_LatestCompatible);
    EXPECT_TRUE(ecdbMapSchema != nullptr) << "Schema '" << schemaKey.GetFullSchemaName().c_str() << "' not found.";
    ECClassCP testClass = ecdbMapSchema->GetClassCP("ClassMap");
    IECInstancePtr ecInctance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ecInctance->SetValue("MapStrategy.Strategy", ECValue("NotMapped"));
    MappingSchema->GetClassP("B")->SetCustomAttribute(*ecInctance);
    WCharCP fileName=L"NotMappedClassMapping.ecdb";
    deleteExistingDgnb(fileName);
    DbResult stat = db.CreateNewDb (projectFile.GetNameUtf8 ().c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test ECDb file failed.";
    auto status = db. Schemas ().ImportECSchemas (MappingSchemaContext->GetCache (), ECDbSchemaManager::ImportOptions (false, false));
    ASSERT_EQ (SUCCESS, status);
    EXPECT_FALSE(db.TableExists("sm_ArrayOfB"));
    }
/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Adeel.Shoukat                        04/13
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbSchemaFixture,ClassMapCustomAttributeSharedTablePolymorphic)
    {
    ECDbTestUtility::ReadECSchemaFromDisk(MappingSchema,MappingSchemaContext,L"SchemaMapping.01.00.ecschema.xml");
    SchemaKey schemaKey("ECDbMap", 1, 0);
    ECSchemaPtr ecdbMapSchema = MappingSchemaContext->LocateSchema(schemaKey, SCHEMAMATCHTYPE_LatestCompatible);
    EXPECT_TRUE(ecdbMapSchema != nullptr) << "Schema '" << schemaKey.GetFullSchemaName().c_str() << "' not found.";
    ECClassCP testClass = ecdbMapSchema->GetClassCP("ClassMap");
    IECInstancePtr ecInctance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ecInctance->SetValue("MapStrategy.Strategy", ECValue("SharedTable"));
    ecInctance->SetValue("MapStrategy.AppliesToSubclasses", ECValue(true));
    MappingSchema->GetClassP("B")->SetCustomAttribute(*ecInctance);
    WCharCP fileName=L"SharedTablePolymorphicClassMapping.ecdb";
    deleteExistingDgnb(fileName);
    DbResult stat = db.CreateNewDb (projectFile.GetNameUtf8 ().c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test ECDb file failed.";
    auto status = db. Schemas ().ImportECSchemas (MappingSchemaContext->GetCache (), ECDbSchemaManager::ImportOptions (false, false));
    ASSERT_EQ (SUCCESS, status);
    EXPECT_TRUE (db.TableExists ("sm_ArrayOfB"));
    EXPECT_FALSE(db.TableExists("sm_b"));
    EXPECT_FALSE(db.TableExists("sm_ArrayOfA"));
    EXPECT_TRUE(db.TableExists("sm_a"));
    EXPECT_TRUE(db.TableExists("sm_ArrayOfC"));
    }


/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Adeel.Shoukat                        04/13
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbSchemaFixture, ClassMapCustomAttributeNotMappedPolymorphic)
    {
    ECSchemaReadContextPtr MappingSchemaContext=ECSchemaReadContext::CreateContext();
    ECDbTestUtility::ReadECSchemaFromDisk(MappingSchema,MappingSchemaContext,L"SchemaMapping.01.00.ecschema.xml");
    SchemaKey schemaKey ("ECDbMap", 1, 0);
    ECSchemaPtr ecdbMapSchema =  MappingSchemaContext->LocateSchema(schemaKey,SCHEMAMATCHTYPE_LatestCompatible);
    EXPECT_TRUE(ecdbMapSchema != nullptr) << "Schema '" << schemaKey.GetFullSchemaName().c_str() << "' not found.";
    ECClassCP testClass = ecdbMapSchema->GetClassCP("ClassMap");
    IECInstancePtr ecInctance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ecInctance->SetValue("MapStrategy.Strategy", ECValue("NotMapped"));
    ecInctance->SetValue("MapStrategy.AppliesToSubclasses", ECValue(true));
    MappingSchema->GetClassP("B")->SetCustomAttribute(*ecInctance);
    WCharCP fileName=L"NotMappedPolymorphicClassMapping.ecdb";
    deleteExistingDgnb(fileName);
    DbResult stat = db.CreateNewDb (projectFile.GetNameUtf8 ().c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test ECDb file failed.";
    auto status = db. Schemas ().ImportECSchemas (MappingSchemaContext->GetCache (), ECDbSchemaManager::ImportOptions (false, false));
    ASSERT_EQ (SUCCESS, status);
    EXPECT_FALSE (db.TableExists ("sm_ArrayOfB"));
    EXPECT_FALSE(db.TableExists("sm_B"));
    EXPECT_TRUE(db.TableExists("sm_A"));
    EXPECT_TRUE(db.TableExists("sm_ArrayOfC"));
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Affan.Khan                        06/13
 +---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr GenerateNewECSchema(Utf8StringCR name, Utf8StringCR prefix)
    {
    ECSchemaPtr newSchema;
    ECSchema::CreateSchema(newSchema, name, 1, 0);
    newSchema->SetNamespacePrefix(prefix);
    ECClassP equipment, pipe;
    PrimitiveECPropertyP primitveP;
    newSchema->CreateClass(equipment, "EQUIPMENT");
    equipment->CreatePrimitiveProperty(primitveP, "NAME", PrimitiveType::PRIMITIVETYPE_String);
    equipment->CreatePrimitiveProperty(primitveP, "EQID", PrimitiveType::PRIMITIVETYPE_Integer);
    newSchema->CreateClass(pipe, "PIPE");
    pipe->AddBaseClass(*equipment);
    pipe->CreatePrimitiveProperty(primitveP, "TYPE", PrimitiveType::PRIMITIVETYPE_String);
    pipe->CreatePrimitiveProperty(primitveP, "LENGTH", PrimitiveType::PRIMITIVETYPE_Integer);
    return newSchema;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Affan.Khan                        06/13
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, Verify_TFS_14829_A)
   {
    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create ("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", false);
    //auto fileName =saveTestProject.GetECDb().GetDbFileName();
    //db.CloseDb();

    //DbResult stat = db.OpenBeSQLiteDb (fileName, Db::OpenParams(Db::OpenMode::ReadWrite));
    //EXPECT_EQ (BE_SQLITE_OK, stat);

    bool bUpdate = true;
    ECSchemaPtr ecSchema = nullptr;
    ECSchemaReadContextPtr schemaContext = nullptr;

    ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, schemaContext, L"StartupCompany.02.00.ecschema.xml");
    auto schemaStatus = db. Schemas ().ImportECSchemas (schemaContext->GetCache (), ECDbSchemaManager::ImportOptions (true, bUpdate));
    ASSERT_EQ (SUCCESS, schemaStatus);

    ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, schemaContext, L"SimpleCompany.01.00.ecschema.xml");
    schemaStatus = db. Schemas ().ImportECSchemas (schemaContext->GetCache (), ECDbSchemaManager::ImportOptions (true, bUpdate));
    ASSERT_EQ (SUCCESS, schemaStatus);

    ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, schemaContext, L"RSComponents.01.00.ecschema.xml");
    schemaStatus = db. Schemas ().ImportECSchemas (schemaContext->GetCache (), ECDbSchemaManager::ImportOptions (true, bUpdate));
    ASSERT_EQ (SUCCESS, schemaStatus);

    ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, schemaContext, L"RSComponents.02.00.ecschema.xml");
    ecSchema->SetVersionMajor(1);
    ecSchema->SetVersionMinor(22);
    schemaStatus = db. Schemas ().ImportECSchemas (schemaContext->GetCache (), ECDbSchemaManager::ImportOptions (true, bUpdate));
    ASSERT_EQ (SUCCESS, schemaStatus);
   }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Affan.Khan                        06/13
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, Verify_TFS_14829_B)
   {
    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create ("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", false);

    ECSchemaPtr ecSchema = nullptr;
    ECSchemaReadContextPtr schemaContext = nullptr;

    ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, schemaContext, L"StartupCompany.02.00.ecschema.xml");
    auto schemaStatus = db.Schemas().ImportECSchemas (schemaContext->GetCache());
    ASSERT_EQ (SUCCESS, schemaStatus);

    ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, schemaContext, L"SimpleCompany.01.00.ecschema.xml");
    schemaStatus = db.Schemas().ImportECSchemas (schemaContext->GetCache());
    ASSERT_EQ (SUCCESS, schemaStatus);

    ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, schemaContext, L"RSComponents.01.00.ecschema.xml");
    schemaStatus = db.Schemas().ImportECSchemas (schemaContext->GetCache());
    ASSERT_EQ (SUCCESS, schemaStatus);

    ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, schemaContext, L"RSComponents.02.00.ecschema.xml");
    schemaStatus = db.Schemas().ImportECSchemas (schemaContext->GetCache());
    ASSERT_EQ (SUCCESS, schemaStatus);
   }


//=====Affan/Assert/Test============================================================================================
//struct ThreadArg
//    {
//    enum class AssertType
//        {
//        Bentley,
//        GTest
//        };
//    private:
//        BeConditionVariable m_aConditionVariable;
//        AssertType m_aAssertType;
//
//        static void _GAssert ()
//            {
//            ASSERT_TRUE (false && "GTest Assert failed from thread");
//            }
//        static void _BeAssert ()
//            {
//            BeAssert (false && "BeAssert failed from thread");
//            }
//    public:
//        BeConditionVariable& GetConditionVariable () { return m_aConditionVariable; }
//        AssertType GetAssertType ()
//            {
//            return m_aAssertType;
//            }
//        void Wait ()
//            {
//            m_aConditionVariable.WaitOnCondition (nullptr, BeConditionVariable::Infinite);
//            }
//        ThreadArg (AssertType type)
//            :m_aAssertType (type)
//            {
//            }
//
//        static unsigned  RunTest (void* args)
//            {
//            ThreadArg* threadArg = static_cast<ThreadArg*>(args);
//            BeCriticalSectionHolder aGuard (threadArg->GetConditionVariable ().GetCriticalSection ());
//            BeThreadUtilities::BeSleep (1000);
//            switch (threadArg->GetAssertType ())
//                {
//                    case ThreadArg::AssertType::Bentley:
//                        {
//                        _BeAssert ();
//                        } break;
//                    case ThreadArg::AssertType::GTest:
//                        {
//                        _GAssert ();
//                        } break;
//                    default:
//                        break;
//                }
//
//            threadArg->GetConditionVariable ().Wake (true);
//            return 0;
//            }
//    };
//
//
//
//TEST (ThreadingTest, BentleyAssert)
//    {
//    auto arg = ThreadArg (ThreadArg::AssertType::Bentley);
//    BeThreadUtilities::StartNewThread (10, ThreadArg::RunTest, &arg);
//    arg.Wait ();
//    }
//
//TEST (ThreadingTest, GTestAssert)
//    {
//    auto arg = ThreadArg (ThreadArg::AssertType::GTest);
//    BeThreadUtilities::StartNewThread (10, ThreadArg::RunTest, &arg);
//    arg.Wait ();
//    }
//================================================================================================================
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, IntegrityCheck)
    {
    // Save a test project
    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create("IntegrityCheck.ecdb", L"IntegrityCheck.01.00.ecschema.xml", true);
    Statement stmt;
    std::map<Utf8String, Utf8String> expected;
    expected["ic_TargetBase"] = "CREATE TABLE [ic_TargetBase] ([ECInstanceId] INTEGER NOT NULL, [ECClassId] INTEGER, [I] INTEGER, [S] TEXT, [SourceECInstanceId] INTEGER, PRIMARY KEY ([ECInstanceId]), FOREIGN KEY ([SourceECInstanceId]) REFERENCES [ic_SourceBase] ([ECInstanceId]) ON DELETE CASCADE ON UPDATE NO ACTION)";

    stmt.Prepare(db, "select name, sql from sqlite_master Where type='table' AND tbl_name = 'ic_TargetBase'");
    int nRows = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        nRows = nRows + 1;
        Utf8String name = stmt.GetValueText(0);
        Utf8String sql = stmt.GetValueText(1);
        auto itor = expected.find(name);
        if (itor == expected.end())
            {
            ASSERT_FALSE(true) << "Failed to find expected value [name=" << name << "]";
            }
        if (itor->second != sql)
            {
            ASSERT_FALSE(true) << "SQL def for  [name=" << name << "] has changed \r\n Expected :" << itor->second.c_str() << "\r\n Actual : " << sql.c_str();
            }
        }

    ASSERT_EQ(nRows, expected.size()) << "Number of SQL definitions are not same";
    }
TEST(ECDbSchemas, CheckClassHasCurrentTimeStamp)
    {
    const Utf8CP schema =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"SimpleSchema\" nameSpacePrefix=\"adhoc\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "<ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.11\" prefix=\"besc\" />"
        "<ECClass typeName=\"SimpleClass\" isStruct=\"False\" isDomainClass=\"True\">"
        "<ECProperty propertyName = \"DateTimeProperty\" typeName=\"dateTime\" readOnly=\"True\" />"
        "<ECProperty propertyName = \"testprop\" typeName=\"int\" />"
        "<ECCustomAttributes>"
        "<ClassHasCurrentTimeStampProperty xmlns=\"Bentley_Standard_CustomAttributes.01.11\">"
        "<PropertyName>DateTimeProperty</PropertyName>"
        "</ClassHasCurrentTimeStampProperty>"
        "</ECCustomAttributes>"
        "</ECClass>"
        "</ECSchema>";

    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create("checkClassHasCurrentTimeStamp.ecdb");
    ECSchemaPtr simpleSchema;
    auto readContext = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(simpleSchema, schema, *readContext);
    ASSERT_TRUE(simpleSchema != nullptr);
    auto importStatus = db.Schemas().ImportECSchemas(readContext->GetCache());
    ASSERT_TRUE(importStatus == BentleyStatus::SUCCESS);
    auto ecClass = simpleSchema->GetClassP("SimpleClass");

    ECSqlStatement insertStatement;
    Utf8CP insertQuery = "INSERT INTO adhoc.SimpleClass(testprop) VALUES(12)";
    ASSERT_TRUE(ECSqlStatus::Success == insertStatement.Prepare(db, insertQuery));
    insertStatement.Step();
    db.SaveChanges();
    Utf8String ecsql("SELECT DateTimeProperty FROM ");
    ecsql.append(ECSqlBuilder::ToECSqlSnippet(*ecClass));
    db.SaveChanges();
    ECSqlStatement statement;
    auto stat = statement.Prepare(db, ecsql.c_str());
    ASSERT_EQ(ECSqlStatus::Success, stat);

    ASSERT_EQ(ECSqlStatus::Success, stat);
    BentleyApi::DateTime dateTime1;
    ASSERT_TRUE(statement.Step() == BE_SQLITE_ROW);
        {
        ASSERT_FALSE(statement.IsValueNull(0));
        dateTime1 = statement.GetValueDateTime(0);
        }
    ASSERT_TRUE(statement.Step() == BE_SQLITE_DONE);
    ECSqlStatement updateStatment;
    ecsql = "UPDATE ONLY adhoc.SimpleClass SET testprop = 23 WHERE ECInstanceId = 1";
    stat = updateStatment.Prepare(db, ecsql.c_str());
    ASSERT_TRUE(updateStatment.Step() == BE_SQLITE_DONE);
    ecsql = "SELECT DateTimeProperty FROM ";
    ecsql.append(ECSqlBuilder::ToECSqlSnippet(*ecClass));

    BentleyApi::DateTime dateTime2;
    ECSqlStatement statement2;
    stat = statement2.Prepare(db, ecsql.c_str());
    BeThreadUtilities::BeSleep(100); // make sure the time is different by more than the resolution of the timestamp

    ASSERT_TRUE(statement2.Step() == BE_SQLITE_ROW);
        {
        ASSERT_FALSE(statement2.IsValueNull(0));
        dateTime2 = statement2.GetValueDateTime(0);
        }
    ASSERT_TRUE(statement2.Step() == BE_SQLITE_DONE);
    ASSERT_FALSE(dateTime1 == dateTime2);
    }
END_ECDBUNITTESTS_NAMESPACE
