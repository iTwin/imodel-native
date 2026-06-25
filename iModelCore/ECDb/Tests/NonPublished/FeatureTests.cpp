/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// Tests for the ec_Feature table and FeatureManager string registry.
//=======================================================================================

struct FeatureTests : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// ec_Feature table must be freshly created for every new 4.0.0.6 ECDb file, should have
//  correct columns and should be empty
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, FeatureTableSetup)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("feature_table_exists.ecdb"));
    ProfileVersion const& actualVersion = m_ecdb.GetECDbProfileVersion();
    EXPECT_EQ(ProfileVersion(4, 0, 0, 6), actualVersion) << "Fresh ECDb must be at profile 4.0.0.6 after Task 2.1";

    EXPECT_TRUE(GetHelper().TableExists("ec_Feature")) << "ec_Feature table must exist in a fresh ECDb file (profile 4.0.0.6)";

    EXPECT_TRUE(GetHelper().ColumnExists("ec_Feature", "Name")) << "ec_Feature must have a Name column";
    EXPECT_TRUE(GetHelper().ColumnExists("ec_Feature", "Label")) << "ec_Feature must have a Label column";
    EXPECT_TRUE(GetHelper().ColumnExists("ec_Feature", "Compat")) << "ec_Feature must have a Compat column";

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ec_Feature")) << "Failed to prepare SELECT COUNT(*) on ec_Feature";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0)) << "ec_Feature table must be empty on a fresh ECDb file";
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// Opening an ECDb with an empty feature registry must succeed in read-write mode.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, EmptyRegistry_OpenReadWriteSucceeds)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("feature_open_rw.ecdb"))
        << "Opening a fresh ECDb with an empty feature registry must succeed";
    EXPECT_TRUE(m_ecdb.IsDbOpen());
    }

//---------------------------------------------------------------------------------------
// Opening an ECDb with an empty feature registry must succeed in read-only mode.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, EmptyRegistry_OpenReadonlySucceeds)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("feature_open_ro.ecdb"));
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK,
        m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly)))
        << "Read-only open of a fresh ECDb with an empty feature registry must succeed";
    EXPECT_TRUE(m_ecdb.IsDbOpen());
    }

//---------------------------------------------------------------------------------------
// Opening an ECDb with an empty feature registry must not insert any rows
// into ec_Feature (the registry check must be a pure read operation at open-time).
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(FeatureTests, EmptyRegistry_OpenDoesNotPopulateTable)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("feature_open_nopop.ecdb"));
    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::ReadWrite)));

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ec_Feature"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0))
        << "Opening an ECDb must not insert rows into ec_Feature when the registry is empty";
    }

END_ECDBUNITTESTS_NAMESPACE
