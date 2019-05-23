/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <DgnPlatform/PhysicalMaterialDomain.h>

//========================================================================================
// @bsiclass                                    Shaun.Sewall                    06/2017
//========================================================================================
struct PhysicalMaterialTests : public DgnDbTestFixture
{
};

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    06/2017
//---------------------------------------------------------------------------------------
TEST_F(PhysicalMaterialTests, CRUD)
    {
    SetupSeedProject();
    DefinitionModelR dictionary = m_db->GetDictionaryModel();

    SchemaStatus status = PhysicalMaterialDomain::GetDomain().ImportSchema(*m_db);
    ASSERT_EQ(SchemaStatus::Success, status);

    DgnClassId aluminumClassId = m_db->Schemas().GetClassId(PHYSICAL_MATERIAL_DOMAIN_NAME, PHYSICAL_MATERIAL_CLASS_Aluminum);
    DgnClassId concreteClassId = m_db->Schemas().GetClassId(PHYSICAL_MATERIAL_DOMAIN_NAME, PHYSICAL_MATERIAL_CLASS_Concrete);
    DgnClassId steelClassId = m_db->Schemas().GetClassId(PHYSICAL_MATERIAL_DOMAIN_NAME, PHYSICAL_MATERIAL_CLASS_Steel);
    ASSERT_TRUE(aluminumClassId.IsValid());
    ASSERT_TRUE(concreteClassId.IsValid());
    ASSERT_TRUE(steelClassId.IsValid());

    PhysicalMaterial aluminum1(dictionary, aluminumClassId, "AA1050A");
    PhysicalMaterial aluminum2(dictionary, aluminumClassId, "AA2011");
    PhysicalMaterial aluminum3(dictionary, aluminumClassId, "AA3103");
    ASSERT_TRUE(aluminum1.Insert().IsValid());
    ASSERT_TRUE(aluminum2.Insert().IsValid());
    ASSERT_TRUE(aluminum3.Insert().IsValid());

    PhysicalMaterial limecrete(dictionary, concreteClassId, "Limecrete");
    PhysicalMaterial shotcrete(dictionary, concreteClassId, "Shotcrete");
    ASSERT_TRUE(limecrete.Insert().IsValid());
    ASSERT_TRUE(shotcrete.Insert().IsValid());

    PhysicalMaterial alloySteel(dictionary, steelClassId, "Alloy Steel");
    PhysicalMaterial lowCarbonSteel(dictionary, steelClassId, "Low Carbon Steel");
    PhysicalMaterial mediumCarbonSteel(dictionary, steelClassId, "Medium Carbon Steel");
    PhysicalMaterial highCarbonSteel(dictionary, steelClassId, "High Carbon Steel");
    PhysicalMaterial stainlessSteel(dictionary, steelClassId, "Stainless Steel");
    ASSERT_TRUE(alloySteel.Insert().IsValid());
    ASSERT_TRUE(lowCarbonSteel.Insert().IsValid());
    ASSERT_TRUE(mediumCarbonSteel.Insert().IsValid());
    ASSERT_TRUE(highCarbonSteel.Insert().IsValid());
    ASSERT_TRUE(stainlessSteel.Insert().IsValid());

    ASSERT_EQ(3, DgnDbTestUtils::SelectCountFromECClass(*m_db, PHYSICAL_MATERIAL_SCHEMA(PHYSICAL_MATERIAL_CLASS_Aluminum)));
    ASSERT_EQ(2, DgnDbTestUtils::SelectCountFromECClass(*m_db, PHYSICAL_MATERIAL_SCHEMA(PHYSICAL_MATERIAL_CLASS_Concrete)));
    ASSERT_EQ(5, DgnDbTestUtils::SelectCountFromECClass(*m_db, PHYSICAL_MATERIAL_SCHEMA(PHYSICAL_MATERIAL_CLASS_Steel)));
    ASSERT_EQ(10, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA(BIS_CLASS_PhysicalMaterial)));
    }
