//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/ChangeTestFixture.h $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "DgnHandlersTests.h"

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct ChangeTestFixture : public testing::Test
{
private:
    void CreateSeedDgnDb(BeFileNameR seedPathname);

protected:
    Dgn::ScopedDgnHost m_testHost;
    DgnDbPtr m_testDb;
    WString m_testFileName;

    DgnModelId m_testModelId;
    SpatialModelPtr m_testModel;

    DgnCategoryId m_testCategoryId;

    DgnAuthorityId m_testAuthorityId;
    RefCountedCPtr<NamespaceAuthority> m_testAuthority;

    virtual void _CreateDgnDb();

    void CreateDgnDb() { _CreateDgnDb(); m_testDb->SaveChanges("Saving DgnDb at start of test"); }
    void OpenDgnDb();
    void CloseDgnDb();
        
    DgnModelId InsertSpatialModel(Utf8CP modelName);
    DgnCategoryId InsertCategory(Utf8CP categoryName);
    DgnAuthorityId InsertNamespaceAuthority(Utf8CP authorityName);
    DgnElementId InsertPhysicalElement(SpatialModelR model, DgnCategoryId categoryId, int x, int y, int z);
    
    void CreateDefaultView(DgnModelId defaultModelId);
    void UpdateDgnDbExtents();

public:
    ChangeTestFixture(WCharCP testFileName) : m_testFileName (testFileName) {}
    virtual ~ChangeTestFixture() {}
    virtual void SetUp() override {}
    virtual void TearDown() override { if (m_testDb.IsValid()) m_testDb->SaveChanges("Saving DgnDb at end of test"); }
};

