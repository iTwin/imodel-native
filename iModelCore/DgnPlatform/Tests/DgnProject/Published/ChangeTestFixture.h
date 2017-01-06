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
protected:
    BeFileName m_testFileName;
    bool    m_wantTestDomain;

    PhysicalModelPtr m_defaultModel;

    DgnAuthorityId m_defaultAuthorityId;
    DatabaseScopeAuthorityCPtr m_defaultAuthority;

    virtual void _SetupDgnDb();

    void SetupDgnDb() { _SetupDgnDb();}
    void OpenDgnDb();
    void CloseDgnDb();
        
    DgnCategoryId InsertCategory(Utf8CP categoryName);
    DgnAuthorityId InsertDatabaseScopeAuthority(Utf8CP authorityName);
    DgnElementId InsertPhysicalElement(PhysicalModelR model, DgnCategoryId categoryId, int x, int y, int z);
    
    void CreateDefaultView(DgnModelId defaultModelId);
    void UpdateDgnDbExtents();

public:
    ChangeTestFixture(WCharCP testFileName, bool wantTestDomain=false);
    virtual ~ChangeTestFixture() {}
    virtual void SetUp() override {}
    virtual void TearDown() override { if (m_db.IsValid()) m_db->SaveChanges("Saving DgnDb at end of test"); }
};

