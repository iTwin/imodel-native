//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/AnnotationLeader_Test.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "AnnotationTestFixture.h"

//=======================================================================================
// @bsiclass                                                   Umar.Hayat     07/15
//=======================================================================================
struct AnnotationLeaderTest : public AnnotationTestFixture
{

}; // AnnotationLeaderTest

#define DECLARE_AND_SET_DATA_1(LEADER_PTR)\
    AnnotationLeaderTargetAttachmentType type = AnnotationLeaderTargetAttachmentType::PhysicalPoint;    LEADER_PTR->SetTargetAttachmentType(type);\
    DPoint3d point = DPoint3d::From(0.0, 0.0);                                                          LEADER_PTR->SetTargetAttachmentDataForPhysicalPoint(point);

#define VERIFY_DATA_1(LEADER_PTR)\
    EXPECT_TRUE(type == LEADER_PTR->GetTargetAttachmentType());\
    EXPECT_TRUE(point.IsEqual(LEADER_PTR->GetTargetAttachmentDataForPhysicalPoint()));

//---------------------------------------------------------------------------------------
// Creates a style and tests accessors.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(AnnotationLeaderTest, DefaultsAndAccessors)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb();

    //.............................................................................................
    AnnotationLeaderStylePtr style = createAnnotationLeaderStyle(project, "TestLeaderStyle");
    AnnotationLeaderPtr leader = AnnotationLeader::Create(project,style->GetElementId());
    ASSERT_TRUE(leader.IsValid());

    // Basics
    EXPECT_TRUE(&project == &leader->GetDbR());
    EXPECT_TRUE(style->GetElementId() == leader->GetStyleId());

    AnnotationLeaderPtr leader2 = AnnotationLeader::Create(project, style->GetElementId());
    ASSERT_TRUE(leader2.IsValid());

    EXPECT_TRUE(&project == &leader->GetDbR());
    leader->SetStyleId(style->GetElementId(),SetAnnotationLeaderStyleOptions::Default);
    EXPECT_TRUE(style->GetElementId() == leader->GetStyleId());
    leader->SetTargetAttachmentType(AnnotationLeaderTargetAttachmentType::PhysicalPoint);
    EXPECT_TRUE(AnnotationLeaderTargetAttachmentType::PhysicalPoint == leader->GetTargetAttachmentType());
    DPoint3d attachementPoint = DPoint3d::FromOne();
    leader->SetTargetAttachmentDataForPhysicalPoint(attachementPoint);
    EXPECT_TRUE(attachementPoint.IsEqual(leader->GetTargetAttachmentDataForPhysicalPoint()));
    leader->SetSourceAttachmentType(AnnotationLeaderSourceAttachmentType::Id);
    EXPECT_TRUE(AnnotationLeaderSourceAttachmentType::Id == leader->GetSourceAttachmentType());
    leader->SetSourceAttachmentDataForId(5);
    EXPECT_TRUE(5 == leader->GetSourceAttachmentDataForId());
    }

//---------------------------------------------------------------------------------------
// Creates and clones a style.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(AnnotationLeaderTest, DeepCopy)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb();

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

