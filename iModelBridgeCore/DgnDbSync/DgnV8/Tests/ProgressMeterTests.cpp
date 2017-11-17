/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/ProgressMeterTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"
#include <VersionedDgnV8Api/ECObjects/ECObjectsAPI.h>

BEGIN_UNNAMED_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                      08/15
//----------------------------------------------------------------------------------------
struct ProgressMeterTests : public ConverterTestBaseFixture
{
    DEFINE_T_SUPER(ConverterTestBaseFixture);
    void SetUp();
    void TearDown();
};

END_UNNAMED_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ProgressMeterTests::SetUp()
    {
    T_Super::SetUp();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ProgressMeterTests::TearDown()
    {
    T_Super::TearDown();
    }
//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                      08/15
//----------------------------------------------------------------------------------------
struct TestDgnProgressMeter : public DgnProgressMeter
{

    virtual Abort _ShowProgress() override { 
        printf("Progress meter is showing progress... \n");
        return ABORT_Yes; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProgressMeterTests, ProgressOnAbort)
    {
    LineUpFiles(L"ProgressOnAbort.ibim", L"Test3d.dgn", false);

    TestDgnProgressMeter * meter = new TestDgnProgressMeter();
    T_HOST.SetProgressMeter(meter);
    
    // *** TRICKY: the converter takes a reference to and will MODIFY its Params. Make a copy, so that it does not pollute m_params.
    RootModelConverter::RootModelSpatialParams params(m_params);

    params.SetInputFileName(m_v8FileName);
    params.SetBridgeRegSubKey(RootModelConverter::GetRegistrySubKey());

    RootModelConverter creator(params);
    creator.SetWantDebugCodes(true);
    auto db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db.IsValid());
    creator.SetDgnDb(*db);
    creator.SetIsUpdating(false);
    creator.AttachSyncInfo();
    ASSERT_EQ(BentleyApi::SUCCESS, creator.InitRootModel());
    creator.MakeSchemaChanges();
    ASSERT_TRUE( creator.WasAborted() );
    ASSERT_EQ(RootModelConverter::ImportJobCreateStatus::Success, creator.InitializeJob());
    creator.Process();
    ASSERT_TRUE( creator.WasAborted() );
    db->SaveChanges();

    T_HOST.SetProgressMeter(false);
    }
