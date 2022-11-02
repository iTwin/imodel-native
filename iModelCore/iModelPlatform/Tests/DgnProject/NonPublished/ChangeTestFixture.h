//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------

#include "DgnHandlersTests.h"
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ChangeTestFixture : public DgnDbTestFixture
{
protected:

    PhysicalModelPtr m_defaultModel;

    CodeSpecCPtr m_defaultCodeSpec;

    void SetupDgnDb(BeFileName seedFileName, WCharCP newFileName);
    void OpenDgnDb(BeFileName fileName, Db::OpenMode openMode = Db::OpenMode::ReadWrite);
    void OpenDgnDb(BeFileName fileName, DgnDb::OpenParams openParams);
    void CloseDgnDb();
    DgnCategoryId InsertCategory(Utf8CP categoryName);
    static DgnElementId InsertPhysicalElement(DgnDbR db, PhysicalModelR model, DgnCategoryId categoryId, int x, int y, int z);

    static void CreateDefaultView(DgnDbR db);

public:
    static DgnPlatformSeedManager::SeedDbInfo s_seedFileInfo;
    static CodeSpecId m_defaultCodeSpecId;

    ChangeTestFixture(){}
    virtual ~ChangeTestFixture() {}
    static void SetUpTestCase();

    void SetUp() override {}
    void TearDown() override { if (m_db.IsValid()) m_db->SaveChanges("Saving DgnDb at end of test"); }
};

