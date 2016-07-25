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
#define OVERFLOWPROP_NAME "OverflowProps"

#define BOOLVALUE true
#define INTVALUE 1000
#define INT64VALUE INT64_C(123456791234)
#define DOUBLEVALUE 6.123123123123
#define STRINGVALUE "Hello, world!!"
#define POINTXVALUE 3.3314134314134
#define POINTYVALUE -133.3314134314134
#define POINTZVALUE 100.3314134314

#define INITIALROWCOUNT 100000
#define ROWCOUNT 5000

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
        BentleyStatus RunUpdateRegularProperty(ECN::PrimitiveType propertyType, int propCount, int rowCount);
        BentleyStatus RunSelectRegularProperty(ECN::PrimitiveType propertyType, int propCount, int rowCount);
        BentleyStatus RunInsertOverflowProperty(ECN::PrimitiveType propertyType, int propCount, int rowCount);
        BentleyStatus RunUpdateOverflowProperty(ECN::PrimitiveType propertyType, int propCount, int rowCount);
        BentleyStatus RunSelectOverflowProperty(ECN::PrimitiveType propertyType, int propCount, int rowCount);
        
        BentleyStatus BindRegularPropsForInsert(Statement&, int firstParameterIndex, PrimitiveType propType, int propCount) const;
        BentleyStatus BindOverflowPropsForInsert(Statement&, int parameterIndex, PrimitiveType propType, int propCount) const;

        BentleyStatus SetupTest(ECDb::OpenParams const&, PrimitiveType propType, int propCount);

        void LogTiming(StopWatch&, Utf8CP logMessageHeader, ECN::PrimitiveType, int propCount, int rowCount);

        DateTime const& GetTestDate() const { return m_testDate; }
        DPoint2d const& GetTestPoint2d() const { return m_testPoint2d; }
        DPoint3d const& GetTestPoint3d() const { return m_testPoint3d; }
        IGeometryCR GetTestGeometry() const { return *m_testGeometry; }
        Byte const* GetTestBlob() const { return m_testBlob; }
        size_t GetTestBlobSize() const { return s_testBlobSize; }

        static void GetColumnName(Utf8String& colName, int propNo) { colName.Sprintf("Prop%d", propNo); }

        static Utf8CP PrimitiveTypeToString(ECN::PrimitiveType);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, InsertRegular_FewProps)
    {
    const int propCount = 10;
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_String, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, ROWCOUNT));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, SelectRegular_FewProps)
    {
    const int propCount = 10;
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_String, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, ROWCOUNT));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, InsertRegular_ManyProps)
    {
    const int propCount = 100;
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_String, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertRegularProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, ROWCOUNT));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, SelectRegular_ManyProps)
    {
    const int propCount = 100;
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_String, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, ROWCOUNT));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, InsertOverflow_FewProps)
    {
    const int propCount = 100;
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_String, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, ROWCOUNT));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, SelectOverflow_FewProps)
    {
    const int propCount = 100;
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_String, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, ROWCOUNT));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, InsertOverflow_ManyProps)
    {
    const int propCount = 150;
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_String, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, ROWCOUNT));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, SelectOverflow_ManyProps)
    {
    const int propCount = 150;
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_String, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, ROWCOUNT));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::RunInsertRegularProperty(PrimitiveType propertyType, int propCount, int rowCount)
    {
    if (SUCCESS != SetupTest(ECDb::OpenParams(Db::OpenMode::ReadWrite), propertyType, propCount))
        return ERROR;

    StopWatch timer;
    Utf8String sql("INSERT INTO " TABLE_NAME "(");
    Utf8String valuesClause(" VALUES(");
    for (int i = 0; i < propCount; i++)
        {
        if (i > 0)
            {
            sql.append(",");
            valuesClause.append(",");
            }

        Utf8String colName;
        GetColumnName(colName, i + 1);

        switch (propertyType)
            {
                case PRIMITIVETYPE_Point2D:
                    sql.append(colName).append("_X, ").append(colName).append("_Y");
                    valuesClause.append("?,?");
                    break;

                case PRIMITIVETYPE_Point3D:
                    sql.append(colName).append("_X, ").append(colName).append("_Y, ").append(colName).append("_Z");
                    valuesClause.append("?,?,?");
                    break;

                default:
                    sql.append(colName);
                    valuesClause.append("?");
                    break;
            }
        }

    sql.append(")").append(valuesClause).append(")");

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), sql.c_str()))
        return ERROR;

    for (int i = 0; i < rowCount; i++)
        {
        if (SUCCESS != BindRegularPropsForInsert(stmt, 1, propertyType, propCount))
            return ERROR;

        if (BE_SQLITE_DONE != stmt.Step())
            return ERROR;

        stmt.ClearBindings();
        stmt.Reset();
        }

    stmt.Finalize();
    timer.Stop();
    LogTiming(timer, "INSERT - Regular Property", propertyType, propCount, rowCount);
    GetECDb().CloseDb();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::RunSelectRegularProperty(PrimitiveType propertyType, int propCount, int rowCount)
    {
    if (SUCCESS != SetupTest(ECDb::OpenParams(Db::OpenMode::Readonly), propertyType, propCount))
        return ERROR;

    StopWatch timer(true);
    Utf8String sql("SELECT ");
    const int propNo = propCount / 2; //pick property in the middle of all props
    Utf8String colName;
    GetColumnName(colName, propNo);
    switch (propertyType)
        {
            case PRIMITIVETYPE_Point2D:
                sql.append(colName).append("_X, ").append(colName).append("_Y");
                break;
            case PRIMITIVETYPE_Point3D:
                sql.append(colName).append("_X, ").append(colName).append("_Y, ").append(colName).append("_Z");
                break;

            default:
                sql.append(colName);
                break;
        }

    sql.append(" FROM " TABLE_NAME);
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
    GetECDb().CloseDb();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::RunInsertOverflowProperty(PrimitiveType propertyType, int propCount, int rowCount)
    {
    if (SUCCESS != SetupTest(ECDb::OpenParams(Db::OpenMode::ReadWrite), propertyType, propCount))
        return ERROR;

    StopWatch timer(true);

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), "INSERT INTO " TABLE_NAME "(" OVERFLOWPROP_NAME ") VALUES(?)"))
        return ERROR;

    for (int i = 0; i < rowCount; i++)
        {
        if (SUCCESS != BindOverflowPropsForInsert(stmt, 1, propertyType, propCount))
            return ERROR;

        if (BE_SQLITE_DONE != stmt.Step())
            return ERROR;

        stmt.ClearBindings();
        stmt.Reset();
        }

    stmt.Finalize();
    timer.Stop();
    LogTiming(timer, "INSERT - OverflowProperty", propertyType, propCount, rowCount);
    GetECDb().CloseDb();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::RunSelectOverflowProperty(PrimitiveType propertyType, int propCount, int rowCount)
    {
    if (SUCCESS != SetupTest(ECDb::OpenParams(Db::OpenMode::Readonly), propertyType, propCount))
        return ERROR;

    StopWatch timer(true);

    Utf8String colName;
    GetColumnName(colName, propCount / 2); //pick property in the middle of all props

    Utf8String sql;
    sql.Sprintf("SELECT json_extract(" OVERFLOWPROP_NAME ",'$%s') FROM " TABLE_NAME, colName.c_str());

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
    GetECDb().CloseDb();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::BindRegularPropsForInsert(Statement& stmt, int firstParameterIndex, PrimitiveType propType, int propCount) const
    {
    for (int j = 0; j < propCount; j++)
        {
        int parameterIndex = j + firstParameterIndex;
        switch (propType)
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

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::BindOverflowPropsForInsert(Statement& stmt, int parameterIndex, PrimitiveType propType, int propCount) const
    {
    Json::Value json(Json::objectValue);
    for (int j = 0; j < propCount; j++)
        {
        Utf8String colName;
        GetColumnName(colName, j + 1);
        Json::Value& val = json[colName.c_str()];
        switch (propType)
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
    if (BE_SQLITE_OK != stmt.BindText(parameterIndex, jsonStr.c_str(), Statement::MakeCopy::No))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::SetupTest(ECDb::OpenParams const& params, PrimitiveType propType, int propCount)
    {
    Utf8String fileNameNoExt;
    fileNameNoExt.Sprintf("performanceoverflow_%s_propcount_%d_initrowcount_%d", PrimitiveTypeToString(propType), propCount, INITIALROWCOUNT);

    Utf8String seedFileName;
    seedFileName.Sprintf("%s_seed_%d.ecdb", fileNameNoExt.c_str(), DateTime::GetCurrentTimeUtc().GetDayOfYear());

    BeFileName seedPath = ECDbTestUtility::BuildECDbPath(seedFileName.c_str());
    if (!seedPath.DoesPathExist())
        {
        ECDbR ecdb = SetupECDb(seedFileName.c_str());

        Utf8String tableDdl("CREATE TABLE " TABLE_NAME "(ECInstanceId INTEGER PRIMARY KEY, ");
        Utf8String insertSql("INSERT INTO " TABLE_NAME "(");
        Utf8String insertValuesClause(" VALUES(");

        for (int i = 0; i < propCount; i++)
            {
            Utf8String colName;
            GetColumnName(colName, i + 1);
            switch (propType)
                {
                    case PRIMITIVETYPE_Binary:
                    case PRIMITIVETYPE_IGeometry:
                        tableDdl.append(colName).append(" BLOB");

                        insertSql.append(colName);
                        insertValuesClause.append("?");
                        break;

                    case PRIMITIVETYPE_Boolean:
                    case PRIMITIVETYPE_Integer:
                    case PRIMITIVETYPE_Long:
                        tableDdl.append(colName).append(" INTEGER");
                        insertSql.append(colName);
                        insertValuesClause.append("?");
                        break;

                    case PRIMITIVETYPE_Double:
                    case PRIMITIVETYPE_DateTime:
                        tableDdl.append(colName).append(" REAL");
                        insertSql.append(colName);
                        insertValuesClause.append("?");
                        break;

                    case PRIMITIVETYPE_String:
                        tableDdl.append(colName).append(" TEXT");
                        insertSql.append(colName);
                        insertValuesClause.append("?");
                        break;

                    case PRIMITIVETYPE_Point2D:
                        tableDdl.append(colName).append("_X REAL, ").append(colName).append("_Y REAL");

                        insertSql.append(colName).append("_X, ").append(colName).append("_Y");
                        insertValuesClause.append("?,?");

                        break;

                    case PRIMITIVETYPE_Point3D:
                        tableDdl.append(colName).append("_X REAL, ").append(colName).append("_Y REAL, ").append(colName).append("_Z REAL");
                        insertSql.append(colName).append("_X, ").append(colName).append("_Y, ").append(colName).append("_Z");
                        insertValuesClause.append("?,?,?");
                        break;

                    default:
                        return ERROR;
                }

            tableDdl.append(", ");
            insertSql.append(",");
            insertValuesClause.append(",");
            }

        tableDdl.append(OVERFLOWPROP_NAME " TEXT)");
        insertSql.append(OVERFLOWPROP_NAME ") ");
        insertValuesClause.append("?)");
        insertSql.append(insertValuesClause);

        if (BE_SQLITE_OK != ecdb.ExecuteSql(tableDdl.c_str()))
            return ERROR;

        Statement insertStmt;
        if (BE_SQLITE_OK != insertStmt.Prepare(ecdb, insertSql.c_str()))
            return ERROR;

        int overflowParameterIndex = propCount + 1;
        if (propType == PRIMITIVETYPE_Point2D)
            overflowParameterIndex = propCount * 2 + 1;
        else if (propType == PRIMITIVETYPE_Point3D)
            overflowParameterIndex = propCount * 3 + 1;

        for (int i = 0; i < INITIALROWCOUNT; i++)
            {
            if (SUCCESS != BindRegularPropsForInsert(insertStmt, 1, propType, propCount))
                return ERROR;

            if (SUCCESS != BindOverflowPropsForInsert(insertStmt, overflowParameterIndex, propType, propCount))
                return ERROR;

            if (BE_SQLITE_DONE != insertStmt.Step())
                return ERROR;

            insertStmt.ClearBindings();
            insertStmt.Reset();
            }

        insertStmt.Finalize();
        ecdb.SaveChanges();
        ecdb.CloseDb();
        }

    Utf8String fileName(fileNameNoExt);
    fileName.append(".ecdb");
    return CloneECDb(m_ecdb, fileName.c_str(), seedPath, params) == BE_SQLITE_OK ? SUCCESS : ERROR;
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
