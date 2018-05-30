/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/DgnDbCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"

DGNDB_TESTFILE(EmptyBim)

TESTFILE_CREATEPHYSICAL(EmptyBim)
    {
    CreateDgnDbParams createParam(GetFileName());
    DbResult status;
    DgnDb::CreateDgnDb(&status, GetResolvedFilePath(), createParam);
    }

struct DgnDbCompatibilityTestFixture : CompatibilityTestFixture
    {
    ScopedDgnHost m_host;

    protected:
       Profile& Profile() const { return ProfileManager().GetProfile(ProfileType::DgnDb); }
    };

//===========================================================Test===========================================================
//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnDbCompatibilityTestFixture, OpenAllVersionInReadWriteMode)
    {
    for (BeFileNameCR testFilePath : Profile().GetAllVersionsOfTestFile(TESTFILE_NAME(EmptyBim)))
        {
        DbResult r = BE_SQLITE_OK;
        DgnDbPtr dgndb = DgnDb::OpenDgnDb(&r, testFilePath, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite)); 
        ASSERT_EQ(BE_SQLITE_OK, r);

        const ProfileState profileState = ProfileManager::Get().GetFileProfileState(testFilePath);
        switch (profileState)
            {
                case ProfileState::Current:
                    ASSERT_TRUE(dgndb->TableExists("dgn_txns")) << dgndb->GetDbFileName();
                    break;

                case ProfileState::Newer:
                    ASSERT_TRUE(dgndb->TableExists("dgn_txns")) << dgndb->GetDbFileName();
                    break;

                case ProfileState::Older:
                    ASSERT_TRUE(dgndb->TableExists("dgn_txns")) << dgndb->GetDbFileName();
                    break;

                default:
                    FAIL() << "Unknown ProfileState: " << (int) profileState << " " << dgndb->GetDbFileName();
            }
        }
    }

