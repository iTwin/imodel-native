/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DPTEST

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Element Geometry Builder
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct GeometryBuilderTests : public DgnDbTestFixture
{
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeometryBuilderTests, CreateElement3d)
    {
    SetupSeedProject();

    DgnElementPtr el = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, DgnCode());

    DgnModelP model = m_db->Models().GetModel(m_defaultModelId).get();
    GeometrySourceP geomElem = el->ToGeometrySourceP();

    GeometryBuilderPtr builder = GeometryBuilder::Create(*model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));

    //  CurvePrimitive
    //
    DEllipse3d ellipseData = DEllipse3d::From(1, 2, 3,
        0, 0, 2,
        0, 3, 0,
        0.0, Angle::TwoPi());
    ICurvePrimitivePtr ellipse = ICurvePrimitive::CreateArc(ellipseData);
    EXPECT_TRUE(builder->Append(*ellipse));

    // Curve Vector
    //
    CurveVectorPtr curveVector =  GeomHelper::computeShape();
    EXPECT_TRUE(builder->Append(*curveVector));

    // SolidPrimitive
    //
    double dz = 3.0;
    double radius = 1.5;
    DgnConeDetail cylinderDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, dz), radius, radius, true);
    ISolidPrimitivePtr cylinder = ISolidPrimitive::CreateDgnCone(cylinderDetail);
    EXPECT_TRUE(builder->Append(*cylinder));

    Render::GeometryParams elemDisplayParams;
    elemDisplayParams.SetCategoryId(m_defaultCategoryId);
    elemDisplayParams.SetWeight(2);
    EXPECT_TRUE( builder->Append(elemDisplayParams));

    // SubCategory
    //
    DgnSubCategory::Appearance appearence;
    appearence.SetInvisible(false);
    appearence.SetColor(ColorDef::DarkRed());
    Utf8CP sub_code = "Test SubCategory";
    Utf8CP sub_desc = "This is a test subcategory";
    DgnSubCategory subCategory(DgnSubCategory::CreateParams(*m_db, m_defaultCategoryId, sub_code, appearence, sub_desc));
    DgnDbStatus status;
    DgnSubCategoryCPtr newSubCategory =  subCategory.Insert(&status);
    EXPECT_TRUE(DgnDbStatus::Success == status);
    EXPECT_TRUE(newSubCategory.IsValid());
    EXPECT_TRUE(newSubCategory->GetSubCategoryId().IsValid());
    EXPECT_TRUE(builder->Append(newSubCategory->GetSubCategoryId()));

    // MSBsplineSurface
    double a = 1000.0 / 3.0;
    MSBsplineSurfacePtr surface = GeomHelper::CreateGridSurface(DPoint3d::From(0, 0, 0), a, a, 3, 4, 4);
    EXPECT_TRUE(builder->Append(*surface));

    // PolyfaceQuery
    IFacetOptionsPtr options = IFacetOptions::Create();
    IPolyfaceConstructionPtr faceBuilder = IPolyfaceConstruction::Create(*options);
    faceBuilder->AddSweptNGon(4, 1.0, 0.0, 2.0, true, true);
    PolyfaceHeaderPtr mesh = faceBuilder->GetClientMeshPtr();
    EXPECT_TRUE(builder->Append(*mesh));

    // TextString
    TextStringPtr text = GeomHelper::CreateTextString(*m_db);
    EXPECT_TRUE(builder->Append(*text));

    EXPECT_EQ(SUCCESS, builder->Finish(*geomElem));
    EXPECT_TRUE(m_db->Elements().Insert(*el).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeometryBuilderTests, RoundTripImageGraphic)
    {
    SetupSeedProject();

    DgnElementPtr el = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, DgnCode());
    DgnModelP model = m_db->Models().GetModel(m_defaultModelId).get();
    GeometrySourceP geomElem = el->ToGeometrySourceP();

    auto builder = GeometryBuilder::Create(*model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));

    // Create an element with an ImageGraphic in its geometry stream.
    DgnTextureId textureId((uint64_t)0x1234);
    ImageGraphic::Corners corners =
        {
            {
            DPoint3d::From(-5, 10, 3),
            DPoint3d::From(20, 10, 3),
            DPoint3d::From(20, 15, 3),
            DPoint3d::From(-5, 15, 3),
            }
        };

    auto input = ImageGraphic::Create(corners, textureId, true);
    EXPECT_TRUE(builder->Append(*input));

    EXPECT_EQ(SUCCESS, builder->Finish(*geomElem));
    auto persistentElem = m_db->Elements().Insert(*el);
    EXPECT_TRUE(persistentElem.IsValid());

    // Pull the ImageGraphic back out of the new persistent element's geometry stream.
    ImageGraphicPtr output;
    GeometryCollection geom(*persistentElem->ToGeometrySource());
    for (auto const& entry : geom)
        {
        EXPECT_TRUE(output.IsNull());
        EXPECT_TRUE(entry.GetEntryType() == GeometryCollection::Iterator::EntryType::Image);
        auto primitive = entry.GetGeometryPtr();
        EXPECT_TRUE(primitive.IsValid());
        output = primitive->GetAsImage();
        EXPECT_TRUE(output.IsValid());
        }

    EXPECT_TRUE(output.IsValid());

    auto compare = [&]()
        {
        EXPECT_EQ(output->GetTextureId().GetValue(), input->GetTextureId().GetValue());
        EXPECT_TRUE(output->HasBorder());

        auto const& outPts = output->GetCorners().m_pts;
        auto const& inPts = corners.m_pts;
        for (auto i = 0; i < 4; i++)
            {
            EXPECT_EQ(outPts[i].x, inPts[i].x);
            EXPECT_EQ(outPts[i].y, inPts[i].y);
            EXPECT_EQ(outPts[i].z, inPts[i].z);
            }
        };

    compare();

    // Test JSON representation.
    BeJsDocument geomJson;
    geom.ToJson(geomJson);
    EXPECT_TRUE(geomJson.isArray());
    EXPECT_EQ(2, geomJson.size());

    auto const& outputJson = geomJson[1]["image"];
    EXPECT_TRUE(outputJson.isObject());
    EXPECT_FALSE(outputJson.isNull());

    BeJsDocument inputJson;
    input->ToJson(inputJson);
    EXPECT_TRUE(outputJson.isExactEqual(inputJson));

    output = ImageGraphic::FromJson(outputJson);
    compare();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeometryBuilderTests, CreateElement2d)
    {
    SetupSeedProject();

    DgnCategoryId categoryId = DgnDbTestUtils::InsertDrawingCategory(*m_db, "MyDrawingCategory");
    DocumentListModelPtr sheetListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "SheetListModel");
    auto sheet = DgnDbTestUtils::InsertSheet(*sheetListModel, 1,1,1, "TestSheet");
    auto sheetModel = DgnDbTestUtils::InsertSheetModel(*sheet);
    DgnModelId sheetModelId = sheetModel->GetModelId();

    DgnElementPtr el = TestElement2d::Create(*m_db, sheetModelId, categoryId, DgnCode(), 100);

    GeometrySourceP geomElem = el->ToGeometrySourceP();

    GeometryBuilderPtr builder = GeometryBuilder::Create(*sheetModel, categoryId, DPoint2d::From(0.0, 0.0));
    TextString textStringElem(*m_db);
    textStringElem.SetText("If we have no text we have no range and insert fails");
    EXPECT_TRUE(builder->Append(textStringElem));

    // 3d should not be appended
    double dz = 3.0;
    double radius = 1.5;
    DgnConeDetail cylinderDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, dz), radius, radius, true);
    ISolidPrimitivePtr cylinder = ISolidPrimitive::CreateDgnCone(cylinderDetail);
    BeTest::SetFailOnAssert(false);
    EXPECT_FALSE(builder->Append(*cylinder));
    BeTest::SetFailOnAssert(true);

    EXPECT_EQ(SUCCESS, builder->Finish(*geomElem));
    EXPECT_TRUE(m_db->Elements().Insert(*el).IsValid());
    }
