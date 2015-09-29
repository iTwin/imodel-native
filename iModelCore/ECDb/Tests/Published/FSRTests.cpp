/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/FSRTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <Bentley/BeDirectoryIterator.h>
#include <vector>
#include <BeSQLite/BeSQLite.h>
#include <BeXml/BeXml.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_LOGGING

using namespace Tests;


/*---------------------------------------------------------------------------------**//**
* Test fixture for testing ECSchema Import and instance creation.
* @bsiclass                           Muhammad.Zaighum                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct FSRTests : public ::testing::Test
    {
    ECSchemaPtr ecSchema;
    ECDb m_db;
    void initBeSQLiteLib()
        {
        BeFileName outputRoot, assetsDir;
        BeTest::GetHost().GetOutputRoot(outputRoot);
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDir);

        BeFileName temporaryDir;
        BeTest::GetHost().GetOutputRoot(temporaryDir);
        ECDb::Initialize(temporaryDir, &assetsDir);
        }
    void createGeometryECDBAndImportSchemas(BeFileName currentSchemaName)
        {
        BeFileName outFolder;
        BeTest::GetHost().GetDocumentsRoot(outFolder);
        outFolder.AppendToPath(currentSchemaName.GetFileNameWithoutExtension().c_str());
        outFolder.append(L".ecdb");
        if (BeFileName::DoesPathExist(outFolder.GetName()))
            {
            // Delete any previously created file
            BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile(outFolder.GetName());
            ASSERT_TRUE(fileDeleteStatus == BeFileNameStatus::Success) << "Could not delete preexisting test ecdb file '" << outFolder.GetName() << "'.";
            }

        DbResult stat = m_db.CreateNewDb(outFolder);
        ASSERT_EQ(BE_SQLITE_OK, stat) << "Creation of test ECDb file failed.";

        ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
        BeFileName schemaDirectory = currentSchemaName;
        schemaDirectory.PopDir();
        schemaReadContext->AddSchemaPath(schemaDirectory);
        currentSchemaName.append(L".ecschema.xml");
        Utf8String schemaName;
        uint32_t major, minor;
        ECSchema::ParseSchemaFullName(schemaName, major, minor, Utf8String(currentSchemaName.GetFileNameWithoutExtension().c_str()));
        SchemaKey key = SchemaKey(schemaName.c_str(), major, minor);
        ecSchema = ECSchema::LocateSchema(key, *schemaReadContext);
        ASSERT_TRUE(ecSchema != NULL);
        LOG.infov("Loaded schema %s", currentSchemaName.GetName());
        auto importSchemaStatus = m_db.Schemas().ImportECSchemas(schemaReadContext->GetCache(), ECDbSchemaManager::ImportOptions(true, true));
        ASSERT_EQ(SUCCESS, importSchemaStatus);


        }
    ECInstanceList ReadInstancesFromDom(BeXmlDomPtr xmlDom, ECInstanceReadContextPtr schemaReadContext)
        {
        IECInstancePtr instance;
        BeXmlNodeP      instanceNode;
        ECInstanceList ecInstances;
        if ((BEXML_Success != xmlDom->SelectNode(instanceNode, "/", NULL, BeXmlDom::NODE_BIAS_First)) || (NULL == instanceNode))
            {
            BeAssert(false);
            LOG.errorv("Invalid ECInstanceXML: Missing a top-level instance node");
            }

        if (BEXMLNODE_Document == instanceNode->GetType())
            instanceNode = instanceNode->GetFirstChild(BEXMLNODE_Element)->GetFirstChild(BEXMLNODE_Element);
        while (instanceNode = instanceNode->GetNextSibling())
            {
            InstanceReadStatus status = IECInstance::ReadFromBeXmlNode(instance, *instanceNode->GetFirstChild(BEXMLNODE_Element), *schemaReadContext);
            BeAssert(status == INSTANCE_READ_STATUS_Success);
            ecInstances.push_back(instance);
            }
        return ecInstances;
        }
    void InsertECInstances(ECInstanceList& ecInstances)
        {

        for(IECInstancePtr instance : ecInstances)
            {
            Savepoint savepoint(m_db, "Populate");
            auto &ecClass = instance->GetClass();
            ECInstanceInserter inserter(m_db, ecClass);
            ECInstanceKey id;
            auto insertStatus = inserter.Insert(id, *(instance.get()));
            if (insertStatus != BentleyStatus::SUCCESS)
                {
                LOG.infov("Could not insert ECInstance of ECClass %s into ECDb file. Error code: InsertStatus::%d", Utf8String(ecClass.GetFullName()).c_str(), insertStatus);
                savepoint.Cancel();
                }
            else
                {
                savepoint.Commit();

                }
            }
        }
    void VerifyECInstnaces(ECInstanceList& instances)
        {
        ECDbR ecdb = m_db;
        for (auto instance : instances)
            {
            auto &ecClass = instance->GetClass();
            SqlPrintfString ecSql("SELECT * FROM ONLY [%s].[%s] WHERE ECInstanceId = %s", Utf8String(ecClass.GetSchema().GetName()).c_str(), Utf8String(ecClass.GetName()).c_str(), Utf8String(instance->GetInstanceId()).c_str());
            ECSqlStatement ecStatement;
            ECSqlStatus status = ecStatement.Prepare(ecdb, ecSql.GetUtf8CP());
            ASSERT_TRUE(ECSqlStatus::Success == status);
            ECInstanceECSqlSelectAdapter adapter(ecStatement);
            ASSERT_TRUE(BE_SQLITE_ROW == ecStatement.Step());
            IECInstancePtr newInstance = adapter.GetInstance();
            ASSERT_TRUE(ECDbTestUtility::CompareECInstances(*newInstance, *instance)) << ecSql.GetUtf8CP() << "\n" << newInstance->GetInstanceId().c_str() << "!=" << instance->GetInstanceId().c_str();

            }
        }
    };
/*---------------------------------------------------------------------------------**//**
* Test for Importing schema and CRUD instances
* @bsimethod                                    Muhammad.Zaighum                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FSRTests, InsertAndVerifyFSRSchemaInstances)
    {
    initBeSQLiteLib();

    ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(m_db.GetSchemaLocater());
    ECSchemaPtr schema;
    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext(*schemaReadContext, *schema, &schema);
    ECInstanceList instances;
    
    BeFileName schemaDirectory;
    BeTest::GetHost().GetDocumentsRoot(schemaDirectory);
    schemaDirectory.AppendToPath(L"DgnDb");
    schemaDirectory.AppendToPath(L"FSR");
    BeDirectoryIterator directoryIterator(schemaDirectory);
    while (directoryIterator.ToNext() != ERROR)
        {
        BeFileName name;
        bool isDir = false;
        StatusInt status = directoryIterator.GetCurrentEntry(name, isDir, true);
        ASSERT_TRUE(SUCCESS==status);
        if (!isDir)
            continue;
        createGeometryECDBAndImportSchemas(name);
        name.AppendToPath(L"*.xml");
        BeFileListIterator fileIterator(name, false);
        while (fileIterator.GetNextFileName(name) != ERROR)
            {
            BeXmlStatus xmlStatus;
            WStringP errorMessage = new WString();
            BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, name.GetWCharCP(), errorMessage, BeXmlDom::ParseOptions::XMLPARSE_OPTION_AssertOnParseError);
            ASSERT_TRUE(xmlStatus == BEXML_Success);
            ECInstanceList instanceList = ReadInstancesFromDom(xmlDom, instanceContext);
            InsertECInstances(instanceList);
            VerifyECInstnaces(instanceList);
            }
        }
    };