/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ECDbInstances_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/ColorUtil.h>
#include <Logging/bentleylogging.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"DgnECDb"))

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

extern int64_t GetV9ElementId(Utf8CP v8Filename, int64_t v8ElementId, DgnDbR project);

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------------
* Demonstrate that you can iterate the rows in the ElementGraphics table using an EC query.
* @bsimethod                                                    Sam.Wilson      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, DgnElement)
    {
    ScopedDgnHost host;

    DgnDbTestDgnManager tdm(L"3dMetricGeneral.ibim");
    DgnDbR project = *tdm.GetDgnProjectP();

    ECN::ECSchemaP dgnschema = NULL;
    auto schemaStat = project.Schemas().GetECSchema(dgnschema, BIS_ECSCHEMA_NAME);
    ASSERT_EQ(SUCCESS, schemaStat);
    WString classNameW(BIS_CLASS_ElementGraphics, BentleyCharEncoding::Utf8);
    ECN::ECClassP elementClass = dgnschema->GetClassP(classNameW.c_str());
    ASSERT_TRUE(elementClass != NULL);

    Utf8String schemaPrefix = Utf8String(elementClass->GetSchema().GetNamespacePrefix().c_str());
    Utf8String className = Utf8String(elementClass->GetName().c_str());
    SqlPrintfString ecSql("SELECT * FROM %ls.%ls", schemaPrefix.c_str(), className.c_str());
    ECSqlStatement statement;
    ECSqlStatus prepareStatus = statement.Prepare(project, ecSql.GetUtf8CP());
    ASSERT_TRUE(prepareStatus.IsSucess());

    bset<DgnElementId> elementInstances;
    ECInstanceECSqlSelectAdapter adapter(statement);
    while (statement.Step() == BE_SQLITE_ROW)
        {
        ECInstanceId id;
        bool status = adapter.GetInstanceId(id);
        ASSERT_TRUE(status);
        ElementRefP ref = project.Models().GetElementById(DgnElementId(id.GetValue())).get();
        ASSERT_TRUE(ref != NULL);
        elementInstances.insert(DgnElementId(id.GetValue()));
        }

    WString elementInstancesStr;
    FOR_EACH(DgnElementId id, elementInstances) { elementInstancesStr.append(WPrintfString(L"%lld ", id.GetValue())); }

    ASSERT_STREQ(elementsStr.c_str(), elementInstancesStr.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/07
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnModelP DgnModels::getAndFill(DgnDbR db, DgnModelId modelID)
    {
    DgnModelP dgnModel = db.Models().GetModel(modelID);
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

    DgnDbTestDgnManager tdm(L"3dMetricGeneral.ibim");
    DgnDbR project = *tdm.GetDgnProjectP();

    bvector<DgnElementId> elements;

    project.Models().FillDictionaryModel();
    FOR_EACH(PersistentElementRefP ref, project.Models().GetDictionaryModel()->GetElementsCollection())
        elements.push_back(ref->GetElementId());

    for (auto const& entry : project.Models().MakeIterator())
        {
        DgnModelP model = getAndFill(project, entry.GetModelId());

        FOR_EACH(PersistentElementRefP ref, model->GetElementsCollection())
            elements.push_back(ref->GetElementId());
        }

    ECN::ECSchemaP dgnschema = NULL;
    auto schemaStat = project.Schemas().GetECSchema(dgnschema, "dgn");
    ASSERT_EQ(SUCCESS, schemaStat);
    ECN::ECClassCP elementClass = dgnschema->GetClassCP(L"Element");
    ASSERT_TRUE(elementClass != NULL);

    //WHERE ECInstanceId = ? query
    {
    Utf8String ecsql("SELECT ECInstanceId FROM ONLY ");
    ecsql.append(elementClass->GetECSqlName()).append(" WHERE ECInstanceId=?");
    ECSqlStatement statement;
    auto stat = statement.Prepare(project, ecsql..c_str());
    ASSERT_EQ(ECSqlStatus::Success()().Get(), stat);

    FOR_EACH(DgnElementId expectedElementId, elements)
        {
        statement.Reset();
        statement.ClearBindings();
        stat = statement.BindInt64(1, expectedElementId.GetValue());
        ASSERT_EQ(ECSqlStatus::Success()().Get(), stat);

        auto stepStat = statement.Step();
        ASSERT_EQ((int) BE_SQLITE_ROW, (int) stepStat);
        ECInstanceId actualElementId = statement.GetValueId<ECInstanceId>(0);

        EXPECT_EQ(expectedElementId.GetValue(), actualElementId.GetValue());

        EXPECT_EQ((int) BE_SQLITE_DONE, (int) statement.Step()) << "Query by a single ECInstanceId is expected to only return one row.";
        }
    }

    //now do a WHERE ECInstanceId IN (...) query
    {
    size_t totalElementIdCount = elements.size();
    size_t testElementIdCount = std::min<size_t>(totalElementIdCount, 10);

    Utf8String ecsql("SELECT ECInstanceId FROM ONLY ");
    ecsql.append(elementClass->GetECSqlName()).append(" WHERE ECInstanceId IN (");
    bset<DgnElementId> expectedElementIds;
    for (size_t i = 0; i < testElementIdCount; i++)
        {
        expectedElementIds.insert(elements[i]);

        if (i > 0)
            ecsql.append(", ");

        Utf8String ecInstanceIdStr;
        ecInstanceIdStr.Sprintf("%lld", elements[i].GetValue());
        ecsql.append(ecInstanceIdStr.c_str());
        }
    ecsql.append(")");

    ECSqlStatement statement;
    auto stat = statement.Prepare(project, ecsql.c_str());
    ASSERT_EQ(ECSqlStatus::Success(), stat);

    bset<DgnElementId> actualElementIds;
    while (BE_SQLITE_ROW == statement.Step())
        {
        ECInstanceId actualElementId = statement.GetValueId<ECInstanceId>(0);
        actualElementIds.insert(DgnElementId(actualElementId.GetValue()));
        }

    //now check that results are as expected
    WString expectedElementIdsStr;
    FOR_EACH(DgnElementId id, expectedElementIds) { expectedElementIdsStr.append(WPrintfString(L"%lld ", id.GetValue())); }
    WString actualElementIdsStr;
    FOR_EACH(DgnElementId id, actualElementIds) { actualElementIdsStr.append(WPrintfString(L"%lld ", id.GetValue())); }

    ASSERT_STREQ(expectedElementIdsStr.c_str(), actualElementIdsStr.c_str());
    }
    }
#endif

#if defined (NEEDS_WORK_DGNITEM)
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   03/2013
//+---------------+---------------+---------------+---------------+---------------+------
DgnElementId AddLineToModel(DgnModelR model)
    {
    EditElementHandle line;
    DSegment3d  segment;

    segment.point[0] = DPoint3d::FromXYZ(0.0, 0.0, 0.0);
    segment.point[1] = DPoint3d::FromXYZ(100.0, 100.0, 100.0);
    GraphicElementHandler::InitializeElement(line, model); // WIP: need to pass in a DgnElementId
    line.AddToModel();

    return line.GetElementId();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DebugDumpJson(const Json::Value& jsonValue)
    {
    Utf8String strValue = Json::StyledWriter().write(jsonValue);
    int len = (int) strValue.size();
    for (int ii = 0; ii < len; ii += 1000)
        {
        // Split the string up - logging can't seem to handle long strings
        Utf8String subStr = strValue.substr(ii, 1000);
        LOG.debugv("%s", subStr.c_str());
        }
    }

static Byte s_Utf8BOM[] = {0xef, 0xbb, 0xbf};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ReadStringFromUtf8File(Utf8String& strValue, WCharCP path)
    {
    strValue = "";

    BeFile file;
    BeFileStatus fileStatus = file.Open(path, BeFileAccess::Read);
    if (!EXPECTED_CONDITION(BeFileStatus::Success == fileStatus))
        return false;

    uint64_t rawSize;
    fileStatus = file.GetSize(rawSize);
    if (!EXPECTED_CONDITION(BeFileStatus::Success == fileStatus && rawSize <= UINT32_MAX))
        {
        file.Close();
        return false;
        }
    uint32_t sizeToRead = (uint32_t) rawSize;

    uint32_t sizeRead;
    ScopedArray<Byte> scopedBuffer(sizeToRead);
    Byte* buffer = scopedBuffer.GetData();
    fileStatus = file.Read(buffer, &sizeRead, sizeToRead);
    if (!EXPECTED_CONDITION(BeFileStatus::Success == fileStatus && sizeRead == sizeToRead))
        {
        file.Close();
        return false;
        }

    // Validate it's a UTF8 file
    if (!EXPECTED_CONDITION(buffer[0] == s_Utf8BOM[0] && buffer[1] == s_Utf8BOM[1] && buffer[2] == s_Utf8BOM[2]))
        {
        file.Close();
        return false;
        }

    for (uint32_t ii = 3; ii < sizeRead; ii++)
        {
        if (buffer[ii] == '\n' || buffer[ii] == '\r')
            continue;
        strValue.append(1, buffer[ii]);
        }

    file.Close();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ReadJsonFromFile(Json::Value& jsonValue, WCharCP path)
    {
    Utf8String strValue;
    if (!ReadStringFromUtf8File(strValue, path))
        return false;

    return Json::Reader::Parse(strValue, jsonValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool WriteStringToUtf8File(WCharCP path, Utf8StringCR strValue)
    {
#if defined (_WIN32)
    FILE* file = _wfsopen(path, L"w" CSS_UTF8, _SH_DENYWR);
#else
    FILE* file = fopen(Utf8String(path).c_str(), "w");
#endif
    if (file == NULL)
        {
        BeAssert(false);
        return false;
        }
    fwprintf(file, L"%ls", WString(strValue.c_str(), BentleyCharEncoding::Utf8).c_str());
    fclose(file);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool WriteJsonToFile(WCharCP path, const Json::Value& jsonValue)
    {
    Utf8String strValue = Json::StyledWriter().write(jsonValue);
    return WriteStringToUtf8File(path, strValue);
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, JsonValueFormatting)
    {
    ScopedDgnHost host;

    DgnDbTestDgnManager tdm(L"rxmrlw1f.ibim");
    DgnDbR project = *tdm.GetDgnProjectP();

    // Make sure that all models are loaded and filled, so that call to dgnFile->FindElementById below will work.
    project.Models().FillDictionaryModel();
    for (auto const& entry : project.Models().MakeIterator(ModelIterate::All))
        {
        project.Models().GetAndFillModelById(NULL, entry.GetModelId(), true);
        }

    // Get some (arbitrary) reference element
    int64_t selectedElementId = GetV9ElementId("rxmrlw1f.dgn", 542696, project);
    ASSERT_TRUE(selectedElementId > 0);
    DgnElementId shownElementId;
    if (!DgnECPersistence::TryGetAssemblyElementWithPrimaryInstance(shownElementId, DgnElementId(selectedElementId), project))
        shownElementId = DgnElementId(selectedElementId);

    // Get all relevant EC information for some arbitrary element
    PersistentElementRefPtr ref = project.Models().GetElementById(shownElementId);
    ASSERT_TRUE(ref.IsValid());
    ECN::ECClassId classId = (ECN::ECClassId) ref->GetECClassId();
    ECClassP ecClass = NULL;
    project.Schemas().GetECClass(ecClass, classId);
    ASSERT_TRUE(ecClass != NULL);
    ECInstanceId instanceId = ref->GetECInstanceId();

    // Construct a ECSqlStatement to retrieve the row in the Db
    ECSqlStatement statement;
    Utf8String schemaPrefix = Utf8String(ecClass->GetSchema().GetNamespacePrefix().c_str());
    Utf8String className = Utf8String(ecClass->GetName().c_str());
    SqlPrintfString ecSql("SELECT * FROM %ls.%ls WHERE ECInstanceId=%lld", schemaPrefix.c_str(), className.c_str(), instanceId.GetValue());
    ECSqlStatus prepareStatus = statement.Prepare(project, ecSql.GetUtf8CP());
    ASSERT_TRUE(prepareStatus.IsSuccess());
    DbResult stepStatus = statement.Step();
    ASSERT_TRUE(BE_SQLITE_ROW == stepStatus);

    // Retrieve formatted JSON
    DgnECPropertyFormatterPtr propertyFormatter = DgnECPropertyFormatter::Create(ref->GetDgnModelP());
    JsonECSqlSelectAdapter jsonAdapter(statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::FormattedStrings, propertyFormatter.get()));
    Json::Value actualJson;
    jsonAdapter.GetRowDisplayInfo(actualJson["DisplayInfo"]);
    bool status = jsonAdapter.GetRow(actualJson["Values"]);
    ASSERT_TRUE(status);

    // Read benchmark JSON
    Json::Value expectedJson;
    BeFileName expectedFile;
    BeTest::GetHost().GetDocumentsRoot(expectedFile);
    expectedFile.AppendToPath(L"DgnDb");
    expectedFile.AppendToPath(L"rxmrlw1f.json");
    status = ReadJsonFromFile(expectedJson, expectedFile.GetName());
    ASSERT_TRUE(status);

    // Ignore "$ECInstanceId" in comparision - it's too volatile. 
    ASSERT_TRUE(actualJson["Values"].size() == expectedJson["Values"].size());
    for (int ii = 0; ii < (int) actualJson["Values"].size(); ii++)
        {
        actualJson["Values"][ii]["$ECInstanceId"] = "*";
        expectedJson["Values"][ii]["$ECInstanceId"] = "*";
        }

    // Validate
    int compare = expectedJson.compare(actualJson);
    if (0 != compare)
        {
        // For convenient android debugging
        //LOG.debugv ("Expected Json:");
        //DebugDumpJson (expectedJson);
        //LOG.debugv ("Actual Json:");
        //DebugDumpJson (actualJson);

        BeFileName actualFile;
        BeTest::GetHost().GetOutputRoot(actualFile);
        actualFile.AppendToPath(L"rxmrlw1f-actual.json");
        WriteJsonToFile(actualFile.GetName(), actualJson);
        FAIL() << "Json retrieved from db \n\t" << actualFile.GetName() << "\ndoes not match expected \n\t" << expectedFile.GetName();
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr CreateEmptyProject(DgnDbPtr& project, WCharCP projectPathname)
    {
    if (BeFileName::DoesPathExist(projectPathname))
        {
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile(projectPathname);
        if (fileDeleteStatus != BeFileNameStatus::Success)
            return NULL;
        }

    CreateDgnDbParams params;
    return DgnDb::CreateDgnDb(NULL, BeFileName(projectPathname), params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr ReadECSchemaFromDisk(WCharCP schemaPathname)
    {
    if (!BeFileName::DoesPathExist(schemaPathname))
        return NULL;

    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    WString schemaPath = BeFileName::GetDirectoryName(schemaPathname);
    schemaContext->AddSchemaPath(schemaPath.c_str());

    ECSchemaPtr schema;
    ECSchema::ReadFromXmlFile(schema, schemaPathname, *schemaContext);
    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImportECSchema(ECSchemaR ecSchema, DgnDbR project)
    {
    ECSchemaCachePtr schemaList = ECSchemaCache::Create();
    schemaList->AddSchema(ecSchema);
    BentleyStatus importSchemaStatus = project.Schemas().ImportECSchemas(*schemaList);
    project.SaveChanges();
    return (SUCCESS == importSchemaStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr CreateStartupCompanyInstance(ECSchemaCR startupSchema)
    {
    ECClassCP anglesStructClass = startupSchema.GetClassCP("AnglesStruct");
    if (anglesStructClass == NULL)
        return NULL;
    ECClassCP fooClass = startupSchema.GetClassCP("Foo");
    if (fooClass == NULL)
        return NULL;

    ECValue doubleValue;
    doubleValue.SetDouble(12.345);
    ECValue intValue;
    intValue.SetInteger(67);
    ECValue anglesStructValue;
    IECInstancePtr anglesStruct = anglesStructClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    anglesStruct->SetValue("Alpha", doubleValue);
    anglesStruct->SetValue("Beta", doubleValue);
    anglesStructValue.SetStruct(anglesStruct.get());

    IECInstancePtr foo = fooClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    ECObjectsStatus status;
    status = foo->SetValue("intFoo", intValue);
    status = foo->SetValue("doubleFoo", doubleValue);
    status = foo->SetValue("anglesFoo.Alpha", doubleValue);
    status = foo->SetValue("anglesFoo.Beta", doubleValue);
    foo->AddArrayElements("arrayOfIntsFoo", 3);
    foo->AddArrayElements("arrayOfAnglesStructsFoo", 3);
    for (int ii = 0; ii < 3; ii++)
        {
        status = foo->SetValue("arrayOfIntsFoo", intValue, ii);
        status = foo->SetValue("arrayOfAnglesStructsFoo", anglesStructValue, ii);
        }

    return foo;
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
        StatusInt CreateArbitraryElement(EditElementHandleR editElementHandle, DgnModelR model);


        typedef void(*PopulatePrimitiveValueCallback)(ECN::ECValueR value, ECN::PrimitiveType primitiveType, ECN::ECPropertyCP ecproperty);
        static BentleyStatus SetECInstanceId(ECN::IECInstanceR instance, ECInstanceId const& instanceId);

        static void            PopulateStructValue(ECN::ECValueR value, ECN::ECClassCR structType, PopulatePrimitiveValueCallback callback);

        static ECN::ECObjectsStatus CopyStruct(ECN::IECInstanceR source, ECN::ECValuesCollectionCR collection, WCharCP baseAccessPath);
        static ECN::ECObjectsStatus CopyStruct(ECN::IECInstanceR target, ECN::IECInstanceCR structValue, WCharCP propertyName);

        static void PopulatePrimitiveValueWithRandomValues(ECN::ECValueR ecValue, ECN::PrimitiveType primitiveType, ECN::ECPropertyCP ecProperty);
        static ECN::IECInstancePtr  CreateArbitraryECInstance(ECN::ECClassCR ecClass, PopulatePrimitiveValueCallback callback = PopulatePrimitiveValueWithRandomValues, bool skipStructs = false, bool skipArrays = false);
        static ECN::IECInstancePtr  CreateECInstance(ECN::ECClassCR ecClass);
        static void                 PopulateECInstance(ECN::IECInstancePtr ecInstance, PopulatePrimitiveValueCallback callback = PopulatePrimitiveValueWithRandomValues, bool skipStructs = false, bool skipArrays = false);

    };

IECInstancePtr DgnECInstanceTests::CreateArbitraryECInstance(ECClassCR ecClass, PopulatePrimitiveValueCallback populatePrimitiveValueCallback, bool skipStructs, bool skipArrays)
    {
    IECInstancePtr instance = ecClass.GetDefaultStandaloneEnabler()->CreateInstance(0);
    PopulateECInstance(instance, populatePrimitiveValueCallback, skipStructs, skipArrays);
    return instance;
    }

IECInstancePtr DgnECInstanceTests::CreateECInstance
(
    ECClassCR ecClass
    )
    {
    StandaloneECEnablerP instanceEnabler = ecClass.GetDefaultStandaloneEnabler();
    POSTCONDITION(instanceEnabler != nullptr, nullptr);
    IECInstancePtr instance = instanceEnabler->CreateInstance();
    POSTCONDITION(instance != nullptr, nullptr);
    return instance;
    }

void DgnECInstanceTests::PopulateECInstance(ECN::IECInstancePtr ecInstance, PopulatePrimitiveValueCallback populatePrimitiveValueCallback, bool skipStructs, bool skipArrays)
    {
    ECValue value;
    for (ECPropertyCP ecProperty : ecInstance->GetClass().GetProperties(true))
        {
        if (!skipStructs && ecProperty->GetIsStruct())
            {
            PopulateStructValue(value, ecProperty->GetAsStructProperty()->GetType(), populatePrimitiveValueCallback);
            CopyStruct(*ecInstance, *value.GetStruct(), ecProperty->GetName().c_str());
            }
        else if (ecProperty->GetIsPrimitive())
            {
            populatePrimitiveValueCallback(value, ecProperty->GetAsPrimitiveProperty()->GetType(), ecProperty);
            ecInstance->SetValue(ecProperty->GetName().c_str(), value);
            }
        else if (!skipArrays && ecProperty->GetIsArray())
            {
            ArrayECPropertyCP arrayProperty = ecProperty->GetAsArrayProperty();
            if (arrayProperty->GetKind() == ARRAYKIND_Primitive && arrayProperty->GetPrimitiveElementType() == PRIMITIVETYPE_IGeometry)
                continue;

            uint32_t arrayCount = 5;
            if (arrayCount < arrayProperty->GetMinOccurs())
                arrayCount = arrayProperty->GetMinOccurs();
            else if (arrayCount > arrayProperty->GetMaxOccurs())
                arrayCount = arrayProperty->GetMaxOccurs();

            ecInstance->AddArrayElements(ecProperty->GetName().c_str(), arrayCount);
            if (arrayProperty->GetKind() == ARRAYKIND_Struct)
                {
                for (uint32_t i = 0; i < arrayCount; i++)
                    {
                    PopulateStructValue(value, *arrayProperty->GetStructElementType(), populatePrimitiveValueCallback);
                    ecInstance->SetValue(ecProperty->GetName().c_str(), value, i);
                    }
                }
            else if (arrayProperty->GetKind() == ARRAYKIND_Primitive)
                {
                for (uint32_t i = 0; i < arrayCount; i++)
                    {
                    populatePrimitiveValueCallback(value, arrayProperty->GetPrimitiveElementType(), ecProperty);
                    ecInstance->SetValue(ecProperty->GetName().c_str(), value, i);
                    }
                }
            }
        }
    }

ECObjectsStatus DgnECInstanceTests::CopyStruct(IECInstanceR source, ECValuesCollectionCR collection, WCharCP baseAccessPath)
    {
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    for (auto& propertyValue : collection)
        {
        auto pvAccessString = propertyValue.GetValueAccessor().GetPropertyName();
        auto accessString = baseAccessPath == nullptr ? pvAccessString : WString(baseAccessPath) + L"." + pvAccessString;

        if (propertyValue.HasChildValues())
            {
            status = CopyStruct(source, *propertyValue.GetChildValues(), accessString.c_str());
            if (status != ECOBJECTS_STATUS_Success)
                {
                return status;
                }
            continue;
            }

        auto& location = propertyValue.GetValueAccessor().DeepestLocationCR();

        //auto property = location.GetECProperty(); 
        //BeAssert(property != nullptr);
        if (location.GetArrayIndex() >= 0)
            {
            source.AddArrayElements(accessString.c_str(), 1);
            status = source.SetValue(accessString.c_str(), propertyValue.GetValue(), location.GetArrayIndex());
            }
        else
            status = source.SetValue(accessString.c_str(), propertyValue.GetValue());

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


void DgnECInstanceTests::PopulateStructValue(ECValueR value, ECClassCR structType, PopulatePrimitiveValueCallback populatePrimitiveValueCallback)
    {
    value.Clear();
    IECInstancePtr inst = CreateArbitraryECInstance(structType, populatePrimitiveValueCallback);
    value.SetStruct(inst.get());
    }

//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>11/2013</date>
//---------------+---------------+---------------+---------------+---------------+-----
void DgnECInstanceTests::PopulatePrimitiveValueWithRandomValues(ECValueR ecValue, PrimitiveType primitiveType, ECPropertyCP ecProperty)
    {
    ecValue.Clear();

    int randomNumber = rand();
    switch (primitiveType)
        {
            case PRIMITIVETYPE_String:
            {
            Utf8String text;
            text.Sprintf("Sample text with random number: %d", randomNumber);
            ecValue.SetUtf8CP(text.c_str(), true);
            }
            break;

            case PRIMITIVETYPE_Integer:
            {
            ecValue.SetInteger(randomNumber);
            }
            break;

            case PRIMITIVETYPE_Long:
            {
            const int32_t intMax = std::numeric_limits<int32_t>::max();
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
            DateTime utcTime = DateTime::GetCurrentTimeUtc();
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
            point2d.x = randomNumber * 1.0;
            point2d.y = randomNumber * 1.8;
            ecValue.SetPoint2D(point2d);
            break;
            }
            case PRIMITIVETYPE_Point3D:
            {
            DPoint3d point3d;
            point3d.x = randomNumber * 1.0;
            point3d.y = randomNumber * 1.8;
            point3d.z = randomNumber * 2.9;
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

StatusInt DgnECInstanceTests::CreateArbitraryElement(EditElementHandleR editElementHandle, DgnModelR model)
    {
    DSegment3d  segment;
    segment.point[0] = DPoint3d::FromXYZ(s_xCoord, s_yCoord, s_zCoord);
    segment.point[1] = DPoint3d::FromXYZ(s_xCoord + s_increment, s_yCoord + s_increment, s_zCoord + s_increment);

    s_xCoord += 2 * s_increment;
    s_yCoord += 2 * s_increment;
    s_zCoord += 2 * s_increment;

    return LineHandler::CreateLineElement(editElementHandle, NULL, segment, model.Is3d(), model);
    }

BentleyStatus DgnECInstanceTests::SetECInstanceId(ECN::IECInstanceR instance, ECInstanceId const& ecInstanceId)
    {
    if (!ecInstanceId.IsValid())
        return ERROR;

    Utf8Char instanceIdStr[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    ecInstanceId.ToString(instanceIdStr)
        const auto ecstat = instance.SetInstanceId(instanceIdStr);
    return ecstat == ECOBJECTS_STATUS_Success ? SUCCESS : ERROR;
    }

TEST_F(DgnECInstanceTests, InstancesAndRelationships)
    {
    ScopedDgnHost host;

    DgnDbTestDgnManager tdm(L"3dMetricGeneral.ibim");
    auto dgnFile = tdm.GetLoadedDgnPtr();
    auto& project = dgnFile->GetDgnProject();

    auto model = tdm.GetDgnModelP();

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
    schemaList->AddSchema(*testSchema);
    project.GetEC().Schemas().ImportECSchemas(*schemaList);

    bvector<IECInstancePtr> orphanedWidgets;
    bvector<IECInstancePtr> orphanedBars;
    bvector<IECInstancePtr> orphanedFoos;

    bvector<IECInstancePtr> lineWidgets;
    bvector<IECInstancePtr> lineBars;

    ECInstanceInserter widgetInserter(*tdm.GetDgnProjectP(), *widgetClass);
    ECInstanceInserter fooInserter(*tdm.GetDgnProjectP(), *fooClass);
    ECInstanceInserter barInserter(*tdm.GetDgnProjectP(), *barClass);

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
        status = CreateArbitraryElement(*eeh, *model);
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

        StatusInt stat2 = DgnECPersistence::SetPrimaryInstanceOnElement(*eeh, ecInstanceKey, project);
        ASSERT_EQ(SUCCESS, stat2);
        ASSERT_EQ(SUCCESS, eeh->AddToModel());
        }

    // add some bar instances to line elements
    for (int i = 0; i < 5; i++)
        {
        StatusInt status;
        EditElementHandle* eeh = new EditElementHandle();
        status = CreateArbitraryElement(*eeh, *model);
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

        StatusInt stat2 = DgnECPersistence::SetPrimaryInstanceOnElement(*eeh, ecInstanceKey, project);
        ASSERT_EQ(SUCCESS, stat2);
        ASSERT_EQ(SUCCESS, eeh->AddToModel());
        }

    auto widgetHasBarEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*widgetHasBar);
    auto widgetHasFooEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*widgetHasFoo);

    auto widgetHasFooInstance = widgetHasFooEnabler->CreateRelationshipInstance();

    widgetHasFooInstance->SetSource(orphanedWidgets[0].get());
    widgetHasFooInstance->SetTarget(orphanedFoos[0].get());

    ECInstanceInserter widgetHasFooInserter(project, *widgetHasFoo);
    ECInstanceKey relInstanceKey;
    widgetHasFooInserter.Insert(relInstanceKey, *widgetHasFooInstance);

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

TEST(ECDbInstances3, BGRJoinedTable)
    {
    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"ReviewVisualization\" nameSpacePrefix=\"rv\" version=\"01.00\" description=\"Defines sets of rules for visualizing models based on EC instance criteria\" displayLabel=\"Review Visualization\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "<ECSchemaReference name=\"Generic\" version=\"01.00\" prefix=\"generic\" />"
        "<ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.12\" prefix=\"bsca\" />"
        "<ECClass typeName=\"VisualizationRule\" description=\"Defines a named rule for finding and coloring a set of EC instances\" displayLabel=\"Visualization Rule\" isStruct=\"true\" isCustomAttributeClass=\"false\" isDomainClass=\"false\">"
        "<ECProperty propertyName=\"Name\" typeName=\"string\" description=\"Rule name (for example, &quot;Installed&quot; for an Equipment Status visualization rule set)\" />"
        "<ECProperty propertyName=\"ColorRed\" typeName=\"int\" description=\"Display color red component value (0-255)\" displayLabel=\"Red Color Component\" />"
        "<ECProperty propertyName=\"ColorGreen\" typeName=\"int\" description=\"Display color green component value (0-255)\" displayLabel=\"Green Color Component\" />"
        "<ECProperty propertyName=\"ColorBlue\" typeName=\"int\" description=\"Display color blue component value (0-255)\" displayLabel=\"Blue Color Component\" />"
        "<ECProperty propertyName=\"QueryCriteria\" typeName=\"string\" description=\"Matching criteria (a WHERE clause without the WHERE keyword)\" displayLabel=\"Query Criteria\" />"
        "</ECClass>"
        "<ECClass typeName=\"VisualizationRuleSet\" description=\"Defines a set of rules for color-coding a model based on EC instance criteria\" displayLabel=\"Visualization Rule Set\" isStruct=\"false\" isCustomAttributeClass=\"false\" isDomainClass=\"true\">"
        "<BaseClass>generic:PhysicalObject</BaseClass>"
        "<ECArrayProperty propertyName=\"Rules\" typeName=\"VisualizationRule\" description=\"Array of visualization rules\" isStruct=\"true\" minOccurs=\"0\" maxOccurs=\"unbounded\" />"
        "<ECArrayProperty propertyName=\"QueryClasses\" typeName=\"string\" description=\"Array of classes to query\" displayLabel=\"Query Classes\" minOccurs=\"0\" maxOccurs=\"unbounded\" />"
        "</ECClass>"
        "</ECSchema>";
    ScopedDgnHost host;

    BeFileName projectPath;
    BeTest::GetHost().GetOutputRoot(projectPath);
    projectPath.AppendToPath(L"bgr.ibim");

    DgnDbPtr dgnDb = CreateEmptyProject(dgnDb, projectPath);

    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater(dgnDb->GetSchemaLocater());
    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    schemaContext->RemoveSchemaLocater(dgnDb->GetSchemaLocater());

    dgnDb->Schemas().ImportECSchemas(schemaContext->GetCache());
    dgnDb->ClearECDbCache();
    dgnDb->SaveChanges();

    //BeFileName bgr(L"f:\\temp\\BGRSubset.i.ibim");
    //DgnDbPtr dgnDb = DgnDb::OpenDgnDb(NULL, bgr, DgnDb::OpenParams(Db::OpenMode::ReadWrite));

    ECSchemaCP review = dgnDb->Schemas().GetECSchema("ReviewVisualization");
    ECClassCP visualizationRuleSet = review->GetClassCP("VisualizationRuleSet");
    ECInstanceInserter inserter(*dgnDb, *visualizationRuleSet);
    }

