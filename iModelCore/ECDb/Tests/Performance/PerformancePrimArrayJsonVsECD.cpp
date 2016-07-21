/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformancePrimArrayJsonVsECD.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"

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

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       07/2016
//=======================================================================================
struct PerformancePrimArrayJsonVsECDTests : ECDbTestFixture
    {
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

    BentleyStatus RunInsertECD(ECN::PrimitiveType arrayType, uint32_t arraySize, int rowCount);
    BentleyStatus RunInsertECD(StopWatch&, Utf8CP fileName, ECN::PrimitiveType arrayType, uint32_t arraySize, int rowCount);
    BentleyStatus RunInsertJson(ECN::PrimitiveType arrayType, uint32_t arraySize, int rowCount);
    BentleyStatus RunInsertJson(StopWatch&, Utf8CP fileName, ECN::PrimitiveType arrayType, uint32_t arraySize, int rowCount);
    BentleyStatus RunSelectECD(ECN::PrimitiveType arrayType, uint32_t arraySize, int rowCount);
    BentleyStatus RunSelectJson(ECN::PrimitiveType arrayType, uint32_t arraySize, int rowCount);

    void LogTiming(StopWatch&, Utf8CP logMessageHeader, ECN::PrimitiveType, uint32_t arraySize, int rowCount);
    static BentleyStatus CreateECDClass(ECSchemaPtr&, ECN::ECEntityClassCP&, uint32_t& propIndex, ECN::PrimitiveType);
    static Utf8CP PrimitiveTypeToString(ECN::PrimitiveType);

    DateTime const& GetTestDate() const { return m_testDate; }
    DPoint2d const& GetTestPoint2d() const { return m_testPoint2d; }
    DPoint3d const& GetTestPoint3d() const { return m_testPoint3d; }
    IGeometryCR GetTestGeometry() const { return *m_testGeometry; }
    Byte const* GetTestBlob() const { return m_testBlob; }
    size_t GetTestBlobSize() const { return s_testBlobSize; }
    };


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformancePrimArrayJsonVsECDTests, InsertJson_SmallArray)
    {
    const int rowCount = 100000;
    const uint32_t arraySize = 10;
    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_Integer, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_Long, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_Double, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_String, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_Point3D, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_Binary, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_IGeometry, arraySize, rowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformancePrimArrayJsonVsECDTests, SelectJson_SmallArray)
    {
    const int rowCount = 100000;
    const uint32_t arraySize = 10;
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_Integer, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_Long, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_Double, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_String, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_Point3D, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_Binary, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_IGeometry, arraySize, rowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformancePrimArrayJsonVsECDTests, InsertJson_LargeArray)
    {
    const int rowCount = 100000;
    const uint32_t arraySize = 10000;
    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_Integer, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_Long, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_Double, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_String, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_Point3D, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_Binary, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertJson(PrimitiveType::PRIMITIVETYPE_IGeometry, arraySize, rowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformancePrimArrayJsonVsECDTests, SelectJson_LargeArray)
    {
    const int rowCount = 100000;
    const uint32_t arraySize = 10000;
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_Integer, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_Long, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_Double, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_String, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_Point3D, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_Binary, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectJson(PrimitiveType::PRIMITIVETYPE_IGeometry, arraySize, rowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformancePrimArrayJsonVsECDTests, InsertECD_SmallArray)
    {
    const int rowCount = 100000;
    const uint32_t arraySize = 10;
    ASSERT_EQ(SUCCESS, RunInsertECD(PrimitiveType::PRIMITIVETYPE_Integer, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertECD(PrimitiveType::PRIMITIVETYPE_Long, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertECD(PrimitiveType::PRIMITIVETYPE_Double, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertECD(PrimitiveType::PRIMITIVETYPE_String, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertECD(PrimitiveType::PRIMITIVETYPE_Point3D, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertECD(PrimitiveType::PRIMITIVETYPE_Binary, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertECD(PrimitiveType::PRIMITIVETYPE_IGeometry, arraySize, rowCount));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformancePrimArrayJsonVsECDTests, SelectECD_SmallArray)
    {
    const int rowCount = 100000;
    const uint32_t arraySize = 10;
    ASSERT_EQ(SUCCESS, RunSelectECD(PrimitiveType::PRIMITIVETYPE_Integer, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectECD(PrimitiveType::PRIMITIVETYPE_Long, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectECD(PrimitiveType::PRIMITIVETYPE_Double, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectECD(PrimitiveType::PRIMITIVETYPE_String, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectECD(PrimitiveType::PRIMITIVETYPE_Point3D, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectECD(PrimitiveType::PRIMITIVETYPE_Binary, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectECD(PrimitiveType::PRIMITIVETYPE_IGeometry, arraySize, rowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformancePrimArrayJsonVsECDTests, InsertECD_LargeArray)
    {
    const int rowCount = 100000;
    const uint32_t arraySize = 10000;
    ASSERT_EQ(SUCCESS, RunInsertECD(PrimitiveType::PRIMITIVETYPE_Integer, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertECD(PrimitiveType::PRIMITIVETYPE_Long, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertECD(PrimitiveType::PRIMITIVETYPE_Double, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertECD(PrimitiveType::PRIMITIVETYPE_String, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertECD(PrimitiveType::PRIMITIVETYPE_Point3D, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertECD(PrimitiveType::PRIMITIVETYPE_Binary, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertECD(PrimitiveType::PRIMITIVETYPE_IGeometry, arraySize, rowCount));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformancePrimArrayJsonVsECDTests, SelectECD_LargeArray)
    {
    const int rowCount = 100000;
    const uint32_t arraySize = 10000;
    ASSERT_EQ(SUCCESS, RunSelectECD(PrimitiveType::PRIMITIVETYPE_Integer, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectECD(PrimitiveType::PRIMITIVETYPE_Long, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectECD(PrimitiveType::PRIMITIVETYPE_Double, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectECD(PrimitiveType::PRIMITIVETYPE_String, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectECD(PrimitiveType::PRIMITIVETYPE_Point3D, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectECD(PrimitiveType::PRIMITIVETYPE_Binary, arraySize, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectECD(PrimitiveType::PRIMITIVETYPE_IGeometry, arraySize, rowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformancePrimArrayJsonVsECDTests::RunInsertJson(PrimitiveType arrayType, uint32_t arraySize, int rowCount)
    {
    Utf8String fileName;
    fileName.Sprintf("json_insert_%s_array_%" PRIu32 "_opcount_%d.ecdb", PrimitiveTypeToString(arrayType), arraySize, rowCount);

    StopWatch timer;
    if (SUCCESS != RunInsertJson(timer, fileName.c_str(), arrayType, arraySize, rowCount))
        return ERROR;

    LogTiming(timer, "INSERT - JSON", arrayType, arraySize, rowCount);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformancePrimArrayJsonVsECDTests::RunInsertJson(StopWatch& timer, Utf8CP fileName, PrimitiveType arrayType, uint32_t arraySize, int rowCount)
    {
    if (SUCCESS != SetupTest(fileName, ECDb::OpenParams(Db::OpenMode::ReadWrite)))
        return ERROR;

    Json::Value arrayElementVal;
    switch (arrayType)
        {
            case PRIMITIVETYPE_Binary:
                if (SUCCESS != ECJsonUtilities::BinaryToJson(arrayElementVal, GetTestBlob(), GetTestBlobSize()))
                    return ERROR;

                break;
            case PRIMITIVETYPE_Boolean:
                arrayElementVal = Json::Value(BOOLVALUE);
                break;

            case PRIMITIVETYPE_DateTime:
            {
            double jd = -1.0;
            if (SUCCESS != GetTestDate().ToJulianDay(jd))
                return ERROR;

            arrayElementVal = Json::Value(jd);
            break;
            }
            case PRIMITIVETYPE_Double:
                arrayElementVal = Json::Value(DOUBLEVALUE);
                break;

            case PRIMITIVETYPE_IGeometry:
            {
            bvector<Byte> fb;
            BackDoor::IGeometryFlatBuffer::GeometryToBytes(fb, GetTestGeometry());
            if (SUCCESS != ECJsonUtilities::BinaryToJson(arrayElementVal, fb.data(), fb.size()))
                return ERROR;

            break;
            }

            case PRIMITIVETYPE_Integer:
                arrayElementVal = Json::Value(INTVALUE);
                break;

            case PRIMITIVETYPE_Long:
                arrayElementVal = BeJsonUtilities::StringValueFromInt64(INT64VALUE);
                break;

            case PRIMITIVETYPE_String:
                arrayElementVal = Json::Value(STRINGVALUE);
                break;

            case PRIMITIVETYPE_Point2D:
                if (SUCCESS != ECJsonUtilities::Point2DToJson(arrayElementVal, GetTestPoint2d()))
                    return ERROR;
                break;

            case PRIMITIVETYPE_Point3D:
                if (SUCCESS != ECJsonUtilities::Point3DToJson(arrayElementVal, GetTestPoint3d()))
                    return ERROR;
                break;

            default:
                return ERROR;
        }

    timer.Start();

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), "INSERT INTO " JSONTABLE_NAME "(val) VALUES(?)"))
        return ERROR;

    for (int i = 0; i < rowCount; i++)
        {
        Json::Value arrayVal(Json::arrayValue);
        for (uint32_t j = 0; j < arraySize; j++)
            {
            arrayVal.append(arrayElementVal);
            }

        Json::FastWriter writer;
        Utf8String jsonStr = writer.write(arrayVal);
        if (BE_SQLITE_OK != stmt.BindText(1, jsonStr.c_str(), Statement::MakeCopy::No))
            return ERROR;

        if (BE_SQLITE_DONE != stmt.Step())
            return ERROR;

        stmt.Reset();
        stmt.ClearBindings();
        }

    stmt.Finalize();
    timer.Stop();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformancePrimArrayJsonVsECDTests::RunSelectJson(PrimitiveType arrayType, uint32_t arraySize, int rowCount)
    {
    Utf8String fileName;
    fileName.Sprintf("json_select_%s_array_%" PRIu32 "_opcount_%d.ecdb", PrimitiveTypeToString(arrayType), arraySize, rowCount);

    StopWatch timer;
    if (SUCCESS != RunInsertJson(timer, fileName.c_str(), arrayType, arraySize, rowCount))
        return ERROR;

    Utf8String filePath = GetECDb().GetDbFileName();
    GetECDb().CloseDb();

    if (BE_SQLITE_OK != m_ecdb.OpenBeSQLiteDb(filePath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly)))
        return ERROR;

    timer.Start();
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), "SELECT val FROM " JSONTABLE_NAME))
        return ERROR;

    while (BE_SQLITE_ROW == stmt.Step())
        {
        Utf8CP jsonStr = stmt.GetValueText(0);
        Json::Value arrayJson;
        Json::Reader reader;
        if (!reader.Parse(jsonStr, arrayJson, false))
            return ERROR;

        for (JsonValueCR arrayElement : arrayJson)
            {
            switch (arrayType)
                {
                    case PRIMITIVETYPE_Binary:
                    {
                    bvector<Byte> blob;
                    if (SUCCESS != ECJsonUtilities::JsonToBinary(blob, arrayElement))
                        return ERROR;

                    if (blob.size() != GetTestBlobSize() ||
                        memcmp(GetTestBlob(), blob.data(), GetTestBlobSize()) != 0)
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_Boolean:
                    {
                    if (!arrayElement.isBool())
                        return ERROR;

                    if (BOOLVALUE != arrayElement.asBool())
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_DateTime:
                    {
                    if (!arrayElement.isDouble())
                        return ERROR;

                    double jd = arrayElement.asDouble();
                    DateTime dt;
                    if (SUCCESS != DateTime::FromJulianDay(dt, jd, DateTime::Info(DateTime::Kind::Utc)))
                        return ERROR;

                    if (!dt.Equals(GetTestDate()))
                        return ERROR;
                    }

                    case PRIMITIVETYPE_Double:
                    {
                    if (!arrayElement.isDouble())
                        return ERROR;

                    if (arrayElement.asDouble() < 0)
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_IGeometry:
                    {
                    bvector<Byte> fb;
                    if (SUCCESS != ECJsonUtilities::JsonToBinary(fb, arrayElement))
                        return ERROR;

                    IGeometryPtr actualGeom = BackDoor::IGeometryFlatBuffer::BytesToGeometry(fb);
                    if (actualGeom == nullptr || !actualGeom->IsSameStructureAndGeometry(GetTestGeometry()))
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_Integer:
                    {
                    if (!arrayElement.isInt())
                        return ERROR;

                    if (INTVALUE != arrayElement.asInt())
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_Long:
                    {
                    int64_t val = BeJsonUtilities::Int64FromValue(arrayElement, INT64_C(-1));
                    if (val < 0)
                        return ERROR;

                    if (INT64VALUE != val)
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_Point2D:
                    {
                    DPoint2d pt;
                    if (SUCCESS != ECJsonUtilities::JsonToPoint2D(pt, arrayElement))
                        return ERROR;

                    if (!pt.AlmostEqual(GetTestPoint2d()))
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_Point3D:
                    {
                    DPoint3d pt;
                    if (SUCCESS != ECJsonUtilities::JsonToPoint3D(pt, arrayElement))
                        return ERROR;

                    if (!pt.AlmostEqual(GetTestPoint3d()))
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_String:
                    {
                    if (!arrayElement.isString())
                        return ERROR;

                    if (strcmp(arrayElement.asCString(), STRINGVALUE) != 0)
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
    LogTiming(timer, "SELECT - JSON", arrayType, arraySize, rowCount);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformancePrimArrayJsonVsECDTests::RunInsertECD(PrimitiveType arrayType, uint32_t arraySize, int rowCount)
    {
    Utf8String fileName;
    fileName.Sprintf("ecd_insert_%s_array_%" PRIu32 "_opcount_%d.ecdb", PrimitiveTypeToString(arrayType), arraySize, rowCount);

    StopWatch timer;
    if (SUCCESS != RunInsertECD(timer, fileName.c_str(), arrayType, arraySize, rowCount))
        return ERROR;

    LogTiming(timer, "INSERT - ECD", arrayType, arraySize, rowCount);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformancePrimArrayJsonVsECDTests::RunInsertECD(StopWatch& timer, Utf8CP fileName, PrimitiveType arrayType, uint32_t arraySize, int rowCount)
    {
    if (SUCCESS != SetupTest(fileName, ECDb::OpenParams(Db::OpenMode::ReadWrite)))
        return ERROR;

    ECSchemaPtr ecdSchema = nullptr;
    ECEntityClassCP ecdClass = nullptr;
    uint32_t propIndex = 0;
    if (SUCCESS != CreateECDClass(ecdSchema, ecdClass, propIndex, arrayType))
        return ERROR;

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

            case PRIMITIVETYPE_Point2D:
                arrayElementVal.SetPoint2D(GetTestPoint2d());
                break;

            case PRIMITIVETYPE_Point3D:
                arrayElementVal.SetPoint3D(GetTestPoint3d());
                break;

            default:
                return ERROR;
        }

    timer.Start();

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), "INSERT INTO " ECDTABLE_NAME "(val) VALUES(?)"))
        return ERROR;

    for (int i = 0; i < rowCount; i++)
        {
        StandaloneECInstancePtr arrayInstance = ecdClass->GetDefaultStandaloneEnabler()->CreateInstance();
        if (arrayInstance == nullptr)
            return ERROR;

        for (uint32_t j = 0; j < arraySize; j++)
            {
            if (ECObjectsStatus::Success != arrayInstance->AddArrayElements(propIndex, 1))
                return ERROR;

            if (ECObjectsStatus::Success != arrayInstance->SetValue(propIndex, arrayElementVal, j))
                return ERROR;
            }

        if (BE_SQLITE_OK != stmt.BindBlob(1, arrayInstance->GetData(), arrayInstance->GetBytesUsed(), Statement::MakeCopy::No))
            return ERROR;

        if (BE_SQLITE_DONE != stmt.Step())
            return ERROR;

        stmt.Reset();
        stmt.ClearBindings();
        }

    stmt.Finalize();
    timer.Stop();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformancePrimArrayJsonVsECDTests::RunSelectECD(PrimitiveType arrayType, uint32_t arraySize, int rowCount)
    {
    Utf8String fileName;
    fileName.Sprintf("ecd_select_%s_array_%" PRIu32 "_opcount_%d.ecdb", PrimitiveTypeToString(arrayType), arraySize, rowCount);

    StopWatch timer;
    if (SUCCESS != RunInsertECD(timer, fileName.c_str(), arrayType, arraySize, rowCount))
        return ERROR;

    Utf8String filePath = GetECDb().GetDbFileName();
    GetECDb().CloseDb();

    if (BE_SQLITE_OK != m_ecdb.OpenBeSQLiteDb(filePath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly)))
        return ERROR;

    ECSchemaPtr ecdSchema = nullptr;
    ECEntityClassCP ecdClass = nullptr;
    uint32_t propIndex = 0;
    if (SUCCESS != CreateECDClass(ecdSchema, ecdClass, propIndex, arrayType))
        return ERROR;

    timer.Start();
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), "SELECT val FROM " ECDTABLE_NAME))
        return ERROR;

    while (BE_SQLITE_ROW == stmt.Step())
        {
        Byte* arrayBlob = (Byte*) stmt.GetValueBlob(0);
        const int arrayBlobSize = stmt.GetColumnBytes(0);

        if (arrayBlob == nullptr)
            return ERROR;

            if (!ECDBuffer::IsCompatibleVersion(nullptr, arrayBlob))
                return ERROR;

            StandaloneECInstancePtr arrayInstance = ecdClass->GetDefaultStandaloneEnabler()->CreateSharedInstance(arrayBlob, arrayBlobSize);
            if (arrayInstance == nullptr)
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
                    case PRIMITIVETYPE_Point2D:
                    {
                    if (arrayInfo.GetElementPrimitiveType() != PRIMITIVETYPE_Point2D)
                        return ERROR;

                    DPoint2d pt = val.GetPoint2D();
                    if (!pt.AlmostEqual(GetTestPoint2d()))
                        return ERROR;

                    break;
                    }
                    case PRIMITIVETYPE_Point3D:
                    {
                    if (arrayInfo.GetElementPrimitiveType() != PRIMITIVETYPE_Point3D)
                        return ERROR;

                    DPoint3d pt = val.GetPoint3D();
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
    LogTiming(timer, "SELECT - ECD", arrayType, arraySize, rowCount);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus PerformancePrimArrayJsonVsECDTests::CreateECDClass(ECSchemaPtr& schema, ECN::ECEntityClassCP& ecdClass, uint32_t& propIndex, ECN::PrimitiveType arrayType)
    {
    if (ECObjectsStatus::Success != ECSchema::CreateSchema(schema, "ECDSchema", 1, 0))
        return ERROR;

    ECEntityClassP ecdClassP = nullptr;
    if (ECObjectsStatus::Success != schema->CreateEntityClass(ecdClassP, "PrimArrayClass"))
        return ERROR;

    ArrayECPropertyP arrayProp = nullptr;
    if (ECObjectsStatus::Success != ecdClassP->CreateArrayProperty(arrayProp, "PrimArrayClass", arrayType))
        return ERROR;

    ecdClass = ecdClassP;
    if (ECObjectsStatus::Success != ecdClass->GetDefaultStandaloneEnabler()->GetPropertyIndex(propIndex, "PrimArrayClass"))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformancePrimArrayJsonVsECDTests::SetupTest(Utf8CP fileName, ECDb::OpenParams const& params)
    {
    ECDbR ecdb = SetupECDb(fileName);
    if (BE_SQLITE_OK != ecdb.ExecuteSql("CREATE TABLE testecd(Id INTEGER PRIMARY KEY, val BLOB);CREATE TABLE testjson(Id INTEGER PRIMARY KEY, val TEXT);"))
        return ERROR;

    ecdb.SaveChanges();
    BeFileName testFilePath;
    testFilePath.AssignUtf8(ecdb.GetDbFileName());
    ecdb.CloseDb();

    return m_ecdb.OpenBeSQLiteDb(testFilePath, params) == BE_SQLITE_OK ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
void PerformancePrimArrayJsonVsECDTests::LogTiming(StopWatch& timer, Utf8CP scenario, ECN::PrimitiveType primType, uint32_t arraySize, int rowCount)
    {
    Utf8String logMessage;
    logMessage.Sprintf("%s array [size %" PRIu32 "] - %s.", PrimitiveTypeToString(primType), arraySize, scenario);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), logMessage.c_str(), rowCount);
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
            case PRIMITIVETYPE_Point2D: return "Point2D";
            case PRIMITIVETYPE_Point3D: return "Point3D";
            case PRIMITIVETYPE_String: return "String";
            default:
                BeAssert(false);
                return nullptr;
        }
    }

END_ECDBUNITTESTS_NAMESPACE

