/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnECNavigator_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <ECDb/ECDbApi.h>

#include <DgnPlatform/DgnECPersistence.h>

#define SYNCINFO_TABLE(name)  "v8sync_" name
#define SYNC_TABLE_Element SYNCINFO_TABLE("Element")

//#if defined (_MSC_VER)
//#pragma warning (disable:4702)
//#endif

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

extern void DebugDumpJson (const Json::Value& jsonValue);
extern bool WriteJsonToFile (WCharCP path, const Json::Value& jsonValue);
extern bool ReadJsonFromFile (Json::Value& jsonValue, WCharCP path);

//=======================================================================================
// @bsiclass                                                Ramanujam.Raman      02/2016
//=======================================================================================
struct SyncInfoUtility : public BeSQLite::Db::AppData
{
private:
    BeSQLite::Db m_syncInfoDb;
    BeSQLite::Statement m_elementStmt;
public:

    SyncInfoUtility() {}

    BentleyStatus Initialize(Dgn::DgnDbCR dgndb)
        {
        BeFileName pathname = dgndb.GetFileName();
        pathname.AppendExtension(L"syncinfo");

        if (!pathname.DoesPathExist())
            {
            BeAssert(false && "Could not find syncinfo db");
            return ERROR;
            }

        DbResult result = m_syncInfoDb.OpenBeSQLiteDb(pathname, Db::OpenParams(Db::OpenMode::Readonly, DefaultTxn::Yes));
        if (result != BE_SQLITE_OK)
            {
            BeAssert(false && "Could not open syncinfo db");
            return ERROR;
            }

        /*
        "v8sync_Element"    ElementId   => V8File, V8Model, V8ElementId;
        "v8sync_Model"      V8Id        => V8Name
        "v8sync_File"       Id          => V8Name (Entire path name - get the file name only)
        */
        // Note: Ignores the file name in getting the V9 Id - but this is sufficient for the ATP
        result = m_elementStmt.Prepare(m_syncInfoDb, "SELECT ElementId FROM " SYNC_TABLE_Element " WHERE V8ElementId=?");
        if (result != BE_SQLITE_OK)
            {
            BeAssert(false && "Cannot prepare statement to get ElementId");
            return ERROR;
            }

        return SUCCESS;
        }

    void Finalize()
        {
        if (m_elementStmt.IsPrepared())
            m_elementStmt.Finalize();
        if (m_syncInfoDb.IsDbOpen())
            m_syncInfoDb.CloseDb();
        }

    ~SyncInfoUtility()
        {
        Finalize();
        }

    // Note: Ignores the file name in getting the V9 Id - but this is sufficient for the ATP
    DgnElementId GetV9ElementId(int64_t v8ElementId)
        {
        m_elementStmt.BindInt64(1, v8ElementId);
        DbResult result = m_elementStmt.Step();
        if (result != BE_SQLITE_ROW)
            {
            BeAssert(false && "Element not found");
            return DgnElementId();
            }
        return m_elementStmt.GetValueId<DgnElementId>(0);
        }
};

//=======================================================================================    
//! @bsiclass                                                Ramanujam.Raman      02/2014
//=======================================================================================   
struct DgnECNavigatorTest : public testing::Test
{
private:
    SyncInfoUtility m_syncInfoUtility;
    ScopedDgnHost m_host;
    
    void ReinitializeL10N()
        {
        BeFileName frameworkSqlang;
        BeTest::GetHost().GetFrameworkSqlangFiles(frameworkSqlang);

        if (frameworkSqlang.DoesPathExist())
            {
            BeSQLite::L10N::Shutdown();
            BeSQLite::L10N::Initialize(frameworkSqlang);
            }
        }

protected:
    DgnECNavigatorTest() {}
    virtual ~DgnECNavigatorTest () {};
    
    DgnDbPtr m_testDb;

    virtual void SetUp() override 
        {
        ReinitializeL10N(); 
        // Note: Seems to be needed since some test sets the pseudo lang pack instead of en causing an ATP failure
        }

    virtual void TearDown() override {}

    void OpenDgnDb(WCharCP testFileName)
        {
        BeFileName pathname;
        BeTest::GetHost().GetDocumentsRoot(pathname);
        pathname.AppendToPath(L"DgnDb");
        pathname.AppendToPath(testFileName);

        OpenDgnDb(pathname);
        }

    void OpenDgnDb(BeFileNameCR testPathname)
        {
        DgnDb::OpenParams openParams(Db::OpenMode::Readonly);
        DbResult openStatus;
        m_testDb = DgnDb::OpenDgnDb(&openStatus, testPathname, openParams);
        ASSERT_TRUE(m_testDb.IsValid()) << "Could not open test project";

        BentleyStatus status = m_syncInfoUtility.Initialize(*m_testDb);
        ASSERT_EQ(SUCCESS, status);
        }

    void CloseDgnDb()
        {
        m_syncInfoUtility.Finalize();
        m_testDb->CloseDb();
        m_testDb = nullptr;
        }

    DgnElementId GetV9ElementId(int64_t v8ElementId)
        {
        return m_syncInfoUtility.GetV9ElementId(v8ElementId);
        }

    BentleyStatus GetElementInfo(Json::Value& elementInfo, DgnElementId selectedElementId)
        {
        Json::Value jsonInstances, jsonDisplayInfo; // Note: Cannot just pass actualElemtnInfo["ecInstances"], actualElementInfo["ecDisplayInfo"] here. 
        BentleyStatus status = DgnECPersistence::GetElementInfo(jsonInstances, jsonDisplayInfo, selectedElementId, *m_testDb);
        if (status != SUCCESS)
            {
            BeAssert(false);
            return status;
            }
        
        elementInfo["ecInstances"] = jsonInstances;
        elementInfo["ecDisplayInfo"] = jsonDisplayInfo;
        return SUCCESS;
        }

    static void ValidateElementInfo(JsonValueR actualElementInfo, WCharCP expectedFileName)
        {
        BeFileName expectedFile(expectedFileName);
        if (!expectedFile.IsAbsolutePath())
            {
            BeTest::GetHost().GetDocumentsRoot(expectedFile);
            expectedFile.AppendToPath(L"DgnDb");
            expectedFile.AppendToPath(expectedFileName);
            }

        Json::Value expectedElementInfo;
        bool readFileStatus = ReadJsonFromFile(expectedElementInfo, expectedFile.GetName());
        ASSERT_TRUE(readFileStatus);

        const char* volatileProperties[] =
            {
            "$ECInstanceId",
            "$ECInstanceLabel",
            "ModelId",
            "Name",
            "LastMod",
            "Code"
            };

        // Ignore some properties in comparison - they too volatile. 
        for (int ii = 0; ii < (int) actualElementInfo["ecInstances"].size(); ii++)
            {
            JsonValueR actualInstance = actualElementInfo["ecInstances"][ii];
            JsonValueR expectedInstance = expectedElementInfo["ecInstances"][ii];

            for (const char* prop : volatileProperties)
                {
                if (actualInstance.isMember(prop)) actualInstance[prop] = "*";
                if (expectedInstance.isMember(prop)) expectedInstance[prop] = "*";
                }
            }

        int compare = expectedElementInfo.compare(actualElementInfo);
        if (0 != compare)
            {
            // For convenient android debugging
            //LOG.debugv ("Expected ElementInfo:");
            //DebugDumpJson (expectedElementInfo);
            //LOG.debugv ("Actual ElementInfo:");
            //DebugDumpJson (actualElementInfo);

            BeFileName actualFile;
            BeTest::GetHost().GetOutputRoot(actualFile);

            WString tmpName = expectedFile.GetFileNameWithoutExtension() + L"_Actual.json";
            actualFile.AppendToPath(tmpName.c_str());
            WriteJsonToFile(actualFile.GetName(), actualElementInfo);

            FAIL() << "Json retrieved from db \n\t" << actualFile.GetName() << "\ndoes not match expected \n\t" << expectedFile.GetName();
            }
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnECNavigatorTest, PlantElementInfo)
    {
    OpenDgnDb (L"04_Plant.i.ibim");
    WCharCP expectedFileName = L"ElementInfo_04_Plant.json";
     
    DgnElementId v9ElementId = GetV9ElementId(140855); // 3645 - Equipment
    ASSERT_TRUE(v9ElementId.IsValid());

    Json::Value actualElementInfo;
    StatusInt elementInfoStatus = GetElementInfo(actualElementInfo, v9ElementId);
    ASSERT_TRUE(elementInfoStatus == SUCCESS);

    ValidateElementInfo(actualElementInfo, expectedFileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnECNavigatorTest, DgnLinksElementInfo)
    {
    OpenDgnDb(L"DgnLinksSample.ibim");
    WCharCP expectedFileName = L"DgnLinksSample.json";
   
    DgnElementId v9ElementId = GetV9ElementId(227); // "DgnLinksSample.dgn"
    ASSERT_TRUE(v9ElementId.IsValid());

    Json::Value actualElementInfo;
    StatusInt elementInfoStatus = GetElementInfo(actualElementInfo, v9ElementId);
    ASSERT_TRUE(elementInfoStatus == SUCCESS);

    ValidateElementInfo(actualElementInfo, expectedFileName);
    }

#ifdef TEST_IFC

// The test requires too large a file and has been commented out until a better 
// test case is found. 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnECNavigatorTest, IfcElementInfo)
    {
    BeFileName testPath(L"IfcMechanicalModel.i.ibim");
    OpenDgnDb(testPath);

    WCharCP expectedFileName = L"IfcMechanicalModel.json";

    DgnElementId v9ElementId = GetV9ElementId(1152921506382597545); // "IfcMechanicalModel.i.dgn"
    ASSERT_TRUE(v9ElementId.IsValid());

    Json::Value actualElementInfo;
    StatusInt elementInfoStatus = GetElementInfo(actualElementInfo, v9ElementId);
    ASSERT_TRUE(elementInfoStatus == SUCCESS);

    // Ignore "$ECInstanceId" in comparison - it's too volatile. 
    for (int ii = 0; ii < (int) actualElementInfo["ecInstances"].size(); ii++)
        {
        // TODO: For some reason the "ThermalTransmittance" property doesn't seem to compare well, even 
        // if the values are exactly same. 
        JsonValueR actualInstance = actualElementInfo["ecInstances"][ii];
        if (actualInstance.isMember("ThermalTransmittance"))
            actualInstance["ThermalTransmittance"] = "<Modified for comparision>";
        }

    ValidateElementInfo(actualElementInfo, expectedFileName);
    }

#endif
