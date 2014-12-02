/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ECDbInstances_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnCore/ColorUtil.h>
#include <Logging/bentleylogging.h>
#include <DgnPlatform/DgnHandlers/DgnLinkTable.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"DgnECDb"))

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE 
USING_NAMESPACE_BENTLEY_SQLITE_EC

extern Int64 GetV9ElementId (Utf8CP v8Filename, Int64 v8ElementId, DgnProjectR project);

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------------
* Demonstrate that you can iterate the rows in the dgn_Element table using an EC query.
* @bsimethod                                                    Sam.Wilson      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, DgnElement)
    {
    ScopedDgnHost host;
    
    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb");
    DgnProjectR project = *tdm.GetDgnProjectP();

    ECN::ECSchemaP dgnschema = NULL;
    auto schemaStat = project.GetEC().GetSchemaManager ().GetECSchema (dgnschema, "dgn");
    ASSERT_EQ (SUCCESS, schemaStat);
    ECN::ECClassP elementClass = dgnschema->GetClassP (L"Element");
    ASSERT_TRUE (elementClass != NULL);

    Utf8String schemaPrefix = Utf8String (elementClass->GetSchema().GetNamespacePrefix().c_str());
    Utf8String className = Utf8String (elementClass->GetName().c_str());
    SqlPrintfString ecSql ("SELECT * FROM %ls.%ls", schemaPrefix.c_str(), className.c_str());
    ECSqlStatement statement;
    ECSqlStatus prepareStatus = statement.Prepare (project, ecSql.GetUtf8CP());
    ASSERT_TRUE (ECSqlStatus::Success == prepareStatus);

    bset<ElementId> elementInstances;
    ECInstanceECSqlSelectAdapter adapter(statement);
    while (statement.Step() == ECSqlStepStatus::HasRow)
        {
        ECInstanceId id;
        bool status = adapter.GetInstanceId (id);
        ASSERT_TRUE (status);
        ElementRefP ref = project.Models().GetElementById (ElementId(id.GetValue())).get();
        ASSERT_TRUE (ref != NULL);
        elementInstances.insert (ElementId(id.GetValue()));
        }

    WString elementInstancesStr;
    FOR_EACH (ElementId id, elementInstances) {elementInstancesStr.append (WPrintfString(L"%lld ",id.GetValue()));}

    ASSERT_STREQ( elementsStr.c_str(), elementInstancesStr.c_str() );
    }
    
/*---------------------------------------------------------------------------------------
* Demonstrate that you can query for DGN elements by ElementId using an EC query.
* @bsimethod                                                    Krischan.Eberle   03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, DgnElementByElementId)
    {
    ScopedDgnHost host;

    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb");
    DgnProjectR project = *tdm.GetDgnProjectP();

    bvector<ElementId> elements;

    project.Models().FillDictionaryModel ();
    FOR_EACH (PersistentElementRefP ref, project.Models().GetDictionaryModel()->GetElementsCollection ())
        elements.push_back (ref->GetElementId());

    for (auto const& entry : project.Models().MakeIterator ())
        {
        DgnModelP model = project.Models().GetAndFillModelById (NULL, entry.GetModelId(), true);

        FOR_EACH (PersistentElementRefP ref, model->GetElementsCollection ())
            elements.push_back (ref->GetElementId());
        }

    ECN::ECSchemaP dgnschema = NULL;
    auto schemaStat = project.GetEC().GetSchemaManager ().GetECSchema (dgnschema, "dgn");
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

        FOR_EACH (ElementId expectedElementId, elements)
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

        bset<ElementId> expectedElementIds;
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

        bset<ElementId> actualElementIds;
        while (ECSqlStepStatus::HasRow == statement.Step ())
            {
            ECInstanceId actualElementId = statement.GetValueId<ECInstanceId> (0);
            actualElementIds.insert (ElementId (actualElementId.GetValue ()));
            }

        //now check that results are as expected
        WString expectedElementIdsStr;
        FOR_EACH (ElementId id, expectedElementIds) {expectedElementIdsStr.append (WPrintfString(L"%lld ",id.GetValue()));}
        WString actualElementIdsStr;
        FOR_EACH (ElementId id, actualElementIds) {actualElementIdsStr.append (WPrintfString(L"%lld ",id.GetValue()));}

        ASSERT_STREQ (expectedElementIdsStr.c_str(), actualElementIdsStr.c_str());
        }
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   03/2013
//+---------------+---------------+---------------+---------------+---------------+------
ElementId AddLineToModel (DgnModelR model)
    {
    EditElementHandle line;
#if defined (NEEDS_WORK_DGNITEM)
    DSegment3d  segment;

    segment.point[0] = DPoint3d::FromXYZ (0.0, 0.0, 0.0); 
    segment.point[1] = DPoint3d::FromXYZ (100.0, 100.0, 100.0);
#endif
    ExtendedElementHandler::InitializeElement (line, NULL, model, model.Is3d());
    line.AddToModel();

    return line.GetElementId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceId AddDgnLink (DgnProjectR project, ElementId const& elementId, int ordinal, Utf8CP displayLabel, Utf8CP url = nullptr)
    {
    const auto isUrlLink = url != nullptr;
    
    ECSchemaP dgnSchema = nullptr;
    project.GetEC().GetSchemaManager ().GetECSchema (dgnSchema, DGNECSCHEMA_SchemaName);

    //create and insert the link instance
    auto linkClass = isUrlLink ? dgnSchema->GetClassCP (DGNECSCHEMA_CLASSNAME_UrlDgnLink) : dgnSchema->GetClassCP (DGNECSCHEMA_CLASSNAME_DgnLink);
    auto linkInstance = linkClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    linkInstance->SetValue (L"Ordinal", ECValue (ordinal));
    linkInstance->SetValue (L"DisplayLabel", ECValue (displayLabel));
    if (isUrlLink)
        {
        linkInstance->SetValue (L"Url", ECValue (url));
        }

    ECInstanceInserter linkInserter (project, *linkClass);
    ECInstanceKey linkKey;
    linkInserter.Insert (linkKey, *linkInstance);
    WChar linkKeyStr[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH];
    ECInstanceIdHelper::ToString (linkKeyStr, ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH, linkKey.GetECInstanceId ());
    linkInstance->SetInstanceId (linkKeyStr);
    
    //relate link to element
    //For ECDb it is sufficient to use empty ECInstances with only the ECInstanceId set for the ends in the relationship instance.
    //This is faster than querying the ECInstance from ECDb first via the following line: 
    //auto elementInstance = ECDbInstanceAdapter::GetInstanceFromId (*dgnSchema->GetClassCP (DGNECSCHEMA_CLASSNAME_Element), elementId.GetValue (), project);
    WChar elementInstanceId[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH];
    ECInstanceIdHelper::ToString (elementInstanceId, ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH, ECInstanceId (elementId.GetValue ()));
    auto elementInstance = dgnSchema->GetClassCP (DGNECSCHEMA_CLASSNAME_Element)->GetDefaultStandaloneEnabler ()->CreateInstance ();
    elementInstance->SetInstanceId (elementInstanceId);

    auto elementHasLinksRelClass = dgnSchema->GetClassCP (DGNECSCHEMA_CLASSNAME_ElementHasLinks)->GetRelationshipClassCP ();
    auto relEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*elementHasLinksRelClass);
    auto relInstance = relEnabler->CreateRelationshipInstance ();

    relInstance->SetSource (elementInstance.get ());
    relInstance->SetTarget (linkInstance.get ());

    ECInstanceInserter relInserter (project, *elementHasLinksRelClass);
    ECInstanceKey relInstanceKey;
    relInserter.Insert (relInstanceKey, *relInstance);

    return linkKey.GetECInstanceId ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   04/2013
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbInstances, SelectDgnLinksForDgnElement)
    {
    ScopedDgnHost host;

    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb");
    DgnProjectR project = *tdm.GetDgnProjectP();

    auto model = tdm.GetDgnModelP ();

    auto elementId = AddLineToModel (*model);
    auto linkId = AddDgnLink (project, elementId, 1, "Non-URL link");
    auto urlLinkId = AddDgnLink (project, elementId, 2, "URL link", "http://www.foo.com");

    ECSchemaP dgnschema = nullptr;
    auto schemaStat = project.GetEC().GetSchemaManager ().GetECSchema (dgnschema, DGNECSCHEMA_SchemaName);
    ASSERT_EQ (SUCCESS, schemaStat);
    auto elementClass = dgnschema->GetClassCP (DGNECSCHEMA_CLASSNAME_Element);
    ASSERT_TRUE (elementClass != nullptr);
    auto dgnLinkClass = dgnschema->GetClassCP (DGNECSCHEMA_CLASSNAME_DgnLink);
    ASSERT_TRUE (dgnLinkClass != nullptr);
    auto elementHasLinksRelClass = dgnschema->GetClassCP (DGNECSCHEMA_CLASSNAME_ElementHasLinks)->GetRelationshipClassCP ();
    ASSERT_TRUE (elementHasLinksRelClass != nullptr);

    //non-polymorphic query (only for DgnLink objects)
        {
        ECSqlSelectBuilder ecsql;
        ecsql.Select ("l.ECInstanceId").From (*dgnLinkClass, "l", false).Join (*elementClass, "e", false).Using (*elementHasLinksRelClass).Where ("e.ECInstanceId = ?");

        ECSqlStatement statement;
        auto stat = statement.Prepare (project, ecsql.ToString ().c_str ());
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

        statement.BindInt64 (1, elementId.GetValue ());

        int count = 0;
        while (statement.Step () == ECSqlStepStatus::HasRow)
            {
            count++;
            ASSERT_EQ (linkId.GetValue (), statement.GetValueInt64 (0));
            }
        ASSERT_EQ (1, count);
        }

     //polymorphic query (for DgnLink and subclasses)
        {
        ECSqlSelectBuilder ecsql;
        ecsql.Select ("l.ECInstanceId").From (*dgnLinkClass, "l", true).Join (*elementClass, "e", false).Using (*elementHasLinksRelClass).Where ("e.ECInstanceId = ?").OrderBy ("e.ECInstanceId");

        ECSqlStatement statement;
        auto stat = statement.Prepare (project, ecsql.ToString ().c_str ());
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

        statement.BindInt64 (1, elementId.GetValue ());

        int count = 0;
        while (statement.Step () == ECSqlStepStatus::HasRow)
            {
            count++;
            if (count == 1)
                {
                ASSERT_EQ (linkId.GetValue(), statement.GetValueInt64 (0));
                }
            else if (count == 2)
                {
                ASSERT_EQ (urlLinkId.GetValue(), statement.GetValueInt64 (0));
                }
            }

        ASSERT_EQ (2, count);
        }
    }

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
    
static byte s_Utf8BOM[] = {0xef, 0xbb, 0xbf};

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

    UInt64 rawSize;
    fileStatus = file.GetSize (rawSize);
    if (!EXPECTED_CONDITION (BeFileStatus::Success == fileStatus && rawSize <= UINT32_MAX))
        {
        file.Close();
        return false;
        }
    UInt32 sizeToRead = (UInt32) rawSize;

    UInt32 sizeRead;
    ScopedArray<byte> scopedBuffer (sizeToRead);
    byte* buffer = scopedBuffer.GetData();
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

    for (UInt32 ii = 3; ii < sizeRead; ii++)
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
    DgnProjectR project = *tdm.GetDgnProjectP();

    // Make sure that all models are loaded and filled, so that call to dgnFile->FindElementById below will work.
    project.Models().FillDictionaryModel ();
    for (auto const& entry : project.Models().MakeIterator (ModelIterate::All))
        {
        project.Models().GetAndFillModelById (NULL, entry.GetModelId(), true);
        }

    // Get some (arbitrary) reference element
    Int64 selectedElementId = GetV9ElementId ("rxmrlw1f.dgn", 542696, project); 
    ASSERT_TRUE (selectedElementId > 0);
    ElementId shownElementId;
    if (!DgnECPersistence::TryGetAssemblyElementWithPrimaryInstance (shownElementId, ElementId (selectedElementId), project))
        shownElementId = ElementId (selectedElementId);

    // Get all relevant EC information for some arbitrary element
    PersistentElementRefPtr ref = project.Models().GetElementById (shownElementId);
    ASSERT_TRUE (ref.IsValid());
    ECN::ECClassId classId = (ECN::ECClassId) ref->GetECClassId();
    ECClassP ecClass = NULL;
    project.GetEC().GetSchemaManager().GetECClass (ecClass, classId);
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
DgnProjectPtr CreateEmptyProject (DgnProjectPtr& project, WCharCP projectPathname)
    {
    if (BeFileName::DoesPathExist (projectPathname))
        {
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (projectPathname);
        if (fileDeleteStatus != BeFileNameStatus::Success)
            return NULL;
        }

    CreateProjectParams params;
    return DgnProject::CreateProject (NULL, BeFileName(projectPathname), params);
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
bool ImportECSchema (ECSchemaR ecSchema, DgnProjectR project)
    {
    ECSchemaCachePtr schemaList = ECSchemaCache::Create();
    schemaList->AddSchema (ecSchema);
    BentleyStatus importSchemaStatus = project.GetEC().GetSchemaManager ().ImportECSchemas (*schemaList);
    return (SUCCESS == importSchemaStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr CreateStartupCompanyInstance (ECSchemaR startupSchema)
    {
    ECClassP anglesStructClass = startupSchema.GetClassP (L"AnglesStruct");
    if (anglesStructClass == NULL)
        return NULL;
    ECClassP fooClass = startupSchema.GetClassP (L"Foo");
    if (fooClass == NULL)
        return NULL;

    ECValue doubleValue;
    doubleValue.SetDouble (12.345);
    ECValue intValue;
    intValue.SetInteger (67);
    ECValue anglesStructValue;
    IECInstancePtr anglesStruct = anglesStructClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    anglesStruct->SetValue (L"Alpha", doubleValue);
    anglesStruct->SetValue (L"Beta", doubleValue);
    anglesStructValue.SetStruct (anglesStruct.get());

    IECInstancePtr foo = fooClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    ECObjectsStatus status;
    status = foo->SetValue (L"intFoo", intValue);
    status = foo->SetValue (L"doubleFoo", doubleValue);
    status = foo->SetValue (L"anglesFoo.Alpha", doubleValue);
    status = foo->SetValue (L"anglesFoo.Beta", doubleValue);
    foo->AddArrayElements (L"arrayOfIntsFoo", 3);
    foo->AddArrayElements (L"arrayOfAnglesStructsFoo", 3);
    for (int ii = 0; ii < 3; ii++)
        {
         status = foo->SetValue (L"arrayOfIntsFoo", intValue, ii);
         status = foo->SetValue (L"arrayOfAnglesStructsFoo", anglesStructValue, ii);
        }

    return foo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImportECInstance (IECInstanceR instance, DgnProjectR project)
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
    DgnProjectPtr project = CreateEmptyProject (project, projectPathname);
    if (!project.IsValid())
        return false;

    ECSchemaPtr importSchema = ReadECSchemaFromDisk (schemaPathname);
    if (!importSchema.IsValid())
        return false;

    ImportECSchema (*importSchema, *project);
    ECSchemaP startupSchema = NULL;
    auto schemaStat = project->GetEC().GetSchemaManager ().GetECSchema (startupSchema, "StartupCompany");
    if (schemaStat != SUCCESS)
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
bool RetrieveStartupCompanyJson (Json::Value& jsonValue, DgnProjectR project)
    {
    jsonValue = Json::nullValue;

    ECClassP fooClass = NULL;
    project.GetEC().GetSchemaManager().GetECClass (fooClass, "StartupCompany", "Foo");
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

    DgnProjectPtr project = DgnProject::OpenProject (NULL, projectPath, DgnProject::OpenParams(Db::OPEN_Readonly));
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
    BentleyStatus importSchemaStatus = ecDb.GetEC().GetSchemaManager().ImportECSchemas (context->GetCache());
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
    ECClassP documentClass = NULL;
    ecDb.GetEC().GetSchemaManager().GetECClass (documentClass, "eB_PW_CommonSchema_WSB", "Document");
    ASSERT_TRUE (documentClass != NULL);
    ECJsonInserter inserter (ecDb, documentClass->GetId());
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
    ECJsonUpdater updater (ecDb, documentClass->GetId());
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
    if (0 != compare)
        {
        BeFileName beforeUpdateFile;
        BeTest::GetHost().GetOutputRoot (beforeUpdateFile);
        beforeUpdateFile.AppendToPath (L"FieldEngineerStructArray-BeforeUpdate.json");
        WriteJsonToFile (beforeUpdateFile.GetName(), beforeUpdateJson);

        BeFileName afterUpdateFile;
        BeTest::GetHost().GetOutputRoot (afterUpdateFile);
        afterUpdateFile.AppendToPath (L"FieldEngineerStructArray-AfterUpdate.json");
        WriteJsonToFile (afterUpdateFile.GetName(), afterUpdateJson);

        FAIL() << "Json retrieved from db \n\t" << afterUpdateFile.GetName() << "\ndoes not match expected \n\t" << beforeUpdateFile.GetName();
        }
    }

