/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/BeDbCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"

BEDB_TESTFILE(EmptyDb)

TESTFILE_CREATEPHYSICAL(EmptyDb)
    {
    Db db;
    db.CreateNewDb(GetResolvedFilePath());
    }

struct BeDbCompatibilityTestFixture : CompatibilityTestFixture 
    {
protected:
    Profile& Profile() const { return ProfileManager().GetProfile(ProfileType::BeDb); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeDbCompatibilityTestFixture, OpenAllVersionInReadWriteMode)
    {
    for (BeFileNameCR testFilePath : Profile().GetAllVersionsOfTestFile(TESTFILE_NAME(EmptyDb)))
        {
        Db db; 
        ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(testFilePath, Db::OpenParams(Db::OpenMode::ReadWrite)));

        const ProfileState profileState = ProfileManager::Get().GetFileProfileState(testFilePath);
        switch (profileState)
            {
                case ProfileState::Current:
                    ASSERT_TRUE(db.TableExists(BEDB_TABLE_Property)) << db.GetDbFileName();
                    break;

                case ProfileState::Newer:
                    ASSERT_TRUE(db.TableExists(BEDB_TABLE_Property)) << db.GetDbFileName();
                    break;

                case ProfileState::Older:
                    ASSERT_TRUE(db.TableExists(BEDB_TABLE_Property)) << db.GetDbFileName();
                    break;

                default:
                    FAIL() << "Unknown ProfileState: " << (int) profileState << " " << db.GetDbFileName();
            }
        }
    }

