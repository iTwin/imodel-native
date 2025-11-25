/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_EC

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ProfileUpgradeTestFixture : public DgnDbTestFixture
{
    BeFileName CopyProfileUpgradeTestFile(WCharCP targetFileName)
        {
        BeFileName sourceFileName;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(sourceFileName);
        sourceFileName.AppendToPath(L"CompatibilityTestFiles");
        sourceFileName.AppendToPath(L"dgndb-2-0-0-7");
        sourceFileName.AppendToPath(L"allProperties.bim");
        EXPECT_TRUE(sourceFileName.DoesPathExist());

        BeFileName destFileName;
        BeTest::GetHost().GetOutputRoot(destFileName);
        destFileName.AppendToPath(targetFileName);

        BeFileNameStatus copyStatus = BeFileName::BeCopyFile(sourceFileName, destFileName);
        EXPECT_EQ(BeFileNameStatus::Success, copyStatus) << "Failed to copy " << sourceFileName.GetName() << " to " << destFileName.GetName();

        return destFileName;
        }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ProfileUpgradeTestFixture, UpgradeProfileFrom2_0_0_7)
    {
    // Create a local copy using helper method
    BeFileName destFileName = CopyProfileUpgradeTestFile(L"UpgradeDgnDbProfileTest.bim");
    ASSERT_TRUE(destFileName.DoesPathExist());

    // Open the database without profile upgrade to check the original version
    DbResult result;
    m_db = DgnDb::OpenIModelDb(&result, destFileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    ASSERT_EQ(BE_SQLITE_OK, result) << "Failed to open file " << Utf8String(destFileName);
    ASSERT_TRUE(m_db.IsValid());

    // Capture the original profile version
    DgnDbProfileVersion originalVersion = m_db->GetProfileVersion();
    EXPECT_TRUE(originalVersion.IsPast());
    EXPECT_FALSE(originalVersion.IsCurrent()) << "Original profile version should be older than current version";

    // Make sure element 2 does not exist
    DgnElementCPtr element = m_db->Elements().GetElement(m_db->Elements().GetSchemaSubjectId());
    EXPECT_FALSE(element.IsValid()) << "Element with ID 2 should not exist before profile upgrade";
    CloseDb();

    // Reopen with SchemaUpgradeOptions to trigger profile upgrade
    auto upgradeParams = DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, 
                                           SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade));
    upgradeParams.m_schemaLockHeld = true;
    m_db = DgnDb::OpenIModelDb(&result, destFileName, upgradeParams);
    ASSERT_EQ(BE_SQLITE_OK, result) << "Failed to open file with upgrade params";
    ASSERT_TRUE(m_db.IsValid());

    // Verify the profile was upgraded to current version
    DgnDbProfileVersion currentVersion = m_db->GetProfileVersion();
    EXPECT_TRUE(currentVersion.IsCurrent()) << "Profile version after upgrade is not current: "
        << currentVersion.ToString();
    EXPECT_TRUE(currentVersion.CompareTo(originalVersion) >= 0) << "Profile version after upgrade is older than original version";

    // Check if element with ID 2 was created
    DgnElementCPtr element2 = m_db->Elements().GetElement(m_db->Elements().GetSchemaSubjectId());
    EXPECT_TRUE(element2.IsValid()) << "Element with ID 2 not found after profile upgrade";

    SaveDb();
    CloseDb();
    }