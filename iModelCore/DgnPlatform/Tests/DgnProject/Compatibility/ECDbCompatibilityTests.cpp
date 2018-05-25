/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/ECDbCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"

ECDB_TESTFILE(EmptyECDb)

TESTFILE_CREATEPHYSICAL(EmptyECDb)
    {
    ECDb db;
    db.CreateNewDb(GetResolvedFilePath());
    }

struct ECDbCompatibilityTestFixture : CompatibilityTestFixture 
    {
    protected:
        Profile& Profile() const { return ProfileManager().GetProfile(ProfileType::ECDb); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, OpenAllVersionInReadWriteMode)
    {
    for (BeFileNameCR testFilePath : Profile().GetAllVersionsOfTestFile(TESTFILE_NAME(EmptyECDb)))
        {
        ECDb testFile;
        ASSERT_EQ(BE_SQLITE_OK, testFile.OpenBeSQLiteDb(testFilePath, Db::OpenParams(Db::OpenMode::ReadWrite)));

        const ProfileState profileState = ProfileManager().GetFileProfileState(testFilePath);
        switch (profileState)
            {
                case ProfileState::Current:
                    ASSERT_TRUE(testFile.TableExists("ec_Class")) << testFile.GetDbFileName();
                    break;

                case ProfileState::Newer:
                    ASSERT_TRUE(testFile.TableExists("ec_Class")) << testFile.GetDbFileName();
                    break;

                case ProfileState::Older:
                    ASSERT_TRUE(testFile.TableExists("ec_Class")) << testFile.GetDbFileName();
                    break;

                default:
                    FAIL() << "Unknown ProfileState: " << (int) profileState << " " << testFile.GetDbFileName();
            }
        }
    }

