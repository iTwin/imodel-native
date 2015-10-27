//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/ChangeTestFixture.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    DgnModelPtr m_testModel;
    DgnCategoryId m_testCategoryId;
    RefCountedPtr<NamespaceAuthority> m_testAuthority;

    void CreateDgnDb(WCharCP testFileName);
    void OpenDgnDb(WCharCP testFileName);
    void CloseDgnDb();
        
    void InsertModel();
    void InsertCategory();
    void InsertAuthority();
    DgnElementId InsertElement(int x, int y, int z);
    
    void CreateDefaultView();
    void UpdateDgnDbExtents();
public:
    ChangeTestFixture() {}
    virtual ~ChangeTestFixture() {}
};

