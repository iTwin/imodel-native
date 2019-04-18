/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlExecutionFrameworkHelper.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//*************** ECSqlStatementHelper ***************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindNull(int Parameterindex)
    {
    return m_parent.stmt.BindNull(Parameterindex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindInt(int Parameterindex, int intVal)
    {
    return m_parent.stmt.BindInt(Parameterindex, intVal);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindLong(int Parameterindex, int int64Val)
    {
    return m_parent.stmt.BindInt64(Parameterindex, int64Val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindText(int Parameterindex, Utf8CP stringVal, IECSqlBinder::MakeCopy makeCopy)
    {
    return m_parent.stmt.BindText(Parameterindex, stringVal, makeCopy);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindDouble(int Parameterindex, double doubleVal)
    {
    return m_parent.stmt.BindDouble(Parameterindex, doubleVal);
    }

ECSqlStatus ECSqlStatementHelper::Bindings::bindBoolean(int Parameterindex, bool boolVal)
    {
    return m_parent.stmt.BindBoolean(Parameterindex, boolVal);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindBinary(int Parameterindex, const void* binaryVal, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    return m_parent.stmt.BindBlob(Parameterindex, binaryVal, binarySize, makeCopy);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindDateTime(int Parameterindex, DateTimeCR dateTime)
    {
    return m_parent.stmt.BindDateTime(Parameterindex, dateTime);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindGeometry(int Parameterindex, IGeometryCR geomVal)
    {
    return m_parent.stmt.BindGeometry(Parameterindex, geomVal);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindPoint2D(int Parameterindex, DPoint2dCR value)
    {
    return m_parent.stmt.BindPoint2d(Parameterindex, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindPoint3D(int Parameterindex, DPoint3dCR value)
    {
    return m_parent.stmt.BindPoint3d(Parameterindex, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindId(int Parameterindex, BeInt64Id id)
    {
    return m_parent.stmt.BindId(Parameterindex, id);
    }

//*************** For Named Parameter Binding ***************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindNull(Utf8CP ParameterName)
    {
    int index = m_parent.stmt.GetParameterIndex(ParameterName);
    return m_parent.stmt.BindNull(index);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindInt(Utf8CP ParameterName, int intVal)
    {
    int index = m_parent.stmt.GetParameterIndex(ParameterName);
    return m_parent.stmt.BindInt(index, intVal);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindLong(Utf8CP ParameterName, int int64Val)
    {
    int index = m_parent.stmt.GetParameterIndex(ParameterName);
    return m_parent.stmt.BindInt64(index, int64Val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindText(Utf8CP ParameterName, Utf8CP stringVal, IECSqlBinder::MakeCopy makeCopy)
    {
    int index = m_parent.stmt.GetParameterIndex(ParameterName);
    return m_parent.stmt.BindText(index, stringVal, makeCopy);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindDouble(Utf8CP ParameterName, double doubleVal)
    {
    int index = m_parent.stmt.GetParameterIndex(ParameterName);
    return m_parent.stmt.BindDouble(index, doubleVal);
    }

ECSqlStatus ECSqlStatementHelper::Bindings::bindBoolean(Utf8CP ParameterName, bool boolVal)
    {
    int index = m_parent.stmt.GetParameterIndex(ParameterName);
    return m_parent.stmt.BindBoolean(index, boolVal);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindBinary(Utf8CP ParameterName, const void* binaryVal, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    int index = m_parent.stmt.GetParameterIndex(ParameterName);
    return m_parent.stmt.BindBlob(index, binaryVal, binarySize, makeCopy);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindDateTime(Utf8CP ParameterName, DateTimeCR dateTime)
    {
    int index = m_parent.stmt.GetParameterIndex(ParameterName);
    return m_parent.stmt.BindDateTime(index, dateTime);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindGeometry(Utf8CP ParameterName, IGeometryCR geomVal)
    {
    int index = m_parent.stmt.GetParameterIndex(ParameterName);
    return m_parent.stmt.BindGeometry(index, geomVal);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindPoint2D(Utf8CP ParameterName, DPoint2dCR value)
    {
    int index = m_parent.stmt.GetParameterIndex(ParameterName);
    return m_parent.stmt.BindPoint2d(index, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindPoint3D(Utf8CP ParameterName, DPoint3dCR value)
    {
    int index = m_parent.stmt.GetParameterIndex(ParameterName);
    return m_parent.stmt.BindPoint3d(index, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlStatementHelper::Bindings::bindId(Utf8CP ParameterName, BeInt64Id id)
    {
    int index = m_parent.stmt.GetParameterIndex(ParameterName);
    return m_parent.stmt.BindId(index, id);
    }

//---------------------------------------------------------------------------------------
//! Searches the referenced schema on the default/absolute Path.
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
BeFileName ECSqlStatementHelper::SearchPath(BeFileName schemaName)
    {
    if (schemaName.IsAbsolutePath())
        {
        return schemaName;
        }
    else
        {
        BeTest::GetHost().GetDocumentsRoot(schemaDir);
        schemaDir.AppendToPath(L"ECDb");
        schemaDir.AppendToPath(L"Schemas");
        schemaDir.AppendToPath((WCharCP) schemaName);
        return schemaDir;
        }
    }

//---------------------------------------------------------------------------------------
//! Reads the schema from the path.
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
SchemaReadStatus ECSqlStatementHelper::ReadXml(ECDbCR ecdb, BeFileName SchemaDir)
    {
    ECSchemaPtr schemaPtr;
    context = ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(ecdb.GetSchemaLocater());
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schemaPtr, SchemaDir, *context);
    return status;
    }

//---------------------------------------------------------------------------------------
//! Creates a test project,reads the schema from the specified path and imports it.
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlStatementHelper::SetUpTest(Utf8String TestName, Utf8CP schema, Utf8CP ecdbFileName)
    {
    m_item = testItem[TestName];
    m_item.path = schema;
    schemaDir = (BeFileName) m_item.path;

    //Get the Schema Path
    SearchPath(schemaDir);
    ASSERT_EQ(true, schemaDir.DoesPathExist()) << "Invalid schema name/search path.";

    //Creates a project
    EXPECT_EQ(BE_SQLITE_OK, ECDbTestFixture::CreateECDb(ecdb, ecdbFileName));
    EXPECT_TRUE(ecdb.IsDbOpen());

    //Read Schema
    EXPECT_EQ(SchemaReadStatus::Success, ReadXml(ecdb, schemaDir));

    //Imports schemas and cross checks.
    EXPECT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()));
    }

//------------------------------------------------------------------------------------------------------------------------------------------
//! Prepares the statement with the specified ECSQL and compares the status returned by Prepare method and the status specified by the user.
// @bsimethod                                            Maha Nasir                                     12/15
//+---------------+---------------+---------------+---------------+---------------+----------------------------------------------------------
ECSqlStatus ECSqlStatementHelper::PrepareStatement(Utf8String query, ECSqlStatus expectedStatus)
    {
    if (stmt.IsPrepared())
        {
        stmt.Reset();
        stmt.ClearBindings();
        stmt.Finalize();
        }
    if (!query.empty())
        {
        m_item.query = query;
        }
    return stmt.Prepare(ecdb, m_item.query.c_str());
    }

//----------------------------------------------------------------------------------------------------------------------------------
//! Perform a single step on the ECSQL statement.Checks whether the status returned by Step() equals the status specified by user.
// @bsimethod                                            Maha Nasir                                     12/15
//+---------------+---------------+---------------+---------------+---------------+--------------------------------------------------
DbResult ECSqlStatementHelper::ExecuteStatement(DbResult expectedResult)
    {
    EXPECT_TRUE(stmt.IsPrepared()) << "\nStatement is unprepared.Can't call Step on an unprepared statement.\n";
    return stmt.Step();
    }

//---------------------------------------------------------------------------------------
//! Verify values for the given column in the result set
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlStatementHelper::assertNull(int columnIndex, bool expectedVal)
    {
    return stmt.IsValueNull(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
int ECSqlStatementHelper::assertInt(int columnIndex, int expectedVal)
    {
    return stmt.GetValueInt(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ECSqlStatementHelper::assertText(int columnIndex, Utf8CP expectedVal)
    {
    return stmt.GetValueText(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlStatementHelper::assertBoolean(int columnIndex, bool expectedVal)
    {
    return stmt.GetValueBoolean(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
DateTime ECSqlStatementHelper::assertDateTime(int columnIndex, DateTime expectedVal)
    {
    return stmt.GetValueDateTime(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
double ECSqlStatementHelper::assertDouble(int columnIndex, double expectedVal)
    {
    return stmt.GetValueDouble(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr ECSqlStatementHelper::assertGeometry(int columnIndex, IGeometryPtr expectedVal)
    {
    return stmt.GetValueGeometry(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
int64_t ECSqlStatementHelper::assertInt64(int columnIndex, int64_t expectedVal)
    {
    return stmt.GetValueInt64(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t ECSqlStatementHelper::assertUInt64(int columnIndex, int64_t expectedVal)
    {
    return stmt.GetValueUInt64(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
template <class TBeInt64Id> TBeInt64Id ECSqlStatementHelper::assertId(int columnIndex, int64_t expectedVal)
    {
    return stmt.GetValueId<TBeInt64Id>(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d ECSqlStatementHelper::assertPoint2D(int columnIndex, double X, double Y)
    {
    return stmt.GetValuePoint2d(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d ECSqlStatementHelper::assertPoint3D(int columnIndex, double X, double Y, double Z)
    {
    return stmt.GetValuePoint3d(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void const* ECSqlStatementHelper::assertBinary(int columnIndex, int* binarySize)
    {
    return stmt.GetValueBlob(columnIndex, binarySize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlStatementHelper::AddQuery(Utf8CP query)
    {
    queryList.push_back(query);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlStatementHelper::ExecuteQueries()
    {
    if (stmt.IsPrepared())
        {
        stmt.Reset();
        stmt.ClearBindings();
        stmt.Finalize();
        }

    for (Utf8CP query : queryList)
        {
        ASSERT_EQ(ECSqlStatus::Success, PrepareStatement(query)) << "Prepare failed for statement " << query;
        ASSERT_EQ(DbResult::BE_SQLITE_DONE, ExecuteStatement());
        stmt.Finalize();
        }
    queryList.clear();
    }

//---------------------------------------------------------------------------------------
//! Finalize the last statement and closes the database.
// @bsimethod                                     Maha Nasir                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlStatementHelper::CloseDataBase()
    {
    stmt.Finalize();
    ecdb.CloseDb();
    }

END_ECDBUNITTESTS_NAMESPACE