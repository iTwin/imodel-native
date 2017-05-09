/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BulkCrudTestFixture.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BulkCrudTestFixture.h"
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeTextFile.h>
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//*****************************************************************************************
// BulkCrudTestFixture
//*****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
void BulkCrudTestFixture::AssertInsert(TestDataInfo& info, BeFileNameCR testDataJsonFile)
    {

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus BulkCrudTestFixture::CreateTestData(ECDbCR ecdb, BeFileNameCR testDataJsonFile)
    {
    if (!ecdb.IsDbOpen())
        return ERROR;

    const size_t instanceCountPerClass = 2;

    rapidjson::Document dataJson(rapidjson::kArrayType);
    rapidjson::MemoryPoolAllocator<>& jsonAllocator = dataJson.GetAllocator();

    srand((uint32_t) (BeTimeUtilities::QueryMillisecondsCounter() & 0xFFFFFFFF));

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
            if (SUCCESS != CreateTestInstance(ecdb, dataJson, *ecClass, false, jsonAllocator))
                return ERROR;

            if (SUCCESS != CreateTestInstance(ecdb, dataJson, *ecClass, true, jsonAllocator))
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

    BeFileStatus stat = BeFileStatus::Success;
    BeTextFilePtr dataJsonFile = BeTextFile::Open(stat, testDataJsonFile, TextFileOpenType::Write, TextFileOptions::KeepNewLine, TextFileEncoding::Utf8);
    if (BeFileStatus::Success != stat)
        {
        BeAssert(false && "Could not create data json output file");
        return ERROR;
        }


    rapidjson::StringBuffer jsonStrBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStrBuf);
    dataJson.Accept(writer);
    return TextFileWriteStatus::Success == dataJsonFile->PutLine(WString(jsonStrBuf.GetString(), BentleyCharEncoding::Utf8).c_str(), false) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus BulkCrudTestFixture::AssignPropValuePairs(ECDbCR ecdb, rapidjson::Value& json, ECPropertyIterableCR properties, bool ignoreNullableProps, rapidjson::MemoryPoolAllocator<>& jsonAllocator)
    {
    for (ECPropertyCP prop : properties)
        {
        if (ignoreNullableProps && IsNullableProperty(*prop))
            continue;

        if (SUCCESS != AssignPropValuePair(ecdb, json, *prop, ignoreNullableProps, jsonAllocator))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus BulkCrudTestFixture::AssignPropValuePair(ECDbCR ecdb, rapidjson::Value& json, ECPropertyCR prop, bool ignoreNullableMemberProps, rapidjson::MemoryPoolAllocator<>& jsonAllocator)
    {
    BeAssert(json.IsObject());
    rapidjson::GenericStringRef<Utf8Char> memberName(prop.GetName().c_str(), (rapidjson::SizeType) prop.GetName().size());
    //Caution: return value is struct again, not the inserted member
    json.AddMember(memberName, rapidjson::Value().Move(), jsonAllocator);
    rapidjson::Value& valueJson = json[memberName.s];
    if (prop.GetIsPrimitive())
        return AssignPrimitiveValue(valueJson, prop.GetAsPrimitiveProperty()->GetType(), jsonAllocator);

    if (prop.GetIsPrimitiveArray())
        {
        const PrimitiveType arrayType = prop.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
        size_t arraySize = (size_t) (rand() % 5) + 1;
        valueJson.SetArray();
        for (size_t i = 0; i < arraySize; i++)
            {
            valueJson.PushBack(rapidjson::Value().Move(), jsonAllocator);
            rapidjson::Value& newArrayElementJson = valueJson[(rapidjson::SizeType) i];
            if (SUCCESS != AssignPrimitiveValue(newArrayElementJson, arrayType, jsonAllocator))
                return ERROR;
            }

        return SUCCESS;
        }

    if (prop.GetIsStruct())
        {
        valueJson.SetObject();
        ECClassCR structType = prop.GetAsStructProperty()->GetType();
        return AssignPropValuePairs(ecdb, valueJson, structType.GetProperties(), ignoreNullableMemberProps, jsonAllocator);
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
            if (SUCCESS != AssignPropValuePairs(ecdb, newArrayElementJson, structType.GetProperties(), ignoreNullableMemberProps, jsonAllocator))
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
BentleyStatus BulkCrudTestFixture::AssignPrimitiveValue(rapidjson::Value& json, ECN::PrimitiveType primType, rapidjson::MemoryPoolAllocator<>& jsonAllocator)
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

            return ECRapidJsonUtilities::BinaryToJson(json, blob, sizeof(blob), jsonAllocator);
            }

            case PRIMITIVETYPE_Boolean:
                json.SetBool(rand() % 2 == 0);
                return SUCCESS;

            case PRIMITIVETYPE_DateTime:
            {
            DateTime dt = DateTime::GetCurrentTimeUtc();
            double jd = 0.0;
            if (SUCCESS != dt.ToJulianDay(jd))
                return ERROR;

            json.SetDouble(jd);
            return SUCCESS;
            }

            case PRIMITIVETYPE_Double:
                json.SetDouble(rand() * 1.4);
                return SUCCESS;

            case PRIMITIVETYPE_IGeometry:
            {
            IGeometryPtr geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(rand() * 1.0, rand() * 2.0, rand() * 1.0,
                                                                                               rand() * 1.0, rand() * 2.0, rand() * 1.0)));

            bvector<Byte> fb;
            BackDoor::BentleyGeometryFlatBuffer::GeometryToBytes(fb, *geom);
            return ECRapidJsonUtilities::BinaryToJson(json, fb.data(), fb.size(), jsonAllocator);
            }
            case PRIMITIVETYPE_Integer:
                json.SetInt(rand());
                return SUCCESS;

            case PRIMITIVETYPE_Long:
                json.SetInt64(rand() * 1111);
                return SUCCESS;

            case PRIMITIVETYPE_Point2d:
                return ECRapidJsonUtilities::Point2dToJson(json, DPoint2d::From(rand() * 1.4, rand() * (-1.0)), jsonAllocator);

            case PRIMITIVETYPE_Point3d:
                return ECRapidJsonUtilities::Point3dToJson(json, DPoint3d::From(rand() * 1.4, rand() * (-1.0), rand() * 1.0), jsonAllocator);

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
bool BulkCrudTestFixture::IsNullableProperty(ECN::ECPropertyCR prop)
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

//*****************************************************************************************
// BulkCrudTestFixture::TestDataInfo
//*****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
void BulkCrudTestFixture::TestDataInfo::ReadTestData(BeFileNameCR testDataJsonFile)
    {
    ASSERT_TRUE(testDataJsonFile.DoesPathExist()) << testDataJsonFile.GetNameUtf8().c_str();
    BeFileStatus stat;
    BeTextFilePtr file = BeTextFile::Open(stat, testDataJsonFile.GetName(), TextFileOpenType::Read, TextFileOptions::NewLinesToSpace);
    ASSERT_EQ(BeFileStatus::Success, stat) << "Opening " << testDataJsonFile.GetNameUtf8().c_str();
    WString fileContent;
    ASSERT_EQ(TextFileReadStatus::Success, file->GetLine(fileContent)) << testDataJsonFile.GetNameUtf8().c_str();
    ASSERT_FALSE(m_testDataJson.Parse<0>(Utf8String(fileContent).c_str()).HasParseError()) << testDataJsonFile.GetNameUtf8().c_str();
    }


//*****************************************************************************************
// BulkBisDomainCrudTestFixture
//*****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus BulkBisDomainCrudTestFixture::CreateFakeBimFile(Utf8CP fileName, BeFileNameCR bisSchemaFolder)
    {
    ECDbCR ecdb = SetupECDb(fileName);
    if (!ecdb.IsDbOpen())
        return ERROR;

    //BIS ECSchema needs this table to pre-exist
    if (BE_SQLITE_OK != ecdb.ExecuteSql("CREATE VIRTUAL TABLE dgn_SpatialIndex USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)"))
        return ERROR;

    return ImportSchemasFromFolder(bisSchemaFolder);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus BulkBisDomainCrudTestFixture::SetupDomainBimFile(Utf8CP fileName, BeFileName const& domainSchemaFolder, BeFileName const& bisSchemaFolder)
    {
    if (SUCCESS != CreateFakeBimFile(fileName, bisSchemaFolder))
        return ERROR;

    return ImportSchemasFromFolder(domainSchemaFolder);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus BulkBisDomainCrudTestFixture::ImportSchemasFromFolder(BeFileName const& schemaFolder)
    {
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext(false, true);
    ctx->AddSchemaLocater(GetECDb().GetSchemaLocater());
    ctx->AddSchemaPath(schemaFolder);

    bvector<BeFileName> schemaPaths;
    BeDirectoryIterator::WalkDirsAndMatch(schemaPaths, schemaFolder, L"*.ecschema.xml", false);

    if (schemaPaths.empty())
        return ERROR;

    for (BeFileName const& schemaXml : schemaPaths)
        {
        ECN::ECSchemaPtr ecSchema = nullptr;
        const SchemaReadStatus stat = ECN::ECSchema::ReadFromXmlFile(ecSchema, schemaXml.GetName(), *ctx);
        //duplicate schema error is ok, as the ReadFromXmlFile reads schema references implicitly.
        if (ECN::SchemaReadStatus::Success != stat && ECN::SchemaReadStatus::DuplicateSchema != stat)
            return ERROR;
        }

    if (SUCCESS != GetECDb().Schemas().ImportSchemas(ctx->GetCache().GetSchemas()))
        return ERROR;

    GetECDb().ClearECDbCache();
    return GetECDb().SaveChanges() == BE_SQLITE_OK ? SUCCESS : ERROR;
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