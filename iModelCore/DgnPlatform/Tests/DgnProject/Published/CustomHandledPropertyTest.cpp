/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/CustomHandledPropertyTest.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_DPTEST

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Ridha.Malik     02/17
//----------------------------------------------------------------------------------------
struct GetSetCustomHandledProprty : public DgnDbTestFixture
    {

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GetSetCustomHandledProprty, ReadOnly)
    {
    //test Custom Attributes when we get them
    SetupSeedProject();
    DgnClassId classId(m_db->Schemas().GetECClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
    TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
    TestElement el(params);

    //Check a few CustomhandleProperties 
    ECN::ECValue checkValue1, checkValue2;
    uint32_t LMindex , Spindex, Mindex, Gsindex ,Orgindex; 
    DateTime dateTime = DateTime(DateTime::Kind::Utc, 2016, 2, 14, 9, 58, 17, 456);
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(LMindex, "LastMod"));
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(Mindex, "Model"));
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(Spindex, "InSpatialIndex"));
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(Gsindex, "GeometryStream"));

    // Try to set readonly property
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, "LastMod"));
    ASSERT_EQ(DgnDbStatus::ReadOnly,el.SetPropertyValue(LMindex,ECN::ECValue(dateTime)));
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue2, LMindex));
    ASSERT_TRUE(checkValue1.Equals(checkValue2));
    checkValue1.Clear();
    checkValue2.Clear();

    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, "Model"));
    ASSERT_EQ(DgnDbStatus::ReadOnly, el.SetPropertyValue(Mindex, ECN::ECValue(2)));
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue2, Mindex));
    ASSERT_TRUE(checkValue1.Equals(checkValue2));
    checkValue1.Clear();
    checkValue2.Clear();

    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue1, "InSpatialIndex"));
    ASSERT_EQ(DgnDbStatus::ReadOnly, el.SetPropertyValue(Spindex, ECN::ECValue(true)));
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue2, Spindex));
    ASSERT_TRUE(checkValue1.Equals(checkValue2));
    checkValue1.Clear();
    checkValue2.Clear();

    DgnElementCPtr eleid = el.Insert();
    ASSERT_TRUE(eleid.IsValid());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GetSetCustomHandledProprty, InaccessibleProperty)
    {
    //test Custom Attributes when we get them
    SetupSeedProject();
    DgnClassId classId(m_db->Schemas().GetECClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
    TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
    TestElement el(params);

    ECN::ECValue checkValue1;
    uint32_t Gsindex; 
    const static int DataSize = 10;
    Byte DummyData[DataSize] = { 1,2,3,4,5,6,7,8,9,10 };
    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(Gsindex, "GeometryStream"));

    //Try to set inaccessible property
    ASSERT_EQ(DgnDbStatus::BadRequest, el.GetPropertyValue(checkValue1, "GeometryStream"));
    EXPECT_EQ(DgnDbStatus::BadRequest, el.SetPropertyValue(Gsindex, ECN::ECValue(DummyData, sizeof(DummyData))));

    DgnElementCPtr eleid = el.Insert();
    ASSERT_TRUE(eleid.IsValid());
    }