/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbMapPersistence_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE
using namespace rapidjson;

static Byte s_Utf8BOM[] = { 0xef, 0xbb, 0xbf };
//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  03/2015
//---------------------------------------------------------------------------------------
struct ECDbMapPersistenceTests : public ::testing::Test
    {
    static bool ReadStringFromUtf8File (Utf8String& strValue, WCharCP path);
    static bool ReadJsonFromFile (Utf8String& jsonValue, WCharCP path);
    static void WriteJsonDocumentToFile (rapidjson::StringBuffer& buffer, WCharCP path);
    static void GetClassesPerSchemaId (ECDbR db, rapidjson::Document& doc, rapidjson::Value& classArray, int ecSchemaId, bool mapProperties);
    static void GetPropertiesPerClass (ECDbR db, rapidjson::Document& doc, rapidjson::Value& classArray, int ecClassId);
    static void GetRelationshipConstraintClasses (ECDbR db, rapidjson::Document& doc, rapidjson::Value& constraintClasses, int ecClassId);
    static void GetBaseClass (ECDbR db, rapidjson::Document& doc, rapidjson::Value& baseClassObject, int ecClassId);
    static void GetSchemaObjectById (ECDbR db, rapidjson::Document& doc, rapidjson::Value& schemaObject, int ecSchemaId, bool mapClasses);
    static void GetJsonBasedOnTableName (ECDbR db, rapidjson::Document& doc, rapidjson::Value& schemaObject, Utf8String tableName);
    static void GetKeyProperty (ECDbR db, rapidjson::Document& doc, rapidjson::Value& constraintArray, int ecClassId, int relationecClassId);
    static void GetTableColumnsPerTableId (ECDbR db, rapidjson::Document& doc, rapidjson::Value& tableObject, int TableId);
    static void GetColumnsPerTable (ECDbR db, rapidjson::Document & doc, rapidjson::Value& columnArray, int TableId);
    static void InsertRelationshipInstances (ECDbR db, Utf8CP relationshipClassName, Utf8CP sourceClassName, Utf8CP targetClassName, Utf8CP schemaName, int numberOfSourceInstances, int numberOfTargetInstancesPerSource);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                    Muhammad Hassan                  03/2015
//---------------------------------------------------------------------------------------
bool ECDbMapPersistenceTests::ReadStringFromUtf8File (Utf8String& strValue, WCharCP path)
    {
    strValue = "";
    uint64_t rawSize;

    BeFile file;
    BeFileStatus fileStatus = file.Open (path, BeFileAccess::Read);
    if ((BeFileStatus::Success != fileStatus))
        return false;

    fileStatus = file.GetSize (rawSize);
    if (!(BeFileStatus::Success == fileStatus && rawSize <= UINT32_MAX))
        {
        file.Close ();
        return false;
        }

    uint32_t sizeToRead = static_cast<uint32_t>(rawSize);
    uint32_t sizeRead;
    ScopedArray<Byte> scopedBuffer (sizeToRead);
    Byte* buffer = scopedBuffer.GetData ();
    fileStatus = file.Read (buffer, &sizeRead, sizeToRead);
    if (!(BeFileStatus::Success == fileStatus && sizeRead == sizeToRead))
        {
        file.Close ();
        return false;
        }
    // Validate it's a UTF8 file
    if (!(buffer[0] == s_Utf8BOM[0] && buffer[1] == s_Utf8BOM[1] && buffer[2] == s_Utf8BOM[2]))
        {
        file.Close ();
        return false;
        }

    for (uint32_t ii = 3; ii < sizeRead; ii++)
        {
        if (buffer[ii] == '\n' || buffer[ii] == '\r')
            continue;
        strValue.append (1, buffer[ii]);
        }

    file.Close ();
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Muhammad Hassan                  03/2015
//---------------------------------------------------------------------------------------
bool ECDbMapPersistenceTests::ReadJsonFromFile (Utf8String& jsonValue, WCharCP path)
    {
    rapidjson::Document doc;
    Utf8String strValue;
    if (!ReadStringFromUtf8File (strValue, path))
        {
        EXPECT_FALSE (true);
        return false;
        }
    else
        {
        if (doc.Parse<0> (strValue.c_str ()).HasParseError ())
            {
            LOG.infov ("Json Read parsing Error %s", path);
            EXPECT_TRUE (false);
            return false;
            }
        else
            {
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> write (buffer);
            doc.Accept (write);
            jsonValue = buffer.GetString ();
            return true;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Muhammad Hassan                  03/2015
//---------------------------------------------------------------------------------------
void ECDbMapPersistenceTests::WriteJsonDocumentToFile (rapidjson::StringBuffer& buffer, WCharCP path)
    {
    FILE *file;
    const char* str = buffer.GetString ();
    const Byte utf8BOM[] = { 0xef, 0xbb, 0xbf };
    if ((file = fopen (Utf8String (path).c_str (), "w")) != NULL)
        {
        size_t t = fwrite (utf8BOM, sizeof(Utf8Char), 3, file); // to Make file UTF8
        t = fwrite (str, sizeof(Utf8Char), strlen (str), file);
        ASSERT_TRUE (t == t);
        fclose (file);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Muhammad Hassan                  03/2015
//---------------------------------------------------------------------------------------
void ECDbMapPersistenceTests::GetBaseClass (ECDbR db, rapidjson::Document& doc, rapidjson::Value& baseClassObject, int ecClassId)
    {
    BeSQLite::Statement sqlstmt;
    auto status = sqlstmt.Prepare (db, "Select Name from ec_Class where Id = (select BaseECClassId from ec_BaseClass where ECClassId=?)");
    ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
    sqlstmt.BindInt (1, ecClassId);
    if (sqlstmt.Step () == DbResult::BE_SQLITE_ROW)
        {
        Utf8String baseClassName = sqlstmt.GetValueText (0);
        Value value;
        value.SetString (baseClassName.c_str (), doc.GetAllocator ());
        baseClassObject.AddMember ("BaseClass", value, doc.GetAllocator ());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Muhammad Hassan                  03/2015
//---------------------------------------------------------------------------------------
void ECDbMapPersistenceTests::GetKeyProperty (ECDbR db, rapidjson::Document& doc, rapidjson::Value& constraintArray, int ecClassId, int relationecClassId)
    {
    BeSQLite::Statement statement;
    rapidjson::Value obj (rapidjson::kObjectType);
    rapidjson::Value keyProperty_array (rapidjson::kArrayType);
    auto status = statement.Prepare (db, "select ec_Property.[Name] from ec_Property where Id IN (select ec_RelationshipConstraintClassProperty.[RelationECPropertyId] from [ec_RelationshipConstraintClassProperty] where ECClassId = ? and RelationECClassId = ?)");
    ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
    statement.BindInt (1, ecClassId);
    statement.BindInt (2, relationecClassId);

    while (statement.Step () == DbResult::BE_SQLITE_ROW)
        {
        Utf8String columnValue = statement.GetValueText (0);
        Value value;
        value.SetString (columnValue.c_str (), doc.GetAllocator ());
        keyProperty_array.PushBack (value, doc.GetAllocator ());
        }

    obj.AddMember ("KeyProperties", keyProperty_array, doc.GetAllocator ());
    constraintArray.PushBack (obj, doc.GetAllocator ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Muhammad Hassan                  03/2015
//---------------------------------------------------------------------------------------
void ECDbMapPersistenceTests::GetRelationshipConstraintClasses (ECDbR db, rapidjson::Document& doc, rapidjson::Value& constraintClasses, int ecClassId)
    {
    rapidjson::Value sourceClassArray (rapidjson::kArrayType);
    rapidjson::Value targetClassArray (rapidjson::kArrayType);
    BeSQLite::Statement sqlStatement;
    auto status = sqlStatement.Prepare (db, "SELECT ECRelationshipEnd, RelationECClassId FROM ec_RelationshipConstraintClass WHERE ECClassId=?");
    ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
    sqlStatement.BindInt (1, ecClassId);

    while (sqlStatement.Step () == DbResult::BE_SQLITE_ROW)
        {
        BeSQLite::Statement sqlstmt;
        int relationECClassId = sqlStatement.GetValueInt (1);
        status = sqlstmt.Prepare (db, "SELECT Name FROM ec_Class WHERE Id=?");
        ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
        sqlstmt.BindInt (1, relationECClassId);
        if (sqlstmt.Step () == DbResult::BE_SQLITE_ROW)
            {
            Utf8String columnValue = sqlstmt.GetValueText (0);
            Value value;
            value.SetString (columnValue.c_str (), doc.GetAllocator ());
            if (sqlStatement.GetValueInt (0) == 1)
                {
                sourceClassArray.PushBack (value, doc.GetAllocator ());
                GetKeyProperty (db, doc, sourceClassArray, ecClassId, relationECClassId);
                }
            else
                {
                targetClassArray.PushBack (value, doc.GetAllocator ());
                GetKeyProperty (db, doc, targetClassArray, ecClassId, relationECClassId);
                }
            }
        }

    constraintClasses.AddMember ("Source Classes", sourceClassArray, doc.GetAllocator ());
    constraintClasses.AddMember ("Target Classes", targetClassArray, doc.GetAllocator ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Muhammad Hassan                  03/2015
//---------------------------------------------------------------------------------------
void ECDbMapPersistenceTests::GetClassesPerSchemaId (ECDbR db, rapidjson::Document& doc, rapidjson::Value& classArray, int ecSchemaId, bool mapProperties)
    {
    Utf8String classPath;
    BeSQLite::Statement sqlStatement;
    auto status = sqlStatement.Prepare (db, "SELECT Id,Name, DisplayLabel, Description, IsDomainClass, IsStruct, IsCustomAttribute, RelationStrength, RelationStrengthDirection, IsRelationship  FROM ec_Class where ECschemaId = ?");
    ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
    sqlStatement.BindInt (1, ecSchemaId);

    while (sqlStatement.Step () == DbResult::BE_SQLITE_ROW)
        {
        bool PropertiesMapped = false;
        Utf8String relationStrength;
        Utf8String relationStrengthDirection;
        int ecClassId = sqlStatement.GetValueInt (0);
        int columncount = sqlStatement.GetColumnCount ();
        rapidjson::Value row_obj (rapidjson::kObjectType);
        for (int column = 0; column < columncount; column++)
            {
            BeSQLite::DbValueType columnType = sqlStatement.GetColumnType (column);
            Utf8String columnValue;
            if (columnType == BeSQLite::DbValueType::IntegerVal || columnType == BeSQLite::DbValueType::TextVal)
                {
                Utf8String columnname = sqlStatement.GetColumnName (column);
                Value jsonColumnName;
                jsonColumnName.SetString (columnname.c_str (), doc.GetAllocator ());
                if (columnname == "Id")
                    continue;

                if (columnname == "IsRelationship")
                    {
                    if (sqlStatement.GetValueInt (column) == 1)
                        {
                        rapidjson::Value relationConstraints (rapidjson::kObjectType);
                        Value jsonColumnValue;
                        jsonColumnValue.SetString (relationStrength.c_str (), doc.GetAllocator ());
                        relationConstraints.AddMember ("RelationStrength", jsonColumnValue, doc.GetAllocator ());
                        jsonColumnValue.SetString (relationStrengthDirection.c_str (), doc.GetAllocator ());
                        relationConstraints.AddMember ("RelationStrengthDirection", jsonColumnValue, doc.GetAllocator ());
                        GetRelationshipConstraintClasses (db, doc, relationConstraints, ecClassId);
                        row_obj.AddMember (jsonColumnName, relationConstraints, doc.GetAllocator ());
                        }
                    }
                else if (columnname == "RelationStrength")
                    {
                    relationStrength = sqlStatement.GetValueText (column);
                    }
                else if (columnname == "RelationStrengthDirection")
                    {
                    relationStrengthDirection = sqlStatement.GetValueText (column);
                    }
                else
                    {
                    columnValue = sqlStatement.GetValueText (column);
                    Value jsonColumnValue;
                    jsonColumnValue.SetString (columnValue.c_str (), doc.GetAllocator ());
                    row_obj.AddMember (jsonColumnName, jsonColumnValue, doc.GetAllocator ());
                    }
                }
            }

        GetBaseClass (db, doc, row_obj, ecClassId);
        if (!PropertiesMapped && mapProperties)
            {
            rapidjson::Value properties_array (rapidjson::kArrayType);
            GetPropertiesPerClass (db, doc, properties_array, ecClassId);
            row_obj.AddMember ("Properties", properties_array, doc.GetAllocator ());
            }

        classArray.PushBack (row_obj, doc.GetAllocator ());
        }

    sqlStatement.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Muhammad Hassan                  03/2015
//---------------------------------------------------------------------------------------
void ECDbMapPersistenceTests::GetPropertiesPerClass (ECDbR db, rapidjson::Document& doc, rapidjson::Value& classArray, int ecClassId)
    {
    BeSQLite::Statement sqlStatement;
    auto status = sqlStatement.Prepare (db, "SELECT Name, DisplayLabel, Description, IsArray, TypeCustom, TypeECPrimitive, TypeECStruct, IsReadOnly, MinOccurs, MaxOccurs FROM ec_Property where ECClassId = ?");
    sqlStatement.BindInt (1, ecClassId);
    ASSERT_EQ (status, DbResult::BE_SQLITE_OK);

    while (sqlStatement.Step () == DbResult::BE_SQLITE_ROW)
        {
        rapidjson::Value row_obj (rapidjson::kObjectType);
        for (int column = 0; column < 10; column++)
            {
            BeSQLite::DbValueType columnType = sqlStatement.GetColumnType (column);
            if (columnType == BeSQLite::DbValueType::IntegerVal || columnType == BeSQLite::DbValueType::TextVal)
                {
                Utf8String columnValue;
                Utf8String columnname = sqlStatement.GetColumnName (column);
                Value jsonColumnName;
                jsonColumnName.SetString (columnname.c_str (), doc.GetAllocator ());
                if (columnname != "TypeECStruct")
                    {
                    columnValue = sqlStatement.GetValueText (column);
                    }
                else
                    {
                    BeSQLite::Statement stmt;
                    status = stmt.Prepare (db, "SELECT Name FROM ec_Class WHERE Id = ?");
                    ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
                    stmt.BindInt (1, sqlStatement.GetValueInt (column));
                    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
                    columnValue = stmt.GetValueText (0);
                    }
                Value jsonColumnValue;
                jsonColumnValue.SetString (columnValue.c_str (), doc.GetAllocator ());
                row_obj.AddMember (jsonColumnName, jsonColumnValue, doc.GetAllocator ());
                }
            }

        classArray.PushBack (row_obj, doc.GetAllocator ());
        }

    sqlStatement.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Muhammad Hassan                  03/2015
//---------------------------------------------------------------------------------------
void ECDbMapPersistenceTests::GetSchemaObjectById (ECDbR db, rapidjson::Document& doc, rapidjson::Value& schemaObject, int ecSchemaId, bool mapClasses)
    {
    BeSQLite::Statement sqlStatement;
    auto status = sqlStatement.Prepare (db, "SELECT Name, DisplayLabel, Description, NamespacePrefix, VersionMajor, VersionMinor FROM ec_Schema WHERE Id = ?");
    ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
    sqlStatement.BindInt (1, ecSchemaId);
    ASSERT_EQ (sqlStatement.Step (), DbResult::BE_SQLITE_ROW);

    for (int column = 0; column < 6; column++)
        {
        BeSQLite::DbValueType columnType = sqlStatement.GetColumnType (column);
        if (columnType == BeSQLite::DbValueType::IntegerVal || columnType == BeSQLite::DbValueType::TextVal)
            {
            Utf8String columnValue = sqlStatement.GetValueText (column);
            Value jsonColumnValue;
            jsonColumnValue.SetString (columnValue.c_str (), doc.GetAllocator ());
            Utf8String columnname = sqlStatement.GetColumnName (column);
            Value jsonColumnName;
            jsonColumnName.SetString (columnname.c_str (), doc.GetAllocator ());
            schemaObject.AddMember (jsonColumnName, jsonColumnValue, doc.GetAllocator ());
            }
        }

    if (mapClasses)
        {
        rapidjson::Value objects_array (rapidjson::kArrayType);
        GetClassesPerSchemaId (db, doc, objects_array, ecSchemaId, true);
        schemaObject.AddMember ("Classes", objects_array, doc.GetAllocator ());
        }

    sqlStatement.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Muhammad Hassan                  03/2015
//---------------------------------------------------------------------------------------
void ECDbMapPersistenceTests::GetColumnsPerTable (ECDbR db, rapidjson::Document & doc, rapidjson::Value& columnArray, int TableId)
    {
    BeSQLite::Statement sqlStatement;
    auto status = sqlStatement.Prepare (db, "SELECT ec_Column.[Id], ec_Column.[Name], ec_Column.[Type], ec_Column.KnownColumn FROM ec_Column WHERE ec_Column.TableId = ? ");
    ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
    sqlStatement.BindInt (1, TableId);

    while (sqlStatement.Step () == DbResult::BE_SQLITE_ROW)
        {
        rapidjson::Value columnObject (rapidjson::kObjectType);
        int columnId = sqlStatement.GetValueInt (0);
        for (int column = 1; column < 4; column++)
            {
            BeSQLite::DbValueType columnType = sqlStatement.GetColumnType (column);
            if (columnType == BeSQLite::DbValueType::IntegerVal || columnType == BeSQLite::DbValueType::TextVal)
                {
                Utf8String columnValue = sqlStatement.GetValueText (column);
                Value jsonColumnVale;
                jsonColumnVale.SetString (columnValue.c_str (), doc.GetAllocator ());
                Utf8String columnname = sqlStatement.GetColumnName (column);
                Value jsonColumnName;
                jsonColumnName.SetString (columnname.c_str (), doc.GetAllocator ());
                columnObject.AddMember (jsonColumnName, jsonColumnVale, doc.GetAllocator ());
                }
            }

        BeSQLite::Statement stmt;
        status = stmt.Prepare (db, "SELECT ec_Class.Name, ec_PropertyPath.AccessString FROM ec_PropertyMap JOIN ec_Class ON ec_Class.Id = ec_propertyMap.ClassMapId JOIN ec_PropertyPath ON ec_PropertyPath.Id = ec_PropertyMap.PropertyPathId WHERE ec_PropertyMap.ColumnId = ?");
        ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
        stmt.BindInt (1, columnId);
        rapidjson::Value Properties_obj (rapidjson::kObjectType);
        while (stmt.Step () != DbResult::BE_SQLITE_DONE)
            {
            Utf8String columnValue = stmt.GetValueText (0);
            columnValue.append (":");
            columnValue.append (stmt.GetValueText (1));
            Value jsonColumnValue;
            jsonColumnValue.SetString (columnValue.c_str (), doc.GetAllocator ());
            Properties_obj.AddMember ("AccessString", jsonColumnValue, doc.GetAllocator ());
            }
        columnObject.AddMember ("Properties", Properties_obj, doc.GetAllocator ());
        columnArray.PushBack (columnObject, doc.GetAllocator ());
        stmt.Finalize ();
        }

    sqlStatement.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Muhammad Hassan                  03/2015
//---------------------------------------------------------------------------------------
void ECDbMapPersistenceTests::GetTableColumnsPerTableId (ECDbR db, rapidjson::Document& doc, rapidjson::Value& tableObject, int TableId)
    {
    BeSQLite::Statement sqlStatement;
    auto status = sqlStatement.Prepare (db, "SELECT ec_Table.Name, ec_Table.[IsOwnedByECDb], ec_Table.[IsVirtual] FROM ec_Table Where ec_Table.[Id] = ?");
    ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
    sqlStatement.BindInt (1, TableId);
    ASSERT_EQ (sqlStatement.Step (), DbResult::BE_SQLITE_ROW);
    for (int column = 0; column < 3; column++)
        {
        BeSQLite::DbValueType columnType = sqlStatement.GetColumnType (column);
        if (columnType == BeSQLite::DbValueType::IntegerVal || columnType == BeSQLite::DbValueType::TextVal)
            {
            Utf8String columnValue = sqlStatement.GetValueText (column);
            Value jsonColumnValue;
            jsonColumnValue.SetString (columnValue.c_str (), doc.GetAllocator ());
            Utf8String columnname = sqlStatement.GetColumnName (column);
            Value jsonColumnName;
            jsonColumnName.SetString (columnname.c_str (), doc.GetAllocator ());
            tableObject.AddMember (jsonColumnName, jsonColumnValue, doc.GetAllocator ());
            }
        }

    rapidjson::Value columns_array (rapidjson::kArrayType);
    GetColumnsPerTable (db, doc, columns_array, TableId);
    tableObject.AddMember ("Columns", columns_array, doc.GetAllocator ());
    sqlStatement.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  03/2015
//---------------------------------------------------------------------------------------
TEST_F (ECDbMapPersistenceTests, DGNSchemaStructureMapPersistence)
    {
    ECDbTestProject::Initialize ();
    ECDb ECDbMapPersistence;
    auto stat = ECDbTestUtility::CreateECDb (ECDbMapPersistence, NULL, L"ECDbMappingPersistence.ecdb");
    ASSERT_EQ (stat, BE_SQLITE_OK);

    BeFileName ecSchemaSearchPath;
    BeTest::GetHost ().GetDgnPlatformAssetsDirectory (ecSchemaSearchPath);
    ecSchemaSearchPath.AppendToPath (L"ECSchemas\\Dgn");
    ECSchemaPtr ecSchema = nullptr;
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create ();
    ECSchemaReadContextPtr schemaContext = nullptr;
    ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, schemaContext, L"dgn.02.00.ecschema.xml", ecSchemaSearchPath.c_str ());
    ASSERT_TRUE (ecSchema != nullptr);
    ASSERT_EQ (SUCCESS, schemaCache->AddSchema (*ecSchema));
    ASSERT_EQ (SUCCESS, ECDbMapPersistence.Schemas ().ImportECSchemas (*schemaCache, ECDbSchemaManager::ImportOptions (false, false))) << "Schema import Failed";
    ECSchemaCP simpleCompany = ECDbMapPersistence.Schemas ().GetECSchema ("dgn");
    ASSERT_TRUE (simpleCompany != NULL);

    rapidjson::Document doc;
    doc.SetObject ();
    rapidjson::Value schemas_Array (rapidjson::kArrayType);
    BeSQLite::Statement sqlStatment;
    stat = sqlStatment.Prepare (ECDbMapPersistence, "SELECT Id FROM ec_Schema ORDER BY Id");
    ASSERT_EQ (stat, DbResult::BE_SQLITE_OK);
    while (sqlStatment.Step () == DbResult::BE_SQLITE_ROW)
        {
        rapidjson::Value row_obj (rapidjson::kObjectType);
        GetSchemaObjectById (ECDbMapPersistence, doc, row_obj, sqlStatment.GetValueInt (0), true);
        schemas_Array.PushBack (row_obj, doc.GetAllocator ());
        }
    doc.AddMember ("Schemas", schemas_Array, doc.GetAllocator ());

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer (buffer);
    doc.Accept (writer);
    Utf8String actualJson = buffer.GetString ();

    BeFileName filesPath;
    BeTest::GetHost ().GetDocumentsRoot (filesPath);
    filesPath.AppendToPath (L"DgnDb");
    BeFileName benchmarkPath = filesPath;
    benchmarkPath.AppendToPath (L"dgnbenchmark.json");
    Utf8String dgnBenchmarkJson;

    if (ReadJsonFromFile (dgnBenchmarkJson, benchmarkPath))
        {
        if (strcmp (dgnBenchmarkJson.c_str (), actualJson.c_str ()) != 0)
            {
            EXPECT_TRUE (false);
            BeFileName newjson = filesPath;
            newjson.AppendToPath (L"Actualjson.json");
            WriteJsonDocumentToFile (buffer, newjson.GetName ());
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  03/2015
//---------------------------------------------------------------------------------------
TEST_F (ECDbMapPersistenceTests, DGNSchemaTableColumnPropertyMapPersistence)
    {
    ECDbTestProject::Initialize ();
    ECDb ECDbMapPersistence;
    auto stat = ECDbTestUtility::CreateECDb (ECDbMapPersistence, NULL, L"ECDbMappingPersistence.ecdb");
    ASSERT_EQ (stat, BE_SQLITE_OK);

    BeFileName ecSchemaSearchPath;
    BeTest::GetHost ().GetDgnPlatformAssetsDirectory (ecSchemaSearchPath);
    ecSchemaSearchPath.AppendToPath (L"ECSchemas\\Dgn");
    ECSchemaPtr ecSchema = nullptr;
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create ();
    ECSchemaReadContextPtr schemaContext = nullptr;
    ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, schemaContext, L"dgn.02.00.ecschema.xml", ecSchemaSearchPath.c_str ());
    ASSERT_TRUE (ecSchema != nullptr);
    ASSERT_EQ (SUCCESS, schemaCache->AddSchema (*ecSchema));
    ASSERT_EQ (SUCCESS, ECDbMapPersistence.Schemas ().ImportECSchemas (*schemaCache, ECDbSchemaManager::ImportOptions (false, false))) << "Schema import Failed";
    ECSchemaCP simpleCompany = ECDbMapPersistence.Schemas ().GetECSchema ("dgn");
    ASSERT_TRUE (simpleCompany != NULL);

    rapidjson::Document doc;
    rapidjson::Value table_array (rapidjson::kArrayType);
    doc.SetObject ();
    BeSQLite::Statement sqlStatment;
    auto sql = "SELECT Id FROM ec_Table ORDER BY Id";
    stat = sqlStatment.Prepare (ECDbMapPersistence, sql);
    ASSERT_EQ (stat, DbResult::BE_SQLITE_OK);

    while (sqlStatment.Step () == DbResult::BE_SQLITE_ROW)
        {
        rapidjson::Value row_obj (rapidjson::kObjectType);
        GetTableColumnsPerTableId (ECDbMapPersistence, doc, row_obj, sqlStatment.GetValueInt (0));
        table_array.PushBack (row_obj, doc.GetAllocator ());
        }
    doc.AddMember ("Tables", table_array, doc.GetAllocator ());

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer (buffer);
    doc.Accept (writer);
    Utf8String actualJson = buffer.GetString ();

    BeFileName filesPath;
    BeTest::GetHost ().GetDocumentsRoot (filesPath);
    filesPath.AppendToPath (L"DgnDb");
    BeFileName benchmarkPath = filesPath;
    benchmarkPath.AppendToPath (L"TableColumnPropertyMapppingBenchmark.json");
    Utf8String dgnBenchmarkJson;
    if (ReadJsonFromFile (dgnBenchmarkJson, benchmarkPath))
        {
        if (strcmp (dgnBenchmarkJson.c_str (), actualJson.c_str ()) != 0)
            {
            EXPECT_TRUE (false);
            BeFileName newjson = filesPath;
            newjson.AppendToPath (L"Actualjson.json");
            WriteJsonDocumentToFile (buffer, newjson.GetName ());
            }
        }
    }

END_ECDBUNITTESTS_NAMESPACE