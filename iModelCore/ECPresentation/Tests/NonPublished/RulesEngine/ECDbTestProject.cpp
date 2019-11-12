/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include "ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      04/2012
+===============+===============+===============+===============+===============+======*/
struct ECDbTestUtility
{
    #define MAX_ARRAY_TEST_ENTRIES 3
    typedef void(*PopulatePrimitiveValueCallback)(ECN::ECValueR value, ECN::PrimitiveType primitiveType, ECN::ECPropertyCP ecproperty);

private:
    static void PopulateStructValue(ECValueR value, ECClassCR structType, PopulatePrimitiveValueCallback populatePrimitiveValueCallback)
        {
        value.Clear();
        IECInstancePtr inst = CreateArbitraryECInstance(structType, populatePrimitiveValueCallback);
        value.SetStruct(inst.get());
        }
    static void PopulatePrimitiveValue(ECN::ECValueR value, ECN::PrimitiveType primitiveType, ECN::ECPropertyCP ecProperty)
        {
        value.Clear();
        switch (primitiveType)
            {
                case PRIMITIVETYPE_String:
                    value.SetUtf8CP("Sample string"); break;
                case PRIMITIVETYPE_Integer:
                    value.SetInteger(123); break;
                case PRIMITIVETYPE_Long:
                    value.SetLong(123456789); break;
                case PRIMITIVETYPE_Double:
                    value.SetDouble(PI); break;
                case PRIMITIVETYPE_DateTime:
                {
                DateTime::Info dti;
                if (ecProperty != nullptr && StandardCustomAttributeHelper::GetDateTimeInfo(dti, *ecProperty) == ECObjectsStatus::Success)
                    {
                    if (!dti.IsValid())
                        dti = DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified);

                    if (dti.GetKind() == DateTime::Kind::Local)
                        {
                        //local date times are not supported by ECObjects
                        break;
                        }

                    DateTime dt;
                    DateTime::FromJulianDay(dt, 2456341.75, dti);
                    value.SetDateTime(dt);
                    }
                break;
                }

                case PRIMITIVETYPE_Binary:
                {
                Byte blob[] = {0x0c, 0x0b, 0x0c, 0x0b, 0x0c, 0x0b, 65, 66, 67, 68, 0x0c};
                value.SetBinary(blob, 11, true);
                break;
                }
                case PRIMITIVETYPE_Boolean:
                    value.SetBoolean(true); break;
                case PRIMITIVETYPE_Point2d:
                {
                DPoint2d point2d;
                point2d.x = 11.25;
                point2d.y = 22.16;
                value.SetPoint2d(point2d);
                break;
                }
                case PRIMITIVETYPE_Point3d:
                {
                DPoint3d point3d;
                point3d.x = 11.23;
                point3d.y = 22.14;
                point3d.z = 33.12;
                value.SetPoint3d(point3d);
                break;
                }

                case PRIMITIVETYPE_IGeometry:
                {
                IGeometryPtr line = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
                value.SetIGeometry(*line);
                break;
                }
            }
        }
    static ECObjectsStatus CopyStruct(IECInstanceR source, ECValuesCollectionCR collection, Utf8CP baseAccessPath)
        {
        ECObjectsStatus status = ECObjectsStatus::Success;
        for (auto& propertyValue : collection)
            {
            auto pvAccessString = propertyValue.GetValueAccessor().GetPropertyName();
            auto accessString = baseAccessPath == nullptr ? pvAccessString : Utf8String(baseAccessPath) + "." + pvAccessString;

            if (propertyValue.HasChildValues())
                {
                status = CopyStruct(source, *propertyValue.GetChildValues(), accessString.c_str());
                if (status != ECObjectsStatus::Success)
                    {
                    return status;
                    }
                continue;
                }

            auto& location = propertyValue.GetValueAccessor().DeepestLocationCR();

            //auto property = location.GetECProperty(); 
            //BeAssert(property != nullptr);
            if (location.GetArrayIndex() >= 0)
                {
                source.AddArrayElements(accessString.c_str(), 1);
                status = source.SetValue(accessString.c_str(), propertyValue.GetValue(), location.GetArrayIndex());
                }
            else
                status = source.SetValue(accessString.c_str(), propertyValue.GetValue());

            if (status != ECObjectsStatus::Success)
                {
                return status;
                }
            }
        return status;
        }
    static ECObjectsStatus CopyStruct(IECInstanceR target, IECInstanceCR structValue, Utf8CP propertyName)
        {
        return CopyStruct(target, *ECValuesCollection::Create(structValue), propertyName);
        }

public:
    static BeFileName BuildECDbPath(Utf8CP ecdbFileName)
        {
        BeFileName ecdbPath;
        BeTest::GetHost().GetOutputRoot(ecdbPath);
        ecdbPath.AppendToPath(WString(ecdbFileName, BentleyCharEncoding::Utf8).c_str());
        return ecdbPath;
        }
    static ECN::ECSchemaPtr ReadECSchemaFromDisk(Utf8CP ecSchemaFileName, Utf8CP ecSchemaSearchPath = nullptr)
        {
        ECSchemaPtr schema = nullptr;
        ECSchemaReadContextPtr context = nullptr;
        ReadECSchemaFromDisk(schema, context, ecSchemaFileName, ecSchemaSearchPath);
        EXPECT_TRUE(schema.IsValid());
        return schema;
        }
    static void ReadECSchemaFromDisk(ECN::ECSchemaPtr& ecSchema, ECN::ECSchemaReadContextPtr& ecSchemaContext, Utf8CP ecSchemaFileName, Utf8CP ecSchemaSearchPath = nullptr)
        {
        // Construct the path to the sample schema
        BeFileName ecSchemaPath;
        if (ecSchemaSearchPath == nullptr)
            {
            BeTest::GetHost().GetDocumentsRoot(ecSchemaPath);
            ecSchemaPath.AppendToPath(L"ECPresentationTestData");
            }
        else
            {
            ecSchemaPath.SetName(WString(ecSchemaSearchPath, true));
            }

        BeFileName ecSchemaFile(ecSchemaPath);
        ecSchemaFile.AppendToPath(WString(ecSchemaFileName, true).c_str());
        ASSERT_TRUE(BeFileName::DoesPathExist(ecSchemaFile.GetName()));

        // Read the sample schema
        if (!ecSchemaContext.IsValid())
            ecSchemaContext = ECSchemaReadContext::CreateContext();
        ecSchemaContext->AddSchemaPath(ecSchemaPath.GetName());

        SchemaReadStatus ecSchemaStatus = ECSchema::ReadFromXmlFile(ecSchema, ecSchemaFile.GetName(), *ecSchemaContext);
        if (ecSchemaStatus == SchemaReadStatus::Success)
            {
            ASSERT_TRUE(ecSchema.IsValid());
            return;
            }

        ASSERT_EQ(ecSchemaStatus, SchemaReadStatus::Success);
        ASSERT_TRUE(ecSchema.IsValid());
        }
    static BentleyStatus SetECInstanceId(ECN::IECInstanceR instance, ECInstanceId const& instanceId)
        {
        if (!instanceId.IsValid())
            return ERROR;

        Utf8Char instanceIdStr[BeInt64Id::ID_STRINGBUFFER_LENGTH];
        instanceId.ToString(instanceIdStr);
        return instance.SetInstanceId(instanceIdStr) == ECObjectsStatus::Success ? SUCCESS : ERROR;
        }

    static ECN::IECInstancePtr CreateArbitraryECInstance(ECN::ECClassCR ecClass, PopulatePrimitiveValueCallback populatePrimitiveValueCallback = PopulatePrimitiveValue, bool skipStructs = false, bool skipArrays = false)
        {
        IECInstancePtr instance = ecClass.GetDefaultStandaloneEnabler()->CreateInstance(0);
        PopulateECInstance(instance, populatePrimitiveValueCallback, skipStructs, skipArrays);
        return instance;
        }
    static void PopulateECInstance(ECN::IECInstancePtr ecInstance, PopulatePrimitiveValueCallback populatePrimitiveValueCallback = PopulatePrimitiveValue, bool skipStructs = false, bool skipArrays = false)
        {
        ECValue value;
        for (ECPropertyCP ecProperty : ecInstance->GetClass().GetProperties(true))
            {
            if (!skipStructs && ecProperty->GetIsStruct())
                {
                PopulateStructValue(value, ecProperty->GetAsStructProperty()->GetType(), populatePrimitiveValueCallback);
                CopyStruct(*ecInstance, *value.GetStruct(), ecProperty->GetName().c_str());
                }
            else if (ecProperty->GetIsPrimitive())
                {
                populatePrimitiveValueCallback(value, ecProperty->GetAsPrimitiveProperty()->GetType(), ecProperty);
                ecInstance->SetValue(ecProperty->GetName().c_str(), value);
                }
            else if (!skipArrays && ecProperty->GetIsArray())
                {
                ArrayECPropertyCP arrayProperty = ecProperty->GetAsArrayProperty();
                if (arrayProperty->GetIsPrimitiveArray() && arrayProperty->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType() == PRIMITIVETYPE_IGeometry)
                    continue;

                uint32_t arrayCount = MAX_ARRAY_TEST_ENTRIES;
                if (arrayCount < arrayProperty->GetMinOccurs())
                    arrayCount = arrayProperty->GetMinOccurs();
                else if (arrayCount > arrayProperty->GetMaxOccurs())
                    arrayCount = arrayProperty->GetMaxOccurs();

                ecInstance->AddArrayElements(ecProperty->GetName().c_str(), arrayCount);
                if (arrayProperty->GetIsStructArray())
                    {
                    StructArrayECPropertyCP structArrayProperty = ecProperty->GetAsStructArrayProperty();
                    for (uint32_t i = 0; i < arrayCount; i++)
                        {
                        PopulateStructValue(value, structArrayProperty->GetStructElementType(), populatePrimitiveValueCallback);
                        ecInstance->SetValue(ecProperty->GetName().c_str(), value, i);
                        }
                    }
                else if (arrayProperty->GetIsPrimitiveArray())
                    {
                    PrimitiveArrayECPropertyCP primitiveArrayProperty = ecProperty->GetAsPrimitiveArrayProperty();
                    for (uint32_t i = 0; i < arrayCount; i++)
                        {
                        populatePrimitiveValueCallback(value, primitiveArrayProperty->GetPrimitiveElementType(), ecProperty);
                        ecInstance->SetValue(ecProperty->GetName().c_str(), value, i);
                        }
                    }
                }
            }
        }
};

static bool s_isInitialized = false; 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void Initialize()
    {
    if (!s_isInitialized)
        {
        //establish standard schema search paths (they are in the application dir)
        BeFileName applicationSchemaDir;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory (applicationSchemaDir);

        BeFileName temporaryDir;
        BeTest::GetHost().GetOutputRoot (temporaryDir);

        ECDb::Initialize (temporaryDir, &applicationSchemaDir);
        srand ((uint32_t)(0xffffffff & BeTimeUtilities::QueryMillisecondsCounter ()));
        s_isInitialized = true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbTestProject::ECDbTestProject(ECDb* db) : m_ecdb(nullptr != db ? db : new ECDb()) {Initialize();}

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                11/2012
//+---------------+---------------+---------------+---------------+---------------+-
ECDbTestProject::~ECDbTestProject () 
    {
    m_inserterCache.clear ();

    if (m_ecdb->IsDbOpen ())
        {
        m_ecdb->CloseDb ();
        }
    delete m_ecdb;
    m_ecdb = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                11/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECDbR ECDbTestProject::Create (Utf8CP ecdbFileName)
    {
    CreateEmpty (ecdbFileName);
    return GetECDb ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbR ECDbTestProject::Create (Utf8CP ecdbFileName, Utf8CP testSchemaXmlFileName)
    {
    CreateEmpty (ecdbFileName);
    ECSchemaPtr schema = nullptr;
    if (ImportECSchema(schema, testSchemaXmlFileName) != SUCCESS)
        {
        BeAssert(false);
        return GetECDb();
        }

    return GetECDb ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbTestProject::CreateEmpty (Utf8CP ecdbFileName)
    {
    BeFileName ecdbFilePath = ECDbTestUtility::BuildECDbPath(ecdbFileName);
    if (ecdbFilePath.DoesPathExist())
        {
        // Delete any previously created file
        ASSERT_EQ (BeFileNameStatus::Success, BeFileName::BeDeleteFile (ecdbFilePath.GetName ()));
        }
    
    Utf8String ecdbFilePathUtf8 = ecdbFilePath.GetNameUtf8 ();
    DbResult stat = m_ecdb->CreateNewDb (ecdbFilePathUtf8.c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test ECDb file failed.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                04/2013
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDbTestProject::Open (Utf8CP ecdbFileName, ECDb::OpenParams openParams)
    {
    BeFileName ecdbFilePath(ecdbFileName);
    if (!ecdbFilePath.IsAbsolutePath())
        ecdbFilePath = ECDbTestUtility::BuildECDbPath(ecdbFileName);
    m_ecdb->RemoveIssueListener();
    return m_ecdb->OpenBeSQLiteDb (ecdbFilePath, openParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                04/2013
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDbTestProject::ReOpen()
    {
    BeFileName path(m_ecdb->GetDbFileName());
    m_ecdb->CloseDb();
    return m_ecdb->OpenBeSQLiteDb(path, BeSQLite::Db::OpenParams(Db::OpenMode::ReadWrite));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbTestProject::ImportECSchema(ECN::ECSchemaPtr& schema, Utf8CP testSchemaXmlFileName)
    {
    if (Utf8String::IsNullOrEmpty(testSchemaXmlFileName))
        return ERROR;

    m_schemaContext = ECSchemaReadContext::CreateContext();
    m_schemaContext->AddSchemaLocater(GetECDbCR().GetSchemaLocater());
    ECDbTestUtility::ReadECSchemaFromDisk(schema, m_schemaContext, testSchemaXmlFileName);
    EXPECT_TRUE(schema != nullptr);
    if (schema == nullptr)
        return ERROR;

    if (SUCCESS != GetECDbCR().Schemas().ImportSchemas(m_schemaContext->GetCache().GetSchemas()))
        return ERROR;

    GetECDb().SaveChanges();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbTestProject::InsertECInstance (ECInstanceKey& ecInstanceKey, IECInstancePtr ecInstance)
    {
    EXPECT_TRUE (ecInstance.IsValid ());

    ECClassCR ecClass = ecInstance->GetClass();
    ECInstanceInserter const* inserter = nullptr;
    auto it = m_inserterCache.find (&ecClass);
    if (it == m_inserterCache.end ())
        {
        auto newInserter = std::unique_ptr<ECInstanceInserter> (new ECInstanceInserter (GetECDb (), ecClass, nullptr));
        if (!newInserter->IsValid ())
            //ECClass is not mapped or not instantiable -> ignore it
            return ERROR;

        inserter = newInserter.get ();
        m_inserterCache[&ecClass] = std::move (newInserter);
        }
    else
        inserter = it->second.get ();

    if (BE_SQLITE_OK == inserter->Insert (ecInstanceKey, *ecInstance))
        {
        if (SUCCESS != ECDbTestUtility::SetECInstanceId (*ecInstance, ecInstanceKey.GetInstanceId ()))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   05/12
---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbTestProject::GetInstances (bvector<IECInstancePtr>& instances, Utf8CP schemaName, Utf8CP className, bool polymorphic)
    {
    instances.clear();

    ECClassCP ecClass = GetECDb().Schemas().GetClass (schemaName, className);
    EXPECT_TRUE (ecClass != nullptr) << "ECDbTestProject::GetInstances> ECClass '" << className << "' not found.";
    if (ecClass == nullptr)
        return ERROR;

    SqlPrintfString ecSql ("SELECT * %s [%s].[%s]", polymorphic ? "FROM" : "FROM ONLY", ecClass->GetSchema().GetName().c_str(), className);
    ECSqlStatement ecStatement;
    ECSqlStatus status = ecStatement.Prepare (GetECDb(), ecSql.GetUtf8CP());
    EXPECT_EQ(ECSqlStatus::Success, status) << "ECDbTestProject::GetInstances> Preparing ECSQL '" << ecSql.GetUtf8CP () << "' failed.";
    if (status != ECSqlStatus::Success)
        return ERROR;

    ECInstanceECSqlSelectAdapter adapter (ecStatement);
    while (BE_SQLITE_ROW == ecStatement.Step())
        {
        IECInstancePtr instance = adapter.GetInstance();
        BeAssert (instance.IsValid());
        if (instance != nullptr)
            instances.push_back (instance);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr ECDbTestProject::CreateECInstance(ECClassCR ecClass)
    {
    StandaloneECEnablerPtr instanceEnabler = ecClass.GetDefaultStandaloneEnabler ();
    POSTCONDITION (instanceEnabler != nullptr, nullptr);
    IECInstancePtr instance = instanceEnabler->CreateInstance ();
    POSTCONDITION (instance != nullptr, nullptr);
    return instance;
    }
