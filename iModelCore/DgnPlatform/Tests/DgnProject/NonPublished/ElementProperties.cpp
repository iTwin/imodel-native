/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ElementProperties.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECSqlBuilder.h>
#include "../TestFixture/DgnDbTestFixtures.h"

//=======================================================================================
//! Test Fixtrue for tests
// @bsiclass                                                     Majd.Uddin      06/15
//=======================================================================================
struct ElementDisplayProperties : public DgnDbTestFixture
{

};

/*---------------------------------------------------------------------------------**//**
* Test for Setting and Getting Gradient properties.
* @bsimethod                                    Maha.Nasir      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ElementDisplayProperties, SetGradient)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementDisplayProperties.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    auto seedModelId3 = m_defaultModelId;

    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId3);
    DgnModelPtr model3 = seedModel->Clone(DgnModel::CreateModelCode("model3"));
    ASSERT_TRUE (model3 != nullptr);
    model3->Insert();
    DgnModelId m3id = m_db->Models().QueryModelId(DgnModel::CreateModelCode("model3"));

    ElemDisplayParams ep;
    ep.SetCategoryId(m_defaultCategoryId);
    ep.SetFillDisplay(FillDisplay::Always);

    GradientSymbPtr   gradient = GradientSymb::Create();
    double   keyValues[2];
    ColorDef    keyColors[2];

    keyValues[0] = 0.0;
    keyValues[1] = 0.5;
    keyColors[0] = ColorDef::Yellow();
    keyColors[1] = ColorDef::Red();

    gradient->SetMode(GradientMode::Spherical);
    gradient->SetFlags(0);
    gradient->SetAngle(8.0);
    gradient->SetTint(1.0);
    gradient->SetShift(1.0);
    gradient->SetKeys(2, keyColors, keyValues);
    ep.SetGradient(gradient.get());

    DgnElementCPtr pE1 = InsertElement(DgnElement::Code(), ep, m3id);
    EXPECT_TRUE(pE1.IsValid());

    GeometricElementP geomElem = const_cast<GeometricElementP>(pE1->ToGeometricElement());
    ElementGeometryCollection collection(*geomElem);

    for (ElementGeometryPtr geom : collection)
        {
        ElemDisplayParamsCR params = collection.GetElemDisplayParams();
        GradientSymbCP gradient = params.GetGradient();
        EXPECT_NE (nullptr, params.GetGradient());
        EXPECT_EQ (GradientMode::Spherical, gradient->GetMode());
        EXPECT_EQ (0, gradient->GetFlags());
        EXPECT_EQ (8.0, gradient->GetAngle());
        EXPECT_EQ (1.0, gradient->GetTint());
        EXPECT_EQ (1.0, gradient->GetShift());
        EXPECT_EQ (2, gradient->GetNKeys());
        }
    }

/*---------------------------------------------------------------------------------**//**
* Test for Setting and Getting Pattern parameters.
* @bsimethod                                    Maha.Nasir      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementDisplayProperties, SetDisplayPattern)
    {
    SetupProject (L"3dMetricGeneral.idgndb", L"ElementDisplayProperties.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    auto seedModelId3 = m_defaultModelId;

    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId3);
    DgnModelPtr model3 = seedModel->Clone(DgnModel::CreateModelCode("model3"));
    ASSERT_TRUE(model3 != nullptr);
    model3->Insert();
    DgnModelId m3id = m_db->Models().QueryModelId(DgnModel::CreateModelCode("model3"));

    ElemDisplayParams ep;
    ep.SetCategoryId (m_defaultCategoryId);

    PatternParamsPtr pattern = PatternParams::Create (); 
    pattern->SetColor (ColorDef::Cyan ());
    pattern->SetWeight (6);
    pattern->SetStyle (1);
    ep.SetPatternParams (pattern.get());
    EXPECT_EQ( NULL != ep.GetPatternParams() )

    auto keyE1 = InsertElement(DgnElement::Code(), ep, m3id);
    DgnElementId E1id = keyE1->GetElementId ();
    DgnElementCP pE1 = m_db->Elements ().FindElement (E1id);

    GeometricElementP geomElem = const_cast<GeometricElementP>(pE1->ToGeometricElement ());
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld (*geomElem);
    ElementGeometryCollection collection (*geomElem);

    for (ElementGeometryPtr geom : collection)
        {
        ElemDisplayParamsCR params = collection.GetElemDisplayParams ();
        PatternParamsCP pattern = params.GetPatternParams ();
        ASSERT_NE(nullptr, pattern );
        EXPECT_EQ(ColorDef::Cyan(), pattern->GetColor());
        EXPECT_EQ (6, pattern->GetWeight ()); 
        EXPECT_EQ (1, pattern->GetStyle ());
        } 
    }

/*---------------------------------------------------------------------------------**//**
* Test for Setting and Getting Transparency.
* @bsimethod                                    Maha.Nasir      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ElementDisplayProperties, SetTransparency)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementDisplayProperties.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    auto seedModelId3 = m_defaultModelId;
    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId3);
    DgnModelPtr model3 = seedModel->Clone(DgnModel::CreateModelCode("model3"));
    ASSERT_TRUE (model3 != nullptr);
    model3->Insert();
    DgnModelId m3id = m_db->Models().QueryModelId(DgnModel::CreateModelCode("model3"));

    ElemDisplayParams ep;
    ep.SetCategoryId(m_defaultCategoryId);
    ep.SetTransparency(0.5);

    DgnElementCPtr pE1 = InsertElement(DgnElement::Code(), ep, m3id);
    EXPECT_TRUE(pE1.IsValid());

    GeometricElementP geomElem = const_cast<GeometricElementP>(pE1->ToGeometricElement());
    ElementGeometryCollection collection(*geomElem);

    for (ElementGeometryPtr geom : collection)
        {
        ElemDisplayParamsCR params = collection.GetElemDisplayParams();
        EXPECT_EQ (0.5, params.GetTransparency());
        EXPECT_EQ (0.5, params.GetNetTransparency());
        }
    }

/*---------------------------------------------------------------------------------**//**
* Test for Setting and Getting Cateogory and SubCategory Id.
* @bsimethod                                    Maha.Nasir      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ElementDisplayProperties, SetCategory)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementDisplayProperties.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    auto seedModelId3 = m_defaultModelId;
    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId3);
    DgnModelPtr model3 = seedModel->Clone(DgnModel::CreateModelCode("model3"));
    ASSERT_TRUE (model3 != nullptr);
    model3->Insert();
    DgnModelId m3id = m_db->Models().QueryModelId(DgnModel::CreateModelCode("model3"));

    ElemDisplayParams ep;
    ep.SetCategoryId(m_defaultCategoryId);

    DgnElementCPtr pE1 = InsertElement(DgnElement::Code(), ep, m3id);
    EXPECT_TRUE(pE1.IsValid());

    GeometricElementP geomElem = const_cast<GeometricElementP>(pE1->ToGeometricElement());
    ElementGeometryCollection collection(*geomElem);

    for (ElementGeometryPtr geom : collection)
        {
        ElemDisplayParamsCR params = collection.GetElemDisplayParams();
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
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementDisplayProperties.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    auto seedModelId3 = m_defaultModelId;
    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId3);
    DgnModelPtr model3 = seedModel->Clone(DgnModel::CreateModelCode("model3"));
    ASSERT_TRUE (model3 != nullptr);
    model3->Insert();
    DgnModelId m3id = m_db->Models().QueryModelId(DgnModel::CreateModelCode("model3"));

    ElemDisplayParams ep;
    ep.SetCategoryId(m_defaultCategoryId);
    ep.SetWeight(21);
    ep.SetDisplayPriority(2);

    DgnElementCPtr pE1 = InsertElement(DgnElement::Code(), ep, m3id);
    EXPECT_TRUE(pE1.IsValid());

    GeometricElementP geomElem = const_cast<GeometricElementP>(pE1->ToGeometricElement());
    ElementGeometryCollection collection(*geomElem);

    for (ElementGeometryPtr geom : collection)
        {
        ElemDisplayParamsCR params = collection.GetElemDisplayParams();
        EXPECT_EQ (21, params.GetWeight());
        bool weight = params.IsWeightFromSubCategoryAppearance();
        EXPECT_FALSE (weight);
        EXPECT_EQ (2, params.GetDisplayPriority());
        }
    }

/*---------------------------------------------------------------------------------**//**
* Test for Setting Fill properties.
* @bsimethod                                    Maha.Nasir      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ElementDisplayProperties, FillProperties)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementDisplayProperties.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    auto seedModelId3 = m_defaultModelId;
    DgnModelPtr seedModel = m_db->Models().GetModel(seedModelId3);
    DgnModelPtr model3 = seedModel->Clone(DgnModel::CreateModelCode("model3"));
    ASSERT_TRUE (model3 != nullptr);
    model3->Insert();
    DgnModelId m3id = m_db->Models().QueryModelId(DgnModel::CreateModelCode("model3"));

    ElemDisplayParams ep;
    ep.SetCategoryId(m_defaultCategoryId);
    ep.SetFillDisplay(FillDisplay::Always);
    ep.SetFillColor(ColorDef::Red());
    ep.SetFillTransparency(0.8);

    DgnElementCPtr pE1 = InsertElement(DgnElement::Code(), ep, m3id);
    EXPECT_TRUE(pE1.IsValid());

    GeometricElementP geomElem = const_cast<GeometricElementP>(pE1->ToGeometricElement());
    ElementGeometryCollection collection(*geomElem);

    for (ElementGeometryPtr geom : collection)
        {
        ElemDisplayParamsCR params = collection.GetElemDisplayParams();
        EXPECT_EQ (FillDisplay::Always, params.GetFillDisplay());
        EXPECT_EQ (ColorDef::Red(), params.GetFillColor());
        bool FillColor = params.IsFillColorFromSubCategoryAppearance();
        EXPECT_FALSE (FillColor);
        EXPECT_EQ (0.8, params.GetFillTransparency());
        EXPECT_EQ (0.8, params.GetNetFillTransparency());
        }
    }
