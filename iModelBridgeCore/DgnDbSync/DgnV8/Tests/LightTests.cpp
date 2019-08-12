/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"
#include <VersionedDgnV8Api/DgnPlatform/LxoProcedure.h>
//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            01/2018
//---------------+---------------+---------------+---------------+---------------+-------
struct LightTests : public ConverterTestBaseFixture
    {
    DEFINE_T_SUPER(ConverterTestBaseFixture);
    void SetUp();

    void CreateLightWithInstance(DgnV8Api::LightElementPtr light);
    void CreateLight(DgnV8Api::LightElementPtr light);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2018
//---------------+---------------+---------------+---------------+---------------+-------
void LightTests::SetUp()
    {
    T_Super::SetUp();
    //if (false == DgnV8Api::ConfigurationManager::IsVariableDefined(L"_USTN_DGNLIBLIST_SYSTEM"))
        {
        BentleyApi::BeFileName inFile = GetInputFileName(L"SystemCells.dgnlib");
        WString directory = inFile.GetDirectoryName();
        DgnV8Api::ConfigurationManager::DefineVariable(L"_USTN_DGNLIBLIST_SYSTEM", directory.c_str(), DgnV8Api::ConfigurationVariableLevel::User);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2019
//---------------+---------------+---------------+---------------+---------------+-------
void LightTests::CreateLightWithInstance(DgnV8Api::LightElementPtr light)
    {
    DgnV8Api::ElementId eidWithInst;

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    if (true)
        {
        // Create Light
        light->SetModelRef(v8editor.m_defaultModel);
        light->Save();
        DgnV8Api::ElementHandle eh(light->GetElementRef());
        eidWithInst = eh.GetElementId();
        Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" nameSpacePrefix="test" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Foo" isDomainClass="True">
                <ECProperty propertyName="Goo" typeName="string" />
            </ECClass>
        </ECSchema>)xml";

        ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
        ECObjectsV8::ECSchemaPtr schema;
        EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));
        EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));

        DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
        EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"Foo"));
        Bentley::ECN::ECValue v;
        v.SetUtf8CP("PointLightHandler");
        createdDgnECInstance->SetValue(L"Goo", v);
        createdDgnECInstance->WriteChanges();
        v8editor.Save();
        }

    DoConvert(m_dgnDbFileName, m_v8FileName);

    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        SyncInfoReader syncInfo(m_params, db);
        RepositoryLinkId editV8FileSyncInfoId;
        syncInfo.MustFindFileByName(editV8FileSyncInfoId, m_v8FileName);
        DgnModelId editModelId;
        syncInfo.MustFindModelByV8ModelId(editModelId, editV8FileSyncInfoId, v8editor.m_defaultModel->GetModelId());
        DgnElementId dgnDbElementId;
        syncInfo.MustFindElementByV8ElementId(dgnDbElementId, editModelId, eidWithInst);

        auto dgnDbElement = db->Elements().GetElement(dgnDbElementId);
        ASSERT_TRUE(dgnDbElement.IsValid());

        Utf8String selEcSql;
        selEcSql.append("SELECT [Goo] FROM TestSchema.FooElementAspect WHERE [Element].[Id]=?");
        EC::ECSqlStatement stmt;
        stmt.Prepare(*db, selEcSql.c_str());
        stmt.BindId(1, dgnDbElementId);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_TRUE(0 == strcmp("PointLightHandler", stmt.GetValueText(0)));
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2019
//---------------+---------------+---------------+---------------+---------------+-------
void LightTests::CreateLight(DgnV8Api::LightElementPtr light)
    {

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    if (true)
        {
        // Create Light
        light->SetModelRef(v8editor.m_defaultModel);
        light->Save();
        v8editor.Save();
        }

    DoConvert(m_dgnDbFileName, m_v8FileName);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     04/10
//---------------------------------------------------------------------------------------
static DgnV8Api::LightElementPtr createPointLightElement(bool setTypeOnly)
    {
    DgnV8Api::PointLightPtr result = DgnV8Api::PointLight::Create();
    if (setTypeOnly)
        return result.get();

    result->SetIntensity(1.0);
    result->GetColorR().red = result->GetColorR().green = result->GetColorR().blue = 1.0;
    result->SetIsEnabled(true);
    result->SetBrightness(2650);

    result->SetUseVolumetrics(false);
    result->SetVolumeSampleCount(40);
    result->SetVolumeScattering(50.0);
    result->SetVolumeDensity(50.0);
    result->SetVolumeAttenuation(10.0);
    result->SetVolumeShift(0.0);
    result->SetVolumeHeightInMeters(2.0);
    result->SetVolumeRadiusBaseInMeters(1.0);
    result->GetScatterColorR().red = result->GetScatterColorR().green = result->GetScatterColorR().blue = 0.5;

    result->SetShadowType(DgnV8Api::AdvancedLight::SHADOWTYPE_RayTrace);
    result->SetDeepShadowSamples(1024);
    result->SetVolumeAffectDiffuse(100.0);
    result->SetVolumeAffectSpecular(100.0);
    result->SetVolumeAffectCaustics(100.0);
    result->SetCastsShadows(true);
    result->SetShadowQuality(DgnV8Api::LightElement::SHADOWQUALITY_Sharp);
    result->GetShadowColorR().red = result->GetShadowColorR().green = result->GetShadowColorR().blue = 0.0;

    result->SetName(L"Point Light (1)");
    result->SetPresetName(L"[Select]");
    result->SetUsesIesData(false);
    result->SetIesFileName(L"");
    result->GetIesReferenceR().Init(0.0, 0.0, 1.0);
    result->SetIesRotation(0.0);

    result->SetBulbSizeInUors(0.0);
    result->SetBulbCount(1);
    result->SetTemperatureInKelvin(0);

    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_Color)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_VolumetricDensity)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_VolumetricScatterColor)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_DiffuseAmount)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_SpecularAmount)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_ShadowColor)->SetIsEnabled(false);

    return result.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     04/10
//---------------------------------------------------------------------------------------
static DgnV8Api::LightElementPtr createDistantLightElement(bool setTypeOnly)
    {
    DgnV8Api::DistantLightPtr result = DgnV8Api::DistantLight::Create();
    if (setTypeOnly)
        return result.get();

    result->SetIntensity(4.65);
    result->GetColorR().red = 1.0;
    result->GetColorR().green = 0.647;
    result->GetColorR().blue = 0.31;
    result->SetIsEnabled(true);
    result->SetBrightness(2650);

    result->SetUseVolumetrics(false);
    result->SetVolumeSampleCount(40);
    result->SetVolumeScattering(50.0);
    result->SetVolumeDensity(50.0);
    result->SetVolumeAttenuation(10.0);
    result->SetVolumeShift(0.0);
    result->SetVolumeHeightInMeters(2.0);
    result->SetVolumeRadiusBaseInMeters(1.0);
    result->GetScatterColorR().red = result->GetScatterColorR().green = result->GetScatterColorR().blue = 0.5;

    result->SetShadowType(DgnV8Api::AdvancedLight::SHADOWTYPE_RayTrace);
    result->SetDeepShadowSamples(1024);
    result->SetVolumeAffectDiffuse(100.0);
    result->SetVolumeAffectSpecular(100.0);
    result->SetVolumeAffectCaustics(100.0);
    result->SetCastsShadows(false);
    result->SetShadowQuality(DgnV8Api::LightElement::SHADOWQUALITY_SoftMedium);
    result->GetShadowColorR().red = 176.0 / 255.0;
    result->GetShadowColorR().green = 176.0 / 255.0;
    result->GetShadowColorR().blue = 176.0 / 255.0;

    result->SetName(L"Directional Light (1)");
    result->SetPresetName(L"[Select]");
    result->SetUsesIesData(false);
    result->SetIesFileName(L"");
    result->GetIesReferenceR().Init(0.0, -1.0, 0.0);
    result->SetIesRotation(0.0);

    result->SetTemperatureInKelvin(2600);

    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_Color)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_DiffuseAmount)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_SpecularAmount)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_ShadowColor)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_VolumetricDensity)->SetIsEnabled(false);

    DgnV8Api::LightMap* map = result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_VolumetricScatterColor);
    map->SetIsEnabled(true);
    map->SetStyle(DgnV8Api::LightMap::MAPSTYLE_Image);
    map->SetUnits(DgnV8Api::MapUnits::Meters);
    map->GetSizeR().Init(3.0, 2.0, 1.0);
    map->GetOffsetR().Init(5.0, 7.0, 0.0);
    map->SetAngleInRadians(25.0 * msGeomConst_radiansPerDegree);
    map->SetHorizontalWrap(DgnV8Api::LightMap::WRAPMODE_Mirror);
    map->SetVerticalWrap(DgnV8Api::LightMap::WRAPMODE_Edge);
    map->SetMode(DgnV8Api::MapMode::Spherical);
    map->SetFileName(L"block08.jpg");

    return result.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     04/10
//---------------------------------------------------------------------------------------
static DgnV8Api::LightElementPtr createSpotLightElement(bool setTypeOnly)
    {
    DgnV8Api::SpotLightPtr result = DgnV8Api::SpotLight::Create();
    if (setTypeOnly)
        return result.get();

    result->SetIntensity(1.0);
    result->GetColorR().red = result->GetColorR().green = result->GetColorR().blue = 1.0;
    result->SetIsEnabled(true);
    result->SetBrightness(15);

    result->SetUseVolumetrics(true);
    result->SetVolumeSampleCount(40);
    result->SetVolumeScattering(50.0);
    result->SetVolumeDensity(50.0);
    result->SetVolumeAttenuation(10.0);
    result->SetVolumeShift(0.0);
    result->SetVolumeHeightInMeters(2.0);
    result->SetVolumeRadiusBaseInMeters(0.0);
    result->GetScatterColorR().red = result->GetScatterColorR().green = result->GetScatterColorR().blue = 0.5;

    result->SetShadowType(DgnV8Api::AdvancedLight::SHADOWTYPE_RayTrace);
    result->SetVolumeAffectDiffuse(62.318841);
    result->SetVolumeAffectSpecular(85.507246);
    result->SetVolumeAffectCaustics(49.275362);
    result->SetCastsShadows(true);
    result->SetShadowQuality(DgnV8Api::LightElement::SHADOWQUALITY_Sharp);
    result->GetShadowColorR().red = result->GetShadowColorR().green = result->GetShadowColorR().blue = 0.0;

    result->SetName(L"Caetano Veloso");
    result->SetPresetName(L"[Select]");
    result->SetUsesIesData(false);
    result->SetIesFileName(L"");
    result->GetIesReferenceR().Init(0.0, 0.0, 1.0);
    result->SetIesRotation(0.0);

    result->SetTemperatureInKelvin(0);
    result->SetDeltaAngleInRadians(5.0 * msGeomConst_radiansPerDegree);
    result->SetOuterAngleInRadians(30.0 * msGeomConst_radiansPerDegree);
    result->SetBulbCount(1);
    result->SetBulbSizeInUors(0.0);

    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_VolumetricDensity)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_SpecularAmount)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_DiffuseAmount)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_ShadowColor)->SetIsEnabled(false);

    LightMapP map = result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_Color);
    map->SetIsEnabled(true);
    map->SetStyle(DgnV8Api::LightMap::MAPSTYLE_Procedure);
    map->SetUnits(DgnV8Api::MapUnits::Meters);
    map->GetSizeR().Init(1.0, 1.0, 1.0);
    map->GetOffsetR().Init(0.0, 0.0, 0.0);
    map->SetAngleInRadians(0.0);
    map->SetHorizontalWrap(DgnV8Api::LightMap::WRAPMODE_Mirror);
    map->SetVerticalWrap(DgnV8Api::LightMap::WRAPMODE_Repeat);
    map->SetMode(DgnV8Api::MapMode::Spherical);

    LxoConstantProcedureP constant = reinterpret_cast <LxoConstantProcedureP> (map->AddLxoProcedure(DgnV8Api::LxoProcedure::PROCEDURETYPE_Constant));
    constant->GetColorR().red = 159.0f / 255.0f;
    constant->GetColorR().green = 57.0f / 255.0f;
    constant->GetColorR().blue = 121.0f / 255.0f;

    map = result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_VolumetricScatterColor);
    map->SetIsEnabled(true);
    map->SetStyle(DgnV8Api::LightMap::MAPSTYLE_Procedure);
    map->SetUnits(DgnV8Api::MapUnits::Relative);
    map->GetSizeR().Init(1.0, 1.0, 1.0);
    map->GetOffsetR().Init(0.0, 0.0, 0.0);
    map->SetAngleInRadians(0.0);
    map->SetHorizontalWrap(DgnV8Api::LightMap::WRAPMODE_Repeat);
    map->SetVerticalWrap(DgnV8Api::LightMap::WRAPMODE_Repeat);
    map->SetMode(DgnV8Api::MapMode::FrontProject);

    LxoDotProcedureP dot = reinterpret_cast <LxoDotProcedureP> (map->AddLxoProcedure(DgnV8Api::LxoProcedure::PROCEDURETYPE_Dot));
    dot->GetDotColorR().red = 79.0f / 255.0f;
    dot->GetDotColorR().green = 184.0f / 255.0f;
    dot->GetDotColorR().blue = 159.0f / 255.0f;
    dot->GetFillerColorR().red = 13.0f / 255.0f;
    dot->GetFillerColorR().green = 18.0f / 255.0f;
    dot->GetFillerColorR().blue = 82.0f / 255.0f;
    dot->SetDotType(DgnV8Api::LxoDotProcedure::DOTTYPE_Hexagon);
    dot->SetDotAlpha(47.169811);
    dot->SetFillerAlpha(7.54717);
    dot->SetTransitionWidth(39.9);
    dot->SetDotWidth(24.2);
    dot->SetBias(69.811321);
    dot->SetGain(26.415094);

    return result.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     04/10
//---------------------------------------------------------------------------------------
static DgnV8Api::LightElementPtr createAreaLightElement(bool setTypeOnly)
    {
    DgnV8Api::AreaLightPtr result = DgnV8Api::AreaLight::Create();
    if (setTypeOnly)
        return result.get();

    result->SetIntensity(0.9);
    result->GetColorR().red = 0.38;
    result->GetColorR().green = 0.592;
    result->GetColorR().blue = 0.788;
    result->SetIsEnabled(true);
    result->SetBrightness(5000);

    result->SetShadowType(DgnV8Api::AdvancedLight::SHADOWTYPE_RayTrace);
    result->SetDeepShadowSamples(1024);
    result->SetVolumeAffectDiffuse(62.318841);
    result->SetVolumeAffectSpecular(97.101449);
    result->SetVolumeAffectCaustics(56.521739);
    result->SetCastsShadows(true);
    result->SetShadowQuality(DgnV8Api::LightElement::SHADOWQUALITY_SoftVeryFine);
    result->GetShadowColorR().red = 37.0 / 255.0;
    result->GetShadowColorR().green = 37.0 / 255.0;
    result->GetShadowColorR().blue = 204.0 / 255.0;

    result->SetName(L"Area Light");
    result->SetPresetName(L"halogen 250W Bulb");
    result->SetUsesIesData(true);
    result->SetIesFileName(L"sample.ies");
    result->GetIesReferenceR().Init(1.0, 0.0, 0.0);
    result->SetIesRotation(1.0);

    result->SetBulbCount(3);
    result->SetTemperatureInKelvin(0);

    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_Color)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_VolumetricDensity)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_DiffuseAmount)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_SpecularAmount)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_VolumetricScatterColor)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_ShadowColor)->SetIsEnabled(false);
    result->GetMapsR().AddMap(DgnV8Api::LightMap::MAPTYPE_VolumetricDensity)->SetIsEnabled(false);

    return result.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
static DgnV8Api::LightElementPtr createSkyOpeningLightElement(bool setTypeOnly)
    {
    DgnV8Api::SkyOpeningLightPtr result = DgnV8Api::SkyOpeningLight::Create();
    if (setTypeOnly)
        return result.get();

    result->SetName(L"Sky Opening");
    result->SetMinimumSamples(16);
    result->SetMaximumSamples(64);
    result->GetIesReferenceR().Init(-1.0, 0.0, 0.0);

    return result.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LightTests, LightSetup)
    {
    LineUpFiles(L"ProjectProperties.bim", L"Test3d.dgn", false);

    if (true)
        {
        V8FileEditor editor;
        editor.Open(m_v8FileName);
        // Create Light
        DgnV8Api::LightElementPtr light = createPointLightElement(false);
        light->SetModelRef(editor.m_defaultModel);
        light->Save();

        Bentley::LightManagerR lightManager = DgnV8Api::LightManager::GetManagerR();
        Bentley::LightSetupCP activeSetup = lightManager.GetActiveLightSetupForModel(true, *editor.m_defaultModel);
        EXPECT_TRUE(nullptr != activeSetup);
        }
    DoConvert(m_dgnDbFileName, m_v8FileName);

    // Update light 
    // Do update
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(LightTests, CreatePointLightWithECInstance)
    {
    LineUpFiles(L"PointLight.bim", L"Test3d.dgn", false);
    DgnV8Api::LightElementPtr light = createPointLightElement(false);

    CreateLightWithInstance(light);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2019
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(LightTests, CreateDistantLightWithECInstance)
    {
    LineUpFiles(L"DistantLight.bim", L"Test3d.dgn", false);
    DgnV8Api::LightElementPtr light = createDistantLightElement(false);

    CreateLightWithInstance(light);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2019
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(LightTests, CreateSpotLightWithECInstance)
    {
    LineUpFiles(L"SpotLight.bim", L"Test3d.dgn", false);
    DgnV8Api::LightElementPtr light = createSpotLightElement(false);

    CreateLightWithInstance(light);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2019
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(LightTests, CreateAreaLightWithECInstance)
    {
    LineUpFiles(L"AreaLight.bim", L"Test3d.dgn", false);
    DgnV8Api::LightElementPtr light = createAreaLightElement(false);

    CreateLight(light);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2019
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(LightTests, CreateSkyOpeningLightWithECInstance)
    {
    LineUpFiles(L"SkyOpeningLight.bim", L"Test3d.dgn", false);
    DgnV8Api::LightElementPtr light = createSkyOpeningLightElement(false);

    CreateLight(light);
    }
