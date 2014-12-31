#include "ECDbPublishedTests.h"
#include <Bentley/BeDirectoryIterator.h>
#include <ECObjects/ECObjectsAPI.h>
#include <BeSQLite/ECDb/ECDbApi.h>
#include <BeSQLite/BeSQLite.h>
#include "RandomECInstanceGenerator.h"
#include <vector>
#include <fstream>
#include <sstream>

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
    WString                     m_dbName;
    WString                     m_schemaFullPath;
    WString                     m_dirName;
    WString                     m_className;
    std::vector<ECClassCP>      m_classList;
    bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>>      m_generatedInstances;
    bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>>      m_generatedRelationshipInstances;


    virtual void SetUp() override;
    static void initBeSQLiteLib();
    static std::vector<WString> GetTestClassNames();
    static bool addClass(ECClassCP ecClass);

    void setupSteps(WStringCR param);
    void createDb();
    void importSchema();

    int inserECInstances(bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & generatedInstances);
    int countECInstnacesAndCompare(bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & generatedInstances);
    void addClassesForRelationship(ECRelationshipClassCP relClass);
    void insertECClassInstances(ECClassCP ecClass);
    void insertECRelationshipClassInstances();

    void deleteECClassInstances(ECClassCP ecClass);
    void deleteECRelationshipClassInstances();
    int deleteECInstances(bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & instances);

    void updateECClassInstances(ECClassCP ecClass);
    void updateECRelationshipClassInstances();
    int updateECInstances(bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & insertedInstances, bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & toUpdateInstances);
};
/*---------------------------------------------------------------------------------**//**
* Setup method for the Test class
* @bsimethod                           Majd Uddin                   09/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::SetUp()
{
    InstanceCRUDTests::initBeSQLiteLib();
}
/*---------------------------------------------------------------------------------**//**
* Initialize SQLite Lib for the tests
* @bsimethod                           Majd Uddin                   09/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::initBeSQLiteLib()
{
    BeFileName outputRoot, assetsDir;
    BeTest::GetHost().GetOutputRoot(outputRoot);
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDir);

    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    ECDb::Initialize(temporaryDir, &assetsDir);
}
/*---------------------------------------------------------------------------------**//**
* Generates list of classes to be used as parameters in the test
* @bsimethod                           Majd Uddin                   09/14
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<WString> InstanceCRUDTests::GetTestClassNames()
{
    InstanceCRUDTests::initBeSQLiteLib();
    std::vector<WString> classNames;
    BeFileName assetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDir);
    BeFileName productSchemas(assetsDir);
    productSchemas.AppendToPath(L"BeTestDocuments\\DgnDb\\BentleyProductSchema");

    if (BeFileName::DoesPathExist(productSchemas.GetName()))
    {
        BeFileName entryName;
        bool isDir;
        for (BeDirectoryIterator dirs(productSchemas); dirs.GetCurrentEntry(entryName, isDir) == SUCCESS; dirs.ToNext())
        {
            if (isDir)
            {
                WString toAdd;
                toAdd = toAdd + entryName + L"\\";
                entryName.AppendToPath(L"*.ecschema.xml");
                BeFileListIterator fileIterator(entryName, false);
                BeFileName name;
                while (fileIterator.GetNextFileName(name) != ERROR)
                {
                    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
                    schemaReadContext->AddSchemaPath(toAdd.c_str());

                    WString toAdd1(toAdd);
                    toAdd1 = toAdd1 + name.GetFileNameWithoutExtension();
                    toAdd1 = toAdd1 + L".xml";
                    ECSchemaPtr schema;
                    auto result = ECSchema::ReadFromXmlFile(schema, toAdd1.c_str(), *schemaReadContext);
                    EXPECT_EQ(result, SUCCESS) << "Failed to read schema: " << toAdd1.c_str();
                    //Skip Standard, System and Supplemental schemas
                    if (ECSchema::IsStandardSchema(schema->GetName().c_str()) || schema->IsSystemSchema() || toAdd1.find(L"Supplemental") != WString::npos)
                    {
                        continue;
                    }
                    auto const& classes = schema->GetClasses();
                    FOR_EACH(ECClassCP ecClass, classes)
                    {
                        if (InstanceCRUDTests::addClass(ecClass))
                        {
                            WString className(toAdd1);
                            className = className + L">" + ecClass->GetName();
                            classNames.push_back(className);
                        }
                    }
                }
            }
        }
    }
    return classNames;
}
/*---------------------------------------------------------------------------------**//**
* Insert ECinstances from given list
* @bsimethod                           Muhammad.Zaighum                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
int InstanceCRUDTests::inserECInstances(bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & generatedInstances)
{
    int instanceCount = 0;
    for (auto& entry : generatedInstances)
    {
        auto ecClass = entry.first;
        auto instanceList = entry.second;
        ECDbR ecdb = m_db;
        for (auto &instance : instanceList)
        {
            if (!instance.IsValid())
            {
                continue;
            }
            Savepoint savepoint(ecdb, "Populate");
            ECInstanceInserter inserter(ecdb, *ecClass);
            auto insertStatus = inserter.Insert(*instance);

            if (insertStatus != BentleyStatus::SUCCESS)
            {
                IECRelationshipInstancePtr relInstance = dynamic_cast<IECRelationshipInstance*>(instance.get());
                if (relInstance.IsValid()) // this is a relationship class
                {
                    EXPECT_TRUE(false) << "I'm a relationship";
                    //save output for record
                    EXPECT_TRUE(false) << "Could not insert ECInstance of ECClass: " << Utf8String(ecClass->GetFullName()).c_str() << " The error is: " << insertStatus;
                    // save output for record
                    IECInstancePtr sourceInstance = relInstance->GetSource();
                    IECInstancePtr targetInstance = relInstance->GetTarget();

                    WString file(m_dirName);
                    file = file.append(ecClass->GetFullName());
                    file = file.append(L"_Src_");
                    file = file.append(sourceInstance->GetInstanceId());
                    file = file.append(L".xml");
                    InstanceWriteStatus status = sourceInstance->WriteToXmlFile(file.c_str(), true, true);
                    EXPECT_EQ(INSTANCE_WRITE_STATUS_Success, status);

                    WString file1(m_dirName);
                    file1 = file1.append(ecClass->GetFullName());
                    file1 = file1.append(L"_Tar_");
                    file1 = file1.append(targetInstance->GetInstanceId());
                    file1 = file1.append(L".xml");
                    status = targetInstance->WriteToXmlFile(file1.c_str(), true, true);
                    EXPECT_EQ(INSTANCE_WRITE_STATUS_Success, status);
                }
                else
                {
                    EXPECT_TRUE(false) << "Could not insert ECInstance of ECClass: " << Utf8String(ecClass->GetFullName()).c_str() << " The error is: " << insertStatus;
                    // save output for record
                    WString file(m_dirName);
                    file = file.append(ecClass->GetFullName());
                    file = file.append(instance->GetInstanceId());
                    file = file.append(L".xml");
                    InstanceWriteStatus status = instance->WriteToXmlFile(file.c_str(), true, true);
                    EXPECT_EQ(INSTANCE_WRITE_STATUS_Success, status);
                }
            }
            else
            {
                savepoint.Commit();
                instanceCount++;
            }
        }
    }
    return instanceCount;
}
/*---------------------------------------------------------------------------------**//**
* Compare ECinstances and return number of ECInstnaces
* @bsimethod                           Muhammad.Zaighum                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
int InstanceCRUDTests::countECInstnacesAndCompare(bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & generatedInstances)
{
    int numberOfInstnace = 0;
    ECDbR ecdb = m_db;
    for (auto& entry : generatedInstances)
    {
        auto ecClass = entry.first;
        SqlPrintfString ecSql("SELECT * FROM ONLY [%s].[%s]", Utf8String(ecClass->GetSchema().GetName()).c_str(), Utf8String(ecClass->GetName()).c_str());
        ECSqlStatement ecStatement;
        ECSqlStatus status = ecStatement.Prepare(ecdb, ecSql.GetUtf8CP());
        if (ECSqlStatus::Success != status)
        {
            LOG.infov("Could Prepare statement : %s ", ecSql.GetUtf8CP());
            return 0;
        }
        bvector<IECInstancePtr> instances;
        ECInstanceECSqlSelectAdapter adapter(ecStatement);
        while (ECSqlStepStatus::HasRow == ecStatement.Step())
        {
            IECInstancePtr newInstance = adapter.GetInstance();
            if (!newInstance.IsValid())
                continue;
            if (newInstance != nullptr)
            {
                auto const& instanceList = entry.second;
                bool foundStatus = false;
                for_each(instanceList.begin(), instanceList.end(), [&ecdb, &ecClass, &foundStatus, &newInstance](IECInstancePtr const& instance)
                {
                    if (newInstance->GetInstanceId().Equals(instance->GetInstanceId()))
                    {
                        if (ECDbTestUtility::CompareECInstances(*newInstance, *instance))
                        {
                            foundStatus = true;
                        }
                        else
                        {
                            Utf8String ins1;
                            newInstance->WriteToXmlString(ins1, true, true);
                            Utf8String ins2;
                            instance->WriteToXmlString(ins2, true, true);
                            if (!ins1.Equals(ins2))
                                LOG.infov("ECInstances are not equal:\n %s \n %s ", ins1.c_str(),ins2.c_str());
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
bool InstanceCRUDTests::addClass(ECClassCP ecClass)
{
    //Check if NULL
    if (ecClass == NULL)
        return false;
    //Skip Abstract Classes
    if (!ecClass->GetIsDomainClass())
        return false;
    //Skip CustomAttribute Classes
    if (ecClass->GetIsCustomAttributeClass())
        return false;
    //Skip AnyClass
    if (ecClass->GetName().find(L"AnyClass") != WString::npos)
        return false;
    //Class is good to Add in the test
    return true;
}
/*---------------------------------------------------------------------------------**//**
* Extracts and set paths and names from the param
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::setupSteps(WStringCR param)
{
    m_schemaFullPath = param.substr(0, param.find_last_of(L">"));
    m_className = param.substr((param.find_last_of(L">")) + 1, param.length());
    BeFileName dbName(m_schemaFullPath);
    m_dirName = dbName.GetDirectoryName();
    WString schemaName = dbName.GetFileNameWithoutExtension();
    WString dbName1(m_dirName);
    dbName1 = dbName1 + schemaName;
    dbName1.AppendA(".");
    dbName1 = dbName1 + m_className;
    dbName1.AppendA(".ecdb");
    m_dbName = dbName1;

    LOG1.infov("Current Schema is: %ls.xml", schemaName.c_str());
    createDb();
    importSchema();
    m_classList.clear();

}
/*---------------------------------------------------------------------------------**//**
* Create the Db and removes if there is one already
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::createDb()
{
    BeFileName dbPath(m_dbName);
    //Remove existing db if any
    if (BeFileName::DoesPathExist(dbPath.GetName()))
    {
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile(dbPath.GetName());
        if (fileDeleteStatus != BeFileNameStatus::Success)
        {
            LOG1.errorv("Could not delete preexisting test ecdb file : %ls", m_dbName.c_str());
            EXPECT_TRUE(false);
        }
    }
    auto stat = m_db.CreateNewDb(dbPath);
    if (stat != SUCCESS)
    {
        LOG1.errorv("Db could not be created for: %ls", m_dbName.c_str());
        EXPECT_TRUE(false);
    }
    else
        LOG1.infov("Created ECDb for class: %ls", m_className.c_str());
}
/*---------------------------------------------------------------------------------**//**
* Imports all needed schemas into the Db
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::importSchema()
{
    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaPath(m_dirName.c_str());

    auto result = ECSchema::ReadFromXmlFile(m_schema, m_schemaFullPath.c_str(), *schemaReadContext);
    EXPECT_EQ(result, SUCCESS);
    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*m_schema);

    // If there are any Supplemental schemas, thos need to be in the cache also for supplementation
    BeFileName dir(m_dirName);
    bvector<BeFileName> suppls;
    BeDirectoryIterator::WalkDirsAndMatch(suppls, dir, L"*Supplemental*.ecschema.xml", true);
    FOR_EACH(BeFileName entry, suppls)
    {
        WString supplSchemaPath(entry);
        ECSchemaPtr schema1;
        auto result = ECSchema::ReadFromXmlFile(schema1, supplSchemaPath.c_str(), *schemaReadContext);
        EXPECT_EQ(result, SUCCESS);
        cache->AddSchema(*schema1);
    }
    auto importSchemaStatus = m_db.GetSchemaManager().ImportECSchemas(*cache, ECDbSchemaManager::ImportOptions(true, true));
    if (importSchemaStatus != SUCCESS)
    {
        LOG1.errorv("The schema could not be imported. Schema Name: %ls", m_schemaFullPath.c_str());
        EXPECT_TRUE(false);
    }
    else
        LOG1.infov("Schemas imported for class: %ls", m_className.c_str());
}
/*---------------------------------------------------------------------------------**//**
* Insert instances of a given EC Class. Deals with Relationship class and calls it
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::insertECClassInstances(ECClassCP ecClass)
{
    LOG1.infov("Starting Insert operation for class: %ls", m_className.c_str());
    ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP();
    if (relClass == NULL)
    {
        m_classList.push_back(ecClass);
        int inserted = 0;
        int actualCount = 0;

        RandomECInstanceGenerator insert(m_classList);
        auto status = insert.Generate(false);
        if (status != SUCCESS)
        {
            LOG1.errorv("Failed to generate random instance for: %ls", m_className.c_str());
            EXPECT_TRUE(false);
        }
        m_generatedInstances = insert.GetGeneratedInstances();
        inserted = inserECInstances(m_generatedInstances);
        actualCount = countECInstnacesAndCompare(m_generatedInstances);
        if (inserted <= 0 || inserted != actualCount)
        {
            LOG1.errorv("Either no instances are inserted OR count doesn't match for class: %ls", m_className.c_str());
            EXPECT_TRUE(false);
        }
        LOG1.infov("Instances were inserted for class: %ls", m_className.c_str());
    }
    else // This is a Relationship class
    {
        addClassesForRelationship(relClass);
        m_classList.push_back(relClass);
        insertECRelationshipClassInstances();
    }
}
/*---------------------------------------------------------------------------------**//**
* Insert instances of a given ECRelationshipClass.
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::insertECRelationshipClassInstances()
{
    int inserted = 0;
    int actualCount = 0;

    RandomECInstanceGenerator insert(m_classList);
    auto status = insert.Generate(false);
    if (status != SUCCESS)
    {
        LOG1.errorv("Failed to generate random instance for: %ls", m_className.c_str());
        EXPECT_TRUE(false);
    }
    m_generatedInstances = insert.GetGeneratedInstances();
    inserted = inserECInstances(m_generatedInstances);
    actualCount = countECInstnacesAndCompare(m_generatedInstances);
    if (inserted <= 0 || inserted != actualCount)
    {
        LOG1.errorv("Either no instances are inserted OR count doesn't match for class: %ls", m_className.c_str());
        EXPECT_TRUE(false);
    }

    //Now let's create instance of Relationship class
    inserted = 0;
    actualCount = 0;
    status = insert.GenerateRelationships();
    if (status != SUCCESS)
    {
        LOG1.errorv("Failed to generate random instance for: %ls", m_className.c_str());
        EXPECT_TRUE(false);
    }
    m_generatedRelationshipInstances = insert.GetGeneratedRelationshipInstances();
    inserted = inserECInstances(m_generatedRelationshipInstances);
    actualCount = countECInstnacesAndCompare(m_generatedRelationshipInstances);
    if (inserted <= 0 || inserted != actualCount)
    {
        LOG1.errorv("Either no instances are inserted OR count doesn't match for class: %ls", m_className.c_str());
        EXPECT_TRUE(false);
    }
    LOG1.infov("Instances were inserted for Relationship: %ls", m_className.c_str());

}
/*---------------------------------------------------------------------------------**//**
* Add all needed classes for a given Relationship Class
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::addClassesForRelationship(ECRelationshipClassCP relClass)
{
    const ECConstraintClassesList& sourceClasses = relClass->GetSource().GetClasses();
    const ECConstraintClassesList& targetClasses = relClass->GetTarget().GetClasses();
    FOR_EACH(ECClassCP ecClass2, sourceClasses)
    {
        if (InstanceCRUDTests::addClass(ecClass2))
        {
            m_classList.push_back(ecClass2);
            //For some relationships, it needs the struct Class instances to be there.
            FOR_EACH(ECPropertyCP ecProperty, ecClass2->GetProperties(true))
            {
                if (ecProperty->GetIsStruct())
                {
                    ECClassCP structClass = m_schema->GetClassCP(ecProperty->GetTypeName().c_str());
                    if (InstanceCRUDTests::addClass(structClass))
                        m_classList.push_back(structClass);
                }
            }
        }
        //For some cases, we have to add child Domain classes of an Abstract class
        auto const& derivedClasses = ecClass2->GetDerivedClasses();
        FOR_EACH(ECClassCP ecClass4, derivedClasses)
        {
            if (InstanceCRUDTests::addClass(ecClass4))
                m_classList.push_back(ecClass4);
        }
    }
    FOR_EACH(ECClassCP ecClass3, targetClasses)
    {
        if (InstanceCRUDTests::addClass(ecClass3))
        {
            m_classList.push_back(ecClass3);
            //For some relationships, it needs the struct Class instances to be there.
            FOR_EACH(ECPropertyCP ecProperty, ecClass3->GetProperties(true))
            {
                if (ecProperty->GetIsStruct())
                {
                    ECClassCP structClass2 = m_schema->GetClassCP(ecProperty->GetTypeName().c_str());
                    if (InstanceCRUDTests::addClass(structClass2))
                        m_classList.push_back(structClass2);
                }
            }
        }
        //For some cases, we have to add child Domain classes of an Abstract class
        auto const& derivedClasses = ecClass3->GetDerivedClasses();
        FOR_EACH(ECClassCP ecClass5, derivedClasses)
        {
            if (InstanceCRUDTests::addClass(ecClass5))
                m_classList.push_back(ecClass5);
        }
    }

}
/*---------------------------------------------------------------------------------**//**
* Deletes all instances of provided class. Takes care of Relationship class case
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::deleteECClassInstances(ECClassCP ecClass)
{
    ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP();

    if (relClass == NULL)
    {
        int delCount = deleteECInstances(m_generatedInstances);
        if (delCount == 0)
        {
            LOG1.errorv("No instances were deleted of ECClass %ls", m_className.c_str());
            EXPECT_TRUE(false);
        }
        else
            LOG1.infov("Instances were deleted for class: %ls", m_className.c_str());
    }
    else //This is relationshipClass
    {
        deleteECRelationshipClassInstances();
    }
}
/*---------------------------------------------------------------------------------**//**
* Deletes all instances of Relationship class
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::deleteECRelationshipClassInstances()
{
    int delCount = deleteECInstances(m_generatedRelationshipInstances);
    if (delCount == 0)
    {
        LOG1.errorv("No instances were deleted of Relationship %ls", m_className.c_str());
        EXPECT_TRUE(false);
    }
    else
        LOG1.infov("Instances were deleted for Relationship: %ls", m_className.c_str());
}
/*---------------------------------------------------------------------------------**//**
* The method that does the deletion and returns delete count
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
int InstanceCRUDTests::deleteECInstances(bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & instances)
{
    int count = 0;
    for (auto& entry : instances)
    {
        auto ecClass = entry.first;
        auto const& instanceList = entry.second;
        for (auto &instance : instanceList)
        {
            ECInstanceDeleter deleter(m_db, *ecClass);
            if (!instance.IsValid())
                continue;
            auto deleteStatus = deleter.Delete(*instance.get());
            if (deleteStatus != SUCCESS)
            {
                LOG1.errorv("Could not delete ECInstance of ECClass %ls", m_className.c_str());
                EXPECT_TRUE(false);
            }
            else
                count++;
        }
    }
    int actualCount = countECInstnacesAndCompare(instances);
    if (actualCount != 0)
    {
        LOG1.errorv("Not all ECInstances were deleted of ECClass %ls", m_className.c_str());
        EXPECT_TRUE(false);
    }
    return count;
}
/*---------------------------------------------------------------------------------**//**
* Updatess all instances of provided class. Takes care of Relationship class case
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::updateECClassInstances(ECClassCP ecClass)
{
    ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP();

    if (relClass == NULL)
    {
        RandomECInstanceGenerator update(m_classList);
        auto status = update.Generate(false);
        if (status != SUCCESS)
        {
            LOG1.errorv("Failed to generate random instances for %s", m_className.c_str());
            EXPECT_TRUE(false);
        }
        auto updateInstances = update.GetGeneratedInstances();

        int updateCount = updateECInstances(m_generatedInstances, updateInstances);
        if (updateCount == 0)
        {
            LOG1.errorv("No instances were updated of ECClass %ls", m_className.c_str());
            EXPECT_TRUE(false);
        }
        else
            LOG1.infov("Instances were updated for class: %ls", m_className.c_str());
    }
    else //This is relationshipClass
    {
        updateECRelationshipClassInstances();
    }
}
/*---------------------------------------------------------------------------------**//**
* Updates all instances of Relationship class
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCRUDTests::updateECRelationshipClassInstances()
{
    RandomECInstanceGenerator update(m_classList);
    //For Relationship instances, we need to insert instances first
    auto status = update.Generate(false);
    if (status != SUCCESS)
    {
        LOG1.errorv("Failed to generate random instance for: %ls", m_className.c_str());
        EXPECT_TRUE(false);
    }
    auto generatedInstances = update.GetGeneratedInstances();
    int inserted = inserECInstances(generatedInstances);
    int actualCount = countECInstnacesAndCompare(generatedInstances);
    if (inserted <= 0 || inserted != actualCount)
    {
        LOG1.errorv("Either no instances are inserted OR count doesn't match for class: %ls", m_className.c_str());
        EXPECT_TRUE(false);
    }

    status = update.GenerateRelationships();
    if (status != SUCCESS)
    {
        LOG1.errorv("Failed to generate random instances for %s", m_className.c_str());
        EXPECT_TRUE(false);
    }
    auto updateInstances = update.GetGeneratedRelationshipInstances();

    int updateCount = updateECInstances(m_generatedRelationshipInstances, updateInstances);
    if (updateCount == 0)
    {
        LOG1.errorv("No instances were updated of Relationship %ls", m_className.c_str());
        EXPECT_TRUE(false);
    }
    else
        LOG1.infov("Instances were updated for Relationship: %ls", m_className.c_str());
}
/*---------------------------------------------------------------------------------**//**
* The method that does the update and returns update count
* @bsimethod                           Majd Uddin                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
int InstanceCRUDTests::updateECInstances(bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & insertedInstances, bmap<ECN::ECClassCP, std::vector<ECN::IECInstancePtr>> const & toUpdateInstances)
{
    int count = 0;
    for (auto insertEntry = insertedInstances.begin(),
        updateEntry = toUpdateInstances.begin();
        insertEntry != insertedInstances.end(); ++insertEntry, ++updateEntry)
    {
        auto ecClass = updateEntry->first;
        for (auto updateinstance = updateEntry->second.begin(),
            insertedInstance = insertEntry->second.begin();
            insertedInstance != insertEntry->second.end();
        updateinstance++, insertedInstance++)
        {
            if (!insertedInstance->IsValid())
                continue;
            updateinstance->get()->SetInstanceId(insertedInstance->get()->GetInstanceId().c_str());
            ECInstanceUpdater updater(m_db, *ecClass);
            auto updateStatus = updater.Update(*(updateinstance->get()));
            if (updateStatus != SUCCESS)
            {
                LOG1.errorv("Could not update ECInstance of ECClass %ls", m_className.c_str());
                EXPECT_TRUE(false);
            }
            else
            {
                count++;
            }
        }
    }
    return count;
}

/*---------------------------------------------------------------------------------**//**
* Test to INSERT EC Instances of Classes and Relationships in all schemas
* @bsimethod                           Majd Uddin                       10/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceCRUDTests, Insert)
{
    std::vector<WString> classNames = InstanceCRUDTests::GetTestClassNames();
    LOG1.infov("========INSERT TEST ====TOTAL CLASSES: %d ==============", classNames.size());

    int i = 0;
    FOR_EACH(WString param, classNames)
    {
        //Setup things
        setupSteps(param);
        //Get the class and insert instances
        ECClassCP ecClass = m_schema->GetClassCP(m_className.c_str());
        if (ecClass == NULL)
        {
            LOG1.errorv("Could not load class: %ls", m_className.c_str());
            EXPECT_TRUE(false);
        }
        insertECClassInstances(ecClass);
        //close the db
        m_db.CloseDb();
        i++;

        LOG1.infov("Ending Insert operation for class: %ls", m_className.c_str());
        LOG1.infov("===============END OF INSERT TEST %d ======================", i);
    }
}
/*---------------------------------------------------------------------------------**//**
* Test to DELETE EC Instances of Classes and Relationships in all schemas.
* We also Insert some instances but that is a preReq and not the intent of the test
* @bsimethod                           Majd Uddin                       10/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceCRUDTests, Delete)
{
    std::vector<WString> classNames = InstanceCRUDTests::GetTestClassNames();
    LOG1.infov("========DELETE TEST ====TOTAL CLASSES: %d ==============", classNames.size());

    int i = 0;
    FOR_EACH(WString param, classNames)
    {
        //Setup things
        setupSteps(param);
        //Get the class and insert instances
        ECClassCP ecClass = m_schema->GetClassCP(m_className.c_str());
        if (ecClass == NULL)
        {
            LOG1.errorv("Could not load class: %ls", m_className.c_str());
            EXPECT_TRUE(false);
        }
        insertECClassInstances(ecClass);
        LOG1.infov("Ending Insert operation for class: %ls", m_className.c_str());
        //Now lets delete and verify
        LOG1.infov("Starting Delete operation for class: %ls", m_className.c_str());
        deleteECClassInstances(ecClass);
        //close the db
        m_db.CloseDb();
        i++;
        LOG1.infov("Ending Delete operation for class: %ls", m_className.c_str());
        LOG1.infov("================END OF DELETE TEST %d ======================", i);
    }
}
/*---------------------------------------------------------------------------------**//**
* Test to UPDATE EC Instances of Classes and Relationships in all schemas.
* We also Insert some instances but that is a preReq and not the intent of the test
* @bsimethod                           Majd Uddin                       10/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceCRUDTests, Update)
{
    std::vector<WString> classNames = InstanceCRUDTests::GetTestClassNames();
    LOG1.infov("========DELETE TEST ====TOTAL CLASSES: %d ==============", classNames.size());

    int i = 0;
    FOR_EACH(WString param, classNames)
    {
        //Setup things
        setupSteps(param);
        //Get the class and insert instances
        ECClassCP ecClass = m_schema->GetClassCP(m_className.c_str());
        if (ecClass == NULL)
        {
            LOG1.errorv("Could not load class: %ls", m_className.c_str());
            EXPECT_TRUE(false);
        }
        insertECClassInstances(ecClass);
        LOG1.infov("Ending Insert operation for class: %ls", m_className.c_str());
        //Now lets update
        LOG1.infov("Starting Update operation for class: %ls", m_className.c_str());
        updateECClassInstances(ecClass);
        //close the db
        m_db.CloseDb();
        i++;
        LOG1.infov("Ending Update operation for class: %ls", m_className.c_str());
        LOG1.infov("================END OF UPDATE TEST %d ======================", i);
    }
}
/*---------------------------------------------------------------------------------**//**
* Test to perform INSERT or UPDATE or DELETE for a particular class in a particular schema
* @bsimethod                           Majd Uddin                       10/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceCRUDTests, SingleClass)
{
    std::vector<WString> classNames = InstanceCRUDTests::GetTestClassNames();
    LOG1.infov("========SINGLE CLASS TEST ==================");

    //setup whether to test for Update or Delete. Insert is always done.
    bool updateOp = false;
    bool deleteOp = false;
    FOR_EACH(WString param, classNames)
    {
        //filter for a schema and class name
        WString schemaFullPath = param.substr(0, param.find_last_of(L">"));
        WString classToFind = param.substr((param.find_last_of(L">")) + 1, param.length());
        BeFileName dbName(schemaFullPath);
        WString schemaToFind = dbName.GetFileNameAndExtension();

        //Type name of Schema and class here to run only for this.
        if (schemaToFind.Equals(L"SimpleCompany.01.00.ecschema.xml") && classToFind.Equals(L"Employee"))
        {
            setupSteps(param);
            //Get the class and insert instances
            ECClassCP ecClass = m_schema->GetClassCP(m_className.c_str());
            if (ecClass == NULL)
            {
                LOG1.errorv("Could not load class: %ls", m_className.c_str());
                EXPECT_TRUE(false);
            }
            insertECClassInstances(ecClass);
            LOG1.infov("Ending Insert operation for class: %ls", m_className.c_str());

            if (updateOp)
            {
                LOG1.infov("Starting Update operation for class: %ls", m_className.c_str());
                updateECClassInstances(ecClass);
                LOG1.infov("Ending Update operation for class: %ls", m_className.c_str());
            }
            if (deleteOp)
            {
                LOG1.infov("Starting Delete operation for class: %ls", m_className.c_str());
                deleteECClassInstances(ecClass);
                LOG1.infov("Ending Delete operation for class: %ls", m_className.c_str());
            }
            //close the db
            m_db.CloseDb();
            LOG1.infov("==========END OF SINGLE CLASS TEST ================");
        }
        else
            continue;
    }
}
