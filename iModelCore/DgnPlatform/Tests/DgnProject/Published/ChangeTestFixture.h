//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/ChangeTestFixture.h $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "DgnHandlersTests.h"
#include "../TestFixture/DgnDbTestFixtures.h"

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct ChangeTestFixture : public DgnDbTestFixture
{
private:
    void CreateSeedDgnDb(BeFileNameR seedPathname);

protected:
    DgnDbPtr m_testDb;
    BeFileName m_testFileName;
    bool    m_wantTestDomain;

    DgnModelId m_testModelId;
    PhysicalModelPtr m_testModel;

    DgnCategoryId m_testCategoryId;

    DgnAuthorityId m_testAuthorityId;
    RefCountedCPtr<NamespaceAuthority> m_testAuthority;

    virtual void _CreateDgnDb();

    void CreateDgnDb() { _CreateDgnDb(); m_testDb->SaveChanges("Saving DgnDb at start of test"); }
    void OpenDgnDb();
    void CloseDgnDb();
        
    DgnCategoryId InsertCategory(Utf8CP categoryName);
    DgnAuthorityId InsertNamespaceAuthority(Utf8CP authorityName);
    DgnElementId InsertPhysicalElement(PhysicalModelR model, DgnCategoryId categoryId, int x, int y, int z);
    
    void CreateDefaultView(DgnModelId defaultModelId);
    void UpdateDgnDbExtents();

public:
    //static void SetUpTestCase();
    //static void TearDownTestCase();
    ChangeTestFixture(WCharCP testFileName, bool wantTestDomain=false);
    virtual ~ChangeTestFixture() {}
    virtual void SetUp() override {}
    virtual void TearDown() override { if (m_testDb.IsValid()) m_testDb->SaveChanges("Saving DgnDb at end of test"); }
};

