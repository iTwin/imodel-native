//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/AnnotationLeader_Test.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "AnnotationTestFixture.h"

// Republish API:           bb re DgnPlatform:PublishedApi
// Rebuild API:             bb re DgnPlatformDll
// Republish seed files:    bb re UnitTests_Documents
// Rebuild test:            bb re DgnProjectUnitTests BeGTestExe
// All code:                bb re DgnPlatform:PublishedApi DgnPlatformDll DgnProjectUnitTests BeGTestExe
// Run test:                %SrcRoot%BeGTest\RunTests.py -ax64 --gtest_filter="BasicAnnotationLeaderTest.*:AnnotationLeaderTest.*"

//=======================================================================================
// @bsiclass                                                   Umar.Hayat     07/15
//=======================================================================================
struct AnnotationLeaderTest : public AnnotationTestFixture
{
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Umar.Hayat     07/15
    //---------------------------------------------------------------------------------------
public: AnnotationLeaderTest() :
    AnnotationTestFixture(__FILE__, false /*2D*/, false /*needBriefcase*/)
        {
        }

}; // AnnotationLeaderTest

#define DECLARE_AND_SET_DATA_1(LEADER_PTR)\
    AnnotationLeaderTargetAttachmentType type = AnnotationLeaderTargetAttachmentType::PhysicalPoint;    LEADER_PTR->SetTargetAttachmentType(type);\
    DPoint3d point = DPoint3d::From(0.0, 0.0);                                                          LEADER_PTR->SetTargetAttachmentDataForPhysicalPoint(&point);

#define VERIFY_DATA_1(LEADER_PTR)\
    EXPECT_TRUE(type == LEADER_PTR->GetTargetAttachmentType());\
    EXPECT_TRUE(point.IsEqual(*LEADER_PTR->GetTargetAttachmentDataForPhysicalPoint()));

//---------------------------------------------------------------------------------------
// Creates a style and tests accessors.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(AnnotationLeaderTest, DefaultsAndAccessors)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    AnnotationLeaderStylePtr style = createAnnotationLeaderStyle(project, "TestLeaderStyle");
    AnnotationLeaderPtr leader = AnnotationLeader::Create(project,style->GetElementId());
    ASSERT_TRUE(leader.IsValid());

    // Basics
    EXPECT_TRUE(&project == &leader->GetDbR());
    EXPECT_TRUE(style->GetElementId() == leader->GetStyleId());

    AnnotationLeaderPtr leader2 = AnnotationLeader::Create(project, style->GetElementId());
    ASSERT_TRUE(leader2.IsValid());

    // Basics
    EXPECT_TRUE(&project == &leader->GetDbR());
    leader->SetStyleId(style->GetElementId(),SetAnnotationLeaderStyleOptions::Default);
    EXPECT_TRUE(style->GetElementId() == leader->GetStyleId());
    leader->SetTargetAttachmentType(AnnotationLeaderTargetAttachmentType::PhysicalPoint);
    EXPECT_TRUE(AnnotationLeaderTargetAttachmentType::PhysicalPoint == leader->GetTargetAttachmentType());
    
    
    }

//---------------------------------------------------------------------------------------
// Creates and clones a style.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(AnnotationLeaderTest, DeepCopy)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    AnnotationLeaderPtr leader = AnnotationLeader::Create(project);
    ASSERT_TRUE(leader.IsValid());
    

    DECLARE_AND_SET_DATA_1(leader);

    //.............................................................................................
    AnnotationLeaderPtr clonedLeader = leader->Clone();
    ASSERT_TRUE(clonedLeader.IsValid());
    ASSERT_TRUE(leader.get() != clonedLeader.get());
    
    EXPECT_TRUE(&project == &clonedLeader->GetDbR());
    EXPECT_TRUE(leader->GetStyleId() == clonedLeader->GetStyleId());
    VERIFY_DATA_1(clonedLeader);
    }

