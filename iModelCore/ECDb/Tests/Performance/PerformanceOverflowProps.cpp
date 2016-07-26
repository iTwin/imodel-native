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
    protected: 
        enum class JsonApi
            {
            RapidJson,
            String,
            JsonCpp
            };
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
        BentleyStatus RunSelectRegularProperty(ECN::PrimitiveType propertyType, int propCount);
        BentleyStatus RunInsertOverflowProperty(JsonApi, ECN::PrimitiveType propertyType, int propCount, int rowCount);
        BentleyStatus RunSelectOverflowProperty( ECN::PrimitiveType propertyType, int propCount, JsonApi pointxd = JsonApi::RapidJson);
        
        BentleyStatus BindRegularPropsForInsert(Statement&, int firstParameterIndex, PrimitiveType propType, int propCount) const;
        BentleyStatus CreateOverflowPropsStringForInsert(Utf8StringR, JsonApi, PrimitiveType propType, int propCount) const;
        BentleyStatus CreateOverflowPropsStringForInsertWithJsonCpp(Utf8StringR, PrimitiveType propType, int propCount) const;
        BentleyStatus CreateOverflowPropsStringForInsertWithRapidJson(Utf8StringR, PrimitiveType propType, int propCount) const;
        BentleyStatus CreateOverflowPropsStringForInsertWithStringApi(Utf8StringR, PrimitiveType propType, int propCount) const;

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
        static Utf8CP JsonApiToString(JsonApi);
    };


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, Point3DOverflowRetrieval)
    {
    const int propCount = 100;
    ASSERT_EQ(SUCCESS, SetupTest(ECDb::OpenParams(Db::OpenMode::Readonly), PRIMITIVETYPE_Point3D, propCount));

    Utf8String colName;
    GetColumnName(colName, propCount / 2);

    Utf8String compoundSql;
    compoundSql.Sprintf("SELECT json_extract(" OVERFLOWPROP_NAME ",'$.%s') FROM " TABLE_NAME, colName.c_str());

    StopWatch timer(true);
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(GetECDb(), compoundSql.c_str()));
    int actualRowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        rapidjson::Document document;
        ASSERT_FALSE(document.Parse<0>(stmt.GetValueText(0)).HasParseError());

        DPoint3d pt;
        ASSERT_EQ(SUCCESS, ECRapidJsonUtilities::JsonToPoint3D(pt, document));
        ASSERT_TRUE(pt.AlmostEqual(GetTestPoint3d()));
        actualRowCount++;
        }

    stmt.Finalize();
    timer.Stop();
    ASSERT_EQ(INITIALROWCOUNT, actualRowCount);
    LogTiming(timer, "SELECT - Overflow Property - as compound - parsed with rapidjson", PRIMITIVETYPE_Point3D, propCount, INITIALROWCOUNT);

    Utf8String expandedSql;
    expandedSql.Sprintf("SELECT json_extract(" OVERFLOWPROP_NAME ",'$.%s.x'), json_extract(" OVERFLOWPROP_NAME ",'$.%s.y'), json_extract(" OVERFLOWPROP_NAME ",'$.%s.z') FROM " TABLE_NAME,
                        colName.c_str(), colName.c_str(), colName.c_str());

    timer.Start();
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(GetECDb(), expandedSql.c_str()));

    actualRowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DPoint3d pt = DPoint3d::From(stmt.GetValueDouble(0), stmt.GetValueDouble(1), stmt.GetValueDouble(2));
        ASSERT_TRUE(pt.AlmostEqual(GetTestPoint3d()));
        actualRowCount++;
        }

    stmt.Finalize();
    timer.Stop();
    ASSERT_EQ(INITIALROWCOUNT, actualRowCount);

    LogTiming(timer, "SELECT - Overflow Property - expanded", PRIMITIVETYPE_Point3D, propCount, INITIALROWCOUNT);

    GetECDb().CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, ParsingRapidJsonVersusSqliteJson)
    {
    const PrimitiveType propType = PRIMITIVETYPE_Integer;
    const int propCount = 100;
    ASSERT_EQ(SUCCESS, SetupTest(ECDb::OpenParams(Db::OpenMode::Readonly), propType, propCount));

    Utf8String colName;
    GetColumnName(colName, propCount / 2);

    StopWatch timer(true);
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(GetECDb(), "SELECT " OVERFLOWPROP_NAME " FROM " TABLE_NAME));
    int actualRowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        rapidjson::Document document;
        ASSERT_FALSE(document.Parse<0>(stmt.GetValueText(0)).HasParseError());

        RapidJsonValueCR val = document[colName.c_str()];
        ASSERT_TRUE(val.IsInt());
        ASSERT_EQ(INTVALUE, val.GetInt());
        actualRowCount++;
        }

    stmt.Finalize();
    timer.Stop();
    ASSERT_EQ(INITIALROWCOUNT, actualRowCount);
    LogTiming(timer, "SELECT - Overflow Property - as whole - parsed with rapidjson", propType, propCount, INITIALROWCOUNT);

    Utf8String jsonExtractSql;
    jsonExtractSql.Sprintf("SELECT json_extract(" OVERFLOWPROP_NAME ",'$.%s') FROM " TABLE_NAME, colName.c_str());

    timer.Start();
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(GetECDb(), jsonExtractSql.c_str()));

    actualRowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ASSERT_EQ(INTVALUE, stmt.GetValueInt(0));
        actualRowCount++;
        }

    stmt.Finalize();
    timer.Stop();
    ASSERT_EQ(INITIALROWCOUNT, actualRowCount);

    LogTiming(timer, "SELECT - Overflow Property - SQLite json_extract", propType, propCount, INITIALROWCOUNT);

    GetECDb().CloseDb();
    }

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
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_String, propCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount));
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
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_String, propCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount));
    ASSERT_EQ(SUCCESS, RunSelectRegularProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, InsertOverflow_FewProps_JsonCpp)
    {
    const int propCount = 10;
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::JsonCpp, PrimitiveType::PRIMITIVETYPE_Integer, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::JsonCpp, PrimitiveType::PRIMITIVETYPE_Long, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::JsonCpp, PrimitiveType::PRIMITIVETYPE_Double, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::JsonCpp, PrimitiveType::PRIMITIVETYPE_String, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::JsonCpp, PrimitiveType::PRIMITIVETYPE_Point3D, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::JsonCpp, PrimitiveType::PRIMITIVETYPE_Binary, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::JsonCpp, PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, ROWCOUNT));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, InsertOverflow_FewProps_RapidJson)
    {
    const int propCount = 10;
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::RapidJson, PrimitiveType::PRIMITIVETYPE_Integer, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::RapidJson, PrimitiveType::PRIMITIVETYPE_Long, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::RapidJson, PrimitiveType::PRIMITIVETYPE_Double, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::RapidJson, PrimitiveType::PRIMITIVETYPE_String, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::RapidJson, PrimitiveType::PRIMITIVETYPE_Point3D, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::RapidJson, PrimitiveType::PRIMITIVETYPE_Binary, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::RapidJson, PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, ROWCOUNT));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, InsertOverflow_FewProps_StringApi)
    {
    const int propCount = 10;
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::String, PrimitiveType::PRIMITIVETYPE_Integer, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::String, PrimitiveType::PRIMITIVETYPE_Long, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::String, PrimitiveType::PRIMITIVETYPE_Double, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::String, PrimitiveType::PRIMITIVETYPE_String, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::String, PrimitiveType::PRIMITIVETYPE_Point3D, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::String, PrimitiveType::PRIMITIVETYPE_Binary, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::String, PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, ROWCOUNT));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, SelectOverflow_FewProps)
    {
    const int propCount = 10;
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_String, propCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount));

    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, JsonApi::JsonCpp));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, JsonApi::RapidJson));

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, InsertOverflow_ManyProps_JsonCpp)
    {
    const int propCount = 100;
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::JsonCpp, PrimitiveType::PRIMITIVETYPE_Integer, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::JsonCpp, PrimitiveType::PRIMITIVETYPE_Long, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::JsonCpp, PrimitiveType::PRIMITIVETYPE_Double, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::JsonCpp, PrimitiveType::PRIMITIVETYPE_String, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::JsonCpp, PrimitiveType::PRIMITIVETYPE_Point3D, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::JsonCpp, PrimitiveType::PRIMITIVETYPE_Binary, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::JsonCpp, PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, ROWCOUNT));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, InsertOverflow_ManyProps_RapidJson)
    {
    const int propCount = 100;
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::RapidJson, PrimitiveType::PRIMITIVETYPE_Integer, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::RapidJson, PrimitiveType::PRIMITIVETYPE_Long, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::RapidJson, PrimitiveType::PRIMITIVETYPE_Double, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::RapidJson, PrimitiveType::PRIMITIVETYPE_String, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::RapidJson, PrimitiveType::PRIMITIVETYPE_Point3D, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::RapidJson, PrimitiveType::PRIMITIVETYPE_Binary, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::RapidJson, PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, ROWCOUNT));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, InsertOverflow_ManyProps_StringApi)
    {
    const int propCount = 100;
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::String, PrimitiveType::PRIMITIVETYPE_Integer, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::String, PrimitiveType::PRIMITIVETYPE_Long, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::String, PrimitiveType::PRIMITIVETYPE_Double, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::String, PrimitiveType::PRIMITIVETYPE_String, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::String, PrimitiveType::PRIMITIVETYPE_Point3D, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::String, PrimitiveType::PRIMITIVETYPE_Binary, propCount, ROWCOUNT));
    ASSERT_EQ(SUCCESS, RunInsertOverflowProperty(JsonApi::String, PrimitiveType::PRIMITIVETYPE_IGeometry, propCount, ROWCOUNT));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceOverflowPropsTests, SelectOverflow_ManyProps)
    {
    const int propCount = 100;
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Integer, propCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Long, propCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Double, propCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_String, propCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Binary, propCount));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_IGeometry, propCount));

    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, JsonApi::JsonCpp));
    ASSERT_EQ(SUCCESS, RunSelectOverflowProperty(PrimitiveType::PRIMITIVETYPE_Point3D, propCount, JsonApi::RapidJson));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::RunInsertRegularProperty(PrimitiveType propertyType, int propCount, int rowCount)
    {
    if (SUCCESS != SetupTest(ECDb::OpenParams(Db::OpenMode::ReadWrite), propertyType, propCount))
        return ERROR;

    StopWatch timer(true);
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
BentleyStatus PerformanceOverflowPropsTests::RunSelectRegularProperty(PrimitiveType propertyType, int propCount)
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

    int actualRowCount = 0;
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
        actualRowCount++;
        }
    
    stmt.Finalize();
    timer.Stop();   
    if (INITIALROWCOUNT != actualRowCount)
        {
        EXPECT_EQ(INITIALROWCOUNT, actualRowCount);
        return ERROR;
        }

    LogTiming(timer, "SELECT - Regular Property",propertyType, propCount, INITIALROWCOUNT);
    GetECDb().CloseDb();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::RunInsertOverflowProperty(JsonApi jsonApi, PrimitiveType propertyType, int propCount, int rowCount)
    {
    if (SUCCESS != SetupTest(ECDb::OpenParams(Db::OpenMode::ReadWrite), propertyType, propCount))
        return ERROR;

    StopWatch timer(true);

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), "INSERT INTO " TABLE_NAME "(" OVERFLOWPROP_NAME ") VALUES(?)"))
        return ERROR;

    for (int i = 0; i < rowCount; i++)
        {
        Utf8String overflowJson;
        if (SUCCESS != CreateOverflowPropsStringForInsert(overflowJson, jsonApi, propertyType, propCount))
            return ERROR;

        if (stmt.BindText(1, overflowJson.c_str(), Statement::MakeCopy::No))
            return ERROR;

        if (BE_SQLITE_DONE != stmt.Step())
            return ERROR;

        stmt.ClearBindings();
        stmt.Reset();
        }

    stmt.Finalize();
    timer.Stop();
    Utf8String message;
    message.Sprintf("INSERT - OverflowProperty - JSON API: %s", JsonApiToString(jsonApi));
    LogTiming(timer, message.c_str(), propertyType, propCount, rowCount);
    GetECDb().CloseDb();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::RunSelectOverflowProperty(PrimitiveType propertyType, int propCount, JsonApi pointxdJsonApi)
    {
    if (SUCCESS != SetupTest(ECDb::OpenParams(Db::OpenMode::Readonly), propertyType, propCount))
        return ERROR;

    StopWatch timer(true);

    Utf8String colName;
    GetColumnName(colName, propCount / 2); //pick property in the middle of all props

    Utf8String sql;
    sql.Sprintf("SELECT json_extract(" OVERFLOWPROP_NAME ",'$.%s') FROM " TABLE_NAME, colName.c_str());

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), sql.c_str()))
        return ERROR;

    int actualRowCount = 0;
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
                switch (pointxdJsonApi)
                    {
                        case JsonApi::JsonCpp:
                        {
                        Json::Value json;
                        Json::Reader reader;
                        if (!reader.Parse(stmt.GetValueText(0), json, false))
                            return ERROR;

                        DPoint2d pt;
                        if (SUCCESS != ECJsonUtilities::JsonToPoint2D(pt, json))
                            return ERROR;

                        if (!pt.AlmostEqual(GetTestPoint2d()))
                            return ERROR;

                        break;
                        }

                        case JsonApi::RapidJson:
                        {
                        rapidjson::Document json;
                        if (json.Parse<0>(stmt.GetValueText(0)).HasParseError())
                            return ERROR;

                        DPoint2d pt;
                        if (SUCCESS != ECRapidJsonUtilities::JsonToPoint2D(pt, json))
                            return ERROR;

                        if (!pt.AlmostEqual(GetTestPoint2d()))
                            return ERROR;

                        break;
                        }

                        default:
                            return ERROR;
                    }

                break;
                }
                case PRIMITIVETYPE_Point3D:
                {
                switch (pointxdJsonApi)
                    {
                        case JsonApi::JsonCpp:
                        {
                        Json::Value json;
                        Json::Reader reader;
                        if (!reader.Parse(stmt.GetValueText(0), json, false))
                            return ERROR;

                        DPoint3d pt;
                        if (SUCCESS != ECJsonUtilities::JsonToPoint3D(pt, json))
                            return ERROR;

                        if (!pt.AlmostEqual(GetTestPoint3d()))
                            return ERROR;

                        break;
                        }

                        case JsonApi::RapidJson:
                        {
                        rapidjson::Document json;
                        if (json.Parse<0>(stmt.GetValueText(0)).HasParseError())
                            return ERROR;

                        DPoint3d pt;
                        if (SUCCESS != ECRapidJsonUtilities::JsonToPoint3D(pt, json))
                            return ERROR;

                        if (!pt.AlmostEqual(GetTestPoint3d()))
                            return ERROR;

                        break;
                        }

                        default:
                            return ERROR;
                    }

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

            actualRowCount++;
        }

    stmt.Finalize();
    timer.Stop();
    if (INITIALROWCOUNT != actualRowCount)
        {
        EXPECT_EQ(INITIALROWCOUNT, actualRowCount);
        return ERROR;
        }

    Utf8String message("SELECT - OverflowProperty");
    if (propertyType == PRIMITIVETYPE_Point2D || propertyType == PRIMITIVETYPE_Point3D)
        message.Sprintf("SELECT - OverflowProperty - JSON API: %s", JsonApiToString(pointxdJsonApi));

    LogTiming(timer, message.c_str(), propertyType, propCount, INITIALROWCOUNT);
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
BentleyStatus PerformanceOverflowPropsTests::CreateOverflowPropsStringForInsert(Utf8String& jsonStr, JsonApi jsonApi, PrimitiveType propType, int propCount) const
    {
    switch (jsonApi)
        {
            case JsonApi::JsonCpp:
                return CreateOverflowPropsStringForInsertWithJsonCpp(jsonStr, propType, propCount);

            case JsonApi::RapidJson:
                return CreateOverflowPropsStringForInsertWithRapidJson(jsonStr, propType, propCount);

            case JsonApi::String:
                return CreateOverflowPropsStringForInsertWithStringApi(jsonStr, propType, propCount);

            default:
                BeAssert(false);
                return ERROR;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::CreateOverflowPropsStringForInsertWithStringApi(Utf8String& jsonStr, PrimitiveType propType, int propCount) const
    {
    Utf8String json("{");
    for (int j = 0; j < propCount; j++)
        {
        Utf8String colName;
        GetColumnName(colName, j + 1);
        json.append("\"").append(colName).append("\":");
        switch (propType)
            {
                case PRIMITIVETYPE_Binary:
                {
                Utf8String blobStr;
                if (SUCCESS != Base64Utilities::Encode(blobStr, GetTestBlob(), GetTestBlobSize()))
                    return ERROR;
                
                json.append("\"").append(blobStr).append("\"");
                break;
                }

                case PRIMITIVETYPE_Boolean:
                {
                json.append(BOOLVALUE ? "true" : "false");
                break;
                }

                case PRIMITIVETYPE_DateTime:
                {
                double jd = -1.0;
                if (SUCCESS != GetTestDate().ToJulianDay(jd))
                    return ERROR;

                Utf8String jdStr;
                jdStr.Sprintf("%f", jd);
                json.append(jdStr);
                break;
                }

                case PRIMITIVETYPE_Double:
                {
                Utf8String jdStr;
                jdStr.Sprintf("%f", DOUBLEVALUE);
                json.append(jdStr);
                break;
                }

                case PRIMITIVETYPE_IGeometry:
                {
                bvector<Byte> fb;
                BackDoor::IGeometryFlatBuffer::GeometryToBytes(fb, GetTestGeometry());

                Utf8String blobStr;
                if (SUCCESS != Base64Utilities::Encode(blobStr, fb.data(), fb.size()))
                    return ERROR;

                json.append("\"").append(blobStr).append("\"");
                break;
                }

                case PRIMITIVETYPE_Integer:
                {
                Utf8String str;
                str.Sprintf("%d", INTVALUE);
                json.append(str);
                break;
                }
                case PRIMITIVETYPE_Long:
                {
                //int64 must be serialized as strings in JSON
                char str[32];
                sprintf(str, "\"%" PRId64 "\"", INT64VALUE);
                json.append(str);
                break;
                }
                case PRIMITIVETYPE_Point2D:
                {
                Utf8String str;
                str.Sprintf("{\"x\":%f,\"y\":%f}", POINTXVALUE, POINTYVALUE);
                json.append(str);
                break;
                }
                case PRIMITIVETYPE_Point3D:
                {
                Utf8String str;
                str.Sprintf("{\"x\":%f,\"y\":%f,\"z\":%f}", POINTXVALUE, POINTYVALUE, POINTZVALUE);
                json.append(str);
                break;
                }
                case PRIMITIVETYPE_String:
                {
                json.append("\"" STRINGVALUE "\"");
                break;
                }

                default:
                    return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::CreateOverflowPropsStringForInsertWithRapidJson(Utf8String& jsonStr, PrimitiveType propType, int propCount) const
    {
    rapidjson::Document json;
    json.SetObject();

    for (int j = 0; j < propCount; j++)
        {
        Utf8String colName;
        GetColumnName(colName, j + 1);
        rapidjson::Value v;
        switch (propType)
            {
                case PRIMITIVETYPE_Binary:
                {
                if (ECRapidJsonUtilities::BinaryToJson(v, GetTestBlob(), GetTestBlobSize(), json.GetAllocator()))
                    return ERROR;
                break;
                }

                case PRIMITIVETYPE_Boolean:
                {
                v.SetBool(BOOLVALUE);
                break;
                }

                case PRIMITIVETYPE_DateTime:
                {
                double jd = -1.0;
                if (SUCCESS != GetTestDate().ToJulianDay(jd))
                    return ERROR;

                v.SetDouble(jd);
                break;
                }

                case PRIMITIVETYPE_Double:
                {
                v.SetDouble(DOUBLEVALUE);
                break;
                }

                case PRIMITIVETYPE_IGeometry:
                {
                bvector<Byte> fb;
                BackDoor::IGeometryFlatBuffer::GeometryToBytes(fb, GetTestGeometry());

                if (ECRapidJsonUtilities::BinaryToJson(v, fb.data(), fb.size(), json.GetAllocator()))
                    return ERROR;

                break;
                }

                case PRIMITIVETYPE_Integer:
                {
                v.SetInt(INTVALUE);
                break;
                }
                case PRIMITIVETYPE_Long:
                {
                ECRapidJsonUtilities::Int64ToStringJsonValue(v, INT64VALUE, json.GetAllocator());
                break;
                }
                case PRIMITIVETYPE_Point2D:
                {
                if (ECRapidJsonUtilities::Point2DToJson(v, GetTestPoint2d(), json.GetAllocator()))
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_Point3D:
                {
                if (ECRapidJsonUtilities::Point3DToJson(v, GetTestPoint3d(), json.GetAllocator()))
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_String:
                {
                v.SetString(STRINGVALUE);
                break;
                }

                default:
                    return ERROR;
            }

        json.AddMember(rapidjson::Value(colName.c_str(), json.GetAllocator()).Move(), v, json.GetAllocator());
        }

    rapidjson::StringBuffer stringBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
    json.Accept(writer);

    jsonStr.assign(stringBuffer.GetString());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PerformanceOverflowPropsTests::CreateOverflowPropsStringForInsertWithJsonCpp(Utf8String& jsonStr, PrimitiveType propType, int propCount) const
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
                if (SUCCESS != ECJsonUtilities::Point2DToJson(val, GetTestPoint2d()))
                    return ERROR;

                break;
                }
                case PRIMITIVETYPE_Point3D:
                {
                if (SUCCESS != ECJsonUtilities::Point3DToJson(val, GetTestPoint3d()))
                    return ERROR;

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
    jsonStr.assign(writer.write(json));
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

            Utf8String overflowJson;
            if (SUCCESS != CreateOverflowPropsStringForInsert(overflowJson, JsonApi::RapidJson, propType, propCount))
                return ERROR;

            if (BE_SQLITE_OK != insertStmt.BindText(overflowParameterIndex, overflowJson.c_str(), Statement::MakeCopy::No))
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

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle       07/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP PerformanceOverflowPropsTests::JsonApiToString(JsonApi jsonApi)
    {
    switch (jsonApi)
        {
            case JsonApi::JsonCpp: return "JsonCpp";
            case JsonApi::RapidJson: return "RapidJson";
            case JsonApi::String: return "String API";

            default:
                BeAssert(false);
                return nullptr;
        }
    }

END_ECDBUNITTESTS_NAMESPACE
