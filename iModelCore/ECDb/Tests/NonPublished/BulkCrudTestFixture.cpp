/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BulkCrudTestFixture.h"
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeTextFile.h>
#include <BeSQLite/BeBriefcaseBasedIdSequence.h>
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//*****************************************************************************************
// BulkCrudTestFixture
//*****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
void BulkCrudTestFixture::AssertInsert(TestDataset& testData)
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(testData.GetDb(), "SELECT rowid, ClassId, propvalues FROM data")) << testData.GetDb().GetLastError().c_str();

    Statement updateGenIdStmt;
    ASSERT_EQ(BE_SQLITE_OK, updateGenIdStmt.Prepare(testData.GetDb(), "UPDATE data SET generatedecinstanceid=? WHERE rowid=?")) << testData.GetDb().GetLastError().c_str();

    std::map<ECClassId, std::unique_ptr<JsonInserter>> inserterCache;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        const int64_t rowid = stmt.GetValueInt64(0);
        const ECClassId classId = stmt.GetValueId<ECClassId>(1);
        
        JsonInserter* inserter = nullptr;
        auto it = inserterCache.find(classId);
        if (it != inserterCache.end())
            inserter = it->second.get();
        else
            {
            ECClassCP ecClass = m_ecdb.Schemas().GetClass(classId);
            ASSERT_TRUE(ecClass != nullptr);
            std::unique_ptr<JsonInserter> inserterPtr(new JsonInserter(m_ecdb, *ecClass, nullptr));
            inserter = inserterPtr.get();
            inserterCache[classId] = std::move(inserterPtr);
            }

        ASSERT_TRUE(inserter->IsValid()) << "Failed to get inserter for ECClass " << classId.ToString().c_str();

        rapidjson::Document propValuesJson(rapidjson::kObjectType);
        if (!stmt.IsColumnNull(2))
            ASSERT_EQ(SUCCESS, testData.ParseJson(propValuesJson, stmt.GetValueText(2))) << "Row id: " << rowid;

        ASSERT_TRUE(propValuesJson.IsObject());
        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_OK, inserter->Insert(key, propValuesJson)) << "Row id: " << rowid;

        ASSERT_EQ(BE_SQLITE_OK, updateGenIdStmt.BindId(1, key.GetInstanceId())) << "Update data with generated ECInstanceId for row id: " << rowid << " " << testData.GetDb().GetLastError().c_str();
        ASSERT_EQ(BE_SQLITE_OK, updateGenIdStmt.BindInt64(2, rowid)) << "Update data with generated ECInstanceId for row id: " << rowid << " " << testData.GetDb().GetLastError().c_str();
        ASSERT_EQ(BE_SQLITE_DONE, updateGenIdStmt.Step()) << "Update data with generated ECInstanceId for row id: " << rowid << " " << testData.GetDb().GetLastError().c_str();
        ASSERT_EQ(1, testData.GetDb().GetModifiedRowCount()) << "Update data with generated ECInstanceId for row id: " << rowid << " " << testData.GetDb().GetLastError().c_str();
        updateGenIdStmt.Reset();
        updateGenIdStmt.ClearBindings();
        }
    }

//*****************************************************************************************
// BulkCrudTestFixture::TestDataset
//*****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
BentleyStatus BulkCrudTestFixture::TestDataset::Populate(ECDbCR ecdb)
    {
    if (!ecdb.IsDbOpen())
        return ERROR;

    const size_t instanceCountPerClass = 2;
    srand((uint32_t) (BeTimeUtilities::QueryMillisecondsCounter() & 0xFFFFFFFF));

    if (SUCCESS != Setup(ecdb))
        return ERROR;

    //select all non-abstract entity classes that are mapped, but not mapped to ExistingTable
    //and create test data for them (We could use ECSQL against Meta schema, but class map is not exposed there
    Statement classesStmt;
    if (BE_SQLITE_OK != classesStmt.Prepare(ecdb,
                                                    R"sql(SELECT c.Id FROM ec_Class c, ec_ClassMap cm WHERE c.Id=cm.ClassId AND
                                 c.Type=0 AND c.Modifier<>1 AND cm.MapStrategy NOT IN (0,3))sql"))
        return ERROR;


    while (BE_SQLITE_ROW == classesStmt.Step())
        {
        ECClassCP ecClass = ecdb.Schemas().GetClass(classesStmt.GetValueId<ECClassId>(0));
        if (ecClass == nullptr || !ecClass->IsEntityClass())
            {
            BeAssert(false);
            return ERROR;
            }

        for (size_t i = 0; i < instanceCountPerClass; i++)
            {
            if (SUCCESS != InsertTestInstance(ecdb, *ecClass, false))
                return ERROR;

            if (SUCCESS != InsertTestInstance(ecdb, *ecClass, true))
                return ERROR;
            }
        }

    classesStmt.Finalize();

    //select all non-abstract link table classes and create test data for them
 /*   ECSqlStatement relsStmt;
    if (ECSqlStatus::Success == relsStmt.Prepare(ecdb,
                                                    R"sql(SELECT Id FROM meta.ECClassDef WHERE Type=1 AND Modifier<>1)sql"))
        return ERROR;


    while (BE_SQLITE_ROW == relsStmt.Step())
        {
        ECClassCP ecClass = ecdb.Schemas().GetClass(relsStmt.GetValueId<ECClassId>(0));
        if (ecClass == nullptr || !ecClass->IsRelationshipClass())
            {
            BeAssert(false);
            return ERROR;
            }

        if (SUCCESS != CreateTestInstance(ecdb, dataJson, *ecClass, jsonAllocator))
            return ERROR;
        }

    relsStmt.Finalize();
    */

    return BE_SQLITE_OK == m_dataDb.SaveChanges() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus BulkCrudTestFixture::TestDataset::ParseJson(rapidjson::Document& json, Utf8CP jsonStr)
    {
    if (json.Parse<0>(jsonStr).HasParseError())
        {
        BeAssert(false && "Could not parse JSON string.");
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
BentleyStatus BulkCrudTestFixture::TestDataset::InsertTestInstance(ECDbCR ecdb, ECN::ECClassCR ecClass, bool ignoreNullableProps)
    {
    rapidjson::Document propValues(rapidjson::kObjectType);
    if (SUCCESS != GeneratePropValuePairs(ecdb, propValues, ecClass.GetProperties(), ignoreNullableProps, propValues.GetAllocator()))
        return ERROR;

    CachedStatementPtr stmt = m_dataDb.GetCachedStatement("INSERT INTO data(classid,propvalues) VALUES(?,?)");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(1, ecClass.GetId());
    rapidjson::StringBuffer jsonStrBuf;
    if (!propValues.GetObject().ObjectEmpty())
        {
        rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStrBuf);
        propValues.Accept(writer);
        if (BE_SQLITE_OK != stmt->BindText(2, jsonStrBuf.GetString(), Statement::MakeCopy::No))
            return ERROR;
        }

    return stmt->Step() == BE_SQLITE_DONE ? SUCCESS : ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus BulkCrudTestFixture::TestDataset::GeneratePropValuePairs(ECDbCR ecdb, rapidjson::Value& json, ECPropertyIterableCR properties, bool ignoreNullableProps, rapidjson::MemoryPoolAllocator<>& jsonAllocator)
    {
    for (ECPropertyCP prop : properties)
        {
        if (ignoreNullableProps && IsNullableProperty(*prop))
            continue;

        if (SUCCESS != GeneratePropValuePair(ecdb, json, *prop, ignoreNullableProps, jsonAllocator))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus BulkCrudTestFixture::TestDataset::GeneratePropValuePair(ECDbCR ecdb, rapidjson::Value& json, ECPropertyCR prop, bool ignoreNullableMemberProps, rapidjson::MemoryPoolAllocator<>& jsonAllocator)
    {
    BeAssert(json.IsObject());
    rapidjson::GenericStringRef<Utf8Char> memberName(prop.GetName().c_str(), (rapidjson::SizeType) prop.GetName().size());
    //Caution: return value is struct again, not the inserted member
    json.AddMember(memberName, rapidjson::Value().Move(), jsonAllocator);
    rapidjson::Value& valueJson = json[memberName.s];
    if (prop.GetIsPrimitive())
        return GeneratePrimitiveValue(valueJson, prop.GetAsPrimitiveProperty()->GetType(), jsonAllocator);

    if (prop.GetIsPrimitiveArray())
        {
        const PrimitiveType arrayType = prop.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
        size_t arraySize = (size_t) (rand() % 5) + 1;
        valueJson.SetArray();
        for (size_t i = 0; i < arraySize; i++)
            {
            valueJson.PushBack(rapidjson::Value().Move(), jsonAllocator);
            rapidjson::Value& newArrayElementJson = valueJson[(rapidjson::SizeType) i];
            if (SUCCESS != GeneratePrimitiveValue(newArrayElementJson, arrayType, jsonAllocator))
                return ERROR;
            }

        return SUCCESS;
        }

    if (prop.GetIsStruct())
        {
        valueJson.SetObject();
        ECClassCR structType = prop.GetAsStructProperty()->GetType();
        return GeneratePropValuePairs(ecdb, valueJson, structType.GetProperties(), ignoreNullableMemberProps, jsonAllocator);
        }

    if (prop.GetIsStructArray())
        {
        ECClassCR structType = prop.GetAsStructArrayProperty()->GetStructElementType();
        size_t arraySize = (size_t) (rand() % 5) + 1;
        valueJson.SetArray();
        for (size_t i = 0; i < arraySize; i++)
            {
            valueJson.PushBack(rapidjson::Value().Move(), jsonAllocator);
            rapidjson::Value& newArrayElementJson = valueJson[(rapidjson::SizeType) i];
            if (SUCCESS != GeneratePropValuePairs(ecdb, newArrayElementJson, structType.GetProperties(), ignoreNullableMemberProps, jsonAllocator))
                return ERROR;
            }

        return SUCCESS;
        }

    if (prop.GetIsNavigation())
        {
        /*std::function<ECRelationshipClassCP(ECDbCR, ECRelationshipClassCR)> getConcreteSubRel;
        getConcreteSubRel = [&getConcreteSubRel] (ECDbCR ecdb, ECRelationshipClassCR baseRel)
            {
            if (baseRel.GetClassModifier() != ECClassModifier::Abstract)
                return &baseRel;

            ECRelationshipClassCP concreteSubClass = nullptr;
            for (ECClassCP subclass : ecdb.Schemas().GetDerivedClasses(baseRel))
                {
                BeAssert(subclass->IsRelationshipClass());
                concreteSubClass = getConcreteSubRel(ecdb, *subclass->GetRelationshipClassCP());
                if (concreteSubClass != nullptr)
                    return concreteSubClass;
                }

            return concreteSubClass;
            };
            */
        NavigationECPropertyCP navProp = prop.GetAsNavigationProperty();
        ECRelationshipClassCP rel = navProp->GetRelationshipClass();
        if (rel == nullptr)
            {
            BeAssert(false && "Nav prop without concrete relationship");
            return ERROR;
            }


        ECRelationshipConstraintCR navTargetConstraint = navProp->GetDirection() == ECRelatedInstanceDirection::Backward ? rel->GetSource() : rel->GetTarget();
        ECClassCP navTargetClass = nullptr;
        for (ECClassCP constraintClass : navTargetConstraint.GetConstraintClasses())
            {
            if (constraintClass->GetClassModifier() != ECClassModifier::Abstract)
                navTargetClass = constraintClass;
            }

     /*   if (navTargetClass == nullptr && !navTargetConstraint.GetIsPolymorphic())
            {
            BeAssert(false && "Found nav prop which doesn't have non-abstract navigation target class");
            return ERROR;
            }
            */
        return SUCCESS;
        }

    BeAssert(false);
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus BulkCrudTestFixture::TestDataset::GeneratePrimitiveValue(rapidjson::Value& json, ECN::PrimitiveType primType, rapidjson::MemoryPoolAllocator<>& jsonAllocator)
    {
    switch (primType)
        {
            case PRIMITIVETYPE_Binary:
            {
            Byte blob[128];
            for (size_t i = 0; i < 128; i++)
                {
                blob[i] = (Byte) (rand() & 0xFF);
                }

            return ECJsonUtilities::BinaryToJson(json, blob, sizeof(blob), jsonAllocator);
            }

            case PRIMITIVETYPE_Boolean:
                json.SetBool(rand() % 2 == 0);
                return SUCCESS;

            case PRIMITIVETYPE_DateTime:
            {
            ECJsonUtilities::DateTimeToJson(json, DateTime::GetCurrentTimeUtc(), jsonAllocator);
            return SUCCESS;
            }

            case PRIMITIVETYPE_Double:
                json.SetDouble(rand() * 1.4);
                return SUCCESS;

            case PRIMITIVETYPE_IGeometry:
            {
            IGeometryPtr geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(rand() * 1.0, rand() * 2.0, rand() * 1.0,
                                                                                               rand() * 1.0, rand() * 2.0, rand() * 1.0)));

            return ECJsonUtilities::IGeometryToJson(json, *geom, jsonAllocator);
            }
            case PRIMITIVETYPE_Integer:
                json.SetInt(rand());
                return SUCCESS;

            case PRIMITIVETYPE_Long:
                json.SetInt64(rand() * 1111);
                return SUCCESS;

            case PRIMITIVETYPE_Point2d:
                return ECJsonUtilities::Point2dToJson(json, DPoint2d::From(rand() * 1.4, rand() * (-1.0)), jsonAllocator);

            case PRIMITIVETYPE_Point3d:
                return ECJsonUtilities::Point3dToJson(json, DPoint3d::From(rand() * 1.4, rand() * (-1.0), rand() * 1.0), jsonAllocator);

            case PRIMITIVETYPE_String:
            {
            Utf8String str;
            str.Sprintf("How random the number %d is...", rand());
            json.SetString(str.c_str(), (rapidjson::SizeType) str.size(), jsonAllocator);
            return SUCCESS;
            }

            default:
                BeAssert(false);
                return ERROR;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
//static
bool BulkCrudTestFixture::TestDataset::IsNullableProperty(ECN::ECPropertyCR prop)
    {
    if (prop.GetIsNavigation())
        {
        NavigationECPropertyCP navProp = prop.GetAsNavigationProperty();
        ECRelationshipConstraintCR navTargetConstraint = navProp->GetDirection() == ECRelatedInstanceDirection::Backward ? navProp->GetRelationshipClass()->GetSource() : navProp->GetRelationshipClass()->GetTarget();
        return navTargetConstraint.GetMultiplicity().GetLowerLimit() == 0;
        }

    IECInstancePtr propMapCA = prop.GetCustomAttributeLocal("ECDbMap", "PropertyMap");
    if (propMapCA == nullptr)
        return true;

    BeAssert(propMapCA->GetClass().GetPropertyP("IsNullable") != nullptr && "PropertyMap CA class has changed. It is expected to have a property called 'IsNullable'.");
    ECValue isNullableVal;
    propMapCA->GetValue(isNullableVal, "IsNullable");
    return isNullableVal.IsNull() || isNullableVal.GetBoolean();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
BentleyStatus BulkCrudTestFixture::TestDataset::Setup(ECDbCR testECDb)
    {
    BeFileName dataDbPath(testECDb.GetDbFileName());
    dataDbPath.AppendExtension(L"testdata.db");
    if (dataDbPath.DoesPathExist())
        {
        if (BeFileNameStatus::Success != dataDbPath.BeDeleteFile())
            return ERROR;
        }

    if (BE_SQLITE_OK != m_dataDb.CreateNewDb(dataDbPath))
        {
        BeAssert(false && "Failed to create test data SQLite DB.");
        return ERROR;
        }

    if (BE_SQLITE_OK != m_dataDb.ExecuteSql(
        R"sql(
            CREATE TABLE data(classid INTEGER NOT NULL, generatedecinstanceid INTEGER, propvalues TEXT);
            CREATE INDEX ix_data_generatedecinstanceid ON data(generatedecinstanceid);
            )sql"))
        {
        BeAssert(false && "Failed to create data table.");
        return ERROR;
        }

    return BE_SQLITE_OK == m_dataDb.SaveChanges() ? SUCCESS : ERROR;
    }


//*****************************************************************************************
// BulkBisDomainCrudTestFixture
//*****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
BentleyStatus BulkBisDomainCrudTestFixture::CreateFakeBimFile(Utf8CP fileName, BeFileNameCR bisSchemaFolder)
    {
    if (BE_SQLITE_OK != SetupECDb(fileName))
        return ERROR;

    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    {
    Db db;
    if (BE_SQLITE_OK != db.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite)))
        return ERROR;

    uint64_t initialId = UINT64_C(500);
    if (BE_SQLITE_DONE != db.SaveBriefcaseLocalValue("ec_classidsequence", initialId))
        return ERROR;

    //save initial id for fileformat compatibility test
    if (BE_SQLITE_OK != db.SaveProperty(PropertySpec("InitialClassId_BisCore", "ECDb_FileFormatCompatiblity_Test"), &initialId, sizeof(initialId)))
        return ERROR;

    if (BE_SQLITE_OK != db.SaveChanges())
        return ERROR;
    }

    if (BE_SQLITE_OK != OpenECDb(filePath))
        return ERROR;

    //BIS ECSchema needs this table to pre-exist
    if (BE_SQLITE_OK != m_ecdb.ExecuteSql("CREATE VIRTUAL TABLE dgn_SpatialIndex USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)"))
        return ERROR;

    PERFLOG_START("ECDb ATP", "BIS schema import");
    const BentleyStatus stat = ImportSchemasFromFolder(bisSchemaFolder);
    PERFLOG_FINISH("ECDb ATP", "BIS schema import");
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
BentleyStatus BulkBisDomainCrudTestFixture::SetupDomainBimFile(Utf8CP fileName, BeFileName const& domainSchemaFolder, BeFileName const& bisSchemaFolder)
    {
    if (SUCCESS != CreateFakeBimFile(fileName, bisSchemaFolder))
        return ERROR;

    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    {
    Db db;
    if (BE_SQLITE_OK != db.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite)))
        return ERROR;

    uint64_t initialId = UINT64_C(1000);
    if (BE_SQLITE_DONE != db.SaveBriefcaseLocalValue("ec_classidsequence", initialId))
        return ERROR;

    //save initial id for fileformat compatibility test
    if (BE_SQLITE_OK != db.SaveProperty(PropertySpec("InitialClassId_BisDomains", "ECDb_FileFormatCompatiblity_Test"), &initialId, sizeof(initialId)))
        return ERROR;

    if (BE_SQLITE_OK != db.SaveChanges())
        return ERROR;
    }

    if (BE_SQLITE_OK != OpenECDb(filePath))
        return ERROR;

    PERFLOG_START("ECDb ATP", "BIS domain schema import");
    const BentleyStatus stat = ImportSchemasFromFolder(domainSchemaFolder);
    PERFLOG_FINISH("ECDb ATP", "BIS domain schema import");
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
BentleyStatus BulkBisDomainCrudTestFixture::ImportSchemasFromFolder(BeFileName const& schemaFolder)
    {
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext(false, true);
    ctx->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    ctx->AddSchemaPath(schemaFolder);

    BeFileName ecdbSchemaSearchPath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(ecdbSchemaSearchPath);
    ecdbSchemaSearchPath.AppendToPath(L"ECSchemas").AppendToPath(L"ECDb");
    ctx->AddSchemaPath(ecdbSchemaSearchPath);

    bvector<BeFileName> schemaPaths;
    BeDirectoryIterator::WalkDirsAndMatch(schemaPaths, schemaFolder, L"*.ecschema.xml", false);

    if (schemaPaths.empty())
        return ERROR;

    for (BeFileName const& schemaXmlFile : schemaPaths)
        {
        ECN::ECSchemaPtr ecSchema = nullptr;
        const SchemaReadStatus stat = ECN::ECSchema::ReadFromXmlFile(ecSchema, schemaXmlFile.GetName(), *ctx);
        //duplicate schema error is ok, as the ReadFromXmlFile reads schema references implicitly.
        if (SchemaReadStatus::Success != stat && SchemaReadStatus::DuplicateSchema != stat)
            return ERROR;
        }

    if (SUCCESS != m_ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas()))
        {
        m_ecdb.AbandonChanges();
        return ERROR;
        }

    m_ecdb.SaveChanges();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
//static
BeFileName BulkBisDomainCrudTestFixture::GetDomainSchemaFolder(BeFileName& bisSchemaFolder)
    {
    BeFileName domainSchemaFolder;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(domainSchemaFolder);
    domainSchemaFolder.AppendToPath(L"..\\..\\");
    domainSchemaFolder.AppendToPath(L"AllBisSchemas").AppendToPath(L"Assets").AppendToPath(L"ECSchemas");

    bisSchemaFolder.assign(domainSchemaFolder);
    bisSchemaFolder.AppendToPath(L"Dgn");

    domainSchemaFolder.AppendToPath(L"Domain");
    return domainSchemaFolder;
    }
END_ECDBUNITTESTS_NAMESPACE