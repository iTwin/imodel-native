/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once 

#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//Creates a test project,reads the schema from the specified path and imports it.
#define ECTEST_SETUP(TestName, schema, ecdbFileName)              \
    ECDbTestFixture::Initialize ();                               \
    ECSqlStatementHelper obj;                                     \
    obj.SetUpTest (TestName, schema, ecdbFileName);

//Prepares the statement with the specified ECSQL.The default expected status is set to Success. 
#define STATEMENT_PREPARE_SUCCESS(query)                                                                    \
    {ECSqlStatus prepareStatus = obj.PrepareStatement (query);                                              \
    ASSERT_EQ (ECSqlStatus::Success, prepareStatus) << "\nStatement prepare failed for:" << query << "\n"; }

  //Prepares the statement with the specified ECSQL and compares the status returned by Prepare method and the status specified by the user. 
#define ASSERT_STATEMENT_PREPARE(query, expectedStatus)                                                     \
      {ECSqlStatus prepareStatus = obj.PrepareStatement (query, expectedStatus);                            \
      ASSERT_EQ (expectedStatus, prepareStatus) << "\nStatement prepare failed for:" << query << "\n"; }

#define EXPECT_STATEMENT_PREPARE(query, expectedStatus)                                                     \
      {ECSqlStatus prepareStatus = obj.PrepareStatement (query, expectedStatus);                            \
      EXPECT_EQ (expectedStatus, prepareStatus) << "\nStatement prepare failed for:" << query << "\n"; }

  //Perform a single step on the ECSQL statement.The default expected result is set to BE_SQLITE_DONE.
#define STATEMENT_EXECUTE_SUCCESS()                                                   \
      {DbResult stepResult = obj.ExecuteStatement ();                                 \
      ASSERT_EQ (DbResult::BE_SQLITE_DONE, stepResult) << "\nStep failed.\n"; }

  //Perform a single step on the ECSQL statement.Checks whether the status returned by Step() equals the status specified by user.
#define ASSERT_STATEMENT_EXECUTE(expectedResult)                                      \
      {DbResult stepResult = obj.ExecuteStatement (expectedResult);                   \
      EXPECT_EQ (expectedResult, stepResult) << "\nStep failed.\n"; }

#define EXPECT_STATEMENT_EXECUTE(expectedResult)                                     \
      {DbResult stepResult = obj.ExecuteStatement (expectedResult);                  \
      EXPECT_EQ (expectedResult, stepResult) << "\nStep failed.\n"; }

//Adds the specified query in a list.
#define ADD_QUERY(query)                                                             \
    obj.AddQuery(query)

//Iterates the list of queries and executes them sequentially.
#define EXECUTE_LIST()                                                               \
    obj.ExecuteQueries()

//Close the database.
#define CLOSE_DB()                                                                  \
    obj.CloseDataBase()

//Methods to bind values to an ECSQL parameter.The status is assumed to be success in this case.
#define BIND_NULL(ParameterIndex)                                                                   \
      EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindNull (ParameterIndex));
#define BIND_INT(ParameterIndex, Val)                                                               \
      EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindInt (ParameterIndex, Val));
#define BIND_LONG(ParameterIndex,Val)                                                               \
      EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindLong (ParameterIndex, Val));
#define BIND_TEXT(ParameterIndex, Val, makeCopy)                                                    \
      EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindText (ParameterIndex, Val, makeCopy));
#define BIND_DOUBLE(ParameterIndex, Val)                                                            \
      EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindDouble (ParameterIndex, Val));
#define BIND_BOOLEAN(ParameterIndex, Val)                                                           \
      EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindBoolean (ParameterIndex, Val));
#define BIND_DATETIME(ParameterIndex, Val)                                                          \
      EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindDateTime (ParameterIndex, Val));
#define BIND_GEOMETRY(ParameterIndex, Val)                                                          \
      EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindGeometry (ParameterIndex, Val));
#define BIND_POINT2D(ParameterIndex, Val)                                                           \
      EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindPoint2D (ParameterIndex, Val));
#define BIND_POINT3D(ParameterIndex, Val)                                                           \
      EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindPoint3D (ParameterIndex, Val));
#define BIND_ID(ParameterIndex, Val)                                                                \
      EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindId(ParameterIndex, Val));
#define BIND_STRUCT(ParameterIndex)                                                                 \
      obj.m_bindValues.bindStruct(ParameterIndex))
#define BIND_ARRAY(ParameterIndex, initialArrayCapacity)                                            \
      obj.m_bindValues.bindArray(ParameterIndex, initialArrayCapacity)
#define BIND_BINARY(ParameterIndex, Val, binarySize, makeCopy)                                      \
      EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindBinary (ParameterIndex, Val, binarySize, makeCopy));

//Compares the bind status with the status specified by the user.
#define BIND_NULL_STATUS(ParameterIndex, expectedStatus)                                            \
      EXPECT_EQ (expectedStatus,obj.m_bindValues.bindNull (ParameterIndex));
#define BIND_INT_STATUS(ParameterIndex, Val, expectedStatus)                                        \
      EXPECT_EQ (expectedStatus,obj.m_bindValues.bindInt (ParameterIndex, Val));
#define BIND_LONG_STATUS(ParameterIndex,Val, expectedStatus)                                        \
      EXPECT_EQ (expectedStatus,obj.m_bindValues.bindLong (ParameterIndex, Val));
#define BIND_TEXT_STATUS(ParameterIndex, Val, makeCopy, expectedStatus)                             \
      EXPECT_EQ (expectedStatus,obj.m_bindValues.bindText (ParameterIndex, Val, makeCopy));
#define BIND_DOUBLE_STATUS(ParameterIndex, Val, expectedStatus)                                     \
      EXPECT_EQ (expectedStatus,obj.m_bindValues.bindDouble (ParameterIndex, Val));
#define BIND_BOOLEAN_STATUS(ParameterIndex, Val, expectedStatus)                                    \
      EXPECT_EQ (expectedStatus,obj.m_bindValues.bindBoolean (ParameterIndex, Val));
#define BIND_DATETIME_STATUS(ParameterIndex, Val, expectedStatus)                                   \
      EXPECT_EQ (expectedStatus,obj.m_bindValues.bindDateTime (ParameterIndex, Val));
#define BIND_GEOMETRY_STATUS(ParameterIndex, Val, expectedStatus)                                   \
      EXPECT_EQ (expectedStatus,obj.m_bindValues.bindGeometry (ParameterIndex, Val));
#define BIND_POINT2D_STATUS(ParameterIndex, Val, expectedStatus)                                    \
      EXPECT_EQ (expectedStatus,obj.m_bindValues.bindPoint2D (ParameterIndex, Val));
#define BIND_POINT3D_STATUS(ParameterIndex, Val, expectedStatus)                                    \
      EXPECT_EQ (expectedStatus,obj.m_bindValues.bindPoint3D (ParameterIndex, Val));
#define BIND_ID_STATUS(ParameterIndex, Val, expectedStatus)                                         \
      EXPECT_EQ (expectedStatus,obj.m_bindValues.bindId(ParameterIndex, Val));
#define BIND_BINARY_STATUS(ParameterIndex, Val, binarySize, makeCopy, expectedStatus)               \
    EXPECT_EQ (expectedStatus, obj.m_bindValues.bindBinary (ParameterIndex, Val, binarySize, makeCopy));

  //Verify values for the given coloumns.
#define ASSERT_NULL(ColumnIndex, Val)                               \
      {bool Null = obj.assertNull (ColumnIndex, Val);               \
      ASSERT_EQ (Val, Null); }
#define ASSERT_INT(ColumnIndex,Val)                                 \
      {int Int = obj.assertInt (ColumnIndex, Val);                  \
      ASSERT_EQ (Val, Int); }
#define ASSERT_TEXT(ColumnIndex,Val)                                \
      {Utf8CP String = obj.assertText (ColumnIndex, Val);           \
      ASSERT_STREQ (Val, String); }
#define ASSERT_BOOLEAN(ColumnIndex,Val)                             \
      {bool Bool = obj.assertBoolean (ColumnIndex, Val);            \
      ASSERT_EQ (Val, Bool); }
#define ASSERT_DATETIME(ColumnIndex,Val)                            \
      {DateTime dateTime = obj.assertDateTime (ColumnIndex, Val);   \
      ASSERT_EQ (Val, dateTimeVal); }
#define ASSERT_DOUBLE(ColumnIndex,Val)                              \
      {double Double = obj.assertDouble (ColumnIndex, Val);         \
      ASSERT_EQ (Val, Double); }
#define ASSERT_INT64(ColumnIndex,Val)                               \
      {int64_t int64 = obj.assertInt64 (ColumnIndex, Val);          \
      ASSERT_EQ (Val, int64); }
#define ASSERT_UINT64(ColumnIndex,Val)                              \
      {uint64_t uInt64 = obj.assertUInt64 (ColumnIndex, Val);       \
      ASSERT_EQ (Val, uInt64); }
#define ASSERT_GEOMETRY(ColumnIndex,Val)                                \
      {IGeometryPtr Geometry = obj.assertGeometry (ColumnIndex, Val);   \
      ASSERT_EQ (Val, Geometry); }
#define ASSERT_POINT2D(ColumnIndex, X,Y)                               \
      {DPoint2d pointer2D = obj.assertPoint2D (columnIndex, X, Y);     \
      ASSERT_TRUE (pointer2D.x == X);                                  \
      ASSERT_TRUE (pointer2D.y == Y); }
#define ASSERT_POINT3D(ColumnIndex,X,Y,Z)                              \
      {DPoint3d pointer3D = obj.assertPoint3D (ColumnIndex, X, Y, Z);  \
      ASSERT_TRUE (pointer3D.x == X);                                  \
      ASSERT_TRUE (pointer3D.y == Y);                                  \
      ASSERT_TRUE (pointer3D.z == Z); }
#define ASSERT_BINARY(ColumnIndex,Val)                              \
      {void const* Binary = obj.assertBinary (ColumnIndex, Val);    \
      ASSERT_EQ (Val, Binary);}

//---------------------------------------------------------------------------------------
// @bsiclass                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlStatementHelper
    {
    //Member variables.
    ECDb ecdb;
    ECSqlStatement stmt;
    BeFileName schemaDir;
    ECSchemaReadContextPtr context;
    bvector<Utf8CP> queryList;

//---------------------------------------------------------------------------------------
// @bsiclass                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
    struct Bindings
        {
        private:
            ECSqlStatementHelper& m_parent;
        public:
            explicit Bindings (ECSqlStatementHelper& parent)
                :m_parent (parent)
                {}

            ECSqlStatus bindNull (int Parameterindex);
            ECSqlStatus bindInt (int Parameterindex, int intVal);
            ECSqlStatus bindLong (int Parameterindex, int int64Val);
            ECSqlStatus bindText (int Parameterindex, Utf8CP stringVal, IECSqlBinder::MakeCopy makeCopy);
            ECSqlStatus bindDouble (int Parameterindex, double doubleVal);
            ECSqlStatus bindBoolean (int Parameterindex, bool boolVal);
            ECSqlStatus bindBinary (int Parameterindex, const void* binaryVal, int binarySize, IECSqlBinder::MakeCopy makeCopy);
            ECSqlStatus bindDateTime (int Parameterindex, DateTimeCR dateTime);
            ECSqlStatus bindGeometry (int Parameterindex, IGeometryCR geomVal);
            ECSqlStatus bindPoint2D (int Parameterindex, DPoint2dCR value);
            ECSqlStatus bindPoint3D (int Parameterindex, DPoint3dCR value);
            ECSqlStatus bindId (int Parameterindex, BeInt64Id id);

            //For Named Parameter binding
            ECSqlStatus bindNull (Utf8CP ParameterName);
            ECSqlStatus bindInt (Utf8CP ParameterName, int intVal);
            ECSqlStatus bindLong (Utf8CP ParameterName, int int64Val);
            ECSqlStatus bindText (Utf8CP ParameterName, Utf8CP stringVal, IECSqlBinder::MakeCopy makeCopy);
            ECSqlStatus bindDouble (Utf8CP ParameterName, double doubleVal);
            ECSqlStatus bindBoolean (Utf8CP ParameterName, bool boolVal);
            ECSqlStatus bindBinary (Utf8CP ParameterName, const void* binaryVal, int binarySize, IECSqlBinder::MakeCopy makeCopy);
            ECSqlStatus bindDateTime (Utf8CP ParameterName, DateTimeCR dateTime);
            ECSqlStatus bindGeometry (Utf8CP ParameterName, IGeometryCR geomVal);
            ECSqlStatus bindPoint2D (Utf8CP ParameterName, DPoint2dCR value);
            ECSqlStatus bindPoint3D (Utf8CP ParameterName, DPoint3dCR value);
            ECSqlStatus bindId (Utf8CP ParameterName, BeInt64Id id);
        };

    Bindings m_bindValues;
    ECSqlStatementHelper ()
        :m_bindValues (*this)
        {}

    ~ECSqlStatementHelper ()
        {
        CloseDataBase ();
        }

 //---------------------------------------------------------------------------------------
// @bsiclass                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
    struct TestItem
        {
        Utf8String path;
        Utf8String query; 
        };

    bmap<Utf8String, TestItem> testItem;
    TestItem m_item;

    //Searches the referenced schema on the default/absolute Path.
    BeFileName SearchPath (BeFileName schemaName);

    //Reads the schema from the path.
    SchemaReadStatus ReadXml (ECDbCR, BeFileName SchemaDir);

    //Creates a test project,reads the schema from the specified path and imports it.
    void SetUpTest (Utf8String TestName, Utf8CP schema, Utf8CP ecdbFileName);

    //! Prepares the statement with the specified ECSQL and compares the status returned by Prepare method and the status specified by the user.
    ECSqlStatus PrepareStatement (Utf8String query, ECSqlStatus expectedStatus = ECSqlStatus::Success);

    //! Perform a single step on the ECSQL statement.Checks whether the status returned by Step() equals the status specified by user.
    DbResult ExecuteStatement (DbResult expectedResult = DbResult::BE_SQLITE_DONE);

   //Verify values for the given column in the result set
    bool assertNull (int columnIndex, bool expectedVal);
    int assertInt (int columnIndex, int expectedVal);
    Utf8CP assertText (int columnIndex, Utf8CP expectedVal);
    bool assertBoolean (int columnIndex, bool expectedVal);
    DateTime assertDateTime (int columnIndex, DateTime expectedVal);
    double assertDouble (int columnIndex, double expectedVal);
    IGeometryPtr assertGeometry (int columnIndex, IGeometryPtr expectedVal);
    int64_t assertInt64 (int columnIndex, int64_t expectedVal);
    uint64_t assertUInt64 (int columnIndex, int64_t expectedVal);
    template <class TBeInt64Id> TBeInt64Id assertId (int columnIndex, int64_t expectedVal);
    DPoint2d assertPoint2D (int columnIndex, double X, double Y);
    DPoint3d assertPoint3D (int columnIndex, double X, double Y, double Z);
    void const* assertBinary (int columnIndex, int* binarySize = nullptr);

    //Adds the query in a list.
    void AddQuery (Utf8CP query);

    //Iterates through the list of queries and executes them.
    void ExecuteQueries ();

    //! Finalize the last statement and closes the database.
    void CloseDataBase ();
    };

END_ECDBUNITTESTS_NAMESPACE