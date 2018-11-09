/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ElementProperties.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTimeUtilities.h>
#include "../TestFixture/DgnDbTestFixtures.h"

//=======================================================================================
//! Test Fixtrue for tests
// @bsiclass                                                     Majd.Uddin      06/15
//=======================================================================================
struct ElementDisplayProperties : public DgnDbTestFixture
{

};

/*---------------------------------------------------------------------------------**//**
* Test for Setting and Getting Transparency.
* @bsimethod                                    Maha.Nasir      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ElementDisplayProperties, SetTransparency)
    {
    SetupSeedProject();
    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, TEST_NAME);

    Render::GeometryParams ep;
    ep.SetCategoryId(m_defaultCategoryId);
    ep.SetTransparency(0.5);

    DgnElementCPtr pE1 = InsertElement(ep, model->GetModelId());
    EXPECT_TRUE(pE1.IsValid());

    GeometrySourceCP geomElem = pE1->ToGeometrySource();
    GeometryCollection collection(*geomElem);

    for (auto iter : collection)
        {
        GeometricPrimitivePtr geom = iter.GetGeometryPtr();

        if (!geom.IsValid())
            continue;

        Render::GeometryParamsCR params = iter.GetGeometryParams();
        EXPECT_EQ(0.5, params.GetTransparency());
        EXPECT_EQ(0.5, params.GetNetTransparency());
        }
    }

/*---------------------------------------------------------------------------------**//**
* Test for Setting and Getting Cateogory and SubCategory Id.
* @bsimethod                                    Maha.Nasir      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ElementDisplayProperties, SetCategory)
    {
    SetupSeedProject();
    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, TEST_NAME);

    Render::GeometryParams ep;
    ep.SetCategoryId(m_defaultCategoryId);

    DgnElementCPtr pE1 = InsertElement(ep, model->GetModelId());
    EXPECT_TRUE(pE1.IsValid());

    GeometrySourceCP geomElem = pE1->ToGeometrySource();
    GeometryCollection collection(*geomElem);

    for (auto iter : collection)
        {
        GeometricPrimitivePtr geom = iter.GetGeometryPtr();

        if (!geom.IsValid())
            continue;

        Render::GeometryParamsCR params = iter.GetGeometryParams();
        DgnCategoryId CId = params.GetCategoryId();
        ASSERT_TRUE (CId.IsValid());
        //Setting the Category Id also sets the SubCategory to the default.
        DgnSubCategoryId SId = params.GetSubCategoryId();
        ASSERT_TRUE (SId.IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* Test for Setting and Getting Display Parameters.
* @bsimethod                                    Maha Nasir      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ElementDisplayProperties, SetDisplayParams)
    {
    SetupSeedProject();
    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, TEST_NAME);

    Render::GeometryParams ep;
    ep.SetCategoryId(m_defaultCategoryId);
    ep.SetWeight(21);

    DgnElementCPtr pE1 = InsertElement(ep, model->GetModelId());
    EXPECT_TRUE(pE1.IsValid());

    GeometrySourceCP geomElem = pE1->ToGeometrySource();
    GeometryCollection collection(*geomElem);

    for (auto iter : collection)
        {
        GeometricPrimitivePtr geom = iter.GetGeometryPtr();

        if (!geom.IsValid())
            continue;

        Render::GeometryParamsCR params = iter.GetGeometryParams();
        EXPECT_EQ(21, params.GetWeight());
        bool weight = params.IsWeightFromSubCategoryAppearance();
        EXPECT_FALSE(weight);
        }
    }
