/*--------------------------------------------------------------------------------------+       22
|
|  $Source: Tests/DgnProject/NonPublished/ElementMaterial_tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"
#include <numeric>

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Element Geometry Builder
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementGeometryBuilderTests : public DgnDbTestFixture
{

};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void setUpView (DgnDbR dgnDb, DgnModelR model, ElementAlignedBox3d elementBox, DgnCategoryId categoryId)
    {
    DgnViews::View view;

    view.SetDgnViewType(DgnClassId(dgnDb.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalView)), DgnViewType::Physical);
    view.SetDgnViewSource(DgnViewSource::Generated);
    view.SetName("TestView");
    view.SetBaseModelId(model.GetModelId());

    EXPECT_TRUE(BE_SQLITE_OK == dgnDb.Views().Insert(view));
    EXPECT_TRUE(view.GetId().IsValid());

    ViewController::MarginPercent viewMargin(0.1, 0.1, 0.1, 0.1);

    PhysicalViewController viewController (dgnDb, view.GetId());
    viewController.SetStandardViewRotation(StandardView::Iso);
    viewController.LookAtVolume(elementBox, nullptr, &viewMargin);
    viewController.GetViewFlagsR().SetRenderMode(DgnRenderMode::SmoothShade);
    viewController.ChangeCategoryDisplay(categoryId, true);
    viewController.ChangeModelDisplay(model.GetModelId(), true);

    EXPECT_TRUE(BE_SQLITE_OK == viewController.Save());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnMaterialId     createTexturedMaterial (DgnDbR dgnDb, Utf8CP materialName, WCharCP pngFileName, RenderMaterial::MapUnits unitMode)
    {
    Json::Value                     renderMaterialAsset;
    RgbFactor                       red = { 1.0, 0.0, 0.0};
    bvector <Byte>                  fileImageData, imageData;
    uint32_t                        width, height;
    ImageUtilities::RgbImageInfo    rgbImageInfo;
    BeFile                          imageFile;

    
    RenderMaterial::SetColor (renderMaterialAsset, RENDER_MATERIAL_Color, red);
    renderMaterialAsset[RENDER_MATERIAL_FlagHasBaseColor] = true;
    

    if (BeFileStatus::Success == imageFile.Open (pngFileName, BeFileAccess::Read) &&
        SUCCESS == ImageUtilities::ReadImageFromPngFile (fileImageData, rgbImageInfo, imageFile))
        {
        width = rgbImageInfo.width;
        height = rgbImageInfo.height;

        imageData.resize (width * height * 4);

        for (size_t i=0, j=0; i<imageData.size(); )
            {
            imageData[i++] = fileImageData[j++];
            imageData[i++] = fileImageData[j++];
            imageData[i++] = fileImageData[j++];
            imageData[i++] = 255;     // Alpha.
            }
        }
    else
        {
        width = height = 512;
        imageData.resize (width * height * 4);

        size_t      value = 0;
        for (auto& imageByte : imageData)
            imageByte = ++value % 0xff;        
        }


    DgnTextures::Texture    texture (DgnTextures::TextureData (DgnTextures::Format::RAW, &imageData.front(), imageData.size(), width, height));
    DgnTextureId            textureId = dgnDb.Textures().Insert (texture);

    Json::Value     patternMap, mapsMap;

    patternMap[RENDER_MATERIAL_TextureId]        = textureId.GetValue();
    patternMap[RENDER_MATERIAL_PatternScaleMode] = (int) unitMode;
    patternMap[RENDER_MATERIAL_PatternMapping]   = (int) RenderMaterial::MapMode::Parametric;

    mapsMap[RENDER_MATERIAL_MAP_Pattern] = patternMap;
    renderMaterialAsset[RENDER_MATERIAL_Map] = mapsMap;


    DgnMaterials::Material material (materialName, "Test Palette");

    material.SetRenderingAsset (renderMaterialAsset);

    return dgnDb.Materials().Insert(material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void appendGeometry (DPoint3dR origin, ElementGeometryBuilderR builder)
    {
    double      dz = 3.0;
    double      radius = 1.5;
    DPoint3d    localOrigin = origin;

    DgnConeDetail       cylinderDetail(localOrigin, DPoint3d::FromSumOf (localOrigin, DVec3d::From (0.0, 0.0, dz)), radius, radius, true);
    ISolidPrimitivePtr  cylinder = ISolidPrimitive::CreateDgnCone(cylinderDetail);

    EXPECT_TRUE(builder.Append(*cylinder));

    localOrigin.z +=  dz + radius + 1.0;

    DgnSphereDetail        sphereDetail(localOrigin, RotMatrix::FromIdentity(), radius);
    ISolidPrimitivePtr     sphere = ISolidPrimitive::CreateDgnSphere(sphereDetail);

    EXPECT_TRUE(builder.Append(*sphere));

    double              width   = 1.0;
    double              height  = 2.0;
    double              length  = 3.0;

    localOrigin.z +=  radius + 1.0;

    DgnBoxDetail        boxDetail (localOrigin, DPoint3d::FromSumOf (localOrigin, DVec3d::From (0.0, 0.0,  height)), DVec3d::From (1.0, 0.0, 0.0), DVec3d::From (0.0, 1.0, 0.0), width, length, width, length, true);
    ISolidPrimitivePtr  box = ISolidPrimitive::CreateDgnBox (boxDetail);

    EXPECT_TRUE(builder.Append(*box));

    origin.x += 4.0;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeometryBuilderTests, CreateElementWithMaterials)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElemGeometryBuilderWithMaterials.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    DgnElementPtr el = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, DgnElement::Code("Test1"));

    DgnModelP model = m_db->Models().GetModel(m_defaultModelId).get();
    GeometricElementP geomElem = const_cast<GeometricElementP>(el->ToGeometricElement());

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));

    BeFileName textureImage;
    ASSERT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(textureImage, L"TextureImage.png", L"TextureImage.png", __FILE__));

    ElemDisplayParams elemDisplayParams;
    elemDisplayParams.SetCategoryId(m_defaultCategoryId);
    elemDisplayParams.SetMaterial(createTexturedMaterial(*m_db, "Parametric Texture", textureImage.c_str(), RenderMaterial::MapUnits::Relative));
    EXPECT_TRUE( builder->Append(elemDisplayParams));

    DPoint3d        origin = DPoint3d::FromZero();

    appendGeometry (origin, *builder);

    elemDisplayParams.SetMaterial(createTexturedMaterial(*m_db, "Meter Texture", textureImage.c_str() , RenderMaterial::MapUnits::Meters));
    EXPECT_TRUE( builder->Append(elemDisplayParams));

    appendGeometry (origin, *builder);


    EXPECT_EQ(SUCCESS, builder->SetGeomStreamAndPlacement(*geomElem));
    EXPECT_TRUE(m_db->Elements().Insert(*el).IsValid());

    Placement3d        placement;

    builder->GetPlacement (placement);

    setUpView (*m_db, *model, placement.GetElementBox(), m_defaultCategoryId);
    m_db->SaveSettings();   
    }

