/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <ECDb/ECDbApi.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include "../TestFixture/DgnDbTestFixtures.h"
//#if defined (_MSC_VER)
//#pragma warning (disable:4702)
//#endif

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST
extern void DebugDumpJson (const Json::Value& jsonValue);
extern bool WriteJsonToFile (WCharCP path, const Json::Value& jsonValue);
extern bool ReadJsonFromFile (Json::Value& jsonValue, WCharCP path);

//=======================================================================================    
//! @bsiclass                                                Ramanujam.Raman      02/2014
//=======================================================================================   
struct DgnECNavigatorTest :public DgnDbTestFixture
{
    /* DgnECPersistence is deprecated. Adjust test.
    BentleyStatus GetElementInfo(Json::Value& elementInfo, DgnElementId selectedElementId)
        {
        Json::Value jsonInstances, jsonDisplayInfo; // Note: Cannot just pass actualElemtnInfo["ecInstances"], actualElementInfo["ecDisplayInfo"] here. 
        BentleyStatus status = DgnECPersistence::GetElementInfo(jsonInstances, jsonDisplayInfo, selectedElementId, *m_db);
        if (status != SUCCESS)
            {
            BeAssert(false);
            return status;
            }
        
        elementInfo["ecInstances"] = jsonInstances;
        elementInfo["ecDisplayInfo"] = jsonDisplayInfo;
        return SUCCESS;
        }
        */

    static void ValidateElementInfo(JsonValueR actualElementInfo, WCharCP expectedFileName)
        {
        BeFileName expectedFile(expectedFileName);
        Json::Value expectedElementInfo;
        bool readFileStatus = ReadJsonFromFile(expectedElementInfo, expectedFile.GetName());
        ASSERT_TRUE(readFileStatus);

        const char* volatileProperties[] =
            {
            "id",
            "modelId",
            "name",
            "lastMod",
            "code"
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
            WString tmpName = expectedFile.GetFileNameWithoutExtension() + L"_Actual.json";
            actualFile.AppendToPath(expectedFile.GetDirectoryName());
            actualFile.AppendToPath(tmpName.c_str());
            WriteJsonToFile(actualFile.GetName(), actualElementInfo);

            FAIL() << "Json retrieved from db \n\t" << actualFile.GetName() << "\ndoes not match expected \n\t" << expectedFile.GetName();
            }
        }
};

/* DgnECPersistence was deprecated -> Rewrite this test or remove it if no longer applicable 
//---------------------------------------------------------------------------------
// @bsimethod                                   Ridha.Malik                   11/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnECNavigatorTest, GetElementInfo)
    {
    // Test need to be enhance when the issue of ElemtmentInfo get resolved 
    SetupSeedProject();
    // Inserts element
    DgnElementPtr ele = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId);
    DgnElementCPtr id=ele->Insert();
    ASSERT_TRUE(id.IsValid());
    m_db->SaveChanges();
    Json::Value actualElementInfo;
    Json::Value expectedElementInfo;
    StatusInt expectedElementInfoStatus = GetElementInfo(expectedElementInfo, id->GetElementId());
    ASSERT_TRUE(expectedElementInfoStatus == SUCCESS);
    StatusInt actualElementInfoStatus = GetElementInfo(expectedElementInfo, id->GetElementId());
    ASSERT_TRUE(actualElementInfoStatus == SUCCESS);
   // ValidateElementInfo(actualElementInfo, expectedFileName);
    }
    */
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST_F(DgnECNavigatorTest, DgnLinksElementInfo)
//    {
//    OpenDgnDb(L"DgnLinksSample.ibim");
//    WCharCP expectedFileName = L"DgnLinksSample.json";
//   
//    DgnElementId v9ElementId = GetV9ElementId(227); // "DgnLinksSample.dgn"
//    ASSERT_TRUE(v9ElementId.IsValid());
//
//    Json::Value actualElementInfo;
//    StatusInt elementInfoStatus = GetElementInfo(actualElementInfo, v9ElementId);
//    ASSERT_TRUE(elementInfoStatus == SUCCESS);
//
//  //  ValidateElementInfo(actualElementInfo, expectedFileName);
//    }

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
