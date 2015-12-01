/*--------------------------------------------------------------------------------------+      
|
|  $Source: Tests/DgnProject/NonPublished/ElementMaterial_tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"
#include <numeric>
#include <DgnPlatform/DgnMaterial.h>
#include <DgnPlatform/DgnTexture.h>

USING_NAMESPACE_BENTLEY_DPTEST

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
static void setUpView(DgnDbR dgnDb, DgnModelR model, ElementAlignedBox3d elementBox, DgnCategoryId categoryId)
    {
    CameraViewDefinition view(CameraViewDefinition::CreateParams(dgnDb, "TestView", ViewDefinition::Data(model.GetModelId(), DgnViewSource::Generated)));
    EXPECT_TRUE(view.Insert().IsValid());
    EXPECT_TRUE(view.GetViewId().IsValid());

    ViewController::MarginPercent viewMargin(0.1, 0.1, 0.1, 0.1);

    PhysicalViewController viewController (dgnDb, view.GetViewId());
    viewController.SetStandardViewRotation(StandardView::Iso);
    viewController.LookAtVolume(elementBox, nullptr, &viewMargin);
    viewController.GetViewFlagsR().SetRenderMode(RenderMode::SmoothShade);
    viewController.ChangeCategoryDisplay(categoryId, true);
    viewController.ChangeModelDisplay(model.GetModelId(), true);

    EXPECT_TRUE(BE_SQLITE_OK == viewController.Save());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnMaterialId createTexturedMaterial(DgnDbR dgnDb, Utf8CP materialName, WCharCP pngFileName, JsonRenderMaterial::TextureMap::Units unitMode)
    {
    RgbFactor red = { 1.0, 0.0, 0.0};
    ByteStream fileImageData, imageData;
    uint32_t width, height;
    ImageUtilities::RgbImageInfo rgbImageInfo;
   
    JsonRenderMaterial renderMaterialAsset;
    renderMaterialAsset.SetColor(RENDER_MATERIAL_Color, red);
    renderMaterialAsset.SetBool(RENDER_MATERIAL_FlagHasBaseColor, true);

    BeFile imageFile;
    if (BeFileStatus::Success == imageFile.Open(pngFileName, BeFileAccess::Read) &&
        SUCCESS == ImageUtilities::ReadImageFromPngFile(fileImageData, rgbImageInfo, imageFile))
        {
        width = rgbImageInfo.width;
        height = rgbImageInfo.height;

        imageData.ReserveMemory(width * height * 4);
        Byte* p = imageData.GetDataP(); 
        Byte* s = fileImageData.GetDataP(); 
        for (uint32_t i=0; i<imageData.GetSize(); ++i)
            {
            *p++ = *s++;
            *p++ = *s++;
            *p++ = *s++;
            *p++ = 255;     // Alpha.
            }
        }
    else
        {
        width = height = 512;
        imageData.ReserveMemory(width * height * 4);

        size_t      value = 0;
        Byte* imageByte=imageData.GetDataP();
        for (uint32_t i=0; i<imageData.GetSize(); ++i)
            *imageByte++ = ++value % 0xff;        
        }

    DgnTexture::Data textureData(DgnTexture::Format::RAW, imageData.GetData(), imageData.GetSize(), width, height);
    DgnTexture texture(DgnTexture::CreateParams(dgnDb, materialName/*###TODO unnamed textures*/, textureData));
    texture.Insert();
    DgnTextureId textureId = texture.GetTextureId();
    EXPECT_TRUE(textureId.IsValid());

    Json::Value     patternMap, mapsMap;

    patternMap[RENDER_MATERIAL_TextureId]        = textureId.GetValue();
    patternMap[RENDER_MATERIAL_PatternScaleMode] = (int) unitMode;
    patternMap[RENDER_MATERIAL_PatternMapping]   = (int) JsonRenderMaterial::TextureMap::Mode::Parametric;

    mapsMap[RENDER_MATERIAL_MAP_Pattern] = patternMap;
    renderMaterialAsset.GetValueR()[RENDER_MATERIAL_Map] = mapsMap;

    DgnMaterial material(DgnMaterial::CreateParams(dgnDb, "Test Palette", materialName));
    material.SetRenderingAsset(renderMaterialAsset.GetValue());
    auto createdMaterial = material.Insert();
    EXPECT_TRUE(createdMaterial.IsValid());
    return createdMaterial.IsValid() ? createdMaterial->GetMaterialId() : DgnMaterialId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void appendGeometry(DPoint3dR origin, ElementGeometryBuilderR builder)
    {
    double      dz = 3.0;
    double      radius = 1.5;
    DPoint3d    localOrigin = origin;

    DgnConeDetail       cylinderDetail(localOrigin, DPoint3d::FromSumOf(localOrigin, DVec3d::From(0.0, 0.0, dz)), radius, radius, true);
    ISolidPrimitivePtr  cylinder = ISolidPrimitive::CreateDgnCone(cylinderDetail);

    EXPECT_TRUE(builder.Append(*cylinder));

    localOrigin.z +=  dz + radius + 1.0;

    DgnSphereDetail        sphereDetail(localOrigin, RotMatrix::FromIdentity(), radius);
    ISolidPrimitivePtr     sphere = ISolidPrimitive::CreateDgnSphere(sphereDetail);

    EXPECT_TRUE(builder.Append(*sphere));

    double width = 1.0;
    double height = 2.0;
    double length = 3.0;

    localOrigin.z +=  radius + 1.0;

    DgnBoxDetail        boxDetail(localOrigin, DPoint3d::FromSumOf(localOrigin, DVec3d::From(0.0, 0.0,  height)), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), width, length, width, length, true);
    ISolidPrimitivePtr  box = ISolidPrimitive::CreateDgnBox(boxDetail);

    EXPECT_TRUE(builder.Append(*box));

    origin.x += 4.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeometryBuilderTests, CreateElementWithMaterials)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElemGeometryBuilderWithMaterials.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    DgnElementPtr el = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, DgnElement::Code());

    DgnModelP model = m_db->Models().GetModel(m_defaultModelId).get();
    GeometrySourceP geomElem = el->ToGeometrySourceP();

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));

    BeFileName textureImage;
    ASSERT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(textureImage, L"TextureImage.png", L"TextureImage.png", __FILE__));

    Render::GeometryParams elemDisplayParams;
    elemDisplayParams.SetCategoryId(m_defaultCategoryId);
    elemDisplayParams.SetMaterialId(createTexturedMaterial(*m_db, "Parametric Texture", textureImage.c_str(), JsonRenderMaterial::TextureMap::Units::Relative));
    EXPECT_TRUE( builder->Append(elemDisplayParams));

    DPoint3d origin = DPoint3d::FromZero();
    appendGeometry(origin, *builder);

    elemDisplayParams.SetMaterialId(createTexturedMaterial(*m_db, "Meter Texture", textureImage.c_str() , JsonRenderMaterial::TextureMap::Units::Meters));
    EXPECT_TRUE( builder->Append(elemDisplayParams));

    appendGeometry(origin, *builder);

    EXPECT_EQ(SUCCESS, builder->SetGeomStreamAndPlacement(*geomElem));
    EXPECT_TRUE(m_db->Elements().Insert(*el).IsValid());

    Placement3d placement = builder->GetPlacement3d();

    setUpView(*m_db, *model, placement.GetElementBox(), m_defaultCategoryId);
    m_db->SaveSettings();   
    }
