/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/JsonInserterTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct JsonInserterTests : public ECDbTestFixture
    {

    //---------------------------------------------------------------------------------------
    // @bsimethod                                      Muhammad Hassan                  02/16
    //+---------------+---------------+---------------+---------------+---------------+------
    bool WriteJsonToFile(WCharCP path, const Json::Value& jsonValue)
        {
        Utf8String strValue = Json::StyledWriter().write(jsonValue);
        FILE* file = fopen(Utf8String(path).c_str(), "w");

        if (file == NULL)
            {
            BeAssert(false);
            return false;
            }
        fwprintf(file, L"%ls", WString(strValue.c_str(), BentleyCharEncoding::Utf8).c_str());
        fclose(file);
        return true;
        }
    };

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonInserterTests, InsertJsonCppJSON)
    {
    ECDbR ecdb = SetupECDb("insertUsingJsonAPI.ecdb", BeFileName(L"JsonTests.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    // Read JSON input from file
    BeFileName jsonInputFile;
    BeTest::GetHost().GetDocumentsRoot(jsonInputFile);
    jsonInputFile.AppendToPath(L"ECDb");
    jsonInputFile.AppendToPath(L"JsonTestClass.json");

    Json::Value jsonInput;
    ECDbTestUtility::ReadJsonInputFromFile(jsonInput, jsonInputFile);

    ECClassCP documentClass = ecdb.Schemas().GetECClass("JsonTests", "Document");
    ASSERT_TRUE(documentClass != nullptr);
    JsonInserter inserter(ecdb, *documentClass);

    //----------------------------------------------------------------------------------- 
    // Insert using JsonCpp
    //-----------------------------------------------------------------------------------
    ECInstanceKey id;
    ASSERT_EQ(SUCCESS, inserter.Insert(id, jsonInput));
    ecdb.SaveChanges();

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT NULL FROM jt.Document WHERE ECInstanceId=? AND Name=?"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, id.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(2, "A-Model.pdf", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    statement.Finalize();

    /* Retrieve the previously imported instance as JSON */
    ECSqlStatus prepareStatus = statement.Prepare(ecdb, "SELECT * FROM ONLY jt.Document");
    ASSERT_TRUE(ECSqlStatus::Success == prepareStatus);
    DbResult stepStatus = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStatus);

    Json::Value afterImportJson;
    JsonECSqlSelectAdapter jsonAdapter(statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    bool status = jsonAdapter.GetRowInstance(afterImportJson, documentClass->GetId());
    ASSERT_TRUE(status);
    statement.Finalize();

    /* Validate */
    int compare = jsonInput.compare(afterImportJson);
    if (0 != compare)
        {
        BeFileName afterImportFile;
        BeTest::GetHost().GetOutputRoot(afterImportFile);
        afterImportFile.AppendToPath(L"JsonTestClass-AfterImport.json");
        WriteJsonToFile(afterImportFile.GetName(), afterImportJson);
        ASSERT_TRUE(false) << "Inserted and Retrieved Json doesn't match";
        }

    //verify Json Insertion using the other Overload
    ASSERT_EQ(SUCCESS, inserter.Insert(jsonInput));
    ecdb.SaveChanges();

    //Verify Inserted Instances.
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT COUNT(*) FROM jt.Document"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(2, statement.GetValueInt(0));
    statement.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonInserterTests, InsertRapidJson)
    {
    ECDbR ecdb = SetupECDb("InsertUsingRapidJson.ecdb", BeFileName(L"JsonTests.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    // Read JSON input from file
    BeFileName jsonInputFile;
    BeTest::GetHost().GetDocumentsRoot(jsonInputFile);
    jsonInputFile.AppendToPath(L"ECDb");
    jsonInputFile.AppendToPath(L"JsonTestClass.json");

    Json::Value jsonInput;
    ECDbTestUtility::ReadJsonInputFromFile(jsonInput, jsonInputFile);

    //to rapidjson
    rapidjson::Document rapidJsonInput;
    bool parseSuccessful = !rapidJsonInput.Parse<0>(Json::FastWriter().write(jsonInput).c_str()).HasParseError();
    ASSERT_TRUE(parseSuccessful);

    ECClassCP documentClass = ecdb.Schemas().GetECClass("JsonTests", "Document");
    ASSERT_TRUE(documentClass != nullptr);
    JsonInserter inserter(ecdb, *documentClass);

    //-----------------------------------------------------------------------------------
    // Insert using rapidjson
    //-----------------------------------------------------------------------------------
    ECInstanceKey id;
    ASSERT_EQ(SUCCESS, inserter.Insert(id, rapidJsonInput));
    ecdb.SaveChanges();

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT NULL FROM jt.Document WHERE ECInstanceId=? AND Name=?"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, id.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(2, "A-Model.pdf", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    statement.Finalize();

    /* Retrieve the previously imported instance as JSON */
    ECSqlStatus prepareStatus = statement.Prepare(ecdb, "SELECT * FROM ONLY jt.Document");
    ASSERT_TRUE(ECSqlStatus::Success == prepareStatus);
    DbResult stepStatus = statement.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStatus);

    Json::Value afterImportJson;
    JsonECSqlSelectAdapter jsonAdapter(statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    bool status = jsonAdapter.GetRowInstance(afterImportJson, documentClass->GetId());
    ASSERT_TRUE(status);
    statement.Finalize();

    /* Validate */
    int compare = jsonInput.compare(afterImportJson);
    if (0 != compare)
        {
        BeFileName afterImportFile;
        BeTest::GetHost().GetOutputRoot(afterImportFile);
        afterImportFile.AppendToPath(L"JsonTestClass-AfterImport.json");
        WriteJsonToFile(afterImportFile.GetName(), afterImportJson);
        ASSERT_TRUE(false) << "Inserted and Retrieved Json doesn't match";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonInserterTests, CreateRoot_ExistingRoot_ReturnsSameKey_ECDBTEST)
    {
    ECDbTestFixture::Initialize();
    ECDbR ecdb = SetupECDb("schemaupgradetest.ecdb", BeFileName(L"DSCacheSchema.01.03.ecschema.xml"));
    ecdb.SaveChanges();

    IECClassLocaterR classLocater = ecdb.GetClassLocater();
    ECClassCP rootClass = classLocater.LocateClass("DSCacheSchema", "Root");
    ASSERT_NE(nullptr, rootClass);

    // Names
    Utf8String rootName = "Foo";

    // Test quety for same instance
    Utf8String ecsql = "SELECT ECInstanceId FROM [DSC].[Root] WHERE [Name] = ? LIMIT 1 ";
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, ecsql.c_str()));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, rootName.c_str(), IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(BE_SQLITE_DONE, statement.Step());

    // Insert one instnace
    Json::Value rootInstance;
    rootInstance["Name"] = rootName;
    rootInstance["Persistance"] = 0;

    JsonInserter inserter(ecdb, *rootClass);
    ASSERT_EQ(SUCCESS, inserter.Insert(rootInstance));

    // Try again
    statement.Reset();
    statement.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, rootName.c_str(), IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
    EXPECT_EQ(1, statement.GetValueId <ECInstanceId>(0).GetValue());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonInserterTests, ECPrimitiveValueFromJson)
    {
    ECDbR ecdb = SetupECDb("ecprimitivevaluefromjson.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "        <ECProperty propertyName='s' typeName='string' />"
        "        <ECProperty propertyName='p2d' typeName='point2d' />"
        "        <ECProperty propertyName='p3d' typeName='point3d' />"
        "    </ECEntityClass>"
        "</ECSchema>", true), 0);
    ASSERT_TRUE(ecdb.IsDbOpen());

    rapidjson::Document rapidJsonVal;
    rapidJsonVal.SetObject();
    rapidJsonVal.AddMember("Price", 3.0003, rapidJsonVal.GetAllocator());
    rapidJsonVal.AddMember("s", "StringVal", rapidJsonVal.GetAllocator());

    //add point2d member
    rapidjson::Value point2dObjValue;
    point2dObjValue.SetObject();
    point2dObjValue.AddMember("x", 0, rapidJsonVal.GetAllocator());
    point2dObjValue.AddMember("y", 0, rapidJsonVal.GetAllocator());
    rapidJsonVal.AddMember("p2d", point2dObjValue, rapidJsonVal.GetAllocator());

    //add point3d member
    rapidjson::Value point3dObjValue;
    point3dObjValue.SetObject();
    point3dObjValue.AddMember("x", 0, rapidJsonVal.GetAllocator());
    point3dObjValue.AddMember("y", 0, rapidJsonVal.GetAllocator());
    point3dObjValue.AddMember("z", 0, rapidJsonVal.GetAllocator());
    rapidJsonVal.AddMember("p3d", point3dObjValue, rapidJsonVal.GetAllocator());

    ECClassCP parentClass = ecdb.Schemas().GetECClass("TestSchema", "Parent");
    ASSERT_TRUE(parentClass != nullptr);
    JsonInserter inserter(ecdb, *parentClass);

    ECInstanceKey key;
    ASSERT_EQ(SUCCESS, inserter.Insert(key, rapidJsonVal));
    ecdb.SaveChanges();

    ECSqlStatement stmt;
    ECSqlStatus prepareStatus = stmt.Prepare(ecdb, "SELECT * FROM ts.Parent WHERE p2d=? AND p3d=?");
    stmt.BindPoint2D(1, DPoint2d::From(0, 0));
    stmt.BindPoint3D(2, DPoint3d::From(0, 0, 0));
    DbResult stepStatus = stmt.Step();
    ASSERT_EQ(BE_SQLITE_ROW, stepStatus);
    }

#define JSONTABLE_NAME "testjson"

#define BOOLVALUE true
#define INTVALUE 1000
#define INT64VALUE INT64_C(123456791234)
#define DOUBLEVALUE 6.123123123123
#define STRINGVALUE "Hello, world!!"
#define POINTXVALUE 3.3314134314134
#define POINTYVALUE -133.3314134314134
#define POINTZVALUE 100.3314134314

#define rowCount 10
#define arraySize 3

//---------------------------------------------------------------------------------------
// @bsiclass                                      Muhammad Hassan                  08/16
//+---------------+---------------+---------------+---------------+---------------+------
struct PrimArrayJsonInserterTests : public ECDbTestFixture
    {
    private:
        DateTime m_testDate;
        DPoint2d m_testPoint2d;
        DPoint3d m_testPoint3d;
        IGeometryPtr m_testGeometry;
        static const size_t s_testBlobSize = 128;
        Byte m_testBlob[s_testBlobSize];

    protected:

        PrimArrayJsonInserterTests()
            : m_testDate(DateTime::GetCurrentTimeUtc()), m_testPoint2d(DPoint2d::From(POINTXVALUE, POINTYVALUE)),
            m_testPoint3d(DPoint3d::From(POINTXVALUE, POINTYVALUE, POINTZVALUE)),
            m_testGeometry(IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0))))
            {
            for (size_t i = 0; i < 128; i++)
                {
                m_testBlob[i] = static_cast<Byte> (i + 32);
                }
            }

        BentleyStatus SetupTest(Utf8CP fileName);

        BentleyStatus RunInsertJson(ECN::PrimitiveType arrayType);
        BentleyStatus RunSelectJson(ECN::PrimitiveType arrayType);

        static Utf8CP PrimitiveTypeToString(ECN::PrimitiveType);

        DateTime const& GetTestDate() const { return m_testDate; }
        DPoint2d const& GetTestPoint2d() const { return m_testPoint2d; }
        DPoint3d const& GetTestPoint3d() const { return m_testPoint3d; }
        IGeometryCR GetTestGeometry() const { return *m_testGeometry; }
        Byte const* GetTestBlob() const { return m_testBlob; }
        size_t GetTestBlobSize() const { return s_testBlobSize; }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  08/16
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PrimArrayJsonInserterTests::SetupTest(Utf8CP fileName)
    {
    ECDbR ecdb = SetupECDb(fileName);
    if (BE_SQLITE_OK != ecdb.ExecuteSql("CREATE TABLE testjson(Id INTEGER PRIMARY KEY, val TEXT);"))
        return ERROR;

    ecdb.SaveChanges();
    BeFileName testFilePath;
    testFilePath.AssignUtf8(ecdb.GetDbFileName());
    ecdb.CloseDb();

    return m_ecdb.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  08/16
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PrimArrayJsonInserterTests::RunInsertJson(PrimitiveType arrayType)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), "INSERT INTO " JSONTABLE_NAME "(val) VALUES(?)"))
        return ERROR;

    for (int i = 0; i < rowCount; i++)
        {
        rapidjson::Document json;
        json.SetArray();
        for (uint32_t j = 0; j < arraySize; j++)
            {
            rapidjson::Value arrayElementJson;
            switch (arrayType)
                {
                    case PRIMITIVETYPE_Binary:
                    {
                    if (ECRapidJsonUtilities::BinaryToJson(arrayElementJson, GetTestBlob(), GetTestBlobSize(), json.GetAllocator()))
                        return ERROR;
                    break;
                    }

                    case PRIMITIVETYPE_Boolean:
                    {
                    arrayElementJson.SetBool(BOOLVALUE);
                    break;
                    }

                    case PRIMITIVETYPE_Double:
                    {
                    arrayElementJson.SetDouble(DOUBLEVALUE);
                    break;
                    }

                    case PRIMITIVETYPE_IGeometry:
                    {
                    bvector<Byte> fb;
                    BackDoor::IGeometryFlatBuffer::GeometryToBytes(fb, GetTestGeometry());

                    if (ECRapidJsonUtilities::BinaryToJson(arrayElementJson, fb.data(), fb.size(), json.GetAllocator()))
                        return ERROR;
                    break;
                    }

                    case PRIMITIVETYPE_Integer:
                    {
                    arrayElementJson.SetInt(INTVALUE);
                    break;
                    }

                    case PRIMITIVETYPE_Long:
                    {
                    ECRapidJsonUtilities::Int64ToStringJsonValue(arrayElementJson, INT64VALUE, json.GetAllocator());
                    break;
                    }

                    case PRIMITIVETYPE_Point2D:
                    {
                    if (ECRapidJsonUtilities::Point2DToJson(arrayElementJson, GetTestPoint2d(), json.GetAllocator()))
                        return ERROR;
                    break;
                    }

                    case PRIMITIVETYPE_Point3D:
                    {
                    if (ECRapidJsonUtilities::Point3DToJson(arrayElementJson, GetTestPoint3d(), json.GetAllocator()))
                        return ERROR;
                    break;
                    }

                    case PRIMITIVETYPE_String:
                    {
                    arrayElementJson.SetString(STRINGVALUE);
                    break;
                    }

                    default:
                        return ERROR;
                }

            json.PushBack(arrayElementJson, json.GetAllocator());
            }

        rapidjson::StringBuffer stringBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
        json.Accept(writer);

        if (BE_SQLITE_OK != stmt.BindText(1, stringBuffer.GetString(), Statement::MakeCopy::No))
            return ERROR;

        if (BE_SQLITE_DONE != stmt.Step())
            return ERROR;

        stmt.Reset();
        stmt.ClearBindings();
        }

    stmt.Finalize();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  08/16
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PrimArrayJsonInserterTests::RunSelectJson(PrimitiveType arrayType)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), "SELECT val FROM " JSONTABLE_NAME))
        return ERROR;

    while (BE_SQLITE_ROW == stmt.Step())
        {
        rapidjson::Document arrayJson;
        if (arrayJson.Parse<0>(stmt.GetValueText(0)).HasParseError())
            return ERROR;

        for (auto it = arrayJson.Begin(); it != arrayJson.End(); ++it)
            {
            switch (arrayType)
                {
                    case PRIMITIVETYPE_Binary:
                    {
                    if (!it->IsString())
                        return ERROR;

                    bvector<Byte> blob;
                    if (SUCCESS != ECRapidJsonUtilities::JsonToBinary(blob, *it))
                        return ERROR;

                    if (blob.size() != GetTestBlobSize() ||
                        memcmp(GetTestBlob(), blob.data(), GetTestBlobSize()) != 0)
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_Boolean:
                    {
                    if (!it->IsBool())
                        return ERROR;

                    if (BOOLVALUE != it->GetBool())
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_Double:
                    {
                    if (!it->IsDouble())
                        return ERROR;

                    if (it->GetDouble() < 0)
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_IGeometry:
                    {
                    if (!it->IsString())
                        return ERROR;

                    bvector<Byte> fb;
                    if (SUCCESS != ECRapidJsonUtilities::JsonToBinary(fb, *it))
                        return ERROR;

                    IGeometryPtr actualGeom = BackDoor::IGeometryFlatBuffer::BytesToGeometry(fb);
                    if (actualGeom == nullptr || !actualGeom->IsSameStructureAndGeometry(GetTestGeometry()))
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_Integer:
                    {
                    if (!it->IsInt())
                        return ERROR;

                    if (INTVALUE != it->GetInt())
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_Long:
                    {
                    if (!it->IsString())
                        return ERROR;

                    const int64_t val = ECRapidJsonUtilities::Int64FromJson(*it);
                    if (INT64VALUE != val)
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_Point2D:
                    {
                    DPoint2d pt;
                    if (SUCCESS != ECRapidJsonUtilities::JsonToPoint2D(pt, *it))
                        return ERROR;

                    if (!pt.AlmostEqual(GetTestPoint2d()))
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_Point3D:
                    {
                    DPoint3d pt;
                    if (SUCCESS != ECRapidJsonUtilities::JsonToPoint3D(pt, *it))
                        return ERROR;

                    if (!pt.AlmostEqual(GetTestPoint3d()))
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_String:
                    {
                    if (!it->IsString())
                        return ERROR;

                    if (strcmp(it->GetString(), STRINGVALUE) != 0)
                        return ERROR;

                    break;
                    }

                    default:
                        return ERROR;
                }
            }
        }

    stmt.Finalize();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  08/16
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP PrimArrayJsonInserterTests::PrimitiveTypeToString(ECN::PrimitiveType primType)
    {
    switch (primType)
        {
            case PRIMITIVETYPE_Binary: return "Binary";
            case PRIMITIVETYPE_Boolean: return "Boolean";
            case PRIMITIVETYPE_Double: return "Double";
            case PRIMITIVETYPE_IGeometry: return "IGeometry";
            case PRIMITIVETYPE_Integer: return "Integer";
            case PRIMITIVETYPE_Long: return "Long";
            case PRIMITIVETYPE_Point2D: return "Point2D";
            case PRIMITIVETYPE_Point3D: return "Point3D";
            case PRIMITIVETYPE_String: return "String";
            default:
                BeAssert(false);
                return nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  08/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PrimArrayJsonInserterTests, InsertJsonArray_Integer)
    {
    ASSERT_EQ(SUCCESS, SetupTest("primitivearrayjsoninsertertests.ecdb"));

    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_Integer));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_Integer));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  08/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PrimArrayJsonInserterTests, InsertJsonArray_Double)
    {
    ASSERT_EQ(SUCCESS, SetupTest("primitivearrayjsoninsertertests.ecdb"));

    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_Double));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_Double));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  08/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PrimArrayJsonInserterTests, InsertJsonArray_Long)
    {
    ASSERT_EQ(SUCCESS, SetupTest("primitivearrayjsoninsertertests.ecdb"));

    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_Long));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_Long));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  08/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PrimArrayJsonInserterTests, InsertJsonArray_string)
    {
    ASSERT_EQ(SUCCESS, SetupTest("primitivearrayjsoninsertertests.ecdb"));

    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_String));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  08/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PrimArrayJsonInserterTests, InsertJsonArray_Point2D)
    {
    ASSERT_EQ(SUCCESS, SetupTest("primitivearrayjsoninsertertests.ecdb"));

    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_Point2D));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_Point2D));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  08/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PrimArrayJsonInserterTests, InsertJsonArray_Point3D)
    {
    ASSERT_EQ(SUCCESS, SetupTest("primitivearrayjsoninsertertests.ecdb"));

    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_Point3D));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_Point3D));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  08/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PrimArrayJsonInserterTests, InsertJsonArray_Binary)
    {
    ASSERT_EQ(SUCCESS, SetupTest("primitivearrayjsoninsertertests.ecdb"));

    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_Binary));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_Binary));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  08/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PrimArrayJsonInserterTests, InsertJsonArray_IGeometry)
    {
    ASSERT_EQ(SUCCESS, SetupTest("primitivearrayjsoninsertertests.ecdb"));

    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_IGeometry));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_IGeometry));
    }

END_ECDBUNITTESTS_NAMESPACE