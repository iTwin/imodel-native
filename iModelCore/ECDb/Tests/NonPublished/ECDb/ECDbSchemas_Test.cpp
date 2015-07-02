/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/ECDb/ECDbSchemas_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/NonPublished/ECDb/ECDbTestProject.h>
#include <initializer_list>
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool TableExist(DbR db, WCharCP table)
    {
    Utf8String tableName(table);
    return db.TableExists(tableName.c_str());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/12
+---------------+---------------+---------------+---------------+---------------+------*/
static int GetColumnCount(DbR db, WCharCP table)
    {
    Utf8String tableName(table);
    Statement stmt;
    stmt.Prepare (db, SqlPrintfString("SELECT * FROM %s LIMIT 1", tableName.c_str()));
    return stmt.GetColumnCount();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ColumnExist(DbR db, WCharCP table, WCharCP column)
    {
    Utf8String tableName(table);
    Utf8String columnName(column);
    return db.ColumnExists(tableName.c_str(), columnName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteECSchemaDiffToLog (ECDiffR diff, NativeLogging::SEVERITY severity = NativeLogging::LOG_INFO)
    {
    WString diffString;
    ASSERT_EQ (diff.WriteToString(diffString, 2), DIFFSTATUS_Success);
    LOG.message (severity,  L"ECDiff: Legend [L] Added from left schema, [R] Added from right schema, [!] conflicting value");
    LOG.message (severity, L"=====================================[ECDiff Start]=====================================");
    //LOG doesnt allow single large string
    WString eol = L"\r\n";
    WString::size_type i = 0;
    WString::size_type j = diffString.find (eol, i);
    while ( j > i && j != WString::npos)
        {
        WString line = diffString.substr (i, j - i);
        LOG.messagev (severity, L"> %ls" , line.c_str()); //print out the difference
        i = j + eol.size();
        j = diffString.find (eol, i);
        }
    LOG.message (severity, L"=====================================[ECDiff End]=====================================");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                          04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PopulatePrimitiveValueWithCustomDataSet2 (ECValueR value, PrimitiveType primitiveType, ECPropertyCP ecProperty)
    {
    value.Clear();
    switch (primitiveType)
        {
        case PRIMITIVETYPE_String  : value.SetString(L"Tim Cook"); break;
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
        WString schemaName;
        uint32_t schemaMajor, schemaMinor;
        if (ECSchema::ParseSchemaFullName (schemaName, schemaMajor, schemaMinor, schemaFullName.substr (0, extPos)) == ECOBJECTS_STATUS_Success)
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
    //hlpPrintStdOut (L"Importing EC Schema [%ls]\n", ecSchemaFile.GetName ());

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
        L"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        L"<ECSchema schemaName=\"OrderSchema\" nameSpacePrefix=\"os\" version=\"1.0\" xmlns = \"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        L"  <ECClass typeName=\"OrderedStruct\" isDomainClass=\"True\" isStruct =\"True\">"
        L"   <ECProperty propertyName=\"a\" typeName=\"string\"/>"
        L"	 <ECProperty propertyName=\"g\" typeName=\"integer\"/>"
        L"	 <ECProperty propertyName=\"c\" typeName=\"dateTime\"/>"
        L"   <ECProperty propertyName=\"z\" typeName=\"point3d\"/>"
        L"	 <ECProperty propertyName=\"y\" typeName=\"point2d\"/>"
        L"	 <ECProperty propertyName=\"t\" typeName=\"boolean\"/>"
        L"   <ECProperty propertyName=\"u\" typeName=\"double\"/>"
        L"	 <ECProperty propertyName=\"k\" typeName=\"string\"/>"
        L"	 <ECProperty propertyName=\"r\" typeName=\"string\"/>"
        L"  </ECClass>"
        L"  <ECClass typeName=\"PropertyOrderTest\" isDomainClass=\"True\" isStruct =\"True\">"
        L"   <ECProperty propertyName=\"x\" typeName=\"string\"/>"
        L"	 <ECProperty propertyName=\"h\" typeName=\"integer\"/>"
        L"	 <ECProperty propertyName=\"i\" typeName=\"dateTime\"/>"
        L"   <ECProperty propertyName=\"d\" typeName=\"point3d\"/>"
        L"	 <ECProperty propertyName=\"u\" typeName=\"point2d\"/>"
        L"	 <ECProperty propertyName=\"f\" typeName=\"boolean\"/>"
        L"   <ECProperty propertyName=\"e\" typeName=\"double\"/>"
        L"	 <ECProperty propertyName=\"p\" typeName=\"string\"/>"
        L"	 <ECStructProperty propertyName=\"o\" typeName=\"OrderedStruct\"/>"
        L"	 <ECProperty propertyName=\"z\" typeName=\"long\"/>"
        L"  </ECClass>"
        L"</ECSchema>";

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
        bmap<WString, DiffNodeState> searchResults;
        diff->GetNodesState(searchResults, L"*.ArrayInfo");
        if (!searchResults.empty())
            LOG.error(L"*** Feature missing : Array type property Maxoccurs and Minoccurs are not stored currently by ECDbSchemaManager");
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
            LOG.errorv(L"Failed to create ECInstanceInserter for %ls", ecClass->GetName().c_str());
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

TEST(ECDbSchemas, UpdateExistingECSchemaWithNewProperties)
    {
    ECDbTestProject testProject;
    ECDbR db = testProject.Create("updateSchemaMinorVersion.ecdb");

    ECSchemaPtr schema12;
    ECSchema::CreateSchema(schema12, L"TestSchema", 1, 2);
    schema12->SetNamespacePrefix(L"ts");
    schema12->SetDescription(L"Schema for testing upgrades");
    schema12->SetDisplayLabel(L"Test Schema");

    ECClassP widget12;
    schema12->CreateClass(widget12, L"WIDGET");
    PrimitiveECPropertyP stringProp12;
    widget12->CreatePrimitiveProperty(stringProp12, L"propA");

    auto schemaCache12 = ECSchemaCache::Create ();
    schemaCache12->AddSchema (*schema12);

    auto importSchemaStatus = db.Schemas().ImportECSchemas (*schemaCache12);
    ASSERT_EQ (SUCCESS, importSchemaStatus);

    Utf8String ecdbFileName = Utf8String(db.GetDbFileName ());
    db.CloseDb();
    db.OpenBeSQLiteDb(ecdbFileName.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));

    ECSchemaPtr schema11;
    ECSchema::CreateSchema(schema11, L"TestSchema", 1, 1);
    schema11->SetNamespacePrefix(L"ts");
    schema11->SetDescription(L"Schema for testing upgrades");
    schema11->SetDisplayLabel(L"Test Schema");

    ECClassP widget11;
    schema11->CreateClass(widget11, L"WIDGET");
    PrimitiveECPropertyP stringProp11;
    widget11->CreatePrimitiveProperty(stringProp11, L"propA");

    auto schemaCache11 = ECSchemaCache::Create ();
    schemaCache11->AddSchema (*schema11);
    importSchemaStatus = db.Schemas ().ImportECSchemas (*schemaCache11, ECDbSchemaManager::ImportOptions (true, true));
    ASSERT_EQ(ERROR, importSchemaStatus);

    ECSchemaPtr schema13;
    ECSchema::CreateSchema(schema13, L"TestSchema", 1, 3);
    schema13->SetNamespacePrefix(L"ts");
    schema13->SetDescription(L"Schema for testing upgrades");
    schema13->SetDisplayLabel(L"Test Schema");

    ECClassP widget13;
    ECClassP gadget13;
    schema13->CreateClass(widget13, L"WIDGET");
    schema13->CreateClass(gadget13, L"GADGET");
    PrimitiveECPropertyP stringProp13;
    PrimitiveECPropertyP intProp13;
    widget13->CreatePrimitiveProperty(stringProp13, L"propA");
    widget13->CreatePrimitiveProperty(intProp13, L"propB");

    auto schemaCache13 = ECSchemaCache::Create ();
    schemaCache13->AddSchema (*schema13);
    importSchemaStatus = db. Schemas ().ImportECSchemas (*schemaCache13, ECDbSchemaManager::ImportOptions (true, true));
    ASSERT_EQ(SUCCESS, importSchemaStatus);
    db.CloseDb();
    db.OpenBeSQLiteDb(ecdbFileName.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));

    ECSchemaCP updatedECSchema = db. Schemas ().GetECSchema ("TestSchema");
    ASSERT_TRUE (updatedECSchema != nullptr);

    ECClassCP updatedGadget = updatedECSchema->GetClassCP(L"GADGET");
    ASSERT_TRUE(nullptr != updatedGadget);
    ECClassCP updatedWidget = updatedECSchema->GetClassCP(L"WIDGET");
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
    ASSERT_EQ (ColumnExist (ecDb, L"DSC_CachedFileInfo", L"ForeignECClassId_CachedFileInfoRelationship"), true);
    ASSERT_EQ (ColumnExist (ecDb, L"DSC_CachedFileInfo", L"ForeignECInstanceId_CachedFileInfoRelationship"), true);

    ASSERT_EQ (ColumnExist (ecDb, L"DSC_CachedInstanceInfo", L"ForeignECInstanceId_CachedInstanceInfoRelationship"), true);
    ASSERT_EQ (ColumnExist (ecDb, L"DSC_CachedInstanceInfo", L"ForeignECClassId_CachedInstanceInfoRelationship"), true);

    ASSERT_EQ (ColumnExist (ecDb, L"DSCJS_RootRelationship", L"SourceECInstanceId"), true);
    ASSERT_EQ (ColumnExist (ecDb, L"DSCJS_RootRelationship", L"TargetECInstanceId"), true);
    ASSERT_EQ (ColumnExist (ecDb, L"DSCJS_RootRelationship", L"TargetECClassId"), true);

    ASSERT_EQ (ColumnExist (ecDb, L"DSCJS_NavigationBaseRelationship", L"SourceECInstanceId"), true);
    ASSERT_EQ (ColumnExist (ecDb, L"DSCJS_NavigationBaseRelationship", L"TargetECInstanceId"), true);
    ASSERT_EQ (ColumnExist (ecDb, L"DSCJS_NavigationBaseRelationship", L"TargetECClassId"), true);

    auto ecsql = "SELECT s.* FROM ONLY [DSC].[CachedInstanceInfo] s JOIN ONLY [DSC].[NavigationBase] t USING [DSCJS].[CachedInstanceInfoRelationship] FORWARD WHERE t.ECInstanceId = 8 LIMIT 1";

    ECSqlStatement stmt;
    auto prepareStatus = stmt.Prepare (ecDb, ecsql);
    ASSERT_TRUE(prepareStatus ==  ECSqlStatus::Success);
    auto stepStatus = stmt.Step ();
    ASSERT_TRUE (stepStatus == ECSqlStepStatus::HasRow);
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
   
    bset<WString> expectedSchemas;
    expectedSchemas.insert (L"Bentley_Standard_CustomAttributes");
    expectedSchemas.insert (L"Bentley_Standard_Classes");
    expectedSchemas.insert (L"EditorCustomAttributes");
    expectedSchemas.insert(L"ECDbMap");
    expectedSchemas.insert(L"ECDbSystem");
    expectedSchemas.insert (L"ECDb_FileInfo");
    expectedSchemas.insert (L"StartupCompany");
    expectedSchemas.insert (L"Unit_Attributes");

    // Validate the expected ECSchemas in the project
    Statement stmt;
    stmt.Prepare (db, "SELECT NAME FROM ec_Schema");
    int nSchemas = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        nSchemas++;
        WString schemaName;
        schemaName.AssignUtf8(stmt.GetValueText(0));
        if (expectedSchemas.end() == expectedSchemas.find(schemaName))
            LOG.errorv(L"Found unexpected ECSchema '%ls'", schemaName.c_str());
        }
    EXPECT_EQ (expectedSchemas.size(), nSchemas);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                       10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void AssertParseECSql (Utf8CP ecsql)
    {
    Utf8String parseTree, error;
    ECSqlParseTreeFormatter::ParseAndFormatECSqlParseNodeTree (parseTree, error, ecsql);
    if (!error.empty())
        {
        ASSERT_TRUE(FALSE) << "Failed to parse ECSQL:" << error;
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                       10/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECSqlParseTest, ForAndroid)
    {
    AssertParseECSql ("SELECT '''' FROM stco.Hardware");
    AssertParseECSql ("SELECT 'aa', '''', b FROM stco.Hardware WHERE Name = 'a''b'");
    AssertParseECSql ("SELECT _Aa, _bC, _123, Abc, a123, a_123, a_b, _a_b_c FROM stco.Hardware WHERE Name = 'Fusion'");
    AssertParseECSql ("SELECT * FROM stco.Hardware WHERE Name = 'Fusion'");
    AssertParseECSql ("SELECT [Foo].[Name] FROM stco.[Hardware] [Foo]");
    AssertParseECSql ("SELECT [Foo].[Name] FROM stco.[Hardware] [Foo] WHERE [Name] = 'HelloWorld'");
    AssertParseECSql ("Select EQUIP_NO From only appdw.Equipment where EQUIP_NO = '50E-101A' ");
    AssertParseECSql ("INSERT INTO [V8TagsetDefinitions].[STRUCTURE_IL1] ([VarFixedStartZ], [DeviceID1], [ObjectType], [PlaceMethod], [CopyConstrDrwToProj]) VALUES ('?', '-E1-1', 'SGL', '1', 'Y')");
    AssertParseECSql ("INSERT INTO [V8TagsetDefinitions].[grid__x0024__0__x0024__CB_1] ([CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457],[CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454]) VALUES ('', '1.1', '', '', '', '2.2', '', '', '', '2.5', '', '', '', '2.5', '', '', '', '2.1', '', '', '', 'E.3', '', '', '', 'B.4', '', '', '', 'D.4', '', '')");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                       10/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas, VerifyEmptyECSchemaCanBeRead)
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
    ECSchema::CreateSchema(emptySchema,L"EmptyECSchema", 1, 0);
    auto cache = ECSchemaCache::Create();
    cache->AddSchema (*emptySchema);
    auto schemaStat = ecdb.Schemas().ImportECSchemas(*cache);
    ASSERT_EQ (SUCCESS, schemaStat) << "Importing empty ECSchema failed";
    ecdb.ClearECDbCache();
    ECSchemaCP stroedEmptySchema = ecdb. Schemas ().GetECSchema ("EmptyECSchema");
    ASSERT_TRUE (stroedEmptySchema != nullptr);
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
   
    WCharCP tblClassWithPrimitiveProperties = L"sc_ClassWithPrimitiveProperties";
    EXPECT_TRUE (TableExist  (db, tblClassWithPrimitiveProperties));
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveProperties, L"ECInstanceId"));
    EXPECT_EQ   (13, GetColumnCount(db, tblClassWithPrimitiveProperties));
    //Verify columns columns in this class is renamed to 
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveProperties, L"myColumn_intProp"));
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveProperties, L"myColumn_longProp"));
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveProperties, L"myColumn_doubleProp"));
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveProperties, L"myColumn_stringProp"));
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveProperties, L"myColumn_dateTimeProp"));
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveProperties, L"myColumn_binaryProp"));
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveProperties, L"myColumn_booleanProp"));
    //point2Prop is stored as x,y 2 columns
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveProperties, L"myColumn_point2dProp_X"));
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveProperties, L"myColumn_point2dProp_Y"));
    //point3Prop is stored as x,y,z 3 columns
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveProperties, L"myColumn_point3dProp_X"));
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveProperties, L"myColumn_point3dProp_Y"));
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveProperties, L"myColumn_point3dProp_Z"));

    //========================[sc_StructWithPrimitiveProperties==================================
    WCharCP tblStructWithPrimitiveProperties = L"sc_ArrayOfStructWithPrimitiveProperties";
    EXPECT_TRUE (TableExist  (db, tblStructWithPrimitiveProperties));
    EXPECT_EQ   (16, GetColumnCount(db, tblStructWithPrimitiveProperties));
    ASSERT_TRUE (ColumnExist (db, tblStructWithPrimitiveProperties, L"ECInstanceId"));
    ASSERT_TRUE (ColumnExist (db, tblStructWithPrimitiveProperties, L"ParentECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveProperties, L"ECPropertyPathId"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveProperties, L"ECArrayIndex"));

    //Verify columns
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveProperties, L"intProp"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveProperties, L"longProp"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveProperties, L"doubleProp"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveProperties, L"stringProp"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveProperties, L"dateTimeProp"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveProperties, L"binaryProp"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveProperties, L"booleanProp"));
    //point2Prop is stored as x,y 2 columns
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveProperties, L"point2dProp_X"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveProperties, L"point2dProp_Y"));
    //point3Prop is stored as x,y,z 3 columns
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveProperties, L"point3dProp_X"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveProperties, L"point3dProp_Y"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveProperties, L"point3dProp_Z"));

    //========================[sc_ClassWithPrimitiveArrayProperties==============================
    //Array properties doesnt take any column currently it will take in case of embeded senario but
    //we need to make sure it doesnt exist right now. They uses special System arrray tables 
    WCharCP tblClassWithPrimitiveArrayProperties = L"sc_ClassWithPrimitiveArrayProperties";
    EXPECT_TRUE (TableExist  (db, tblClassWithPrimitiveArrayProperties));
    EXPECT_EQ   (10, GetColumnCount(db, tblClassWithPrimitiveArrayProperties));    
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveArrayProperties, L"ECInstanceId"));

    //Verify columns
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveArrayProperties, L"intArrayProp"));
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveArrayProperties, L"longArrayProp"));
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveArrayProperties, L"doubleArrayProp"));
    EXPECT_TRUE  (ColumnExist (db, tblClassWithPrimitiveArrayProperties, L"stringArrayProp"));// MapStrategy=Blob
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveArrayProperties, L"dateTimeArrayProp"));
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveArrayProperties, L"binaryArrayProp"));
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveArrayProperties, L"booleanArrayProp"));
    EXPECT_TRUE (ColumnExist (db, tblClassWithPrimitiveArrayProperties, L"point2dArrayProp"));
    EXPECT_TRUE  (ColumnExist (db, tblClassWithPrimitiveArrayProperties, L"point3dArrayProp")); // MapStrategy=Blob

    //========================[sc_StructWithPrimitiveArrayProperties=============================
    //Array properties doesnt have any column currently it will take in case of embeded senario but
    //we need to make sure it doesnt exist right now. They uses special System arrray tables 
    WCharCP tblStructWithPrimitiveArrayProperties = L"sc_ArrayOfStructWithPrimitiveArrayProperties";
    EXPECT_TRUE (TableExist  (db, tblStructWithPrimitiveArrayProperties));
    EXPECT_EQ   (13, GetColumnCount(db, tblStructWithPrimitiveArrayProperties));    
    ASSERT_TRUE (ColumnExist (db, tblStructWithPrimitiveArrayProperties, L"ECInstanceId"));
    ASSERT_TRUE (ColumnExist (db, tblStructWithPrimitiveArrayProperties, L"ParentECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveArrayProperties, L"ECPropertyPathId"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveArrayProperties, L"ECArrayIndex"));

    //Verify columns
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveArrayProperties, L"intArrayProp"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveArrayProperties, L"longArrayProp"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveArrayProperties, L"doubleArrayProp"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveArrayProperties, L"stringArrayProp"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveArrayProperties, L"dateTimeArrayProp"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveArrayProperties, L"binaryArrayProp"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveArrayProperties, L"booleanArrayProp"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveArrayProperties, L"point2dArrayProp"));
    EXPECT_TRUE (ColumnExist (db, tblStructWithPrimitiveArrayProperties, L"point3dArrayProp"));

    //verify system array tables. They are created if  a primitive array property is ecounter in schema
#if ECDB_CREATE_TABLES_FOR_PRIMITIVE_ARRAYS
    //========================[ec_ArrayOfBinary]================================================
    EXPECT_TRUE (TableExist(db, L"ec_ArrayOfBinary"));
    EXPECT_EQ   (4, GetColumnCount(db, L"ec_ArrayOfBinary"));    
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfBinary", L"ECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfBinary", L"ECPropertyPathId"));    
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfBinary", L"ECArrayIndex"));    
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfBinary", L"ElementValue"));

    //========================[ec_ArrayOfBoolean]===============================================
    EXPECT_TRUE (TableExist(db, L"ec_ArrayOfBoolean"));
    EXPECT_EQ   (4, GetColumnCount(db, L"ec_ArrayOfBoolean"));        
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfBoolean", L"ECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfBoolean", L"ECPropertyPathId"));    
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfBoolean", L"ECArrayIndex"));        
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfBoolean", L"ElementValue"));

    //========================[ec_ArrayOfDateTime]==============================================
    EXPECT_TRUE (TableExist(db, L"ec_ArrayOfDateTime"));
    EXPECT_EQ   (4, GetColumnCount(db, L"ec_ArrayOfDateTime"));        
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfDateTime", L"ECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfDateTime", L"ECPropertyPathId"));    
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfDateTime", L"ECArrayIndex"));            
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfDateTime", L"ElementValue"));

    //========================[ec_ArrayOfDouble]================================================
    EXPECT_TRUE (TableExist(db, L"ec_ArrayOfDouble"));
    EXPECT_EQ   (4, GetColumnCount(db, L"ec_ArrayOfDouble"));        
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfDouble", L"ECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfDouble", L"ECPropertyPathId"));    
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfDouble", L"ECArrayIndex"));            
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfDouble", L"ElementValue"));

    //========================[ec_ArrayOfInteger]===============================================
    EXPECT_TRUE (TableExist(db, L"ec_ArrayOfInteger"));
    EXPECT_EQ   (4, GetColumnCount(db, L"ec_ArrayOfInteger"));        
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfInteger", L"ECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfInteger", L"ECPropertyPathId"));    
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfInteger", L"ECArrayIndex"));            
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfInteger", L"ElementValue"));

    //========================[ec_ArrayOfLong]==================================================
    EXPECT_TRUE (TableExist(db, L"ec_ArrayOfLong"));
    EXPECT_EQ   (4, GetColumnCount(db, L"ec_ArrayOfLong"));        
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfLong", L"ECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfLong", L"ECPropertyPathId"));    
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfLong", L"ECArrayIndex"));            
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfLong", L"ElementValue"));

    //========================[ec_ArrayOfString]================================================
    EXPECT_TRUE (TableExist(db, L"ec_ArrayOfString"));
    EXPECT_EQ   (4, GetColumnCount(db, L"ec_ArrayOfString"));        
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfString", L"ECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfString", L"ECPropertyPathId"));    
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfString", L"ECArrayIndex"));            
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfString", L"ElementValue"));

    //========================[ec_ArrayOfPoint2d]===============================================
    EXPECT_TRUE (TableExist(db, L"ec_ArrayOfPoint2d"));
    EXPECT_EQ   (5, GetColumnCount(db, L"ec_ArrayOfPoint2d"));        
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfPoint2d", L"ECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfPoint2d", L"ECPropertyPathId"));    
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfPoint2d", L"ECArrayIndex"));            
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfPoint2d", L"ElementValue_X"));
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfPoint2d", L"ElementValue_Y"));

    //========================[ec_ArrayOfPoint3d]==============================================
    EXPECT_TRUE (TableExist(db, L"ec_ArrayOfPoint3d"));
    EXPECT_EQ   (6, GetColumnCount(db, L"ec_ArrayOfPoint3d"));        
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfPoint3d", L"ECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfPoint3d", L"ECPropertyPathId"));    
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfPoint3d", L"ECArrayIndex"));            
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfPoint3d", L"ElementValue_X"));
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfPoint3d", L"ElementValue_Y"));
    EXPECT_TRUE (ColumnExist (db, L"ec_ArrayOfPoint3d", L"ElementValue_Z"));
#endif
    //========================[TablePerHieracrchy Test]============================================
    // TablePerHierarchy Should have one table for all base class
    //========================[sc_Asset]=========================================================
    //baseClass
    WCharCP tblAsset = L"sc_Asset";
    EXPECT_TRUE (TableExist  (db, tblAsset));    
    EXPECT_EQ   (8, GetColumnCount(db, tblAsset));            
    EXPECT_TRUE (ColumnExist (db, tblAsset, L"ECInstanceId"));
    
    EXPECT_TRUE (ColumnExist (db, tblAsset, L"AssetID"));
    EXPECT_TRUE (ColumnExist (db, tblAsset, L"AssetOwner"));
    EXPECT_TRUE (ColumnExist (db, tblAsset, L"BarCode"));
    EXPECT_TRUE (ColumnExist (db, tblAsset, L"AssetUserID"));
    EXPECT_TRUE (ColumnExist (db, tblAsset, L"Cost"));
    EXPECT_TRUE (ColumnExist (db, tblAsset, L"Room"));
    EXPECT_TRUE (ColumnExist (db, tblAsset, L"AssetRecordKey")); 
    
    //========================[sc_Furniture]=====================================================
    //TablePerHierarchy
    WCharCP tblFurniture = L"sc_Furniture"; //Drived from Asset
    EXPECT_TRUE (TableExist (db, tblFurniture));
    EXPECT_EQ   (21, GetColumnCount (db, tblFurniture));
    //Table for this child classes of Furniture should not exist    
    EXPECT_FALSE (TableExist (db, L"sc_Desk")); 
    EXPECT_FALSE (TableExist (db, L"sc_Chair"));
    
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"ECInstanceId"));
    //It must have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"ECClassId")); 
    
    //BaseClass properties
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"AssetID"));
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"AssetOwner"));
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"BarCode"));
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"AssetUserID"));
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"Cost"));
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"Room"));
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"AssetRecordKey")); 
    //Local properties of Furniture   
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"Condition")); 
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"Material")); 
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"Weight")); 
    // Properties of Chair which is derived from Furniture
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"ChairFootPrint"));
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"Type")); 
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"Color")); 
    
    // Properties of Desk which is derived from Furniture    
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"DeskFootPrint")); 
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"NumberOfCabinets")); 
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"Size")); 
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"Type")); 
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"Breadth")); 
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"Length")); 
    
    //relation key
    EXPECT_TRUE (ColumnExist (db, tblFurniture, L"Employee__src_01_id")); 
    
    //========================[sc_Employee]======================================================
    //Related to Furniture. Employee can have one or more furniture
    WCharCP tblEmployee = L"sc_Employee";
    EXPECT_TRUE (TableExist  (db, tblEmployee));    
    EXPECT_EQ   (31, GetColumnCount(db, tblEmployee));
    EXPECT_TRUE (ColumnExist (db, tblEmployee, L"ECInstanceId"));
    
    EXPECT_TRUE (ColumnExist (db, tblEmployee, L"EmployeeID"));
    EXPECT_TRUE (ColumnExist (db, tblEmployee, L"FirstName"));
    EXPECT_TRUE (ColumnExist (db, tblEmployee, L"JobTitle"));
    EXPECT_TRUE (ColumnExist (db, tblEmployee, L"LastName"));
    EXPECT_TRUE (ColumnExist (db, tblEmployee, L"ManagerID"));
    EXPECT_TRUE (ColumnExist (db, tblEmployee, L"Room"));
    EXPECT_TRUE (ColumnExist (db, tblEmployee, L"SSN")); 
    EXPECT_TRUE (ColumnExist (db, tblEmployee, L"Project")); 
    EXPECT_TRUE (ColumnExist (db, tblEmployee, L"FullName")); 
    EXPECT_TRUE (ColumnExist (db, tblEmployee, L"EmployeeType")); 
    EXPECT_TRUE (ColumnExist (db, tblEmployee, L"EmployeeRecordKey")); 
    EXPECT_TRUE (ColumnExist (db, tblEmployee, L"Company__trg_11_id")); 
    EXPECT_FALSE(ColumnExist (db, tblEmployee, L"EmployeeCertification")); //struct array property

    //========================[sc_Company]=======================================================
    WCharCP tblCompany = L"sc_Company";
    EXPECT_TRUE (TableExist  (db, tblCompany));    
    EXPECT_EQ   (14, GetColumnCount(db, tblCompany));
    EXPECT_TRUE (ColumnExist (db, tblCompany, L"ECInstanceId"));
    
    EXPECT_TRUE (ColumnExist (db, tblCompany, L"Name"));
    EXPECT_TRUE (ColumnExist (db, tblCompany, L"NumberOfEmployees"));
    EXPECT_TRUE (ColumnExist (db, tblCompany, L"ContactAddress"));
    EXPECT_TRUE (ColumnExist (db, tblCompany, L"RecordKey"));

    //========================[sc_EmployeeCertifications]========================================
    WCharCP tblEmployeeCertification = L"sc_ArrayOfEmployeeCertification";
    EXPECT_TRUE (TableExist  (db, tblEmployeeCertification));    
    EXPECT_EQ   (9, GetColumnCount(db, tblEmployeeCertification));

    ASSERT_TRUE (ColumnExist (db, tblEmployeeCertification, L"ECInstanceId"));
    ASSERT_TRUE (ColumnExist (db, tblEmployeeCertification, L"ParentECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, tblEmployeeCertification, L"ECPropertyPathId"));
    EXPECT_TRUE (ColumnExist (db, tblEmployeeCertification, L"ECArrayIndex"));
    
    EXPECT_TRUE (ColumnExist (db, tblEmployeeCertification, L"Name"));
    EXPECT_TRUE (ColumnExist (db, tblEmployeeCertification, L"StartDate"));
    EXPECT_TRUE (ColumnExist (db, tblEmployeeCertification, L"ExpiryDate"));
    EXPECT_TRUE (ColumnExist (db, tblEmployeeCertification, L"Technology"));
    EXPECT_TRUE (ColumnExist (db, tblEmployeeCertification, L"Level"));
    
    //========================[sc_Hardware]======================================================
    //TablePerClass
    WCharCP tblHardware = L"sc_Hardware"; //Drived from Asset
    EXPECT_TRUE (TableExist  (db, tblHardware));
    EXPECT_EQ   (14, GetColumnCount(db, tblHardware));            
    
    EXPECT_TRUE (ColumnExist (db, tblHardware, L"ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(ColumnExist (db, tblHardware, L"ECClassId")); 
    
    //base class Asset properties
    EXPECT_TRUE (ColumnExist (db, tblHardware, L"AssetID"));
    EXPECT_TRUE (ColumnExist (db, tblHardware, L"AssetOwner"));
    EXPECT_TRUE (ColumnExist (db, tblHardware, L"BarCode"));
    EXPECT_TRUE (ColumnExist (db, tblHardware, L"AssetUserID"));
    EXPECT_TRUE (ColumnExist (db, tblHardware, L"Cost"));
    EXPECT_TRUE (ColumnExist (db, tblHardware, L"Room"));
    EXPECT_TRUE (ColumnExist (db, tblHardware, L"AssetRecordKey")); 
    //Local properties of Hardware   
    EXPECT_TRUE (ColumnExist (db, tblHardware, L"HasWarranty")); 
    EXPECT_TRUE (ColumnExist (db, tblHardware, L"IsCompanyProperty")); 
    EXPECT_TRUE (ColumnExist (db, tblHardware, L"Make")); 
    EXPECT_TRUE (ColumnExist (db, tblHardware, L"Model")); 
    EXPECT_TRUE (ColumnExist (db, tblHardware, L"WarrantyExpiryDate")); 
        
    //========================[sc_Computer]======================================================
    //TablePerClass
    WCharCP tblComputer = L"sc_Computer"; //Drived from Asset
    EXPECT_TRUE (TableExist  (db, tblComputer));
    EXPECT_EQ   (17, GetColumnCount(db, tblComputer));            
    
    EXPECT_TRUE (ColumnExist (db, tblComputer, L"ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(ColumnExist (db, tblComputer, L"ECClassId")); 
    
    //base class properties of Asset 
    EXPECT_TRUE (ColumnExist (db, tblComputer, L"AssetID"));
    EXPECT_TRUE (ColumnExist (db, tblComputer, L"AssetOwner"));
    EXPECT_TRUE (ColumnExist (db, tblComputer, L"BarCode"));
    EXPECT_TRUE (ColumnExist (db, tblComputer, L"AssetUserID"));
    EXPECT_TRUE (ColumnExist (db, tblComputer, L"Cost"));
    EXPECT_TRUE (ColumnExist (db, tblComputer, L"Room"));
    EXPECT_TRUE (ColumnExist (db, tblComputer, L"AssetRecordKey")); 
    //base class properties of Hardware   
    EXPECT_TRUE (ColumnExist (db, tblComputer, L"HasWarranty")); 
    EXPECT_TRUE (ColumnExist (db, tblComputer, L"IsCompanyProperty")); 
    EXPECT_TRUE (ColumnExist (db, tblComputer, L"Make")); 
    EXPECT_TRUE (ColumnExist (db, tblComputer, L"Model")); 
    EXPECT_TRUE (ColumnExist (db, tblComputer, L"WarrantyExpiryDate")); 
    //local properties
    EXPECT_TRUE (ColumnExist (db, tblComputer, L"Vendor")); 
    EXPECT_TRUE (ColumnExist (db, tblComputer, L"Weight")); 
    EXPECT_TRUE (ColumnExist (db, tblComputer, L"Type")); 

    //========================[sc_Monitor]======================================================
    //TablePerClass
    WCharCP tblMonitor = L"sc_Monitor"; //Drived from Asset
    EXPECT_TRUE (TableExist  (db, tblMonitor));
    EXPECT_EQ   (18, GetColumnCount(db, tblMonitor));            
    
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(ColumnExist (db, tblMonitor, L"ECClassId")); 
    
    //base class properties of Asset 
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"AssetID"));
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"AssetOwner"));
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"BarCode"));
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"AssetUserID"));
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"Cost"));
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"Room"));
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"AssetRecordKey")); 
    //base class properties of Hardware   
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"HasWarranty")); 
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"IsCompanyProperty")); 
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"Make")); 
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"Model")); 
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"WarrantyExpiryDate")); 
    //local properties
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"Size")); 
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"Type")); 
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"Vendor")); 
    EXPECT_TRUE (ColumnExist (db, tblMonitor, L"Weight")); 
    
    //========================[sc_Phone]=========================================================
    //TablePerClass
    WCharCP tblPhone = L"sc_Phone"; //Drived from Asset
    EXPECT_TRUE (TableExist  (db, tblPhone));
    EXPECT_EQ   (12, GetColumnCount(db, tblPhone));            
    
    EXPECT_TRUE (ColumnExist (db, tblPhone, L"ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(ColumnExist (db, tblPhone, L"ECClassId")); 
    
    //base class properties of Asset 
    EXPECT_TRUE (ColumnExist (db, tblPhone, L"AssetID"));
    EXPECT_TRUE (ColumnExist (db, tblPhone, L"AssetOwner"));
    EXPECT_TRUE (ColumnExist (db, tblPhone, L"BarCode"));
    EXPECT_TRUE (ColumnExist (db, tblPhone, L"AssetUserID"));
    EXPECT_TRUE (ColumnExist (db, tblPhone, L"Cost"));
    EXPECT_TRUE (ColumnExist (db, tblPhone, L"Room"));
    EXPECT_TRUE (ColumnExist (db, tblPhone, L"AssetRecordKey")); 
    //local properties
    EXPECT_TRUE (ColumnExist (db, tblPhone, L"Number")); 
    EXPECT_TRUE (ColumnExist (db, tblPhone, L"Owner")); 
    EXPECT_TRUE (ColumnExist (db, tblPhone, L"User")); 
    
    //========================[sc_Widget]========================================================
    WCharCP tblWidget = L"sc_Widget"; 
    EXPECT_TRUE (TableExist  (db, tblWidget));
    EXPECT_EQ   (3, GetColumnCount(db, tblWidget));            
    
    EXPECT_TRUE (ColumnExist (db, tblWidget, L"ECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, tblWidget, L"stringOfWidget")); 
    
    //========================[sc_Project]=======================================================
    WCharCP tblProject = L"sc_Project"; 
    EXPECT_TRUE (TableExist  (db, tblProject));
    EXPECT_EQ   (14, GetColumnCount(db, tblProject));            
    
    EXPECT_TRUE (ColumnExist (db, tblProject, L"ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(ColumnExist (db, tblProject, L"ECClassId")); 

    EXPECT_TRUE (ColumnExist (db, tblProject, L"CompletionDate")); 
    EXPECT_TRUE (ColumnExist (db, tblProject, L"EstimatedCost")); 
    EXPECT_TRUE (ColumnExist (db, tblProject, L"ProjectName")); 
    EXPECT_TRUE (ColumnExist (db, tblProject, L"ProjectDescription")); 
    EXPECT_TRUE (ColumnExist (db, tblProject, L"ProjectState")); 
    EXPECT_TRUE (ColumnExist (db, tblProject, L"StartDate")); 
    EXPECT_TRUE (ColumnExist (db, tblProject, L"InProgress")); 
    EXPECT_TRUE (ColumnExist (db, tblProject, L"TeamSize")); 
    EXPECT_TRUE (ColumnExist (db, tblProject, L"Logo")); 
    EXPECT_TRUE (ColumnExist (db, tblProject, L"Manager")); 
    EXPECT_TRUE (ColumnExist (db, tblProject, L"ProjectRecordKey")); 
    //struct/arrays mapped to table
    EXPECT_TRUE(ColumnExist (db, tblProject, L"TeamMemberList"));  //int array
    //relation
    EXPECT_TRUE (ColumnExist (db, tblProject, L"Company__src_11_id")); 
    
    //========================[sc_Building]======================================================
    WCharCP tblBuilding = L"sc_Building"; 
    EXPECT_TRUE (TableExist  (db, tblBuilding));    
    EXPECT_EQ   (14, GetColumnCount(db, tblBuilding));
    
    EXPECT_TRUE (ColumnExist (db, tblBuilding, L"ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(ColumnExist (db, tblBuilding, L"ECClassId")); 

    EXPECT_TRUE (ColumnExist (db, tblBuilding, L"Number")); 
    EXPECT_TRUE (ColumnExist (db, tblBuilding, L"Name")); 
    EXPECT_TRUE (ColumnExist (db, tblBuilding, L"NumberOfFloors")); 
    EXPECT_TRUE (ColumnExist (db, tblBuilding, L"BuildingCode")); 
    EXPECT_TRUE (ColumnExist (db, tblBuilding, L"RecordKey")); 
    //struct array
    EXPECT_FALSE(ColumnExist (db, tblBuilding, L"Location"));
    
    //========================[sc_Location]======================================================
    WCharCP tblLocation = L"sc_ArrayOfLocation"; 
    EXPECT_TRUE (TableExist  (db, tblLocation));
    EXPECT_EQ   (12, GetColumnCount(db, tblLocation));            

    ASSERT_TRUE (ColumnExist (db, tblLocation, L"ECInstanceId"));
    ASSERT_TRUE (ColumnExist (db, tblLocation, L"ParentECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, tblLocation, L"ECPropertyPathId"));
    EXPECT_TRUE (ColumnExist (db, tblLocation, L"ECArrayIndex"));

    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(ColumnExist (db, tblLocation, L"ECClassId")); 

    EXPECT_TRUE (ColumnExist (db, tblLocation, L"Coordinate_X")); 
    EXPECT_TRUE (ColumnExist (db, tblLocation, L"Coordinate_Y")); 
    EXPECT_TRUE (ColumnExist (db, tblLocation, L"Coordinate_Z")); 
    EXPECT_TRUE (ColumnExist (db, tblLocation, L"Street")); 
    EXPECT_TRUE (ColumnExist (db, tblLocation, L"City")); 
    EXPECT_TRUE (ColumnExist (db, tblLocation, L"State")); 
    EXPECT_TRUE (ColumnExist (db, tblLocation, L"Country")); 
    EXPECT_TRUE (ColumnExist (db, tblLocation, L"Zip")); 
    
    //========================[sc_BuildingFloor]=================================================
    WCharCP tblBuildingFloor = L"sc_BuildingFloor"; 
    EXPECT_TRUE (TableExist  (db, tblBuildingFloor));
    EXPECT_EQ   (8, GetColumnCount(db, tblBuildingFloor));            
    
    EXPECT_TRUE (ColumnExist (db, tblBuildingFloor, L"ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(ColumnExist (db, tblBuildingFloor, L"ECClassId")); 

    EXPECT_TRUE (ColumnExist (db, tblBuildingFloor, L"FloorNumber")); 
    EXPECT_TRUE (ColumnExist (db, tblBuildingFloor, L"BuildingCode")); 
    EXPECT_TRUE (ColumnExist (db, tblBuildingFloor, L"NumberOfOffices")); 
    EXPECT_TRUE (ColumnExist (db, tblBuildingFloor, L"Area")); 
    EXPECT_TRUE (ColumnExist (db, tblBuildingFloor, L"FloorCode")); 
    EXPECT_TRUE (ColumnExist (db, tblBuildingFloor, L"RecordKey")); 
    //relation
    EXPECT_TRUE (ColumnExist (db, tblBuildingFloor, L"Building__src_11_Id")); 
    
    
//========================[sc_Cubicle]=================================================
    WCharCP tblCubicle = L"sc_Cubicle"; 
    EXPECT_TRUE (TableExist  (db, tblCubicle));
    EXPECT_EQ   (13, GetColumnCount(db, tblCubicle));            
    
    EXPECT_TRUE (ColumnExist (db, tblCubicle, L"ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(ColumnExist (db, tblCubicle, L"ECClassId")); 

    EXPECT_TRUE (ColumnExist (db, tblCubicle, L"Bay")); 
    EXPECT_TRUE (ColumnExist (db, tblCubicle, L"IsOccupied")); 
    EXPECT_TRUE (ColumnExist (db, tblCubicle, L"BuildingFloor")); 
    EXPECT_TRUE (ColumnExist (db, tblCubicle, L"Length")); 
    EXPECT_TRUE (ColumnExist (db, tblCubicle, L"Breadth")); 
    EXPECT_TRUE (ColumnExist (db, tblCubicle, L"NumberOfOccupants")); 
    EXPECT_TRUE (ColumnExist (db, tblCubicle, L"BuildingCode")); 
    EXPECT_TRUE (ColumnExist (db, tblCubicle, L"OfficeCode")); 
    EXPECT_TRUE (ColumnExist (db, tblCubicle, L"Area")); 
    EXPECT_TRUE (ColumnExist (db, tblCubicle, L"RecordKey"));     
    //array    
    EXPECT_TRUE(ColumnExist (db, tblCubicle, L"OccupiedBy"));         
    //relation
    EXPECT_TRUE (ColumnExist (db, tblCubicle, L"BuildingFloor__src_11_id")); 
        
    //========================[sc_AnglesStruct]======================================================
    WCharCP tblAnglesStruct = L"sc_ArrayOfAnglesStruct"; 
    EXPECT_TRUE (TableExist  (db, tblAnglesStruct));
    EXPECT_EQ   (7, GetColumnCount(db, tblAnglesStruct));            

    ASSERT_TRUE (ColumnExist (db, tblAnglesStruct, L"ECInstanceId"));
    ASSERT_TRUE (ColumnExist (db, tblAnglesStruct, L"ParentECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, tblAnglesStruct, L"ECPropertyPathId"));
    EXPECT_TRUE (ColumnExist (db, tblAnglesStruct, L"ECArrayIndex"));

    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(ColumnExist (db, tblAnglesStruct, L"ECClassId")); 

    EXPECT_TRUE (ColumnExist (db, tblAnglesStruct, L"Alpha")); 
    EXPECT_TRUE (ColumnExist (db, tblAnglesStruct, L"Beta")); 
    EXPECT_TRUE (ColumnExist (db, tblAnglesStruct, L"Theta")); 

    //========================[sc_ABFoo]======================================================
    WCharCP tblABFoo = L"sc_ABFoo"; 
    EXPECT_TRUE (TableExist  (db, tblABFoo));
    EXPECT_EQ   (2, GetColumnCount(db, tblABFoo));            
    
    EXPECT_TRUE (ColumnExist (db, tblABFoo, L"ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(ColumnExist (db, tblABFoo, L"ECClassId")); 
    
    EXPECT_TRUE (ColumnExist (db, tblABFoo, L"stringABFoo")); 
    
    //========================[sc_AAFoo]=========================================================
    WCharCP tblAAFoo = L"sc_AAFoo"; 
    EXPECT_TRUE (TableExist  (db, tblAAFoo));
    EXPECT_FALSE(TableExist  (db, L"AFooChild")); //This child class of AAFoo which have TablePerHierarchy so table for its child classes should not be created
    EXPECT_EQ   (25, GetColumnCount(db, tblAAFoo));
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"ECInstanceId"));
    //This a TablePerHieracrchy
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"ECClassId")); 
    
    //Local properties
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"FooTag"));
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"intAAFoo"));
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"longAAFoo"));
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"stringAAFoo"));
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"doubleAAFoo"));
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"datetimeAAFoo"));
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"binaryAAFoo"));
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"booleanAAFoo"));
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"point2dAAFoo_X"));
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"point2dAAFoo_Y"));
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"point3dAAFoo_X"));
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"point3dAAFoo_Y"));
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"point3dAAFoo_Z"));
    //EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"anglesAAFoo")); // we are no longer stuffing structs into blobs
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"commonGeometryAAFoo"));
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"colorAAFoo"));

    // arrays
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"arrayOfIntsAAFoo"));
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"arrayOfpoint2dAAFoo"));
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"arrayOfpoint3dAAFoo"));
    
    //From ABFoo since its one of the base class of child class AFooChild
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"stringABFoo"));
    //From AFooChild which is child of AAFoo
    EXPECT_TRUE (ColumnExist (db, tblAAFoo, L"binaryAFooChild"));

    //========================[sc_Bar]===========================================================
    WCharCP tblBar = L"sc_Bar"; //this table has be renamed from tblBar=>FOO_FIGHTERS
    EXPECT_TRUE (TableExist  (db, tblBar));
    EXPECT_EQ   (4, GetColumnCount(db, tblBar));
    EXPECT_TRUE (ColumnExist (db, tblBar, L"ECInstanceId"));
    //This a TablePerHieracrchy
    EXPECT_FALSE(ColumnExist (db, tblBar, L"ECClassId"));
    //Local properties
    EXPECT_TRUE (ColumnExist (db, tblBar, L"stringBar"));
    //Relations
    EXPECT_TRUE (ColumnExist (db, tblBar, L"ForeignECInstanceId_Foo_has_Bars"));
    EXPECT_TRUE (ColumnExist (db, tblBar, L"ForeignECInstanceId_Foo_has_Bars_hint"));
    
    //========================[sc_Foo]===========================================================
    WCharCP tblFoo = L"FOO_FIGHTERS"; //this table has be renamed from tblFoo=>FOO_FIGHTERS
    EXPECT_TRUE (TableExist  (db, tblFoo));
    EXPECT_EQ   (19, GetColumnCount(db, tblFoo));
    
    EXPECT_TRUE (ColumnExist (db, tblFoo, L"ECInstanceId"));
    //This a TablePerHieracrchy
    EXPECT_TRUE (ColumnExist (db, tblFoo, L"ECClassId")); 
    
    //Local properties
    EXPECT_TRUE (ColumnExist (db, tblFoo, L"intFoo"));
    EXPECT_TRUE (ColumnExist (db, tblFoo, L"longFoo"));
    EXPECT_TRUE (ColumnExist (db, tblFoo, L"stringFoo"));
    EXPECT_TRUE (ColumnExist (db, tblFoo, L"doubleFoo"));
    EXPECT_TRUE (ColumnExist (db, tblFoo, L"datetimeFoo"));
    EXPECT_TRUE (ColumnExist (db, tblFoo, L"binaryFoo"));
    EXPECT_TRUE (ColumnExist (db, tblFoo, L"booleanFoo"));
    EXPECT_TRUE (ColumnExist (db, tblFoo, L"point2dFoo_X"));
    EXPECT_TRUE (ColumnExist (db, tblFoo, L"point2dFoo_Y"));
    EXPECT_TRUE (ColumnExist (db, tblFoo, L"point3dFoo_X"));
    EXPECT_TRUE (ColumnExist (db, tblFoo, L"point3dFoo_Y"));
    EXPECT_TRUE (ColumnExist (db, tblFoo, L"point3dFoo_Z"));
    EXPECT_TRUE (ColumnExist (db, tblFoo, L"commonGeometryFoo"));
    
    // arrays/struct
    EXPECT_TRUE (ColumnExist (db, tblFoo, L"arrayOfIntsFoo"));
    EXPECT_FALSE (ColumnExist (db, tblFoo, L"arrayOfAnglesStructsFoo"));
    EXPECT_FALSE (ColumnExist (db, tblFoo, L"anglesFoo"));

    //========================[sc_ArrayOfStructDomainClass]===========================================================
    WCharCP tbl = L"sc_ArrayOfStructDomainClass"; 
    EXPECT_TRUE (TableExist  (db, tbl));
    EXPECT_EQ   (5, GetColumnCount(db, tbl));

    EXPECT_TRUE (ColumnExist (db, tbl, L"ECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, tbl, L"ParentECInstanceId"));
    //Local properties
    EXPECT_TRUE (ColumnExist (db, tbl, L"stringProp"));
    EXPECT_TRUE (ColumnExist (db, tbl, L"ECPropertyPathId"));
    EXPECT_TRUE (ColumnExist (db, tbl, L"ECArrayIndex"));

    //========================[sc_ArrayOfStructNoneDomainClass]===========================================================
    tbl = L"sc_ArrayOfStructNoneDomainClass"; 
    EXPECT_TRUE (TableExist  (db, tbl));
    EXPECT_EQ   (5, GetColumnCount(db, tbl));

    EXPECT_TRUE (ColumnExist (db, tbl, L"ECInstanceId"));
    EXPECT_TRUE (ColumnExist (db, tbl, L"ParentECInstanceId"));
    //Local properties
    EXPECT_TRUE (ColumnExist (db, tbl, L"stringProp"));
    EXPECT_TRUE (ColumnExist (db, tbl, L"ECPropertyPathId"));
    EXPECT_TRUE (ColumnExist (db, tbl, L"ECArrayIndex"));
    
    //========================[sc_ArrayOfStructDomainClassWithNoProperties]===========================================================
    tbl = L"sc_ArrayOfStructDomainClassWithNoProperties"; 
    EXPECT_TRUE (TableExist  (db, tbl));
    
    //========================[sc_ArrayOfStructNoneDomainClassWithNoProperties]===========================================================
    tbl = L"sc_ArrayOfStructNoneDomainClassWithNoProperties"; 
    EXPECT_TRUE (TableExist  (db, tbl));

    //========================[sc_DomainClass]===========================================================
    tbl = L"sc_DomainClass"; 
    EXPECT_TRUE (TableExist  (db, tbl));
    EXPECT_EQ   (2, GetColumnCount(db, tbl));
    
    EXPECT_TRUE (ColumnExist (db, tbl, L"ECInstanceId"));
    //Local properties
    EXPECT_TRUE (ColumnExist (db, tbl, L"stringProp"));

    //========================[sc_NoneDomainClass]===========================================================
    tbl = L"sc_NoneDomainClass"; 
    EXPECT_FALSE (TableExist  (db, tbl));
    
    //========================[sc_DomainClassWithNoProperties]===========================================================
    tbl = L"sc_DomainClassWithNoProperties"; 
    EXPECT_TRUE (TableExist  (db, tbl));
    
    //========================[sc_NoneDomainClassWithNoProperties]===========================================================
    tbl = L"sc_NoneDomainClassWithNoProperties"; 
    EXPECT_FALSE (TableExist  (db, tbl));
 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaCachePtr CreateImportSchemaAgainstExistingTablesTestSchema ()
    {
    ECSchemaPtr testSchema = nullptr;
    ECSchema::CreateSchema (testSchema, L"test", 1, 0);
    testSchema->SetNamespacePrefix (L"t");
    ECClassP fooClass = nullptr;
    testSchema->CreateClass (fooClass, L"Foo");
    PrimitiveECPropertyP prop = nullptr;
    fooClass->CreatePrimitiveProperty (prop, L"Name", PRIMITIVETYPE_String);

    ECClassP gooClass = nullptr;
    testSchema->CreateClass (gooClass, L"Goo");
    prop = nullptr;
    gooClass->CreatePrimitiveProperty (prop, L"Price", PRIMITIVETYPE_Double);

    ECRelationshipClassP oneToManyRelClass = nullptr;
    testSchema->CreateRelationshipClass (oneToManyRelClass, L"FooHasGoo");
    oneToManyRelClass->SetStrength (STRENGTHTYPE_Holding);
    oneToManyRelClass->GetSource ().AddClass (*fooClass);
    oneToManyRelClass->GetSource ().SetCardinality (RelationshipCardinality::OneOne ());
    oneToManyRelClass->GetTarget ().AddClass (*gooClass);
    oneToManyRelClass->GetTarget ().SetCardinality (RelationshipCardinality::ZeroMany ());

    ECRelationshipClassP manyToManyRelClass = nullptr;
    testSchema->CreateRelationshipClass (manyToManyRelClass, L"RelFooGoo");
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
TEST (ECDbSchemas, DeigoRelationshipTest)
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
// @bsimethod                                   Affan.Khan                       08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbSchemas, DeigoRelationshipTest2)
    {
    // Create a sample project
    ECDbTestProject test;
    auto& ecdb = test.Create ("importecschema.ecdb");
    const int nSchemas = 10;
    WChar schemaNames[nSchemas][255] = {
        L"Bentley_Civil_Objects.02.00",
        L"HW_Bentley_Civil__Model_Geometry.03.00",
        L"HW_Bentley_Civil__Model_Geometry_ContentManagement.03.00",
        L"HW_Bentley_Civil__Model_Base.03.00",
        L"HW_Bentley_Civil__Model_ContentManagement.03.00",
        L"HW_Bentley_Civil__Model_DesignStandards.03.00",
        L"HW_Bentley_Civil__Model_DTMFilterGroups.03.00",
        L"HW_Bentley_Civil__Model_FilterGroups.03.00",
        L"HW_Bentley_Civil__Model_MX.03.00",
        L"HW_Bentley_Civil__Model_ProjectSettings.03.00"
        };
    ECSchemaPtr loadedSchema;
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext ();
    ctx->AddSchemaLocater (ecdb. GetSchemaLocater());
    for (int i= 0; i <nSchemas; i++)
        {
        SchemaKey schemaKey;
        SchemaKey::ParseSchemaFullName (schemaKey, schemaNames[i]);
        if (ctx->GetCache().GetSchema (schemaKey, SchemaMatchType::SCHEMAMATCHTYPE_Exact))
            continue;

        WString schemaFile = schemaNames[i];
        schemaFile.append(L".ecschema.xml");
        ECDbTestUtility::ReadECSchemaFromDisk (loadedSchema, ctx, schemaFile.c_str(), nullptr);
        ASSERT_TRUE (loadedSchema.IsValid ());
        }

    //now import test schema where the table already exists for the ECClass
    ASSERT_EQ (SUCCESS, ecdb. Schemas ().ImportECSchemas (ctx->GetCache(),
        ECDbSchemaManager::ImportOptions (false, false))) << "ImportECSchema is expected to return success for schemas with classes that map to an existing table.";

    auto& schemaManager = ecdb. Schemas ();
    auto aCivilModel = schemaManager.GetECClass ("HW_Bentley_Civil__Model_Base", "CivilModel");
    auto aDataSetModel = schemaManager.GetECClass ("HW_Bentley_Civil__Model_Base", "DataSetModel");
    auto aCivilModel__DataSetModel = schemaManager.GetECClass ("HW_Bentley_Civil__Model_Base", "CivilModel__DataSetModel");
    auto aGeometricModel = schemaManager.GetECClass ("HW_Bentley_Civil__Model_Geometry", "GeometricModel");
    auto aDesignStandardsModel = schemaManager.GetECClass ("HW_Bentley_Civil__Model_DesignStandards", "DesignStandardsModel");

    ASSERT_TRUE (aCivilModel != nullptr);
    ASSERT_TRUE (aDataSetModel != nullptr);
    ASSERT_TRUE (aCivilModel__DataSetModel != nullptr);
    ASSERT_TRUE (aGeometricModel != nullptr);
    ASSERT_TRUE (aDesignStandardsModel != nullptr);

    auto  iCivilModel = ECDbTestProject::CreateArbitraryECInstance (*aCivilModel);
    auto  iGeometricModel = ECDbTestProject::CreateArbitraryECInstance (*aGeometricModel);
    auto  iDesignStandardsModel = ECDbTestProject::CreateArbitraryECInstance (*aDesignStandardsModel);
    auto  iDataSetModel = ECDbTestProject::CreateArbitraryECInstance (*aDataSetModel);

    auto relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*(aCivilModel__DataSetModel->GetRelationshipClassCP ()));

    auto iCivilModel__DataSetModel1 = relationshipEnabler->CreateRelationshipInstance ();
    iCivilModel__DataSetModel1->SetSource (iCivilModel.get ());
    iCivilModel__DataSetModel1->SetTarget (iGeometricModel.get ());

    auto iCivilModel__DataSetModel2 = relationshipEnabler->CreateRelationshipInstance ();
    iCivilModel__DataSetModel2->SetSource (iCivilModel.get ());
    iCivilModel__DataSetModel2->SetTarget (iDesignStandardsModel.get ());

    ECInstanceInserter aiCivilModel (ecdb, *aCivilModel);
    ASSERT_TRUE (aiCivilModel.IsValid ());
    auto insertStatus = aiCivilModel.Insert (*iCivilModel);
    ASSERT_EQ (insertStatus, BentleyStatus::SUCCESS);

    ECInstanceInserter aiGeometricModel (ecdb, *aGeometricModel);
    ASSERT_TRUE (aiGeometricModel.IsValid ());
    insertStatus = aiGeometricModel.Insert (*iGeometricModel);
    ASSERT_EQ (insertStatus, BentleyStatus::SUCCESS);

    ECInstanceInserter aiDesignStandardsModel (ecdb, *aDesignStandardsModel);
    ASSERT_TRUE (aiDesignStandardsModel.IsValid ());
    insertStatus = aiDesignStandardsModel.Insert (*iDesignStandardsModel);
    ASSERT_EQ (insertStatus, BentleyStatus::SUCCESS);

    ECInstanceInserter aiCivilModel__DataSetModel (ecdb, *aCivilModel__DataSetModel);
    ASSERT_TRUE (aiCivilModel__DataSetModel.IsValid ());

    insertStatus = aiCivilModel__DataSetModel.Insert (*iCivilModel__DataSetModel1);
    ASSERT_EQ (insertStatus, BentleyStatus::SUCCESS);

    insertStatus = aiCivilModel__DataSetModel.Insert (*iCivilModel__DataSetModel2);
    ASSERT_EQ (insertStatus, BentleyStatus::SUCCESS);

    ECSqlStatement stmt;
    auto status = stmt.Prepare(ecdb, "SELECT ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM [HW_Bentley_Civil__Model_Base].[CivilModel__DataSetModel] WHERE SourceECInstanceId = 1");
    ASSERT_EQ ((int)status, (int)ECSqlStatus::Success);

    int iRows = 0;
    while (stmt.Step () == ECSqlStepStatus::HasRow)
        {
        iRows++;
        }
    ASSERT_EQ (iRows, 2);
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
    ECObjectsStatus stat = ECSchema::CreateSchema (schema, L"foo", 1, 0);
    ASSERT_EQ (ECOBJECTS_STATUS_Success, stat) << L"Creating test schema failed";

    ECClassP domainClass = nullptr;
    stat = schema->CreateClass (domainClass, L"domain1");
    ASSERT_EQ (ECOBJECTS_STATUS_Success, stat) << L"Creating domain class 1 in schema failed";
    domainClass->SetIsDomainClass (true);
    domainClass->SetIsCustomAttributeClass (false);
    domainClass->SetIsStruct (false);

    ECClassP domainClass2 = nullptr;
    stat = schema->CreateClass (domainClass2, L"domain2");
    ASSERT_EQ (ECOBJECTS_STATUS_Success, stat) << L"Creating domain class 2 in schema failed";
    domainClass2->SetIsDomainClass (true);
    domainClass2->SetIsCustomAttributeClass (false);
    domainClass2->SetIsStruct (false);

    ECClassP caClass = nullptr;
    stat = schema->CreateClass (caClass, L"MyCA");
    ASSERT_EQ (ECOBJECTS_STATUS_Success, stat) << L"Creating CA class in schema failed";
    caClass->SetIsDomainClass (false);
    caClass->SetIsCustomAttributeClass (true);
    caClass->SetIsStruct (false);

    PrimitiveECPropertyP dateProp = nullptr;
    caClass->CreatePrimitiveProperty (dateProp, L"dateprop", PRIMITIVETYPE_DateTime);

    PrimitiveECPropertyP stringProp = nullptr;
    caClass->CreatePrimitiveProperty (stringProp, L"stringprop", PRIMITIVETYPE_String);

    PrimitiveECPropertyP doubleProp = nullptr;
    caClass->CreatePrimitiveProperty (doubleProp, L"doubleprop", PRIMITIVETYPE_Double);

    PrimitiveECPropertyP pointProp = nullptr;
    caClass->CreatePrimitiveProperty (pointProp, L"pointprop", PRIMITIVETYPE_Point3D);

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
WCharCP containerClassName,
WCharCP caClassName,
WCharCP instanceId,
bmap<WString, ECValue> const& caPropValues
)
    {
    ECClassP caClass = schema->GetClassP (caClassName);
    IECInstancePtr ca = caClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ASSERT_TRUE (ca.IsValid ());

    ECObjectsStatus stat;
    if (instanceId != nullptr)
        {
        stat = ca->SetInstanceId (instanceId);
        ASSERT_EQ (ECOBJECTS_STATUS_Success, stat) << L"Setting instance id in CA instance failed";
        }

    typedef bpair<WString, ECValue> T_PropValuePair;

    for (T_PropValuePair const& pair : caPropValues)
        {
        stat = ca->SetValue (pair.first.c_str (), pair.second);
        ASSERT_EQ (ECOBJECTS_STATUS_Success, stat) << L"Assigning property value to CA instance failed";
        }

    ECClassP containerClass = schema->GetClassP (containerClassName);
    stat = containerClass->SetCustomAttribute (*ca);
    ASSERT_EQ (ECOBJECTS_STATUS_Success, stat) << L"Assigning CA instance to container class failed";

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
    bmap<WString, ECValue> propValueMap;
    propValueMap[WString (L"dateprop")] = ECValue (DateTime (DateTime::Kind::Unspecified, 1971, 4, 30, 21, 9, 0, 0));
    propValueMap[WString (L"stringprop")] = ECValue ("hello world", true);
    propValueMap[WString (L"doubleprop")] = ECValue (3.14);
    DPoint3d point;
    point.x = 1.0;
    point.y = -2.0;
    point.z = 3.0;
    propValueMap[WString (L"pointprop")] = ECValue (point);

    IECInstancePtr ca = nullptr;
    AssignCustomAttribute (ca, testSchema, L"domain1", L"MyCA", L"bla bla", propValueMap);

    return ca;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemas, ReadCustomAttributesTest)
    {
    WCharCP const CAClassName = L"MyCA";

    ECSchemaPtr testSchema = nullptr;
    ECSchemaCachePtr testSchemaCache = nullptr;
    CreateCustomAttributeTestSchema (testSchema, testSchemaCache);

    //assign CA with instance id and all props populated
    IECInstancePtr expectedCAInstanceWithInstanceId = CreateAndAssignRandomCAInstance (testSchema);

    //assign CA without instance id and only a few props populated
    bmap<WString, ECValue> propValueMap;
    propValueMap[WString (L"doubleprop")] = ECValue (3.14);
    IECInstancePtr expectedCAInstanceWithoutInstanceId = nullptr;
    AssignCustomAttribute (expectedCAInstanceWithoutInstanceId, testSchema, L"domain2", CAClassName, nullptr, propValueMap);

    //create test db and close it again
    Utf8String dbPath;
        {
        ECDbTestProject testProject;
        ECDbR db = testProject.Create ("customattributestest.ecdb");
        auto importStat = db. Schemas ().ImportECSchemas (*testSchemaCache);
        ASSERT_EQ (SUCCESS, importStat) << L"Could not import test schema into ECDb file";

        dbPath = testProject.GetECDbPath ();
        }

    //reopen test ECDb file (to make sure that the stored schema is read correctly)
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ (BE_SQLITE_OK, stat) << L"Could not open test ECDb file";

    ECSchemaCP readSchema = db. Schemas ().GetECSchema (Utf8String (testSchema->GetName().c_str ()).c_str ());
    ASSERT_TRUE (readSchema != nullptr) << L"Could not read test schema from reopened ECDb file.";
    //*** assert custom attribute instance with instance id
    ECClassCP domainClass1 = readSchema->GetClassCP (L"domain1");
    ASSERT_TRUE (domainClass1 != nullptr) << L"Could not retrieve domain class 1 from re-read test schema.";
    IECInstancePtr actualCAInstanceWithInstanceId = domainClass1->GetCustomAttribute (CAClassName);
    ASSERT_TRUE (actualCAInstanceWithInstanceId.IsValid ()) << L"Test custom attribute instance not found on domain class 1.";

    //compare instance ids
    ASSERT_STREQ (expectedCAInstanceWithInstanceId->GetInstanceId ().c_str (), actualCAInstanceWithInstanceId->GetInstanceId ().c_str ()) << L"Instance Ids of retrieved custom attribute instance doesn't match.";
    
    //compare rest of instance
    bool equal = ECDbTestUtility::CompareECInstances (*expectedCAInstanceWithInstanceId, *actualCAInstanceWithInstanceId);
    ASSERT_TRUE (equal) << L"Read custom attribute instance with instance id differs from expected.";

    //*** assert custom attribute instance without instance id
    ECClassCP domainClass2 = readSchema->GetClassCP (L"domain2");
    ASSERT_TRUE (domainClass2 != nullptr) << L"Could not retrieve domain class 2 from re-read test schema.";
    IECInstancePtr actualCAInstanceWithoutInstanceId = domainClass2->GetCustomAttribute (CAClassName);
    ASSERT_TRUE (actualCAInstanceWithoutInstanceId.IsValid ()) << L"Test custom attribute instance not found on domain class 2.";

    //compare instance ids
    ASSERT_STREQ (expectedCAInstanceWithoutInstanceId->GetInstanceId ().c_str (), actualCAInstanceWithoutInstanceId->GetInstanceId ().c_str ()) << L"Instance Ids of retrieved custom attribute instance doesn't match.";
    ASSERT_STREQ (L"", actualCAInstanceWithoutInstanceId->GetInstanceId ().c_str ()) << L"Instance Ids of retrieved custom attribute instance is expected to be empty";
    
    //compare rest of instance
    equal = ECDbTestUtility::CompareECInstances (*expectedCAInstanceWithoutInstanceId, *actualCAInstanceWithoutInstanceId);
    ASSERT_TRUE (equal) << L"Read custom attribute instance without instance id differs from expected.";
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
    ASSERT_EQ (SUCCESS, importStat) << L"Could not import test schema into ECDb file";

    //now retrieve the persisted CA XML from ECDb directly
    Statement stmt;
    DbResult stat = stmt.Prepare (db, "SELECT Instance from ec_CustomAttribute ca, ec_Class c where ca.ClassId = c.Id AND c.Name = 'MyCA'");
    ASSERT_EQ (BE_SQLITE_OK, stat) << L"Preparing the SQL statement to fetch the persisted CA XML string failed.";

    int rowCount = 0;
    while (stmt.Step () == BE_SQLITE_ROW)
        {
        rowCount++;
        Utf8CP caXml = stmt.GetValueText (0);
        ASSERT_TRUE (caXml != nullptr) << L"Retrieved custom attribute XML string is expected to be not null.";
        Utf8String caXmlString (caXml);
        EXPECT_LT (0, (int) caXmlString.length ()) << L"Retrieved custom attribute XML string is not expected to be empty.";

        //It is expected that the XML string doesn't contain the XML descriptor.
        size_t found = caXmlString.find ("<?xml");
        EXPECT_EQ (Utf8String::npos, found) << L"The custom attribute XML string is expected to not contain the XML description tag.";

        //It is expected that the XML string does contain the instance id if the original CA was assigned one
        found = caXmlString.find ("instanceID=");
        EXPECT_NE (Utf8String::npos, found) << L"The custom attribute XML string is expected to contain the instance id for the given custom attribute instance.";
        }

    ASSERT_EQ (1, rowCount) << L"Only one test custom attribute instance had been created.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemas, HandlingMismatchesBetweenCAInstanceAndCAClassTest)
    {
    WCharCP const CAClassName = L"MyCA";
    WCharCP nameOfCAPropertyToRemove = L"dateprop";

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
        ASSERT_EQ (SUCCESS, importStat) << L"Could not import test schema into ECDb file";

        //now remove one of the CA properties only in the CA class definition again
        Statement stmt;
        DbResult stat = stmt.Prepare (db, "delete from ec_Property WHERE Name = 'dateprop' and ClassId = (select Id from ec_Class where Name = 'MyCA')");
        ASSERT_EQ (BE_SQLITE_OK, stat) << L"Preparing the SQL statement to delete row from ec_Property failed.";
        stat = stmt.Step ();
        ASSERT_EQ (BE_SQLITE_DONE, stat) << L"Executing SQL statement to delete row from ec_Property failed";
        EXPECT_EQ (1, db.GetModifiedRowCount ()) << L"The SQL statement to delete row from ec_Property is expected to only delete one row";

        dbPath = testProject.GetECDbPath ();
        }

    //now reopen the out-synched ECDb file (to make sure that the schema stuff is read into memory from scratch
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb (dbPath.c_str (), Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ (BE_SQLITE_OK, stat) << L"Could not open test ECDb file";

    ECSchemaCP readSchema = db. Schemas ().GetECSchema (Utf8String (testSchema->GetName ().c_str ()).c_str ());
    ASSERT_TRUE (readSchema != nullptr) << L"Could not read test schema from reopened ECDb file.";

    //assert custom attribute instance with instance id
    ECClassCP domainClass1 = readSchema->GetClassCP (L"domain1");
    ASSERT_TRUE (domainClass1 != nullptr) << L"Could not retrieve domain class 1 from re-read test schema.";

    IECInstancePtr actualCAInstance = domainClass1->GetCustomAttribute (CAClassName);
    ASSERT_TRUE (actualCAInstance.IsValid ()) << L"Test custom attribute instance not found on domain class 1.";

    //removed property is expected to not be found anymore in the instance
    bool isNull = false;
    ECObjectsStatus ecStat = actualCAInstance->IsPropertyNull (isNull, nameOfCAPropertyToRemove);
    EXPECT_EQ (ECOBJECTS_STATUS_PropertyNotFound, ecStat) << L"Calling IsPropertyNull on CA instance";

    //now check whether the rest of the instance is still the same
    ECValuesCollectionPtr expectedValueCollection = ECValuesCollection::Create (*expectedCAInstance);
    for (ECPropertyValueCR expectedPropertyValue : *expectedValueCollection)
        {
        WCharCP expectedPropertyName = expectedPropertyValue.GetValueAccessor ().GetAccessString ();
        if (BeStringUtilities::Wcsicmp (expectedPropertyName, nameOfCAPropertyToRemove) == 0)
            {
            continue;
            }

        ECValue actualValue;
        ecStat = actualCAInstance->GetValue (actualValue, expectedPropertyName);
        EXPECT_EQ (ECOBJECTS_STATUS_Success, stat) << L"Property '" << expectedPropertyName << L"' not found in actual CA instance.";
        EXPECT_TRUE (expectedPropertyValue.GetValue ().Equals (actualValue)) << L"Property values for property '" << expectedPropertyName << L"' do not match";
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
    SchemaKey key (L"StartupCompany", 2, 0);

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
    ECClassCP aaa2 = startupCompanySchema->GetClassCP (L"AAA");

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
   ECSchemaCP ecdbSystemSchema = db.Schemas().GetECSchema ("ECDbSystem");
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

    auto arrayTestClass = startupCompanySchema->GetClassCP (L"ArrayTestclass");
    ASSERT_TRUE (arrayTestClass != nullptr);

    auto p0_unbounded = arrayTestClass->GetPropertyP (L"p0_unbounded")->GetAsArrayProperty ();
    ASSERT_TRUE (p0_unbounded != nullptr);
    ASSERT_EQ (p0_unbounded->GetMinOccurs (), 0);
    ASSERT_EQ (p0_unbounded->GetMaxOccurs (), UINT32_MAX);

    auto p1_unbounded = arrayTestClass->GetPropertyP (L"p1_unbounded")->GetAsArrayProperty ();
    ASSERT_TRUE (p1_unbounded != nullptr);
    ASSERT_EQ (p1_unbounded->GetMinOccurs (), 1);
    ASSERT_EQ (p1_unbounded->GetMaxOccurs (), UINT32_MAX);

    auto p0_1 = arrayTestClass->GetPropertyP (L"p0_1")->GetAsArrayProperty ();
    ASSERT_TRUE (p0_1 != nullptr);
    ASSERT_EQ (p0_1->GetMinOccurs (), 0);
    ASSERT_EQ (p0_1->GetMaxOccurs (), UINT32_MAX);

    auto p1_1 = arrayTestClass->GetPropertyP (L"p1_1")->GetAsArrayProperty ();
    ASSERT_TRUE (p1_1 != nullptr);
    ASSERT_EQ (p1_1->GetMinOccurs (), 1);
    ASSERT_EQ (p1_1->GetMaxOccurs (), UINT32_MAX);

    auto p1_10000 = arrayTestClass->GetPropertyP (L"p1_10000")->GetAsArrayProperty ();
    ASSERT_TRUE (p1_10000 != nullptr);
    ASSERT_EQ (p1_10000->GetMinOccurs (), 1);
    ASSERT_EQ (p1_10000->GetMaxOccurs (), UINT32_MAX);

    auto p100_10000 = arrayTestClass->GetPropertyP (L"p100_10000")->GetAsArrayProperty ();
    ASSERT_TRUE (p100_10000 != nullptr);
    ASSERT_EQ (p100_10000->GetMinOccurs (), 100);
    ASSERT_EQ (p100_10000->GetMaxOccurs (), UINT32_MAX);

    auto p123_12345 = arrayTestClass->GetPropertyP (L"p123_12345")->GetAsArrayProperty ();
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
    ASSERT_EQ (ECSchema::CreateSchema(testSchema, L"TestSchema", 1, 1), ECOBJECTS_STATUS_Success);
    ASSERT_EQ (testSchema->IsDynamicSchema(), false);
    ASSERT_EQ (testSchema->SetIsDynamicSchema(true), ECOBJECTS_STATUS_DynamicSchemaCustomAttributeWasNotFound);
    //reference BCSA, DynamicSchema CA introduce in 1.6
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    SchemaKey bscaKey (L"Bentley_Standard_CustomAttributes", 1, 6);
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
    LOG.infov (L"Loading %ls From DataBase Took : %.4lf seconds", openPlant3D->GetFullSchemaName().c_str(), s0.GetElapsedSeconds());
    StopWatch s1 ("Comparing ECSchema", true);
    
    //Diff two schema too see if they are different in any way diff.Merge();
    ECDiffPtr diff = ECDiff::Diff(*diskSchema, *openPlant3D);
    ASSERT_EQ (diff->GetStatus() , DIFFSTATUS_Success);
    if (!diff->IsEmpty())
        {
        bmap<WString, DiffNodeState> searchResults;
        diff->GetNodesState(searchResults, L"*.ArrayInfo");
        if (!searchResults.empty())
            LOG.error(L"*** Feature missing : Array type property MaxOccurs and MinOccurs are not stored currently by ECDbSchemaManager");
        WriteECSchemaDiffToLog(*diff, NativeLogging::LOG_ERROR);

        }
#if 0
    EXPECT_EQ (diff->IsEmpty() , true);
#endif   

    s1.Stop();
    LOG.infov (L"Comparing Db %ls to disk version Took : %.4lf seconds", openPlant3D->GetFullSchemaName().c_str(), s1.GetElapsedSeconds());

    //////////////////////////////////////////////////////////////////////
    db.ClearECDbCache();
    ECClassKeys inSchemaClassKeys;
    EXPECT_EQ (SUCCESS, schemaManager.GetECClassKeys (inSchemaClassKeys, "StartupCompany"));
    LOG.infov(L"No of classes in StartupCompany is %d", (int)inSchemaClassKeys.size());
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

        LOG.infov (L"%3ld. Accessing random class took : %.4lf seconds (%ls)", i, randomClassSW.GetElapsedSeconds(), WString(key.GetName(),true).c_str());
        }

    LOG.infov (L"It took Total : %.4lf seconds to load %d classes", totalTime, maxClassesToLoad);
     
    EXPECT_EQ (SUCCESS, schemaManager.GetECSchemaKeys (schemasInDb));

    LOG.infov (L"Testing SchemaManager APIs");
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

    bmap<WString,DiffNodeState> unitStates;
    diff->GetNodesState (unitStates, L"*.CustomAttributes.Unit_Attributes:UnitSpecification");
    diff->GetNodesState (unitStates, L"*.CustomAttributes.Unit_Attributes:UnitSpecifications");
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
void VerifyRelationshipConstraint(ECN::ECSchemaCR schema, WCharCP relationName, WCharCP sourceClass, WCharCP targetClass)
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
    
    VerifyRelationshipConstraint(*mergedSchema, L"STRUFRMW",   L"STRU", L"FRMW");
    VerifyRelationshipConstraint(*mergedSchema, L"SUBELEVEL5", L"SUBE", L"LEVEL5");
    VerifyRelationshipConstraint(*mergedSchema, L"ZONEEQUI",   L"ZONE", L"EQUI");
    VerifyRelationshipConstraint(*mergedSchema, L"ZONESTRU",   L"ZONE", L"STRU");
    VerifyRelationshipConstraint(*mergedSchema, L"EQUISUBE",   L"EQUI", L"SUBE");
    VerifyRelationshipConstraint(*mergedSchema, L"FRMWLEVEL5", L"FRMW", L"LEVEL5");
    VerifyRelationshipConstraint(*mergedSchema, L"FRMWSBFR",   L"FRMW", L"SBFR");

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
    bmap<WString,DiffNodeState> unitStates;
    diff->GetNodesState (unitStates, L"*.CustomAttributes.Unit_Attributes:UnitSpecification");
    diff->GetNodesState (unitStates, L"*.CustomAttributes.Unit_Attributes:UnitSpecifications");
    ASSERT_EQ(unitStates.size(), 2);
    WriteECSchemaDiffToLog (*diff);
    ECSchemaPtr mergedSchema;
    MergeStatus status = diff->Merge (mergedSchema, CONFLICTRULE_TakeLeft);
    ASSERT_EQ(status , MERGESTATUS_Success);   
    ASSERT_TRUE(mergedSchema.IsValid());
    ECClassP classPtr=mergedSchema->GetClassP(L"Employee");
    uint32_t classCount=mergedSchema->GetClassCount();
    EXPECT_EQ(classCount,8);
    bool bclassDisplayLabel=classPtr->GetIsDisplayLabelDefined();
    WString className=classPtr->GetName();
    EXPECT_STREQ(className.c_str(),L"Employee");
    EXPECT_TRUE(bclassDisplayLabel);
    classPtr=mergedSchema->GetClassP(L"RightFoo");
    className=classPtr->GetName();
    EXPECT_STREQ(className.c_str(),L"RightFoo");
    classPtr=mergedSchema->GetClassP(L"StableClass");
    className=classPtr->GetName();
    EXPECT_STREQ(className.c_str(),L"StableClass");
    classPtr=mergedSchema->GetClassP(L"TestR");
    className=classPtr->GetName();
    EXPECT_STREQ(className.c_str(),L"TestR");
    classPtr=mergedSchema->GetClassP(L"UnitConflictClass");
    className=classPtr->GetName();
    ECSchemaCR SchemaToCheck=classPtr->GetSchema();
    WString SchemaName=SchemaToCheck.GetFullSchemaName();
    EXPECT_STREQ(SchemaName.c_str(),L"LeftSchema.01.00");
    EXPECT_STREQ(className.c_str(),L"UnitConflictClass");
    classPtr=mergedSchema->GetClassP(L"Employee");
    className=classPtr->GetName();
    EXPECT_STREQ(className.c_str(),L"Employee");
    WString classDisplayLabel =classPtr->GetDisplayLabel();
    EXPECT_STREQ(classDisplayLabel.c_str(),L"Employee Left");
    classPtr=mergedSchema->GetClassP(L"LeftFoo");
    className=classPtr->GetName();
    EXPECT_STREQ(className.c_str(),L"LeftFoo");
    classPtr=mergedSchema->GetClassP(L"StableClass");
    className=classPtr->GetName();
    EXPECT_STREQ(className.c_str(),L"StableClass");
    classPtr=mergedSchema->GetClassP(L"StableClass");
    className=classPtr->GetName();
    EXPECT_STREQ(className.c_str(),L"StableClass");
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
    bmap<WString,DiffNodeState> unitStates;
    diff->GetNodesState (unitStates, L"*.CustomAttributes.Unit_Attributes:UnitSpecification");
    diff->GetNodesState (unitStates, L"*.CustomAttributes.Unit_Attributes:UnitSpecifications");
    ASSERT_EQ(unitStates.size(), 2);
    WriteECSchemaDiffToLog (*diff);
    ECSchemaPtr mergedSchema;
    MergeStatus status = diff->Merge (mergedSchema, CONFLICTRULE_TakeLeft);
    ASSERT_EQ(status , MERGESTATUS_Success);   
    ASSERT_TRUE(mergedSchema.IsValid());
    ECRelationshipClassCP relationshipClassPtr = mergedSchema->GetClassP(L"RightRelationshipClass")->GetRelationshipClassP();
    WString relationshipClassName=relationshipClassPtr->GetName();
    EXPECT_STREQ(relationshipClassName.c_str(),L"RightRelationshipClass");
    relationshipClassPtr=mergedSchema->GetClassP(L"TestRelationshipClass")->GetRelationshipClassP();
    relationshipClassName=relationshipClassPtr->GetName();
    EXPECT_STREQ(relationshipClassName.c_str(),L"TestRelationshipClass");
    relationshipClassPtr=mergedSchema->GetClassP(L"TestR")->GetRelationshipClassP();
    relationshipClassName=relationshipClassPtr->GetName();
    EXPECT_STREQ(relationshipClassName.c_str(),L"TestR");
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
    bmap<WString,DiffNodeState> unitStates;
    diff->GetNodesState (unitStates, L"*.CustomAttributes.Unit_Attributes:UnitSpecification");
    diff->GetNodesState (unitStates, L"*.CustomAttributes.Unit_Attributes:UnitSpecifications");
    ASSERT_EQ(unitStates.size(), 2);
    WriteECSchemaDiffToLog (*diff);
    ECSchemaPtr mergedSchema;
    MergeStatus status = diff->Merge (mergedSchema, CONFLICTRULE_TakeLeft);
    ASSERT_EQ(status , MERGESTATUS_Success);   
    ASSERT_TRUE(mergedSchema.IsValid());
    ECClassP ecClassPtr = mergedSchema->GetClassP(L"Employee");
    ECPropertyP  ecPropertyPtr=  ecClassPtr->GetPropertyP(L"Address");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    WString ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(),L"Address");
    ecPropertyPtr=  ecClassPtr->GetPropertyP(L"Department");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(),L"Department");
    ecPropertyPtr=  ecClassPtr->GetPropertyP(L"EmployeeId");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(),L"EmployeeId");
    ecPropertyPtr=  ecClassPtr->GetPropertyP(L"LeftAddedThisProperty");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(),L"LeftAddedThisProperty");
    ecPropertyPtr=  ecClassPtr->GetPropertyP(L"Name");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(),L"Name");
    ecPropertyPtr=  ecClassPtr->GetPropertyP(L"PhoneNumbers");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(),L"PhoneNumbers");
    ecClassPtr = mergedSchema->GetClassP(L"UnitConflictClass");
    ecPropertyPtr=  ecClassPtr->GetPropertyP(L"UCCProp");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(),L"UCCProp");
    ecClassPtr = mergedSchema->GetClassP(L"Employee");
    ecPropertyPtr=  ecClassPtr->GetPropertyP(L"RightAddedThisProperty");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(),L"RightAddedThisProperty");
    ecClassPtr = mergedSchema->GetClassP(L"StableClass");
    ecPropertyPtr=  ecClassPtr->GetPropertyP(L"arrayProperty");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(),L"arrayProperty");
    ecClassPtr = mergedSchema->GetClassP(L"RightFoo");
    ecPropertyPtr=  ecClassPtr->GetPropertyP(L"RightDoubleProperty");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(),L"RightDoubleProperty");
    ECRelationshipClassP  ecRelationshipClassPtr = mergedSchema->GetClassP(L"RightRelationshipClass")->GetRelationshipClassP();
    ecPropertyPtr=  ecRelationshipClassPtr->GetPropertyP(L"Property");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(),L"Property");
    ecRelationshipClassPtr = mergedSchema->GetClassP(L"TestRelationshipClass")->GetRelationshipClassP();
    ecPropertyPtr=  ecRelationshipClassPtr->GetPropertyP(L"PropertyB");
    EXPECT_FALSE(ecPropertyPtr==nullptr);
    ecPropertyName=ecPropertyPtr->GetName();
    EXPECT_STREQ(ecPropertyName.c_str(),L"PropertyB");
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
    SchemaKey schemaKey (L"ECDbMap", 1, 0);
    ECSchemaPtr ecdbMapSchema = MappingSchemaContext->LocateSchema(schemaKey, SCHEMAMATCHTYPE_LatestCompatible);
    EXPECT_TRUE(ecdbMapSchema != nullptr) << "Schema '" << schemaKey.GetFullSchemaName().c_str() << "' not found.";
    ECClassCP testClass = ecdbMapSchema->GetClassCP(L"SchemaMap");
    IECInstancePtr ecInctance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ecInctance->SetValue(L"TablePrefix",ECValue(L"Pre"));
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
    SchemaKey schemaKey (L"ECDbMap", 1, 0);
    ECSchemaPtr ecdbMapSchema = MappingSchemaContext->LocateSchema(schemaKey, SCHEMAMATCHTYPE_LatestCompatible);
    EXPECT_TRUE(ecdbMapSchema != nullptr) << "Schema '" << schemaKey.GetFullSchemaName().c_str() << "' not found.";
    ECClassCP testClass = ecdbMapSchema->GetClassCP(L"ClassMap");
    IECInstancePtr ecInctance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ecInctance->SetValue(L"MapStrategy.Strategy",ECValue(L"OwnTable"));
    MappingSchema->GetClassP(L"B")->SetCustomAttribute(*ecInctance);
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
    SchemaKey schemaKey (L"ECDbMap", 1, 0);
    ECSchemaPtr ecdbMapSchema = MappingSchemaContext->LocateSchema(schemaKey, SCHEMAMATCHTYPE_LatestCompatible);
    EXPECT_TRUE(ecdbMapSchema != nullptr) << "Schema '" << schemaKey.GetFullSchemaName().c_str() << "' not found.";
    ECClassCP testClass = ecdbMapSchema->GetClassCP(L"ClassMap");
    IECInstancePtr ecInctance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ecInctance->SetValue(L"MapStrategy.Strategy",ECValue(L"OwnTable"));
    ecInctance->SetValue(L"MapStrategy.IsPolymorphic", ECValue(true));
    MappingSchema->GetClassP(L"B")->SetCustomAttribute(*ecInctance);
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
    SchemaKey schemaKey (L"ECDbMap", 1, 0);
    ECSchemaPtr ecdbMapSchema = MappingSchemaContext->LocateSchema(schemaKey, SCHEMAMATCHTYPE_LatestCompatible);
    EXPECT_TRUE(ecdbMapSchema != nullptr) << "Schema '" << schemaKey.GetFullSchemaName().c_str() << "' not found.";
    ECClassCP testClass = ecdbMapSchema->GetClassCP(L"ClassMap");
    IECInstancePtr ecInctance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ecInctance->SetValue(L"MapStrategy.Strategy",ECValue(L"NotMapped"));
    MappingSchema->GetClassP(L"B")->SetCustomAttribute(*ecInctance);
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
    SchemaKey schemaKey(L"ECDbMap", 1, 0);
    ECSchemaPtr ecdbMapSchema = MappingSchemaContext->LocateSchema(schemaKey, SCHEMAMATCHTYPE_LatestCompatible);
    EXPECT_TRUE(ecdbMapSchema != nullptr) << "Schema '" << schemaKey.GetFullSchemaName().c_str() << "' not found.";
    ECClassCP testClass = ecdbMapSchema->GetClassCP(L"ClassMap");
    IECInstancePtr ecInctance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ecInctance->SetValue(L"MapStrategy.Strategy",ECValue(L"SharedTable"));
    ecInctance->SetValue(L"MapStrategy.IsPolymorphic", ECValue(true));
    MappingSchema->GetClassP(L"B")->SetCustomAttribute(*ecInctance);
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
    SchemaKey schemaKey (L"ECDbMap", 1, 0);
    ECSchemaPtr ecdbMapSchema =  MappingSchemaContext->LocateSchema(schemaKey,SCHEMAMATCHTYPE_LatestCompatible);
    EXPECT_TRUE(ecdbMapSchema != nullptr) << "Schema '" << schemaKey.GetFullSchemaName().c_str() << "' not found.";
    ECClassCP testClass = ecdbMapSchema->GetClassCP(L"ClassMap");
    IECInstancePtr ecInctance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ecInctance->SetValue(L"MapStrategy.Strategy",ECValue(L"NotMapped"));
    ecInctance->SetValue(L"MapStrategy.IsPolymorphic", ECValue(true));
    MappingSchema->GetClassP(L"B")->SetCustomAttribute(*ecInctance);
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
ECSchemaPtr GenerateNewECSchema(WStringCR name, WStringCR prefix)
    {
    ECSchemaPtr newSchema;
    ECSchema::CreateSchema(newSchema, name, 1, 0);
    newSchema->SetNamespacePrefix(prefix);
    ECClassP equipment, pipe;
    PrimitiveECPropertyP primitveP;
    newSchema->CreateClass(equipment, L"EQUIPMENT");
    equipment->CreatePrimitiveProperty(primitveP,L"NAME",PrimitiveType::PRIMITIVETYPE_String);
    equipment->CreatePrimitiveProperty(primitveP,L"EQID",PrimitiveType::PRIMITIVETYPE_Integer);
    newSchema->CreateClass(pipe, L"PIPE");
    pipe->AddBaseClass(*equipment);
    pipe->CreatePrimitiveProperty(primitveP,L"TYPE",PrimitiveType::PRIMITIVETYPE_String);
    pipe->CreatePrimitiveProperty(primitveP,L"LENGTH",PrimitiveType::PRIMITIVETYPE_Integer);
    return newSchema;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Affan.Khan                        06/13
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbSchemaFixture,TestDuplicateNamespacePrefix)
    {
    WString data[][3] = 
        {
            {L"APlant" , L"op"    , L"op"       },
            {L"BPlant" , L"op"    , L"op1"      },
            {L"CPlant" , L"op"    , L"op2"      },
            {L"DPlant" , L"op"    , L"op3"      },
            {L"EPlant" , L"op"    , L"op4"      },
            {L"FPlant" , L"op"    , L"op5"      },

            {L"GPlant" , L"op"    , L"op6"      },
            {L"HPlant" , L"op1"   , L"op11"      },
            {L"IPlant" , L"op2"   , L"op21"      },
            {L"JPlant" , L"op3"   , L"op31"      },
            {L"KPlant" , L"op4"   , L"op41"     },
            {L"LPlant" , L"op5"   , L"op51"     },

            {L"MPlant" , L""      , L"MPlant"   },
            {L"NPlant" , L""      , L"NPlant"   },

            {L"OPlant" , L"PPlant", L"PPlant"   },
            {L"PPlant" , L""      , L"PPlant1"  },
            {L"PPlant1", L""      , L"PPlant11" }
        };

    Utf8String tables[] = 
        {
        "EQUIPMENT" , "PIPE"  
        };
    ECSchemaCachePtr cache = ECSchemaCache::Create();
    for(auto& x : data)
        {
        auto name       = x[0];
        auto prefix     = x[1];
        //auto expected   = x[2];
        cache->AddSchema(*GenerateNewECSchema(name, prefix));
        }

    DbResult stat = db.CreateNewDb (projectFile.GetNameUtf8 ().c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test ECDb file failed.";

    auto status = db.Schemas ().ImportECSchemas (*cache, ECDbSchemaManager::ImportOptions (true, false));
    ASSERT_EQ (SUCCESS, status);
    db.ClearECDbCache();
    for(auto& x : data)
        {
        auto name       = x[0];
        auto prefix     = x[1];
        auto expected   = x[2];
        ECSchemaCP out = db.Schemas().GetECSchema(Utf8String(name.c_str()).c_str(), true);
        ASSERT_TRUE (out != nullptr);
        EXPECT_TRUE(out->GetNamespacePrefix().Equals(expected));
        if (out->GetNamespacePrefix().Equals(expected))
            {
            for(auto& table : tables)
                {
                auto tableName = Utf8String(expected.c_str())+ "_" + table;
                EXPECT_TRUE(db.TableExists(tableName.c_str()));
                }
            }
        }
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
    const WCharCP schema =
        L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        L"<ECSchema schemaName=\"SimpleSchema\" nameSpacePrefix=\"adhoc\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        L"<ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.11\" prefix=\"besc\" />"
        L"<ECClass typeName=\"SimpleClass\" isStruct=\"False\" isDomainClass=\"True\">"
        L"<ECProperty propertyName = \"DateTimeProperty\" typeName=\"dateTime\" readOnly=\"True\" />"
        L"<ECProperty propertyName = \"testprop\" typeName=\"int\" />"
        L"<ECCustomAttributes>"
        L"<ClassHasCurrentTimeStampProperty xmlns=\"Bentley_Standard_CustomAttributes.01.11\">"
        L"<PropertyName>DateTimeProperty</PropertyName>"
        L"</ClassHasCurrentTimeStampProperty>"
        L"</ECCustomAttributes>"
        L"</ECClass>"
        L"</ECSchema>";

    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create("checkClassHasCurrentTimeStamp.ecdb");
    ECSchemaPtr simpleSchema;
    auto readContext = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(simpleSchema, schema, *readContext);
    ASSERT_TRUE(simpleSchema != nullptr);
    auto importStatus = db.Schemas().ImportECSchemas(readContext->GetCache());
    ASSERT_TRUE(importStatus == BentleyStatus::SUCCESS);
    auto ecClass = simpleSchema->GetClassP(L"SimpleClass");

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
    ASSERT_TRUE(statement.Step() == ECSqlStepStatus::HasRow);
        {
        ASSERT_FALSE(statement.IsValueNull(0));
        dateTime1 = statement.GetValueDateTime(0);
        }
    ASSERT_TRUE(statement.Step() == ECSqlStepStatus::Done);
    ECSqlStatement updateStatment;
    ecsql = "UPDATE  ONLY adhoc.SimpleClass SET testprop = 23 WHERE ECInstanceId = 1";
    stat = updateStatment.Prepare(db, ecsql.c_str());
    ASSERT_TRUE(updateStatment.Step() == ECSqlStepStatus::Done);
    ecsql = "SELECT DateTimeProperty FROM ";
    ecsql.append(ECSqlBuilder::ToECSqlSnippet(*ecClass));

    BentleyApi::DateTime dateTime2;
    ECSqlStatement statement2;
    stat = statement2.Prepare(db, ecsql.c_str());
    BeThreadUtilities::BeSleep(100); // make sure the time is different by more than the resolution of the timestamp

    ASSERT_TRUE(statement2.Step() == ECSqlStepStatus::HasRow);
        {
        ASSERT_FALSE(statement2.IsValueNull(0));
        dateTime2 = statement2.GetValueDateTime(0);
        }
    ASSERT_TRUE(statement2.Step() == ECSqlStepStatus::Done);
    ASSERT_FALSE(dateTime1 == dateTime2);
    }
END_ECDBUNITTESTS_NAMESPACE
