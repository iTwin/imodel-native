/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECDbTestHelper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbTests.h"
USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//Creates a test project,reads the schema from the specified path and imports it.
#define ECTEST_SETUP(TestName, schema, ecdbFileName)              \
    ECDbTestFixture::Initialize ();                               \
    ECSqlStatementHelper obj;                                     \
    obj.SetUpTest (TestName, schema, ecdbFileName);

//Prepares the statement with the specified ECSQL.The default expected status is set to Success. 
#define STATEMENT_PREPARE_SUCCESS(query)                 (obj.PrepareStatement (query))

//Prepares the statement with the specified ECSQL and compares the status returned by Prepare method and the status specified by the user. 
#define STATEMENT_PREPARE(query, expectedStatus)       (obj.PrepareStatement (query, expectedStatus))

//Perform a single step on the ECSQL statement.The default expected result is set to BE_SQLITE_DONE.
#define STATEMENT_EXECUTE_SUCCESS()                      (obj.ExecuteStatement ())

//Perform a single step on the ECSQL statement.Checks whether the status returned by Step() equals the status specified by user. 
#define STATEMENT_EXECUTE(expectedResult)                (obj.ExecuteStatement (expectedResult))

#define CLOSE_DB()       (obj.CloseDataBase())

//Methods to bind values to an ECSQL parameter
#define BIND_NULL(ParameterIndex)                      {EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindNull (ParameterIndex));}
#define BIND_INT(ParameterIndex, Val)                  {EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindInt (ParameterIndex, Val));}
#define BIND_LONG(ParameterIndex,Val)                  {EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindLong (ParameterIndex, Val));}
#define BIND_TEXT(ParameterIndex, Val, makeCopy)       {EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindText (ParameterIndex, Val, makeCopy));}
#define BIND_DOUBLE(ParameterIndex, Val)               {EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindDouble (ParameterIndex, Val));}
#define BIND_BOOLEAN(ParameterIndex, Val)              {EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindBoolean (ParameterIndex, Val));}
#define BIND_DATETIME(ParameterIndex, Val)             {EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindDateTime (ParameterIndex, Val));}
#define BIND_GEOMETRY(ParameterIndex, Val)             {EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindGeometry (ParameterIndex, Val));}
#define BIND_POINT_2D(ParameterIndex, Val)             {EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindPoint2D (ParameterIndex, Val));}
#define BIND_POINT_3D(ParameterIndex, Val)             {EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindPoint3D (ParameterIndex, Val));}
#define BIND_ID(ParameterIndex, Val)                   {EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindId(ParameterIndex, Val));}
#define BIND_STRUCT(ParameterIndex)                              obj.m_bindValues.bindStruct(ParameterIndex))
#define BIND_ARRAY(ParameterIndex, initialArrayCapacity)         obj.m_bindValues.bindArray(ParameterIndex, initialArrayCapacity)
#define BIND_BINARY(ParameterIndex, Val, binarySize, makeCopy)   {EXPECT_EQ (ECSqlStatus::Success,obj.m_bindValues.bindBinary (ParameterIndex, Val, binarySize, makeCopy));}

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

//---------------------------------------------------------------------------------------
// @bsiclass                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
    struct Bindings
        {
        private:
            ECSqlStatementHelper& m_parent;
        public:
            Bindings (ECSqlStatementHelper& parent)
                :m_parent (parent)
                {}

            ECSqlStatus bindNull (int Parameterindex)
                {
                return m_parent.stmt.BindNull (Parameterindex);
                }

            ECSqlStatus bindInt (int Parameterindex, int intVal)
                {
                return m_parent.stmt.BindInt (Parameterindex, intVal);
                }

            ECSqlStatus bindLong (int Parameterindex, int int64Val)
                {
                return m_parent.stmt.BindInt64 (Parameterindex, int64Val);
                }

            ECSqlStatus bindText (int Parameterindex, Utf8CP stringVal, IECSqlBinder::MakeCopy makeCopy)
                {
                return m_parent.stmt.BindText (Parameterindex, stringVal, makeCopy);
                }

            ECSqlStatus bindDouble (int Parameterindex, double doubleVal)
                {
                return m_parent.stmt.BindDouble (Parameterindex, doubleVal);
                }

            ECSqlStatus bindBoolean (int Parameterindex, bool boolVal)
                {
                return m_parent.stmt.BindBoolean (Parameterindex, boolVal);
                }

            ECSqlStatus bindBinary (int Parameterindex, const void* binaryVal, int binarySize, IECSqlBinder::MakeCopy makeCopy)
                {
                return m_parent.stmt.BindBinary (Parameterindex, binaryVal, binarySize, makeCopy);
                }

            ECSqlStatus bindDateTime (int Parameterindex, DateTimeCR dateTime)
                {
                return m_parent.stmt.BindDateTime (Parameterindex, dateTime);
                }

            ECSqlStatus bindGeometry (int Parameterindex, IGeometryCR geomVal)
                {
                return m_parent.stmt.BindGeometry (Parameterindex, geomVal);
                }

            ECSqlStatus bindPoint2D (int Parameterindex, DPoint2dCR value)
                {
                return m_parent.stmt.BindPoint2D (Parameterindex, value);
                }

            ECSqlStatus bindPoint3D (int Parameterindex, DPoint3dCR value)
                {
                return m_parent.stmt.BindPoint3D (Parameterindex, value);
                }

            ECSqlStatus bindId (int Parameterindex, BeInt64Id id)
                {
                return m_parent.stmt.BindId (Parameterindex, id);
                }

            IECSqlStructBinder& bindStruct (int Parameterindex)
                {
                return m_parent.stmt.BindStruct (Parameterindex);
                }

            IECSqlArrayBinder& bindArray (int Parameterindex, uint32_t initialArrayCapacity)
                {
                return m_parent.stmt.BindArray (Parameterindex, initialArrayCapacity);
                }
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
    BeFileName SearchPath (BeFileName schemaName)
        {
        if (schemaName.IsAbsolutePath ())
            {
            return schemaName;
            }
        else
            {
            BeTest::GetHost ().GetDocumentsRoot (schemaDir);
            schemaDir.AppendToPath (L"ECDb");
            schemaDir.AppendToPath (L"Schemas");
            schemaDir.AppendToPath ((WCharCP)schemaName);
            return schemaDir;
            }
        }

//Reads the schema from the path.
    SchemaReadStatus ReadXml (BeFileName SchemaDir)
        {
        ECSchemaPtr schemaPtr;
        context = ECSchemaReadContext::CreateContext ();
        SchemaReadStatus status = ECSchema::ReadFromXmlFile (schemaPtr, SchemaDir, *context);
        return status;
        }

 //Creates a test project,reads the schema from the specified path and imports it.
    void SetUpTest (Utf8String TestName, Utf8CP schema, WCharCP ecdbFileName)
        {
        m_item = testItem[TestName];
        m_item.path = schema;
        schemaDir = (BeFileName)m_item.path;

        //Get the Schema Path
        SearchPath (schemaDir);
        ASSERT_EQ (true, schemaDir.DoesPathExist ()) << "Invalid search path.";

        //Read Schema
        EXPECT_EQ (SchemaReadStatus::Success, ReadXml (schemaDir));

        //Creates a project
        EXPECT_EQ (BE_SQLITE_OK, ECDbTestUtility::CreateECDb (ecdb, nullptr, ecdbFileName));
        EXPECT_TRUE (ecdb.IsDbOpen ());

        //Imports schemas and cross checks.
        EXPECT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (context->GetCache ()));
        }

//! Prepares the statement with the specified ECSQL and compares the status returned by Prepare method and the status specified by the user.
    void PrepareStatement (Utf8String query, ECSqlStatus expectedStatus = ECSqlStatus::Success)
        {
        ECSqlStatus prepareStatus;
        if (stmt.IsPrepared ())
            {
            stmt.Reset ();
            stmt.ClearBindings ();
            stmt.Finalize ();
            }

        if (!query.empty ())
            {
            m_item.query = query;
            prepareStatus = stmt.Prepare (ecdb, m_item.query.c_str ());
            if (prepareStatus != expectedStatus)
                ASSERT_EQ (prepareStatus, expectedStatus) << "\nPrepare failed for statement:" << m_item.query.c_str ()<<"\n";
            }
        }

    //! Perform a single step on the ECSQL statement.Checks whether the status returned by Step() equals the status specified by user.
    void ExecuteStatement (DbResult expectedResult = DbResult::BE_SQLITE_DONE)
        {
        DbResult actualResult;
        actualResult = stmt.Step ();
        if (actualResult, expectedResult)
            ASSERT_EQ (actualResult, expectedResult) << "\nStep failed for statement :" << m_item.query.c_str ()<<"\n";
        }

    //! Finalize the last statement and closes the database.
    void CloseDataBase ()
        {
        stmt.Finalize ();
        ecdb.CloseDb ();
        }
    };

END_ECDBUNITTESTS_NAMESPACE