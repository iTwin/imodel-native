/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "DgnHandlersTests.h"
#include <DgnPlatform/PlatformLib.h>
#include <Bentley/BeTimeUtilities.h>
#include "../TestFixture/DgnDbTestFixtures.h"

//=======================================================================================
//! Test Fixtrue for tests
// @bsiclass
//=======================================================================================
struct ElementDisplayProperties : public DgnDbTestFixture
{

};

/*---------------------------------------------------------------------------------**//**
* Test for Setting and Getting Transparency.
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
