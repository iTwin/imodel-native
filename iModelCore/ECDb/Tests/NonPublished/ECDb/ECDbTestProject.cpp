/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/ECDb/ECDbTestProject.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../PublicApi/NonPublished/ECDb/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle     09/2015
//---------------------------------------------------------------------------------------
void ECDbIssueListener::_OnIssueReported(ECDbIssueSeverity severity, Utf8CP message) const
    {
    m_issue = ECDbIssue(severity, message);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle     09/2015
//---------------------------------------------------------------------------------------
ECDbIssue ECDbIssueListener::GetIssue() const
    {
    if (!m_issue.IsIssue())
        return m_issue;

    ECDbIssue copy(m_issue);
    //reset cached issue before returning
    m_issue = ECDbIssue();
    return std::move(copy);
    }


//********************* ECDbTestSchemaManager ********************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECDbTestSchemaManager::ECDbTestSchemaManager (ECDb& ecdb)
: m_ecdb (ecdb), m_testSchema (nullptr)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECClassCP ECDbTestSchemaManager::GetClass (Utf8CP className) const
    {
    return GetClass (GetTestSchemaName ().c_str (), className);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECClassCP ECDbTestSchemaManager::GetClass (Utf8CP schemaName, Utf8CP className) const
    {
    return Schemas ().GetECClass (schemaName, className);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbTestSchemaManager::ImportTestSchema (WCharCP testSchemaFileName)
    {
    EXPECT_FALSE (WString::IsNullOrEmpty (testSchemaFileName));
    if (WString::IsNullOrEmpty (testSchemaFileName))
        return ERROR;

    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext ();
    StopWatch s ("", true);
    ECDbTestUtility::ReadECSchemaFromDisk (m_testSchema, schemaReadContext, testSchemaFileName);
    s.Stop ();
    EXPECT_TRUE (m_testSchema != nullptr);
    if (m_testSchema == nullptr)
        return ERROR;

    LOG.infov ("Loaded %s from disk [%.4lf seconds]", m_testSchema->GetFullSchemaName ().c_str (), s.GetElapsedSeconds ());

    s.Start ();
    auto importSchemaStatus = Schemas ().ImportECSchemas (schemaReadContext->GetCache (), ECDbSchemaManager::ImportOptions (true, false));
    s.Stop();
    LOG.infov ("Imported %s into DgnDb [%.4lf seconds]", m_testSchema->GetFullSchemaName ().c_str (), s.GetElapsedSeconds());
    EXPECT_EQ (SUCCESS, importSchemaStatus);
    if (SUCCESS != importSchemaStatus)
        return ERROR;

    GetECDb().SaveChanges();

    //cache test schema name in UTF-8
    m_testSchemaName = m_testSchema->GetName ().c_str ();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaPtr ECDbTestSchemaManager::GetTestSchema () const
    {
    BeAssert (m_testSchema != nullptr && "ECDbTestSchemaManager::GetTestSchema can only be called after ImportTestSchema was called.");
    return m_testSchema;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR ECDbTestSchemaManager::GetTestSchemaName () const
    {
    BeAssert (m_testSchema != nullptr && "ECDbTestSchemaManager::GetTestSchemaName can only be called after ImportTestSchema was called.");
    return m_testSchemaName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECDbSchemaManagerCR ECDbTestSchemaManager::Schemas () const
    {
    return GetECDb (). Schemas ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECDbR ECDbTestSchemaManager::GetECDb () const
    {
    return m_ecdb;
    }

//********************* ECDbTestProject ********************************************

#define MAX_ARRAY_TEST_ENTRIES 3

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//static
bool ECDbTestProject::s_isInitialized = false; 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbTestProject::ECDbTestProject () 
    : m_ecdb (new ECDb), m_testSchemaFileName (nullptr), m_testSchemaManager (*m_ecdb)
    {
    Initialize ();
    }

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void ECDbTestProject::Initialize ()
    {
    if (!s_isInitialized)
        {
        //establish standard schema search paths (they are in the application dir)
        BeFileName applicationSchemaDir;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory (applicationSchemaDir);

        BeFileName temporaryDir;
        BeTest::GetHost().GetOutputRoot (temporaryDir);

        ECDb::Initialize (temporaryDir, &applicationSchemaDir);
        srand ((uint32_t)BeTimeUtilities::QueryMillisecondsCounter ());
        s_isInitialized = true;
        }
    }   

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                08/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbTestProject::IsInitialized ()
    {
    return s_isInitialized;
    }   

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                11/2012
//+---------------+---------------+---------------+---------------+---------------+-
Utf8CP ECDbTestProject::GetECDbPath 
(
) const
    {
    return GetECDbCR ().GetDbFileName ();
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                11/2012
//+---------------+---------------+---------------+---------------+---------------+-
//static
Utf8String ECDbTestProject::BuildECDbPath 
(
Utf8CP ecdbFileName
)
    {
    WString ecdbFileNameW (ecdbFileName, BentleyCharEncoding::Utf8);
    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot (outputDir);
    BeFileName dbFile (nullptr, outputDir.GetName(), ecdbFileNameW.c_str (), nullptr);
    return dbFile.GetNameUtf8 ();
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
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbR ECDbTestProject::Create (Utf8CP ecdbFileName, WCharCP testSchemaXmlFileName, bool importArbitraryECInstances)
    {
    //default number of ECInstances (if import flag is true) is 3
    const int numberOfECInstancesToImport = importArbitraryECInstances ? 3 : 0;

    return Create (ecdbFileName, testSchemaXmlFileName, numberOfECInstancesToImport);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbR ECDbTestProject::Create (Utf8CP ecdbFileName, WCharCP testSchemaXmlFileName, int numberOfArbitraryECInstancesToImport)
    {
    CreateEmpty (ecdbFileName);
    if (ImportECSchema (testSchemaXmlFileName) != SUCCESS)
        return GetECDb ();
        
    if (numberOfArbitraryECInstancesToImport > 0)
        ImportArbitraryECInstances (numberOfArbitraryECInstancesToImport);

    return GetECDb ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbTestProject::CreateEmpty (Utf8CP ecdbFileName)
    {
    WString ecdbFileNameW (ecdbFileName, BentleyCharEncoding::Utf8);
    BeFileName ecdbFilePath;
    BeTest::GetHost ().GetOutputRoot (ecdbFilePath);
    ecdbFilePath.AppendToPath (ecdbFileNameW.c_str ());

    if (BeFileName::DoesPathExist (ecdbFilePath.GetName ()))
        {
        // Delete any previously created file
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (ecdbFilePath.GetName ());
        ASSERT_TRUE (BeFileNameStatus::Success == fileDeleteStatus);
        }
    
    Utf8String ecdbFilePathUtf8 = ecdbFilePath.GetNameUtf8 ();
    DbResult stat = m_ecdb->CreateNewDb (ecdbFilePathUtf8.c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test ECDb file failed.";

    LOG.debugv("Created test ECDb file '%s'", ecdbFilePathUtf8.c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                04/2013
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECDbTestProject::Open (Utf8CP ecdbFilePath, ECDb::OpenParams openParams)
    {
    m_ecdb->RemoveIssueListener();
    return m_ecdb->OpenBeSQLiteDb (ecdbFilePath, openParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbR ECDbTestProject::GetECDb ()
    {
    return *m_ecdb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbCR ECDbTestProject::GetECDbCR () const
    {
    return *m_ecdb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbTestProject::ImportECSchema (WCharCP testSchemaXmlFileName)
    {
    return m_testSchemaManager.ImportTestSchema (testSchemaXmlFileName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
ECObjectsStatus ECDbTestProject::CopyStruct(IECInstanceR source, ECValuesCollectionCR collection, Utf8CP baseAccessPath)
    { 
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    for(auto& propertyValue : collection)
        {
        auto pvAccessString = propertyValue.GetValueAccessor().GetPropertyName();
        auto accessString =  baseAccessPath == nullptr ? pvAccessString : Utf8String(baseAccessPath) + "." + pvAccessString;

        if (propertyValue.HasChildValues())
            {
            status = CopyStruct (source, *propertyValue.GetChildValues(), accessString.c_str());
            if (status != ECOBJECTS_STATUS_Success)
                {
                return status;
                }
            continue;
            }

        auto& location = propertyValue.GetValueAccessor().DeepestLocationCR();
        
        //auto property = location.GetECProperty(); 
        //BeAssert(property != nullptr);
        if (location.GetArrayIndex()>= 0)
            {
            source.AddArrayElements(accessString.c_str(), 1);
            status = source.SetValue (accessString.c_str(), propertyValue.GetValue(), location.GetArrayIndex()); 
            }
        else
            status = source.SetValue (accessString.c_str(), propertyValue.GetValue()); 

        if (status != ECOBJECTS_STATUS_Success)
            {
            return status;
            }
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
ECObjectsStatus ECDbTestProject::CopyStruct(IECInstanceR target, IECInstanceCR structValue, Utf8CP propertyName)
    {
    return CopyStruct(target, *ECValuesCollection::Create(structValue), propertyName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     03/12
//---------------------------------------------------------------------------------------
//static
IECInstancePtr ECDbTestProject::CreateArbitraryECInstance(ECClassCR ecClass, PopulatePrimitiveValueCallback populatePrimitiveValueCallback, bool skipStructs, bool skipArrays)
    {
    IECInstancePtr instance = ecClass.GetDefaultStandaloneEnabler()->CreateInstance(0);
    PopulateECInstance (instance, populatePrimitiveValueCallback, skipStructs, skipArrays);
    return instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     03/12
//---------------------------------------------------------------------------------------
void ECDbTestProject::PopulateECInstance(ECN::IECInstancePtr ecInstance, PopulatePrimitiveValueCallback populatePrimitiveValueCallback, bool skipStructs, bool skipArrays)
    {
    ECValue value;
    for (ECPropertyCP ecProperty : ecInstance->GetClass().GetProperties(true))
        {
        if (!skipStructs && ecProperty->GetIsStruct())
            {
            PopulateStructValue (value, ecProperty->GetAsStructProperty()->GetType(), populatePrimitiveValueCallback);
            CopyStruct(*ecInstance, *value.GetStruct(), ecProperty->GetName().c_str());
            }
        else if (ecProperty->GetIsPrimitive())
            {
            populatePrimitiveValueCallback (value, ecProperty->GetAsPrimitiveProperty()->GetType(), ecProperty);
            ecInstance->SetValue (ecProperty->GetName().c_str(), value);
            }
        else if (!skipArrays && ecProperty->GetIsArray())
            {
            ArrayECPropertyCP arrayProperty = ecProperty->GetAsArrayProperty();
            if(arrayProperty->GetKind() == ARRAYKIND_Primitive && arrayProperty->GetPrimitiveElementType() == PRIMITIVETYPE_IGeometry)
                continue;

            uint32_t arrayCount = MAX_ARRAY_TEST_ENTRIES;
            if (arrayCount < arrayProperty->GetMinOccurs ())
                arrayCount = arrayProperty->GetMinOccurs ();
            else if (arrayCount > arrayProperty->GetMaxOccurs ())
                arrayCount = arrayProperty->GetMaxOccurs ();

            ecInstance->AddArrayElements (ecProperty->GetName ().c_str (), arrayCount);
            if (arrayProperty->GetKind() == ARRAYKIND_Struct)
                {
                for (uint32_t i = 0; i < arrayCount; i++)
                    {
                    PopulateStructValue (value, *arrayProperty->GetStructElementType(), populatePrimitiveValueCallback);
                    ecInstance->SetValue (ecProperty->GetName().c_str (), value, i);
                    }
                }
            else if (arrayProperty->GetKind() == ARRAYKIND_Primitive )
                {
                for (uint32_t i = 0; i < arrayCount; i++)
                    {
                    populatePrimitiveValueCallback (value, arrayProperty->GetPrimitiveElementType(), ecProperty);
                    ecInstance->SetValue (ecProperty->GetName().c_str (), value, i);
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbTestProject::ImportArbitraryECInstances
(
int numberOfECInstances
)
    {
    StopWatch stopwatch ("", true);
    int classCount = 0;
    for (ECClassCP ecClass : GetTestSchemaManager ().GetTestSchema ()->GetClasses())
        {
        if (ecClass->GetRelationshipClassCP())
            continue; // skip relationships
        
        classCount++;
        for (int i = 0; i < numberOfECInstances; i++)
            {
            IECInstancePtr ecInstance = CreateArbitraryECInstance (*ecClass);
            ImportECInstance (ecInstance);
            }

        //no need to longer hold inserter for a class as each class is only touched once in this method.
        m_inserterCache.clear ();
        }

    GetECDb ().SaveChanges();
    stopwatch.Stop ();
    LOG.infov (L"Inserting %d test instances per class for %d classes took %lf milliseconds.", numberOfECInstances, classCount, stopwatch.GetElapsedSeconds () * 1000);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbTestProject::InsertECInstance 
(
ECInstanceKey& ecInstanceKey,
IECInstancePtr ecInstance
)
    {
    EXPECT_TRUE (ecInstance.IsValid ());

    ECClassCR ecClass = ecInstance->GetClass();
    ECInstanceInserter const* inserter = nullptr;
    auto it = m_inserterCache.find (&ecClass);
    if (it == m_inserterCache.end ())
        {
        auto newInserter = std::unique_ptr<ECInstanceInserter> (new ECInstanceInserter (GetECDb (), ecClass));
        if (!newInserter->IsValid ())
            //ECClass is not mapped or not instantiable -> ignore it
            return ERROR;

        inserter = newInserter.get ();
        m_inserterCache[&ecClass] = std::move (newInserter);
        }
    else
        inserter = it->second.get ();

    auto stat = inserter->Insert (ecInstanceKey, *ecInstance);
    if (stat == SUCCESS)
        {
        if (SUCCESS != ECDbTestUtility::SetECInstanceId (*ecInstance, ecInstanceKey.GetECInstanceId ()))
            return ERROR;
        }

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbTestProject::ImportECInstance (IECInstancePtr ecInstance)
    {
    ECInstanceKey ecInstanceKey;
    auto status = InsertECInstance (ecInstanceKey, ecInstance);
    if (ERROR == status)
        {
        return;
        }

    ECInstanceMap::const_iterator it = m_ecInstances.find (ecInstanceKey.GetECInstanceId ());
    ASSERT_TRUE (it == m_ecInstances.end ()) << "Duplicate instance IDs inserted" << ecInstanceKey.GetECInstanceId ().GetValue () << "for class " << ecInstance->GetClass ().GetName ().c_str () << "\n";
    m_ecInstances[ecInstanceKey.GetECInstanceId ()] = ecInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//static
IECInstancePtr ECDbTestProject::CreateECInstance 
(
ECClassCR ecClass
)
    {
    StandaloneECEnablerP instanceEnabler = ecClass.GetDefaultStandaloneEnabler ();
    POSTCONDITION (instanceEnabler != nullptr, nullptr);
    IECInstancePtr instance = instanceEnabler->CreateInstance ();
    POSTCONDITION (instance != nullptr, nullptr);
    return instance;
    }

//---------------------------------------------------------------------------------------
//! @remarks Currently only supports primitive properties (except for IGeometry).
//! @bsimethod                                 Krischan.Eberle                09/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ECDbTestProject::AssignRandomValueToECInstance 
(
ECValueP createdValue,
IECInstancePtr instance, 
Utf8CP propertyName
)
    {
    ECPropertyP ecProperty = instance->GetClass ().GetPropertyP (propertyName);
    ASSERT_TRUE (ecProperty != nullptr);
    ASSERT_TRUE (ecProperty->GetIsPrimitive ());

    PrimitiveType type = ecProperty->GetAsPrimitiveProperty ()->GetType ();
    
    ECValue value;
    GenerateRandomValue (value, type, ecProperty);

    instance->SetValue (propertyName, value);
    //if requested, return the value created (as copy)
    if (createdValue != nullptr)
        {
        *createdValue = value;
        }
    }

//---------------------------------------------------------------------------------------
//! @remarks Currently only supports primitive properties (except for IGeometry).
//! @bsimethod                                 Krischan.Eberle                09/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ECDbTestProject::GenerateRandomValue
(
ECValueR value,
PrimitiveType type,
ECPropertyCP ecProperty
)
    {
    srand ((uint32_t)BeTimeUtilities::QueryMillisecondsCounter ());
    int randomNumber = rand ();
    switch (type)
        {
        case PRIMITIVETYPE_String: 
            {
            Utf8String text;
            text.Sprintf ("Sample text with random number: %d", randomNumber);
            value.SetUtf8CP(text.c_str (), true); 
            }
            break;

        case PRIMITIVETYPE_Integer: 
            {
            value.SetInteger(randomNumber); 
            }
            break;

        case PRIMITIVETYPE_Long: 
            {
            const int32_t intMax = std::numeric_limits<int32_t>::max ();
            const int64_t longValue = static_cast<int64_t> (intMax) + randomNumber;
            value.SetLong(longValue); 
            }
            break;

        case PRIMITIVETYPE_Double: 
            {
            value.SetDouble(randomNumber * PI);
            }
            break;

        case PRIMITIVETYPE_DateTime: 
            {
            DateTime dt;
            DateTimeInfo dti;
            if (ecProperty != nullptr && StandardCustomAttributeHelper::GetDateTimeInfo (dti, *ecProperty) == ECOBJECTS_STATUS_Success)
                {
                DateTime::Info info = dti.GetInfo (true);
                if (info.GetKind () == DateTime::Kind::Local)
                    {
                    //local date times are not supported by ECDb
                    break;
                    }

                uint64_t jd = 0ULL;
                DateTime::GetCurrentTimeUtc ().ToJulianDay (jd);
                
                DateTime::FromJulianDay (dt, jd, info); 
                }
            else
                {
                dt = DateTime::GetCurrentTimeUtc ();
                }

            value.SetDateTime(dt); 
            }
            break;

        case PRIMITIVETYPE_Boolean: 
            {
            value.SetBoolean(randomNumber % 2 != 0); 
            }
            break;

        case PRIMITIVETYPE_Point2D: 
            {
            DPoint2d point2d;
            point2d.x=randomNumber * 1.0;
            point2d.y=randomNumber * 1.8;
            value.SetPoint2D(point2d);
            break;
            }
        case PRIMITIVETYPE_Point3D:
            {
            DPoint3d point3d;
            point3d.x=randomNumber * 1.0;
            point3d.y=randomNumber * 1.8;
            point3d.z=randomNumber * 2.9;
            value.SetPoint3D(point3d);
            break;
            }

        case PRIMITIVETYPE_Binary  : 
            {
            Byte blob[]= {0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x1a, 0xaa, 0xfa, 0x00};
            value.SetBinary(blob, 10);
            break;
            }

        case PRIMITIVETYPE_IGeometry:
            {
            LOG.debug (L"Generating random values for IGeometry not supported by ECDbTest yet.");
            break;
            }
        default:
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                          03/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void ECDbTestProject::PopulateStructValue (ECValueR value, ECClassCR structType, PopulatePrimitiveValueCallback populatePrimitiveValueCallback)
    {
    value.Clear();
    IECInstancePtr inst = CreateArbitraryECInstance (structType, populatePrimitiveValueCallback);
    value.SetStruct(inst.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                          03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbTestProject::PopulatePrimitiveValue (ECValueR value, PrimitiveType primitiveType, ECPropertyCP ecProperty)
    {
    value.Clear();
    switch (primitiveType)
        {
        case PRIMITIVETYPE_String  : 
            value.SetUtf8CP("Sample string"); break;
        case PRIMITIVETYPE_Integer : 
            value.SetInteger(123); break;
        case PRIMITIVETYPE_Long    : 
            value.SetLong(123456789); break;
        case PRIMITIVETYPE_Double  : 
            value.SetDouble(PI);break;
        case PRIMITIVETYPE_DateTime: 
            {
            DateTimeInfo dti;
            if (ecProperty != nullptr && StandardCustomAttributeHelper::GetDateTimeInfo (dti, *ecProperty) == ECOBJECTS_STATUS_Success)
                {
                DateTime::Info info = dti.GetInfo (true);
                if (info.GetKind () == DateTime::Kind::Local)
                    {
                    //local date times are not supported by ECObjects
                    break;
                    }

                DateTime dt;
                DateTime::FromJulianDay (dt, 2456341.75, info);
                value.SetDateTime (dt);
                }
            break;
            }

        case PRIMITIVETYPE_Binary  : 
            {
            Byte blob[]= {0x0c, 0x0b, 0x0c, 0x0b, 0x0c, 0x0b, 65, 66, 67, 68, 0x0c};
            value.SetBinary(blob, 11, true);
            break;
            }
        case PRIMITIVETYPE_Boolean : 
            value.SetBoolean(true); break;
        case PRIMITIVETYPE_Point2D : 
            {
            DPoint2d point2d;
            point2d.x=11.25;
            point2d.y=22.16;
            value.SetPoint2D(point2d);
            break;
            }
        case PRIMITIVETYPE_Point3D :
            {
            DPoint3d point3d;
            point3d.x=11.23;
            point3d.y=22.14;
            point3d.z=33.12;
            value.SetPoint3D(point3d);
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

//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>11/2013</date>
//---------------+---------------+---------------+---------------+---------------+-----
void ECDbTestProject::PopulatePrimitiveValueWithRandomValues (ECValueR ecValue, PrimitiveType primitiveType, ECPropertyCP ecProperty)
    {
    ecValue.Clear();
    
    int randomNumber = rand ();
    switch (primitiveType)
        {
        case PRIMITIVETYPE_String: 
            {
            Utf8String text;
            text.Sprintf ("Sample text with random number: %d", randomNumber);
            ecValue.SetUtf8CP(text.c_str (), true); 
            }
            break;

        case PRIMITIVETYPE_Integer: 
            {
            ecValue.SetInteger(randomNumber); 
            }
            break;

        case PRIMITIVETYPE_Long: 
            {
            const int32_t intMax = std::numeric_limits<int32_t>::max ();
            const int64_t longValue = static_cast<int64_t> (intMax) + randomNumber;
            ecValue.SetLong(longValue); 
            }
            break;

        case PRIMITIVETYPE_Double: 
            {
            ecValue.SetDouble(randomNumber * PI);
            }
            break;

        case PRIMITIVETYPE_DateTime: 
            {
            DateTime utcTime = DateTime::GetCurrentTimeUtc ();
            ecValue.SetDateTime(utcTime); 
            }
            break;

        case PRIMITIVETYPE_Boolean: 
            {
            ecValue.SetBoolean(randomNumber % 2 != 0); 
            }
            break;

        case PRIMITIVETYPE_Point2D: 
            {
            DPoint2d point2d;
            point2d.x=randomNumber * 1.0;
            point2d.y=randomNumber * 1.8;
            ecValue.SetPoint2D(point2d);
            break;
            }
        case PRIMITIVETYPE_Point3D:
            {
            DPoint3d point3d;
            point3d.x=randomNumber * 1.0;
            point3d.y=randomNumber * 1.8;
            point3d.z=randomNumber * 2.9;
            ecValue.SetPoint3D(point3d);
            break;
            }

        case PRIMITIVETYPE_IGeometry:
            {
            IGeometryPtr line = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(randomNumber, randomNumber*2.0, randomNumber*3.0, 
                                                                                -randomNumber, randomNumber*(-2.0), randomNumber*(-3.0))));
            ecValue.SetIGeometry(*line);
            break;
            }

        default:
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   05/12
---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbTestProject::GetInstances (bvector<IECInstancePtr>& instances, Utf8CP className)
    {
    instances.clear();

    ECClassCP ecClass = GetTestSchemaManager ().GetTestSchema ()->GetClassP (className);
    EXPECT_TRUE (ecClass != nullptr) << "ECDbTestProject::GetInstances> ECClass '" << className << "' not found.";
    if (ecClass == nullptr)
        return ERROR;

    SqlPrintfString ecSql ("SELECT * FROM ONLY [%s].[%s]", ecClass->GetSchema().GetName().c_str(), className);
    ECSqlStatement ecStatement;
    ECSqlStatus status = ecStatement.Prepare (GetECDb(), ecSql.GetUtf8CP());
    EXPECT_EQ ((int) ECSqlStatus::Success, (int) status) << "ECDbTestProject::GetInstances> Preparing ECSQL '" << ecSql.GetUtf8CP () << "' failed.";
    if (status != ECSqlStatus::Success)
        return ERROR;

    ECInstanceECSqlSelectAdapter adapter (ecStatement);
    while (ECSqlStepStatus::HasRow == ecStatement.Step())
        {
        IECInstancePtr instance = adapter.GetInstance();
        BeAssert (instance.IsValid());
        if (instance != nullptr)
            instances.push_back (instance);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECDbTestSchemaManager const& ECDbTestProject::GetTestSchemaManager () const
    {
    return m_testSchemaManager;
    }

//*************** ECDbTestUtility **********************************************************

//Datetimes in ECDb are stored as JulianDays as doubles. Accuracy therefore is approx. at the milliseconds level only
static const int DATETIME_ACCURACY_TOLERANCE_HNS = 5000;


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  03/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDbTestUtility::CreateECDb (ECDbR ecdb, BeFileNameP ecdbFullPath, WCharCP ecdbFileName)
    {
    BeFileName outputDir;
    BeTest::GetHost ().GetOutputRoot (outputDir);
    BeFileName fullPath (nullptr, outputDir.GetName (), ecdbFileName, nullptr);
    if (BeFileName::DoesPathExist (fullPath.GetName ()))
        {
        // Delete any previously created file
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (fullPath.GetName ());
        if (fileDeleteStatus != BeFileNameStatus::Success)
            return BE_SQLITE_ERROR;
        }

    if (ecdbFullPath != nullptr)
        *ecdbFullPath = fullPath;

    return ecdb.CreateNewDb (fullPath.GetNameUtf8 ().c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSchemaPtr ECDbTestUtility::ReadECSchemaFromDisk (WCharCP ecSchemaFileName, WCharCP ecSchemaSearchPath)
    {
    ECSchemaPtr schema = nullptr;
    ECSchemaReadContextPtr context = nullptr;
    ReadECSchemaFromDisk (schema, context, ecSchemaFileName, ecSchemaSearchPath);
    EXPECT_TRUE (schema.IsValid ());
    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void ECDbTestUtility::ReadECSchemaFromDisk 
(
ECSchemaPtr& ecSchema, 
ECSchemaReadContextPtr& ecSchemaContext, 
WCharCP ecSchemaFileName,
WCharCP ecSchemaSearchPath
)
    {
    // Construct the path to the sample schema
    BeFileName ecSchemaPath;
    if (ecSchemaSearchPath == nullptr)
        {
        BeTest::GetHost().GetDocumentsRoot (ecSchemaPath);
        ecSchemaPath.AppendToPath (L"DgnDb");
        ecSchemaPath.AppendToPath (L"ECDb");
        ecSchemaPath.AppendToPath (L"Schemas");
        }
    else
        {
        ecSchemaPath.SetName(ecSchemaSearchPath);
        }

    BeFileName ecSchemaFile (ecSchemaPath);
    ecSchemaFile.AppendToPath (ecSchemaFileName);
    ASSERT_TRUE (BeFileName::DoesPathExist (ecSchemaFile.GetName()));
    
    // Read the sample schema
    if (!ecSchemaContext.IsValid())
        ecSchemaContext = ECSchemaReadContext::CreateContext ();
    ecSchemaContext->AddSchemaPath (ecSchemaPath.GetName());

    SchemaReadStatus ecSchemaStatus = ECSchema::ReadFromXmlFile (ecSchema, ecSchemaFile.GetName (), *ecSchemaContext);
    if (ecSchemaStatus == SCHEMA_READ_STATUS_Success)
        {
        ASSERT_TRUE (ecSchema.IsValid ());
        return;
        }

    ASSERT_EQ (ecSchemaStatus, SCHEMA_READ_STATUS_Success);
    ASSERT_TRUE (ecSchema.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbTestUtility::ReadECSchemaFromString (ECSchemaReadContextPtr& schemaContext, Utf8CP ecschemaXmlString)
    {
    ECSchemaPtr schema = nullptr;
    const auto stat = ECSchema::ReadFromXmlString (schema, ecschemaXmlString, *schemaContext);
    return stat == SCHEMA_READ_STATUS_Success ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECN::ECSchemaCachePtr ECDbTestUtility::ReadECSchemaFromString (Utf8CP ecschemaXmlString)
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext ();
    ECSchemaPtr schema = nullptr;
    ECSchema::ReadFromXmlString (schema, ecschemaXmlString, *context);

    ECSchemaCachePtr schemaCache = ECSchemaCache::Create ();
    schemaCache->AddSchema (*schema);

    return schemaCache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbTestUtility::WriteECSchemaToDisk (ECSchemaCR ecSchema, WCharCP filenameNoVerExt)
    {
    // Construct the pathname to be written
    WCharCP tmpFilename = (filenameNoVerExt != nullptr) ? filenameNoVerExt : WString(ecSchema.GetName().c_str(), BentleyCharEncoding::Utf8).c_str();
    size_t size = wcslen(tmpFilename) + wcslen (L".VV.vv.ecschema.xml") + 1;
    WCharP schemaFilename = (wchar_t*) malloc(size * sizeof(wchar_t));
    BeStringUtilities::Snwprintf (schemaFilename, size, L"%ls.%02d.%02d.ecschema.xml", tmpFilename, ecSchema.GetVersionMajor(), ecSchema.GetVersionMinor());
    BeFileName schemaFile;
    BeTest::GetHost().GetOutputRoot (schemaFile);
    schemaFile.AppendToPath (schemaFilename);
    free (schemaFilename);
    WCharCP schemaPathname = schemaFile.GetName();

     // Delete any existing file
    if (BeFileName::DoesPathExist (schemaPathname))
        {
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (schemaPathname);
        ASSERT_TRUE (fileDeleteStatus == BeFileNameStatus::Success);
        }

    // Write the file
    SchemaWriteStatus schemaWriteStatus = ecSchema.WriteToXmlFile (schemaPathname);
    ASSERT_EQ (schemaWriteStatus, SCHEMA_WRITE_STATUS_Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool CompareRelationships (IECRelationshipInstanceCR a, IECRelationshipInstanceCR b)
    {
    if (a.GetSource () == nullptr || b.GetSource() == nullptr || a.GetSource()->GetInstanceId() != b.GetSource()->GetInstanceId())
        return false;
    if (a.GetTarget() == nullptr || b.GetTarget () == nullptr || a.GetTarget()->GetInstanceId() != b.GetTarget()->GetInstanceId())
        return false;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool CompareProperties (IECInstanceCR actual, ECValuesCollectionCR expected)
    {
    for (ECPropertyValueCR expectedPropertyValue : expected)
        {
        ECValueAccessorCR valueAccessor = expectedPropertyValue.GetValueAccessor ();
        const Utf8String propertyName = valueAccessor.GetPropertyName ();

        if (expectedPropertyValue.HasChildValues())
            {
            if (!CompareProperties (actual, *expectedPropertyValue.GetChildValues()))
                {
                LOG.infov ("CompareProperties - Failed for child of property %s", propertyName.c_str());
                return false;
                }
            continue;
            }

        ECValue actualValue;
        ECObjectsStatus status = actual.GetValueUsingAccessor(actualValue, valueAccessor);
        if (status != ECOBJECTS_STATUS_Success)
            {
            LOG.infov ("CompareProperties - GetValue failed for %s", propertyName.c_str());
            return false;
            }

        ECValueCR expectedValue = expectedPropertyValue.GetValue ();
        const bool expectedValueIsNull = expectedValue.IsNull ();
        const bool actualValueIsNull = actualValue.IsNull ();

        if (expectedValueIsNull != actualValueIsNull)
            {
            if (expectedValueIsNull)
                {
                LOG.infov ("CompareProperties - Expected NULL value for property '%s' but the actual value was not NULL.", propertyName.c_str());
                }
            else
                {
                LOG.infov ("CompareProperties - Expected a non-NULL value for property '%s' but the actual value was NULL.", propertyName.c_str());
                }
            return false;
            }

        if (expectedValue.Equals(actualValue))
            {
            continue;
            }

        PrimitiveType actualType = actualValue.GetPrimitiveType();
        if (actualType == PRIMITIVETYPE_DateTime)
            {
            //Storing dates in DgnDb only allows accuracy of approx millisecs right now.
            int64_t expectedECTicks = expectedValue.GetDateTimeTicks ();
            int64_t actualECTicks = actualValue.GetDateTimeTicks ();
            if (ECDbTestUtility::CompareECDateTimes (expectedECTicks, actualECTicks))
                {
                continue;
                }
            }

            ValueKind actualKind = actualValue.GetKind();
            Utf8String expectedValueWStr = expectedValue.ToString();
            Utf8String actualValueWstr = actualValue.ToString();
            LOG.infov ("CompareProperties - Values not equal for property '%s' (%d %d) - actual %s expected %s", propertyName.c_str(), actualKind, actualType, actualValueWstr.c_str(), expectedValueWStr.c_str());
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbTestUtility::CompareECInstances (IECInstanceCR expected, IECInstanceCR actual)
    {
    IECRelationshipInstanceCP relExpected = dynamic_cast<IECRelationshipInstanceCP> (&expected);
    IECRelationshipInstanceCP relActual = dynamic_cast<IECRelationshipInstanceCP> (&actual);
    if (relExpected != nullptr || relActual != nullptr)
        {
        if (relExpected == nullptr || relActual == nullptr)
            return false; // both have to be non null
        if (!CompareRelationships (*relExpected, *relActual))
            return false;
        }

    if (&expected.GetClass() == &actual.GetClass() && expected.GetClass().GetPropertyCount(true) == 0 && actual.GetClass().GetPropertyCount(true) == 0)
        return true;
    
    ECValuesCollectionPtr propertyValuesExpected = ECValuesCollection::Create (expected);
    if (propertyValuesExpected.IsNull()) 
        return false;
    return CompareProperties (actual, *propertyValuesExpected);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbTestUtility::CompareJsonWithECInstance (const Json::Value& json, IECInstanceCR referenceInstance)
    {
    IECRelationshipInstanceCP relationshipInstance = dynamic_cast<IECRelationshipInstanceCP> (&referenceInstance);
    if (relationshipInstance != nullptr)
        return false; // Cannot handle relationships!!
        
    ECValuesCollectionPtr propertyValues = ECValuesCollection::Create (referenceInstance);
    if (propertyValues.IsNull()) 
        return false;

    for (ECPropertyValueCR propertyValue : *propertyValues)
        {
        Utf8String propertyName = propertyValue.GetValueAccessor().GetPropertyName();
        Utf8String propertyAccessString = propertyValue.GetValueAccessor().GetAccessString();

        ECValueCR referenceValue = propertyValue.GetValue();
        if (referenceValue.IsNull() && !json.isMember (propertyName))
            continue;
        if (!json.isMember (propertyName))
            return false;

        Json::Value jsonValue = json[propertyName];
        if (!CompareJsonWithECValue (jsonValue, referenceValue, referenceInstance, propertyAccessString.c_str()))
            return false;
        }
    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbTestUtility::CompareJsonWithECValue (const Json::Value& jsonValue, ECValueCR referenceValue, IECInstanceCR referenceInstance, Utf8CP referencePropertyAccessString)
    {
    Json::ValueType jsonValueType = jsonValue.type();

    // NULL values
    if (referenceValue.IsNull())
        return (jsonValueType == Json::nullValue);

    // Primitive values
    if (referenceValue.IsPrimitive())
        return CompareJsonWithECPrimitiveValue (jsonValue, referenceValue);

    // Struct values
    if (referenceValue.IsStruct())
        return CompareJsonWithECStructValue (jsonValue, referenceValue);

    // Array values
    if (referenceValue.IsArray())
        return CompareJsonWithECArrayValue (jsonValue, referenceValue, referenceInstance, referencePropertyAccessString);

    return false;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbTestUtility::CompareJsonWithECPrimitiveValue (const Json::Value& jsonValue, ECValueCR referenceValue)
    {
    if (!referenceValue.IsPrimitive())
        return false;
    Json::ValueType jsonValueType = jsonValue.type();
    PrimitiveType primitiveType = referenceValue.GetPrimitiveType();
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Binary:
            {
            if (jsonValueType != Json::stringValue)
                return false;
            size_t binarySize;
            referenceValue.GetBinary (binarySize);
            SqlPrintfString referenceNativeValue ("BLOB (%d bytes)", (int) binarySize);
            Utf8String jsonNativeValue = jsonValue.asString();
            if (0 != strcmp (referenceNativeValue.GetUtf8CP(), jsonNativeValue.c_str()) )
                return false;
            break;
            }
        case PRIMITIVETYPE_Boolean:
            {
            if (jsonValueType != Json::booleanValue)
                return false;
            bool referenceNativeValue = referenceValue.GetBoolean();
            bool jsonNativeValue = jsonValue.asBool();
            if (referenceNativeValue != jsonNativeValue)
                return false;
            break;
            }
        case PRIMITIVETYPE_DateTime:
            {
            if (jsonValueType != Json::realValue)
                return false;

            const int64_t referenceECTicks = referenceValue.GetDateTimeTicks();
            const double jsonJd = jsonValue.asDouble();
            const int64_t jsonECTicks = JulianDayToCommonEraTicks (jsonJd);
            return CompareECDateTimes (referenceECTicks, jsonECTicks);
            }

        case PRIMITIVETYPE_Double:
            {
            if (jsonValueType != Json::realValue)
                return false;
            double referenceNativeValue = referenceValue.GetDouble();
            double jsonNativeValue = jsonValue.asDouble();
            if (referenceNativeValue != jsonNativeValue)
                return false;
            break;
            }
        case PRIMITIVETYPE_Integer:
            {
            if (jsonValueType != Json::intValue)
                return false;
            int referenceNativeValue = referenceValue.GetInteger();
            int jsonNativeValue = jsonValue.asInt();
            if (referenceNativeValue != jsonNativeValue)
                return false;
            break;
            }
        case PRIMITIVETYPE_Long:
            {
            if (jsonValueType != Json::intValue)
                return false;
            int64_t referenceNativeValue = referenceValue.GetLong();
            int64_t jsonNativeValue = jsonValue.asInt64();
            if (referenceNativeValue != jsonNativeValue)
                return false;
            break;
            }
        case PRIMITIVETYPE_Point2D:
            {
            if (jsonValueType != Json::objectValue)
                return false;
            DPoint2d referenceNativeValue = referenceValue.GetPoint2D();
            DPoint2d jsonNativeValue = DPoint2d::From (jsonValue["X"].asDouble(), jsonValue["Y"].asDouble());
            if (!referenceNativeValue.IsEqual (jsonNativeValue))
                return false;
            break;
            }
        case PRIMITIVETYPE_Point3D:
            {
            if (jsonValueType != Json::objectValue)
                return false;
            DPoint3d referenceNativeValue = referenceValue.GetPoint3D();
            DPoint3d jsonNativeValue = DPoint3d::From (jsonValue["X"].asDouble(), jsonValue["Y"].asDouble(), jsonValue["Z"].asDouble());
            if (!referenceNativeValue.IsEqual (jsonNativeValue))
                return false;
            break;
            }
        case PRIMITIVETYPE_String:
            {
            if (jsonValueType != Json::stringValue)
                return false;
            Utf8String  referenceNativeValue(referenceValue.GetUtf8CP());
            Utf8String jsonNativeValue = jsonValue.asString();
            if (referenceNativeValue != jsonNativeValue)
                return false;
            break;
            }
        default:
            return false;
            break;
        }

    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbTestUtility::CompareJsonWithECStructValue (const Json::Value& jsonValue, ECValueCR referenceValue)
    {
    if (!referenceValue.IsStruct())
        return false;
    Json::ValueType jsonValueType = jsonValue.type();
    if (jsonValueType != Json::objectValue)
        return false;

    IECInstancePtr structInstance = referenceValue.GetStruct();
    if (structInstance.IsNull())
        return false;
    if (!CompareJsonWithECInstance (jsonValue, *structInstance))
        return false;
    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbTestUtility::CompareJsonWithECArrayValue (const Json::Value& jsonValue, ECValueCR referenceValue, IECInstanceCR referenceInstance, Utf8CP referencePropertyAccessString)
    {
    if (referenceValue.IsNull())
        return false;
     if (!referenceValue.IsArray())
        return false;

    Json::ValueType jsonValueType = jsonValue.type();
    if (jsonValueType != Json::arrayValue)
        return false;

    ArrayInfo arrayInfo = referenceValue.GetArrayInfo();
    int referenceCount = arrayInfo.GetCount();
    if (referenceCount != jsonValue.size())
        return false;
    for (int ii=0; ii  <referenceCount; ii++)
        {
        ECValue referenceMemberValue;
        referenceInstance.GetValue (referenceMemberValue, referencePropertyAccessString, ii);
        if (!CompareJsonWithECValue (jsonValue[ii], referenceMemberValue, referenceInstance, referencePropertyAccessString))
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ECDbTestUtility::GetIterableCount (ECCustomAttributeInstanceIterable const& iterable)
    {
    size_t count = 0;
    for (IECInstancePtr instance : iterable)
        count++;
    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ECDbTestUtility::GetIterableCount (ECPropertyIterable const& iterable)
    {
    size_t count = 0;
    for (ECPropertyP p : iterable)
        {
        p; // To avoid unreferenced variable compilation warning
        count++;
        }
    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Krischan.Eberle                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
bool ECDbTestUtility::IsECValueNull 
(
ECValueCR value
)
    {
    return value.IsNull () || (value.IsArray () && value.GetArrayInfo ().GetCount () == 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbTestUtility::IsECValueNullOrEmpty (ECValueCR value)
    {
    if (IsECValueNull (value))
        return true;

    if (value.IsString())
        {
        if (value.IsString())
            {
            Utf8CP valueStr = value.GetUtf8CP();
            if(valueStr == nullptr || valueStr[0] == 0)
                return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Krischan.Eberle                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void ECDbTestUtility::AssertECDateTime 
(
ECValueCR expectedECValue,
const Db& db,
double actualJd
)
    {
    const DateTime expectedDateTime = expectedECValue.GetDateTime ();

    Utf8String assertMessageHeader;
    assertMessageHeader.Sprintf ("EC date time assertion failed for '%s': ", expectedDateTime.ToUtf8String().c_str ());

    const int64_t expectedCETicks = expectedECValue.GetDateTimeTicks ();
    double expectedJd;
    BentleyStatus stat = expectedECValue.GetDateTime ().ToJulianDay (expectedJd);
    EXPECT_EQ (SUCCESS, stat) << assertMessageHeader.c_str () << L"Julian Day for expected ECValue could not be computed.";

    const int64_t actualCETicks = JulianDayToCommonEraTicks (actualJd);

    AssertECDateTime (expectedCETicks, actualCETicks, assertMessageHeader.c_str ());

    //now compare with SQLite date time functions
    Utf8String sql;
    sql.Sprintf ("select julianday('%s');", expectedDateTime.ToUtf8String ().c_str ());
    Statement sqliteStatement;
    DbResult dbStat = sqliteStatement.Prepare (db, sql.c_str ());
    EXPECT_EQ (BE_SQLITE_OK, dbStat) << assertMessageHeader.c_str () << "SQL statement '" << sql.c_str () << "' could not be prepared";
    dbStat = sqliteStatement.Step ();
    EXPECT_EQ (BE_SQLITE_ROW, dbStat) << assertMessageHeader.c_str () << "SQL statement '" << sql.c_str () << "' could not be executed";;
    const double sqliteJd = sqliteStatement.GetValueDouble (0);
    const int64_t sqliteCETicks = JulianDayToCommonEraTicks (sqliteJd);
    LOG.infov ("Expected JD: %lf - SQLite JD: %lf", expectedJd, sqliteJd);
    assertMessageHeader.Sprintf ("Comparison of Julian Days computed by Bentley::DateTime and SQLite date function failed for '%s'. ", expectedDateTime.ToUtf8String().c_str ());
    AssertECDateTime (expectedCETicks, sqliteCETicks, assertMessageHeader.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Krischan.Eberle                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void ECDbTestUtility::AssertECDateTime 
(
int64_t expectedCETicks,
int64_t actualCETicks,
Utf8CP assertMessageHeader
)
    {
    if (!CompareECDateTimes (expectedCETicks,actualCETicks))
        {
        FAIL () << assertMessageHeader << "Common Era ticks difference outside the tolerance. Expected ticks: " << expectedCETicks << " Actual ticks: " << actualCETicks;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Krischan.Eberle                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
bool ECDbTestUtility::CompareECDateTimes 
(
int64_t expectedECTicks, 
int64_t actualECTicks
)
    {
    int64_t diff = expectedECTicks - actualECTicks;
    //get absolute diff
    if (diff < 0)
        {
        diff *= -1;
        }

    if (diff == 0)
        {
        return true;
        }

    if (diff < DATETIME_ACCURACY_TOLERANCE_HNS)
        {
        LOG.debugv(L"DateTime value differs by %d hecto-nanoseconds but is within tolerance of %d hecto-nanoseconds.", 
                        diff,
                        DATETIME_ACCURACY_TOLERANCE_HNS);
        return true;
        }
    else
        {
        LOG.infov (L"DateTime value differs by %d hecto-nanoseconds which is outside the tolerance of %d hecto-nanoseconds.", 
                        diff,
                        DATETIME_ACCURACY_TOLERANCE_HNS);
        return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbTestUtility::GetClassUsageStatistics
(
size_t& instanceCount,
size_t& propertyCount,
size_t& nullPropertyCount,
size_t& customAttributeInstanceCount,
ECClassCR ecClass, 
ECDbR ecdb
)
    {
    // Initialize default return counts
    instanceCount = 0;
    ECPropertyIterable allPropertiesIterable = ecClass.GetProperties(true);
    propertyCount = GetIterableCount (allPropertiesIterable);
    nullPropertyCount = propertyCount;

    // Find total number of custom attributes
    customAttributeInstanceCount = GetIterableCount (ecClass.GetCustomAttributes (false));
    for (ECPropertyCP p : ecClass.GetProperties (false))
        customAttributeInstanceCount += GetIterableCount (p->GetCustomAttributes (false));

    // Find count of instances of the specified class in the Db
    SqlPrintfString ecSql ("SELECT * FROM [%s].[%s]", Utf8String(ecClass.GetName()).c_str(), Utf8String(ecClass.GetSchema().GetName()).c_str());
    ECSqlStatement ecStatement;
    ECSqlStatus status = ecStatement.Prepare (ecdb, ecSql.GetUtf8CP());
    ASSERT_TRUE (ECSqlStatus::Success == status);

    bvector<IECInstancePtr> instances;
    ECInstanceECSqlSelectAdapter adapter (ecStatement);
    while (ECSqlStepStatus::HasRow == ecStatement.Step())
        {
        IECInstancePtr instance = adapter.GetInstance();
        BeAssert (instance.IsValid());
        instances.push_back (instance);
        }
    instanceCount = instances.size();

    // Find count of null properties (properties that are null in *all* instances)
    bset<unsigned int> nonNullPropertyIndices;
    for (IECInstancePtr instance : instances)
        {         
        ASSERT_TRUE  (instance.IsValid());
        int propertyIndex = 0;
        for (ECPropertyCP p : allPropertiesIterable)
            {
            ECValue v;
            instance->GetValue (v, p->GetName().c_str());
            if (!IsECValueNullOrEmpty (v))
                nonNullPropertyIndices.insert (propertyIndex);
            propertyIndex++;
            }
        }
    ASSERT_TRUE ((size_t)nonNullPropertyIndices.size() <= propertyCount);
    nullPropertyCount = propertyCount - nonNullPropertyIndices.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbTestUtility::DumpECSchemaUsageStatistics (ECSchemaCR schema, ECDbR ecdb, bool dumpEmptyClasses)
    {
    LOG.infov ("ECSchema: %s.%02d.%02d", schema.GetName().c_str(), schema.GetVersionMajor(), schema.GetVersionMinor());
    size_t totalClassCount = 0;
    size_t totalUsedClassCount = 0;
    size_t totalCustomAttributeClassCount = 0;
    size_t totalCustomAttributeInstanceCount = GetIterableCount (schema.GetCustomAttributes (false));
    size_t totalPropertyCount = 0;
    size_t totalNullPropertyCount = 0;

    for (ECClassP ecClass : schema.GetClasses())
        {
        totalClassCount++;
        if (ecClass->GetIsCustomAttributeClass())
            {
            totalCustomAttributeClassCount++;
            LOG.infov ("    ECClass: %-40s\t\t(Custom Attribute Class)", ecClass->GetName().c_str());
            continue;
            }

        ECInstanceInserter inserter (ecdb, *ecClass);
        if (!inserter.IsValid ())
            {
            LOG.infov ("    ECClass: %-40s\t\t(Not mapped to any table)", ecClass->GetName().c_str());
            continue;
            }

        size_t instanceCount, propertyCount, nullPropertyCount, customAttributeInstanceCount;
        GetClassUsageStatistics (instanceCount, propertyCount, nullPropertyCount, customAttributeInstanceCount, 
            *ecClass, ecdb);
        if (instanceCount > 0 && dumpEmptyClasses)
            {
            LOG.infov ("    ECClass: %-40s: %5d instances, %5d properties, %5d null-properties, %5d cust-attr-instances", 
                ecClass->GetName().c_str(), instanceCount, propertyCount, nullPropertyCount, customAttributeInstanceCount);
            }
        if (instanceCount > 0) totalUsedClassCount++;
        totalCustomAttributeInstanceCount += customAttributeInstanceCount;
        totalPropertyCount += propertyCount;
        totalNullPropertyCount += nullPropertyCount;
        }

    LOG.infov ("%-30s = %5d", "Total Classes", totalClassCount);
    LOG.infov ("%-30s = %5d", "Total CustAttr Classes", totalCustomAttributeClassCount);
    LOG.infov ("%-30s = %5d", "Total Used Classes", totalUsedClassCount);
    LOG.infov ("%-30s = %5d", "Total Unused Classes", totalClassCount - totalUsedClassCount - totalCustomAttributeClassCount);
    LOG.infov ("%-30s = %5d", "Total Custom Attr Instances", totalCustomAttributeInstanceCount);
    LOG.infov ("%-30s = %5d", "Total Property Count", totalPropertyCount);
    LOG.infov ("%-30s = %5d", "Total Null Property Count", totalNullPropertyCount);
    LOG.infov ("");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t ECDbTestUtility::ReadCellValueAsInt64 (DbR db, Utf8CP tableName, Utf8CP columnName, Utf8CP whereClause)
    {
    Utf8String str;
    str.Sprintf ("SELECT %s FROM %s %s", columnName, tableName, whereClause);
    Statement stmt;
    DbResult result = stmt.Prepare (db, str.c_str());
    BeAssert (result == BE_SQLITE_OK);
    return (BE_SQLITE_ROW != stmt.Step()) ? 0 : stmt.GetValueInt64(0);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbTestUtility::DebugDumpJson (const Json::Value& jsonValue)
    {
    Utf8String strValue = Json::StyledWriter().write (jsonValue);
    int len = (int) strValue.size();
    for (int ii = 0; ii < len; ii += 1000)
        {
        // Split the string up - logging can't seem to handle long strings
        Utf8String subStr = strValue.substr (ii, 1000);
        LOG.infov ("%s", subStr.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   02/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
int64_t ECDbTestUtility::JulianDayToCommonEraTicks (double jd)
    {
    uint64_t jdHns = DateTime::RationalDayToHns (jd);
    return DateTime::JulianDayToCommonEraTicks (jdHns);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbTestUtility::SetECInstanceId (ECN::IECInstanceR instance, ECInstanceId const& instanceId)
    {
    if (!instanceId.IsValid ())
        return ERROR;

    Utf8Char instanceIdStr[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH];
    if (!ECInstanceIdHelper::ToString (instanceIdStr, ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH, instanceId))
        {
        LOG.errorv ("Could not set ECInstanceId %lld on the ECInstanceId. Conversion to string failed.", instanceId.GetValue ());
        return ERROR;
        }

    const auto ecstat = instance.SetInstanceId (instanceIdStr);
    return ecstat == ECOBJECTS_STATUS_Success ? SUCCESS : ERROR;
    }

END_ECDBUNITTESTS_NAMESPACE