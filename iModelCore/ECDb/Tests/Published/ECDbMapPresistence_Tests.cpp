/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbMapPresistence_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"
#include "ECInstanceAdaptersTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE
using namespace rapidjson;

static Byte s_Utf8BOM[] = { 0xef, 0xbb, 0xbf };
struct ECDbMapPersistenceTests : public ::testing::Test
    {
    static bool ReadStringFromUtf8File (Utf8String& strValue, WCharCP path);
    static bool ReadJsonFromFile (Utf8String& jsonValue, WCharCP path);
    static void writeJsonDocumentToFile (rapidjson::StringBuffer& buffer, WCharCP path);
    static void getClassesPerSchemaId (ECDbR db, rapidjson::Document& doc, rapidjson::Value& classess_array, int ecSchemaId, bool mapProperties);
    static void getPropertiesPerClass (ECDbR db, rapidjson::Document& doc, rapidjson::Value& classess_array, int ecClassId);
    static void getRelationshipConstraintClasses (ECDbR db, rapidjson::Document& doc, rapidjson::Value& constraintclass_obj, int ecClassId);
    static void getBaseClass (ECDbR db, rapidjson::Document& doc, rapidjson::Value& baseClass_obj, int ecClassId);
    static void getSchemaObjectById (ECDbR db, rapidjson::Document& doc, rapidjson::Value& schema_obj, int ecSchemaId, bool mapClasses);
    static void getJsonBasedOnTableName (ECDbR db, rapidjson::Document& doc, rapidjson::Value& schema_obj, Utf8String tableName);
    static void getKeyProperty (ECDbR db, rapidjson::Document& doc, rapidjson::Value& constraint_array, int ecClassId, int relationecClassId);
    };

//+---------------+---------------+---------------+---------------+---------------+------
//                                               Muhammad Hassan                  03/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbMapPersistenceTests::ReadStringFromUtf8File (Utf8String& strValue, WCharCP path)
    {
    strValue = "";

    BeFile file;
    BeFileStatus fileStatus = file.Open (path, BeFileAccess::Read);
    if ((BeFileStatus::Success != fileStatus))
        return false;

    uint64_t rawSize;
    fileStatus = file.GetSize (rawSize);
    if (!(BeFileStatus::Success == fileStatus && rawSize <= UINT32_MAX))
        {
        file.Close ();
        return false;
        }
    uint32_t sizeToRead = (uint32_t)rawSize;

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
void ECDbMapPersistenceTests::writeJsonDocumentToFile (rapidjson::StringBuffer& buffer, WCharCP path)
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
void ECDbMapPersistenceTests::getBaseClass (ECDbR db, rapidjson::Document& doc, rapidjson::Value& baseClass_obj, int ecClassId)
    {
    BeSQLite::Statement sqlstmt;
    auto sql = "Select Name from ec_Class where ECClassId = (select BaseECClassId from ec_BaseClass where ECClassId=?)";
    auto status = sqlstmt.Prepare (db, sql);
    ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
    sqlstmt.BindInt (1, ecClassId);
    if (sqlstmt.Step () == DbResult::BE_SQLITE_ROW)
        {
        Utf8String baseClassName = sqlstmt.GetValueText (0);
        Value value;
        value.SetString (baseClassName.c_str (), doc.GetAllocator ());
        baseClass_obj.AddMember ("BaseClass", value, doc.GetAllocator ());
        }
    }
void ECDbMapPersistenceTests::getKeyProperty (ECDbR db, rapidjson::Document& doc, rapidjson::Value& constraint_array, int ecClassId, int relationecClassId)
    {
    BeSQLite::Statement stmt;
    auto sql2 = "select ec_Property.[Name] from ec_Property where ECPropertyId IN (select ec_RelationshipConstraintClassProperty.[RelationECPropertyId] from [ec_RelationshipConstraintClassProperty] where ECClassId = ? and RelationECClassId = ?)";
    auto status = stmt.Prepare (db, sql2);
    ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
    stmt.BindInt (1, ecClassId);
    stmt.BindInt (2, relationecClassId);
    rapidjson::Value obj (rapidjson::kObjectType);
    rapidjson::Value keyProperty_array (rapidjson::kArrayType);
    while (stmt.Step () == DbResult::BE_SQLITE_ROW)
        {
        Utf8String columnValue = stmt.GetValueText (0);
        Value value;
        value.SetString (columnValue.c_str (), doc.GetAllocator ());
        keyProperty_array.PushBack (value, doc.GetAllocator ());
        }
    obj.AddMember ("KeyProperties", keyProperty_array, doc.GetAllocator ());
    constraint_array.PushBack (obj, doc.GetAllocator ());
    }
void ECDbMapPersistenceTests::getRelationshipConstraintClasses (ECDbR db, rapidjson::Document& doc, rapidjson::Value& constraintclass_obj, int ecClassId)
    {
    rapidjson::Value sourceClasses_array (rapidjson::kArrayType);
    rapidjson::Value targetClasses_array (rapidjson::kArrayType);
    BeSQLite::Statement sqlStatement;
    auto sql = "SELECT ECRelationshipEnd, RelationECClassId FROM ec_RelationshipConstraintClass WHERE ECClassId=?";
    auto status = sqlStatement.Prepare (db, sql);
    ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
    sqlStatement.BindInt (1, ecClassId);
    while (sqlStatement.Step () == DbResult::BE_SQLITE_ROW)
        {
        BeSQLite::Statement sqlstmt;
        int relationECClassId = sqlStatement.GetValueInt (1);
        auto sql1 = "SELECT Name FROM ec_Class WHERE ECClassId=?";
        status = sqlstmt.Prepare (db, sql1);
        ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
        sqlstmt.BindInt (1, relationECClassId);
        if (sqlstmt.Step () == DbResult::BE_SQLITE_ROW)
            {
            Utf8String columnValue = sqlstmt.GetValueText (0);
            Value value;
            value.SetString (columnValue.c_str (), doc.GetAllocator ());
            if (sqlStatement.GetValueInt (0) == 1)
                {
                sourceClasses_array.PushBack (value, doc.GetAllocator ());
                getKeyProperty (db, doc, sourceClasses_array, ecClassId, relationECClassId);
                }
            else
                {
                targetClasses_array.PushBack (value, doc.GetAllocator ());
                getKeyProperty (db, doc, targetClasses_array, ecClassId, relationECClassId);
                }
            }
        }
    constraintclass_obj.AddMember ("Source Classes", sourceClasses_array, doc.GetAllocator ());
    constraintclass_obj.AddMember ("Target Classes", targetClasses_array, doc.GetAllocator ());
    }
void ECDbMapPersistenceTests::getClassesPerSchemaId (ECDbR db, rapidjson::Document& doc, rapidjson::Value& classess_array, int ecSchemaId, bool mapProperties)
    {
    Utf8String classPath = "";
    BeSQLite::Statement sqlStatement;

    auto sql = "SELECT ECClassId,Name, DisplayLabel, Description, IsDomainClass, IsStruct, IsCustomAttribute, RelationStrength, RelationStrengthDirection, IsRelationship  FROM ec_Class where ECschemaId = ?";
    auto status = sqlStatement.Prepare (db, sql);
    ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
    sqlStatement.BindInt (1, ecSchemaId);

    while (sqlStatement.Step () == DbResult::BE_SQLITE_ROW)
        {
        Utf8String IsRelationship;
        bool flag = false;
        int columncount;
        int ecClassId = 0;
        Utf8String RelationStrength = "";
        Utf8String RelationStrengthDirection = "";
        ecClassId = sqlStatement.GetValueInt (0);
        columncount = sqlStatement.GetColumnCount ();
        rapidjson::Value row_obj (rapidjson::kObjectType);
        for (int column = 0; column < columncount; column++)
            {
            BeSQLite::DbValueType columnType = sqlStatement.GetColumnType (column);
            Utf8String columnValue = "";
            if (columnType == BeSQLite::DbValueType::IntegerVal || columnType == BeSQLite::DbValueType::TextVal)
                {
                Utf8String columnname = sqlStatement.GetColumnName (column);
                Value temp;
                temp.SetString (columnname.c_str (), doc.GetAllocator ());
                if (columnname == "ECClassId")
                    {
                    }
                else if (columnname == "IsRelationship")
                    {
                    if (sqlStatement.GetValueInt (column) == 1)
                        {
                        rapidjson::Value constraintclass_obj (rapidjson::kObjectType);
                        Value tmp;
                        tmp.SetString (RelationStrength.c_str (), doc.GetAllocator ());
                        constraintclass_obj.AddMember ("RelationStrength", tmp, doc.GetAllocator ());
                        tmp.SetString (RelationStrengthDirection.c_str (), doc.GetAllocator ());
                        constraintclass_obj.AddMember ("RelationStrengthDirection", tmp, doc.GetAllocator ());
                        getRelationshipConstraintClasses (db, doc, constraintclass_obj, ecClassId);
                        row_obj.AddMember (temp, constraintclass_obj, doc.GetAllocator ());
                        }
                    }
                else if (columnname == "RelationStrength")
                    {
                    RelationStrength = sqlStatement.GetValueText (column);
                    }
                else if (columnname == "RelationStrengthDirection")
                    {
                    RelationStrengthDirection = sqlStatement.GetValueText (column);
                    }
                else
                    {
                    columnValue = sqlStatement.GetValueText (column);
                    Value tmp;
                    tmp.SetString (columnValue.c_str (), doc.GetAllocator ());
                    row_obj.AddMember (temp, tmp, doc.GetAllocator ());
                    }
                }
            }
        getBaseClass (db, doc, row_obj, ecClassId);
        if (!flag && mapProperties)
            {
            rapidjson::Value properties_array (rapidjson::kArrayType);
            getPropertiesPerClass (db, doc, properties_array, ecClassId);
            row_obj.AddMember ("Properties", properties_array, doc.GetAllocator ());
            }
        classess_array.PushBack (row_obj, doc.GetAllocator ());
        }
    sqlStatement.Finalize ();
    }
void ECDbMapPersistenceTests::getPropertiesPerClass (ECDbR db, rapidjson::Document& doc, rapidjson::Value& classess_array, int ecClassId)
    {
    BeSQLite::Statement sqlStatement;

    auto sql = "SELECT Name, DisplayLabel, Description, IsArray, TypeCustom, TypeECPrimitive, TypeGeometry, TypeECStruct, IsReadOnly, MinOccurs, MaxOccurs FROM ec_Property where ECClassId = ?";
    auto status = sqlStatement.Prepare (db, sql);
    sqlStatement.BindInt (1, ecClassId);
    ASSERT_EQ (status, DbResult::BE_SQLITE_OK);

    while (sqlStatement.Step () == DbResult::BE_SQLITE_ROW)
        {
        int columncount;
        columncount = sqlStatement.GetColumnCount ();
        rapidjson::Value row_obj (rapidjson::kObjectType);
        for (int column = 0; column < columncount; column++)
            {
            BeSQLite::DbValueType columnType = sqlStatement.GetColumnType (column);
            if (columnType == BeSQLite::DbValueType::IntegerVal || columnType == BeSQLite::DbValueType::TextVal)
                {
                Utf8String columnValue = "";
                Utf8String columnname = sqlStatement.GetColumnName (column);
                Value temp;
                temp.SetString (columnname.c_str (), doc.GetAllocator ());
                if (columnname != "TypeECStruct")
                    {
                    columnValue = sqlStatement.GetValueText (column);
                    }
                else
                    {
                    BeSQLite::Statement stmt;
                    auto sql1 = "SELECT Name FROM ec_Class WHERE ECClassId = ?";
                    status = stmt.Prepare (db, sql1);
                    ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
                    stmt.BindInt (1, sqlStatement.GetValueInt (column));
                    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
                    columnValue = stmt.GetValueText (0);
                    }
                Value tmp;
                tmp.SetString (columnValue.c_str (), doc.GetAllocator ());
                row_obj.AddMember (temp, tmp, doc.GetAllocator ());
                }
            }
        classess_array.PushBack (row_obj, doc.GetAllocator ());
        }
    sqlStatement.Finalize ();
    }
void ECDbMapPersistenceTests::getSchemaObjectById (ECDbR db, rapidjson::Document& doc, rapidjson::Value& schema_obj, int ecSchemaId, bool mapClasses)
    {
    BeSQLite::Statement sqlStatement;
    auto sql = "SELECT Name, DisplayLabel, Description, NamespacePrefix, VersionMajor, VersionMinor FROM ec_Schema WHERE ECSchemaId = ?";
    auto status = sqlStatement.Prepare (db, sql);
    ASSERT_EQ (status, DbResult::BE_SQLITE_OK);
    sqlStatement.BindInt (1, ecSchemaId);
    ASSERT_EQ (sqlStatement.Step (), DbResult::BE_SQLITE_ROW);
    bool flag = false;
    int columncount;
    columncount = sqlStatement.GetColumnCount ();
    for (int column = 0; column < columncount; column++)
        {
        BeSQLite::DbValueType columnType = sqlStatement.GetColumnType (column);
        if (columnType == BeSQLite::DbValueType::IntegerVal || columnType == BeSQLite::DbValueType::TextVal)
            {
            Utf8String columnValue = sqlStatement.GetValueText (column);
            Value tmp;
            tmp.SetString (columnValue.c_str (), doc.GetAllocator ());
            Utf8String columnname = sqlStatement.GetColumnName (column);
            Value temp;
            temp.SetString (columnname.c_str (), doc.GetAllocator ());
            schema_obj.AddMember (temp, tmp, doc.GetAllocator ());
            }
        }
    if (!flag && mapClasses)
        {
        rapidjson::Value objects_array (rapidjson::kArrayType);
        getClassesPerSchemaId (db, doc, objects_array, ecSchemaId, true);
        schema_obj.AddMember ("Classes", objects_array, doc.GetAllocator ());
        flag = true;
        }
    sqlStatement.Finalize ();
    }
//+---------------+---------------+---------------+---------------+---------------+------
//                                               Muhammad Hassan                  03/2015
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbMapPersistenceTests, DGNSchemaStructureMappingPersistence)
    {
    ECDbTestProject::Initialize ();
    ECDb ECDbMappingPersistence;
    auto stat = ECDbTestUtility::CreateECDb (ECDbMappingPersistence, NULL, L"ECDbMappingPersistence.ecdb");
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
    ASSERT_EQ (SUCCESS, ECDbMappingPersistence.GetSchemaManager ().ImportECSchemas (*schemaCache, ECDbSchemaManager::ImportOptions (false, false))) << "Schema import Failed";
    ECSchemaCP simpleCompany = ECDbMappingPersistence.GetSchemaManager ().GetECSchema ("dgn");
    ASSERT_TRUE (simpleCompany != NULL);

    rapidjson::Document doc;
    doc.SetObject ();
    rapidjson::Value schemas_Array (rapidjson::kArrayType);
    BeSQLite::Statement sqlStatment;
    auto sql = "SELECT ECSchemaId FROM ec_Schema order by ECSchemaId";
    stat = sqlStatment.Prepare (ECDbMappingPersistence, sql);
    ASSERT_EQ (stat, DbResult::BE_SQLITE_OK);
    while (sqlStatment.Step () == DbResult::BE_SQLITE_ROW)
        {
        rapidjson::Value row_obj (rapidjson::kObjectType);
        getSchemaObjectById (ECDbMappingPersistence, doc, row_obj, sqlStatment.GetValueInt (0), true);
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
            writeJsonDocumentToFile (buffer, newjson.GetName ());
            }
        }
    }
TEST_F (ECDbMapPersistenceTests, MappingStrategyTest)
    {
    ECDbTestProject::Initialize ();
    ECDb ECDbMappingPersistence;
    auto stat = ECDbTestUtility::CreateECDb (ECDbMappingPersistence, NULL, L"MappingStrategyDb.ecdb");
    ASSERT_EQ (stat, BE_SQLITE_OK);

    ECSchemaPtr ecSchema = nullptr;
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create ();
    ECSchemaReadContextPtr schemaContext = nullptr;
    ECDbTestUtility::ReadECSchemaFromDisk (ecSchema, schemaContext, L"TablePerHierarchyPrecedence.01.00.ecschema.xml", nullptr);

    ASSERT_TRUE (ecSchema != nullptr);
    ASSERT_EQ (SUCCESS, schemaCache->AddSchema (*ecSchema));
    ASSERT_EQ (SUCCESS, ECDbMappingPersistence.GetSchemaManager ().ImportECSchemas (*schemaCache, ECDbSchemaManager::ImportOptions (false, false))) << "Schema import Failed \n";
    ECSchemaCP tablePerHierarchySchema = ECDbMappingPersistence.GetSchemaManager ().GetECSchema ("dgn");
    ASSERT_TRUE (tablePerHierarchySchema != NULL);
    }

END_ECDBUNITTESTS_NAMESPACE