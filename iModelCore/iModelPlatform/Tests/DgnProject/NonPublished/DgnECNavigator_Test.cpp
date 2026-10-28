/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
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
extern bool WriteJsonToFile (WCharCP path, BeJsConst jsonValue);
extern bool ReadJsonFromFile (BeJsDocument& jsonValue, WCharCP path);

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct DgnECNavigatorTest :public DgnDbTestFixture
{
    static void ValidateElementInfo(BeJsValue actualElementInfo, WCharCP expectedFileName)
        {
        BeFileName expectedFile(expectedFileName);
        BeJsDocument expectedElementInfo;
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
            BeJsValue actualInstance = actualElementInfo["ecInstances"][ii];
            BeJsValue expectedInstance = expectedElementInfo["ecInstances"][ii];

            for (const char* prop : volatileProperties)
                {
                // hasMember (not isMember): isMember reports false for a member that is
                // present but null.
                if (actualInstance.hasMember(prop)) actualInstance[prop] = "*";
                if (expectedInstance.hasMember(prop)) expectedInstance[prop] = "*";
                }
            }

        if (!expectedElementInfo.isExactEqual(actualElementInfo))
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

