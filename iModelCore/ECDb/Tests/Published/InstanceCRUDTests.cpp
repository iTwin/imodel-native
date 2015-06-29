#include "ECDbPublishedTests.h"
#include <Bentley/BeDirectoryIterator.h>
#include <ECObjects/ECObjectsAPI.h>
#include <ECDb/ECDbApi.h>
#include <BeSQLite/BeSQLite.h>
#include "RandomECInstanceGenerator.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <stdlib.h>

//Use own logger for these tests
#define LOG1 (*NativeLogging::LoggingManager::GetLogger (L"CRUDTests"))

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_LOGGING
using namespace Tests;

/*================================================================================**//**
* @bsiclass                                                     Majd Uddin      09/14
+===============+===============+===============+===============+===============+======*/
struct InstanceCRUDTests : public ::testing::Test
    {
    ECDb                        m_db;
    ECSchemaPtr                 m_schema;
    WString						m_classDbName;
    WString                     m_dbName;
    WString                     m_schemaFullPath;
    WString                     m_dirName;
    WString                     m_className;
    bool                        m_insert = false;
    bool                        m_update = false;
    bool                        m_delete = false;
    std::vector<ECClassCP>      m_classList;
    bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>>      m_generatedInstances;
    bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>>      m_generatedRelationshipInstances;

    static void initBeSQLiteLib ();
    static bool addClass (ECClassCP ecClass);
    void checkECClassCRUDfeasibility (ECClassCP ecClass);
    ECClassCP addClassInPlaceOFAnyClass (ECSchemaPtr schemaPtr);

    bool setUpDefaultECdbAndImportSchema ();
    void setUpECDbForClass ();
    bool setUpECdbSingleClass ();
    void createDb ();
    bool importSchema (WString schemaNameWithoutVerionAndExtension);

    int insertECInstances (bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & generatedInstances);
    int countECInstnacesAndCompare (bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & generatedInstances);
    void addClassesForRelationship (ECRelationshipClassCP relClass);
    void insertECClassInstances (ECClassCP ecClass);
    void insertECRelationshipClassInstances ();

    void deleteECClassInstances (ECClassCP ecClass);
    void deleteECRelationshipClassInstances ();
    bool deleteECInstances (bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & instances);

    void updateECClassInstances (ECClassCP ecClass);
    void updateECRelationshipClassInstances ();
    bool updateECInstances (bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & insertedInstances, bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & toUpdateInstances);
    };
/*---------------------------------------------------------------------------------**//**
* Initialize SQLite Lib for the tests
* @bsimethod                           Majd Uddin                   09/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::initBeSQLiteLib ()
    {
    BeFileName assetsDir;
    BeTest::GetHost ().GetDgnPlatformAssetsDirectory (assetsDir);

    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    ECDb::Initialize (temporaryDir, &assetsDir);
    }
/*---------------------------------------------------------------------------------**//**
* Insert ECinstances from given list
* @bsimethod                           Muhammad.Zaighum                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
int InstanceCRUDTests::insertECInstances (bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & generatedInstances)
    {
    int instanceCount = 0;
    for (auto& entry : generatedInstances)
        {
        bool insert = false;
        auto ecClass = entry.first;
        auto instanceList = entry.second;
        ECDbR ecdb = m_db;
        for (auto &instance : instanceList)
            {
            ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP ();
            if (relClass == NULL)
                {
                if (!instance.IsValid ())
                    {
                    EXPECT_TRUE (false) << "Invalid Instance for Class:" << ecClass->GetName ().c_str ();
                    continue;
                    }
                else
                    insert = true;
                }
            else
                {
                IECRelationshipInstancePtr relInstance = dynamic_cast<IECRelationshipInstance*>(instance.get ());
                if (!relInstance.IsValid ())
                    {
                    EXPECT_TRUE (false) << "Invalid Instance for relationship Class:" << ecClass->GetName ().c_str ();
                    continue;
                    }
                else
                    insert = true;
                }
            if (insert)
                {
                Savepoint savepoint (ecdb, "Populate");
                ECInstanceInserter inserter (ecdb, *ecClass);
                auto insertStatus = inserter.Insert (*instance);
                if (insertStatus != BentleyStatus::SUCCESS)
                    {
                    EXPECT_TRUE (false) << "Couldn't Insert Instance for Class:" << ecClass->GetName ().c_str ();
                    }
                else
                    {
                    savepoint.Commit ();
                    instanceCount++;
                    }
                }
            }
        }
    return instanceCount;
    }
/*---------------------------------------------------------------------------------**//**
* Compare ECinstances and return number of ECInstnaces
* @bsimethod                           Muhammad.Zaighum                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
int InstanceCRUDTests::countECInstnacesAndCompare (bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & generatedInstances)
    {
    int numberOfInstnace = 0;
    ECDbR ecdb = m_db;
    for (auto& entry : generatedInstances)
        {
        auto ecClass = entry.first;
        SqlPrintfString ecSql ("SELECT * FROM ONLY [%s].[%s]", Utf8String (ecClass->GetSchema ().GetName ()).c_str (), Utf8String (ecClass->GetName ()).c_str ());
        ECSqlStatement ecStatement;
        ECSqlStatus status = ecStatement.Prepare (ecdb, ecSql.GetUtf8CP ());
        if (ECSqlStatus::Success != status)
            {
            LOG.infov("Could Prepare statement : %s ", ecSql.GetUtf8CP());
            return 0;
            }
        bvector<IECInstancePtr> instances;
        ECInstanceECSqlSelectAdapter adapter (ecStatement);
        while (ECSqlStepStatus::HasRow == ecStatement.Step ())
            {
            IECInstancePtr newInstance = adapter.GetInstance ();
            if (!newInstance.IsValid ())
                continue;
            if (newInstance != nullptr)
                {
                auto const& instanceList = entry.second;
                bool foundStatus = false;
                for_each (instanceList.begin (), instanceList.end (), [&ecdb, &ecClass, &foundStatus, &newInstance](IECInstancePtr const& instance)
                    {
                    if (newInstance->GetInstanceId ().Equals (instance->GetInstanceId ()))
                        {
                        if (ECDbTestUtility::CompareECInstances (*newInstance, *instance))
                            {
                            foundStatus = true;
                            }
                        else
                            {
                            Utf8String ins1;
                            newInstance->WriteToXmlString (ins1, true, true);
                            Utf8String ins2;
                            instance->WriteToXmlString (ins2, true, true);
                            if (!ins1.Equals (ins2))
                                LOG.infov ("ECInstances are not equal:\n %s \n %s ", ins1.c_str (), ins2.c_str ());
                            }
                        }
                    });
                if (foundStatus)
                    {
                    numberOfInstnace++;
                    }
                }
            }
        }
    return numberOfInstnace;
    }
/*---------------------------------------------------------------------------------**//**
* Skips any classes that should not be added for testing
* @bsimethod                           Majd Uddin                       10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceCRUDTests::addClass (ECClassCP ecClass)
    {
    //Check if NULL
    if (ecClass == NULL)
        {
        LOG1.info ("ecClass is NULL");
        return false;
        }
    //Skip Abstract Classes
    else if (!ecClass->GetIsDomainClass ())
        {
        LOG1.info ("This is not a Domain Class");
        return false;
        }
    //Skip CustomAttribute Classes
    else if (ecClass->GetIsCustomAttributeClass ())
        {
        LOG1.info ("This is a CustomAttributeClass");
        return false;
        }
    else if (ecClass->GetName () == L"AnyClass")
        {
        return false;
        }
    else
        {
        return true;
        }
    }
//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  1/15
//+---------------+---------------+---------------+---------------+---------------+------
ECClassCP InstanceCRUDTests::addClassInPlaceOFAnyClass (ECSchemaPtr schemaPtr)
    {
    std::vector<ECClassCP> list;
    ECRelationshipClassCP relClass;
    ECClassContainerCR classContainerCR = schemaPtr->GetClasses ();
    FOR_EACH (ECClassCP classInContainer, classContainerCR)
        {
        relClass = NULL;
        relClass = classInContainer->GetRelationshipClassCP ();
        if (relClass == NULL && classInContainer->GetIsDomainClass () && !classInContainer->GetIsCustomAttributeClass () && classInContainer->GetName () != L"AnyClass")
            {
            list.push_back (classInContainer);
            }
        }
    size_t classCount = list.size ();
    std::srand ((unsigned int)time (0));
    size_t secretNumber = rand () % classCount;
    return list.at (secretNumber);
    }
bool InstanceCRUDTests::setUpDefaultECdbAndImportSchema ()
    {
    bool status = false;
    BeFileName dbname (m_schemaFullPath);
    WString schemaName = m_schemaFullPath.substr (m_schemaFullPath.find_last_of (L"\\") + 1, m_schemaFullPath.length ());
    m_dirName = dbname.GetDirectoryName ();
    WString schemaName1 = dbname.GetFileNameWithoutExtension ();
    WString schemaNameWOVAE = schemaName.substr (0, schemaName.find_first_of (L"."));
    schemaNameWOVAE.append (L"_Supplemental");
    WString dbName1 (m_dirName);
    dbName1 = dbName1 + schemaName1;
    dbName1.AppendA (".");
    dbName1.AppendA ("Default");
    dbName1.AppendA (".ecdb");
    m_dbName = dbName1;
    createDb ();
    status = importSchema (schemaNameWOVAE);
    m_db.CloseDb ();
    if (status)
        return true;
    else
        return false;
    }
void InstanceCRUDTests::setUpECDbForClass ()
    {
    BeFileName dbName2 (m_schemaFullPath);
    m_dirName = dbName2.GetDirectoryName ();
    WString schemaName2 = dbName2.GetFileNameWithoutExtension ();
    WString dbName3 (m_dirName);
    dbName3 = dbName3 + schemaName2;
    dbName3.AppendA (".");
    dbName3 = dbName3 + m_className;

    static const size_t JOURNAL_SUFFIX_LENGTH = 8;
    if (dbName3.size () + 6 + JOURNAL_SUFFIX_LENGTH > MAX_PATH)
        {
        LOG1.infov ("Db file path is exceeding the limit(260): %ls", dbName3.c_str ());
        size_t exceededSize = dbName3.size () + 6 + JOURNAL_SUFFIX_LENGTH - MAX_PATH;
        dbName3.resize (dbName3.size () - exceededSize);
        }

    dbName3.AppendA (".ecdb");
    m_classDbName = dbName3;
    WString newdb = dbName3;

    auto stat = BeFileName::BeCopyFile (m_dbName.c_str (), newdb.c_str (), false);
    ASSERT_EQ (stat, BeFileNameStatus::Success);
    BeFileName dbnamee (newdb);

    m_classList.clear ();
    ASSERT_EQ (BE_SQLITE_OK, m_db.OpenBeSQLiteDb (dbnamee, ECDb::OpenParams (Db::OpenMode::ReadWrite))) << "Could not open test file " << newdb.c_str ();
    }
bool InstanceCRUDTests::setUpECdbSingleClass ()
    {
    BeFileName dbname (m_schemaFullPath);
    WString schemaName = m_schemaFullPath.substr (m_schemaFullPath.find_last_of (L"\\") + 1, m_schemaFullPath.length ());
    m_dirName = dbname.GetDirectoryName ();
    WString schemaName1 = dbname.GetFileNameWithoutExtension ();
    WString schemaNameWOVAE = schemaName.substr (0, schemaName.find_first_of (L"."));
    schemaNameWOVAE.append (L"_Supplemental");
    WString dbName1 (m_dirName);
    dbName1 = dbName1 + schemaName1;
    dbName1.AppendA (".");
    dbName1 = dbName1 + m_className;

    static const size_t JOURNAL_SUFFIX_LENGTH = 8;
    if (dbName1.size () + 6 + JOURNAL_SUFFIX_LENGTH > MAX_PATH)
        {
        LOG1.infov ("Db file path is exceeding the limit(260): %ls", dbName1.c_str ());
        size_t exceededSize = dbName1.size () + 6 + JOURNAL_SUFFIX_LENGTH - MAX_PATH;
        dbName1.resize (dbName1.size () - exceededSize);
        }
    dbName1.AppendA (".ecdb");
    m_dbName = dbName1;
    m_classList.clear ();
    createDb ();
    auto status = importSchema (schemaNameWOVAE);
    if (status)
        return true;
    else
        return false;
    }
/*---------------------------------------------------------------------------------**//**
* Create the Db and removes if there is one already
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::createDb ()
    {
    BeFileName dbPath (m_dbName);
    //Remove existing db if any
    if (BeFileName::DoesPathExist (dbPath.GetName ()))
        {
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (dbPath.GetName ());
        if (fileDeleteStatus != BeFileNameStatus::Success)
            {
            LOG1.errorv ("Could not delete preexisting test ecdb file : %ls", m_dbName.c_str ());
            EXPECT_TRUE (false);
            }
        }
    auto stat = m_db.CreateNewDb (dbPath);
    if (stat != SUCCESS)
        {
        LOG1.errorv ("Db could not be created for: %ls", m_dbName.c_str ());
        EXPECT_TRUE (false);
        }
    else
        LOG1.info ("Created ECDb ");
    }
/*---------------------------------------------------------------------------------**//**
* Imports all needed schemas into the Db
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceCRUDTests::importSchema (WString schemaNameWithoutVerionAndExtension)
    {
    bool importStatus = false;
    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext ();
    schemaReadContext->AddSchemaPath (m_dirName.c_str ());

    auto result = ECSchema::ReadFromXmlFile (m_schema, m_schemaFullPath.c_str (), *schemaReadContext);
    EXPECT_EQ (result, SUCCESS);
    auto cache = ECSchemaCache::Create ();
    cache->AddSchema (*m_schema);

    // If there are any Supplemental schemas, they need to be in the cache for supplementation
    BeFileName dir (m_dirName);
    bvector<BeFileName> suppls;
    BeDirectoryIterator::WalkDirsAndMatch (suppls, dir, L"*Supplemental*.ecschema.xml", true);
    for (BeFileName entry: suppls)
        {
        if (entry.find (schemaNameWithoutVerionAndExtension) != WString::npos)
            {
            WString supplSchemaPath (entry);
            ECSchemaPtr schema1;
            auto result = ECSchema::ReadFromXmlFile (schema1, supplSchemaPath.c_str (), *schemaReadContext);
            if (result == SUCCESS)
                cache->AddSchema (*schema1);
            else
                LOG1.infov ("[SRF] Schema Read Failed :%ls \n", entry.GetName ());
            }
        }
    auto importSchemaStatus = m_db.Schemas ().ImportECSchemas (*cache, ECDbSchemaManager::ImportOptions (true, true));
    if (importSchemaStatus == SUCCESS)
        {
        LOG1.infov ("[SIS] Schema Import successful: %ls \n", m_schemaFullPath.c_str ());
        importStatus = true;
        }
    else
        {
        LOG1.errorv ("[SIF] Schema Import Failed: %ls \n", m_schemaFullPath.c_str ());
        EXPECT_TRUE (false);
        }
    if (importStatus)
        return true;
    else
        return false;
    }
/*---------------------------------------------------------------------------------**//**
* Insert instances of a given EC Class. Deals with Relationship class and calls it
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::insertECClassInstances (ECClassCP ecClass)
    {
    bool instatus = false;
    ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP ();
    if (relClass == NULL)
        {
        LOG1.infov ("Starting Insert operation for class: %ls", m_className.c_str ());
        m_classList.push_back (ecClass);
        int inserted = 0;
        int actualCount = 0;

        RandomECInstanceGenerator insert (m_classList);
        auto status = insert.Generate (false);
        if (status != SUCCESS)
            {
            LOG1.errorv ("Instance generation failed for: %ls", m_className.c_str ());
            EXPECT_TRUE (false);
            }
        else
            {
            m_generatedInstances = insert.GetGeneratedInstances ();
            inserted = insertECInstances (m_generatedInstances);
            actualCount = countECInstnacesAndCompare (m_generatedInstances);
            if (inserted <= 0)
                {
                LOG1.errorv ("No instances were inserted for the class: %ls", m_className.c_str ());
                EXPECT_TRUE (false);
                }
            else if (inserted != actualCount)
                {
                LOG1.errorv ("Insert count doesn't match for the class: %ls", m_className.c_str ());
                EXPECT_TRUE (false);
                }
            else
                {
                LOG1.infov ("Instance insertion successful for class: %ls", m_className.c_str ());
                instatus = true;
                }
            }
        if (instatus)
            LOG1.infov ("[IIS] Instance insertion successful for class: %ls \n", m_className.c_str ());
        else
            LOG1.errorv ("[IIF] Insertion failed for class: %ls \n", m_className.c_str ());
        }
    else // This is a Relationship class
        {
        LOG1.infov ("Starting Insert operation for Relationship class: %ls", m_className.c_str ());
        addClassesForRelationship (relClass);
        m_classList.push_back (relClass);
        insertECRelationshipClassInstances ();
        }
    }
/*---------------------------------------------------------------------------------**//**
* Insert instances of a given ECRelationshipClass.
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::insertECRelationshipClassInstances ()
    {
    int inserted = 0;
    int actualCount = 0;
    bool stat = false;
    bool relstat = false;
    if (m_classList.size () == 1)
        {
        LOG1.infov ("Source and Target constraints of this relationship class are AnyClass %ls", m_className.c_str ());
        ECClassCP temp = addClassInPlaceOFAnyClass (m_schema);
        LOG1.infov ("Adding another Class: %ls", temp->GetName ().c_str ());
        m_classList.push_back (temp);
        }
    RandomECInstanceGenerator insert (m_classList);
    RandomECInstanceGenerator moreInstances (m_classList);
    auto status = insert.Generate (false);
    if (status != SUCCESS)
        {
        LOG1.errorv ("no Instances were generated for constraint classes of: %ls", m_className.c_str ());
        EXPECT_TRUE (false);
        }
    else
        {
        m_generatedInstances = insert.GetGeneratedInstances ();
        inserted = insertECInstances (m_generatedInstances);
        actualCount = countECInstnacesAndCompare (m_generatedInstances);
        if (inserted <= 0)
            {
            LOG1.errorv ("No instances were inserted for: %ls", m_className.c_str ());
            EXPECT_TRUE (false);
            }
        else if (inserted != actualCount)
            {
            LOG1.errorv ("Insert count doesn't match for: %ls", m_className.c_str ());
            EXPECT_TRUE (false);
            }
        else
            {
            LOG1.infov ("Instance insertion successfull for Constraints of: %ls", m_className.c_str ());
            stat = true;
            }
        if (stat)
            {
            m_generatedInstances.clear ();
            inserted = 0;
            actualCount = 0;
            status = moreInstances.Generate (false);
            m_generatedInstances = moreInstances.GetGeneratedInstances ();
            inserted = insertECInstances (m_generatedInstances);
            actualCount = countECInstnacesAndCompare (m_generatedInstances);
            EXPECT_FALSE (inserted != actualCount);
            }
        }
    inserted = 0;
    actualCount = 0;
    status = insert.GenerateRelationships ();
    if (status != SUCCESS)
        {
        LOG1.errorv ("Instance Generation failed for Relationship Class: %ls", m_className.c_str ());
        EXPECT_TRUE (false);
        }
    else
        {
        m_generatedRelationshipInstances = insert.GetGeneratedRelationshipInstances ();
        inserted = insertECInstances (m_generatedRelationshipInstances);
        actualCount = countECInstnacesAndCompare (m_generatedRelationshipInstances);
        if (inserted <= 0)
            {
            LOG1.errorv ("No instances were inserted for %ls", m_className.c_str ());
            EXPECT_TRUE (false);
            }
        else if (inserted != actualCount)
            {
            LOG1.errorv ("Insert count doesn't match for %ls", m_className.c_str ());
            EXPECT_TRUE (false);
            }
        else
            {
            LOG1.infov ("Instance insertion successfull for %ls", m_className.c_str ());
            relstat = true;
            }
        if (relstat)
            {
            m_generatedRelationshipInstances.clear ();
            inserted = 0;
            actualCount = 0;
            status = moreInstances.GenerateRelationships ();
            m_generatedRelationshipInstances = moreInstances.GetGeneratedRelationshipInstances ();
            inserted = insertECInstances (m_generatedRelationshipInstances);
            actualCount = countECInstnacesAndCompare (m_generatedRelationshipInstances);
            EXPECT_FALSE (inserted != actualCount);
            }
        }
    if (relstat && stat)
        LOG1.infov ("[IIS] Instance insertion successful for relationship class: %ls \n", m_className.c_str ());
    else
        LOG1.errorv ("[IIF] Insertion failed for Relationship Class: %ls \n", m_className.c_str ());
    }

void InstanceCRUDTests::addClassesForRelationship (ECRelationshipClassCP relClass)
    {
    const ECConstraintClassesList& sourceClasses = relClass->GetSource ().GetClasses ();
    const ECConstraintClassesList& targetClasses = relClass->GetTarget ().GetClasses ();
    for (ECClassCP ecClass2: sourceClasses)
        {
        if (InstanceCRUDTests::addClass (ecClass2))
            {
            m_classList.push_back (ecClass2);
            //For some relationships, it needs the struct Class instances to be there.
            for (ECPropertyCP ecProperty: ecClass2->GetProperties(true))
                {
                if (ecProperty->GetIsStruct ())
                    {
                    ECClassCP structClass = m_schema->GetClassCP (ecProperty->GetTypeName ().c_str ());
                    if (InstanceCRUDTests::addClass (structClass))
                        m_classList.push_back (structClass);
                    }
                }
            }
        //For some cases, we have to add child Domain classes of an Abstract class
        auto const& derivedClasses = ecClass2->GetDerivedClasses ();
        for (ECClassCP ecClass4: derivedClasses)
            {
            if (InstanceCRUDTests::addClass (ecClass4))
                m_classList.push_back (ecClass4);
            }
        }
    for (ECClassCP ecClass3: targetClasses)
        {
        if (InstanceCRUDTests::addClass (ecClass3))
            {
            m_classList.push_back (ecClass3);
            //For some relationships, it needs the struct Class instances to be there.
            for (ECPropertyCP ecProperty: ecClass3->GetProperties(true))
                {
                if (ecProperty->GetIsStruct ())
                    {
                    ECClassCP structClass2 = m_schema->GetClassCP (ecProperty->GetTypeName ().c_str ());
                    if (InstanceCRUDTests::addClass (structClass2))
                        m_classList.push_back (structClass2);
                    }
                }
            }
        //For some cases, we have to add child Domain classes of an Abstract class
        auto const& derivedClasses = ecClass3->GetDerivedClasses ();
        for (ECClassCP ecClass5: derivedClasses)
            {
            if (InstanceCRUDTests::addClass (ecClass5))
                m_classList.push_back (ecClass5);
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* Deletes all instances of provided class. Takes care of Relationship class case
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::deleteECClassInstances (ECClassCP ecClass)
    {
    ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP ();
    if (relClass == NULL)
        {
        LOG1.infov ("Starting delete operation for class: %ls", m_className.c_str ());
        if (deleteECInstances (m_generatedInstances))
            LOG1.infov ("[IDS] Instances were deleted for class: %ls \n", m_className.c_str ());
        else
            {
            LOG1.errorv ("[IDF] Instance deletion Failed for class: %ls \n", m_className.c_str ());
            EXPECT_TRUE (false);
            }
        }
    else //This is relationshipClass
        {
        LOG1.infov ("Starting delete operation for relationship class: %ls", m_className.c_str ());
        deleteECRelationshipClassInstances ();
        }
    }
/*---------------------------------------------------------------------------------**//**
* Deletes all instances of Relationship class
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::deleteECRelationshipClassInstances ()
    {
    if (deleteECInstances (m_generatedRelationshipInstances))
        LOG1.infov ("[IDS] Instances were deleted for Relationship Class: %ls \n", m_className.c_str ());
    else
        {
        LOG1.errorv ("[IDF] Instance Deletion failed for Relationship Class: %ls \n", m_className.c_str ());
        EXPECT_TRUE (false);
        }
    }
/*---------------------------------------------------------------------------------**//**
* The method that does the deletion and returns delete count
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceCRUDTests::deleteECInstances (bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & instances)
    {
    bool delstatus = true;
    for (auto& entry : instances)
        {
        if (delstatus)
            {
            auto ecClass = entry.first;
            auto const& instanceList = entry.second;
            for (auto &instance : instanceList)
                {
                ECInstanceDeleter deleter (m_db, *ecClass);
                if (!instance.IsValid ())
                    continue;
                WString i = instance->GetInstanceId ();
                auto deleteStatus = deleter.Delete (*instance.get ());
                if (deleteStatus != SUCCESS)
                    {
                    LOG1.errorv ("Could not delete ECInstance of ECClass %ls", m_className.c_str ());
                    EXPECT_TRUE (false);
                    delstatus = false;
                    break;
                    }
                }
            }
        else
            break;
        }
    return delstatus;
    }
/*---------------------------------------------------------------------------------**//**
* Updatess all instances of provided class. Takes care of Relationship class case
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::updateECClassInstances (ECClassCP ecClass)
    {
    bool upstatus = false;
    ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP ();
    if (relClass == NULL)
        {
        LOG1.infov ("Starting update operation for class: %ls", m_className.c_str ());
        RandomECInstanceGenerator update (m_classList);
        auto status = update.Generate (false);
        if (status != SUCCESS)
            {
            LOG1.errorv ("Failed to generate random instances for %s", m_className.c_str ());
            EXPECT_TRUE (false);
            }
        else
            {
            auto updateInstances = update.GetGeneratedInstances ();
            if (updateECInstances (m_generatedInstances, updateInstances))
                {
                LOG1.infov ("Instances were updated for class: %ls", m_className.c_str ());
                upstatus = true;
                }
            else
                {
                LOG1.errorv ("No instances were updated of ECClass %ls", m_className.c_str ());
                EXPECT_TRUE (false);
                }
            }
        if (upstatus)
            LOG1.infov ("[IUS] Instances were updated for class: %ls \n", m_className.c_str ());
        else
            LOG1.errorv ("[IUF] Instance update failed for class: %ls \n", m_className.c_str ());
        }
    else //This is relationshipClass
        {
        LOG1.infov ("Starting update operation for relationship class: %ls", m_className.c_str ());
        updateECRelationshipClassInstances ();
        }
    }
/*---------------------------------------------------------------------------------**//**
* Updates all instances of Relationship class
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::updateECRelationshipClassInstances ()
    {
    bool relstatus = false;
    RandomECInstanceGenerator update (m_classList);
    auto status = update.Generate (false);
    if (status != SUCCESS)
        {
        LOG1.errorv ("Instance Generation failed for Constrainst classes of: %ls", m_className.c_str ());
        EXPECT_TRUE (false);
        }
    else
        {   //no need to update relationship constraint Classes instances because we are performing this operation seperately for every class
        auto generatedInstances = update.GetGeneratedInstances ();
        int inserted = insertECInstances (generatedInstances);
        int actualCount = countECInstnacesAndCompare (generatedInstances);
        EXPECT_FALSE (inserted != actualCount);
        }
    status = update.GenerateRelationships ();
    if (status != SUCCESS)
        {
        LOG1.errorv ("Instance Generation failed for relationship : %ls", m_className.c_str ());
        EXPECT_TRUE (false);
        }
    else
        {
        auto updatedRelationInstances = update.GetGeneratedRelationshipInstances ();
        if (updateECInstances (m_generatedRelationshipInstances, updatedRelationInstances))
            {
            LOG1.infov ("Instances were updated for Relationship: %ls", m_className.c_str ());
            relstatus = true;
            }
        else
            {
            LOG1.errorv ("No instances were updated for: %ls", m_className.c_str ());
            EXPECT_TRUE (false);
            }
        }
    if (relstatus)
        {
        LOG1.infov ("[IUS] Instances update successfull for relationship Class: %ls \n", m_className.c_str ());
        }
    else
        LOG1.errorv ("[IUF] Instance update failed for Relationship Class: %ls \n", m_className.c_str ());
    }
/*---------------------------------------------------------------------------------**//**
* The method that does the update and returns update count
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceCRUDTests::updateECInstances (bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & insertedInstances, bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & toUpdateInstances)
    {
    bool upstat = true;
    for (auto insertEntry = insertedInstances.begin (),
         updateEntry = toUpdateInstances.begin ();
         insertEntry != insertedInstances.end (); ++insertEntry, ++updateEntry)
        {
        if (upstat)
            {
            auto ecClass = updateEntry->first;
            for (auto updateinstance = updateEntry->second.begin (),
                 insertedInstance = insertEntry->second.begin ();
                 insertedInstance != insertEntry->second.end ();
            updateinstance++, insertedInstance++)
                {
                if (insertedInstance->IsValid () || updateinstance->IsValid ())
                    {
                    updateinstance->get ()->SetInstanceId (insertedInstance->get ()->GetInstanceId ().c_str ());
                    ECInstanceUpdater updater (m_db, *ecClass);
                    auto updateStatus = updater.Update (*(updateinstance->get ()));
                    if (updateStatus != SUCCESS)
                        {
                        LOG1.errorv ("Could not update ECInstance of ECClass %ls \n", m_className.c_str ());
                        EXPECT_TRUE (false);
                        upstat = false;
                        break;
                        }
                    }
                else
                    {
                    LOG1.errorv ("Either inserted or updateinstance is invalid for class: %ls \n", m_className.c_str ());
                    EXPECT_TRUE (false);
                    }
                }
            }
        else
            break;
        }
    return upstat;
    }
//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  1/15
//+---------------+---------------+---------------+---------------+---------------+------
void InstanceCRUDTests::checkECClassCRUDfeasibility (ECClassCP ecClass)
    {
    m_insert = false;
    m_update = false;
    m_delete = false;
    if (ecClass->GetIsDomainClass () && !ecClass->GetIsCustomAttributeClass () && ecClass->GetName () != L"AnyClass")
        {
        ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP ();
        if (relClass == NULL)
            {
            if (ecClass->GetPropertyCount (true) > 0)
                {
                m_insert = true;
                m_update = true;
                m_delete = true;
                LOG1.infov ("CRUD operations can be performed for class: %ls", ecClass->GetName ().c_str ());
                }
            else
                {
                m_insert = true;
                m_update = false;
                m_delete = true;
                LOG1.infov ("Property Count Zero: update operation cannot be performed for this class %ls", ecClass->GetName ().c_str ());
                }
            }
        else
            {
            const ECConstraintClassesList& sourceClasses = relClass->GetSource ().GetClasses ();
            const ECConstraintClassesList& targetClasses = relClass->GetTarget ().GetClasses ();
            if (sourceClasses.empty () || targetClasses.empty ())
                {
                LOG1.infov ("Empty Source or Target Constraints: This is not a valid Relationship Class", relClass->GetName ().c_str ());
                }
            else
                {
                bool validSourceClasses = true;
                bool validTargetClasses = true;
                bool isDomainOrNonCustom = true;
                bool notAnyClass = true;
                for (ECClassCP s_ecClass : sourceClasses)
                    {
                    if (s_ecClass->GetIsDomainClass () && !s_ecClass->GetIsCustomAttributeClass ())
                        {
                        if (s_ecClass->GetPropertyCount (true) > 0)
                            continue;
                        else
                            {
                            LOG1.infov ("Constraint Class %ls of relationship class %ls has zero property count", s_ecClass->GetName ().c_str (), ecClass->GetName ().c_str ());
                            validSourceClasses = false;
                            }
                        }
                    else if (!s_ecClass->GetIsDomainClass () && s_ecClass->GetName () != L"AnyClass")
                        {
                        if (s_ecClass->GetDerivedClasses ().size () > 0)
                            continue;
                        else
                            isDomainOrNonCustom = false;
                        }
                    else
                        {
                        if (s_ecClass->GetName () == L"AnyClass")
                            notAnyClass = false;
                        else
                            {
                            isDomainOrNonCustom = false;
                            break;
                            }
                        }
                    }
                for (ECClassCP t_ecClass : targetClasses)
                    {
                    if (t_ecClass->GetIsDomainClass () && !t_ecClass->GetIsCustomAttributeClass ())
                        {
                        if (t_ecClass->GetPropertyCount (true) > 0)
                            continue;
                        else
                            {
                            LOG1.infov ("Constraint Class %ls of relationship class %ls has zero property count", t_ecClass->GetName ().c_str (), ecClass->GetName ().c_str ());
                            validSourceClasses = false;
                            }
                        }
                    else if (!t_ecClass->GetIsDomainClass () && t_ecClass->GetName () != L"AnyClass")
                        {
                        if (t_ecClass->GetDerivedClasses ().size () > 0)
                            continue;
                        else
                            isDomainOrNonCustom = false;
                        }
                    else
                        {
                        if (t_ecClass->GetName () == L"AnyClass")
                            notAnyClass = false;
                        else
                            {
                            isDomainOrNonCustom = false;
                            break;
                            }
                        }
                    }
                if (validSourceClasses && validTargetClasses && isDomainOrNonCustom && notAnyClass)
                    {
                    LOG1.infov ("CRUD Operations can be performed for this relationship Class: %ls", ecClass->GetName ().c_str ());
                    m_insert = true;
                    m_update = true;
                    m_delete = true;
                    }
                else if (!isDomainOrNonCustom)
                    {
                    LOG1.infov ("CRUD Operations cannot be performed for the relationship Class: %ls", ecClass->GetName ().c_str ());
                    }
                else if (!validSourceClasses || !validTargetClasses || !notAnyClass)
                    {
                    LOG1.infov ("Insert and Delete Operations can be performed but update cannot be performed for this relationship class %ls", ecClass->GetName ().c_str ());
                    m_insert = true;
                    m_update = false;
                    m_delete = true;
                    }
                }
            }
        }
    else
        {
        if (!ecClass->GetIsDomainClass ())
            {
            LOG1.infov ("[NDC]Non-domain Class: CRUD opertions cann't be performed for class: %ls", ecClass->GetName ().c_str ());
            }
        else if (ecClass->GetIsCustomAttributeClass ())
            {
            LOG1.infov ("[CAC]Custom Attribute Class: CRUD operations cannot be performed for class: %ls", ecClass->GetName ().c_str ());
            }
        else if (ecClass->GetName () == L"AnyClass")
            {
            LOG1.infov ("AnyClass: CRUD operations cannot be performed: %ls", ecClass->GetName ().c_str ());
            }
        }
    }
TEST_F (InstanceCRUDTests, InsertUpdateDeleteTest)
    {
    InstanceCRUDTests::initBeSQLiteLib ();
    BeFileName productSchemas;
    BeTest::GetHost ().GetDgnPlatformAssetsDirectory (productSchemas);
    productSchemas.AppendToPath (L"BeTestDocuments\\DgnDb\\BentleyProductSchema");
    if (BeFileName::DoesPathExist (productSchemas.GetName ()))
        {
		WString tempDirName;
        BeFileName entryName;
        bool isDir;
        for (BeDirectoryIterator dir (productSchemas); dir.GetCurrentEntry (entryName, isDir) == SUCCESS; dir.ToNext ())
            {
			tempDirName = entryName;
            if (isDir)
                {
                WString schemaPath;
                schemaPath = entryName + L"\\";
                entryName.AppendToPath (L"*.ecschema.xml");
                BeFileListIterator fileIterator (entryName, false);
                BeFileName name;
                int schemaCount = 0;
                while (fileIterator.GetNextFileName (name) != ERROR)
                    {
                    schemaCount++;
                    if (name.find (L"Supplemental") != WString::npos)
                        {
                        continue;
                        }
                    else
                        {
                        ECSchemaReadContextPtr schemaReadContext = nullptr;
                        ECSchemaPtr schemaPtr = nullptr;
                        WString schemaName = name.substr (name.find_last_of (L"\\") + 1, name.length ());
                        ECDbTestUtility::ReadECSchemaFromDisk (schemaPtr, schemaReadContext, schemaName.c_str (), schemaPath.c_str ());
                        if (schemaPtr != NULL)
                            {
                            if (schemaPtr->IsStandardSchema () || schemaPtr->IsSystemSchema ())
                                continue;
                            m_schemaFullPath = name;
                            if (setUpDefaultECdbAndImportSchema ())
                                {
                                auto const& classes = schemaPtr->GetClasses ();
                                for (ECClassCP ecClass : classes)
                                    {
                                    checkECClassCRUDfeasibility (ecClass);
                                    if (m_insert || m_update || m_delete)
                                        {
                                        m_className = ecClass->GetName ();
                                        setUpECDbForClass ();
                                        ECClassCP ecClassToInsertInstances = m_schema->GetClassCP (m_className.c_str ());
                                        if (m_insert)
                                            insertECClassInstances (ecClassToInsertInstances);
                                        if (m_update)
                                            updateECClassInstances (ecClassToInsertInstances);
                                        if (m_delete)
                                            {
                                            m_db.CloseDb ();
                                            setUpECDbForClass ();
                                            insertECClassInstances (ecClassToInsertInstances);
                                            deleteECClassInstances (ecClassToInsertInstances);
                                            }
                                        m_db.CloseDb ();
                                        BeFileName dbPath (m_classDbName);
                                        //Remove Db specific to Class
                                        if (BeFileName::DoesPathExist (dbPath.GetName ()))
                                            {
                                            BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (dbPath.GetName ());
                                            if (fileDeleteStatus != BeFileNameStatus::Success)
                                                {
                                                LOG1.errorv ("Could not delete preexisting test ecdb file : %ls", m_classDbName.c_str ());
                                                EXPECT_TRUE (false);
                                                }
                                            }
                                        }
                                    }
                                }
                            else
                                continue;
                            }
                        else
                            {
                            LOG1.errorv ("[SRF] Schema Read Failed: %ls ", name.GetName ());
                            }
                        }
                    }
                }
                BeFileName::EmptyAndRemoveDirectory(tempDirName.c_str());
            }
        }
    }
TEST_F (InstanceCRUDTests, singleClassTest)
    {
    WCharCP schma = L"SimpleCompany.01.00.ecschema.xml";//schema name
    WCharCP classname = L"EducationStruct";//class name
    WCharCP folderName = L"StartupCompany";//folder name
    InstanceCRUDTests::initBeSQLiteLib ();
    BeFileName schemaPath;
    BeTest::GetHost ().GetDgnPlatformAssetsDirectory (schemaPath);
    schemaPath.AppendToPath (L"BeTestDocuments\\DgnDb\\BentleyProductSchema");
    schemaPath.AppendToPath (folderName);
    schemaPath.AppendToPath (L"\\");
    ECSchemaReadContextPtr schemaReadContext = nullptr;
    ECSchemaPtr schemaPtr = nullptr;
    ECDbTestUtility::ReadECSchemaFromDisk (schemaPtr, schemaReadContext, schma, schemaPath.c_str ());
    ASSERT_TRUE (schemaPtr != NULL);
    m_schemaFullPath = schemaPath + schma;
    auto ecClass = schemaPtr->GetClassCP (classname);
    m_className = ecClass->GetName ();
    if (setUpECdbSingleClass ())
        {
        ECClassCP ecClassToInsertInstances = m_schema->GetClassCP (m_className.c_str ());
        insertECClassInstances (ecClassToInsertInstances);
        updateECClassInstances (ecClassToInsertInstances);
        deleteECClassInstances (ecClassToInsertInstances);
        }
    m_db.CloseDb ();
    }
