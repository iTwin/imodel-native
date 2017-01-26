//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/ChangeTestFixture.h $
//  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "DgnHandlersTests.h"
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct ChangeTestFixture : public DgnDbTestFixture
{
protected:

    PhysicalModelPtr m_defaultModel;

    CodeSpecCPtr m_defaultCodeSpec;

    void SetupDgnDb(BeFileName seedFileName, WCharCP newFileName);
    void OpenDgnDb(BeFileName fileName);
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

