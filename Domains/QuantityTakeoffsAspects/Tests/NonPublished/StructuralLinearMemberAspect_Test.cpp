/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "QuantityTakeoffsAspectsTestFixtureBase.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/GenericDomain.h>
#include <BeSQLite/BeSQLite.h>
#include <QuantityTakeoffsAspects/Elements/StructuralLinearMemberAspect.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
// Sets up environment for StructuralLinearMemberAspect testing.
// @bsiclass                                    Elonas.Seviakovas                05/2019
//=======================================================================================
struct StructuralLinearMemberAspectTestFixture : public QuantityTakeoffsAspectsTestFixtureBase
    {
    public:
        StructuralLinearMemberAspectTestFixture() {};
        ~StructuralLinearMemberAspectTestFixture() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StructuralLinearMemberAspectTestFixture, Create)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = CreateAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    double crossSectionalArea1 = 150.0f;
    Utf8String sectionName1 = "Support Pillar";
    StructuralFramingType type1 = StructuralFramingType::Column;

    StructuralLinearMemberAspectPtr aspect = StructuralLinearMemberAspect::Create(crossSectionalArea1, sectionName1, type1);
    ASSERT_TRUE(aspect.IsValid());

    StructuralLinearMemberAspect::SetAspect(*object, *aspect);

    object->Update();

    StructuralLinearMemberAspectPtr aspect2 = StructuralLinearMemberAspect::GetP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(crossSectionalArea1, aspect2->GetCrossSectionalArea());
    ASSERT_EQ(sectionName1, *aspect2->GetSectionName());
    ASSERT_EQ(type1, aspect2->GetType());

    double crossSectionalArea2 = 500.0f;
    Utf8String sectionName2 = "Panel";
    StructuralFramingType type2 = StructuralFramingType::Cladding;

    aspect2->SetCrossSectionalArea(crossSectionalArea2);
    aspect2->SetSectionName(sectionName2);
    aspect2->SetType(type2);

    object->Update();
    StructuralLinearMemberAspectCPtr aspect3 = StructuralLinearMemberAspect::GetCP(*object);
    ASSERT_TRUE(aspect3.IsValid());

    ASSERT_EQ(crossSectionalArea2, aspect3->GetCrossSectionalArea());
    ASSERT_EQ(sectionName2, *aspect3->GetSectionName());
    ASSERT_EQ(type2, aspect3->GetType());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE