/*----------------------------------------------------------------------+
|
|   $Source: DgnCore/XmlMaterials.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatformInternal/DgnCore/MaterialTokens.h>

// static Utf8CP INTERNALMATERIALTAGS_Material                  = "Material";
// static Utf8CP INTERNALMATERIALTAGS_Layer                     = "Layer";
// static Utf8CP INTERNALMATERIALTAGS_UserData                  = "UserData";
static Utf8CP INTERNALMATERIALTAGS_Flags                     = "Flags";
static Utf8CP INTERNALMATERIALTAGS_BumpFlags                 = "BumpFlags";
static Utf8CP INTERNALMATERIALTAGS_PatternFlags              = "PatternFlags";
static Utf8CP INTERNALMATERIALTAGS_SpecularFlags             = "SpecularFlags";
static Utf8CP INTERNALMATERIALTAGS_ReflectFlags              = "ReflectFlags";
static Utf8CP INTERNALMATERIALTAGS_TransparencyFlags         = "TransparencyFlags";
static Utf8CP INTERNALMATERIALTAGS_TranslucencyFlags         = "TranslucencyFlags";
static Utf8CP INTERNALMATERIALTAGS_FinishFlags               = "FinishFlags";
static Utf8CP INTERNALMATERIALTAGS_DiffuseFlags              = "DiffuseFlags";
static Utf8CP INTERNALMATERIALTAGS_GlowAmountFlags           = "GlowAmountFlags";
static Utf8CP INTERNALMATERIALTAGS_ClearcoatAmountFlags      = "ClearcoatAmountFlags";
static Utf8CP INTERNALMATERIALTAGS_AnisotropicDirectionFlags = "AnisotropicDirectionFlags";
static Utf8CP INTERNALMATERIALTAGS_DisplacementFlags         = "DisplacementFlags";
static Utf8CP INTERNALMATERIALTAGS_NormalFlags               = "NormalFlags";
static Utf8CP INTERNALMATERIALTAGS_FurLengthFlags            = "FurLengthFlags";
static Utf8CP INTERNALMATERIALTAGS_FurDensityFlags           = "FurDensityFlags";
static Utf8CP INTERNALMATERIALTAGS_FurJitterFlags            = "FurJitterFlags";
static Utf8CP INTERNALMATERIALTAGS_FurFlexFlags              = "FurFlexFlags";
static Utf8CP INTERNALMATERIALTAGS_FurClumpsFlags            = "FurClumpsFlags";
static Utf8CP INTERNALMATERIALTAGS_FurDirectionFlags         = "FurDirectionFlags";
static Utf8CP INTERNALMATERIALTAGS_FurVectorFlags            = "FurVectorFlags";
static Utf8CP INTERNALMATERIALTAGS_FurBumpFlags              = "FurBumpFlags";
static Utf8CP INTERNALMATERIALTAGS_FurCurlsFlags             = "FurCurlsFlags";
static Utf8CP INTERNALMATERIALTAGS_RefractionRoughnessFlags  = "RefractionRoughnessFlags";
static Utf8CP INTERNALMATERIALTAGS_SpecularFresnelFlags      = "SpecularFresnelFlags";
static Utf8CP INTERNALMATERIALTAGS_FileName                  = "Filename";
static Utf8CP INTERNALMATERIALTAGS_Type                      = "Type";
static Utf8CP INTERNALMATERIALTAGS_Map                       = "Map";
static Utf8CP INTERNALMATERIALTAGS_Shader                    = "Shader";
static Utf8CP INTERNALMATERIALTAGS_Fur                       = "Fur";
static Utf8CP INTERNALMATERIALTAGS_FurMaterialName           = "FurMaterialName";
static Utf8CP INTERNALMATERIALTAGS_FurPaletteName            = "FurPaletteName";
static Utf8CP INTERNALMATERIALTAGS_LxoProcedure              = "LxoProcedure";
static Utf8CP INTERNALMATERIALTAGS_LxoProcedureType          = "LxoProcedureType";
static Utf8CP INTERNALMATERIALTAGS_LxoEnvelope               = "LxoEnvelope";
// static Utf8CP INTERNALMATERIALTAGS_LxoEnvelopeData           = "LxoEnvelopeData";
static Utf8CP INTERNALMATERIALTAGS_LayerType                 = "LayerType";
static Utf8CP INTERNALMATERIALTAGS_LayerFlags                = "LayerFlags";
static Utf8CP INTERNALMATERIALTAGS_LayerDataFlags            = "LayerDataFlags";
static Utf8CP INTERNALMATERIALTAGS_Color1                    = "Color1";
static Utf8CP INTERNALMATERIALTAGS_Color2                    = "Color2";
static Utf8CP INTERNALMATERIALTAGS_Alpha1                    = "Alpha1";
static Utf8CP INTERNALMATERIALTAGS_Alpha2                    = "Alpha2";
static Utf8CP INTERNALMATERIALTAGS_FrequencyRatio            = "FrequencyRatio";
static Utf8CP INTERNALMATERIALTAGS_AmplitudeRatio            = "AmplitudeRatio";
static Utf8CP INTERNALMATERIALTAGS_Bias                      = "Bias";
static Utf8CP INTERNALMATERIALTAGS_Gain                      = "Gain";
static Utf8CP INTERNALMATERIALTAGS_Frequencies               = "Frequencies";
static Utf8CP INTERNALMATERIALTAGS_NoiseType                 = "NoiseType";
static Utf8CP INTERNALMATERIALTAGS_CheckerType               = "CheckerType";
static Utf8CP INTERNALMATERIALTAGS_TransitionWidth           = "TransitionWidth";
static Utf8CP INTERNALMATERIALTAGS_LineColor                 = "LineColor";
static Utf8CP INTERNALMATERIALTAGS_FillerColor               = "FillerColor";
static Utf8CP INTERNALMATERIALTAGS_LineAlpha                 = "LineAlpha";
static Utf8CP INTERNALMATERIALTAGS_FillerAlpha               = "FillerAlpha";
static Utf8CP INTERNALMATERIALTAGS_LineWidth                 = "LineWidth";
static Utf8CP INTERNALMATERIALTAGS_GridType                  = "GridType";
static Utf8CP INTERNALMATERIALTAGS_DotColor                  = "DotColor";
static Utf8CP INTERNALMATERIALTAGS_DotAlpha                  = "DotAlpha";
static Utf8CP INTERNALMATERIALTAGS_DotWidth                  = "DotWidth";
static Utf8CP INTERNALMATERIALTAGS_DotType                   = "DotType";
static Utf8CP INTERNALMATERIALTAGS_Color                     = "Color";
static Utf8CP INTERNALMATERIALTAGS_CellColor                 = "CellColor";
static Utf8CP INTERNALMATERIALTAGS_CellAlpha                 = "CellAlpha";
static Utf8CP INTERNALMATERIALTAGS_CellWidth                 = "CellWidth";
static Utf8CP INTERNALMATERIALTAGS_ValueVariation            = "ValueVariation";
static Utf8CP INTERNALMATERIALTAGS_HueVariation              = "HueVariation";
static Utf8CP INTERNALMATERIALTAGS_SaturationVariation       = "SaturationVariation";
static Utf8CP INTERNALMATERIALTAGS_CellType                  = "CellType";
static Utf8CP INTERNALMATERIALTAGS_RingColor                 = "RingColor";
static Utf8CP INTERNALMATERIALTAGS_RingAlpha                 = "RingAlpha";
static Utf8CP INTERNALMATERIALTAGS_RingScale                 = "RingScale";
static Utf8CP INTERNALMATERIALTAGS_WaveScale                 = "WaveScale";
static Utf8CP INTERNALMATERIALTAGS_Waviness                  = "Waviness";
static Utf8CP INTERNALMATERIALTAGS_Graininess                = "Graininess";
static Utf8CP INTERNALMATERIALTAGS_GrainScale                = "GrainScale";
static Utf8CP INTERNALMATERIALTAGS_YarnColor                 = "YarnColor";
static Utf8CP INTERNALMATERIALTAGS_GapColor                  = "GapColor";
static Utf8CP INTERNALMATERIALTAGS_YarnAlpha                 = "YarnAlpha";
static Utf8CP INTERNALMATERIALTAGS_GapAlpha                  = "GapAlpha";
static Utf8CP INTERNALMATERIALTAGS_YarnWidth                 = "YarnWidth";
static Utf8CP INTERNALMATERIALTAGS_Roundness                 = "Roundness";
static Utf8CP INTERNALMATERIALTAGS_CrestColor                = "CrestColor";
static Utf8CP INTERNALMATERIALTAGS_TroughColor               = "TroughColor";
static Utf8CP INTERNALMATERIALTAGS_CrestAlpha                = "CrestAlpha";
static Utf8CP INTERNALMATERIALTAGS_TroughAlpha               = "TroughAlpha";
static Utf8CP INTERNALMATERIALTAGS_Wavelength                = "Wavelength";
static Utf8CP INTERNALMATERIALTAGS_Phase                     = "Phase";
static Utf8CP INTERNALMATERIALTAGS_Sources                   = "Sources";
static Utf8CP INTERNALMATERIALTAGS_Value1                    = "Value1";
static Utf8CP INTERNALMATERIALTAGS_Value2                    = "Value2";
static Utf8CP INTERNALMATERIALTAGS_LineValue                 = "LineValue";
static Utf8CP INTERNALMATERIALTAGS_FillerValue               = "FillerValue";
static Utf8CP INTERNALMATERIALTAGS_DotValue                  = "DotValue";
static Utf8CP INTERNALMATERIALTAGS_RingValue                 = "RingValue";
static Utf8CP INTERNALMATERIALTAGS_YarnValue                 = "YarnValue";
static Utf8CP INTERNALMATERIALTAGS_GapValue                  = "GapValue";
static Utf8CP INTERNALMATERIALTAGS_CrestValue                = "CrestValue";
static Utf8CP INTERNALMATERIALTAGS_TroughValue               = "TroughValue";
static Utf8CP INTERNALMATERIALTAGS_CellValue                 = "CellValue";
static Utf8CP INTERNALMATERIALTAGS_EnvelopeVersion           = "EnvVersion";
// static Utf8CP INTERNALMATERIALTAGS_EnvelopeType              = "EnvType";
static Utf8CP INTERNALMATERIALTAGS_Pre                       = "Pre";
static Utf8CP INTERNALMATERIALTAGS_Post                      = "Post";
static Utf8CP INTERNALMATERIALTAGS_KeyX                      = "KeyX";
static Utf8CP INTERNALMATERIALTAGS_KeyY                      = "KeyY";
static Utf8CP INTERNALMATERIALTAGS_TANISlopeType             = "TANISlopeType";
static Utf8CP INTERNALMATERIALTAGS_TANIWeightType            = "TANIWeightType";
static Utf8CP INTERNALMATERIALTAGS_TANISlope                 = "TANISlope";
static Utf8CP INTERNALMATERIALTAGS_TANIWeight                = "TANIWeight";
static Utf8CP INTERNALMATERIALTAGS_TANIValue                 = "TANIValue";
static Utf8CP INTERNALMATERIALTAGS_TANOBreaks                = "TANOBreaks";
static Utf8CP INTERNALMATERIALTAGS_TANOSlopeType             = "TANOSlopeType";
static Utf8CP INTERNALMATERIALTAGS_TANOWeightType            = "TANOWeightType";
static Utf8CP INTERNALMATERIALTAGS_TANOSlope                 = "TANOSlope";
static Utf8CP INTERNALMATERIALTAGS_TANOWeight                = "TANOWeight";
static Utf8CP INTERNALMATERIALTAGS_TANOValue                 = "TANOValue";
static Utf8CP INTERNALMATERIALTAGS_EnvelopeFlag              = "Flag";
static Utf8CP INTERNALMATERIALTAGS_Input                     = "Input";
static Utf8CP INTERNALMATERIALTAGS_RedSuffix                 = ".r";
static Utf8CP INTERNALMATERIALTAGS_GreenSuffix               = ".g";
static Utf8CP INTERNALMATERIALTAGS_BlueSuffix                = ".b";
static Utf8CP INTERNALMATERIALTAGS_LxoRedSuffix              = ".R";
static Utf8CP INTERNALMATERIALTAGS_LxoGreenSuffix            = ".G";
static Utf8CP INTERNALMATERIALTAGS_LxoBlueSuffix             = ".B";
static Utf8CP INTERNALMATERIALTAGS_XSuffix                   = ".x";
static Utf8CP INTERNALMATERIALTAGS_YSuffix                   = ".y";
static Utf8CP INTERNALMATERIALTAGS_ZSuffix                   = ".z";
// static Utf8CP INTERNALMATERIALTAGS_GradientEnvelope          = "GradientEnvelope";
// static Utf8CP INTERNALMATERIALTAGS_CutSectionMaterialName    = "CutSectionMaterialName";
// static Utf8CP INTERNALMATERIALTAGS_CutSectionMaterialPalette = "CutSectionMaterialPalette";
static Utf8CP INTERNALMATERIALTAGS_OcclusionDistance         = "OcclusionDistance";
static Utf8CP INTERNALMATERIALTAGS_Variance                  = "Variance";
static Utf8CP INTERNALMATERIALTAGS_VarianceScale             = "VarianceScale";
static Utf8CP INTERNALMATERIALTAGS_SpreadAngle               = "SpreadAngle";
static Utf8CP INTERNALMATERIALTAGS_MaxCavityAngle            = "MaxCavityAngle";
static Utf8CP INTERNALMATERIALTAGS_OcclusionType             = "OcclusionType";
static Utf8CP INTERNALMATERIALTAGS_OcclusionRays             = "OcclusionRays";
static Utf8CP INTERNALMATERIALTAGS_SameSurface               = "SameSurface";
static Utf8CP INTERNALMATERIALTAGS_WoodColor                 = "WoodColor";
static Utf8CP INTERNALMATERIALTAGS_CrackWidth                = "CrackWidth";
static Utf8CP INTERNALMATERIALTAGS_BrightnessVariation       = "BrightnessVariation";
static Utf8CP INTERNALMATERIALTAGS_HorizontalVariation       = "HorizontalVariation";
static Utf8CP INTERNALMATERIALTAGS_BoardsPerColumn           = "BoardsPerColumn";
static Utf8CP INTERNALMATERIALTAGS_BoardsPerRow              = "BoardsPerRow";
static Utf8CP INTERNALMATERIALTAGS_PatternID                 = "PatternID";
static Utf8CP INTERNALMATERIALTAGS_BrickColor                = "BrickColor";
static Utf8CP INTERNALMATERIALTAGS_MortarColor               = "MortarColor";
static Utf8CP INTERNALMATERIALTAGS_Noisiness                 = "Noisiness";
static Utf8CP INTERNALMATERIALTAGS_AspectRatio               = "AspectRatio";
static Utf8CP INTERNALMATERIALTAGS_BondType                  = "BondType";
static Utf8CP INTERNALMATERIALTAGS_HeaderCourseInterval      = "HeaderCourseInterval";
static Utf8CP INTERNALMATERIALTAGS_FlemishHeaders            = "FlemishHeaders";
static Utf8CP INTERNALMATERIALTAGS_ChecksPerMeter            = "ChecksPerMeter";
static Utf8CP INTERNALMATERIALTAGS_CloudColor                = "CloudColor";
static Utf8CP INTERNALMATERIALTAGS_SkyColor                  = "SkyColor";
static Utf8CP INTERNALMATERIALTAGS_Thickness                 = "Thickness";
static Utf8CP INTERNALMATERIALTAGS_Complexity                = "Complexity";
static Utf8CP INTERNALMATERIALTAGS_Noise                     = "Noise";
static Utf8CP INTERNALMATERIALTAGS_CloudsOnly                = "CloudsOnly";
static Utf8CP INTERNALMATERIALTAGS_FlameHeight               = "FlameHeight";
static Utf8CP INTERNALMATERIALTAGS_FlameWidth                = "FlameWidth";
static Utf8CP INTERNALMATERIALTAGS_Turbulence                = "Turbulence";
static Utf8CP INTERNALMATERIALTAGS_FlickerSpeed              = "FlickerSpeed";
static Utf8CP INTERNALMATERIALTAGS_MarbleColor               = "MarbleColor";
static Utf8CP INTERNALMATERIALTAGS_VeinColor                 = "VeinColor";
static Utf8CP INTERNALMATERIALTAGS_VeinTightness             = "VeinTightness";
static Utf8CP INTERNALMATERIALTAGS_SandColor                 = "SandColor";
static Utf8CP INTERNALMATERIALTAGS_Fraction                  = "Fraction";
static Utf8CP INTERNALMATERIALTAGS_TintColor                 = "TintColor";
static Utf8CP INTERNALMATERIALTAGS_StoneColors               = "StoneColors";
static Utf8CP INTERNALMATERIALTAGS_StoneColorOffset          = "StoneColorOffset";
static Utf8CP INTERNALMATERIALTAGS_MortarWidth               = "MortarWidth";
static Utf8CP INTERNALMATERIALTAGS_TurfColor                 = "TurfColor";
static Utf8CP INTERNALMATERIALTAGS_ContrastColor             = "ContrastColor";
static Utf8CP INTERNALMATERIALTAGS_WaterColor                = "WaterColor";
static Utf8CP INTERNALMATERIALTAGS_RippleScale               = "RippleScale";
static Utf8CP INTERNALMATERIALTAGS_RippleComplexity          = "RippleComplexity";
static Utf8CP INTERNALMATERIALTAGS_WaveComplexity            = "WaveComplexity";
static Utf8CP INTERNALMATERIALTAGS_WaveMinimum               = "WaveMinimum";
static Utf8CP INTERNALMATERIALTAGS_RipplesPerWave            = "RipplesPerWave";
static Utf8CP INTERNALMATERIALTAGS_Roughness                 = "Roughness";
static Utf8CP INTERNALMATERIALTAGS_WavesColor                = "WavesColor";
static Utf8CP INTERNALMATERIALTAGS_FogColor                  = "FogColor";
static Utf8CP INTERNALMATERIALTAGS_MinDensity                = "MinDensity";
static Utf8CP INTERNALMATERIALTAGS_MaxDensity                = "MaxDensity";
static Utf8CP INTERNALMATERIALTAGS_DriftSpeed                = "DriftSpeed";
static Utf8CP INTERNALMATERIALTAGS_SwirlSpeed                = "SwirlSpeed";
static Utf8CP INTERNALMATERIALTAGS_Contrast                  = "Contrast";

static WCharCP INTERNALMATERIALTAGS_LayerPrefix              = L"layer";

//=======================================================================================
// @bsiclass                                                    MattGooding     01/13
//=======================================================================================
struct XmlMaterialReader
{
    friend struct Dgn::MaterialManager;

private:
    /*=================================================================================**//**
    * @bsiclass                                                     MattGooding     11/09
    * This flags structure was part of the V8 BSIMaterialLayer structure and was serialized
    * separately from the other layer flags.  Rather than confuse the layer structure with
    * two separate flags structures, this one is represented only here for the purposes of
    * serializing and deserializing V8 materials.
    +===============+===============+===============+===============+===============+======*/
    struct LegacyLayerFlags
        {
        unsigned int m_on       : 1;
        unsigned int m_bgTrans  : 1;
        unsigned int m_invert   : 1;
        unsigned int m_padding  : 29;
        };

    static void                 SetLegacyLayerFlagsFromUInt32 (MaterialMapLayerR layer, uint32_t uint32Value);
    static void                 SetUInt32FromLegacyLayerFlags (uint32_t& uint32Value, MaterialMapLayerCR layer);

    static BentleyStatus        SkipLayerToken (WStringR layerType);
    static BentleyStatus        ConvertProcedureToLuxology (MaterialMapLayerR layer, WCharCP name, BeXmlNodeR rootElement, MaterialVersion version);

    static bool                 GetDouble (double& result, BeXmlNodeR rootElement, PaletteToken token, MaterialVersion version);
    static bool                 GetInt32 (int32_t& result, BeXmlNodeR rootElement, PaletteToken token, MaterialVersion version);
    static bool                 GetUInt32 (uint32_t& result, BeXmlNodeR rootElement, PaletteToken token, MaterialVersion version);
    static bool                 GetColor (RgbFactor& result, BeXmlNodeR rootElement, PaletteToken token, MaterialVersion version);
    static bool                 GetColorFromLxoProcedureXml (RgbFactor& result, BeXmlNodeR rootElement, Utf8CP tag);
    static bool                 GetPoint3d (DPoint3d& result, BeXmlNodeR rootElement, PaletteToken token, MaterialVersion version);
    static bool                 GetPoint2d (DPoint2d& result, BeXmlNodeR rootElement, PaletteToken token, MaterialVersion version);
    static Utf8CP               GetFlagNameFromType (MaterialMapCR map);

    static void                 GetLegacyProcedure (MaterialMapLayer::LegacyProcedureDataR layer, BeXmlNodeR rootElement, MaterialVersion version);
    static void                 GetLxoNoiseProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoCheckerProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoGridProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoDotProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoConstantProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoCellularProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoWoodProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoWeaveProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoRipplesProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoFloatEnvelope (LxoFloatEnvelopeR envelope, BeXmlNodeR rootElement);
    static void                 GetLxoGradientProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoOcclusionProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoBoardsProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoBrickProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoBentleyCheckerProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoChecker3dProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoCloudsProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoFlameProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoFogProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoMarbleProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoSandProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoStoneProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoTurbulenceProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoTurfProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoWaterProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoWavesProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoBentleyWoodProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoAdvancedWoodProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);

    static void                 GetLxoProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoProcedure (MaterialMapLayerR layer, BeXmlNodeR rootElement);

    static void                 GetLxoBoardsProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoBrickProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoCheckerProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoChecker3dProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoCloudsProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoFlameProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoFogProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoMarbleProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoSandProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoStoneProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoTurbulenceProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoTurfProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoWaterProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoWavesProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoBentleyWoodProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoAdvancedWoodProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);
    static void                 GetLxoBGradientProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement);

    static void                 GetMaterialMapLayers (MaterialMapR map, BeXmlNodeR rootElement, MaterialVersion version);
    static void                 GetMaterialShader (MaterialShaderR shader, BeXmlNodeR rootElement, MaterialVersion version);
    static void                 GetMaterialMap (MaterialSettingsR settings, BeXmlNodeR rootElement, MaterialVersion version);
    static void                 GetMaterialFur (MaterialFurR fur, BeXmlNodeR rootElement, MaterialVersion version, DgnDbR source);
    static bool                 IsLayeredProcedural (WCharCP fileName);
    static bool                 IsPMAProcedural (WCharCP fileName);

public:
    static void                 GetMaterialSettings (MaterialSettingsR settings, MaterialVersion version, BeXmlNodeR rootElement, DgnDbR source);
    static BentleyStatus        GetMaterialMapLayer (MaterialMapLayerP layer, BeXmlNodeR layerElement, MaterialVersion version);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool XmlMaterialReader::IsLayeredProcedural (WCharCP fileName)
    {
    WString name, extension;
    BeFileName::ParseName (NULL, NULL, &name, &extension, fileName);
    return name.EqualsI(L"layers") && extension.EqualsI(L"pma");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool XmlMaterialReader::IsPMAProcedural (WCharCP fileName)
    {
    WString extension;
    BeFileName::ParseName (NULL, NULL, NULL, &extension, fileName);
    return extension.EqualsI(L"pma");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool XmlMaterialReader::GetDouble (double& result, BeXmlNodeR rootElement, PaletteToken token, MaterialVersion version)
    {
    Utf8String keyword;
    MaterialTokenManager::GetManagerR ().ResolvePaletteKeyword (keyword, token, version);

    return BEXML_Success == rootElement.GetAttributeDoubleValue (result, keyword.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool XmlMaterialReader::GetInt32 (int32_t& result, BeXmlNodeR rootElement, PaletteToken token, MaterialVersion version)
    {
    Utf8String keyword;
    MaterialTokenManager::GetManagerR ().ResolvePaletteKeyword (keyword, token, version);

    return BEXML_Success == rootElement.GetAttributeInt32Value (result, keyword.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool XmlMaterialReader::GetUInt32 (uint32_t& result, BeXmlNodeR rootElement, PaletteToken token, MaterialVersion version)
    {
    Utf8String keyword;
    MaterialTokenManager::GetManagerR ().ResolvePaletteKeyword (keyword, token, version);

    return BEXML_Success == rootElement.GetAttributeUInt32Value (result, keyword.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool XmlMaterialReader::GetColor (RgbFactor& result, BeXmlNodeR rootElement, PaletteToken token, MaterialVersion version)
    {
    Utf8String keyword, fullKeyword;
    MaterialTokenManager::GetManagerR ().ResolvePaletteKeyword (keyword, token, version);

    fullKeyword = keyword + INTERNALMATERIALTAGS_RedSuffix;
    if (BEXML_Success != rootElement.GetAttributeDoubleValue (result.red, fullKeyword.c_str ()))
        return false;

    fullKeyword = keyword + INTERNALMATERIALTAGS_GreenSuffix;
    if (BEXML_Success != rootElement.GetAttributeDoubleValue (result.green, fullKeyword.c_str ()))
        return false;

    fullKeyword = keyword + INTERNALMATERIALTAGS_BlueSuffix;
    return (BEXML_Success == rootElement.GetAttributeDoubleValue (result.blue, fullKeyword.c_str ()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool XmlMaterialReader::GetColorFromLxoProcedureXml (RgbFactor& result, BeXmlNodeR rootElement, Utf8CP tag)
    {
    // In V8 DGNs, colors for LxoProcedure were saved with uppercase RGB instead of lowercase rgb like all other material colors...
    Utf8String keyword = tag, fullKeyword;
    fullKeyword = keyword + INTERNALMATERIALTAGS_LxoRedSuffix;
    if (BEXML_Success != rootElement.GetAttributeDoubleValue (result.red, fullKeyword.c_str ()))
        return false;

    fullKeyword = keyword + INTERNALMATERIALTAGS_LxoGreenSuffix;
    if (BEXML_Success != rootElement.GetAttributeDoubleValue (result.green, fullKeyword.c_str ()))
        return false;

    fullKeyword = keyword + INTERNALMATERIALTAGS_LxoBlueSuffix;
    return (BEXML_Success == rootElement.GetAttributeDoubleValue (result.blue, fullKeyword.c_str ()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool XmlMaterialReader::GetPoint3d (DPoint3d& result, BeXmlNodeR rootElement, PaletteToken token, MaterialVersion version)
    {
    Utf8String keyword, fullKeyword;
    MaterialTokenManager::GetManagerR ().ResolvePaletteKeyword (keyword, token, version);

    fullKeyword = keyword + INTERNALMATERIALTAGS_XSuffix;
    if (BEXML_Success != rootElement.GetAttributeDoubleValue (result.x, fullKeyword.c_str () ))
        return false;

    fullKeyword = keyword + INTERNALMATERIALTAGS_YSuffix;
    if (BEXML_Success != rootElement.GetAttributeDoubleValue (result.y, fullKeyword.c_str ()))
        return false;

    fullKeyword = keyword + INTERNALMATERIALTAGS_ZSuffix;
    return (BEXML_Success == rootElement.GetAttributeDoubleValue (result.z, fullKeyword.c_str ()
    ));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool XmlMaterialReader::GetPoint2d (DPoint2d& result, BeXmlNodeR rootElement, PaletteToken token, MaterialVersion version)
    {
    Utf8String keyword, fullKeyword;
    MaterialTokenManager::GetManagerR ().ResolvePaletteKeyword (keyword, token, version);

    fullKeyword = keyword + INTERNALMATERIALTAGS_XSuffix;
    if (BEXML_Success != rootElement.GetAttributeDoubleValue (result.x, fullKeyword.c_str ()))
        return false;

    fullKeyword = keyword + INTERNALMATERIALTAGS_YSuffix;
    return (BEXML_Success == rootElement.GetAttributeDoubleValue (result.y, fullKeyword.c_str ()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP XmlMaterialReader::GetFlagNameFromType (MaterialMapCR map)
    {
    if (MaterialMap::MAPTYPE_Pattern != map.GetType () && MaterialMap::MAPTYPE_Bump != map.GetType () && !map.IsValueMap ())
        return NULL;

    switch (map.GetType ())
        {
        case MaterialMap::MAPTYPE_Pattern:
            return INTERNALMATERIALTAGS_PatternFlags;
        case MaterialMap::MAPTYPE_Bump:
            return INTERNALMATERIALTAGS_BumpFlags;
        case MaterialMap::MAPTYPE_Reflect:
            return INTERNALMATERIALTAGS_ReflectFlags;
        case MaterialMap::MAPTYPE_Transparency:
            return INTERNALMATERIALTAGS_TransparencyFlags;
        case MaterialMap::MAPTYPE_Translucency:
            return INTERNALMATERIALTAGS_TranslucencyFlags;
        case MaterialMap::MAPTYPE_Finish:
            return INTERNALMATERIALTAGS_FinishFlags;
        case MaterialMap::MAPTYPE_Diffuse:
            return INTERNALMATERIALTAGS_DiffuseFlags;
        case MaterialMap::MAPTYPE_GlowAmount:
            return INTERNALMATERIALTAGS_GlowAmountFlags;
        case MaterialMap::MAPTYPE_ClearcoatAmount:
            return INTERNALMATERIALTAGS_ClearcoatAmountFlags;
        case MaterialMap::MAPTYPE_AnisotropicDirection:
            return INTERNALMATERIALTAGS_AnisotropicDirectionFlags;
        case MaterialMap::MAPTYPE_Displacement:
            return INTERNALMATERIALTAGS_DisplacementFlags;
        case MaterialMap::MAPTYPE_Normal:
            return INTERNALMATERIALTAGS_NormalFlags;
        case MaterialMap::MAPTYPE_FurLength:
            return INTERNALMATERIALTAGS_FurLengthFlags;
        case MaterialMap::MAPTYPE_FurDensity:
            return INTERNALMATERIALTAGS_FurDensityFlags;
        case MaterialMap::MAPTYPE_FurJitter:
            return INTERNALMATERIALTAGS_FurJitterFlags;
        case MaterialMap::MAPTYPE_FurFlex:
            return INTERNALMATERIALTAGS_FurFlexFlags;
        case MaterialMap::MAPTYPE_FurClumps:
            return INTERNALMATERIALTAGS_FurClumpsFlags;
        case MaterialMap::MAPTYPE_FurDirection:
            return INTERNALMATERIALTAGS_FurDirectionFlags;
        case MaterialMap::MAPTYPE_FurVector:
            return INTERNALMATERIALTAGS_FurVectorFlags;
        case MaterialMap::MAPTYPE_FurBump:
            return INTERNALMATERIALTAGS_FurBumpFlags;
        case MaterialMap::MAPTYPE_FurCurls:
            return INTERNALMATERIALTAGS_FurCurlsFlags;
        case MaterialMap::MAPTYPE_RefractionRoughness:
            return INTERNALMATERIALTAGS_RefractionRoughnessFlags;
        case MaterialMap::MAPTYPE_SpecularFresnel:
            return INTERNALMATERIALTAGS_SpecularFresnelFlags;
        case MaterialMap::MAPTYPE_Specular:
        default:
            return INTERNALMATERIALTAGS_SpecularFlags;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoNoiseProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoNoiseProcedureP noise = reinterpret_cast <LxoNoiseProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_Color1))                     { noise->GetColor1R () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_Color2))                     { noise->GetColor2R () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Alpha1))             { noise->SetAlpha1 (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Alpha2))             { noise->SetAlpha2 (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_FrequencyRatio))     { noise->SetFrequencyRatio (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_AmplitudeRatio))     { noise->SetAmplitudeRatio (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Bias))               { noise->SetBias (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Gain))               { noise->SetGain (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Value1))             { noise->SetValue1 (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Value2))             { noise->SetValue2 (doubleValue); }

    uint32_t uint32Value;
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_Frequencies))        { noise->SetFrequencies (uint32Value); }
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_NoiseType))
        noise->SetNoiseType (static_cast <LxoNoiseProcedure::NoiseType> (uint32Value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoCheckerProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoCheckerProcedureP checker = reinterpret_cast <LxoCheckerProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_Color1))                     { checker->GetColor1R () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_Color2))                     { checker->GetColor2R () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Alpha1))             { checker->SetAlpha1 (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Alpha2))             { checker->SetAlpha2 (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_TransitionWidth))    { checker->SetTransitionWidth (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Bias))               { checker->SetBias (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Gain))               { checker->SetGain (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Value1))             { checker->SetValue1 (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Value2))             { checker->SetValue2 (doubleValue); }

    uint32_t uint32Value;
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_CheckerType))
        checker->SetCheckerType (static_cast <LxoCheckerProcedure::CheckerType> (uint32Value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoGridProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoGridProcedureP grid = reinterpret_cast <LxoGridProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_LineColor))                  { grid->GetLineColorR () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_FillerColor))                { grid->GetFillerColorR () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_LineAlpha))          { grid->SetLineAlpha (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_FillerAlpha))        { grid->SetFillerAlpha (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_TransitionWidth))    { grid->SetTransitionWidth (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_LineWidth))          { grid->SetLineWidth (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Bias))               { grid->SetBias (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Gain))               { grid->SetGain (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_LineValue))          { grid->SetLineValue (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_FillerValue))        { grid->SetFillerValue (doubleValue); }

    uint32_t uint32Value;
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_GridType))
        grid->SetGridType (static_cast <LxoGridProcedure::GridType> (uint32Value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoDotProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoDotProcedureP dot = reinterpret_cast <LxoDotProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_DotColor))                   { dot->GetDotColorR () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_FillerColor))                { dot->GetFillerColorR () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_DotAlpha))           { dot->SetDotAlpha (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_FillerAlpha))        { dot->SetFillerAlpha (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_TransitionWidth))    { dot->SetTransitionWidth (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_DotWidth))           { dot->SetDotWidth (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Bias))               { dot->SetBias (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Gain))               { dot->SetGain (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_DotValue))           { dot->SetDotValue (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_FillerValue))        { dot->SetFillerValue (doubleValue); }

    uint32_t uint32Value;
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_DotType))
        dot->SetDotType (static_cast <LxoDotProcedure::DotType> (uint32Value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoConstantProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoConstantProcedureP constant = reinterpret_cast <LxoConstantProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_Color))                      { constant->GetColorR () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Value1))             { constant->SetValue (doubleValue); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoCellularProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoCellularProcedureP cellular = reinterpret_cast <LxoCellularProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_CellColor))                      { cellular->GetCellColorR () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_FillerColor))                    { cellular->GetFillerColorR () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_CellAlpha))              { cellular->SetCellAlpha (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_FillerAlpha))            { cellular->SetFillerAlpha (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_TransitionWidth))        { cellular->SetTransitionWidth (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_CellWidth))              { cellular->SetCellWidth (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Bias))                   { cellular->SetBias (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Gain))                   { cellular->SetGain (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_FrequencyRatio))         { cellular->SetFrequencyRatio (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_AmplitudeRatio))         { cellular->SetAmplitudeRatio (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_ValueVariation))         { cellular->SetValueVariation (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_HueVariation))           { cellular->SetHueVariation (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_SaturationVariation))    { cellular->SetSaturationVariation (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_CellValue))              { cellular->SetCellValue (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_FillerValue))            { cellular->SetFillerValue (doubleValue); }

    uint32_t uint32Value;
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_Frequencies))            { cellular->SetFrequencies (uint32Value); }
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_CellType))
        cellular->SetCellularType (static_cast <LxoCellularProcedure::CellularType> (uint32Value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoWoodProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoWoodProcedureP wood = reinterpret_cast <LxoWoodProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_RingColor))                      { wood->GetRingColorR () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_FillerColor))                    { wood->GetFillerColorR () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_RingAlpha))              { wood->SetRingAlpha (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_FillerAlpha))            { wood->SetFillerAlpha (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Bias))                   { wood->SetBias (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Gain))                   { wood->SetGain (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_RingScale))              { wood->SetRingScale (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_WaveScale))              { wood->SetWaveScale (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Waviness))               { wood->SetWaviness (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Graininess))             { wood->SetGraininess (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_GrainScale))             { wood->SetGrainScale (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_RingValue))              { wood->SetRingValue (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_FillerValue))            { wood->SetFillerValue (doubleValue); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoWeaveProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoWeaveProcedureP weave = reinterpret_cast <LxoWeaveProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_YarnColor))                      { weave->GetYarnColorR () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_GapColor))                       { weave->GetGapColorR () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_YarnAlpha))              { weave->SetYarnAlpha (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_GapAlpha))               { weave->SetGapAlpha (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_YarnWidth))              { weave->SetYarnWidth (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Roundness))              { weave->SetRoundness (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Bias))                   { weave->SetBias (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Gain))                   { weave->SetGain (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_YarnValue))              { weave->SetYarnValue (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_GapValue))               { weave->SetGapValue (doubleValue); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoRipplesProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoRipplesProcedureP ripples = reinterpret_cast <LxoRipplesProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_CrestColor))                     { ripples->GetCrestColorR () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_TroughColor))                    { ripples->GetTroughColorR () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_CrestAlpha))             { ripples->SetCrestAlpha (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_TroughAlpha))            { ripples->SetTroughAlpha (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Wavelength))             { ripples->SetWavelength (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Phase))                  { ripples->SetPhase (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Bias))                   { ripples->SetBias (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Gain))                   { ripples->SetGain (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_CrestValue))             { ripples->SetCrestValue (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_TroughValue))            { ripples->SetTroughValue (doubleValue); }

    uint32_t uint32Value;
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_Sources))                { ripples->SetSources (uint32Value); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoOcclusionProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoOcclusionProcedureP occl = reinterpret_cast <LxoOcclusionProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_Color1))                     { occl->GetColor1R () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_Color2))                     { occl->GetColor2R () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Alpha1))                        { occl->SetAlpha1 (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Alpha2))                        { occl->SetAlpha2 (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Value1))                        { occl->SetValue1 (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Value2))                        { occl->SetValue2 (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Bias))                          { occl->SetBias (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Gain))                          { occl->SetGain (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_OcclusionDistance))             { occl->SetOcclusionDistance (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Variance))                      { occl->SetVariance (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_VarianceScale))                 { occl->SetVarianceScale (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_SpreadAngle))                   { occl->SetSpreadAngle (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_MaxCavityAngle))                { occl->SetMaxCavityAngle (doubleValue); }

    uint32_t uint32Value;
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_SameSurface))                   { occl->SetSameSurface (TO_BOOL (uint32Value)); }
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_OcclusionType))                 { occl->SetOcclusionType (static_cast <LxoOcclusionProcedure::OcclusionType> (uint32Value)); }
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_OcclusionRays))                 { occl->SetOcclusionRays (uint32Value); }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoFloatEnvelope (LxoFloatEnvelopeR envelope, BeXmlNodeR rootElement)
    {
    BeXmlNodeP envelopeElement = rootElement.SelectSingleNode (INTERNALMATERIALTAGS_LxoEnvelope);
    if (NULL == envelopeElement)
        return;

    uint32_t                uint32Value;
    LxoEnvelopeXmlVersion   envelopeVersion;
    if (BEXML_Success == envelopeElement->GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_EnvelopeVersion))
        envelopeVersion = static_cast <LxoEnvelopeXmlVersion> (uint32Value);

    if (BEXML_Success == envelopeElement->GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_Pre))
        envelope.SetPreBehavior (static_cast <LxoFloatEnvelope::EndBehavior> (uint32Value));

    if (BEXML_Success == envelopeElement->GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_Post))
        envelope.SetPostBehavior (static_cast <LxoFloatEnvelope::EndBehavior> (uint32Value));

    envelope.GetComponentsR ().InitDefaults ();
    for (BeXmlNodeP childElement = envelopeElement->GetFirstChild(); NULL != childElement; childElement = childElement->GetNextSibling())
        {
        LxoFloatEnvelopeComponentR  component = envelope.GetComponentsR ().AddComponent ();
        uint32_t                    uint32Value;
        double                      doubleValue;

        LxoFloatEnvelopeKeyR key = component.GetKeyR ();
        if (BEXML_Success == childElement->GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_KeyX))              { key.GetValueR ().x = doubleValue; }
        if (BEXML_Success == childElement->GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_KeyY))              { key.GetValueR ().y = doubleValue; }

        LxoEnvelopeTangentInR tanIn = component.GetTangentInR ();
        if (BEXML_Success == childElement->GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_TANISlopeType))     { tanIn.SetSlopeType (static_cast <LxoEnvelopeSlopeType> (uint32Value)); }
        if (BEXML_Success == childElement->GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_TANIWeightType))    { tanIn.SetWeightType (static_cast <LxoEnvelopeWeightType> (uint32Value)); }
        if (BEXML_Success == childElement->GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_TANISlope))         { tanIn.SetSlope (doubleValue); }
        if (BEXML_Success == childElement->GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_TANIWeight))        { tanIn.SetWeight (doubleValue); }
        if (BEXML_Success == childElement->GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_TANIValue))         { tanIn.SetValue (doubleValue); }

        LxoEnvelopeTangentOutR tanOut = component.GetTangentOutR ();
        if (BEXML_Success == childElement->GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_TANOBreaks))
            tanOut.SetBreak (0 != uint32Value ? (LxoEnvelopeBreak)uint32Value : LxoEnvelopeBreak::Value);

        if (BEXML_Success == childElement->GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_TANOSlopeType))     { tanOut.SetSlopeType (static_cast <LxoEnvelopeSlopeType> (uint32Value)); }
        if (BEXML_Success == childElement->GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_TANOWeightType))    { tanOut.SetWeightType (static_cast <LxoEnvelopeWeightType> (uint32Value)); }
        if (BEXML_Success == childElement->GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_TANOSlope))         { tanOut.SetSlope (doubleValue); }
        if (BEXML_Success == childElement->GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_TANOWeight))        { tanOut.SetWeight (doubleValue); }
        if (BEXML_Success == childElement->GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_TANOValue))         { tanOut.SetValue (doubleValue); }

        if (BEXML_Success == childElement->GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_EnvelopeFlag))      { component.SetFlag (uint32Value); }
        }

    // Upgrade envelope depending on version here.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoGradientProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoGradientProcedureP gradient = reinterpret_cast <LxoGradientProcedureP> (&procedure);

    uint32_t uint32Value;
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_Input))
        gradient->SetGradientInput (static_cast <LxoGradientProcedure::GradientInput> (uint32Value));

    int     i= 0;
    for (BeXmlNodeP lxoFloatEnvelopeEl = rootElement.GetFirstChild(); NULL != lxoFloatEnvelopeEl; lxoFloatEnvelopeEl = lxoFloatEnvelopeEl->GetNextSibling())
        {
        LxoFloatEnvelopeP envelope = NULL;

        switch (i)
            {
            case 0:
                envelope = &gradient->GetValueEnvelopeR ();
                break;
            case 1:
                envelope = &gradient->GetRedEnvelopeR ();
                break;
            case 2:
                envelope = &gradient->GetGreenEnvelopeR ();
                break;
            case 3:
                envelope = &gradient->GetBlueEnvelopeR ();
                break;
            case 4:
                envelope = &gradient->GetAlphaEnvelopeR ();
                break;
            default:
                BeAssert (false);
                break;
            }

        if (NULL == envelope)
            break;

        GetLxoFloatEnvelope (*envelope, *lxoFloatEnvelopeEl);
        i++;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoBoardsProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoBoardsProcedureP proc = reinterpret_cast <LxoBoardsProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_WoodColor))                     { proc->GetWoodColorR () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_RingColor))                     { proc->GetRingColorR () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_CrackWidth))                        { proc->SetCrackWidth (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_BrightnessVariation))               { proc->SetBrightnessVariation (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_HorizontalVariation))               { proc->SetHorizontalVariation (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_GrainScale))                        { proc->SetGrainScale (doubleValue); }

    uint32_t uint32Value;
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_BoardsPerColumn))                   { proc->SetBoardsPerColumn (uint32Value); }
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_BoardsPerRow))                      { proc->SetBoardsPerRow (static_cast <LxoOcclusionProcedure::OcclusionType> (uint32Value)); }
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_PatternID))                         { proc->SetPatternId (uint32Value); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoBrickProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoBrickProcedureP proc = reinterpret_cast <LxoBrickProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_BrickColor))                     { proc->GetBrickColorR () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_MortarColor))                    { proc->GetMortarColorR () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_MortarWidth))                       { proc->SetMortarWidth (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_BrightnessVariation))               { proc->SetBrightnessVariation (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_HorizontalVariation))               { proc->SetHorizontalVariation (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Noisiness))                         { proc->SetNoisiness (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_AspectRatio))                       { proc->SetAspectRatio (doubleValue); }

    uint32_t uint32Value;
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_BondType))                          { proc->SetBondType (static_cast <LxoBrickProcedure::BrickBondType> (uint32Value)); }
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_FlemishHeaders))                    { proc->SetFlemishHeaders (TO_BOOL (uint32Value)); }
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_HeaderCourseInterval))              { proc->SetHeaderCourseInterval (uint32Value); }
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_PatternID))                         { proc->SetPatternId (uint32Value); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoBentleyCheckerProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoBentleyCheckerProcedureP  proc = reinterpret_cast <LxoBentleyCheckerProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_Color1))                         { proc->GetColor1R () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_Color2))                         { proc->GetColor2R () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_ChecksPerMeter))    { proc->SetChecksPerMeter (doubleValue); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoChecker3dProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoChecker3dProcedureP  proc = reinterpret_cast <LxoChecker3dProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_Color1))                         { proc->GetColor1R () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_Color2))                         { proc->GetColor2R () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_ChecksPerMeter))    { proc->SetChecksPerMeter (doubleValue); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoCloudsProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoCloudsProcedureP proc = reinterpret_cast <LxoCloudsProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_CloudColor))                     { proc->GetCloudColorR () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_SkyColor))                       { proc->GetSkyColorR () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Thickness))                { proc->SetThickness (doubleValue * 20.0); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Complexity))               { proc->SetComplexity (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Noise))                    { proc->SetNoise (doubleValue); }

    uint32_t uint32Value;
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_CloudsOnly))               { proc->SetCloudsOnly (TO_BOOL (uint32Value)); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoFlameProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoFlameProcedureP proc = reinterpret_cast <LxoFlameProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_Color))                     { proc->GetColorR () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_FlameHeight))           { proc->SetFlameHeight (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_FlameWidth))            { proc->SetFlameWidth (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Turbulence))            { proc->SetTurbulence (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Complexity))            { proc->SetComplexity (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_FlickerSpeed))          { proc->SetFlickerSpeed (doubleValue); }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoFogProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoFogProcedureP proc = reinterpret_cast <LxoFogProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_FogColor))                     { proc->GetColorR () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_MinDensity))            { proc->SetMinDensity (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_MaxDensity))            { proc->SetMaxDensity (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_DriftSpeed))            { proc->SetDriftSpeed (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_SwirlSpeed))            { proc->SetSwirlSpeed (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Thickness))             { proc->SetThickness (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Contrast))              { proc->SetContrast (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Complexity))            { proc->SetComplexity (doubleValue); }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoMarbleProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoMarbleProcedureP proc = reinterpret_cast <LxoMarbleProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_MarbleColor))                    { proc->GetMarbleColorR () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_VeinColor))                      { proc->GetVeinColorR () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_VeinTightness))         { proc->SetVeinTightness (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Complexity))            { proc->SetComplexity (doubleValue); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoSandProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoSandProcedureP proc = reinterpret_cast <LxoSandProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_SandColor))                      { proc->GetSandColorR () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_ContrastColor))                  { proc->GetContrastColorR () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Fraction))         { proc->SetFraction (doubleValue); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoStoneProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoStoneProcedureP proc = reinterpret_cast <LxoStoneProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_TintColor))                      { proc->GetTintColorR () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_MortarColor))                    { proc->GetMortarColorR () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_MortarWidth))             { proc->SetMortarWidth (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Noisiness))               { proc->SetNoisiness (doubleValue); }

    uint32_t uint32Value;
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_StoneColors))               { proc->SetStoneColors (uint32Value); }
    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_StoneColorOffset))          { proc->SetStoneColorOffset (uint32Value); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoTurbulenceProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoTurbulenceProcedureP proc = reinterpret_cast <LxoTurbulenceProcedureP> (&procedure);

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Complexity))             { proc->SetComplexity (doubleValue); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoTurfProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoTurfProcedureP proc = reinterpret_cast <LxoTurfProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_TurfColor))                      { proc->GetTurfColorR () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_ContrastColor))                  { proc->GetContrastColorR () = colorValue; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoWaterProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoWaterProcedureP proc = reinterpret_cast <LxoWaterProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_WaterColor))                      { proc->GetWaterColorR () = colorValue; }

    double doubleValue;
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_RippleScale))               { proc->SetRippleScale (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_RippleComplexity))          { proc->SetRippleComplexity (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_WaveScale))                 { proc->SetWaveScale (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_WaveComplexity))            { proc->SetWaveComplexity (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_WaveMinimum))               { proc->SetWaveMinimum (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_RipplesPerWave))            { proc->SetRipplesPerWave (doubleValue); }
    if (BEXML_Success == rootElement.GetAttributeDoubleValue (doubleValue, INTERNALMATERIALTAGS_Roughness))                 { proc->SetRoughness (doubleValue); }

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoWavesProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoWavesProcedureP proc = reinterpret_cast <LxoWavesProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_WavesColor))                         { proc->GetWavesColorR () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_ContrastColor))                      { proc->GetContrastColorR () = colorValue; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoBentleyWoodProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoBentleyWoodProcedureP proc = reinterpret_cast <LxoBentleyWoodProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_WoodColor))                          { proc->GetWoodColorR () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_RingColor))                          { proc->GetRingColorR () = colorValue; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoAdvancedWoodProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoAdvancedWoodProcedureP proc = reinterpret_cast <LxoAdvancedWoodProcedureP> (&procedure);

    RgbFactor colorValue;
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_WoodColor))                          { proc->GetWoodColorR () = colorValue; }
    if (GetColorFromLxoProcedureXml (colorValue, rootElement, INTERNALMATERIALTAGS_RingColor))                          { proc->GetRingColorR () = colorValue; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoBoardsProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoBoardsProcedureP boards = reinterpret_cast <LxoBoardsProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

        if (0 == keyword.compare (L"pattern_id"))
            boards->SetPatternId (BeStringUtilities::Wtoi (value.c_str ()));
        else if (0 == keyword.compare (L"primary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &boards->GetWoodColorR ().red, &boards->GetWoodColorR ().green, &boards->GetWoodColorR ().blue);
        else if (0 == keyword.compare (L"secondary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &boards->GetRingColorR ().red, &boards->GetRingColorR ().green, &boards->GetRingColorR ().blue);
        else if (0 == keyword.compare (L"boards_per_column"))
            boards->SetBoardsPerColumn (BeStringUtilities::Wtoi (value.c_str ()));
        else if (0 == keyword.compare (L"boards_per_row"))
            boards->SetBoardsPerRow (BeStringUtilities::Wtoi (value.c_str ()));
        else if (0 == keyword.compare (L"crack_width"))
            boards->SetCrackWidth (BeStringUtilities::Wtof (value.c_str ()) * 100.0);
        else if (0 == keyword.compare (L"brightness_variation"))
            boards->SetBrightnessVariation (BeStringUtilities::Wtof (value.c_str ()) * 100.0);
        else if (0 == keyword.compare (L"horizontal_variation"))
            boards->SetHorizontalVariation (BeStringUtilities::Wtof (value.c_str ()) * 100.0);
        else if (0 == keyword.compare (L"wood_grain_scale_factor"))
            boards->SetGrainScale (BeStringUtilities::Wtof (value.c_str ()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoBrickProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoBrickProcedureP  brick = reinterpret_cast <LxoBrickProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

        if (0 == keyword.compare (L"pattern_id"))
            brick->SetPatternId (BeStringUtilities::Wtoi (value.c_str ()));
        else if (0 == keyword.compare (L"primary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &brick->GetBrickColorR ().red, &brick->GetBrickColorR ().green, &brick->GetBrickColorR ().blue);
        else if (0 == keyword.compare (L"secondary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &brick->GetMortarColorR ().red, &brick->GetMortarColorR ().green, &brick->GetMortarColorR ().blue);
        else if (0 == keyword.compare (L"header_course_interval"))
            brick->SetHeaderCourseInterval (BeStringUtilities::Wtoi (value.c_str ()));
        else if (0 == keyword.compare (L"flemish_headers"))
            brick->SetFlemishHeaders (TO_BOOL (BeStringUtilities::Wtoi (value.c_str ())));
        else if (0 == keyword.compare (L"morter_width"))
            brick->SetMortarWidth (BeStringUtilities::Wtof (value.c_str ()) * 100.0);
        else if (0 == keyword.compare (L"brightness_variation"))
            brick->SetBrightnessVariation (BeStringUtilities::Wtof (value.c_str ()) * 100.0);
        else if (0 == keyword.compare (L"horizontal_variation"))
            brick->SetHorizontalVariation (BeStringUtilities::Wtof (value.c_str ()) * 100.0);
        else if (0 == keyword.compare (L"noisiness"))
            brick->SetNoisiness (BeStringUtilities::Wtof (value.c_str ()) * 100.0);
        else if (0 == keyword.compare (L"aspect_ratio"))
            brick->SetAspectRatio (BeStringUtilities::Wtof (value.c_str ()));
        else if (0 == keyword.compare (L"bond_type"))
            {
            if (0 == value.compare (L"Running"))
                brick->SetBondType (LxoBrickProcedure::BRICKBONDTYPE_Running);
            else if (0 == value.compare (L"Stack"))
                brick->SetBondType (LxoBrickProcedure::BRICKBONDTYPE_Stack);
            else if (0 == value.compare (L"One-third"))
                brick->SetBondType (LxoBrickProcedure::BRICKBONDTYPE_OneThirdRunning);
            else if (0 == value.compare (L"Flemish"))
                brick->SetBondType (LxoBrickProcedure::BRICKBONDTYPE_Flemish);
            else if (0 == value.compare (L"Common"))
                brick->SetBondType (LxoBrickProcedure::BRICKBONDTYPE_Common);
            else if (0 == value.compare (L"Dutch"))
                brick->SetBondType (LxoBrickProcedure::BRICKBONDTYPE_Dutch);
            else if (0 == value.compare (L"English"))
                brick->SetBondType (LxoBrickProcedure::BRICKBONDTYPE_English);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoCheckerProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoBentleyCheckerProcedureP  proc = reinterpret_cast <LxoBentleyCheckerProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

        if (0 == keyword.compare (L"primary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetColor1R ().red, &proc->GetColor1R ().green, &proc->GetColor1R ().blue);
        else if (0 == keyword.compare (L"secondary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetColor2R ().red, &proc->GetColor2R ().green, &proc->GetColor2R ().blue);
        else if (0 == keyword.compare (L"scale_factor"))
            proc->SetChecksPerMeter (BeStringUtilities::Wtof (value.c_str ()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoChecker3dProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoChecker3dProcedureP  proc = reinterpret_cast <LxoChecker3dProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

        if (0 == keyword.compare (L"primary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetColor1R ().red, &proc->GetColor1R ().green, &proc->GetColor1R ().blue);
        else if (0 == keyword.compare (L"secondary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetColor2R ().red, &proc->GetColor2R ().green, &proc->GetColor2R ().blue);
        else if (0 == keyword.compare (L"scale_factor"))
            proc->SetChecksPerMeter (BeStringUtilities::Wtof (value.c_str ()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoCloudsProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoCloudsProcedureP  proc = reinterpret_cast <LxoCloudsProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

        if (0 == keyword.compare (L"primary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetCloudColorR ().red, &proc->GetCloudColorR ().green, &proc->GetCloudColorR ().blue);
        else if (0 == keyword.compare (L"secondary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetSkyColorR ().red, &proc->GetSkyColorR ().green, &proc->GetSkyColorR ().blue);
        if (0 == keyword.compare (L"thickness"))
            proc->SetThickness (BeStringUtilities::Wtof (value.c_str ())  * 20.0); // thickness is 0 -> 5 stored as convert it to 0 -> 100
        if (0 == keyword.compare (L"level_of_detail"))
            proc->SetComplexity (BeStringUtilities::Wtof (value.c_str ()));
        if (0 == keyword.compare (L"step_size"))
            proc->SetNoise (BeStringUtilities::Wtof (value.c_str ()));
        if (0 == keyword.compare (L"only"))
            proc->SetCloudsOnly (TO_BOOL (BeStringUtilities::Wtoi (value.c_str ())));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoFlameProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoFlameProcedureP  proc = reinterpret_cast <LxoFlameProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

        if (0 == keyword.compare (L"primary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetColorR ().red, &proc->GetColorR ().green, &proc->GetColorR ().blue);
        else if (0 == keyword.compare (L"flame_height"))
            proc->SetFlameHeight (BeStringUtilities::Wtof (value.c_str ()) * 100.0);
        else if (0 == keyword.compare (L"flame_width"))
            proc->SetFlameWidth (BeStringUtilities::Wtof (value.c_str ()) * 100.0);
        else if (0 == keyword.compare (L"flicker_speed"))
            proc->SetFlickerSpeed (BeStringUtilities::Wtof (value.c_str ()));
        else if (0 == keyword.compare (L"turbulence_scale"))
            proc->SetTurbulence (BeStringUtilities::Wtof (value.c_str ()));
        else if (0 == keyword.compare (L"level_of_detail"))
            proc->SetComplexity (BeStringUtilities::Wtof (value.c_str ()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoFogProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoFogProcedureP  proc = reinterpret_cast <LxoFogProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

        if (0 == keyword.compare (L"primary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetColorR ().red, &proc->GetColorR ().green, &proc->GetColorR ().blue);
        else if (0 == keyword.compare (L"minimum_density"))
            proc->SetMinDensity (BeStringUtilities::Wtof (value.c_str ()) * 100.0);
        else if (0 == keyword.compare (L"maximum_density"))
            proc->SetMaxDensity (BeStringUtilities::Wtof (value.c_str ()) * 100.0);
        else if (0 == keyword.compare (L"drift_speed"))
            proc->SetDriftSpeed (BeStringUtilities::Wtof (value.c_str ()));
        else if (0 == keyword.compare (L"swirl_speed"))
            proc->SetSwirlSpeed (BeStringUtilities::Wtof (value.c_str ()));
        else if (0 == keyword.compare (L"thickness"))
            proc->SetThickness (BeStringUtilities::Wtof (value.c_str ()));
        else if (0 == keyword.compare (L"contrast"))
            proc->SetContrast (BeStringUtilities::Wtof (value.c_str ()));
        else if (0 == keyword.compare (L"level_of_detail"))
            proc->SetComplexity (BeStringUtilities::Wtof (value.c_str ()));

        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoMarbleProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoMarbleProcedureP  proc = reinterpret_cast <LxoMarbleProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

        if (0 == keyword.compare (L"primary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetMarbleColorR ().red, &proc->GetMarbleColorR ().green, &proc->GetMarbleColorR ().blue);
        else if (0 == keyword.compare (L"secondary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetVeinColorR ().red, &proc->GetVeinColorR ().green, &proc->GetVeinColorR ().blue);
        else if (0 == keyword.compare (L"level_of_detail"))
            proc->SetComplexity (BeStringUtilities::Wtof (value.c_str ()) * 100.0);
        else if (0 == keyword.compare (L"vein_tightness"))
            proc->SetVeinTightness (BeStringUtilities::Wtof (value.c_str ()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoSandProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoSandProcedureP  proc = reinterpret_cast <LxoSandProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

        if (0 == keyword.compare (L"primary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetSandColorR ().red, &proc->GetSandColorR ().green, &proc->GetSandColorR ().blue);
        else if (0 == keyword.compare (L"secondary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetContrastColorR ().red, &proc->GetContrastColorR ().green, &proc->GetContrastColorR ().blue);
        else if (0 == keyword.compare (L"secondary_fraction"))
            proc->SetFraction (BeStringUtilities::Wtof (value.c_str ()) * 100.0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoStoneProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoStoneProcedureP  proc = reinterpret_cast <LxoStoneProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

        if (0 == keyword.compare (L"primary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetTintColorR ().red, &proc->GetTintColorR ().green, &proc->GetTintColorR ().blue);
        else if (0 == keyword.compare (L"secondary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetMortarColorR ().red, &proc->GetMortarColorR ().green, &proc->GetMortarColorR ().blue);
        else if (0 == keyword.compare (L"stone_colors"))
            proc->SetStoneColors (BeStringUtilities::Wtoi (value.c_str ()));
        else if (0 == keyword.compare (L"stone_color_table_offset"))
            proc->SetStoneColorOffset (BeStringUtilities::Wtoi (value.c_str ()));
        else if (0 == keyword.compare (L"mortar_thickness"))
            proc->SetMortarWidth (BeStringUtilities::Wtof (value.c_str ()) * 100.0);
        else if (0 == keyword.compare (L"noisiness"))
            proc->SetNoisiness (BeStringUtilities::Wtof (value.c_str ()) * 100.0);

        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoTurbulenceProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoTurbulenceProcedureP  proc = reinterpret_cast <LxoTurbulenceProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

        if (0 == keyword.compare (L"level_of_detail"))
            proc->SetComplexity (BeStringUtilities::Wtof (value.c_str ()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoTurfProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoTurfProcedureP  proc = reinterpret_cast <LxoTurfProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

        if (0 == keyword.compare (L"primary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetTurfColorR ().red, &proc->GetTurfColorR ().green, &proc->GetTurfColorR ().blue);
        else if (0 == keyword.compare (L"secondary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetContrastColorR ().red, &proc->GetContrastColorR ().green, &proc->GetContrastColorR ().blue);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoWaterProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoWaterProcedureP  proc = reinterpret_cast <LxoWaterProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

        if (0 == keyword.compare (L"primary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetWaterColorR ().red, &proc->GetWaterColorR ().green, &proc->GetWaterColorR ().blue);
        else if (0 == keyword.compare (L"ripple_scale"))
            proc->SetRippleScale (BeStringUtilities::Wtof (value.c_str ()));
        else if (0 == keyword.compare (L"ripple_level_of_detail"))
            proc->SetRippleComplexity (BeStringUtilities::Wtof (value.c_str ()));
        else if (0 == keyword.compare (L"wave_scale"))
            proc->SetWaveScale (BeStringUtilities::Wtof (value.c_str ()));
        else if (0 == keyword.compare (L"wave_level_of_detail"))
            proc->SetWaveComplexity (BeStringUtilities::Wtof (value.c_str ()));
        else if (0 == keyword.compare (L"wave_minimum"))
            proc->SetWaveMinimum (BeStringUtilities::Wtof (value.c_str ()) * 100.0);
        else if (0 == keyword.compare (L"ripples_per_wave"))
            proc->SetRipplesPerWave (BeStringUtilities::Wtof (value.c_str ()));
        else if (0 == keyword.compare (L"step_size"))
            proc->SetRoughness (BeStringUtilities::Wtof (value.c_str ()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoWavesProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoWavesProcedureP  proc = reinterpret_cast <LxoWavesProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

        if (0 == keyword.compare (L"primary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetWavesColorR ().red, &proc->GetWavesColorR ().green, &proc->GetWavesColorR ().blue);
        else if (0 == keyword.compare (L"secondary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetContrastColorR ().red, &proc->GetContrastColorR ().green, &proc->GetContrastColorR ().blue);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoBentleyWoodProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoBentleyWoodProcedureP  proc = reinterpret_cast <LxoBentleyWoodProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

    if (0 == keyword.compare (L"primary_color"))
        BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetWoodColorR ().red, &proc->GetWoodColorR ().green, &proc->GetWoodColorR ().blue);
    else if (0 == keyword.compare (L"secondary_color"))
        BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetRingColorR ().red, &proc->GetRingColorR ().green, &proc->GetRingColorR ().blue);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoAdvancedWoodProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoAdvancedWoodProcedureP  proc = reinterpret_cast <LxoAdvancedWoodProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

        if (0 == keyword.compare (L"primary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetWoodColorR ().red, &proc->GetWoodColorR ().green, &proc->GetWoodColorR ().blue);
        else if (0 == keyword.compare (L"secondary_color"))
            BE_STRING_UTILITIES_SWSCANF (value.c_str (), L"%lf %lf %lf", &proc->GetRingColorR ().red, &proc->GetRingColorR ().green, &proc->GetRingColorR ().blue);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoBGradientProcedureFromLegacyData (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    LxoBGradientProcedureP  proc = reinterpret_cast <LxoBGradientProcedureP> (&procedure);
    WString             stringValue;

    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;

        childNode->GetContent (content);

        size_t      index = content.find_first_of ('_');

        if (WString::npos == index)
            continue;

        WString keyword (content, index+1);
        index = keyword.find_first_of (' ');
        WString value (keyword, index+1);

        if (0 == keyword.compare (L"input"))
            proc->SetGradientInput (static_cast <LxoGradientProcedure::GradientInput> (BeStringUtilities::Wtoi (value.c_str ())));
        else if (0 == keyword.compare (L"u_x_offsets"))
            {
            LxoFloatEnvelopeR   gValue = proc->GetValueEnvelopeR (), gRed = proc->GetRedEnvelopeR (), gGreen = proc->GetGreenEnvelopeR (),
                                gBlue = proc->GetBlueEnvelopeR (), gAlpha = proc->GetAlphaEnvelopeR ();
            WCharCP             pStr, pStrEnd;
            uint32_t            length;

            gValue.GetComponentsR ().Clear ();
            gRed.GetComponentsR ().Clear ();
            gGreen.GetComponentsR ().Clear ();
            gBlue.GetComponentsR ().Clear ();
            gAlpha.GetComponentsR ().Clear ();
            pStr = value.c_str ();
            pStrEnd = value.c_str () + value.length ();

            for (;pStr < pStrEnd; pStr += length+1)
                {
                double  data;

                BE_STRING_UTILITIES_SWSCANF (pStr, L"%lf%n", &data, &length);

                LxoFloatEnvelopeComponentR vComp = gValue.GetComponentsR ().AddComponent ();
                vComp.GetKeyR ().GetValueR ().x = data;

                LxoFloatEnvelopeComponentR rComp = gRed.GetComponentsR ().AddComponent ();
                rComp.GetKeyR ().GetValueR ().x = data;

                LxoFloatEnvelopeComponentR gComp = gGreen.GetComponentsR ().AddComponent ();
                gComp.GetKeyR ().GetValueR ().x = data;

                LxoFloatEnvelopeComponentR bComp = gBlue.GetComponentsR ().AddComponent ();
                bComp.GetKeyR ().GetValueR ().x = data;

                LxoFloatEnvelopeComponentR aComp = gAlpha.GetComponentsR ().AddComponent ();
                aComp.GetKeyR ().SetValue (data, 1.0);
                }
            }
        else if (0 == keyword.compare (L"x_colors"))
            {
            LxoFloatEnvelopeR   gValue = proc->GetValueEnvelopeR (), gRed = proc->GetRedEnvelopeR (), gGreen = proc->GetGreenEnvelopeR (),
                                gBlue = proc->GetBlueEnvelopeR ();
            WCharCP             pStr, pStrEnd;
            uint32_t            length = 0, index = 0;

            pStr = value.c_str ();
            pStrEnd = value.c_str () + value.length ();

            for (;pStr < pStrEnd;index++)
                {
                RgbFactor   rgb;

                BE_STRING_UTILITIES_SWSCANF (pStr, L"%lf%n", &rgb.red, &length);
                pStr+=length+1;
                BE_STRING_UTILITIES_SWSCANF (pStr, L"%lf%n", &rgb.green, &length);
                pStr+=length+1;
                BE_STRING_UTILITIES_SWSCANF (pStr, L"%lf%n", &rgb.blue, &length);
                pStr+=length+1;

                LxoFloatEnvelopeComponentPtrList::iterator          vIterator = gValue.GetComponentsR ().begin ();
                LxoFloatEnvelopeComponentPtr                        vComp = *(vIterator += index);
                vComp->GetKeyR ().GetValueR ().y = rgb.red;

                LxoFloatEnvelopeComponentPtrList::iterator          rIterator = gRed.GetComponentsR ().begin ();
                LxoFloatEnvelopeComponentPtr                        rComp = *(rIterator += index);
                rComp->GetKeyR ().GetValueR ().y = rgb.red;

                LxoFloatEnvelopeComponentPtrList::iterator          gIterator = gGreen.GetComponentsR ().begin ();
                LxoFloatEnvelopeComponentPtr                        gComp = *(gIterator += index);
                gComp->GetKeyR ().GetValueR ().y = rgb.green;

                LxoFloatEnvelopeComponentPtrList::iterator          bIterator = gBlue.GetComponentsR ().begin ();
                LxoFloatEnvelopeComponentPtr                        bComp = *(bIterator += index);
                bComp->GetKeyR ().GetValueR ().y = rgb.blue;
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     04/10
//---------------------------------------------------------------------------------------
void XmlMaterialReader::GetLxoProcedure (LxoProcedureR procedure, BeXmlNodeR rootElement)
    {
    switch (procedure.GetType ())
        {
        case LxoProcedure::PROCEDURETYPE_Noise:
            GetLxoNoiseProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Checker:
            GetLxoCheckerProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Grid:
            GetLxoGridProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Dot:
            GetLxoDotProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Constant:
            GetLxoConstantProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Cellular:
            GetLxoCellularProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Wood:
            GetLxoWoodProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Weave:
            GetLxoWeaveProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Ripples:
            GetLxoRipplesProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Gradient:
            GetLxoGradientProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Occlusion:
            GetLxoOcclusionProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Boards:
            GetLxoBoardsProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Brick:
            GetLxoBoardsProcedureFromLegacyData (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_BentleyChecker:
            GetLxoBentleyCheckerProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Checker3d:
            GetLxoChecker3dProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Clouds:
            GetLxoCloudsProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Flame:
            GetLxoFlameProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Fog:
            GetLxoFogProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Marble:
            GetLxoMarbleProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Sand:
            GetLxoSandProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Stone:
            GetLxoStoneProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Turbulence:
            GetLxoTurbulenceProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Turf:
            GetLxoTurfProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Water:
            GetLxoWaterProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Waves:
            GetLxoWavesProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_BentleyWood:
            GetLxoBentleyWoodProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_AdvancedWood:
            GetLxoAdvancedWoodProcedure (procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_RGBColorCube:
        case LxoProcedure::PROCEDURETYPE_BWNoise:
        case LxoProcedure::PROCEDURETYPE_ColorNoise:
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLxoProcedure (MaterialMapLayerR layer, BeXmlNodeR rootElement)
    {
    BeXmlNodeP childElement = rootElement.SelectSingleNode (INTERNALMATERIALTAGS_LxoProcedure);
    if (NULL == childElement)
        return;

    uint32_t uint32Value;
    if (BEXML_Success != childElement->GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_LxoProcedureType))
        return;

    LxoProcedureP procedure = layer.AddLxoProcedure (static_cast <LxoProcedure::ProcedureType> (uint32Value));
    if (NULL != procedure)
        GetLxoProcedure (*procedure, *childElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetLegacyProcedure (MaterialMapLayer::LegacyProcedureDataR data, BeXmlNodeR rootElement, MaterialVersion version)
    {
    WString     stringValue;
    for (BeXmlNodeP childNode = rootElement.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        WString     content;
        childNode->GetContent (content);
        data.push_back (content.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::SetLegacyLayerFlagsFromUInt32 (MaterialMapLayerR layer, uint32_t uint32Value)
    {
    LegacyLayerFlags* flags = reinterpret_cast <LegacyLayerFlags *> (&uint32Value);
    layer.SetIsEnabled                  (TO_BOOL(flags->m_on));
    layer.SetIsBackgroundTransparent    (TO_BOOL(flags->m_bgTrans));
    layer.SetIsInverted                 (TO_BOOL(flags->m_invert));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::SetUInt32FromLegacyLayerFlags (uint32_t& uint32Value, MaterialMapLayerCR layer)
    {
    LegacyLayerFlags* flags = reinterpret_cast <LegacyLayerFlags *> (&uint32Value);
    flags->m_on             = layer.IsEnabled ();
    flags->m_bgTrans        = layer.IsBackgroundTransparent ();
    flags->m_invert         = layer.IsInverted ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus XmlMaterialReader::SkipLayerToken (WStringR layerType)
    {
    size_t pos = layerType.find (INTERNALMATERIALTAGS_LayerPrefix);
    if (string::npos == pos)
        return ERROR;

    layerType = layerType.substr (pos + wcslen (INTERNALMATERIALTAGS_LayerPrefix));
    layerType.Trim ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  XmlMaterialReader::ConvertProcedureToLuxology (MaterialMapLayerR layer, WCharCP name, BeXmlNodeR rootElement, MaterialVersion version)
    {
    LxoProcedure::ProcedureType     type;

    if (0 == BeStringUtilities::Wcsicmp (name, L"boards"))
        type = LxoProcedure::PROCEDURETYPE_Boards;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"brick"))
        type = LxoProcedure::PROCEDURETYPE_Brick;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"checker"))
        type = LxoProcedure::PROCEDURETYPE_BentleyChecker;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"checkr3d"))
        type = LxoProcedure::PROCEDURETYPE_Checker3d;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"bwnoise"))
        type = LxoProcedure::PROCEDURETYPE_BWNoise;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"clouds"))
        type = LxoProcedure::PROCEDURETYPE_Clouds;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"colnoise"))
        type = LxoProcedure::PROCEDURETYPE_ColorNoise;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"flame"))
        type = LxoProcedure::PROCEDURETYPE_Flame;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"fog"))
        type = LxoProcedure::PROCEDURETYPE_Fog;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"marble"))
        type = LxoProcedure::PROCEDURETYPE_Marble;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"rgb"))
        type = LxoProcedure::PROCEDURETYPE_RGBColorCube;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"sand"))
        type = LxoProcedure::PROCEDURETYPE_Sand;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"stone"))
        type = LxoProcedure::PROCEDURETYPE_Stone;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"turbulnc"))
        type = LxoProcedure::PROCEDURETYPE_Turbulence;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"turf"))
        type = LxoProcedure::PROCEDURETYPE_Turf;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"water"))
        type = LxoProcedure::PROCEDURETYPE_Water;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"waves"))
        type = LxoProcedure::PROCEDURETYPE_Waves;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"wood"))
        type = LxoProcedure::PROCEDURETYPE_BentleyWood;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"wood01"))
        type = LxoProcedure::PROCEDURETYPE_AdvancedWood;
    else if (0 == BeStringUtilities::Wcsicmp (name, L"grad1d"))
        type = LxoProcedure::PROCEDURETYPE_BGradient;
    else
        return ERROR;

    LxoProcedureP procedure = layer.AddLxoProcedure (type);
    layer.SetType (MaterialMapLayer::LAYERTYPE_LxoProcedure);

    switch (type)
        {
        case LxoProcedure::PROCEDURETYPE_Boards:
            GetLxoBoardsProcedureFromLegacyData (*procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Brick:
            GetLxoBoardsProcedureFromLegacyData (*procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_BWNoise:
            break;
        case LxoProcedure::PROCEDURETYPE_BentleyChecker:
            GetLxoCheckerProcedureFromLegacyData (*procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Checker3d:
            GetLxoChecker3dProcedureFromLegacyData (*procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Clouds:
            GetLxoCloudsProcedureFromLegacyData (*procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_ColorNoise:
            break;
        case LxoProcedure::PROCEDURETYPE_Flame:
            GetLxoFlameProcedureFromLegacyData (*procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Fog:
            GetLxoFogProcedureFromLegacyData (*procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Marble:
            GetLxoMarbleProcedureFromLegacyData (*procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_RGBColorCube:
            break;
        case LxoProcedure::PROCEDURETYPE_Sand:
            GetLxoSandProcedureFromLegacyData (*procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Stone:
            GetLxoStoneProcedureFromLegacyData (*procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Turbulence:
            GetLxoTurbulenceProcedureFromLegacyData (*procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Turf:
            GetLxoTurfProcedureFromLegacyData (*procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Water:
            GetLxoWaterProcedureFromLegacyData (*procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_Waves:
            GetLxoWavesProcedureFromLegacyData (*procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_BentleyWood:
            GetLxoBentleyWoodProcedureFromLegacyData (*procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_AdvancedWood:
            GetLxoAdvancedWoodProcedureFromLegacyData (*procedure, rootElement);
            break;
        case LxoProcedure::PROCEDURETYPE_BGradient:
            GetLxoBGradientProcedureFromLegacyData (*procedure, rootElement);
            layer.SetType (MaterialMapLayer::LAYERTYPE_Gradient);
            break;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     12/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus XmlMaterialReader::GetMaterialMapLayer (MaterialMapLayerP layer, BeXmlNodeR layerElement, MaterialVersion version)
    {
    MaterialTokenManagerR   tokenManager = MaterialTokenManager::GetManagerR ();
    uint32_t                uint32Value;
    int32_t                 int32Value;
    WString                 stringValue;
    DPoint2d                point2dValue;
    double                  doubleValue;
    RgbFactor               colorValue;

    if (BEXML_Success != layerElement.GetAttributeStringValue (stringValue, INTERNALMATERIALTAGS_LayerType) || SUCCESS != SkipLayerToken (stringValue))
        return ERROR;

    WString                     layerArg, ext, name;
    MaterialMapLayer::LayerType layerType;

    layerType = tokenManager.ResolveLayerTypeFromToken (tokenManager.ResolveLayerTypeToken (layerArg, stringValue.c_str ()));
    layer->SetType (layerType);

    if (BEXML_Success == layerElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_LayerDataFlags))    { layer->SetBasicFlags (uint32Value); }

    switch (layerType)
        {
        default:
            LoggingManager::GetLogger (L"DgnCore.DgnMaterials")->messagev (LOG_WARNING, L"GetMaterialMapLayer Bad LayerType  %d",layerType);
            BeAssert (false);
            break;
        case MaterialMapLayer::LAYERTYPE_Image:
        case MaterialMapLayer::LAYERTYPE_Procedure:
        case MaterialMapLayer::LAYERTYPE_Gradient:
        case MaterialMapLayer::LAYERTYPE_LxoProcedure:
            {
            layer->SetFileName (layerArg.c_str ());

            if (BEXML_Success == layerElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_LayerFlags))        { SetLegacyLayerFlagsFromUInt32 (*layer, uint32Value); }

            if (GetInt32 (int32Value, layerElement, PALETTETOKEN_PatternMapping, version))           { layer->SetMode (static_cast <MapMode> (int32Value)); }
            if (GetInt32 (int32Value, layerElement, PALETTETOKEN_PatternScaleMode, version))         { layer->SetUnits (static_cast <MapUnits> (int32Value)); }
            if (GetInt32 (int32Value, layerElement, PALETTETOKEN_TextureFilterType, version))        { layer->SetTextureFilterType (static_cast <MaterialMapLayer::TextureFilterType> (int32Value)); }

            if (GetDouble    (doubleValue,   layerElement,   PALETTETOKEN_PatternAngle, version))    { layer->SetRotation (doubleValue); }
            if (GetDouble    (doubleValue,   layerElement,   PALETTETOKEN_LayerOpacity, version))    { layer->SetOpacity (doubleValue); }
            if (GetDouble    (doubleValue,   layerElement,   PALETTETOKEN_ImageGamma, version))      { layer->SetGamma (doubleValue); }
            if (GetPoint2d   (point2dValue,  layerElement,   PALETTETOKEN_PatternScale, version))    { layer->GetScaleR ().Init (point2dValue); }
            if (GetDouble    (doubleValue,   layerElement,   PALETTETOKEN_ScaleZ, version))          { layer->GetScaleR ().z = doubleValue; }

            if (GetDouble    (doubleValue,   layerElement,   PALETTETOKEN_LowValue, version))        { layer->SetLowValue (doubleValue); }
            if (GetDouble    (doubleValue,   layerElement,   PALETTETOKEN_HighValue, version))       { layer->SetHighValue (doubleValue); }
            if (GetDouble    (doubleValue,   layerElement,   PALETTETOKEN_AntialiasStrength, version))   { layer->SetAntiAliasStrength (doubleValue); }
            if (GetDouble    (doubleValue,   layerElement,   PALETTETOKEN_MinimumSpot, version))     { layer->SetMinimumSpot (doubleValue); }

            if (GetPoint2d   (point2dValue,  layerElement,   PALETTETOKEN_PatternOffset, version))
                {
                if (!GetDouble (doubleValue, layerElement, PALETTETOKEN_OffsetZ, version))
                    doubleValue = 0.0;

                layer->SetOffset (point2dValue.x, point2dValue.y, doubleValue);
                }

            if (0.0 == layer->GetScaleR ().z)
                layer->GetScaleR ().z = 1.0;
            BeFileName::ParseName (NULL, NULL, &name, &ext, layerArg.c_str ());
            if (ext.EqualsI(L"pma"))
                {
                if (SUCCESS != ConvertProcedureToLuxology (*layer, name.c_str(), layerElement, version))
                    GetLegacyProcedure (layer->GetLegacyProcedureDataR (), layerElement, version);
                }
            else if (MaterialMapLayer::LAYERTYPE_LxoProcedure == layerType || MaterialMapLayer::LAYERTYPE_Gradient == layerType)
                GetLxoProcedure (*layer, layerElement);

            break;
            }
        case MaterialMapLayer::LAYERTYPE_GroupStart:
        case MaterialMapLayer::LAYERTYPE_GroupEnd:
        case MaterialMapLayer::LAYERTYPE_AlphaBackgroundStart:
        case MaterialMapLayer::LAYERTYPE_AlphaBackgroundEnd:
            {
            layer->SetOperatorStringValue (layerArg.c_str ());

            if (BEXML_Success == layerElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_LayerFlags))        { SetLegacyLayerFlagsFromUInt32 (*layer, uint32Value); }
            break;
            }
        case MaterialMapLayer::LAYERTYPE_Normal:
        case MaterialMapLayer::LAYERTYPE_Alpha:
        case MaterialMapLayer::LAYERTYPE_Add:
        case MaterialMapLayer::LAYERTYPE_Subtract:
        case MaterialMapLayer::LAYERTYPE_Dissolve:
        case MaterialMapLayer::LAYERTYPE_Atop:
        case MaterialMapLayer::LAYERTYPE_In:
        case MaterialMapLayer::LAYERTYPE_Out:
        case MaterialMapLayer::LAYERTYPE_Difference:
        case MaterialMapLayer::LAYERTYPE_NormalMultiply:
        case MaterialMapLayer::LAYERTYPE_Divide:
        case MaterialMapLayer::LAYERTYPE_Multiply:
        case MaterialMapLayer::LAYERTYPE_Screen:
        case MaterialMapLayer::LAYERTYPE_Overlay:
        case MaterialMapLayer::LAYERTYPE_SoftLight:
        case MaterialMapLayer::LAYERTYPE_HardLight:
        case MaterialMapLayer::LAYERTYPE_Darken:
        case MaterialMapLayer::LAYERTYPE_Lighten:
        case MaterialMapLayer::LAYERTYPE_ColorDodge:
        case MaterialMapLayer::LAYERTYPE_ColorBurn:
            {
            if (BEXML_Success == layerElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_LayerFlags))        { SetLegacyLayerFlagsFromUInt32 (*layer, uint32Value); }
            break;
            }
        case MaterialMapLayer::LAYERTYPE_Gamma:
            {
            if (GetDouble (doubleValue, layerElement, PALETTETOKEN_LayerGamma, version))                         { layer->SetOperatorDoubleValue (doubleValue); }
            if (BEXML_Success == layerElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_LayerFlags))        { SetLegacyLayerFlagsFromUInt32 (*layer, uint32Value); }
            break;
            }
        case MaterialMapLayer::LAYERTYPE_Tint:
            {
            if (GetColor (colorValue, layerElement, PALETTETOKEN_LayerColor, version))                           { layer->GetOperatorColorValueR () = colorValue; }
            if (BEXML_Success == layerElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_LayerFlags))        { SetLegacyLayerFlagsFromUInt32 (*layer, uint32Value); }
            break;
            }
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetMaterialMapLayers (MaterialMapR map, BeXmlNodeR rootElement, MaterialVersion version)
    {
    MaterialMapLayerP       layer        = &map.GetLayersR ().GetTopLayerR ();

    for (BeXmlNodeP childElement = rootElement.GetFirstChild(); NULL != childElement; childElement = childElement->GetNextSibling())
        {
        if (SUCCESS != GetMaterialMapLayer (layer, *childElement, version))
            continue;
        // we add the layer before we execute the loop, so don't do it for the last one.
        if (NULL != childElement->GetNextSibling())
            layer = &map.GetLayersR ().AddLayer ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetMaterialMap (MaterialSettingsR settings, BeXmlNodeR rootElement, MaterialVersion version)
    {
    int32_t                 int32Value;
    uint32_t                uint32Value;
    double                  doubleValue;
    DPoint3d                point3dValue;
    DPoint2d                point2dValue;
    WString                 stringValue;
    MaterialMapP            map;
    MaterialMap::MapType    linkType = MaterialMap::MAPTYPE_None;

    if (BEXML_Success != rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_Type) ||
        NULL == (map = settings.GetMapsR ().AddMap (static_cast <MaterialMap::MapType> (uint32Value))))
        return;

    if (GetInt32 (int32Value, rootElement,       PALETTETOKEN_MapLink, version))             { linkType = static_cast <MaterialMap::MapType> (int32Value); }
    if (GetInt32  (int32Value, rootElement,      PALETTETOKEN_PatternOff, version))          { map->SetIsEnabled (!TO_BOOL (int32Value)); }
    if (GetPoint3d (point3dValue, rootElement,   PALETTETOKEN_ProjectionOffset, version))    { map->SetProjectionOffset (point3dValue.x, point3dValue.y, point3dValue.z); }
    if (GetPoint3d (point3dValue, rootElement,   PALETTETOKEN_ProjectionAngles, version))    { map->SetProjectionRotation (point3dValue.x, point3dValue.y, point3dValue.z); }
    if (GetPoint3d (point3dValue, rootElement,   PALETTETOKEN_ProjectionScale, version))
        {
        if (0.0 == point3dValue.x) point3dValue.x = 1.0;
        if (0.0 == point3dValue.y) point3dValue.y = 1.0;
        if (0.0 == point3dValue.z) point3dValue.z = 1.0;
        map->SetProjectionScale (point3dValue.x, point3dValue.y, point3dValue.z);
        }

    switch (map->GetType ())
        {
        case MaterialMap::MAPTYPE_Pattern:
            if (GetDouble (doubleValue, rootElement, PALETTETOKEN_PatternWeight, version))
                map->SetValue (doubleValue);
            break;
        case MaterialMap::MAPTYPE_Bump:
            if (GetDouble (doubleValue, rootElement, PALETTETOKEN_BumpMapScale, version))
                map->SetValue (doubleValue);
            break;
        }

    // Prior to DgnPlatform, even though all materials had at least one layer, only those with multiple layers were counted as full-fledged
    // layered materials.  For materials with only one layer, the layer data was stored in the main material structure.  Now that this redundancy
    // has been eliminated, we need to watch for the single layer case and handle it appropriately.
    MaterialMapLayerR topLayer = map->GetLayersR ().GetTopLayerR ();
    if (BEXML_Success == rootElement.GetAttributeStringValue (stringValue, INTERNALMATERIALTAGS_FileName))
        {
        if (IsLayeredProcedural (stringValue.c_str ()))
            {
            GetMaterialMapLayers (*map, rootElement, version);
            map->SetLinkType  (linkType);
            return;
            }

        topLayer.SetFileName (stringValue.c_str ());
        }
    topLayer.SetIsEnabled (map->IsEnabled ());

    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_Flags))  { topLayer.SetBasicFlags (uint32Value); }
    if (GetUInt32 (uint32Value, rootElement,     PALETTETOKEN_LayerType, version))
        {
        topLayer.SetType (static_cast <MaterialMapLayer::LayerType> (uint32Value));
        if (MaterialMapLayer::LAYERTYPE_None == topLayer.GetType())
            {
            if (IsPMAProcedural (topLayer.GetFileName ().c_str ()))
                topLayer.SetType (MaterialMapLayer::LAYERTYPE_Procedure);
            else
                topLayer.SetType (MaterialMapLayer::LAYERTYPE_Image);
            }
        else if (MaterialMapLayer::LAYERTYPE_8119LxoProcedure == topLayer.GetType())
            topLayer.SetType (MaterialMapLayer::LAYERTYPE_LxoProcedure);
        }
    if (MaterialMap::MAPTYPE_Geometry == map->GetType ())
        topLayer.SetType (MaterialMapLayer::LAYERTYPE_Cell);

    if (GetInt32 (int32Value, rootElement,       PALETTETOKEN_PatternMapping, version))      { topLayer.SetMode (static_cast <MapMode> (int32Value)); }
    if (GetInt32 (int32Value, rootElement,       PALETTETOKEN_PatternScaleMode, version))    { topLayer.SetUnits (static_cast <MapUnits> (int32Value)); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_PatternAngle, version))        { topLayer.SetRotation (doubleValue); }
    if (GetPoint2d (point2dValue, rootElement,   PALETTETOKEN_PatternScale, version))        { topLayer.GetScaleR ().Init (point2dValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_ScaleZ, version))              { topLayer.GetScaleR ().z = doubleValue; }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_ImageGamma, version))          { topLayer.SetGamma (doubleValue); }
    if (GetPoint2d (point2dValue, rootElement,   PALETTETOKEN_PatternOffset, version))
        {
        if (!GetDouble (doubleValue, rootElement, PALETTETOKEN_OffsetZ, version))
            doubleValue = 0.0;

        topLayer.SetOffset (point2dValue.x, point2dValue.y, doubleValue);
        }
    if (0.0 == topLayer.GetScaleR ().z)
        topLayer.GetScaleR ().z = 1.0;

    Utf8CP flagName = GetFlagNameFromType (*map);
    if (NULL != flagName && BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, flagName))
        {
        // Old flag structure only used 1 field to set background transparency or map inversion.
        if (MaterialMap::MAPTYPE_Pattern == map->GetType ())
            topLayer.SetIsBackgroundTransparent (uint32Value & 1);
        else
            topLayer.SetIsInverted (uint32Value & 1);
        }

    if (MaterialMapLayer::LAYERTYPE_LxoProcedure == topLayer.GetType ())
        GetLxoProcedure (topLayer, rootElement);
    else
        GetLegacyProcedure (topLayer.GetLegacyProcedureDataR (), rootElement, version);

    map->SetLinkType  (linkType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetMaterialFur (MaterialFurR fur, BeXmlNodeR rootElement, MaterialVersion version, DgnDbR source)
    {
    int32_t int32Value;
    uint32_t uint32Value;
    double  doubleValue;

    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_Flags))
        fur.SetFlags (uint32Value);

    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurSpacing, version))          { fur.SetSpacingInMeters (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurLength, version))           { fur.SetLengthInMeters (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurWidth, version))            { fur.SetWidthScale (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurTaper, version))            { fur.SetTaperScale (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurOffset, version))           { fur.SetOffsetInMeters (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurStripRotation, version))    { fur.SetStripRotation (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurGrowthJitter, version))     { fur.SetGrowthJitter (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurPositionJitter, version))   { fur.SetPositionJitter (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurDirectionJitter, version))  { fur.SetDirectionJitter (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurSizeJitter, version))       { fur.SetSizeJitter (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurFlex, version))             { fur.SetFlex (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurRootBend, version))         { fur.SetRootBend (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurCurls, version))            { fur.SetCurls (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurBumpAmplitude, version))    { fur.SetBumpAmplitude (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurGuideRange, version))       { fur.SetGuideRangeInMeters (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurGuideLength, version))      { fur.SetGuideLengthInMeters (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurBlendAmount, version))      { fur.SetBlendAmount (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurBlendAngle, version))       { fur.SetBlendAngle (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurClumps, version))           { fur.SetClumpScale (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurClumpRange, version))       { fur.SetClumpRangeInMeters (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_FurRate, version))             { fur.SetRate (doubleValue); }
    if (GetInt32  (int32Value,  rootElement, PALETTETOKEN_FurSegments, version))         { fur.SetSegmentCount (int32Value); }
    if (GetInt32  (int32Value,  rootElement, PALETTETOKEN_FurBillboard, version))        { fur.SetFurBillboard (static_cast <MaterialFur::FurBillboard> (int32Value)); }

    if (GetInt32 (int32Value, rootElement,   PALETTETOKEN_FurType, version))
        fur.SetType (static_cast <MaterialFur::FurType> (int32Value));

    if (GetInt32 (int32Value, rootElement,   PALETTETOKEN_FurGuides, version))
        fur.SetFurGuides (static_cast <MaterialFur::FurGuides> (int32Value));

    WString stringValue;
    if (BEXML_Success == rootElement.GetAttributeStringValue (stringValue, INTERNALMATERIALTAGS_FurMaterialName))
        fur.GetFurMaterialNameR().Assign (stringValue.c_str());

    if (BEXML_Success == rootElement.GetAttributeStringValue (stringValue, INTERNALMATERIALTAGS_FurPaletteName))
        fur.GetFurMaterialPaletteR ().Assign (Material::ParseLegacyPaletteName (stringValue.c_str()).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetMaterialShader (MaterialShaderR shader, BeXmlNodeR rootElement, MaterialVersion version)
    {
    int32_t int32Value;
    uint32_t uint32Value;
    double  doubleValue;

    if (GetInt32 (int32Value, rootElement, PALETTETOKEN_ShaderType, version))
        shader.SetType (static_cast <MaterialShader::ShaderType> (int32Value));

    if (GetInt32 (int32Value, rootElement, PALETTETOKEN_ShaderBlend, version))
        shader.SetBlendMode (static_cast <MaterialShader::BlendMode> (int32Value));

    if (GetInt32 (int32Value, rootElement, PALETTETOKEN_ShaderIndirectIllumType, version))
        shader.SetIndirectIlluminationType (static_cast <MaterialShader::IlluminationType> (int32Value));

    if (GetInt32 (int32Value, rootElement, PALETTETOKEN_ShaderEffect, version))
        shader.SetEffect (static_cast <MaterialShader::ShaderEffect> (int32Value));

    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_Flags))
        shader.SetFlags (uint32Value);

    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_ShaderRate, version))                  { shader.SetShadingRate (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_ShaderDirectMultiplier, version))      { shader.SetDirectIlluminationMultiplier (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_ShaderIndirectMultiplier, version))    { shader.SetIndirectIlluminationMultiplier (doubleValue); }
    if (GetDouble (doubleValue, rootElement, PALETTETOKEN_ShaderSaturation, version))            { shader.SetIndirectIlluminationSaturation (doubleValue); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     12/10
+---------------+---------------+---------------+---------------+---------------+------*/
void XmlMaterialReader::GetMaterialSettings (MaterialSettingsR settings, MaterialVersion version, BeXmlNodeR rootElement, DgnDbR source)
    {
    uint32_t            uint32Value;
    int32_t             int32Value;
    double              doubleValue;
    RgbFactor           colorValue;

    if (BEXML_Success == rootElement.GetAttributeUInt32Value (uint32Value, INTERNALMATERIALTAGS_Flags))
        settings.SetFlags (uint32Value);

    if (GetColor (colorValue, rootElement,       PALETTETOKEN_Color, version))                       { settings.GetBaseColorR () = colorValue; }
    if (GetColor (colorValue, rootElement,       PALETTETOKEN_SpecularColor, version))               { settings.GetSpecularColorR () = colorValue; }
    if (GetColor (colorValue, rootElement,       PALETTETOKEN_TransparentColor, version))            { settings.GetTransmitColorR () = colorValue; }
    if (GetColor (colorValue, rootElement,       PALETTETOKEN_TranslucencyColor, version))           { settings.GetTranslucencyColorR () = colorValue; }
    if (GetColor (colorValue, rootElement,       PALETTETOKEN_GlowColor, version))                   { settings.GetGlowColorR () = colorValue; }
    if (GetColor (colorValue, rootElement,       PALETTETOKEN_ReflectColor, version))                { settings.GetReflectColorR () = colorValue; }
    if (GetColor (colorValue, rootElement,       PALETTETOKEN_ExitColor, version))                   { settings.GetExitColorR () = colorValue; }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_Ambient, version))                     { settings.SetAmbientIntensity (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_Finish, version))                      { settings.SetFinishScaleFromStorage (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_Reflect, version))                     { settings.SetReflectIntensity (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_Transmit, version))                    { settings.SetTransmitIntensity (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_Diffuse, version))                     { settings.SetDiffuseIntensity (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_Specular, version))                    { settings.SetSpecularIntensity (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_Refract, version))                     { settings.SetRefractIndex (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_Thickness, version))                   { settings.SetThickness (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_Glow, version))                        { settings.SetGlowIntensity (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_Translucency, version))                { settings.SetTranslucencyScale (doubleValue); }
    if (GetInt32 (int32Value, rootElement,       PALETTETOKEN_ReflectionRays, version))              { settings.SetReflectionRays (int32Value); }
    if (GetInt32 (int32Value, rootElement,       PALETTETOKEN_RefractionRays, version))              { settings.SetRefractionRays (int32Value); }
    if (GetInt32 (int32Value, rootElement,       PALETTETOKEN_SubsurfaceSamples, version))           { settings.SetSubsurfaceSamples (int32Value); }
    if (GetInt32 (int32Value, rootElement,       PALETTETOKEN_BlurReflections, version))             { settings.SetBlurReflections (TO_BOOL (int32Value)); }
    if (GetInt32 (int32Value, rootElement,       PALETTETOKEN_BlurRefractions, version))             { settings.SetBlurRefractions (TO_BOOL (int32Value)); }
    if (GetInt32 (int32Value, rootElement,       PALETTETOKEN_UseFur, version))                      { settings.SetUseFur (TO_BOOL (int32Value)); }
    if (GetInt32 (int32Value, rootElement,       PALETTETOKEN_CutSectionUseMaterial, version))       { settings.SetUseCutSectionMaterial (TO_BOOL (int32Value)); }
    if (GetInt32 (int32Value, rootElement,       PALETTETOKEN_BackFaceCulling, version))             { settings.SetBackFaceCulling (static_cast <MaterialSettings::BackFaceCulling> (int32Value)); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_DisplacementDistance, version))        { settings.SetDisplacementDistanceInMillimeters (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_Dispersion, version))                  { settings.SetDispersionAmount (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_Clearcoat, version))                   { settings.SetClearcoatAmount (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_Anisotropy, version))                  { settings.SetAnisotropyAmount (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_FrontWeighting, version))              { settings.SetFrontWeightingAmount (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_ScatteringDistance, version))          { settings.SetScatterDistanceInMeters (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_ReflectFresnel, version))              { settings.SetReflectionFresnel (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_Dissolve, version))                    { settings.SetDissolveAmount (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_AbsorptionDistance, version))          { settings.SetAbsorptionDistance (doubleValue); }
    if (GetDouble (doubleValue, rootElement,     PALETTETOKEN_RefractionRoughness, version))         { settings.SetRefractionRoughness (doubleValue); }

    MaterialShaderR shader = settings.GetShadersR ().GetTopShaderR ();
    shader.SetCastsShadows (settings.CastsShadows ());
    shader.SetIsVisibleToCamera (settings.IsVisible ());

    for (BeXmlNodeP childElement = rootElement.GetFirstChild(); NULL != childElement; childElement = childElement->GetNextSibling())
        {
        Utf8CP  nodeName = childElement->GetName();
        if (0 == BeStringUtilities::Stricmp (INTERNALMATERIALTAGS_Map, nodeName))
            GetMaterialMap (settings, *childElement, version);
        else if (0 == BeStringUtilities::Stricmp (INTERNALMATERIALTAGS_Shader, nodeName))
            GetMaterialShader (shader, *childElement, version);
        else if (0 == BeStringUtilities::Stricmp (INTERNALMATERIALTAGS_Fur, nodeName))
            GetMaterialFur (settings.AddFur (), *childElement, version, source);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     08/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialPtr MaterialManager::CreateFromXml (Utf8CP xmlString, DgnDbR source)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString (xmlStatus, xmlString);
    if (xmlDom.IsNull() || (BEXML_Success != xmlStatus))
        return NULL;

    BeXmlNodeP rootElement;
    if (NULL == (rootElement = xmlDom->GetRootElement()))
        return NULL;

    MaterialPtr         material = Material::Create (source);
    MaterialSettingsR   settings = material->GetSettingsR ();
    MaterialVersion     version  = MATERIALVERSION_Invalid;
    int32_t             int32Value;
    if (XmlMaterialReader::GetInt32 (int32Value, *rootElement, PALETTETOKEN_MaterialVersion, version))
        version = static_cast <MaterialVersion> (int32Value);

    XmlMaterialReader::GetMaterialSettings (settings, version, *rootElement, source);

    settings.UpgradeFromVersion (version);

    return material;
    }
