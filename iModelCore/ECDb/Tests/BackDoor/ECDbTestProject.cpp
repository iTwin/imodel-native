/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/ECDbTestProject.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//static
bool ECDbTestProject::s_isInitialized = false; 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbTestProject::ECDbTestProject () : m_ecdb (new ECDb) { Initialize (); }

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
bool ECDbTestProject::IsInitialized () {return s_isInitialized; }   

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                11/2012
//+---------------+---------------+---------------+---------------+---------------+-
Utf8CP ECDbTestProject::GetECDbPath () const { return GetECDbCR ().GetDbFileName (); }

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
    ECSchemaPtr schema = nullptr;
    if (ImportECSchema (schema, testSchemaXmlFileName) != SUCCESS)
        return GetECDb ();
         
    if (numberOfArbitraryECInstancesToImport > 0)
        ImportArbitraryECInstances (*schema, numberOfArbitraryECInstancesToImport);

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
ECDbR ECDbTestProject::GetECDb () { return *m_ecdb; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbCR ECDbTestProject::GetECDbCR () const { return *m_ecdb; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbTestProject::ImportECSchema (ECN::ECSchemaPtr& schema, WCharCP testSchemaXmlFileName)
    {
    if (WString::IsNullOrEmpty(testSchemaXmlFileName))
        return ERROR;

    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
    ECDbTestUtility::ReadECSchemaFromDisk(schema, schemaReadContext, testSchemaXmlFileName);
    EXPECT_TRUE(schema != nullptr);
    if (schema == nullptr)
        return ERROR;

    if (SUCCESS != GetECDbCR().Schemas().ImportECSchemas(schemaReadContext->GetCache(), ECDbSchemaManager::ImportOptions(true, false)))
        return ERROR;

    GetECDb().SaveChanges();
    return SUCCESS;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbTestProject::ImportArbitraryECInstances(ECSchemaCR schema, int numberOfECInstances)
    {
    StopWatch stopwatch ("", true);
    int classCount = 0;
    for (ECClassCP ecClass : schema.GetClasses())
        {
        if (ecClass->GetRelationshipClassCP())
            continue; // skip relationships
        
        classCount++;
        for (int i = 0; i < numberOfECInstances; i++)
            {
            IECInstancePtr ecInstance = ECDbTestUtility::CreateArbitraryECInstance (*ecClass);
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
BentleyStatus ECDbTestProject::InsertECInstance (ECInstanceKey& ecInstanceKey, IECInstancePtr ecInstance)
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
* @bsimethod                                   Ramanujam.Raman                   05/12
---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbTestProject::GetInstances (bvector<IECInstancePtr>& instances, Utf8CP schemaName, Utf8CP className)
    {
    instances.clear();

    ECClassCP ecClass = GetECDb().Schemas().GetECClass (schemaName, className);
    EXPECT_TRUE (ecClass != nullptr) << "ECDbTestProject::GetInstances> ECClass '" << className << "' not found.";
    if (ecClass == nullptr)
        return ERROR;

    SqlPrintfString ecSql ("SELECT * FROM ONLY [%s].[%s]", ecClass->GetSchema().GetName().c_str(), className);
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

END_ECDBUNITTESTS_NAMESPACE