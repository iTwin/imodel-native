/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceOverflowProps.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

#define TABLE_NAME "test"
#define INTPROP_NAME "IntProp"
#define STRINGPROP_NAME "StringProp"
#define DOUBLEPROP_NAME "DoubleProp"
#define BLOBPROP_NAME "BlobProp"
#define POINTPROP_NAME "PointProp"
#define OVERFLOWPROP_NAME "OverflowProps"

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
struct PerformanceOverflowPropsTests : ECDbTestFixture
    {
    private:
        DateTime m_testDate;
        DPoint2d m_testPoint2d;
        DPoint3d m_testPoint3d;
        IGeometryPtr m_testGeometry;
        static const size_t s_testBlobSize = 128;
        Byte m_testBlob[s_testBlobSize];

    protected:

        PerformanceOverflowPropsTests()
            : m_testDate(DateTime::GetCurrentTimeUtc()), m_testPoint2d(DPoint2d::From(POINTXVALUE, POINTYVALUE)),
            m_testPoint3d(DPoint3d::From(POINTXVALUE, POINTYVALUE, POINTZVALUE)),
            m_testGeometry(IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0))))
            {
            for (size_t i = 0; i < 128; i++)
                {
                m_testBlob[i] = static_cast<Byte> (i + 32);
                }
            }

        BentleyStatus RunInsertRegularProperty(ECN::PrimitiveType propertyType, int propCount, int rowCount);
        BentleyStatus RunInsertRegularProperty(StopWatch&, Utf8CP fileName, ECN::PrimitiveType propertyType, int propCount, int rowCount);
        BentleyStatus RunSelectRegularProperty(ECN::PrimitiveType propertyType, int propCount, int rowCount);
        BentleyStatus RunInsertOverflowProperty(ECN::PrimitiveType propertyType, int propCount, int rowCount);
        BentleyStatus RunInsertOverflowProperty(StopWatch&, Utf8CP fileName, ECN::PrimitiveType propertyType, int propCount, int rowCount);
        BentleyStatus RunSelectOverflowProperty(ECN::PrimitiveType propertyType, int propCount, int rowCount);

        BentleyStatus SetupTest(Utf8CP fileName, ECDb::OpenParams const&, PrimitiveType propType, int propCount);

        void LogTiming(StopWatch&, Utf8CP logMessageHeader, ECN::PrimitiveType, int propCount, int rowCount);

        DateTime const& GetTestDate() const { return m_testDate; }
        DPoint2d const& GetTestPoint2d() const { return m_testPoint2d; }
        DPoint3d const& GetTestPoint3d() const { return m_testPoint3d; }
        IGeometryCR GetTestGeometry() const { return *m_testGeometry; }
        Byte const* GetTestBlob() const { return m_testBlob; }
        size_t GetTestBlobSize() const { return s_testBlobSize; }

        static Utf8CP ColumnNameFor(ECN::PrimitiveType);
        static Utf8CP PrimitiveTypeToString(ECN::PrimitiveType);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, InsertRegular_FewProps)
    {
    const int rowCount = 10000;
    const int propCount = 10;
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_String, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, rowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, SelectRegular_FewProps)
    {
    const int rowCount = 10000;
    const int propCount = 10;
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_String, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, rowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, InsertRegular_ManyProps)
    {
    const int rowCount = 10000;
    const int propCount = 150;
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_String, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, rowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, SelectRegular_ManyProps)
    {
    const int rowCount = 10000;
    const int propCount = 150;
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_String, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, rowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, InsertOverflow_FewProps)
    {
    const int rowCount = 10000;
    const int propCount = 10;
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_String, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, rowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, SelectOverflow_FewProps)
    {
    const int rowCount = 10000;
    const int propCount = 10;
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_String, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, rowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, InsertOverflow_ManyProps)
    {
    const int rowCount = 10000;
    const int propCount = 150;
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_String, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, rowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, SelectOverflow_ManyProps)
    {
    const int rowCount = 10000;
    const int propCount = 150;
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_String, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount, rowCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, rowCount));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::RunInsertRegularProperty(PrimitiveType propertyType, int propCount, int rowCount)
    {
    Utf8String fileName;
    fileName.Sprintf("regular_insert_%s_%d_props_opcount_%d.ecdb", PrimitiveTypeToString(propertyType), propCount, rowCount);

    StopWatch timer;
    if (SUCCESS != RunInsertRegularProperty(timer, fileName.c_str(), propertyType, propCount, rowCount))
        return ERROR;

    LogTiming(timer, "INSERT - Regular Property", propertyType, propCount, rowCount);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::RunInsertRegularProperty(StopWatch& timer, Utf8CP fileName, ECN::PrimitiveType propertyType, int propCount, int rowCount)
    {
    if (SUCCESS != SetupTest(fileName, ECDb::OpenParams(Db::OpenMode::ReadWrite), propertyType, propCount))
        return ERROR;

    Utf8String sql("INSERT INTO " TABLE_NAME "(");
    Utf8String valuesClause(" VALUES(");
    for (int i = 0; i < propCount; i++)
        {
        if (i > 0)
            {
            sql.append(",");
            valuesClause.append(",");
            }

        Utf8String colNoStr;
        colNoStr.Sprintf("_%d", i + 1);

        switch (propertyType)
            {
                case PRIMITIVETYPE_Point2D:
                    sql.append(POINTPROP_NAME).append(colNoStr).append("_X, " POINTPROP_NAME).append(colNoStr).append("_Y");
                    valuesClause.append("?,?");
                    break;

                case PRIMITIVETYPE_Point3D:
                    sql.append(POINTPROP_NAME).append(colNoStr).append("_X, " POINTPROP_NAME).append(colNoStr).append("_Y, " POINTPROP_NAME).append(colNoStr).append("_Z");
                    valuesClause.append("?,?,?");
                    break;

                default:
                    sql.append(ColumnNameFor(propertyType)).append(colNoStr);
                    valuesClause.append("?");
                    break;
            }
        }

    sql.append(")").append(valuesClause).append(")");
    
    timer.Start();
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), sql.c_str()))
        return ERROR;

    for (int i = 0; i < rowCount; i++)
        {
        for (int j = 0; j < propCount; j++)
            {
            int parameterIndex = j + 1;
            switch (propertyType)
                {
                    case PRIMITIVETYPE_Binary:
                        if (BE_SQLITE_OK != stmt.BindBlob(parameterIndex, GetTestBlob(), (int) GetTestBlobSize(), Statement::MakeCopy::Yes))
                            return ERROR;

                        break;

                    case PRIMITIVETYPE_Boolean:
                        if (BE_SQLITE_OK != stmt.BindInt(parameterIndex, BOOLVALUE ? 1 : 0))
                            return ERROR;

                        break;

                    case PRIMITIVETYPE_DateTime:
                    {
                    double jd = -1.0;
                    if (SUCCESS != GetTestDate().ToJulianDay(jd))
                        return ERROR;

                    if (BE_SQLITE_OK != stmt.BindDouble(parameterIndex, jd))
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_Double:
                        if (BE_SQLITE_OK != stmt.BindDouble(parameterIndex, DOUBLEVALUE))
                            return ERROR;

                        break;

                    case PRIMITIVETYPE_IGeometry:
                    {
                    bvector<Byte> fb;
                    BackDoor::IGeometryFlatBuffer::GeometryToBytes(fb, GetTestGeometry());

                    if (BE_SQLITE_OK != stmt.BindBlob(parameterIndex, fb.data(), (int) fb.size(), Statement::MakeCopy::Yes))
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_Integer:
                        if (BE_SQLITE_OK != stmt.BindInt(parameterIndex, INTVALUE))
                            return ERROR;

                        break;

                    case PRIMITIVETYPE_Long:
                        if (BE_SQLITE_OK != stmt.BindInt64(parameterIndex, INT64VALUE))
                            return ERROR;

                        break;

                    case PRIMITIVETYPE_Point2D:
                    {
                    if (BE_SQLITE_OK != stmt.BindDouble(2 * j + 1, POINTXVALUE) ||
                        BE_SQLITE_OK != stmt.BindDouble(2 * j + 2, POINTYVALUE))
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_Point3D:
                    {
                    if (BE_SQLITE_OK != stmt.BindDouble(3 * j + 1, POINTXVALUE) ||
                        BE_SQLITE_OK != stmt.BindDouble(3 * j + 2, POINTYVALUE) ||
                        BE_SQLITE_OK != stmt.BindDouble(3 * j + 3, POINTZVALUE))
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_String:
                        if (BE_SQLITE_OK != stmt.BindText(parameterIndex, STRINGVALUE, Statement::MakeCopy::Yes))
                            return ERROR;

                        break;

                    default:
                        return ERROR;
                }
            }

        if (BE_SQLITE_DONE != stmt.Step())
            return ERROR;

        stmt.ClearBindings();
        stmt.Reset();
        }

    stmt.Finalize();
    timer.Stop();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::RunSelectRegularProperty(PrimitiveType propertyType, int propCount, int rowCount)
    {
    Utf8String fileName;
    fileName.Sprintf("regular_select_%s_%d_props_opcount_%d.ecdb", PrimitiveTypeToString(propertyType), propCount, rowCount);

    StopWatch timer;
    if (SUCCESS != RunInsertRegularProperty(timer, fileName.c_str(), propertyType, propCount, rowCount))
        return ERROR;

    Utf8String filePath = GetECDb().GetDbFileName();
    GetECDb().CloseDb();

    if (BE_SQLITE_OK != m_ecdb.OpenBeSQLiteDb(filePath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly)))
        return ERROR;

    Utf8String sql;
    const int propNo = propCount / 2; //pick property in the middle of all props
    switch (propertyType)
        {
            case PRIMITIVETYPE_Point2D:
            {
            Utf8String colNamePrefix;
            colNamePrefix.Sprintf("%s_%d", ColumnNameFor(propertyType), propNo);

            sql.append("SELECT ").append(colNamePrefix).append("_X, ").append(colNamePrefix).append("_Y FROM " TABLE_NAME);
            break;
            }
            case PRIMITIVETYPE_Point3D:
            {
            Utf8String colNamePrefix;
            colNamePrefix.Sprintf("%s_%d", ColumnNameFor(propertyType), propNo);

            sql.append("SELECT ").append(colNamePrefix).append("_X, ").append(colNamePrefix).append("_Y, ").append(colNamePrefix).append("_Z FROM " TABLE_NAME);
            break;
            }

            default:
                sql.Sprintf("SELECT %s_%d FROM " TABLE_NAME, ColumnNameFor(propertyType), propCount / 2);
                break;
        }

    timer.Start();
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), sql.c_str()))
        return ERROR;

    while (BE_SQLITE_ROW == stmt.Step())
        {
        switch (propertyType)
            {
                case PRIMITIVETYPE_Binary:
                {
                void const* actualBlob = stmt.GetValueBlob(0);
                size_t actualBlobSize = (size_t) stmt.GetColumnBytes(0);
                if (actualBlobSize != GetTestBlobSize() || memcmp(actualBlob, GetTestBlob(), GetTestBlobSize()) != 0)
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_Boolean:
                {
                const bool actualVal = stmt.GetValueInt(0) != 0;
                if (BOOLVALUE != actualVal)
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_DateTime:
                {
                DateTime actualDt;
                if (SUCCESS != DateTime::FromJulianDay(actualDt, stmt.GetValueDouble(0), DateTime::Info(DateTime::Kind::Utc)))
                    return ERROR;

                if (!actualDt.Equals(GetTestDate()))
                    return ERROR;

                break;
                }

                case PRIMITIVETYPE_Double:
                {
                if (stmt.GetValueDouble(0) < 0)
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_IGeometry:
                {
                int blobSize = (int) stmt.GetColumnBytes(0);
                Byte const* blob = static_cast<Byte const*> (stmt.GetValueBlob(0));
                if (blob == nullptr)
                    return ERROR;

                BeAssert(blobSize > 0);
                const size_t blobSizeU = (size_t) blobSize;
                bvector<Byte> byteVec;
                byteVec.reserve(blobSizeU);
                byteVec.assign(blob, blob + blobSizeU);
                IGeometryPtr actualGeom = BackDoor::IGeometryFlatBuffer::BytesToGeometry(byteVec);
                if (actualGeom == nullptr || !actualGeom->IsSameStructureAndGeometry(GetTestGeometry()))
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_Integer:
                {
                if (INTVALUE != stmt.GetValueInt(0))
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_Long:
                {
                if (INT64VALUE != stmt.GetValueInt64(0))
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_Point2D:
                {
                DPoint2d pt = DPoint2d::From(stmt.GetValueDouble(0), stmt.GetValueDouble(1));
                if (!pt.AlmostEqual(GetTestPoint2d()))
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_Point3D:
                {
                DPoint3d pt = DPoint3d::From(stmt.GetValueDouble(0), stmt.GetValueDouble(1), stmt.GetValueDouble(2));
                if (!pt.AlmostEqual(GetTestPoint3d()))
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_String:
                {
                if (strcmp(stmt.GetValueText(0), STRINGVALUE) != 0)
                    return ERROR;

                break;
                }

                default:
                    return ERROR;
            }
        }
    
    stmt.Finalize();
    timer.Stop();
    LogTiming(timer, "SELECT - Regular Property",propertyType, propCount, rowCount);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::RunInsertOverflowProperty(PrimitiveType propertyType, int propCount, int rowCount)
    {
    Utf8String fileName;
    fileName.Sprintf("overflow_insert_%s_%d_props_opcount_%d.ecdb", PrimitiveTypeToString(propertyType), propCount, rowCount);

    StopWatch timer;
    if (SUCCESS != RunInsertOverflowProperty(timer, fileName.c_str(), propertyType, propCount, rowCount))
        return ERROR;

    LogTiming(timer, "INSERT - OverflowProperty", propertyType, propCount, rowCount);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::RunInsertOverflowProperty(StopWatch& timer, Utf8CP fileName, ECN::PrimitiveType propertyType, int propCount, int rowCount)
    {
    if (SUCCESS != SetupTest(fileName, ECDb::OpenParams(Db::OpenMode::ReadWrite), propertyType, propCount))
        return ERROR;

    timer.Start();

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), "INSERT INTO " TABLE_NAME "(" OVERFLOWPROP_NAME ") VALUES(?)"))
        return ERROR;

    for (int i = 0; i < rowCount; i++)
        {
        Json::Value json(Json::objectValue);
        for (int j = 0; j < propCount; j++)
            {
            Utf8String propName;
            propName.Sprintf("%s_%d", ColumnNameFor(propertyType), j + 1);
            Json::Value& val = json[propName.c_str()];
            switch (propertyType)
                {
                    case PRIMITIVETYPE_Binary:
                    {
                    if (SUCCESS != ECJsonUtilities::BinaryToJson(val, GetTestBlob(), GetTestBlobSize()))
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_Boolean:
                    {
                    val = Json::Value(BOOLVALUE);
                    break;
                    }

                    case PRIMITIVETYPE_DateTime:
                    {
                    double jd = -1.0;
                    if (SUCCESS != GetTestDate().ToJulianDay(jd))
                        return ERROR;

                    val = Json::Value(jd);
                    break;
                    }

                    case PRIMITIVETYPE_Double:
                    {
                    val = Json::Value(DOUBLEVALUE);
                    break;
                    }

                    case PRIMITIVETYPE_IGeometry:
                    {
                    bvector<Byte> fb;
                    BackDoor::IGeometryFlatBuffer::GeometryToBytes(fb, GetTestGeometry());

                    if (SUCCESS != ECJsonUtilities::BinaryToJson(val, fb.data(), fb.size()))
                        return ERROR;

                    break;
                    }

                    case PRIMITIVETYPE_Integer:
                    {
                    val = Json::Value(INTVALUE);
                    break;
                    }
                    case PRIMITIVETYPE_Long:
                    {
                    val = BeJsonUtilities::StringValueFromInt64(INT64VALUE);
                    break;
                    }
                    case PRIMITIVETYPE_Point2D:
                    {
                    val["X"] = Json::Value(POINTXVALUE);
                    val["Y"] = Json::Value(POINTYVALUE);
                    break;
                    }
                    case PRIMITIVETYPE_Point3D:
                    {
                    val["X"] = Json::Value(POINTXVALUE);
                    val["Y"] = Json::Value(POINTYVALUE);
                    val["Z"] = Json::Value(POINTZVALUE);
                    break;
                    }
                    case PRIMITIVETYPE_String:
                    {
                    val = Json::Value(STRINGVALUE);
                    break;
                    }

                    default:
                        return ERROR;
                }
            }

        Json::FastWriter writer;
        Utf8String jsonStr = writer.write(json);
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
BentleyStatus PerformanceOverflowPropsTests::RunSelectOverflowProperty(PrimitiveType propertyType, int propCount, int rowCount)
    {
    Utf8String fileName;
    fileName.Sprintf("overflow_select_%s_%d_props_opcount_%d.ecdb", PrimitiveTypeToString(propertyType), propCount, rowCount);

    StopWatch timer;
    if (SUCCESS != RunInsertOverflowProperty(timer, fileName.c_str(), propertyType, propCount, rowCount))
        return ERROR;

    Utf8String filePath = GetECDb().GetDbFileName();
    GetECDb().CloseDb();

    if (BE_SQLITE_OK != m_ecdb.OpenBeSQLiteDb(filePath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly)))
        return ERROR;

    const int propNo = propCount / 2; //pick property in the middle of all props
    Utf8String colName;
    colName.Sprintf("%s_%d", ColumnNameFor(propertyType), propNo);

    Utf8String sql;
    sql.Sprintf("SELECT json_extract(" OVERFLOWPROP_NAME ",'$%s') FROM " TABLE_NAME, colName.c_str());

    timer.Start();
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), sql.c_str()))
        return ERROR;

    while (BE_SQLITE_ROW == stmt.Step())
        {
        switch (propertyType)
            {
                case PRIMITIVETYPE_Binary:
                {
                Utf8CP base64Str = stmt.GetValueText(0);
                bvector<Byte> blob;
                if (SUCCESS != Base64Utilities::Decode(blob, base64Str, strlen(base64Str)))
                    return ERROR;

                if (blob.size() != GetTestBlobSize() || memcmp(blob.data(), GetTestBlob(), GetTestBlobSize()) != 0)
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_Boolean:
                {
                const bool actualVal = stmt.GetValueInt(0) != 0;
                if (BOOLVALUE != actualVal)
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_DateTime:
                {
                DateTime actualDt;
                if (SUCCESS != DateTime::FromJulianDay(actualDt, stmt.GetValueDouble(0), DateTime::Info(DateTime::Kind::Utc)))
                    return ERROR;

                if (!actualDt.Equals(GetTestDate()))
                    return ERROR;

                break;
                }

                case PRIMITIVETYPE_Double:
                {
                if (stmt.GetValueDouble(0) < 0)
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_IGeometry:
                {
                Utf8CP base64Str = stmt.GetValueText(0);
                bvector<Byte> fb;
                if (SUCCESS != Base64Utilities::Decode(fb, base64Str, strlen(base64Str)))
                    return ERROR;

                IGeometryPtr actualGeom = BackDoor::IGeometryFlatBuffer::BytesToGeometry(fb);
                if (actualGeom == nullptr || !actualGeom->IsSameStructureAndGeometry(GetTestGeometry()))
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_Integer:
                {
                if (INTVALUE != stmt.GetValueInt(0))
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_Long:
                {
                if (INT64VALUE != stmt.GetValueInt64(0))
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_Point2D:
                {
                Json::Value json;
                Json::Reader reader;
                if (!reader.Parse(stmt.GetValueText(0), json, false))
                    return ERROR;

                DPoint2d pt = DPoint2d::From(json["X"].asDouble(), json["Y"].asDouble());
                if (!pt.AlmostEqual(GetTestPoint2d()))
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_Point3D:
                {
                Json::Value json;
                Json::Reader reader;
                if (!reader.Parse(stmt.GetValueText(0), json, false))
                    return ERROR;

                DPoint3d pt = DPoint3d::From(json["X"].asDouble(), json["Y"].asDouble(), json["Z"].asDouble());
                if (!pt.AlmostEqual(GetTestPoint3d()))
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_String:
                {
                if (strcmp(stmt.GetValueText(0), STRINGVALUE) != 0)
                    return ERROR;

                break;
                }

                default:
                    return ERROR;
            }
        }

    stmt.Finalize();
    timer.Stop();
    LogTiming(timer, "SELECT - OverflowProperty", propertyType, propCount, rowCount);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::SetupTest(Utf8CP fileName, ECDb::OpenParams const& params, PrimitiveType propType, int propCount)
    {
    ECDbR ecdb = SetupECDb(fileName);

    Utf8String tableDdl("CREATE TABLE test(ECInstanceId INTEGER PRIMARY KEY, ");
    for (int i = 0; i < propCount; i++)
        {
        Utf8String colNoStr;
        colNoStr.Sprintf("_%d", i + 1);
        switch (propType)
            {
                case PRIMITIVETYPE_Binary:
                case PRIMITIVETYPE_IGeometry:
                    tableDdl.append(BLOBPROP_NAME).append(colNoStr).append(" BLOB");
                    break;

                case PRIMITIVETYPE_Boolean:
                case PRIMITIVETYPE_Integer:
                case PRIMITIVETYPE_Long:
                    tableDdl.append(INTPROP_NAME).append(colNoStr).append(" INTEGER");
                    break;

                case PRIMITIVETYPE_Double:
                case PRIMITIVETYPE_DateTime:
                    tableDdl.append(DOUBLEPROP_NAME).append(colNoStr).append(" REAL");
                    break;

                case PRIMITIVETYPE_String:
                    tableDdl.append(STRINGPROP_NAME).append(colNoStr).append(" TEXT");
                    break;

                case PRIMITIVETYPE_Point2D:
                    tableDdl.append(POINTPROP_NAME).append(colNoStr).append("_X REAL, " POINTPROP_NAME).append(colNoStr).append("_Y REAL");
                    break;

                case PRIMITIVETYPE_Point3D:
                    tableDdl.append(POINTPROP_NAME).append(colNoStr).append("_X REAL, " POINTPROP_NAME).append(colNoStr).append("_Y REAL, " POINTPROP_NAME).append(colNoStr).append("_Z REAL");
                    break;

                default:
                    return ERROR;
            }

        tableDdl.append(", ");
        }

    tableDdl.append(OVERFLOWPROP_NAME " TEXT)");

    if (BE_SQLITE_OK != ecdb.ExecuteSql(tableDdl.c_str()))
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
void PerformanceOverflowPropsTests::LogTiming(StopWatch& timer, Utf8CP scenario, ECN::PrimitiveType primType, int propCount, int rowCount)
    {
    Utf8String logMessage;
    logMessage.Sprintf("%s Property [count %d] - %s.", PrimitiveTypeToString(primType), propCount, scenario);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), logMessage.c_str(), rowCount);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP PerformanceOverflowPropsTests::ColumnNameFor(ECN::PrimitiveType primType)
    {
    switch (primType)
        {
            case PRIMITIVETYPE_Binary: 
            case PRIMITIVETYPE_IGeometry:
                return BLOBPROP_NAME;
            
            case PRIMITIVETYPE_Boolean: 
            case PRIMITIVETYPE_Integer:
            case PRIMITIVETYPE_Long:
                return INTPROP_NAME;

            case PRIMITIVETYPE_DateTime:
            case PRIMITIVETYPE_Double:
                return DOUBLEPROP_NAME;

            case PRIMITIVETYPE_Point2D:
            case PRIMITIVETYPE_Point3D:
                return POINTPROP_NAME;

            case PRIMITIVETYPE_String: 
                return STRINGPROP_NAME;
            
            default:
                BeAssert(false);
                return nullptr;
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP PerformanceOverflowPropsTests::PrimitiveTypeToString(ECN::PrimitiveType primType)
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
