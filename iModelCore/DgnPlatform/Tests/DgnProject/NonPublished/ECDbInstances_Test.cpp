/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ECDbInstances_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnCore/ColorUtil.h>
#include <Logging/bentleylogging.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"DgnECDb"))

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE 
USING_NAMESPACE_BENTLEY_SQLITE_EC

extern int64_t GetV9ElementId (Utf8CP v8Filename, int64_t v8ElementId, DgnDbR project);

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------------
* Demonstrate that you can iterate the rows in the ElementGraphics table using an EC query.
* @bsimethod                                                    Sam.Wilson      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, DgnElement)
    {
    ScopedDgnHost host;
    
    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb");
    DgnDbR project = *tdm.GetDgnProjectP();

    ECN::ECSchemaP dgnschema = NULL;
    auto schemaStat = project.Schemas ().GetECSchema (dgnschema, DGN_ECSCHEMA_NAME);
    ASSERT_EQ (SUCCESS, schemaStat);
    WString classNameW(DGN_CLASSNAME_ElementGraphics, BentleyCharEncoding::Utf8);
    ECN::ECClassP elementClass = dgnschema->GetClassP(classNameW.c_str());
    ASSERT_TRUE (elementClass != NULL);

    Utf8String schemaPrefix = Utf8String (elementClass->GetSchema().GetNamespacePrefix().c_str());
    Utf8String className = Utf8String (elementClass->GetName().c_str());
    SqlPrintfString ecSql ("SELECT * FROM %ls.%ls", schemaPrefix.c_str(), className.c_str());
    ECSqlStatement statement;
    ECSqlStatus prepareStatus = statement.Prepare (project, ecSql.GetUtf8CP());
    ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);

    bset<DgnElementId> elementInstances;
    ECInstanceECSqlSelectAdapter adapter(statement);
    while (statement.Step() == ECSqlStepStatus::HasRow)
        {
        ECInstanceId id;
        bool status = adapter.GetInstanceId (id);
        ASSERT_TRUE (status);
        ElementRefP ref = project.Models().GetElementById (DgnElementId(id.GetValue())).get();
        ASSERT_TRUE (ref != NULL);
        elementInstances.insert (DgnElementId(id.GetValue()));
        }

    WString elementInstancesStr;
    FOR_EACH (DgnElementId id, elementInstances) {elementInstancesStr.append (WPrintfString(L"%lld ",id.GetValue()));}

    ASSERT_STREQ( elementsStr.c_str(), elementInstancesStr.c_str() );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/07
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnModelP DgnModels::getAndFill(DgnDbR db, DgnModelId modelID)
    {
    DgnModelP dgnModel = db.Models().GetModel (modelID);
    if (dgnModel == NULL)
        return NULL;

    dgnModel->FillModel();
    return  dgnModel;
    }
    
/*---------------------------------------------------------------------------------------
* Demonstrate that you can query for DGN elements by DgnElementId using an EC query.
* @bsimethod                                                    Krischan.Eberle   03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, DgnElementByElementId)
    {
    ScopedDgnHost host;

    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb");
    DgnDbR project = *tdm.GetDgnProjectP();

    bvector<DgnElementId> elements;

    project.Models().FillDictionaryModel ();
    FOR_EACH (PersistentElementRefP ref, project.Models().GetDictionaryModel()->GetElementsCollection ())
        elements.push_back (ref->GetElementId());

    for (auto const& entry : project.Models().MakeIterator ())
        {
        DgnModelP model = getAndFill(project, entry.GetModelId());

        FOR_EACH (PersistentElementRefP ref, model->GetElementsCollection ())
            elements.push_back (ref->GetElementId());
        }

    ECN::ECSchemaP dgnschema = NULL;
    auto schemaStat = project.Schemas ().GetECSchema (dgnschema, "dgn");
    ASSERT_EQ (SUCCESS, schemaStat);
    ECN::ECClassCP elementClass = dgnschema->GetClassCP (L"Element");
    ASSERT_TRUE (elementClass != NULL);

    //WHERE ECInstanceId = ? query
        {
        Utf8String whereClause;
        whereClause.Sprintf ("%s = ?", ECSqlBuilder::ECINSTANCEID_SYSTEMPROPERTY);
        ECSqlSelectBuilder builder;
        builder.Select (ECSqlBuilder::ECINSTANCEID_SYSTEMPROPERTY).From (*elementClass, false).Where (whereClause.c_str ());
        ECSqlStatement statement;
        auto stat = statement.Prepare (project, builder.ToString ().c_str ());
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

        FOR_EACH (DgnElementId expectedElementId, elements)
            {
            statement.Reset ();
            statement.ClearBindings ();
            stat = statement.BindInt64 (1, expectedElementId.GetValue());
            ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

            auto stepStat = statement.Step();
            ASSERT_EQ ((int) ECSqlStepStatus::HasRow, (int) stepStat);
            ECInstanceId actualElementId = statement.GetValueId<ECInstanceId> (0);

            EXPECT_EQ (expectedElementId.GetValue(), actualElementId.GetValue());

            EXPECT_EQ ((int) ECSqlStepStatus::Done, (int) statement.Step ()) << "Query by a single ECInstanceId is expected to only return one row.";
            }
        }

    //now do a WHERE ECInstanceId IN (...) query
        {
        size_t totalElementIdCount = elements.size ();
        size_t testElementIdCount = std::min<size_t> (totalElementIdCount, 10);

        bset<DgnElementId> expectedElementIds;
        Utf8String whereClause;
        whereClause.append (ECSqlBuilder::ECINSTANCEID_SYSTEMPROPERTY);
        whereClause.append (" IN (");
        for (size_t i = 0; i < testElementIdCount; i++)
            {
            expectedElementIds.insert (elements[i]);

            if (i > 0)
                {
                whereClause.append (", ");
                }

            Utf8String ecInstanceIdStr;
            ecInstanceIdStr.Sprintf ("%lld", elements[i].GetValue());
            whereClause.append (ecInstanceIdStr.c_str ());
            }
        whereClause.append (")");

        ECSqlSelectBuilder builder;
        builder.Select (ECSqlBuilder::ECINSTANCEID_SYSTEMPROPERTY).From (*elementClass, false).Where (whereClause.c_str ());
        ECSqlStatement statement;
        auto stat = statement.Prepare (project, builder.ToString ().c_str ());
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

        bset<DgnElementId> actualElementIds;
        while (ECSqlStepStatus::HasRow == statement.Step ())
            {
            ECInstanceId actualElementId = statement.GetValueId<ECInstanceId> (0);
            actualElementIds.insert (DgnElementId (actualElementId.GetValue ()));
            }

        //now check that results are as expected
        WString expectedElementIdsStr;
        FOR_EACH (DgnElementId id, expectedElementIds) {expectedElementIdsStr.append (WPrintfString(L"%lld ",id.GetValue()));}
        WString actualElementIdsStr;
        FOR_EACH (DgnElementId id, actualElementIds) {actualElementIdsStr.append (WPrintfString(L"%lld ",id.GetValue()));}

        ASSERT_STREQ (expectedElementIdsStr.c_str(), actualElementIdsStr.c_str());
        }
    }
#endif

#if defined (NEEDS_WORK_DGNITEM)
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   03/2013
//+---------------+---------------+---------------+---------------+---------------+------
DgnElementId AddLineToModel (DgnModelR model)
    {
    EditElementHandle line;
    DSegment3d  segment;

    segment.point[0] = DPoint3d::FromXYZ (0.0, 0.0, 0.0); 
    segment.point[1] = DPoint3d::FromXYZ (100.0, 100.0, 100.0);
    GraphicElementHandler::InitializeElement (line, model); // WIP: need to pass in a DgnElementId
    line.AddToModel();

    return line.GetElementId();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DebugDumpJson (const Json::Value& jsonValue)
    {
    Utf8String strValue = Json::StyledWriter().write (jsonValue);
    int len = (int) strValue.size();
    for (int ii = 0; ii < len; ii += 1000)
        {
        // Split the string up - logging can't seem to handle long strings
        Utf8String subStr = strValue.substr (ii, 1000);
        LOG.debugv ("%s", subStr.c_str());
        }
    }
    
static Byte s_Utf8BOM[] = {0xef, 0xbb, 0xbf};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ReadStringFromUtf8File (Utf8String& strValue, WCharCP path)
    {
    strValue = "";

    BeFile file;
    BeFileStatus fileStatus = file.Open (path, BeFileAccess::Read);
    if (!EXPECTED_CONDITION (BeFileStatus::Success == fileStatus))
        return false;

    uint64_t rawSize;
    fileStatus = file.GetSize (rawSize);
    if (!EXPECTED_CONDITION (BeFileStatus::Success == fileStatus && rawSize <= UINT32_MAX))
        {
        file.Close();
        return false;
        }
    uint32_t sizeToRead = (uint32_t) rawSize;

    uint32_t sizeRead;
    ScopedArray<Byte> scopedBuffer (sizeToRead);
    Byte* buffer = scopedBuffer.GetData();
    fileStatus = file.Read (buffer, &sizeRead, sizeToRead);
    if (!EXPECTED_CONDITION (BeFileStatus::Success == fileStatus && sizeRead == sizeToRead))
        {
        file.Close();
        return false;
        }

    // Validate it's a UTF8 file
    if (!EXPECTED_CONDITION (buffer[0] == s_Utf8BOM[0] && buffer[1] == s_Utf8BOM[1] && buffer[2] == s_Utf8BOM[2]))
        {
        file.Close();
        return false;
        }

    for (uint32_t ii = 3; ii < sizeRead; ii++)
        {
        if (buffer[ii] == '\n' || buffer[ii] == '\r')
            continue;
        strValue.append (1, buffer[ii]);
        }

    file.Close();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ReadJsonFromFile (Json::Value& jsonValue, WCharCP path)
    {
    Utf8String strValue;
    if (!ReadStringFromUtf8File (strValue, path))
        return false;

    return Json::Reader::Parse(strValue, jsonValue);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool WriteStringToUtf8File (WCharCP path, Utf8StringCR strValue)
    {
#if defined (_WIN32)
    FILE* file = _wfsopen (path, L"w" CSS_UTF8, _SH_DENYWR);
#else
    FILE* file = fopen (Utf8String(path).c_str(), "w");
#endif
    if (file == NULL)
        {
        BeAssert (false);
        return false;
        }
    fwprintf (file, L"%ls", WString(strValue.c_str(), BentleyCharEncoding::Utf8).c_str());
    fclose (file);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool WriteJsonToFile (WCharCP path, const Json::Value& jsonValue)
    {
    Utf8String strValue = Json::StyledWriter().write (jsonValue);
    return WriteStringToUtf8File (path, strValue);
    }
    
#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, JsonValueFormatting)
    {
    ScopedDgnHost host;

    DgnDbTestDgnManager tdm (L"rxmrlw1f.idgndb");
    DgnDbR project = *tdm.GetDgnProjectP();

    // Make sure that all models are loaded and filled, so that call to dgnFile->FindElementById below will work.
    project.Models().FillDictionaryModel ();
    for (auto const& entry : project.Models().MakeIterator (ModelIterate::All))
        {
        project.Models().GetAndFillModelById (NULL, entry.GetModelId(), true);
        }

    // Get some (arbitrary) reference element
    int64_t selectedElementId = GetV9ElementId ("rxmrlw1f.dgn", 542696, project); 
    ASSERT_TRUE (selectedElementId > 0);
    DgnElementId shownElementId;
    if (!DgnECPersistence::TryGetAssemblyElementWithPrimaryInstance (shownElementId, DgnElementId (selectedElementId), project))
        shownElementId = DgnElementId (selectedElementId);

    // Get all relevant EC information for some arbitrary element
    PersistentElementRefPtr ref = project.Models().GetElementById (shownElementId);
    ASSERT_TRUE (ref.IsValid());
    ECN::ECClassId classId = (ECN::ECClassId) ref->GetECClassId();
    ECClassP ecClass = NULL;
    project.Schemas().GetECClass (ecClass, classId);
    ASSERT_TRUE (ecClass != NULL);
    ECInstanceId instanceId = ref->GetECInstanceId();

    // Construct a ECSqlStatement to retrieve the row in the Db
    ECSqlStatement statement;
    Utf8String schemaPrefix = Utf8String (ecClass->GetSchema().GetNamespacePrefix().c_str());
    Utf8String className = Utf8String (ecClass->GetName().c_str());
    SqlPrintfString ecSql ("SELECT * FROM %ls.%ls WHERE ECInstanceId=%lld", schemaPrefix.c_str(), className.c_str(), instanceId.GetValue());
    ECSqlStatus prepareStatus = statement.Prepare (project, ecSql.GetUtf8CP());
    ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);
    ECSqlStepStatus stepStatus = statement.Step();
    ASSERT_TRUE (ECSqlStepStatus::HasRow == stepStatus);

    // Retrieve formatted JSON
    DgnECPropertyFormatterPtr propertyFormatter = DgnECPropertyFormatter::Create (ref->GetDgnModelP());
    JsonECSqlSelectAdapter jsonAdapter (statement, JsonECSqlSelectAdapter::FormatOptions (ECValueFormat::FormattedStrings, propertyFormatter.get()));
    Json::Value actualJson;
    jsonAdapter.GetRowDisplayInfo (actualJson["DisplayInfo"]);
    bool status = jsonAdapter.GetRow (actualJson["Values"]);
    ASSERT_TRUE (status);

    // Read benchmark JSON
    Json::Value expectedJson;
    BeFileName expectedFile;
    BeTest::GetHost().GetDocumentsRoot (expectedFile);
    expectedFile.AppendToPath (L"DgnDb");
    expectedFile.AppendToPath (L"rxmrlw1f.json");
    status = ReadJsonFromFile (expectedJson, expectedFile.GetName());
    ASSERT_TRUE (status);
    
    // Ignore "$ECInstanceId" in comparision - it's too volatile. 
    ASSERT_TRUE (actualJson["Values"].size() == expectedJson["Values"].size());
    for (int ii=0; ii < (int) actualJson["Values"].size(); ii++)
        {
        actualJson["Values"][ii]["$ECInstanceId"] = "*";
        expectedJson["Values"][ii]["$ECInstanceId"] = "*";
        }

    // Validate
    int compare = expectedJson.compare (actualJson);
    if (0 != compare)
        {
        // For convenient android debugging
        //LOG.debugv ("Expected Json:");
        //DebugDumpJson (expectedJson);
        //LOG.debugv ("Actual Json:");
        //DebugDumpJson (actualJson);

        BeFileName actualFile;
        BeTest::GetHost().GetOutputRoot (actualFile);
        actualFile.AppendToPath (L"rxmrlw1f-actual.json");
        WriteJsonToFile (actualFile.GetName(), actualJson);
        FAIL() << "Json retrieved from db \n\t" << actualFile.GetName() << "\ndoes not match expected \n\t" << expectedFile.GetName();
        }
    }
#endif
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr CreateEmptyProject (DgnDbPtr& project, WCharCP projectPathname)
    {
    if (BeFileName::DoesPathExist (projectPathname))
        {
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (projectPathname);
        if (fileDeleteStatus != BeFileNameStatus::Success)
            return NULL;
        }

    CreateDgnDbParams params;
    return DgnDb::CreateDgnDb (NULL, BeFileName(projectPathname), params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr ReadECSchemaFromDisk (WCharCP schemaPathname)
    {
    if (!BeFileName::DoesPathExist (schemaPathname))
        return NULL;
    
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext ();
    WString schemaPath = BeFileName::GetDirectoryName (schemaPathname);
    schemaContext->AddSchemaPath (schemaPath.c_str());

    ECSchemaPtr schema;
    ECSchema::ReadFromXmlFile (schema, schemaPathname, *schemaContext);
    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImportECSchema (ECSchemaR ecSchema, DgnDbR project)
    {
    ECSchemaCachePtr schemaList = ECSchemaCache::Create();
    schemaList->AddSchema (ecSchema);
    BentleyStatus importSchemaStatus = project.Schemas ().ImportECSchemas (*schemaList);
    return (SUCCESS == importSchemaStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr CreateStartupCompanyInstance (ECSchemaCR startupSchema)
    {
    ECClassCP anglesStructClass = startupSchema.GetClassCP ("AnglesStruct");
    if (anglesStructClass == NULL)
        return NULL;
    ECClassCP fooClass = startupSchema.GetClassCP ("Foo");
    if (fooClass == NULL)
        return NULL;

    ECValue doubleValue;
    doubleValue.SetDouble (12.345);
    ECValue intValue;
    intValue.SetInteger (67);
    ECValue anglesStructValue;
    IECInstancePtr anglesStruct = anglesStructClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    anglesStruct->SetValue ("Alpha", doubleValue);
    anglesStruct->SetValue ("Beta", doubleValue);
    anglesStructValue.SetStruct (anglesStruct.get());

    IECInstancePtr foo = fooClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    ECObjectsStatus status;
    status = foo->SetValue ("intFoo", intValue);
    status = foo->SetValue ("doubleFoo", doubleValue);
    status = foo->SetValue ("anglesFoo.Alpha", doubleValue);
    status = foo->SetValue ("anglesFoo.Beta", doubleValue);
    foo->AddArrayElements ("arrayOfIntsFoo", 3);
    foo->AddArrayElements ("arrayOfAnglesStructsFoo", 3);
    for (int ii = 0; ii < 3; ii++)
        {
         status = foo->SetValue ("arrayOfIntsFoo", intValue, ii);
         status = foo->SetValue ("arrayOfAnglesStructsFoo", anglesStructValue, ii);
        }

    return foo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImportECInstance (IECInstanceR instance, DgnDbR project)
    {
    ECClassCR ecClass = instance.GetClass();
    ECInstanceInserter inserter (project, ecClass);
    if (!inserter.IsValid())
        return false;

    ECInstanceKey instanceKey;
    BentleyStatus insertStatus = inserter.Insert (instanceKey, instance);
    return (SUCCESS == insertStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool CreateStartupCompanyProject (WCharCP projectPathname, WCharCP schemaPathname)
    {
    DgnDbPtr project = CreateEmptyProject (project, projectPathname);
    if (!project.IsValid())
        return false;

    ECSchemaPtr importSchema = ReadECSchemaFromDisk (schemaPathname);
    if (!importSchema.IsValid())
        return false;

    ImportECSchema (*importSchema, *project);
    ECSchemaCP startupSchema = project->Schemas ().GetECSchema ("StartupCompany");
    if (startupSchema == nullptr)
        return false;
        
    IECInstancePtr instance = CreateStartupCompanyInstance (*startupSchema);
    if (!instance.IsValid())
        return false;

    if (!ImportECInstance (*instance, *project))
        return false;

    project->SaveChanges();
    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RetrieveStartupCompanyJson (Json::Value& jsonValue, DgnDbR project)
    {
    jsonValue = Json::nullValue;

    ECClassCP fooClass = project.Schemas().GetECClass ("StartupCompany", "Foo");
    if (fooClass == NULL)
        return false;

    ECSqlStatement statement;
    BeTest::SetFailOnAssert (false); // TODO: Reported the assertion to Affan. Remove this after the issue is fixed.
    ECSqlStatus prepareStatus = statement.Prepare (project, "SELECT * FROM ONLY stco.Foo");
    BeTest::SetFailOnAssert (true);
    if (ECSqlStatus::Success != prepareStatus)
        return false;
    ECSqlStepStatus stepStatus = statement.Step();
    if (stepStatus != ECSqlStepStatus::HasRow)
        return false;

    // Create Json
    JsonECSqlSelectAdapter jsonAdapter (statement);
    jsonAdapter.GetRowDisplayInfo (jsonValue);
    bool status = jsonAdapter.GetRow (jsonValue["Row"]);
    if (!status)
        {
        jsonValue = Json::nullValue;
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, JsonValueStructure)
    {
    ScopedDgnHost host;

    BeFileName projectPath;
    BeTest::GetHost().GetOutputRoot (projectPath);
    projectPath.AppendToPath (L"StartupCompany.idgndb");

    BeFileName ecSchemaPath;
    BeTest::GetHost().GetDocumentsRoot (ecSchemaPath);
    ecSchemaPath.AppendToPath (L"DgnDb\\ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

    bool status = CreateStartupCompanyProject (projectPath.GetName(), ecSchemaPath.GetName());
    ASSERT_TRUE (status) << "Could not create test project";

    DgnDbPtr project = DgnDb::OpenDgnDb (NULL, projectPath, DgnDb::OpenParams(Db::OpenMode::Readonly));
    ASSERT_TRUE (project.IsValid()) << "Cound not open test project";
    
    Json::Value actualJson;
    status = RetrieveStartupCompanyJson (actualJson, *project);
    ASSERT_TRUE (status) << "Could not retrieve test json";

    // Read benchmark JSON
    Json::Value expectedJson;
    BeFileName expectedFile;
    BeTest::GetHost().GetDocumentsRoot (expectedFile);
    expectedFile.AppendToPath (L"DgnDb");
    expectedFile.AppendToPath (L"StartupCompany.json");
    status = ReadJsonFromFile (expectedJson, expectedFile.GetName());
    ASSERT_TRUE (status);

    // Validate
    int compare = expectedJson.compare (actualJson);
    if (0 != compare)
        {
        BeFileName actualFile;
        BeTest::GetHost().GetOutputRoot (actualFile);
        actualFile.AppendToPath (L"StartupCompany-actual.json");
        WriteJsonToFile (actualFile.GetName(), actualJson);
        FAIL() << "Json retrieved from db \n\t" << actualFile.GetName() << "\ndoes not match expected \n\t" << expectedFile.GetName();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                   Ramanujam.Raman                  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ECDbInstances, FieldEngineerStructArray)
    {
    // TODO: This test needs to move to the ECDb layer in Graphite03. 
    ScopedDgnHost host;

    // Create a new empty db
    ECDb ecDb;
    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot (outputDir);
    BeFileName projectFile (NULL, outputDir.GetName(), L"FieldEngineerStructArray.dgndb", NULL);
    if (BeFileName::DoesPathExist (projectFile.GetName()))
        {
        // Delete any previously created file
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (projectFile.GetName());
        ASSERT_TRUE (BeFileNameStatus::Success == fileDeleteStatus);
        }
    DbResult result = ecDb.CreateNewDb (projectFile.GetNameUtf8().c_str());
    ASSERT_EQ (BE_SQLITE_OK, result);

    /* Read the WSB schema */

    // Construct the schema pathname
    BeFileName schemaPath;
    BeTest::GetHost().GetDocumentsRoot (schemaPath);
    schemaPath.AppendToPath (L"DgnDb\\ECDb\\Schemas");
    BeFileName schemaPathname (schemaPath);
    schemaPathname.AppendToPath (L"eB_PW_CommonSchema_WSB.01.00.ecschema.xml");
    ASSERT_TRUE (BeFileName::DoesPathExist (schemaPathname.GetName()));

    // Read the sample schema from disk
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext ();
    context->AddSchemaPath (schemaPathname.GetName()); // Setup locating any dependencies in the same folder
    ECSchemaPtr schema;
    ECSchema::ReadFromXmlFile (schema, schemaPathname.GetName(), *context);
    ASSERT_TRUE (schema.IsValid());

    /* Import schema into Db */
    BentleyStatus importSchemaStatus = ecDb.Schemas().ImportECSchemas (context->GetCache());
    ASSERT_EQ (SUCCESS, importSchemaStatus);

    /* Read JSON from file */
    Json::Value beforeImportJson;
    BeFileName beforeImportFile;
    BeTest::GetHost().GetDocumentsRoot (beforeImportFile);
    beforeImportFile.AppendToPath (L"DgnDb");
    beforeImportFile.AppendToPath (L"FieldEngineerStructArray.json");
    bool status = ReadJsonFromFile (beforeImportJson, beforeImportFile.GetName());
    ASSERT_TRUE (status);

    /* Import JSON into Db */
    ECClassCP documentClass = ecDb.Schemas().GetECClass ("eB_PW_CommonSchema_WSB", "Document");
    ASSERT_TRUE (documentClass != NULL);
    JsonInserter inserter (ecDb, *documentClass);
    StatusInt insertStatus = inserter.Insert (beforeImportJson);
    ASSERT_TRUE (insertStatus == SUCCESS);
    ecDb.SaveChanges();

    /* Retrieve the previously imported instance as JSON */
    ECSqlStatement statement;
    ECSqlStatus prepareStatus = statement.Prepare (ecDb, "SELECT * FROM ONLY eBPWC.Document");
    ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);
    ECSqlStepStatus stepStatus = statement.Step();
    ASSERT_TRUE (stepStatus == ECSqlStepStatus::HasRow); 

    Json::Value afterImportJson;
    JsonECSqlSelectAdapter jsonAdapter (statement, JsonECSqlSelectAdapter::FormatOptions (ECValueFormat::RawNativeValues));
    status = jsonAdapter.GetRowInstance (afterImportJson, documentClass->GetId());
    ASSERT_TRUE (status);

   /* Validate */
    int compare = beforeImportJson.compare (afterImportJson);
    if (0 != compare)
        {
        BeFileName afterImportFile;
        BeTest::GetHost().GetOutputRoot (afterImportFile);
        afterImportFile.AppendToPath (L"FieldEngineerStructArray-AfterImport.json");
        WriteJsonToFile (afterImportFile.GetName(), afterImportJson);
        FAIL() << "Json retrieved from db \n\t" << afterImportFile.GetName() << "\ndoes not match expected \n\t" << beforeImportFile.GetName();
        }

    /* Update */
    JsonUpdater updater (ecDb, *documentClass);
    Json::Value beforeUpdateJson = afterImportJson;
    beforeUpdateJson["Location"]["Coordinates"][0]["x"] = 1.11111;
    beforeUpdateJson["Location"]["Coordinates"][0]["y"] = 2.22222;
    beforeUpdateJson["Location"]["Coordinates"][0]["z"] = 3.33333;
    StatusInt updateStatus = updater.Update (beforeUpdateJson); // Uses ECInstanceId in the currentObject to find the entry to update
    ASSERT_TRUE (updateStatus == SUCCESS);

    /* Retrieve */
    Json::Value afterUpdateJson = Json::nullValue;
    statement.Finalize();
    prepareStatus = statement.Prepare (ecDb, "SELECT * FROM ONLY eBPWC.Document");
    ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);
    stepStatus = statement.Step();
    ASSERT_TRUE (stepStatus == ECSqlStepStatus::HasRow); 
    JsonECSqlSelectAdapter jsonAdapter2 (statement, JsonECSqlSelectAdapter::FormatOptions (ECValueFormat::RawNativeValues));
    status = jsonAdapter2.GetRowInstance(afterUpdateJson, documentClass->GetId());
    ASSERT_TRUE (status);

    /* Validate */
    compare = beforeUpdateJson.compare (afterUpdateJson);
    //if (0 != compare)
        BeFileName beforeUpdateFile;
        BeTest::GetHost().GetOutputRoot (beforeUpdateFile);
        beforeUpdateFile.AppendToPath (L"FieldEngineerStructArray-BeforeUpdate.json");
        WriteJsonToFile (beforeUpdateFile.GetName(), beforeUpdateJson);

        BeFileName afterUpdateFile;
        BeTest::GetHost().GetOutputRoot (afterUpdateFile);
        afterUpdateFile.AppendToPath (L"FieldEngineerStructArray-AfterUpdate.json");
        WriteJsonToFile (afterUpdateFile.GetName(), afterUpdateJson);

        ASSERT_EQ (0, compare) << "Json retrieved from db \n\t" << afterUpdateFile.GetName() << "\ndoes not match expected \n\t" << beforeUpdateFile.GetName();
    }

#ifdef NOT_NOW
struct DgnECInstanceTests : public testing::Test
    {
    private:
        static double s_xCoord;
        static double s_yCoord;
        static double s_zCoord;

        static double s_increment;

    public:
        StatusInt CreateArbitraryElement (EditElementHandleR editElementHandle, DgnModelR model);


    typedef void (*PopulatePrimitiveValueCallback)(ECN::ECValueR value, ECN::PrimitiveType primitiveType, ECN::ECPropertyCP ecproperty);
    static BentleyStatus SetECInstanceId (ECN::IECInstanceR instance, ECInstanceId const& instanceId);

    static void            PopulateStructValue (ECN::ECValueR value, ECN::ECClassCR structType, PopulatePrimitiveValueCallback callback);

    static ECN::ECObjectsStatus CopyStruct (ECN::IECInstanceR source, ECN::ECValuesCollectionCR collection, WCharCP baseAccessPath);
    static ECN::ECObjectsStatus CopyStruct (ECN::IECInstanceR target, ECN::IECInstanceCR structValue, WCharCP propertyName);

    static void PopulatePrimitiveValueWithRandomValues (ECN::ECValueR ecValue, ECN::PrimitiveType primitiveType, ECN::ECPropertyCP ecProperty);
    static ECN::IECInstancePtr  CreateArbitraryECInstance (ECN::ECClassCR ecClass, PopulatePrimitiveValueCallback callback = PopulatePrimitiveValueWithRandomValues, bool skipStructs = false, bool skipArrays = false);
    static ECN::IECInstancePtr  CreateECInstance (ECN::ECClassCR ecClass);
    static void                 PopulateECInstance (ECN::IECInstancePtr ecInstance, PopulatePrimitiveValueCallback callback = PopulatePrimitiveValueWithRandomValues, bool skipStructs = false, bool skipArrays = false);

    };

IECInstancePtr DgnECInstanceTests::CreateArbitraryECInstance (ECClassCR ecClass, PopulatePrimitiveValueCallback populatePrimitiveValueCallback, bool skipStructs, bool skipArrays)
    {
    IECInstancePtr instance = ecClass.GetDefaultStandaloneEnabler()->CreateInstance(0);
    PopulateECInstance (instance, populatePrimitiveValueCallback, skipStructs, skipArrays);
    return instance;
    }

IECInstancePtr DgnECInstanceTests::CreateECInstance 
(
ECClassCR ecClass
)
    {
    StandaloneECEnablerP instanceEnabler = ecClass.GetDefaultStandaloneEnabler ();
    POSTCONDITION (instanceEnabler != nullptr, nullptr);
    IECInstancePtr instance = instanceEnabler->CreateInstance ();
    POSTCONDITION (instance != nullptr, nullptr);
    return instance;
    }

void DgnECInstanceTests::PopulateECInstance (ECN::IECInstancePtr ecInstance, PopulatePrimitiveValueCallback populatePrimitiveValueCallback, bool skipStructs, bool skipArrays)
    {
    ECValue value;
    for (ECPropertyCP ecProperty : ecInstance->GetClass().GetProperties(true))
        {
        if (!skipStructs && ecProperty->GetIsStruct())
            {
            PopulateStructValue (value, ecProperty->GetAsStructProperty()->GetType(), populatePrimitiveValueCallback);
            CopyStruct(*ecInstance, *value.GetStruct(), ecProperty->GetName().c_str());
            }
        else if (ecProperty->GetIsPrimitive())
            {
            populatePrimitiveValueCallback (value, ecProperty->GetAsPrimitiveProperty()->GetType(), ecProperty);
            ecInstance->SetValue (ecProperty->GetName().c_str(), value);
            }
        else if (!skipArrays && ecProperty->GetIsArray())
            {
            ArrayECPropertyCP arrayProperty = ecProperty->GetAsArrayProperty();
            if(arrayProperty->GetKind() == ARRAYKIND_Primitive && arrayProperty->GetPrimitiveElementType() == PRIMITIVETYPE_IGeometry)
                continue;

            uint32_t arrayCount = 5;
            if (arrayCount < arrayProperty->GetMinOccurs ())
                arrayCount = arrayProperty->GetMinOccurs ();
            else if (arrayCount > arrayProperty->GetMaxOccurs ())
                arrayCount = arrayProperty->GetMaxOccurs ();

            ecInstance->AddArrayElements (ecProperty->GetName ().c_str (), arrayCount);
            if (arrayProperty->GetKind() == ARRAYKIND_Struct)
                {
                for (uint32_t i = 0; i < arrayCount; i++)
                    {
                    PopulateStructValue (value, *arrayProperty->GetStructElementType(), populatePrimitiveValueCallback);
                    ecInstance->SetValue (ecProperty->GetName().c_str (), value, i);
                    }
                }
            else if (arrayProperty->GetKind() == ARRAYKIND_Primitive )
                {
                for (uint32_t i = 0; i < arrayCount; i++)
                    {
                    populatePrimitiveValueCallback (value, arrayProperty->GetPrimitiveElementType(), ecProperty);
                    ecInstance->SetValue (ecProperty->GetName().c_str (), value, i);
                    }
                }
            }
        }
    }

ECObjectsStatus DgnECInstanceTests::CopyStruct(IECInstanceR source, ECValuesCollectionCR collection, WCharCP baseAccessPath)
    { 
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    for(auto& propertyValue : collection)
        {
        auto pvAccessString = propertyValue.GetValueAccessor().GetPropertyName();
        auto accessString =  baseAccessPath == nullptr ? pvAccessString : WString(baseAccessPath) + L"." + pvAccessString;

        if (propertyValue.HasChildValues())
            {
            status = CopyStruct (source, *propertyValue.GetChildValues(), accessString.c_str());
            if (status != ECOBJECTS_STATUS_Success)
                {
                return status;
                }
            continue;
            }

        auto& location = propertyValue.GetValueAccessor().DeepestLocationCR();

        //auto property = location.GetECProperty(); 
        //BeAssert(property != nullptr);
        if (location.GetArrayIndex()>= 0)
            {
            source.AddArrayElements(accessString.c_str(), 1);
            status = source.SetValue (accessString.c_str(), propertyValue.GetValue(), location.GetArrayIndex()); 
            }
        else
            status = source.SetValue (accessString.c_str(), propertyValue.GetValue()); 

        if (status != ECOBJECTS_STATUS_Success)
            {
            return status;
            }
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
ECObjectsStatus DgnECInstanceTests::CopyStruct(IECInstanceR target, IECInstanceCR structValue, WCharCP propertyName)
    {
    return CopyStruct(target, *ECValuesCollection::Create(structValue), propertyName);
    }


void DgnECInstanceTests::PopulateStructValue (ECValueR value, ECClassCR structType, PopulatePrimitiveValueCallback populatePrimitiveValueCallback)
    {
    value.Clear();
    IECInstancePtr inst = CreateArbitraryECInstance (structType, populatePrimitiveValueCallback);
    value.SetStruct(inst.get());
    }

//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>11/2013</date>
//---------------+---------------+---------------+---------------+---------------+-----
void DgnECInstanceTests::PopulatePrimitiveValueWithRandomValues (ECValueR ecValue, PrimitiveType primitiveType, ECPropertyCP ecProperty)
    {
    ecValue.Clear();

    int randomNumber = rand ();
    switch (primitiveType)
        {
        case PRIMITIVETYPE_String: 
            {
            Utf8String text;
            text.Sprintf ("Sample text with random number: %d", randomNumber);
            ecValue.SetUtf8CP(text.c_str (), true); 
            }
            break;

        case PRIMITIVETYPE_Integer: 
            {
            ecValue.SetInteger(randomNumber); 
            }
            break;

        case PRIMITIVETYPE_Long: 
            {
            const int32_t intMax = std::numeric_limits<int32_t>::max ();
            const int64_t longValue = static_cast<int64_t> (intMax) + randomNumber;
            ecValue.SetLong(longValue); 
            }
            break;

        case PRIMITIVETYPE_Double: 
            {
            ecValue.SetDouble(randomNumber * PI);
            }
            break;

        case PRIMITIVETYPE_DateTime: 
            {
            DateTime utcTime = DateTime::GetCurrentTimeUtc ();
            ecValue.SetDateTime(utcTime); 
            }
            break;

        case PRIMITIVETYPE_Boolean: 
            {
            ecValue.SetBoolean(randomNumber % 2 != 0); 
            }
            break;

        case PRIMITIVETYPE_Point2D: 
            {
            DPoint2d point2d;
            point2d.x=randomNumber * 1.0;
            point2d.y=randomNumber * 1.8;
            ecValue.SetPoint2D(point2d);
            break;
            }
        case PRIMITIVETYPE_Point3D:
            {
            DPoint3d point3d;
            point3d.x=randomNumber * 1.0;
            point3d.y=randomNumber * 1.8;
            point3d.z=randomNumber * 2.9;
            ecValue.SetPoint3D(point3d);
            break;
            }

        default:
            break;
        }
    }

double DgnECInstanceTests::s_increment = 5.0;
double DgnECInstanceTests::s_xCoord = 0.0;
double DgnECInstanceTests::s_yCoord = 0.0;
double DgnECInstanceTests::s_zCoord = 0.0;

StatusInt DgnECInstanceTests::CreateArbitraryElement (EditElementHandleR editElementHandle, DgnModelR model)
    {
    DSegment3d  segment;
    segment.point[0] = DPoint3d::FromXYZ (s_xCoord, s_yCoord, s_zCoord); 
    segment.point[1] = DPoint3d::FromXYZ (s_xCoord + s_increment, s_yCoord + s_increment, s_zCoord + s_increment);

    s_xCoord += 2 * s_increment;
    s_yCoord += 2 * s_increment;
    s_zCoord += 2 * s_increment;

    return LineHandler::CreateLineElement (editElementHandle, NULL, segment, model.Is3d(), model);
    }

BentleyStatus DgnECInstanceTests::SetECInstanceId (ECN::IECInstanceR instance, ECInstanceId const& ecInstanceId)
    {
    if (!ecInstanceId.IsValid ())
        return ERROR;

    WChar instanceIdStr[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH];
    if (!ECInstanceIdHelper::ToString (instanceIdStr, ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH, ecInstanceId))
        {
        LOG.errorv ("Could not set ECInstanceId %lld on the ECInstanceId. Conversion to string failed.", ecInstanceId.GetValue ());
        BeAssert (false && "Could not set ECInstanceId %lld on the ECInstanceId. Conversion to string failed.");
        return ERROR;
        }

    const auto ecstat = instance.SetInstanceId (instanceIdStr);
    return ecstat == ECOBJECTS_STATUS_Success ? SUCCESS : ERROR;
    }

TEST_F(DgnECInstanceTests, InstancesAndRelationships)
    {
    ScopedDgnHost host;

    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb");
    auto dgnFile = tdm.GetLoadedDgnPtr ();
    auto& project = dgnFile->GetDgnProject ();

    auto model = tdm.GetDgnModelP ();

    ECSchemaPtr testSchema;
    ECSchema::CreateSchema(testSchema, L"TestSchema", 1, 2);
    testSchema->SetNamespacePrefix(L"ts");
    testSchema->SetDescription(L"Schema for testing programmatic construction");
    testSchema->SetDisplayLabel(L"Test Schema");

    ECClassP widgetClass;
    ECClassP fooClass;
    ECClassP barClass;

    testSchema->CreateClass(widgetClass, L"Widget");
    testSchema->CreateClass(fooClass, L"Foo");
    testSchema->CreateClass(barClass, L"Bar");

    PrimitiveECPropertyP widgetNameProp;
    PrimitiveECPropertyP valueProp;
    PrimitiveECPropertyP longProp;
    PrimitiveECPropertyP dateTimeProp;
    PrimitiveECPropertyP boolProp;
    PrimitiveECPropertyP point2dProp;
    widgetClass->CreatePrimitiveProperty(widgetNameProp, L"Name", PRIMITIVETYPE_String);
    widgetClass->CreatePrimitiveProperty(valueProp, L"Value", PRIMITIVETYPE_Integer);
    widgetClass->CreatePrimitiveProperty(longProp, L"LongValue", PRIMITIVETYPE_Long);
    widgetClass->CreatePrimitiveProperty(dateTimeProp, L"Created", PRIMITIVETYPE_DateTime);
    widgetClass->CreatePrimitiveProperty(boolProp, L"Bool", PRIMITIVETYPE_Boolean);
    widgetClass->CreatePrimitiveProperty(point2dProp, L"StartPoint", PRIMITIVETYPE_Point2D);

    PrimitiveECPropertyP fooNameProp;
    fooClass->CreatePrimitiveProperty(fooNameProp, L"Name", PRIMITIVETYPE_String);

    PrimitiveECPropertyP barNameProp;
    PrimitiveECPropertyP numberProp;
    barClass->CreatePrimitiveProperty(barNameProp, L"Name", PRIMITIVETYPE_String);
    barClass->CreatePrimitiveProperty(numberProp, L"Number", PRIMITIVETYPE_Integer);

    ECRelationshipClassP widgetHasFoo;
    testSchema->CreateRelationshipClass(widgetHasFoo, L"WidgetHasFoo");
    widgetHasFoo->SetIsDomainClass(true);
    widgetHasFoo->GetSource().AddClass(*widgetClass);
    widgetHasFoo->GetTarget().AddClass(*fooClass);
    widgetHasFoo->GetSource().SetCardinality(RelationshipCardinality::OneMany());
    widgetHasFoo->GetTarget().SetCardinality(RelationshipCardinality::OneMany());

    ECRelationshipClassP widgetHasBar;
    testSchema->CreateRelationshipClass(widgetHasBar, L"WidgetHasBar");
    widgetHasBar->SetIsDomainClass(true);
    widgetHasBar->GetSource().AddClass(*widgetClass);
    widgetHasBar->GetTarget().AddClass(*barClass);
    widgetHasBar->GetSource().SetCardinality(RelationshipCardinality::OneMany());
    widgetHasBar->GetTarget().SetCardinality(RelationshipCardinality::OneMany());

    ECSchemaCachePtr schemaList = ECSchemaCache::Create();
    schemaList->AddSchema (*testSchema);
    project.GetEC().Schemas ().ImportECSchemas (*schemaList);
    
    bvector<IECInstancePtr> orphanedWidgets;
    bvector<IECInstancePtr> orphanedBars;
    bvector<IECInstancePtr> orphanedFoos;

    bvector<IECInstancePtr> lineWidgets;
    bvector<IECInstancePtr> lineBars;

    ECInstanceInserter widgetInserter (*tdm.GetDgnProjectP(), *widgetClass);
    ECInstanceInserter fooInserter (*tdm.GetDgnProjectP(), *fooClass);
    ECInstanceInserter barInserter (*tdm.GetDgnProjectP(), *barClass);

    for (int i = 0; i < 5; i++)
        {
        ECValue name;
        ECInstanceKey ecInstanceKey;

        IECInstancePtr widget = CreateArbitraryECInstance(*widgetClass, PopulatePrimitiveValueWithRandomValues);
        widget->GetValue(name, L"Name");
        WString nameStr;
        nameStr.Sprintf(L"(orphaned) %ls", name.GetString());
        name.SetString(nameStr.c_str());
        widget->SetValue(L"Name", name);

        orphanedWidgets.push_back(widget);
        widgetInserter.Insert(ecInstanceKey, *widget);
        SetECInstanceId(*widget, ecInstanceKey.GetECInstanceId());

        IECInstancePtr bar = CreateArbitraryECInstance(*barClass, PopulatePrimitiveValueWithRandomValues);
        bar->GetValue(name, L"Name");
        nameStr.Sprintf(L"(orphaned) %ls", name.GetString());
        name.SetString(nameStr.c_str());
        bar->SetValue(L"Name", name);
        
        orphanedBars.push_back(bar);
        barInserter.Insert(ecInstanceKey, *bar);
        SetECInstanceId(*bar, ecInstanceKey.GetECInstanceId());

        IECInstancePtr foo = CreateArbitraryECInstance(*fooClass, PopulatePrimitiveValueWithRandomValues);
        foo->GetValue(name, L"Name");
        nameStr.Sprintf(L"(orphaned) %ls", name.GetString());
        name.SetString(nameStr.c_str());
        foo->SetValue(L"Name", name);
        orphanedFoos.push_back(foo);
        fooInserter.Insert(ecInstanceKey, *foo);
        SetECInstanceId(*foo, ecInstanceKey.GetECInstanceId());

        }

    // add some widget instances to line elements
    for (int i = 0; i < 5; i++)
        {
        StatusInt status;
        EditElementHandle* eeh = new EditElementHandle();
        status = CreateArbitraryElement (*eeh, *model);
        ASSERT_EQ(SUCCESS, status);

        ECValue name;
        IECInstancePtr widget = CreateArbitraryECInstance(*widgetClass, PopulatePrimitiveValueWithRandomValues);
        widget->GetValue(name, L"Name");
        WString nameStr;
        nameStr.Sprintf(L"(line) %ls", name.GetString());
        name.SetString(nameStr.c_str());
        widget->SetValue(L"Name", name);
        lineWidgets.push_back(widget);
        ECInstanceKey ecInstanceKey;
        widgetInserter.Insert(ecInstanceKey, *widget);
        SetECInstanceId(*widget, ecInstanceKey.GetECInstanceId());

        StatusInt stat2 = DgnECPersistence::SetPrimaryInstanceOnElement (*eeh, ecInstanceKey, project);
        ASSERT_EQ (SUCCESS, stat2);
        ASSERT_EQ (SUCCESS, eeh->AddToModel());
        }

    // add some bar instances to line elements
    for (int i = 0; i < 5; i++)
        {
        StatusInt status;
        EditElementHandle* eeh = new EditElementHandle();
        status = CreateArbitraryElement (*eeh, *model);
        ASSERT_EQ(SUCCESS, status);

        IECInstancePtr bar = CreateArbitraryECInstance(*barClass, PopulatePrimitiveValueWithRandomValues);
        ECValue name;
        bar->GetValue(name, L"Name");
        WString nameStr;
        nameStr.Sprintf(L"(line) %ls", name.GetString());
        name.SetString(nameStr.c_str());
        bar->SetValue(L"Name", name);
        lineBars.push_back(bar);
        ECInstanceKey ecInstanceKey;
        barInserter.Insert(ecInstanceKey, *bar);
        SetECInstanceId(*bar, ecInstanceKey.GetECInstanceId());

        StatusInt stat2 = DgnECPersistence::SetPrimaryInstanceOnElement (*eeh, ecInstanceKey, project);
        ASSERT_EQ (SUCCESS, stat2);
        ASSERT_EQ (SUCCESS, eeh->AddToModel());
        }

    auto widgetHasBarEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*widgetHasBar);
    auto widgetHasFooEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*widgetHasFoo);

    auto widgetHasFooInstance = widgetHasFooEnabler->CreateRelationshipInstance ();

    widgetHasFooInstance->SetSource (orphanedWidgets[0].get ());
    widgetHasFooInstance->SetTarget (orphanedFoos[0].get ());

    ECInstanceInserter widgetHasFooInserter (project, *widgetHasFoo);
    ECInstanceKey relInstanceKey;
    widgetHasFooInserter.Insert (relInstanceKey, *widgetHasFooInstance);

    ECInstanceInserter widgetHasBarInserter(project, *widgetHasBar);
    auto widgetHasBarInstance = widgetHasBarEnabler->CreateRelationshipInstance();
    widgetHasBarInstance->SetSource(orphanedWidgets[2].get());
    widgetHasBarInstance->SetTarget(lineBars[0].get());
    widgetHasBarInserter.Insert(relInstanceKey, *widgetHasBarInstance);

    widgetHasBarInstance = widgetHasBarEnabler->CreateRelationshipInstance();
    widgetHasBarInstance->SetSource(lineWidgets[0].get());
    widgetHasBarInstance->SetTarget(lineBars[1].get());
    widgetHasBarInserter.Insert(relInstanceKey, *widgetHasBarInstance);

    widgetHasBarInstance = widgetHasBarEnabler->CreateRelationshipInstance();
    widgetHasBarInstance->SetSource(lineWidgets[1].get());
    widgetHasBarInstance->SetTarget(orphanedBars[0].get());
    widgetHasBarInserter.Insert(relInstanceKey, *widgetHasBarInstance);

    project.SaveChanges();
    }
#endif