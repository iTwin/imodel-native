/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

#define ECDTABLE_NAME "testecd"
#define JSONTABLE_NAME "testjson"

#define BOOLVALUE true
#define INTVALUE 1000
#define INT64VALUE INT64_C(123456791234)
#define DOUBLEVALUE 6.123123123123
#define STRINGVALUE "Hello, world!!"
#define POINTXVALUE 3.3314134314134
#define POINTYVALUE -133.3314134314134
#define POINTZVALUE 100.3314134314

#define OPCOUNT 100000
#define SMALLARRAYLENGTH 10
#define LARGEARRAYLENGTH 100

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       07/2016
//=======================================================================================
struct PerformancePrimArrayJsonVsECDTests : ECDbTestFixture
    {
protected:
    enum class ColumnMode
        {
        Regular,
        Overflow
        };

private:
    DateTime m_testDate;
    DPoint2d m_testPoint2d;
    DPoint3d m_testPoint3d;
    IGeometryPtr m_testGeometry;
    static const size_t s_testBlobSize = 128;
    Byte m_testBlob[s_testBlobSize];
    
protected:

    PerformancePrimArrayJsonVsECDTests()
        : m_testDate(DateTime::GetCurrentTimeUtc()), m_testPoint2d(DPoint2d::From(POINTXVALUE, POINTYVALUE)), 
        m_testPoint3d(DPoint3d::From(POINTXVALUE, POINTYVALUE, POINTZVALUE)),
        m_testGeometry(IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0))))
        {
        for (size_t i = 0; i < 128; i++)
            {
            m_testBlob[i] = static_cast<Byte> (i + 32);
            }
        }

    BentleyStatus SetupTest(Utf8CP fileName, ECDb::OpenParams const&);

    BentleyStatus RunInsertECD(ECN::PrimitiveType arrayType, uint32_t arraySize, int rowCount, ColumnMode);
    BentleyStatus RunInsertECD(StopWatch& timer, Utf8CP fileName, PrimitiveType arrayType, uint32_t arraySize, int rowCount, ColumnMode);
    
    BentleyStatus RunInsertJson(ECN::PrimitiveType arrayType, uint32_t arraySize, int rowCount, ColumnMode);
    BentleyStatus RunInsertJson(StopWatch&, Utf8CP fileName, ECN::PrimitiveType arrayType, uint32_t arraySize, int rowCount, ColumnMode);
    BentleyStatus RunSelectECD(ECN::PrimitiveType arrayType, uint32_t arraySize, int rowCount, ColumnMode);
    BentleyStatus RunSelectJson(ECN::PrimitiveType arrayType, uint32_t arraySize, int rowCount, ColumnMode);


    ECN::StandaloneECInstancePtr CreateECDArray(uint32_t& propIndex, ECN::PrimitiveType arrayType);
    BentleyStatus PopulateECDArray(IECInstanceR ecdArray, uint32_t propIndex, ECN::PrimitiveType arrayType, uint32_t arraySize);
    void LogTiming(StopWatch&, Utf8CP logMessageHeader, ECN::PrimitiveType, uint32_t arraySize, int rowCount);
    static Utf8CP PrimitiveTypeToString(ECN::PrimitiveType);

    DateTime const& GetTestDate() const { return m_testDate; }
    DPoint2d const& GetTestPoint2d() const { return m_testPoint2d; }
    DPoint3d const& GetTestPoint3d() const { return m_testPoint3d; }
    IGeometryCR GetTestGeometry() const { return *m_testGeometry; }
    Byte const* GetTestBlob() const { return m_testBlob; }
    size_t GetTestBlobSize() const { return s_testBlobSize; }

    static std::vector<ECN::PrimitiveType> GetTestPrimitiveTypes()
        {
        return {ECN::PrimitiveType::PRIMITIVETYPE_Integer,
                ECN::PrimitiveType::PRIMITIVETYPE_Long,
                ECN::PrimitiveType::PRIMITIVETYPE_Double,
                ECN::PrimitiveType::PRIMITIVETYPE_String,
                ECN::PrimitiveType::PRIMITIVETYPE_Point3d,
                ECN::PrimitiveType::PRIMITIVETYPE_Binary,
                ECN::PrimitiveType::PRIMITIVETYPE_IGeometry};
        }
    };


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformancePrimArrayJsonVsECDTests, InsertJson_SmallArray)
    {
    for (PrimitiveType testArrayType : GetTestPrimitiveTypes())
        {
        ASSERT_EQ(SUCCESS, RunInsertJson(testArrayType, SMALLARRAYLENGTH, OPCOUNT, ColumnMode::Regular));
        }

    for (PrimitiveType testArrayType : GetTestPrimitiveTypes())
        {
        ASSERT_EQ(SUCCESS, RunInsertJson(testArrayType, SMALLARRAYLENGTH, OPCOUNT, ColumnMode::Overflow));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformancePrimArrayJsonVsECDTests, InsertJson_LargeArray)
    {
    for (PrimitiveType testArrayType : GetTestPrimitiveTypes())
        {
        ASSERT_EQ(SUCCESS, RunInsertJson(testArrayType, LARGEARRAYLENGTH, OPCOUNT, ColumnMode::Regular));
        }

    for (PrimitiveType testArrayType : GetTestPrimitiveTypes())
        {
        ASSERT_EQ(SUCCESS, RunInsertJson(testArrayType, LARGEARRAYLENGTH, OPCOUNT, ColumnMode::Overflow));
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformancePrimArrayJsonVsECDTests, SelectJson_SmallArray)
    {
    for (PrimitiveType testArrayType : GetTestPrimitiveTypes())
        {
        ASSERT_EQ(SUCCESS, RunSelectJson(testArrayType, SMALLARRAYLENGTH, OPCOUNT, ColumnMode::Regular));
        }

    for (PrimitiveType testArrayType : GetTestPrimitiveTypes())
        {
        ASSERT_EQ(SUCCESS, RunSelectJson(testArrayType, SMALLARRAYLENGTH, OPCOUNT, ColumnMode::Overflow));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformancePrimArrayJsonVsECDTests, SelectJson_LargeArray)
    {
    for (PrimitiveType testArrayType : GetTestPrimitiveTypes())
        {
        ASSERT_EQ(SUCCESS, RunSelectJson(testArrayType, LARGEARRAYLENGTH, OPCOUNT, ColumnMode::Regular));
        }

    for (PrimitiveType testArrayType : GetTestPrimitiveTypes())
        {
        ASSERT_EQ(SUCCESS, RunSelectJson(testArrayType, LARGEARRAYLENGTH, OPCOUNT, ColumnMode::Overflow));
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformancePrimArrayJsonVsECDTests, InsertECD_SmallArray)
    {
    for (PrimitiveType testArrayType : GetTestPrimitiveTypes())
        {
        ASSERT_EQ(SUCCESS, RunInsertECD(testArrayType, SMALLARRAYLENGTH, OPCOUNT, ColumnMode::Regular));
        }

    for (PrimitiveType testArrayType : GetTestPrimitiveTypes())
        {
        ASSERT_EQ(SUCCESS, RunInsertECD(testArrayType, SMALLARRAYLENGTH, OPCOUNT, ColumnMode::Overflow));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformancePrimArrayJsonVsECDTests, InsertECD_LargeArray)
    {
    for (PrimitiveType testArrayType : GetTestPrimitiveTypes())
        {
        ASSERT_EQ(SUCCESS, RunInsertECD(testArrayType, LARGEARRAYLENGTH, OPCOUNT, ColumnMode::Regular));
        }

    for (PrimitiveType testArrayType : GetTestPrimitiveTypes())
        {
        ASSERT_EQ(SUCCESS, RunInsertECD(testArrayType, LARGEARRAYLENGTH, OPCOUNT, ColumnMode::Overflow));
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformancePrimArrayJsonVsECDTests, SelectECD_SmallArray)
    {
    for (PrimitiveType testArrayType : GetTestPrimitiveTypes())
        {
        ASSERT_EQ(SUCCESS, RunSelectECD(testArrayType, SMALLARRAYLENGTH, OPCOUNT, ColumnMode::Regular));
        }

    for (PrimitiveType testArrayType : GetTestPrimitiveTypes())
        {
        ASSERT_EQ(SUCCESS, RunSelectECD(testArrayType, SMALLARRAYLENGTH, OPCOUNT, ColumnMode::Overflow));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformancePrimArrayJsonVsECDTests, SelectECD_LargeArray)
    {
    for (PrimitiveType testArrayType : GetTestPrimitiveTypes())
        {
        ASSERT_EQ(SUCCESS, RunSelectECD(testArrayType, LARGEARRAYLENGTH, OPCOUNT, ColumnMode::Regular));
        }

    for (PrimitiveType testArrayType : GetTestPrimitiveTypes())
        {
        ASSERT_EQ(SUCCESS, RunSelectECD(testArrayType, LARGEARRAYLENGTH, OPCOUNT, ColumnMode::Overflow));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformancePrimArrayJsonVsECDTests::RunInsertJson(PrimitiveType arrayType, uint32_t arraySize, int rowCount, ColumnMode mode)
    {
    Utf8String fileName;
    fileName.Sprintf("json_insert_%s_%s_array_%" PRIu32 "_opcount_%d.ecdb", 
                     mode == ColumnMode::Regular ? "regular" : "overflow",
                     PrimitiveTypeToString(arrayType), arraySize, rowCount);

    StopWatch timer;
    if (SUCCESS != RunInsertJson(timer, fileName.c_str(), arrayType, arraySize, rowCount, mode))
        return ERROR;

    LogTiming(timer, mode == ColumnMode::Regular ? "INSERT,JSON,regular" : "INSERT,JSON,overflow", 
              arrayType, arraySize, rowCount);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformancePrimArrayJsonVsECDTests::RunInsertJson(StopWatch& timer, Utf8CP fileName, PrimitiveType arrayType, uint32_t arraySize, int rowCount, ColumnMode mode)
    {
    if (SUCCESS != SetupTest(fileName, ECDb::OpenParams(Db::OpenMode::ReadWrite)))
        return ERROR;

    timer.Start();

    Utf8CP insertSql = nullptr;
    switch (mode)
        {
            case ColumnMode::Regular:
                insertSql = "INSERT INTO " JSONTABLE_NAME "(val) VALUES(?)";
                break;

            case ColumnMode::Overflow:
                insertSql = "INSERT INTO " JSONTABLE_NAME "(val) VALUES(json_object('primarray', ?))";
                break;

            default:
                BeAssert(false);
                return ERROR;
        }
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, insertSql))
        return ERROR;

    rapidjson::Document json;
    json.SetArray();

    for (int i = 0; i < rowCount; i++)
        {
        for (uint32_t j = 0; j < arraySize; j++)
            {
            rapidjson::Value arrayElementJson;
            switch (arrayType)
                {
                    case PRIMITIVETYPE_Binary:
                    {
                    if (ECJsonUtilities::BinaryToJson(arrayElementJson, GetTestBlob(), GetTestBlobSize(), json.GetAllocator()))
                        return ERROR;
                    break;
                    }

                    case PRIMITIVETYPE_Boolean:
                    {
                    arrayElementJson.SetBool(BOOLVALUE);
                    break;
                    }

                    case PRIMITIVETYPE_DateTime:
                    {
                    double jd = -1.0;
                    if (SUCCESS != GetTestDate().ToJulianDay(jd))
                        return ERROR;

                    arrayElementJson.SetDouble(jd);
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
                    BackDoor::BentleyGeometryFlatBuffer::GeometryToBytes(fb, GetTestGeometry());

                    if (ECJsonUtilities::BinaryToJson(arrayElementJson, fb.data(), fb.size(), json.GetAllocator()))
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
                    arrayElementJson.SetInt64(INT64VALUE);
                    break;
                    }
                    case PRIMITIVETYPE_Point2d:
                    {
                    if (ECJsonUtilities::Point2dToJson(arrayElementJson, GetTestPoint2d(), json.GetAllocator()))
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_Point3d:
                    {
                    if (ECJsonUtilities::Point3dToJson(arrayElementJson, GetTestPoint3d(), json.GetAllocator()))
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
        json.Clear();
        }

    stmt.Finalize();
    timer.Stop();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformancePrimArrayJsonVsECDTests::RunSelectJson(PrimitiveType arrayType, uint32_t arraySize, int rowCount, ColumnMode mode)
    {
    Utf8String fileName;
    fileName.Sprintf("json_select_%s_%s_array_%" PRIu32 "_opcount_%d.ecdb", 
                     mode == ColumnMode::Regular ? "regular" : "overflow",
                     PrimitiveTypeToString(arrayType), arraySize, rowCount);

    StopWatch timer;
    if (SUCCESS != RunInsertJson(timer, fileName.c_str(), arrayType, arraySize, rowCount, mode))
        return ERROR;

    Utf8String filePath = m_ecdb.GetDbFileName();
    m_ecdb.CloseDb();

    if (BE_SQLITE_OK != m_ecdb.OpenBeSQLiteDb(filePath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly)))
        return ERROR;

    timer.Start();
    Utf8CP selSql = nullptr;
    switch (mode)
        {
            case ColumnMode::Regular:
                selSql = "SELECT val FROM " JSONTABLE_NAME;
                break;

            case ColumnMode::Overflow:
                selSql = "SELECT json_extract(val,'$.primarray') FROM " JSONTABLE_NAME;
                break;

            default:
                BeAssert(false);
                return ERROR;
        }

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, selSql))
        return ERROR;

    rapidjson::Document arrayJson;

    while (BE_SQLITE_ROW == stmt.Step())
        {
        if (arrayJson.Parse<0>(stmt.GetValueText(0)).HasParseError())
            return ERROR;

        ByteStream blobBuffer;
        for (auto it = arrayJson.Begin(); it != arrayJson.End(); ++it)
            {
            switch (arrayType)
                {
                    case PRIMITIVETYPE_Binary:
                    {
                    if (!it->IsString())
                        return ERROR;

                    blobBuffer.Resize(0);
                    if (SUCCESS != ECJsonUtilities::JsonToBinary(blobBuffer, *it))
                        return ERROR;

                    if (blobBuffer.size() != GetTestBlobSize() ||
                        memcmp(GetTestBlob(), blobBuffer.data(), GetTestBlobSize()) != 0)
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
                    case PRIMITIVETYPE_DateTime:
                    {
                    if (!it->IsDouble())
                        return ERROR;

                    double jd = it->GetDouble();
                    DateTime dt;
                    if (SUCCESS != DateTime::FromJulianDay(dt, jd, DateTime::Info::CreateForDateTime(DateTime::Kind::Utc)))
                        return ERROR;

                    if (!dt.Equals(GetTestDate()))
                        return ERROR;
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

                    
                    blobBuffer.Resize(0);
                    if (SUCCESS != ECJsonUtilities::JsonToBinary(blobBuffer, *it))
                        return ERROR;

                    IGeometryPtr actualGeom = BackDoor::BentleyGeometryFlatBuffer::BytesToGeometry((Byte const*) blobBuffer.data());
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
                    if (!it->IsInt64())
                        return ERROR;

                    if (INT64VALUE != it->GetInt64())
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_Point2d:
                    {
                    DPoint2d pt;
                    if (SUCCESS != ECJsonUtilities::JsonToPoint2d(pt, *it))
                        return ERROR;

                    if (!pt.AlmostEqual(GetTestPoint2d()))
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_Point3d:
                    {
                    DPoint3d pt;
                    if (SUCCESS != ECJsonUtilities::JsonToPoint3d(pt, *it))
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

        arrayJson.Clear();
        }

    stmt.Finalize();
    timer.Stop();
    LogTiming(timer, mode == ColumnMode::Regular ? "SELECT,JSON,regular" : "SELECT,JSON,overflow", arrayType, arraySize, rowCount);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformancePrimArrayJsonVsECDTests::RunInsertECD(PrimitiveType arrayType, uint32_t arraySize, int rowCount, ColumnMode mode)
    {
    Utf8String fileName;
    fileName.Sprintf("ecd_insert_%s_%s_array_%" PRIu32 "_opcount_%d.ecdb", 
                     mode == ColumnMode::Regular ? "regular" : "overflow",
                     PrimitiveTypeToString(arrayType), arraySize, rowCount);

    StopWatch timer;
    if (SUCCESS != RunInsertECD(timer, fileName.c_str(), arrayType, arraySize, rowCount, mode))
        return ERROR;

    LogTiming(timer, mode == ColumnMode::Regular ? "INSERT,ECD,regular" : "INSERT,ECD,overflow", arrayType, arraySize, rowCount);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformancePrimArrayJsonVsECDTests::RunInsertECD(StopWatch& timer, Utf8CP fileName, PrimitiveType arrayType, uint32_t arraySize, int rowCount, ColumnMode mode)
    {
    if (SUCCESS != SetupTest(fileName, ECDb::OpenParams(Db::OpenMode::ReadWrite)))
        return ERROR;

    timer.Start();

    Utf8CP insertSql = nullptr;
    switch (mode)
        {
            case ColumnMode::Regular:
                insertSql = "INSERT INTO " ECDTABLE_NAME "(val) VALUES(?)";
                break;

            case ColumnMode::Overflow:
                insertSql = "INSERT INTO " JSONTABLE_NAME "(val) VALUES(json_object('primarray', BlobToBase64(?)))";
                break;

            default:
                BeAssert(false);
                return ERROR;
        }

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, insertSql))
        return ERROR;

    uint32_t propIndex = 0;
    StandaloneECInstancePtr arrayInstance = CreateECDArray(propIndex, arrayType);
    if (arrayInstance == nullptr)
        return ERROR;

    for (int i = 0; i < rowCount; i++)
        {
        if (SUCCESS != PopulateECDArray(*arrayInstance, propIndex, arrayType, arraySize))
            return ERROR;

        if (BE_SQLITE_OK != stmt.BindBlob(1, arrayInstance->GetData(), arrayInstance->GetBytesUsed(), Statement::MakeCopy::No))
            return ERROR;

        if (BE_SQLITE_DONE != stmt.Step())
            return ERROR;

        stmt.Reset();
        stmt.ClearBindings();
        arrayInstance->ClearValues();
        }

    stmt.Finalize();
    timer.Stop();
    return SUCCESS;
    }




//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformancePrimArrayJsonVsECDTests::RunSelectECD(PrimitiveType arrayType, uint32_t arraySize, int rowCount, ColumnMode mode)
    {
    Utf8String fileName;
    fileName.Sprintf("ecd_select_%s_%s_array_%" PRIu32 "_opcount_%d.ecdb", 
                     mode == ColumnMode::Regular ? "regular" : "overflow",
                     PrimitiveTypeToString(arrayType), arraySize, rowCount);

    StopWatch timer;
    if (SUCCESS != RunInsertECD(timer, fileName.c_str(), arrayType, arraySize, rowCount, mode))
        return ERROR;

    Utf8String filePath = m_ecdb.GetDbFileName();
    m_ecdb.CloseDb();

    if (BE_SQLITE_OK != m_ecdb.OpenBeSQLiteDb(filePath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly)))
        return ERROR;

    uint32_t propIndex = 0;
    StandaloneECInstancePtr arrayInstance = CreateECDArray(propIndex, arrayType);
    if (arrayInstance == nullptr)
        return ERROR;

    if (SUCCESS != PopulateECDArray(*arrayInstance, propIndex, arrayType, arraySize))
        return ERROR;

    Utf8CP selSql = nullptr;
    switch (mode)
        { 
            case ColumnMode::Regular:
                selSql = "SELECT val FROM " ECDTABLE_NAME;
                break;

            case ColumnMode::Overflow:
                selSql = "SELECT Base64ToBlob(json_extract(val,'$.primarray')) FROM " JSONTABLE_NAME;
                break;

            default:
                BeAssert(false);
                return ERROR;
        }

    timer.Start();
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, selSql))
        return ERROR;

    while (BE_SQLITE_ROW == stmt.Step())
        {
        Byte* arrayBlob = (Byte*) stmt.GetValueBlob(0);
        const int arrayBlobSize = stmt.GetColumnBytes(0);

        if (arrayBlob == nullptr)
            return ERROR;

        if (!ECDBuffer::IsCompatibleVersion(nullptr, arrayBlob))
            return ERROR;

        ECValue arrayVal;
        if (ECObjectsStatus::Success != arrayInstance->GetValue(arrayVal, propIndex))
            return ERROR;

        ArrayInfo const& arrayInfo = arrayVal.GetArrayInfo();
        const uint32_t actualArraySize = arrayInfo.GetCount();
        if (arraySize != actualArraySize)
            return ERROR;

        for (uint32_t i = 0; i < actualArraySize; i++)
            {
            ECValue val;
            if (ECObjectsStatus::Success != arrayInstance->GetValue(val, propIndex, i))
                return SUCCESS;

            switch (arrayType)
                {
                    case PRIMITIVETYPE_Binary:
                    {
                    if (arrayInfo.GetElementPrimitiveType() != PRIMITIVETYPE_Binary)
                        return ERROR;

                    size_t actualBlobSize = 0;
                    Byte const* actualBlob = val.GetBinary(actualBlobSize);
                    if (actualBlobSize != GetTestBlobSize() || memcmp(actualBlob, GetTestBlob(), GetTestBlobSize()) != 0)
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_Boolean:
                    {
                    if (arrayInfo.GetElementPrimitiveType() != PRIMITIVETYPE_Boolean)
                        return ERROR;

                    if (BOOLVALUE != val.GetBoolean())
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_DateTime:
                    {
                    if (arrayInfo.GetElementPrimitiveType() != PRIMITIVETYPE_DateTime)
                        return ERROR;

                    if (!val.GetDateTime().Equals(GetTestDate()))
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_Double:
                    {
                    if (arrayInfo.GetElementPrimitiveType() != PRIMITIVETYPE_Double)
                        return ERROR;

                    if (val.GetDouble() < 0)
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_IGeometry:
                    {
                    if (arrayInfo.GetElementPrimitiveType() != PRIMITIVETYPE_IGeometry)
                        return ERROR;

                    if (!GetTestGeometry().IsSameStructureAndGeometry(*val.GetIGeometry()))
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_Integer:
                    {
                    if (arrayInfo.GetElementPrimitiveType() != PRIMITIVETYPE_Integer)
                        return ERROR;

                    if (INTVALUE != val.GetInteger())
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_Long:
                    {
                    if (arrayInfo.GetElementPrimitiveType() != PRIMITIVETYPE_Long)
                        return ERROR;

                    if (INT64VALUE != val.GetLong())
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_Point2d:
                    {
                    if (arrayInfo.GetElementPrimitiveType() != PRIMITIVETYPE_Point2d)
                        return ERROR;

                    DPoint2d pt = val.GetPoint2d();
                    if (!pt.AlmostEqual(GetTestPoint2d()))
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_Point3d:
                    {
                    if (arrayInfo.GetElementPrimitiveType() != PRIMITIVETYPE_Point3d)
                        return ERROR;

                    DPoint3d pt = val.GetPoint3d();
                    if (!pt.AlmostEqual(GetTestPoint3d()))
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_String:
                    {
                    if (arrayInfo.GetElementPrimitiveType() != PRIMITIVETYPE_String)
                        return ERROR;

                    if (strcmp(val.GetUtf8CP(), STRINGVALUE) != 0)
                        return ERROR;
                    break;
                    }

                    default:
                        return ERROR;
                }
            }
        }

    stmt.Finalize();
    timer.Stop();
    LogTiming(timer, mode == ColumnMode::Regular ? "SELECT, ECD, regular" : "SELECT, ECD, overflow", arrayType, arraySize, rowCount);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       01/2017
//+---------------+---------------+---------------+---------------+---------------+------
StandaloneECInstancePtr PerformancePrimArrayJsonVsECDTests::CreateECDArray(uint32_t& propIndex, PrimitiveType arrayType)
    {
    ECSchemaPtr schema = nullptr;
    if (ECObjectsStatus::Success != ECSchema::CreateSchema(schema, "ECDSchema", "ts", 1, 0, 0))
        return nullptr;

    ECEntityClassP ecdClassP = nullptr;
    if (ECObjectsStatus::Success != schema->CreateEntityClass(ecdClassP, "PrimArrayClass"))
        return nullptr;

    PrimitiveArrayECPropertyP arrayProp = nullptr;
    if (ECObjectsStatus::Success != ecdClassP->CreatePrimitiveArrayProperty(arrayProp, "PrimArrayClass", arrayType))
        return nullptr;

    if (ECObjectsStatus::Success != ecdClassP->GetDefaultStandaloneEnabler()->GetPropertyIndex(propIndex, "PrimArrayClass"))
        return nullptr;

    return ecdClassP->GetDefaultStandaloneEnabler()->CreateInstance();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       01/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformancePrimArrayJsonVsECDTests::PopulateECDArray(IECInstanceR arrayInstance, uint32_t propIndex, ECN::PrimitiveType arrayType, uint32_t arraySize)
    {
    ECValue arrayElementVal;
    switch (arrayType)
        {
            case PRIMITIVETYPE_Binary:
                arrayElementVal.SetBinary(GetTestBlob(), GetTestBlobSize());
                break;

            case PRIMITIVETYPE_Boolean:
                arrayElementVal.SetBoolean(BOOLVALUE);
                break;

            case PRIMITIVETYPE_DateTime:
                arrayElementVal.SetDateTime(GetTestDate());
                break;

            case PRIMITIVETYPE_Double:
                arrayElementVal.SetDouble(DOUBLEVALUE);
                break;

            case PRIMITIVETYPE_IGeometry:
                arrayElementVal.SetIGeometry(GetTestGeometry());
                break;

            case PRIMITIVETYPE_Integer:
                arrayElementVal.SetInteger(INTVALUE);
                break;

            case PRIMITIVETYPE_Long:
                arrayElementVal.SetLong(INT64VALUE);
                break;

            case PRIMITIVETYPE_String:
                arrayElementVal.SetUtf8CP(STRINGVALUE);
                break;

            case PRIMITIVETYPE_Point2d:
                arrayElementVal.SetPoint2d(GetTestPoint2d());
                break;

            case PRIMITIVETYPE_Point3d:
                arrayElementVal.SetPoint3d(GetTestPoint3d());
                break;

            default:
                return ERROR;
        }

    for (uint32_t j = 0; j < arraySize; j++)
        {
        if (ECObjectsStatus::Success != arrayInstance.AddArrayElements(propIndex, 1))
            return ERROR;

        if (ECObjectsStatus::Success != arrayInstance.SetValue(propIndex, arrayElementVal, j))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformancePrimArrayJsonVsECDTests::SetupTest(Utf8CP fileName, ECDb::OpenParams const& params)
    {
    EXPECT_EQ(BE_SQLITE_OK, SetupECDb(fileName));
    if (BE_SQLITE_OK != m_ecdb.ExecuteSql("CREATE TABLE testecd(Id INTEGER PRIMARY KEY, val BLOB);CREATE TABLE testjson(Id INTEGER PRIMARY KEY, val TEXT);"))
        return ERROR;

    m_ecdb.SaveChanges();
    BeFileName testFilePath;
    testFilePath.AssignUtf8(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();

    return m_ecdb.OpenBeSQLiteDb(testFilePath, params) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
void PerformancePrimArrayJsonVsECDTests::LogTiming(StopWatch& timer, Utf8CP scenario, ECN::PrimitiveType primType, uint32_t arraySize, int rowCount)
    {
    Utf8String logMessage;
    logMessage.Sprintf("%s array, array size %" PRIu32 ",%s.", PrimitiveTypeToString(primType), arraySize, scenario);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), rowCount, logMessage.c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP PerformancePrimArrayJsonVsECDTests::PrimitiveTypeToString(ECN::PrimitiveType primType)
    {
    switch (primType)
        {
            case PRIMITIVETYPE_Binary: return "Binary";
            case PRIMITIVETYPE_Boolean: return "Boolean";
            case PRIMITIVETYPE_DateTime: return "DateTime";
            case PRIMITIVETYPE_Double: return "Double";
            case PRIMITIVETYPE_IGeometry: return "IGeometry";
            case PRIMITIVETYPE_Integer: return "Integer";
            case PRIMITIVETYPE_Long: return "Long";
            case PRIMITIVETYPE_Point2d: return "Point2d";
            case PRIMITIVETYPE_Point3d: return "Point3d";
            case PRIMITIVETYPE_String: return "String";
            default:
                BeAssert(false);
                return nullptr;
        }
    }

END_ECDBUNITTESTS_NAMESPACE

