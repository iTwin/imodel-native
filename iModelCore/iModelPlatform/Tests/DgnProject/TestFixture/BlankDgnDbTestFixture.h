/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnElementHelpers.h"

#define LOCALIZED_STR(str) str

USING_NAMESPACE_BENTLEY_DGN
USING_DGNDB_UNIT_TESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct BlankDgnDbTestFixture : public ::testing::Test
    {
protected:
    ScopedDgnHost           m_host;
    DgnDbPtr                m_db;

    void SetupProject(WCharCP dbname)
        {
        m_db = CreateDb(dbname);
        }

        DgnDbPtr CreateDb(WCharCP dbname)
        {
        BeFileName filename = DgnDbTestDgnManager::GetOutputFilePath(dbname);
        BeFileName::BeDeleteFile(filename);

        CreateDgnDbParams params;
        params.SetProjectName("BlankDgnDbTestFixture");
        params.SetOverwriteExisting(false);
        DbResult status;
        DgnDbPtr db = DgnDb::CreateIModel(&status, filename, params);
        EXPECT_EQ(BE_SQLITE_OK, status) << status;
        return db;
        }

    DgnDbR      GetDb()
        {
        return *m_db;
        }
    };
