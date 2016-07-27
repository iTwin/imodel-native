/*--------------------------------------------------------------------------------------+      
|
|  $Source: Tests/DgnProject/NonPublished/ElementMaterial_tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"
#include <numeric>
#include <DgnPlatform/DgnMaterial.h>
#include <DgnPlatform/DgnTexture.h>

USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_RENDER

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Element Geometry Builder
* @bsimethod                                                    Umar.Hayat      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct GeometryBuilderTests : public DgnDbTestFixture
{

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnMaterialId createTexturedMaterial(DgnDbR dgnDb, Utf8CP materialName, WCharCP pngFileName, JsonRenderMaterial::TextureMap::Units unitMode)
    {
    RgbFactor red = { 1.0, 0.0, 0.0};
    uint32_t width, height;
   
    JsonRenderMaterial renderMaterialAsset;
    renderMaterialAsset.SetColor(RENDER_MATERIAL_Color, red);
    renderMaterialAsset.SetBool(RENDER_MATERIAL_FlagHasBaseColor, true);

    Image image;
    ImageSource imageSource;
    BeFile imageFile;
    if (BeFileStatus::Success == imageFile.Open(pngFileName, BeFileAccess::Read))
        {
        ByteStream pngBytes;
        imageFile.ReadEntireFile(pngBytes);
        imageSource = ImageSource(ImageSource::Format::Png, std::move(pngBytes));
        image = Image(imageSource);
        }
    else
        {
        width = height = 512;
        ByteStream data(width * height * 3);

        size_t      value = 0;
        Byte* imageByte=data.GetDataP();
        for (uint32_t i=0; i<data.GetSize(); ++i)
            *imageByte++ = ++value % 0xff;        

        image = Image(width, height, std::move(data), Image::Format::Rgb);
        imageSource = ImageSource(image, ImageSource::Format::Png);
        }
    EXPECT_TRUE(imageSource.IsValid());
    EXPECT_TRUE(image.IsValid());

    DgnTexture texture(DgnTexture::CreateParams(dgnDb, materialName/*###TODO unnamed textures*/, imageSource, image.GetWidth(), image.GetHeight()));
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
static void appendGeometry(DPoint3dR origin, GeometryBuilderR builder)
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
TEST_F(GeometryBuilderTests, CreateElementWithMaterials)
    {
    SetupSeedProject();

    DgnElementPtr el = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, DgnCode());

    DgnModelP model = m_db->Models().GetModel(m_defaultModelId).get();
    GeometrySourceP geomElem = el->ToGeometrySourceP();

    GeometryBuilderPtr builder = GeometryBuilder::Create(*model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));

    BeFileName textureImage;
    ASSERT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(textureImage, L"TextureImage.png", L"GeometryBuilderTests\\TextureImage.png", __FILE__));

    Render::GeometryParams elemDisplayParams;
    elemDisplayParams.SetCategoryId(m_defaultCategoryId);
    elemDisplayParams.SetMaterialId(createTexturedMaterial(*m_db, "Parametric Texture", textureImage.c_str(), JsonRenderMaterial::TextureMap::Units::Relative));
    EXPECT_TRUE( builder->Append(elemDisplayParams));

    DPoint3d origin = DPoint3d::FromZero();
    appendGeometry(origin, *builder);

    elemDisplayParams.SetMaterialId(createTexturedMaterial(*m_db, "Meter Texture", textureImage.c_str() , JsonRenderMaterial::TextureMap::Units::Meters));
    EXPECT_TRUE( builder->Append(elemDisplayParams));

    appendGeometry(origin, *builder);

    EXPECT_EQ(SUCCESS, builder->Finish(*geomElem));
    EXPECT_TRUE(m_db->Elements().Insert(*el).IsValid());

    Placement3d placement = builder->GetPlacement3d();

    SetUpSpatialView(*m_db, *model, placement.GetElementBox(), m_defaultCategoryId);
    m_db->SaveSettings();   
    }
